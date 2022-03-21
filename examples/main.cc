#include <sqlite3.h>

#include <chrono>
#include <iostream>
#include <sstream>
#include <fstream>
#include <locale>
#include <algorithm>
#include <cctype>
#include <codecvt>

#ifdef _WIN32
#include <direct.h>
#include <Windows.h>
const char* GBK_LOCALE_NAME = ".936";
#else
#include <unistd.h>
#include <mach-o/dyld.h>
const char* GBK_LOCALE_NAME = "zh_CN.GBK";
#endif

template<class Facet>
struct deletable_facet : Facet
{
	template<class ...Args>
	deletable_facet(Args&& ...args)
		: Facet(std::forward<Args>(args)...) {}
	~deletable_facet() {}
};
using mbs_facet_t = deletable_facet<std::codecvt_byname<wchar_t, char, std::mbstate_t>>;

using namespace std;
typedef void(*xentry)(void);
using Clock = std::chrono::system_clock;
using tp = std::chrono::time_point<Clock>;
using ms = std::chrono::duration<double, std::milli>;
extern "C" int sqlite3_simple_init(sqlite3 *db, char **pzErrMsg, const sqlite3_api_routines *pApi);

// trim from start (in place)
static inline void ltrim(std::string &s) {
	s.erase(s.begin(), std::find_if(s.begin(), s.end(), [](unsigned char ch) {
		return !std::isspace(ch);
	}));
}

// trim from end (in place)
static inline void rtrim(std::string &s) {
	s.erase(std::find_if(s.rbegin(), s.rend(), [](unsigned char ch) {
		return !std::isspace(ch);
	}).base(), s.end());
}

// trim from both ends (in place)
static inline void trim(std::string &s) {
	ltrim(s);
	rtrim(s);
}

std::string ToString(const std::wstring& wstr) {
	const mbs_facet_t& cvt = std::use_facet<mbs_facet_t>(std::locale());
	std::wstring_convert<mbs_facet_t> converter(&cvt);
	std::string str = converter.to_bytes(wstr);
	return str;
}

std::wstring ToWString(const std::string& str) {
	const mbs_facet_t& cvt = std::use_facet<mbs_facet_t>(std::locale());
	std::wstring_convert<mbs_facet_t> converter(&cvt);
	std::wstring wstr = converter.from_bytes(str);
	return wstr;
}

// convert UTF-8 string to wstring
std::wstring utf8_to_wstring(const std::string& str) {
	std::wstring_convert<std::codecvt_utf8<wchar_t>> myconv;
	return myconv.from_bytes(str);
}

// convert wstring to UTF-8 string
std::string wstring_to_utf8(const std::wstring& str) {
	std::wstring_convert<std::codecvt_utf8<wchar_t>> myconv;
	return myconv.to_bytes(str);
}

// convert wstring to UTF-8 string
std::string wstring_to_gbk(const std::wstring& wstr) {
	std::wstring_convert<mbs_facet_t> conv(new mbs_facet_t(GBK_LOCALE_NAME));
	std::string  str = conv.to_bytes(wstr);
	return str;
}

// convert gbk to wstring 
std::wstring gbk_to_wstring(const std::string& str) {
	std::wstring_convert<mbs_facet_t> conv(new mbs_facet_t(GBK_LOCALE_NAME));
	std::wstring  wstr = conv.from_bytes(str);
	return wstr;
}

std::string String2Utf8(const std::string& str) {
#ifdef WIN32
	return wstring_to_utf8(gbk_to_wstring(str));
#else
	return str;
#endif // WIN32
}

std::string Utf82Local(const std::string& str) {
#ifdef WIN32
	return wstring_to_gbk(utf8_to_wstring(str));
#else
	return str;
#endif // WIN32
}

std::string localizeString(const string& str) {
#ifdef WIN32
	return wstring_to_gbk(utf8_to_wstring(str));
#else
	return str;
#endif // WIN32
}

// https://www.tutorialspoint.com/find-out-the-current-working-directory-in-c-cplusplus
std::string get_current_dir() {
#ifdef WIN32
	char szapipath[MAX_PATH] = { 0 };
	GetModuleFileNameA(NULL, szapipath, MAX_PATH);
	char* find = strrchr(szapipath, '\\');
	*find = L'\0';
	return szapipath;
#else
	char path[512];
	unsigned size = 512;
	_NSGetExecutablePath(path, &size);
	char* find = strrchr(path, '/');
	*find = '\0';
	return path;
#endif
}

// Create a callback function
int callback(void *NotUsed, int argc, char **argv, char **azColName) {
	// int argc: holds the number of results
	// (array) azColName: holds each column returned
	// (array) argv: holds each value
	for (int i = 0; i < argc; i++) {
		// Show column name, value, and newline
		std::cout << azColName[i] << ": " << Utf82Local(argv[i]) << std::endl;
	}
	if (argc > 0) {
		std::cout << std::endl;
	}
	// Return successful
	return 0;
}

void handle_rc(sqlite3 *db, int rc) {
	if (rc != SQLITE_OK) {
		std::cout << "sqlite3 rc: " << rc << ", error: " << sqlite3_errmsg(db) << std::endl;
		exit(rc);
	}
}

int simply_execute(sqlite3* db, const char* sql) {
	char *zErrMsg = nullptr;
	int rc = sqlite3_exec(db, sql, callback, 0, &zErrMsg);
	sqlite3_free(zErrMsg);
	handle_rc(db, rc);
	return rc;
}

void simply_query(sqlite3* db, const char* query, const char* col) {
	std::stringstream ss;
	ss << "select simple_highlight(message_index, 0, '[', ']') as hight";
  ss << ", simple_highlight_pos(message_index, 0) as " << col;
	ss << " from message_index where content match simple_query('"
		<< query << "');";
	simply_execute(db, ss.str().c_str());
}

class PrintTime {
public:
	void Start() {
		begin_ = Clock::now();
	}
	void Print(const char* tag = nullptr) {
		ms ms = Clock::now() - begin_;
		std::cout << "------" << (tag ? tag : "") << " time span " << ms.count() << "ms------" << std::endl;
	}
private:
	tp begin_ = Clock::now();
};

bool clear_db(sqlite3 *db) {
	PrintTime printer;
	std::string sql = R"tt(
		DROP TABLE IF EXISTS message_meta;
		DROP TABLE IF EXISTS message_index;
	)tt";
	simply_execute(db, sql.c_str());
	printer.Print("drop table");

	sql = R"tt(
		DROP INDEX IF EXISTS message_meta_conversation_id_idx;
		DROP INDEX IF EXISTS message_meta_create_time_idx;
		DROP INDEX IF EXISTS message_meta_unique_idx;
	)tt";
	simply_execute(db, sql.c_str());
	printer.Print("drop index");

	sql = R"tt(
		DROP TRIGGER IF EXISTS message_meta_ai;
		DROP TRIGGER IF EXISTS message_meta_ad;
		DROP TRIGGER IF EXISTS message_meta_au;
	)tt";
	simply_execute(db, sql.c_str());
	printer.Print("drop trigger");
	return true;
}

bool create_table_message(sqlite3 *db) {
	std::string sql = R"tt(
		CREATE TABLE IF NOT EXISTS message_meta(
		id INTEGER PRIMARY KEY AUTOINCREMENT NOT NULL,
		sender_id TEXT NOT NULL,
		conversation_id TEXT NOT NULL,
		message_id TEXT NOT NULL,
		message_type INTEGER NOT NULL,
		create_time INTEGER NOT NULL,
		content TEXT NOT NULL);
	)tt";
	PrintTime printer;
	simply_execute(db, sql.c_str());
	printer.Print("create msg table");

	sql = R"tt(
		CREATE  INDEX  IF NOT EXISTS 
		message_meta_conversation_id_idx 
		ON 
		message_meta(conversation_id);
	)tt";
	simply_execute(db, sql.c_str());
	printer.Print("create index message_meta_conversation_id_idx");

	sql = R"tt(
		CREATE  INDEX  IF NOT EXISTS 
		message_meta_create_time_idx 
		ON 
		message_meta(create_time);
	)tt";
	simply_execute(db, sql.c_str());
	printer.Print("create index message_meta_create_time_idx");

	sql = R"tt(
		CREATE  UNIQUE INDEX  IF NOT EXISTS 
		message_meta_unique_idx 
		ON 
		message_meta(message_id);
	)tt";
	simply_execute(db, sql.c_str());
	printer.Print("create UNIQUE INDEX message_meta_unique_idx");

	sql = R"tt(CREATE VIRTUAL TABLE IF NOT EXISTS message_index 
		USING FTS5 (content, content='message_meta', content_rowid=id, tokenize="simple 1");
	)tt";
	simply_execute(db, sql.c_str());
	printer.Print("create VIRTUAL TABLE message_index");

	sql = R"tt(
		CREATE TRIGGER IF NOT EXISTS message_meta_ai AFTER INSERT ON message_meta BEGIN  
			INSERT INTO message_index(rowid, content) VALUES (new.id, new.content); 
		END;
		CREATE TRIGGER IF NOT EXISTS message_meta_ad AFTER DELETE ON message_meta BEGIN
			INSERT INTO message_index(message_index, rowid, content) VALUES('delete', old.id, old.content);
		END;
		CREATE TRIGGER IF NOT EXISTS message_meta_au AFTER UPDATE ON message_meta BEGIN
			INSERT INTO message_index(message_index, rowid, content) VALUES('delete', old.id, old.content);
			INSERT INTO message_index(rowid, content) VALUES (new.id, new.content);
		END;
	)tt";
	simply_execute(db, sql.c_str());
	printer.Print("create trigger message_meta_ai message_meta_ad message_meta_au");
	return true;
}

struct message_mate {
    std::string sender_id;
    std::string conversation_id;
    std::string message_id;
    int32_t message_type;
    int64_t create_time;
    std::string content;

    std::string to_sql() {
        std::stringstream ss;
        ss << "insert into message_meta(sender_id, conversation_id, message_id, message_type, create_time, content) values ('"
            << sender_id << "', '"
            << conversation_id << "', '"
            << message_id << "', "
            << message_type << ", "
            << create_time << ", '"
            << content << "')";
        return ss.str();
    }
};

bool insert_message(sqlite3* db) {
	std::string filePath = get_current_dir() + "/一剑独尊.txt";
	std::ifstream myfile;
	myfile.open(filePath);
	if (!myfile.is_open()) {
		std::cout << "file open failed: " << filePath << std::endl;
		return false;
	}

	int counts = 1000;
	PrintTime printer;
	char buf[255] = "";
	for (int index = 0; index < counts; index++) {
		std::string read;
		if (!std::getline(myfile, read)) {
			break;
		}
		trim(read);
		std::string line = String2Utf8(read);
		if (line.empty()) {
			continue;
		}

		message_mate mate;
		mate.create_time = Clock::now().time_since_epoch().count();
		mate.message_id = std::to_string(index * 10);
		mate.conversation_id = std::to_string(index % 100);
		mate.sender_id = "sender";
		mate.message_type = index % 10;
		mate.content = line;

		std::string sql = mate.to_sql();
		simply_execute(db, sql.c_str());

		if (index % 100 == 0) {
			std::stringstream tag;
			tag << "insert " << index << " messages";
			printer.Print(tag.str().c_str());
		}
	}
	printer.Print("insert templates success");
	return true;
}

int main() {
#ifdef WIN32
	int codepage = GetACP();
	std::cout << "code page: " << codepage << std::endl;
#endif // WIN32

	// Pointer to SQLite connection
	sqlite3 *db(nullptr);
	// inject static extension
	PrintTime printer;
	int rc = sqlite3_auto_extension((xentry)sqlite3_simple_init);
	handle_rc(db, rc);
	printer.Print("sqlite3_auto_extension");

	// Save the connection result
	// rc = sqlite3_open(":memory:", &db);
	std::string dbpath = get_current_dir() + "/fts5.db";
	//std::string strpth = ParamString2UTF8(dbpath);
	rc = sqlite3_open(dbpath.c_str(), &db);
	handle_rc(db, rc);
	printer.Print("sqlite3_open");

	// load simple
	rc = sqlite3_enable_load_extension(db, 1);
	handle_rc(db, rc);
	printer.Print("sqlite3_enable_load_extension");

	// clear db
	clear_db(db);
	create_table_message(db);
	insert_message(db);

  printer.Print("db prepare over");

	std::string line;
	while (std::getline(std::cin, line)) {
			if(line == "end") {
					break;
			}
			auto pos = line.find("sql:");
			if (pos != std::string::npos) {
					PrintTime printer;
					std::string sql = line.substr(pos+4);
					sql = String2Utf8(sql);
					simply_execute(db, sql.c_str());
					printer.Print("sql");
					continue;
			}
			pos = line.find("search:");
			if (pos != std::string::npos) {
					PrintTime printer;
					std::string sql = line.substr(pos+7);
					std::cout << "search:" << sql << std::endl;
					sql = String2Utf8(sql);
					simply_query(db, sql.c_str(), "result");
					printer.Print("search");
					continue;
			}
			pos = line.find("insert:");
			if (pos != std::string::npos) {
					PrintTime printer;
					std::string content = line.substr(pos+7);
					std::cout << "insert: " << content << std::endl;
					
					message_mate mate;
					mate.create_time = Clock::now().time_since_epoch().count();
					mate.message_id = std::to_string(mate.create_time + 1);
					mate.conversation_id = std::to_string(mate.create_time + 2);
					mate.sender_id = "sender";
					mate.message_type = mate.create_time % 10;
					mate.content = String2Utf8(content);

					std::string sql = mate.to_sql();
					simply_execute(db, sql.c_str());
					printer.Print("insert");
					continue;
			}
	}

	// Close the connection
	sqlite3_close(db);

	return (0);
}

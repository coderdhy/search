#include "gtest/gtest.h"
#include "simple_tokenizer.h"

using namespace simple_tokenizer;

int printFn(void* pCtx, int flags, const char* index, int len, int start, int end) {
	std::cout << "\t" << index << " " << len << " " << start << " " << end << "\n";
	return 0;
}

TEST(simple, tokenizer_with_pinyin) {
	SimpleTokenizer* t = new SimpleTokenizer(nullptr, 0);
	std::vector<std::string> arr;
	std::vector<std::string> query;
	arr.push_back("english@\"''");
	query.push_back(R"VAGON(( e+n+g+l+i+s+h* OR eng+li+sh* OR english* ) AND "@" AND """" AND "'" AND "'")VAGON");
	arr.push_back("zhou����");
	query.push_back(R"VAGON(( z+h+o+u* OR zhou* ) AND "��" AND "��")VAGON");
	arr.push_back("���� zhou 123");
	query.push_back(R"VAGON("��" AND "��" AND ( z+h+o+u* OR zhou* ) AND "123"*)VAGON");
	for (int i = 0; i < arr.size(); i++) {
		std::string s = arr[i];
		std::cout << s << " as doc:\n";
		t->tokenize(nullptr, FTS5_TOKENIZE_DOCUMENT, s.c_str(), s.length(), printFn);
		std::cout << s << " as query:\n";
		t->tokenize(nullptr, FTS5_TOKENIZE_QUERY, s.c_str(), s.length(), printFn);
		std::cout << s << " as aux:\n";
		std::string result = SimpleTokenizer::tokenize_query(s.c_str(), s.length());
		std::cout << result << "\n";
		ASSERT_EQ(result, query[i]);
	}
}

TEST(simple, tokenizer_disable_pinyin) {
	const char* p = "0";
	SimpleTokenizer* t = new SimpleTokenizer(&p, 1);
	std::vector<std::string> arr;
	std::vector<std::string> query;
	arr.push_back("english@\"''");
	query.push_back(R"VAGON(english* AND "@" AND """" AND "'" AND "'")VAGON");
	arr.push_back("zhou����");
	query.push_back(R"VAGON(zhou* AND "��" AND "��")VAGON");
	arr.push_back("����123");
	query.push_back(R"VAGON("��" AND "��" AND "123"*)VAGON");
	for (int i = 0; i < arr.size(); i++) {
		std::string s = arr[i];
		std::cout << s << " as doc:\n";
		t->tokenize(nullptr, FTS5_TOKENIZE_DOCUMENT, s.c_str(), s.length(), printFn);
		std::cout << s << " as query:\n";
		t->tokenize(nullptr, FTS5_TOKENIZE_QUERY, s.c_str(), s.length(), printFn);
		std::cout << s << " as aux:\n";
		std::string result = SimpleTokenizer::tokenize_query(s.c_str(), s.length(), 0);
		std::cout << result << "\n";
		ASSERT_EQ(result, query[i]);
	}
}

#ifdef USE_JIEBA
TEST(simple, jieba_tokenizer_with_pinyin) {
	SimpleTokenizer* t = new SimpleTokenizer(nullptr, 0);
	std::vector<std::string> arr;
	std::vector<std::string> query;
	arr.push_back("english@\"''");
	query.push_back(R"VAGON(( e+n+g+l+i+s+h* OR eng+li+sh* OR english* ) AND "@" AND """" AND "'" AND "'")VAGON");
	arr.push_back("zhou����");
	query.push_back(R"VAGON(( z+h+o+u* OR zhou* ) AND "����")VAGON");
	arr.push_back("���� zhou 123");
	query.push_back(R"VAGON("����" AND ( z+h+o+u* OR zhou* ) AND "123"*)VAGON");
	for (int i = 0; i < arr.size(); i++) {
		std::string s = arr[i];
		std::cout << s << " as doc:\n";
		t->tokenize(nullptr, FTS5_TOKENIZE_DOCUMENT, s.c_str(), s.length(), printFn);
		std::cout << s << " as query:\n";
		t->tokenize(nullptr, FTS5_TOKENIZE_QUERY, s.c_str(), s.length(), printFn);
		std::cout << s << " as aux:\n";
		std::string result = SimpleTokenizer::tokenize_jieba_query(s.c_str(), s.length());
		std::cout << result << "\n";
		ASSERT_EQ(result, query[i]);
	}
}

TEST(simple, jieba_tokenizer_disable_pinyin) {
	const char* p = "0";
	SimpleTokenizer* t = new SimpleTokenizer(&p, 1);
	std::vector<std::string> arr;
	std::vector<std::string> query;
	arr.push_back("english@\"''");
	query.push_back(R"VAGON(english* AND "@" AND """" AND "'" AND "'")VAGON");
	arr.push_back("zhou����");
	query.push_back(R"VAGON(zhou* AND "����")VAGON");
	arr.push_back("����123");
	query.push_back(R"VAGON("����" AND "123"*)VAGON");
	for (int i = 0; i < arr.size(); i++) {
		std::string s = arr[i];
		std::cout << s << " as doc:\n";
		t->tokenize(nullptr, FTS5_TOKENIZE_DOCUMENT, s.c_str(), s.length(), printFn);
		std::cout << s << " as query:\n";
		t->tokenize(nullptr, FTS5_TOKENIZE_QUERY, s.c_str(), s.length(), printFn);
		std::cout << s << " as aux:\n";
		std::string result = SimpleTokenizer::tokenize_jieba_query(s.c_str(), s.length(), 0);
		std::cout << result << "\n";
		ASSERT_EQ(result, query[i]);
	}
}
#endif

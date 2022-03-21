#include "simple_tokenizer.h"

#include <algorithm>
#include <cctype>
#include <iostream>
#include <memory>
#include <sstream>
#include <string>
#include <utility>
#include <vector>

namespace simple_tokenizer {
	SimpleTokenizer::SimpleTokenizer(const char **azArg, int nArg) {
		if (nArg >= 1) {
			enable_simple_pinyin = atoi(azArg[0]) != 0;
		}
	}

	PinYin *SimpleTokenizer::get_pinyin() {
		static auto *py = new PinYin();
		return py;
	}

	static TokenCategory from_char(char c) {
		if (std::isdigit(c)) {
			// 数字
			return TokenCategory::DIGIT;
		}
		if (std::isspace(c) || std::iscntrl(c)) {
			// 空格
			return TokenCategory::SPACE;
		}
		if (std::isalpha(c)) {
			// 字母
			return TokenCategory::ASCII_ALPHABETIC;
		}
		return TokenCategory::OTHER;
	}

	std::string SimpleTokenizer::tokenize_query(const char *text, int textLen, int flags) {
		int start = 0;
		int index = 0;
		std::string tmp;
		std::string result;
		while (index < textLen) {
			TokenCategory category = from_char(text[index]);
			switch (category) {
			case TokenCategory::OTHER:
				index += PinYin::get_str_len(text[index]);
				break;
			default:
				while (++index < textLen && from_char(text[index]) == category) {
				}
				break;
			}
			tmp.clear();
			std::copy(text + start, text + index, std::back_inserter(tmp));
			append_result(result, tmp, category, start, flags);
			start = index;
		}
		return result;
	}


	void SimpleTokenizer::append_result(std::string &result, std::string part, TokenCategory category, int offset, int flags) {
		if (category != TokenCategory::SPACE) {
			std::string tmp = std::move(part);
			if (category == TokenCategory::ASCII_ALPHABETIC) {
				std::transform(tmp.begin(), tmp.end(), tmp.begin(),
					[](unsigned char c) { return std::tolower(c); });
			}

			if (flags != 0 && category == TokenCategory::ASCII_ALPHABETIC && tmp.size() > 1) {
				if (offset == 0) {
					result.append("( ");
				}
				else {
					result.append(" AND ( ");
				}
				std::set<std::string> pys = SimpleTokenizer::get_pinyin()->split_pinyin(tmp);
				bool addOr = false;
				for (const std::string &s : pys) {
					if (addOr) {
						result.append(" OR ");
					}
					result.append(s);
					result.append("*");
					addOr = true;
				}
				result.append(" )");
			}
			else {
				if (offset > 0) {
					result.append(" AND ");
				}
				if (tmp == "\"") {
					tmp += tmp;
				}
				if (category != TokenCategory::ASCII_ALPHABETIC) {
					result.append('"' + tmp + '"');
				}
				else {
					result.append(tmp);
				}
				if (category != TokenCategory::OTHER) {
					result.append("*");
				}
			}
		}
	}


	void split(std::string s, std::set<std::string> &v, std::string delimiter) {
		size_t pos = 0;
		while ((pos = s.find(delimiter)) != std::string::npos) {
			std::string token = s.substr(0, pos);
			v.insert(token);
			s.erase(0, pos + delimiter.length());
		}
		v.insert(s);

	}


	// https://cloud.tencent.com/developer/article/1198371
	int SimpleTokenizer::tokenize(void *pCtx, int flags, const char *text, int textLen,
		xTokenFn xToken) const {
		int rc = SQLITE_OK;
		int start = 0;
		int index = 0;
		std::string result;
		while (index < textLen) {
			TokenCategory category = from_char(text[index]);
			switch (category) {
			case TokenCategory::OTHER:
				index += PinYin::get_str_len(text[index]);
				break;
			default:
				while (++index < textLen && from_char(text[index]) == category) {
				}
				break;
			}
			if (category != TokenCategory::SPACE) {
				result.clear();
				std::copy(text + start, text + index, std::back_inserter(result));
				if (category == TokenCategory::ASCII_ALPHABETIC) {
					std::transform(result.begin(), result.end(), result.begin(),
						[](unsigned char c) { return std::tolower(c); });
				}

				rc = xToken(pCtx, 0, result.c_str(), (int)result.length(), start, index);
				if (category == TokenCategory::OTHER && (flags & FTS5_TOKENIZE_DOCUMENT)) {
					const std::string py = SimpleTokenizer::get_pinyin()->get_pinyin_str(result);
					std::set<std::string> pinyi;
					split(py, pinyi, ",");
					std::set<std::string> pinyiFirst;
					if (enable_simple_pinyin) {
						// 取出拼音首字母
						for (const std::string &s : pinyi) {
							if (s.length() > 1) {
								pinyiFirst.insert(s.substr(0, 1));
							}
						}
						// 加入拼音首字母
						for (const std::string &s : pinyiFirst) {
							pinyi.insert(s);
						}
					}
					for (const std::string &s : pinyi) {
						rc = xToken(pCtx, FTS5_TOKEN_COLOCATED, s.c_str(), (int)s.length(), start,
							index);
					}
				}
			}
			start = index;
		}
		return rc;
	}

}  // namespace simple_tokenizer

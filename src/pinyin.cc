#include "pinyin.h"
#include "pinyin_const.h"

#include <map>
#include <regex>
#include <set>
#include <sstream>
#include <stdexcept>
#include <vector>


namespace simple_tokenizer {

    PinYin::PinYin() {}

    // Get UTF8 character encoding length(via first byte)
    int PinYin::get_str_len(unsigned char byte) {
        if (byte >= 0xF0)
            return 4;
        else if (byte >= 0xE0)
            return 3;
        else if (byte >= 0xC0)
            return 2;
        return 1;
    }

    // get the first valid utf8 string's code point
    // 中文转换成 UTF8 code int
    int PinYin::codepoint(const std::string &u) {
        size_t l = u.length();
        if (l < 1) return -1;
        size_t len = get_str_len((unsigned char) u[0]);
        if (l < len) return -1;
        switch (len) {
            case 1:
                return (unsigned char) u[0];
            case 2:
                return ((unsigned char) u[0] - 192) * 64 + ((unsigned char) u[1] - 128);
            case 3:  // most Chinese char in here
                return ((unsigned char) u[0] - 224) * 4096 + ((unsigned char) u[1] - 128) * 64 +
                       ((unsigned char) u[2] - 128);
            case 4:
                return ((unsigned char) u[0] - 240) * 262144 + ((unsigned char) u[1] - 128) * 4096 +
                       ((unsigned char) u[2] - 128) * 64 + ((unsigned char) u[3] - 128);
            default:
                throw std::runtime_error("should never happen");
        }
    }

    // 中文转拼音字符串
    const std::string PinYin::get_pinyin_str(const std::string &chinese) {
        int point = codepoint(chinese);
        auto s = pinyin_map.find(point);
        if (s != pinyin_map.end()) {
            return s->second;
        } else {
            return "";
        }
    }

    std::vector<std::string> PinYin::_split_pinyin(const std::string &input, int begin, int end) {
        if (begin >= end) {
            return empty_vector;
        }
        if (begin == end - 1) {
            return {input.substr(begin, end - begin)};
        }
        std::vector<std::string> result;
        std::string full = input.substr(begin, end - begin);
        if (pinyin_prefix.find(full) != pinyin_prefix.end() ||
            pinyin_valid.find(full) != pinyin_valid.end()) {
            result.push_back(full);
        }
        int start = begin + 1;
        while (start < end) {
            std::string first = input.substr(begin, start - begin);
            if (pinyin_valid.find(first) == pinyin_valid.end()) {
                ++start;
                continue;
            }
            std::vector<std::string> tmp = _split_pinyin(input, start, end);
            for (const auto &s : tmp) {
                result.push_back(first + "+" + s);
            }
            ++start;
        }
        return result;
    }

    std::set<std::string> PinYin::split_pinyin(const std::string &input) {
        int slen = (int) input.size();
        const int max_length = 20;
        if (slen > max_length || slen <= 1) {
            return {input};
        }

        std::string spacedInput;
        for (auto c : input) {
            spacedInput.push_back('+');
            spacedInput.push_back(c);
        }
        spacedInput = spacedInput.substr(1, spacedInput.size());

        if (slen > 2) {
            std::vector<std::string> tmp = _split_pinyin(input, 0, slen);
            std::set<std::string> s(tmp.begin(), tmp.end());
            s.insert(spacedInput);
            s.insert(input);
            return s;
        }
        return {input, spacedInput};
    }

}  // namespace simple_tokenizer

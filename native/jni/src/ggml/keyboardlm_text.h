// Текстовые утилиты для языковой модели клавиатуры: UTF-8, регистр (ASCII + кириллица),
// карта «буква -> id токена <CHAR_x>» из словаря модели. Header-only, без зависимостей от JNI.
#pragma once

#include <string>
#include <unordered_map>
#include <vector>

namespace kblm {

inline std::vector<int> utf8_codepoints(const std::string &s) {
    std::vector<int> out;
    size_t i = 0, n = s.size();
    while (i < n) {
        unsigned char c = (unsigned char)s[i];
        int cp, len;
        if (c < 0x80) { cp = c; len = 1; }
        else if ((c & 0xE0) == 0xC0) { cp = c & 0x1F; len = 2; }
        else if ((c & 0xF0) == 0xE0) { cp = c & 0x0F; len = 3; }
        else if ((c & 0xF8) == 0xF0) { cp = c & 0x07; len = 4; }
        else { i++; continue; }  // некорректный ведущий байт
        if (i + len > n) break;
        bool ok = true;
        for (int k = 1; k < len; k++) {
            unsigned char cc = (unsigned char)s[i + k];
            if ((cc & 0xC0) != 0x80) { ok = false; break; }
            cp = (cp << 6) | (cc & 0x3F);
        }
        if (!ok) { i++; continue; }
        out.push_back(cp);
        i += len;
    }
    return out;
}

inline std::string utf8_encode(int cp) {
    std::string s;
    if (cp < 0x80) s += (char)cp;
    else if (cp < 0x800) { s += (char)(0xC0 | (cp >> 6)); s += (char)(0x80 | (cp & 0x3F)); }
    else if (cp < 0x10000) { s += (char)(0xE0 | (cp >> 12)); s += (char)(0x80 | ((cp >> 6) & 0x3F)); s += (char)(0x80 | (cp & 0x3F)); }
    else { s += (char)(0xF0 | (cp >> 18)); s += (char)(0x80 | ((cp >> 12) & 0x3F)); s += (char)(0x80 | ((cp >> 6) & 0x3F)); s += (char)(0x80 | (cp & 0x3F)); }
    return s;
}

inline bool is_upper(int cp) {
    return (cp >= 'A' && cp <= 'Z') || (cp >= 0x0410 && cp <= 0x042F) || cp == 0x0401;
}
inline bool is_lower(int cp) {
    return (cp >= 'a' && cp <= 'z') || (cp >= 0x0430 && cp <= 0x044F) || cp == 0x0451;
}
inline int to_lower(int cp) {
    if (cp >= 'A' && cp <= 'Z') return cp + 32;
    if (cp >= 0x0410 && cp <= 0x042F) return cp + 0x20;
    if (cp == 0x0401) return 0x0451;
    return cp;
}

inline bool first_is_lower(const std::string &s) {
    std::vector<int> cps = utf8_codepoints(s);
    return !cps.empty() && is_lower(cps[0]);
}
inline bool has_lower(const std::string &s) {
    for (int cp : utf8_codepoints(s)) if (is_lower(cp)) return true;
    return false;
}
inline bool all_upper(const std::vector<int> &cps) {
    if (cps.empty()) return false;
    for (int cp : cps) if (!is_upper(cp)) return false;
    return true;
}

inline std::string normalize_for_match(const std::string &s) {
    std::string out;
    for (int cp : utf8_codepoints(s)) {
        if (cp == '\'' || cp == '-' || cp == ' ') continue;
        out += utf8_encode(to_lower(cp));
    }
    return out;
}

struct LetterTokenMap {
    std::unordered_map<int, int> ids;  // to_lower(код-поинт) -> id токена

    int find(int cp) const {
        auto it = ids.find(to_lower(cp));
        return it == ids.end() ? -1 : it->second;
    }
    bool empty() const { return ids.empty(); }

    template <class F>
    static LetterTokenMap from_vocab(int n_vocab, F piece_of) {
        LetterTokenMap m;
        const std::string prefix = "<CHAR_";
        for (int i = 0; i < n_vocab; i++) {
            std::string p = piece_of(i);
            if (p.size() <= prefix.size() + 1 || p.compare(0, prefix.size(), prefix) != 0 || p.back() != '>') continue;
            std::vector<int> cps = utf8_codepoints(p.substr(prefix.size(), p.size() - prefix.size() - 1));
            if (cps.size() != 1) continue;
            m.ids[to_lower(cps[0])] = i;
        }
        return m;
    }
};

}  // namespace kblm

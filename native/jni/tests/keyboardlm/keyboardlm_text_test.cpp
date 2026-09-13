// Host-тест модуля keyboardlm_text.h без gtest: сборка и запуск через run.sh
#include "keyboardlm_text.h"
#include <cassert>
#include <cstdio>
#include <string>
#include <vector>

#define CHECK(cond) do { if (!(cond)) { fprintf(stderr, "FAIL %s:%d: %s\n", __FILE__, __LINE__, #cond); return 1; } } while (0)

int main() {
    using namespace kblm;
    // UTF-8
    std::vector<int> cps = utf8_codepoints("Привет");
    CHECK(cps.size() == 6 && cps[0] == 0x041F && cps[5] == 0x0442);
    CHECK(utf8_encode(0x0451) == "ё" && utf8_encode('a') == "a");
    CHECK(utf8_codepoints("a\xFFб").size() == 2);  // битый байт пропущен
    // регистр
    CHECK(to_lower(0x0416) == 0x0436 && to_lower(0x0401) == 0x0451 && to_lower('Z') == 'z' && to_lower('1') == '1');
    CHECK(is_upper(0x0401) && is_lower(0x0451) && !is_upper('1') && !is_lower('1'));
    CHECK(first_is_lower("привет") && !first_is_lower("Привет") && !first_is_lower(""));
    CHECK(has_lower("ПРИВЕт") && !has_lower("ПРИВЕТ") && !has_lower("123"));
    CHECK(all_upper(utf8_codepoints("ПРИВЕТ")) && !all_upper(utf8_codepoints("ПРИВЕт")) && !all_upper(utf8_codepoints("П1")));
    // нормализация для точного совпадения
    CHECK(normalize_for_match("При-вет'") == "привет");
    CHECK(normalize_for_match("Hello World") == "helloworld");
    // карта букв
    std::vector<std::string> vocab = {"<pad>", "<CHAR_A>", "<CHAR_а>", "<CHAR_ё>", "x", "<CHAR_AB>", "<XBU>"};
    LetterTokenMap m = LetterTokenMap::from_vocab((int)vocab.size(), [&](int i) { return vocab[i]; });
    CHECK(m.find('a') == 1 && m.find('A') == 1);
    CHECK(m.find(0x0430) == 2 && m.find(0x0410) == 2);
    CHECK(m.find(0x0451) == 3 && m.find(0x0401) == 3);
    CHECK(m.find('1') == -1 && m.find('x') == -1 && m.ids.size() == 3);
    CHECK(!m.empty());
    printf("keyboardlm_text_test: all checks passed\n");
    return 0;
}

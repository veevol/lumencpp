#ifndef LUMENCPP_LEXER_H
#define LUMENCPP_LEXER_H

#include <cctype>
#include <string>
#include <string_view>
#include <vector>

#include "exceptions.h"
#include "token.h"

namespace lumen {

class Lexer {
public:
    [[nodiscard]] std::vector<Token>
    lex(std::string_view source, std::string filename);

private:
    [[nodiscard]] char at() const noexcept { return *m_at; }
    [[nodiscard]] bool at_end() const noexcept { return m_at == m_end; }

    char eat() noexcept {
        if (at() == '\n') {
            ++m_position.line;
            m_position.column = 1;
        } else {
            ++m_position.column;
        }

        return *(m_at++);
    }

    void skip_whitespaces() noexcept {
        while (!at_end() && std::isspace(at()) && at() != '\n') {
            eat();
            m_can_parse_long_token = true;
        }
    }

    void skip_comment() noexcept {
        while (!at_end() && at() != '\n') {
            eat();
        }
    }

    void skip_useless() noexcept {
        skip_whitespaces();

        while (!at_end() && at() == '#') {
            skip_comment();
            skip_whitespaces();
        }
    }

    [[nodiscard]] std::string get_integer(auto is_digit) {
        std::string result;

        while (!at_end() && (is_digit(at()) || at() == '_')) {
            if (at() == '_') {
                eat();
                continue;
            }

            result += eat();
        }

        if (result.empty()) {
            Position end_position{m_position.line, m_position.column + 1};

            throw ParseError{
                "expected a digit", m_filename, {m_position, end_position}};
        }

        return result;
    }

    [[nodiscard]] std::string get_integer() {
        return get_integer(
            [](char character) { return std::isdigit(character); });
    }

    [[nodiscard]] Token get_identifier() noexcept;
    [[nodiscard]] Token get_number();
    [[nodiscard]] Token get_string();

    [[nodiscard]] Token get_token();

    std::string_view::iterator m_at{};
    std::string_view::iterator m_end{};

    std::string m_filename;
    Position m_position{};

    bool m_can_parse_long_token = false;
};

} // namespace lumen

#endif

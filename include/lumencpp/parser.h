#ifndef LUMENCPP_PARSER_H
#define LUMENCPP_PARSER_H

#include <array>
#include <sstream>
#include <string>
#include <vector>

#include "exceptions.h"
#include "token.h"
#include "value.h"

namespace lumen {

class Parser {
public:
    [[nodiscard]] Object parse(
        const std::vector<Token>& tokens, std::string filename,
        Object predefined = {});

private:
    [[nodiscard]] auto at() const noexcept { return *m_at; }

    [[nodiscard]] bool at_end() const noexcept {
        return at().type == Token::Type::Eof;
    }

    auto eat() noexcept { return *(m_at++); }

    template <Token::Type First, Token::Type... Expected> auto expect() {
        auto result = eat();

        if (result.type != First && ((result.type != Expected) && ...)) {
            auto message = std::string{"unexpected "} + to_string(result.type) +
                           "; expected " + to_string(First, true);

            if constexpr (sizeof...(Expected) > 0) {
                std::array expected_array{Expected...};

                for (std::size_t i = 0; i < expected_array.size(); ++i) {
                    if (i == expected_array.size() - 1) {
                        message += " or ";
                    } else {
                        message += ", ";
                    }

                    message += to_string(expected_array[i], true);
                }
            }

            throw ParseError{message, m_filename, result.source};
        }

        return result;
    }

    void skip_line_breaks() noexcept {
        while (!at_end() && at().type == Token::Type::LineBreak) {
            eat();
        }
    }

    [[nodiscard]] Value& parse_key_path(
        Object& parent, const Token& token, bool create_if_not_exist = true);

    [[nodiscard]] Value&
    parse_key_path(Object& parent, bool create_if_not_exist = true) {
        return parse_key_path(
            parent, expect<Token::Type::Identifier>(), create_if_not_exist);
    }

    [[nodiscard]] auto get_token_lexeme(const Token& token) {
        if (!token.lexeme.has_value()) {
            throw ParseError{
                std::string{to_string(token.type, true)} +
                    " token must have a value",
                std::move(m_filename), token.source};
        }

        return *token.lexeme;
    }

    template <typename ValueType, typename... Manipulators>
    [[nodiscard]] auto
    from_string(std::string value, Manipulators... manipulators) const {
        ValueType result;

        std::istringstream iss{std::move(value)};
        (manipulators(iss), ...);

        iss >> result;
        return result;
    }

    [[nodiscard]] Array parse_array();
    [[nodiscard]] Object parse_object();
    [[nodiscard]] Value parse_integer(const Token& token);

    [[nodiscard]] Value parse_value();
    void parse_assignment(Object& parent);

    [[nodiscard]] static constexpr const char*
    to_string(Token::Type type, bool add_an_article = false) noexcept {
        switch (type) {
        case Token::Type::Equal:
            return "'='";
        case Token::Type::Semicolon:
            return "';'";
        case Token::Type::Comma:
            return "','";
        case Token::Type::Dot:
            return "'.'";
        case Token::Type::LeftBracket:
            return "'['";
        case Token::Type::RightBracket:
            return "']'";
        case Token::Type::LeftBrace:
            return "'{'";
        case Token::Type::RightBrace:
            return "'}'";
        case Token::Type::Identifier:
            if (add_an_article) {
                return "a key";
            }

            return "key";
        case Token::Type::Integer:
            if (add_an_article) {
                return "an integer";
            }

            return "integer";
        case Token::Type::Boolean:
            if (add_an_article) {
                return "a boolean";
            }

            return "boolean";
        case Token::Type::Float:
            if (add_an_article) {
                return "a float";
            }

            return "float";
        case Token::Type::String:
            if (add_an_article) {
                return "a string";
            }

            return "string";
        case Token::Type::LineBreak:
            if (add_an_article) {
                return "an end of line";
            }

            return "end of line";
        case Token::Type::Eof:
            if (add_an_article) {
                return "an end of file";
            }

            return "end of file";
        }

        return "";
    }

    Object m_data;

    std::string m_filename;

    std::vector<Token>::const_iterator m_at;
};

} // namespace lumen

#endif

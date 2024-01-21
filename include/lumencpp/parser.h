#ifndef LUMENCPP_PARSER_H
#define LUMENCPP_PARSER_H

#include <array>
#include <stdexcept>
#include <string>
#include <type_traits>
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

    template <std::same_as<UInt>>
    [[nodiscard]] UInt
    from_string(SourceRegion source, const std::string& value) {
        try {
            if (value.starts_with("0x")) {
                constexpr auto base = 16;
                return std::stoull(value, nullptr, base);
            }

            if (value.starts_with("0o")) {
                constexpr auto base = 8;
                return std::stoull(value.substr(2), nullptr, base);
            }

            if (value.starts_with("0b")) {
                constexpr auto base = 2;
                return std::stoull(value.substr(2), nullptr, base);
            }

            constexpr auto base = 10;
            return std::stoull(value, nullptr, base);
        } catch (const std::out_of_range&) {
            throw ParseError{
                "integer '" + value + "' is out of range",
                std::move(m_filename), source};
        }
    }

    template <std::same_as<Int>>
    [[nodiscard]] Int
    from_string(SourceRegion source, const std::string& value) {
        try {
            return std::stoll(value);
        } catch (const std::out_of_range&) {
            throw ParseError{
                "integer '" + value + "' is out of range",
                std::move(m_filename), source};
        }
    }

    template <std::same_as<Float>>
    [[nodiscard]] Float
    from_string(SourceRegion source, const std::string& value) {
        try {
            return std::stod(value);
        } catch (const std::out_of_range&) {
            throw ParseError{
                "float '" + value + "' is out of range", std::move(m_filename),
                source};
        }
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
            return add_an_article ? "a key" : "key";
        case Token::Type::Integer:
            return add_an_article ? "an integer" : "integer";
        case Token::Type::Boolean:
            return add_an_article ? "a boolean" : "boolean";
        case Token::Type::Float:
            return add_an_article ? "a float" : "float";
        case Token::Type::String:
            return add_an_article ? "a string" : "string";
        case Token::Type::LineBreak:
            return add_an_article ? "an end of line" : "end of line";
        case Token::Type::Eof:
            return add_an_article ? "an end of file" : "end of file";
        }

        return "";
    }

    Object m_data;

    std::string m_filename;

    std::vector<Token>::const_iterator m_at;
};

} // namespace lumen

#endif

#ifndef LUMENCPP_TOKEN_H
#define LUMENCPP_TOKEN_H

#include <optional>
#include <string>
#include <utility>

namespace lumen {

struct Token {
    enum struct Type {
        Equal,
        Semicolon,
        Comma,
        Dot,
        LeftBracket,
        RightBracket,
        LeftBrace,
        RightBrace,
        Identifier,
        Integer,
        Boolean,
        Float,
        String,
        LineBreak,
        Eof
    };

    [[nodiscard]] Token(
        Type type, std::optional<std::string> lexeme = std::nullopt)
    : type{type}, lexeme{std::move(lexeme)} {}

    Type type;
    std::optional<std::string> lexeme;
};

} // namespace lumen

#endif

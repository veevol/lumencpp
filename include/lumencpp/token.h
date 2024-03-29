#ifndef LUMENCPP_TOKEN_H
#define LUMENCPP_TOKEN_H

#include <cstdint>
#include <optional>
#include <string>
#include <utility>

#include "exceptions.h"

namespace lumen {

struct Token {
    enum struct Type : std::uint8_t {
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
        SourceRegion source, Type type,
        std::optional<std::string> lexeme = std::nullopt)
    : source{source}, type{type}, lexeme{std::move(lexeme)} {}

    SourceRegion source;

    Type type;
    std::optional<std::string> lexeme;
};

} // namespace lumen

#endif

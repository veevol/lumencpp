#ifndef LUMENCPP_EXCEPTIONS_H
#define LUMENCPP_EXCEPTIONS_H

#include <cstddef>
#include <stdexcept>
#include <string>

namespace lumen {

struct Position {
    std::size_t line;
    std::size_t column;
};

struct SyntaxError : std::runtime_error {
    [[nodiscard]] SyntaxError(const char* message, Position position) noexcept
    : std::runtime_error{message}, position{position} {}

    [[nodiscard]] SyntaxError(
        const std::string& message, Position position) noexcept
    : SyntaxError{message.c_str(), position} {}

    [[nodiscard]] auto pretty() const noexcept {
        return std::to_string(position.line) + ":" +
               std::to_string(position.column) + ": error: " + what();
    }

    Position position;
};

} // namespace lumen

#endif

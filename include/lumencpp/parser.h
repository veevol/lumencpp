#ifndef LUMENCPP_PARSER_H
#define LUMENCPP_PARSER_H

#include <stdexcept>
#include <vector>

#include "exceptions.h"
#include "token.h"
#include "value.h"

namespace lumen {

class Parser {
public:
    [[nodiscard]] Value::Object parse(const std::vector<Token>& tokens);

private:
    [[nodiscard]] auto at() const noexcept { return *m_at; }

    [[nodiscard]] bool at_end() const noexcept {
        return at().type == Token::Type::Eof;
    }

    auto eat() noexcept { return *(m_at++); }

    template <std::same_as<Token::Type>... Expected>
    auto expect(Token::Type first, Expected... expected) {
        auto result = eat();

        if (result.type != first && ((result.type != expected) && ...)) {
            throw SyntaxError{"unexpected token", result.position};
        }

        return result;
    }

    void skip_line_breaks() noexcept {
        while (!at_end() && at().type == Token::Type::LineBreak) {
            eat();
        }
    }

    [[nodiscard]] Value&
    parse_key_path(Value::Object& parent, bool create_if_not_exist = true);

    [[nodiscard]] Value::Array parse_array();
    [[nodiscard]] Value::Object parse_object();

    [[nodiscard]] Value parse_value();
    void parse_assignment(Value::Object& parent);

    Value::Object m_data;

    std::vector<Token>::const_iterator m_at;
};

} // namespace lumen

#endif
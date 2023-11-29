#ifndef LUMENCPP_VALUE_H
#define LUMENCPP_VALUE_H

#include <concepts>
#include <cstdint>
#include <string>
#include <unordered_map>
#include <utility>
#include <variant>
#include <vector>

namespace lumen {

class Value {
public:
    using UInt = std::uint64_t;
    using Int = std::int64_t;
    using Float = double;
    using Bool = bool;
    using String = std::string;
    using Array = std::vector<Value>;
    using Object = std::unordered_map<std::string, Value>;

    enum class Type { Unknown, UInt, Int, Float, Bool, String, Array, Object };

    [[nodiscard]] Value() noexcept : m_value{std::monostate{}} {}

    [[nodiscard]] Value(std::unsigned_integral auto value) noexcept
    : m_value{static_cast<UInt>(value)} {}

    [[nodiscard]] Value(std::signed_integral auto value) noexcept
    : m_value{static_cast<Int>(value)} {}

    [[nodiscard]] Value(std::floating_point auto value) noexcept
    : m_value{static_cast<Float>(value)} {}

    [[nodiscard]] Value(Bool value) noexcept : m_value{value} {}

    [[nodiscard]] Value(String value) noexcept : m_value{std::move(value)} {}

    [[nodiscard]] Value(const char* value) noexcept
    : m_value{std::string{value}} {}

    [[nodiscard]] Value(Array value) noexcept : m_value{std::move(value)} {}

    [[nodiscard]] Value(Object value) noexcept : m_value{std::move(value)} {}

    Value& operator=(auto value) noexcept {
        *this = Value{value};
        return *this;
    }

    [[nodiscard]] auto get_type() const noexcept {
        return static_cast<Type>(m_value.index());
    }

    [[nodiscard]] bool is(Type type) const noexcept {
        return get_type() == type;
    }

    template <typename ValueType> [[nodiscard]] bool is() const noexcept {
        return std::holds_alternative<ValueType>(m_value);
    }

    template <typename ValueType> [[nodiscard]] auto get() const {
        return std::get<ValueType>(m_value);
    }

    template <std::unsigned_integral Unsigned> [[nodiscard]] auto get() const {
        return static_cast<Unsigned>(std::get<UInt>(m_value));
    }

    template <std::signed_integral Signed> [[nodiscard]] auto get() const {
        return static_cast<Signed>(std::get<Int>(m_value));
    }

    template <std::floating_point FloatingPoint>
    [[nodiscard]] auto get() const {
        return static_cast<FloatingPoint>(std::get<Float>(m_value));
    }

    template <typename ValueType> [[nodiscard]] auto& get() {
        return std::get<ValueType>(m_value);
    }

    [[nodiscard]] auto operator[](const Object::key_type& key) const {
        return get<Object>().at(key);
    }

    [[nodiscard]] auto& operator[](const Object::key_type& key) {
        return get<Object>()[key];
    }

    [[nodiscard]] auto operator[](Array::size_type index) const {
        return get<Array>()[index];
    }

    [[nodiscard]] auto& operator[](Array::size_type index) {
        return get<Array>()[index];
    }

    [[nodiscard]] bool operator==(const Value& other) const = default;

private:
    std::variant<std::monostate, UInt, Int, Float, Bool, String, Array, Object>
        m_value;
};

} // namespace lumen

#endif

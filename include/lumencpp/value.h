#ifndef LUMENCPP_VALUE_H
#define LUMENCPP_VALUE_H

#include <concepts>
#include <cstdint>
#include <initializer_list>
#include <map>
#include <string>
#include <type_traits>
#include <unordered_map>
#include <utility>
#include <variant>
#include <vector>

#include "exceptions.h"

namespace lumen {

class Value;

using UInt = std::uint64_t;
using Int = std::int64_t;
using Float = double;
using Bool = bool;
using String = std::string;
using Array = std::vector<Value>;
using Object = std::unordered_map<std::string, Value>;

namespace details {

using ValueType =
    std::variant<std::monostate, UInt, Int, Float, Bool, String, Array, Object>;

template <typename ValueType> struct IsStdVector : std::false_type {};

template <typename... Args>
struct IsStdVector<std::vector<Args...>> : std::true_type {};

template <typename ValueType>
concept StdVector = IsStdVector<ValueType>::value;

template <typename ValueType> struct IsStdMap : std::false_type {};

template <typename... Args>
struct IsStdMap<std::map<Args...>> : std::true_type {};

template <typename ValueType>
concept StdMap = IsStdMap<ValueType>::value;

template <typename ValueType> struct IsStdUnorderedMap : std::false_type {};

template <typename... Args>
struct IsStdUnorderedMap<std::unordered_map<Args...>> : std::true_type {};

template <typename ValueType>
concept StdUnorderedMap = IsStdUnorderedMap<ValueType>::value;

template <typename ValueType, typename Variant>
struct IsStdVariantMember : std::false_type {};

template <typename ValueType, typename... Args>
struct IsStdVariantMember<ValueType, std::variant<Args...>>
: std::disjunction<std::is_same<ValueType, Args>...> {};

template <typename ValueType, typename Variant>
concept StdVariantMember = IsStdVariantMember<ValueType, Variant>::value;

} // namespace details

class Value {
public:
    enum struct Type : std::uint8_t {
        Undefined,
        UInt,
        Int,
        Float,
        Bool,
        String,
        Array,
        Object
    };

    [[nodiscard]] Value() noexcept : m_value{std::monostate{}} {}

    [[nodiscard]] Value(std::unsigned_integral auto value) noexcept
    : m_value{static_cast<UInt>(value)} {}

    [[nodiscard]] Value(std::signed_integral auto value) noexcept
    : m_value{static_cast<Int>(value)} {}

    [[nodiscard]] Value(std::floating_point auto value) noexcept
    : m_value{static_cast<Float>(value)} {}

    [[nodiscard]] Value(Bool value) noexcept : m_value{value} {}

    template <typename StringLike>
        requires std::is_constructible_v<String, StringLike> &&
                 (!std::is_same_v<StringLike, String>) &&
                 (!std::is_same_v<StringLike, std::nullptr_t>) &&
                 (!std::is_trivially_copyable_v<StringLike>)
    [[nodiscard]] Value(StringLike value) noexcept
    : m_value{String{std::move(value)}} {}

    template <typename StringLike>
        requires std::is_constructible_v<String, StringLike> &&
                 (!std::is_same_v<StringLike, String>) &&
                 (!std::is_same_v<StringLike, std::nullptr_t>) &&
                 std::is_trivially_copyable_v<StringLike>
    [[nodiscard]] Value(StringLike value) noexcept : m_value{String{value}} {}

    [[nodiscard]] Value(String value) noexcept : m_value{std::move(value)} {}

    [[nodiscard]] Value(Array value) noexcept : m_value{std::move(value)} {}

    [[nodiscard]] Value(Object value) noexcept : m_value{std::move(value)} {}

    [[nodiscard]] Value(
        std::initializer_list<Object::value_type> value) noexcept
    : m_value{Object{value}} {}

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

    template <details::StdVariantMember<details::ValueType> ValueType>
    [[nodiscard]] auto& get_strict() {
        if (get_type() == Type::Undefined) {
            m_value = ValueType{};
        }

        try {
            return std::get<ValueType>(m_value);
        } catch (...) {
            if (get_type() == Type::Undefined) {
                throw TypeMismatch{"attempted to retrieve an undefined value"};
            }

            throw TypeMismatch{
                "attempted to retrieve a value with an incompatible type"};
        }
    }

    template <details::StdVariantMember<details::ValueType> ValueType>
    [[nodiscard]] const auto& get_strict() const {
        try {
            return std::get<ValueType>(m_value);
        } catch (...) {
            if (get_type() == Type::Undefined) {
                throw TypeMismatch{"attempted to retrieve an undefined value"};
            }

            throw TypeMismatch{
                "attempted to retrieve a value with an incompatible type"};
        }
    }

    template <std::integral Integral>
        requires(!std::is_same_v<Integral, Bool>)
    [[nodiscard]] auto get() const {
        if (is<UInt>()) {
            return static_cast<Integral>(get_impl<UInt>());
        }

        return static_cast<Integral>(get_strict<Int>());
    }

    template <std::same_as<Bool>> [[nodiscard]] auto get() const {
        return get_strict<Bool>();
    }

    template <std::floating_point FloatingPoint>
    [[nodiscard]] auto get() const {
        if (is<UInt>()) {
            return static_cast<FloatingPoint>(get_impl<UInt>());
        }

        if (is<Int>()) {
            return static_cast<FloatingPoint>(get_impl<Int>());
        }

        return static_cast<FloatingPoint>(get_strict<Float>());
    }

    template <std::constructible_from<const char*> StringLike>
        requires(
            !std::is_same_v<StringLike, String> &&
            !std::is_same_v<StringLike, Bool>)
    [[nodiscard]] auto get() const {
        return StringLike{get_strict<String>().c_str()};
    }

    template <std::same_as<String>> [[nodiscard]] const auto& get() const {
        return get_strict<String>();
    }

    template <details::StdVector Vector>
        requires(!std::is_same_v<Vector, Array>)
    [[nodiscard]] auto get() const {
        Vector result;
        result.reserve(get_strict<Array>().size());

        for (const auto& value : get_impl<Array>()) {
            result.push_back(value.get<typename Vector::value_type>());
        }

        return result;
    }

    template <std::same_as<Array>> [[nodiscard]] const auto& get() const {
        return get_strict<Array>();
    }

    template <details::StdMap Map>
        requires(
            std::is_constructible_v<typename Map::key_type, Object::key_type>)
    [[nodiscard]] auto get() const {
        Map result;

        for (const auto& [key, value] : get_strict<Object>()) {
            result[typename Map::key_type{key}] =
                value.template get<typename Map::mapped_type>();
        }

        return result;
    }

    template <details::StdUnorderedMap UnorderedMap>
        requires(
            std::is_constructible_v<
                typename UnorderedMap::key_type, Object::key_type> &&
            !std::is_same_v<UnorderedMap, Object>)
    [[nodiscard]] auto get() const {
        UnorderedMap result;
        result.reserve(get_strict<Object>().size());

        for (const auto& [key, value] : get_impl<Object>()) {
            result[typename UnorderedMap::key_type{key}] =
                value.template get<typename UnorderedMap::mapped_type>();
        }

        return result;
    }

    template <std::same_as<Object>> [[nodiscard]] const auto& get() const {
        return get_strict<Object>();
    }

    template <typename ValueType>
    [[nodiscard]] auto get_or(ValueType value) const noexcept {
        try {
            return get<ValueType>();
        } catch (...) {
            return value;
        }
    }

    [[nodiscard]] const auto& operator[](const Object::key_type& key) const {
        return get_strict<Object>().at(key);
    }

    [[nodiscard]] auto& operator[](const Object::key_type& key) {
        return get_strict<Object>()[key];
    }

    [[nodiscard]] const auto& operator[](Array::size_type index) const {
        return get_strict<Array>()[index];
    }

    [[nodiscard]] auto& operator[](Array::size_type index) {
        return get_strict<Array>()[index];
    }

    template <typename ValueType>
    [[nodiscard]] bool operator==(const ValueType& other) const noexcept {
        try {
            return get<ValueType>() == other;
        } catch (...) {
            return false;
        }
    }

    [[nodiscard]] bool operator!=(const auto& other) const noexcept {
        return !(*this == other);
    }

    [[nodiscard]] bool operator==(const Value& other) const noexcept = default;

private:
    template <typename ValueType> [[nodiscard]] ValueType& get_impl() {
        return std::get<ValueType>(m_value);
    }

    template <typename ValueType>
    [[nodiscard]] const ValueType& get_impl() const {
        return std::get<ValueType>(m_value);
    }

    details::ValueType m_value;
};

} // namespace lumen

#endif

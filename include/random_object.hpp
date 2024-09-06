#ifndef _ICY_CONTAINER_WRAPPER_RANDOM_OBJECT_HPP_
#define _ICY_CONTAINER_WRAPPER_RANDOM_OBJECT_HPP_

#include <cstddef>
#include <cassert>
#include <string>
#include <vector>
#include <type_traits>
#include <algorithm>

namespace icy {

namespace {
constexpr size_t _s_percentage_base = 100ul;
constexpr char _s_characters[] = "0123456789ABCDEFGHIJKLMNOPQRSTUVWXYZabcdefghijklmnopqrstuvwxyz";
}

template <typename _Tp> inline auto bound(const _Tp& _x, const _Tp& _b1, const _Tp& _b2) -> const _Tp& {
    return std::min(std::max(_x, std::min(_b1, _b2)), std::max(_b1, _b2));
}
inline size_t _S_diff_to_0(char _c) {
    if ('0' <= _c && _c <= '9') return _c - '0';
    else if ('A' <= _c && _c <= 'Z') return _c - 'A' + 10;
    else if ('a' <= _c && _c <= 'z') return _c - 'a' + 36;
    else return sizeof(_s_characters);
}
void _S_init_random_seed();

struct random_object_base;
template <typename _Tp, typename _Enable = void> struct random_object;



struct random_object_base {
    random_object_base();
    virtual ~random_object_base() = default;
};

/**
 * @brief partial specialization for integral
 * @tparam _Tp integral type
*/
template <typename _Tp> struct random_object<_Tp, typename std::enable_if<std::is_integral<_Tp>::value>::type> : public random_object_base {
    using obj_type = _Tp;
    using bound_type = _Tp;
    static inline bound_type lb = 0; // lower bound
    static inline bound_type ub = 2; // upper bound
    /**
     * @brief return random integral in [_l, _u)
     */
    auto rand(bound_type _l, bound_type _u) const -> obj_type {
        assert(_l < _u);
        return obj_type(std::rand() % (_u - _l) + _l);
    };
    /**
     * @brief return random integral
     */
    inline auto rand() const -> obj_type {
        return rand(lb, ub);
    };
};
template <typename _Tp> using random_integral_object = random_object<_Tp, typename std::enable_if<std::is_integral<_Tp>::value>::type>;

/**
 * @brief partial specialization for floating point
 * @brief _Tp floating point type
 */
template <typename _Tp> struct random_object<_Tp, typename std::enable_if<std::is_floating_point<_Tp>::value>::type> : public random_object_base {
    using obj_type = _Tp;
    using bound_type = _Tp;
    static inline bound_type lb = 0.0; // lower bound
    static inline bound_type ub = 1.0; // upper bound
    /**
     * @brief return random floating point in [_l, _u)
     */
    auto rand(bound_type _l, bound_type _u) const -> obj_type {
        assert(_l < _u);
        return obj_type(static_cast<obj_type>(std::rand()) / RAND_MAX * (_u - _l) + _l);
    };
    /**
     * @brief return random floating point
     */
    inline auto rand() const -> obj_type {
        return rand(lb, ub);
    };
};
template <typename _Tp> using random_floating_point_object = random_object<_Tp, typename std::enable_if<std::is_floating_point<_Tp>::value>::type>;

/**
 * @brief partial specialization for char
 */
template <> struct random_object<char> : public random_object_base {
    using obj_type = char;
    using bound_type = char;
    static inline bound_type lb = 'a'; // lower bound
    static inline bound_type ub = 'z'; // upper bound
    /**
     * @brief return random char in [_l, _u]
     * @param _l from 0 to 9, from A to Z, from a to z
     * @param _u from 0 to 9, from A to Z, from a to z
     */
    auto rand(bound_type _l, bound_type _u) const -> obj_type {
        assert(_l < _u);
        size_t _low = _S_diff_to_0(_l);
        size_t _up = _S_diff_to_0(_u);
        assert(0 <= _low && _up < sizeof(_s_characters));
        return _s_characters[_low + std::rand() % (_up - _low + 1)];
    };
    /**
     * @brief return random char
     */
    inline auto rand() const -> obj_type {
        return rand(lb, ub);
    };
};

/**
 * @brief partial specialization for std::string
 */
template <> struct random_object<std::string> : public random_object_base {
    using obj_type = std::string;
    using bound_type = std::string::value_type;
    static inline bound_type lb = 'a'; // lower bound
    static inline bound_type ub = 'z'; // upper bound
    static inline size_t llb = 3; // lower length bound
    static inline size_t ulb = 4; // upper length bound
    /**
     * @brief return random string, with elements in [_l, _u] and size in [_ll, _ul)
     * @param _ll lower length bound
     * @param _ul upper length bound
     * @param _l from 0 to 9, from A to Z, from a to z
     * @param _u from 0 to 9, from A to Z, from a to z
     */
    auto rand(size_t _ll, size_t _ul, bound_type _l, bound_type _u) const -> obj_type {
        assert(_l < _u);
        size_t _length = _ulro.rand(_ll, _ul);
        std::string _str(_length, 0);
        for (auto& _c : _str) {
            _c = _cro.rand(_l, _u);
        }
        return _str;
    };
    /**
     * @brief return random string, with size in [_ll, _ul)
     * @param _ll lower length bound
     * @param _ul upper length bound
     */
    inline auto rand(size_t _ll, size_t _ul) const -> obj_type {
        return rand(_ll, _ul, lb, ub);
    };
    /**
     * @brief return random string
     */
    inline auto rand() const -> obj_type {
        return rand(llb, ulb, lb, ub);
    };
private:
    random_object<size_t> _ulro;
    random_object<char> _cro;
};

template <> struct random_object<void> : public random_object_base {
    using object_type = size_t;
    using bound_type = double;
    template <typename... _Bounds> random_object(_Bounds&&... _bs) {
        _density.clear();
        _M_init_density(std::forward<_Bounds>(_bs)...);
    }
    /**
     * @brief return random unsigned for probability distribution given in constructor
     */
    auto rand() const -> object_type {
        if (_density.empty()) return 0;
        return std::lower_bound(_density.cbegin(), _density.cend(), _btro.rand(0, _density.back())) - _density.cbegin();
    }
    template <typename _Iter> void update_density(_Iter _b, _Iter _e, auto _getter = [](_Iter _i) -> bound_type { return *_i; }) {
        _density.clear();
        for (_Iter _i = _b; _i != _e; ++_i) {
            _density.push_back(_getter(_i) + ((_density.empty() ? 0 : _density.back())));
        }
    }
private:
    template <typename... _Bounds> void _M_init_density(bound_type _b, _Bounds&&... _bs) {
        _density.push_back(_b + (_density.empty() ? 0 : _density.back()));
        _M_init_density(std::forward<_Bounds>(_bs)...);
    }
    void _M_init_density() {}
private:
    random_object<bound_type> _btro;
    std::vector<bound_type> _density; // probability density
};


}

#endif // _ICY_CONTAINER_WRAPPER_RANDOM_OBJECT_HPP_
/*
 * ngtcp2
 *
 * Copyright (c) 2017 ngtcp2 contributors
 * Copyright (c) 2015 ngttp2 contributors
 *
 * Permission is hereby granted, free of charge, to any person obtaining
 * a copy of this software and associated documentation files (the
 * "Software"), to deal in the Software without restriction, including
 * without limitation the rights to use, copy, modify, merge, publish,
 * distribute, sublicense, and/or sell copies of the Software, and to
 * permit persons to whom the Software is furnished to do so, subject to
 * the following conditions:
 *
 * The above copyright notice and this permission notice shall be
 * included in all copies or substantial portions of the Software.
 *
 * THE SOFTWARE IS PROVIDED "AS IS", WITHOUT WARRANTY OF ANY KIND,
 * EXPRESS OR IMPLIED, INCLUDING BUT NOT LIMITED TO THE WARRANTIES OF
 * MERCHANTABILITY, FITNESS FOR A PARTICULAR PURPOSE AND
 * NONINFRINGEMENT. IN NO EVENT SHALL THE AUTHORS OR COPYRIGHT HOLDERS BE
 * LIABLE FOR ANY CLAIM, DAMAGES OR OTHER LIABILITY, WHETHER IN AN ACTION
 * OF CONTRACT, TORT OR OTHERWISE, ARISING FROM, OUT OF OR IN CONNECTION
 * WITH THE SOFTWARE OR THE USE OR OTHER DEALINGS IN THE SOFTWARE.
 */
#ifndef TEMPLATE_H
#define TEMPLATE_H

#include <functional>
#include <utility>
#include <type_traits>

// inspired by <http://blog.korfuri.fr/post/go-defer-in-cpp/>, but our
// template can take functions returning other than void.
template <typename F, typename... T> struct Defer {
  Defer(F &&f, T &&... t)
      : f(std::bind(std::forward<F>(f), std::forward<T>(t)...)) {}
  Defer(Defer &&o) noexcept : f(std::move(o.f)) {}
  ~Defer() { f(); }

  using ResultType = typename std::result_of<typename std::decay<F>::type(
      typename std::decay<T>::type...)>::type;
  std::function<ResultType()> f;
};

template <typename F, typename... T> Defer<F, T...> defer(F &&f, T &&... t) {
  return Defer<F, T...>(std::forward<F>(f), std::forward<T>(t)...);
}

// use chained methods to call functions on a target object
template <typename O, typename D, typename C, typename... P> struct Anchor {
  Anchor(D &&d, C &&c, P &&... p)
      : o(std::bind(std::forward<C>(c), std::forward<P>(p)...)()),
        d(std::bind(std::forward<D>(d), std::forward<O>(o))) {}
  Anchor(Anchor &&a)
      : o(std::move(a.o)),
        d(std::move(a.d)) { a.o = O(); a.d = nullptr; }
  ~Anchor() { if (d != nullptr) d(); }

  template <typename F, typename... T>
  Anchor<O, D, C, P...>&& chain(F &&f, T &&... t) {
    std::bind(std::forward<F>(f), std::forward<O>(o), std::forward<T>(t)...)();

    return std::move(*this);
  }

  O o;

  using ResultType = typename std::result_of<typename std::decay<D>::type(
      typename std::decay<O>::type)>::type;
  std::function<ResultType()> d;
};

template <typename D, typename C, typename... P,
          typename O = typename std::result_of<typename std::decay<C>::type(
              typename std::decay<P>::type...)>::type>
Anchor<O, D, C, P...> anchor(D &&d, C &&c, P &&... p) {
  return Anchor<O, D, C, P...>(std::forward<D>(d), std::forward<C>(c),
                               std::forward<P>(p)...);
}

// Functions to resolve array sizes
template <typename T, size_t N> constexpr size_t array_size(T (&)[N]) {
  return N;
}

template <typename T, size_t N> constexpr size_t str_size(T (&)[N]) {
  return N - 1;
}

// User-defined literals for K, M, and G (powers of 1024)
constexpr unsigned long long operator"" _k(unsigned long long k) {
  return k * 1024;
}

constexpr unsigned long long operator"" _m(unsigned long long m) {
  return m * 1024 * 1024;
}

constexpr unsigned long long operator"" _g(unsigned long long g) {
  return g * 1024 * 1024 * 1024;
}

#endif // TEMPLATE_H

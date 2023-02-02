/// @file
/// пример метафункций и функций использующих SFINAE
///
/// для примера используется функция @fn print_ip которая в зависимости
/// от параметра на этапе компиляции инстанцирует подходящий вариант
#include <functional>
#include <iostream>
#include <list>
#include <sstream>
#include <tuple>
#include <vector>

/// константа хранящая размер бит в типе int8_t
const size_t count_bit = sizeof(int8_t) * 8;

using namespace std;

template <typename T, typename... Arg>
struct TupleIsGomogenCopare {
  static const bool value = (... && (std::is_same_v<T, Arg>));
  using type = std::integral_constant<bool, value>;
};

template <typename T>
constexpr bool TupleIsGomogenCheckCondition(T) {
  static_assert(std::is_same_v<T, true_type>, "TupleIsNOTGomogen");
  return T{};
}

template <typename... Arg>
struct TupleIsGomogen {
  static const bool value = TupleIsGomogenCheckCondition(typename TupleIsGomogenCopare<Arg...>::type{});
};

template <typename TupleT, std::size_t... Is>
void printTuple(const TupleT& tp, std::index_sequence<Is...>) {
  std::stringstream result;
  ((result << std::get<Is>(tp) << "."), ...);
  std::string res = result.str();
  res.pop_back();  // удаляет последнюю точку
  std::cout << res << '\n';
}

/// @fn для кортежей
template <typename... Arg, typename Check = std::enable_if_t<TupleIsGomogen<Arg...>::value, void>,
          std::size_t tup_size = sizeof...(Arg)>
void print_ip(const std::tuple<Arg...>& tp) {
  printTuple(tp, std::make_index_sequence<tup_size>{});
}

/// @fn для целочисленных типов
template <typename T, typename Check = std::enable_if_t<std::is_integral_v<T>, void>>
void print_ip(T val) {
  int shift = sizeof(T) * count_bit - count_bit;
  for (bool first = true; shift >= 0; shift -= count_bit, first = false) {
    if (!first) {
      std::cout << ".";
    }
    std::cout << ((val >> shift) & 0xFF);
  }
  std::cout << '\n';
}
/// @fn для контейнеров
template <typename T>
void print_ip(const T& val, typename T::iterator = {}) {
  bool first = true;
  for (const auto& v : val) {
    if (!first) {
      std::cout << ".";
    }
    first = false;
    std::cout << v;
  }
  std::cout << '\n';
}

/// @fn для строк
template <>
void print_ip(const std::string& val, string::iterator) {
  std::cout << val << '\n';
}

int main() {
  print_ip(int8_t{-1});                            // 255
  print_ip(int16_t{0});                            // 0.0
  print_ip(int32_t{2130706433});                   // 127.0.0.1
  print_ip(int64_t{8875824491850138409});          // 123.45.67.89.101.112.131.41
  print_ip(std::string{"Hello, World !"});         // Hello, World!
  print_ip(std::vector<int>{100, 200, 300, 400});  // 100.200.300.400
  print_ip(std::list<short>{400, 300, 200, 100});  // 400.300.200.100
  print_ip(std::make_tuple(123, 456, 789, 0));     // 123.456.789.0
  return 0;
}

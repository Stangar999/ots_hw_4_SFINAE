/// @file
/// пример метафункций и функций использующих SFINAE
///
/// для примера используется функция print_ip которая в зависимости
/// от параметра на этапе компиляции инстанцирует подходящий вариант
#include <functional>
#include <iostream>
#include <iterator>
#include <list>
#include <sstream>
#include <tuple>
#include <vector>

using namespace std;

/// константа хранящая размер бит в типе int8_t
const size_t count_bit = sizeof(int8_t) * 8;

namespace detail {
template <class Container>
void printList(Container list) {
  using Type = typename Container::value_type;
  std::stringstream result;
  std::copy(list.begin(), list.end(), std::ostream_iterator<Type>(result, "."));
  auto str = result.str();
  str.pop_back();
  std::cout << str << std::endl;
}
}  // namespace detail

/// метафункция сравнения типов в кортеже
/// @return true_type or false_type
template <typename T, typename... Arg>
struct TupleIsGomogenCopare {
  static const bool value = (... && (std::is_same_v<T, Arg>));
  using type = std::integral_constant<bool, value>;
};

/// функция выброса исключений если кортеж не гомогенен
/// @throw compile_error
template <typename T>
constexpr bool TupleIsGomogenCheckCondition(T) {
  static_assert(std::is_same_v<T, true_type>, "TupleIsNOTGomogen");
  return T{};
}

/// метафункция проверки на гомогенность кортежа
template <typename... Arg>
struct TupleIsGomogen {
  static const bool value = TupleIsGomogenCheckCondition(typename TupleIsGomogenCopare<Arg...>::type{});
};

/// функция печати кортежа
/// @param [in] сылка на кортеж
template <typename TupleT, std::size_t... Is>
void printTuple(const TupleT& tp, std::index_sequence<Is...>) {
  auto list = {std::get<Is>(tp)...};
  detail::printList(list);
}

/// для кортежей
template <typename... Arg, typename Check = std::enable_if_t<TupleIsGomogen<Arg...>::value, void>,
          std::size_t tup_size = sizeof...(Arg)>
void print_ip(const std::tuple<Arg...>& tp) {
  printTuple(tp, std::make_index_sequence<tup_size>{});
}

/// для целочисленных типов
template <typename T, typename Check = std::enable_if_t<std::is_integral_v<T>, void>>
void print_ip(T val) {
  constexpr size_t bytes_count = sizeof(T);
  T shift = bytes_count * count_bit - count_bit;
  std::array<int, bytes_count> bytes{};
  for (size_t i = 0; i < bytes_count; shift -= count_bit, ++i) {
    const int byte = (val >> shift) & 0xFF;
    bytes[i] = byte;
  }
  detail::printList(bytes);
}
/// для контейнеров
template <typename T,
          std::enable_if_t<std::is_same_v<std::list<typename std::decay_t<T>::value_type>, std::decay_t<T>> ||
                               std::is_same_v<std::vector<typename std::decay_t<T>::value_type>, std::decay_t<T>>,
                           int> = 0>
void print_ip(const T& val) {
  detail::printList(val);
}

// для строк
template <typename T, std::enable_if_t<std::is_same_v<std::string, std::decay_t<T>>, int> = 0>
void print_ip(const T& val) {
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

/**
 * @file ${FILE}
 * @brief 
 * @license ${LICENSE}
 * Copyright (c) 11/22/15, Steven
 * 
 **/
#ifndef RAYTRACER_THREADING_H
#define RAYTRACER_THREADING_H

#include <chrono>

/**
 * @brief simple function timer routine
 * @param FN_T generic function type
 * @param ARGS_T parameter type list
 *
 * @param function function parameter
 * @param args parameter list for param "function"
 *
 * @return duration instance defined by std::chrono::high_resolution_clock
 */

namespace sls {
template<class FN_T, class ...ARGS_T>
auto timeit(FN_T function, ARGS_T ...args) -> decltype(auto)
{
  using namespace std;
  auto t_start = chrono::high_resolution_clock::now();

  function(&args...);

  auto t_end = chrono::high_resolution_clock::now();

  return t_end - t_start;
};

template<typename ITOR_T, typename FN_T>
void for_each_async(ITOR_T first, ITOR_T last, FN_T fn)
{
  //// TODO(Steve) actually perform in parallel
  for (auto &i = first; i != last; ++i) {
    fn(*i);
  }
};
}

#endif //RAYTRACER_THREADING_H

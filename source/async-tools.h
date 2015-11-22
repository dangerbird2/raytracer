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
#include <future>
#include <vector>

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

/**
 * @brief Performs a parallel for_each on a random
 * access itorator
 */
template<typename RA_ITOR, typename UNARY_FN>
void for_each_async(RA_ITOR first,
                    RA_ITOR last,
                    UNARY_FN fn,
                    size_t n_threads = 10)
{

  using namespace std;

  if (n_threads < 2) {
    for_each(first, last, fn);
    return;
  }

  auto len = last - first;

  auto step_size = len / n_threads;
  step_size = step_size > 0 ? step_size : 1;

  auto futures = vector<future<void>>(n_threads);

  auto n = 0;

  for (auto itor = first; itor < last; itor += step_size) {
    auto local_last = last - itor <= step_size ?
                      last :
                      itor + step_size;

    auto thread_fn = [](auto a, auto b, auto _fn) {
      for_each(a, b, _fn);
    };
    futures.push_back(async(launch::async, move(thread_fn), itor, local_last, fn));
    n++;
  }


  for (auto &i: futures) {
    i.wait();
  }
  //cout << n << " threads executed\n";

};

template<typename T_NUM>
decltype(auto) range(T_NUM start, T_NUM end, T_NUM inc = 1)
{
  using namespace std;
  size_t size = (end - start) / inc;
  auto res = vector<T_NUM>();
  res.reserve(size);

  for (auto i = start; i< end; i+=inc) {
    res.push_back(i);
  }

  return res;

}

}

#endif //RAYTRACER_THREADING_H

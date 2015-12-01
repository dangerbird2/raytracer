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
#include <set>

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


template<typename T_NUM>
decltype(auto) range(T_NUM start, T_NUM end, T_NUM inc = 1)
{
  using namespace std;
  size_t size = (end - start) / inc;
  auto res = vector<T_NUM>();
  res.reserve(size);

  for (auto i = start; i < end; i += inc) {
    res.push_back(i);
  }

  return res;

}


struct rt_data {
  size_t i;
  size_t j;
  sls::Ray ray;
  vec4 color;
};

template<typename FN_T>
std::future<std::vector<rt_data>>
raycast_async(FN_T fn, std::vector<rt_data> const &work)
{
  using namespace std;

  auto work_fn = [&fn](vector<rt_data> generator) {
    cout << "\twork unit size " << generator.size() << "\n";


    for (auto &i: generator) {
      i = fn(i);
    }
    return generator;
  };

  return async(launch::async, move(work_fn), work);

}

}

#endif //RAYTRACER_THREADING_H

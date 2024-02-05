#pragma once

#include "../representation/Solution.hpp"

// std::vector<int> insertion_best(const std::vector<Solution> &population,
//                                 const Solution &child,
//                                 const std::vector<std::vector<int>> &distances,
//                                 const std::vector<int> &distances_to_child);

std::vector<int> insertion_distance(const std::vector<Solution> &population,
                                    const Solution &child,
                                    const std::vector<std::vector<int>> &distances,
                                    const std::vector<int> &distances_to_child);

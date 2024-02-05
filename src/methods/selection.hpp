#pragma once

#include "../representation/Solution.hpp"

std::vector<std::pair<int, int>>
selection_random(const std::vector<Solution> &population,
                 const std::vector<std::vector<int>> &distances);

std::vector<std::pair<int, int>>
selection_random_closest(const std::vector<Solution> &population,
                         const std::vector<std::vector<int>> &distances);

std::vector<std::pair<int, int>>
selection_head(const std::vector<Solution> &population,
               const std::vector<std::vector<int>> &distances);

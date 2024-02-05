#pragma once

#include "../representation/Solution.hpp"

typedef void (*crossover_ptr)(const Solution &,
                              const Solution &,
                              Solution &,
                              const int &);

/**
 * @brief Get the crossover function
 *
 * @param crossover enum Crossover
 * @return crossover_ptr function crossover
 */
crossover_ptr get_crossover_fct(const std::string &crossover);

/**
 * @brief Recreate parent1 (does nothing with parent2) in the child
 *
 * @param parent1 first parent (used)
 * @param parent2 second parent (unused)
 * @param child created child
 * @param colors_p1 number of colors to pick in parent1 for one color in parent2 (unused)
 */
void no_crossover(const Solution &parent1,
                  const Solution &parent2,
                  Solution &child,
                  const int &colors_p1);

/**
 * @brief Create a child by alternating colors from parent1 and parent2, using bigger
 * colors first
 *
 * From :
 * Galinier, P., Hao, J.-K., 1999.
 * Hybrid Evolutionary Algorithms for Graph Coloring.
 * Journal of Combinatorial Optimization 3, 379–397.
 * https://doi.org/10.1023/A:1009823419804
 *
 * @param parent1 first parent
 * @param parent2 second parent
 * @param child created child
 * @param colors_p1 number of colors to pick in parent1 for one color in parent2
 */
void gpx(const Solution &parent1,
         const Solution &parent2,
         Solution &child,
         const int &colors_p1);

/**
 * @brief Create a child by alternating colors from parent1 and parent2, using color with
 * highest ratio sum weight / max weight first, inspired by GPX
 *
 * @param parent1 first parent
 * @param parent2 second parent
 * @param child created child
 * @param colors_p1 number of colors to pick in parent1 for one color in parent2
 */
void gpx_sum_weights_over_max_weight(const Solution &parent1,
                                     const Solution &parent2,
                                     Solution &child,
                                     const int &colors_p1);

/**
 * @brief Create a child by alternating colors from parent1 and parent2, using smaller
 * colors first then open new colors to colors uncolored vertices
 * This is supposed to be a bad crossover
 *
 * @param parent1 first parent
 * @param parent2 second parent
 * @param child created child
 * @param colors_p1 number of colors to pick in parent1 for one color in parent2
 */
void bad_x(const Solution &parent1,
           const Solution &parent2,
           Solution &child,
           const int &colors_p1);

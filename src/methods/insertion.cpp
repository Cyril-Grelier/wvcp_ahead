#include "insertion.hpp"

#include "../utils/random_generator.hpp"

// std::vector<int> insertion_best(const std::vector<Solution> &population,
//                                 const Solution &child,
//                                 const std::vector<std::vector<int>> &distances,
//                                 const std::vector<int> &distances_to_child) {
// }

std::vector<int> insertion_distance(const std::vector<Solution> &population,
                                    const Solution &child,
                                    const std::vector<std::vector<int>> &distances,
                                    const std::vector<int> &distances_to_child) {
    auto pop_size = population.size();
    int min_dist_child{Graph::g->nb_vertices};
    // find the min distances between each individuals
    std::vector<int> min_distance(Parameters::p->population_size + 1, 0);
    for (size_t i{0}; i < pop_size; ++i) {
        if (distances_to_child[i] < min_dist_child) {
            min_dist_child = distances_to_child[i];
        }
        // the minimum distance start with the distance to the new child
        int min_dist{distances_to_child[i]};
        // look for lower distance in the current population
        for (size_t j{0}; j < pop_size; ++j) {
            if (i == j) {
                continue;
            }
            const int distance_{distances[i][j]};
            if (distance_ < min_dist) {
                min_dist = distance_;
            }
        }
        min_distance[i] = min_dist;
    }
    min_distance[Parameters::p->population_size] = min_dist_child;

    // compute for the insertion acceptance formula from
    // Z. Lü and J.-K. Hao, “A memetic algorithm for graph coloring,”
    // European Journal of Operational Research, vol. 203, no. 1, pp. 241–250,
    // May 2010, doi: 10.1016/j.ejor.2009.07.016.
    std::vector<double> score_distance(Parameters::p->population_size + 1, 0);
    for (size_t i{0}; i < pop_size; ++i) {
        score_distance[i] =
            population[i].score_wvcp() +
            std::exp(0.05 * Graph::g->nb_vertices /
                     std::max(min_distance[i], Graph::g->nb_vertices / 50));
        // if the solution is already in the population, give it a bad score
        if (min_distance[i] == 0) {
            score_distance[i] *= 10;
        }
    }
    score_distance[pop_size] =
        child.score_wvcp() +
        std::exp(0.05 * Graph::g->nb_vertices /
                 std::max(min_distance[pop_size], Graph::g->nb_vertices / 50));
    // if the solution is already in the population, give it a bad score
    if (min_distance[pop_size] == 0) {
        score_distance[pop_size] *= 10;
    }

    // get a list of the indices sorted according to the best scores
    std::vector<int> indices(Parameters::p->population_size + 1);
    std::iota(indices.begin(), indices.end(), 0);
    std::stable_sort(
        indices.begin(), indices.end(), [&score_distance](int i_s1, int i_s2) {
            return score_distance[i_s1] < score_distance[i_s2];
        });

    return indices;
}
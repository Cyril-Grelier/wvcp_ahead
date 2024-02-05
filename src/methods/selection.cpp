#include "selection.hpp"

#include "../utils/random_generator.hpp"

std::vector<std::pair<int, int>>
selection_random(const std::vector<Solution> &population,
                 const std::vector<std::vector<int>> &distances) {
    (void)distances;
    std::vector<std::pair<int, int>> selected;
    selected.reserve(Parameters::p->nb_selected);

    // select the fists parents randomly
    std::set<int> firsts_parents;
    std::uniform_int_distribution<int> dist{0, static_cast<int>(population.size()) - 1};

    while (static_cast<int>(firsts_parents.size()) != Parameters::p->nb_selected) {
        firsts_parents.insert(dist(rd::generator));
    }

    // select the second parent as different from the first one
    for (const auto &first_parent : firsts_parents) {
        int second_parent;
        do {
            second_parent = dist(rd::generator);
        } while (first_parent == second_parent);
        selected.push_back(std::make_pair(first_parent, second_parent));
    }
    return selected;
}

std::vector<std::pair<int, int>>
selection_random_closest(const std::vector<Solution> &population,
                         const std::vector<std::vector<int>> &distances) {

    std::vector<std::pair<int, int>> selected;
    selected.reserve(Parameters::p->nb_selected * 2);

    // select the fists parents randomly
    std::set<int> firsts_parents;
    std::uniform_int_distribution<int> dist{0, static_cast<int>(population.size()) - 1};

    while (static_cast<int>(firsts_parents.size()) != Parameters::p->nb_selected) {
        firsts_parents.insert(dist(rd::generator));
    }

    // select the second parent as the closest individual of the first one
    for (const auto &first_parent : firsts_parents) {

        // closer parent selection
        std::vector<int> seconds_parents;
        for (int second_parent{0}; second_parent < Parameters::p->population_size;
             ++second_parent) {
            if (first_parent == second_parent) {
                continue;
            }
            seconds_parents.emplace_back(second_parent);
        }
        // sort the possible seconds parents per distance
        std::stable_sort(seconds_parents.begin(),
                         seconds_parents.end(),
                         [&distances, &first_parent](int i_s1, int i_s2) {
                             return distances[first_parent][i_s1] <
                                    distances[first_parent][i_s2];
                         });
        // pick one of the 3 closest ones
        seconds_parents.resize(3);
        const int second_parent{rd::choice(seconds_parents)};
        selected.push_back(std::make_pair(first_parent, second_parent));
    }
    return selected;
}

std::vector<std::pair<int, int>>
selection_head(const std::vector<Solution> &population,
               const std::vector<std::vector<int>> &distances) {
    (void)population;
    (void)distances;
    return {{0, 1}, {1, 0}};
}

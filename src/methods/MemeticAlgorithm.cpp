#include "MemeticAlgorithm.hpp"

#include <cassert>
#include <cmath>
#include <fstream>
#include <iomanip>
#include <omp.h>
#include <random>
#include <utility>

#include "../utils/random_generator.hpp"
#include "../utils/utils.hpp"
#include "insertion.hpp"
#include "selection.hpp"
#include "tabu_col.hpp"

MemeticAlgorithm::MemeticAlgorithm()
    : _population(Parameters::p->population_size),
      _distances(Parameters::p->population_size,
                 std::vector<int>(Parameters::p->population_size, 0)),
      _t_best{std::chrono::high_resolution_clock::now()} {

    _selected.reserve(Parameters::p->nb_selected);
    _children.reserve(Parameters::p->nb_selected);

    fmt::print(Parameters::p->output, "{}", header_csv());

    // init population
    auto init_fct{get_initialization_fct(Parameters::p->initialization)};
#pragma omp parallel for
    for (int i = 0; i < Parameters::p->population_size; i++) {
        init_fct(_population[i]);
    }

    // set max nb colors for the neural network
    Solution::max_nb_colors = 0;
    for (const auto &individual : _population) {
        const int nb_colors_sol{individual.nb_colors()};
        if (nb_colors_sol > Solution::max_nb_colors) {
            Solution::max_nb_colors = nb_colors_sol;
        }
    }
    Solution::max_nb_colors += static_cast<int>(Solution::max_nb_colors * 0.25);

    // get list of operators
    if (Parameters::p->adaptive != "neural_net_cross") {
        // pairs of <crossover,local search>
        // if "classical" way
        for (const auto &x : Parameters::p->crossover) {
            for (const auto &b : Parameters::p->colors_p1s) {
                for (const auto &ls : Parameters::p->local_search) {
                    _crossover_and_local_search.emplace_back(std::make_tuple(
                        get_crossover_fct(x), stoi(b), get_local_search_fct(ls)));
                }
            }
        }

        // init the adaptive helper
        adaptive_helper =
            get_adaptive_helper(Parameters::p->adaptive,
                                static_cast<int>(Parameters::p->crossover.size() *
                                                 Parameters::p->colors_p1s.size() *
                                                 Parameters::p->local_search.size()));
    } else {
        // local search and crossover un separated list if the neural net select the right
        // crossover separately
        for (const auto &ls : Parameters::p->local_search) {
            _local_search.emplace_back(get_local_search_fct(ls));
        }
        for (const auto &x : Parameters::p->crossover) {
            for (const auto &b : Parameters::p->colors_p1s) {
                _crossover_cb.emplace_back(
                    std::make_tuple(get_crossover_fct(x), stoi(b)));
            }
        }

        // init the adaptive helper
        adaptive_helper =
            get_adaptive_helper(Parameters::p->adaptive,
                                static_cast<int>(Parameters::p->local_search.size()));
    }

    // compute the distances
#pragma omp parallel for
    for (int i = 0; i < Parameters::p->population_size; i++) {
        for (int j = i + 1; j < Parameters::p->population_size; j++) {
            const int dist{distance(_population[i], _population[j])};
            _distances[i][j] = dist;
            _distances[j][i] = dist;
        }
    }

    std::string operators_str = "";
    for (const auto &x : Parameters::p->crossover) {
        for (const auto &b : Parameters::p->colors_p1s) {
            for (const auto &ls : Parameters::p->local_search) {
                operators_str += ":" + x + "_" + b + "_" + ls;
            }
        }
    }
    fmt::print(Parameters::p->output_tbt, "#operators\n");
    fmt::print(Parameters::p->output_tbt, "#{}\n", operators_str);

    fmt::print(Parameters::p->output_tbt,
               "time,turn,distance_pre_cross,proba,selected,"
               "fitness_pre_crossover,fitness_pre_mutation,"
               "fitness_post_mutation,min_fitness,mean_fitness,max_fitness,"
               "age,min_distance,mean_distance,max_distance\n");

    std::stable_sort(_population.begin(),
                     _population.end(),
                     [](const Solution &indiv1, const Solution &indiv2) {
                         return indiv1.score_wvcp() < indiv2.score_wvcp();
                     });
    update_best_score();
}

bool MemeticAlgorithm::stop_condition() const {
    return (_turn < Parameters::p->nb_max_iterations) and
           (not Parameters::p->time_limit_reached()) and
           not(Solution::best_score_wvcp <= Parameters::p->target)
           // to check if insertion added enough individuals
           // (doesn't add individual when distance of 0 to others)
           and static_cast<int>(_population.size()) >= Parameters::p->nb_selected;
}

void MemeticAlgorithm::run() {
    std::string fit_str = "";
    std::string selected_str = "";

    std::vector<Solution> elites;
    if (Parameters::p->population_size == 2) {
        elites = _population;
    }
    int current_elite = 0;
    std::uniform_int_distribution<int> distribution_elite(0, 1);
    int threshold = static_cast<int>(Graph::g->nb_vertices * 0.99);

    do {
        // if the 2 individual for HEAD are equal
        if (Parameters::p->population_size == 2 and _distances[0][1] == 0) {
            auto init_fct{get_initialization_fct(Parameters::p->initialization)};
            // restart the population and elites
            _population = std::vector<Solution>(Parameters::p->population_size);
            elites = std::vector<Solution>(Parameters::p->population_size);
#pragma omp parallel for
            for (int i = 0; i < Parameters::p->population_size; ++i) {
                init_fct(_population[i]);
            }
            // compute the distances
#pragma omp parallel for
            for (int i = 0; i < Parameters::p->population_size; i++) {
                for (int j = i + 1; j < Parameters::p->population_size; j++) {
                    const int dist{distance(_population[i], _population[j])};
                    _distances[i][j] = dist;
                    _distances[j][i] = dist;
                }
            }

            std::stable_sort(_population.begin(),
                             _population.end(),
                             [](const Solution &indiv1, const Solution &indiv2) {
                                 return indiv1.score_wvcp() < indiv2.score_wvcp();
                             });
            update_best_score();
            init_fct(elites[0]);
            init_fct(elites[1]);
            std::stable_sort(_population.begin(),
                             _population.end(),
                             [](const Solution &indiv1, const Solution &indiv2) {
                                 return indiv1.score_wvcp() < indiv2.score_wvcp();
                             });
            update_best_score();
        }
        // Selection of parents
        _selected = selection_random_closest(_population, _distances);

        std::string selected_indiv_str = "";
        for (const auto &parents : _selected) {
            selected_indiv_str +=
                fmt::format("{}:", _distances[parents.first][parents.second]);
        }
        // remove last ":"
        selected_indiv_str.pop_back();

        // Crossover and local search
        if (Parameters::p->adaptive != "neural_net_cross") {
            fit_str = crossover_and_mutation();
            selected_str = adaptive_helper->get_selected_str();
        } else {
            std::tie(fit_str, selected_str) = crossover_and_mutation_neural_network();
        }

        // Insertion
        if (Parameters::p->population_size == 2) {
            // sort children by score
            std::stable_sort(_children.begin(),
                             _children.end(),
                             [](const Solution &indiv1, const Solution &indiv2) {
                                 return indiv1.score_wvcp() < indiv2.score_wvcp();
                             });
            _population[0] = _children[0];
            _population[1] = _children[1];
            _distances[0][1] = distance(_population[0], _population[1]);
            _distances[1][0] = _distances[0][1];
        } else {
            insertion();
        }

        if (_population[0].penalty() <= elites[current_elite].penalty()) {
            elites[current_elite] = _population[0];
        }

        update_best_score();

        turn_by_turn_line(selected_indiv_str, fit_str, selected_str);

        ++_turn;

        for (auto &indiv : _population) {
            ++indiv.age;
        }

        // reintroduce elites in the population (for a HEAD configuration)
        // elites to -1 or 1 mean no elites
        // other values mean the frequency of elites
        if ((Parameters::p->population_size == 2) and (_turn % 10) == 1) {
            // turn elites = 10 by default in HEAD
            current_elite = (current_elite + 1) % 2;
            int indiv_to_replace = distribution_elite(rd::generator);

            auto dist = distance(_population[indiv_to_replace], elites[current_elite]);
            // if the distance is too small, we change the elite
            if (dist > threshold) {
                indiv_to_replace = (indiv_to_replace + 1) % 2;
                dist = distance(_population[indiv_to_replace], elites[current_elite]);
            }
            _population[indiv_to_replace] = elites[current_elite];
            _distances[0][1] = dist;
            _distances[1][0] = dist;
        }

    } while (stop_condition());

    fmt::print(Parameters::p->output, "{}", line_csv());
}

std::string MemeticAlgorithm::crossover_and_mutation() {
    std::string fit_str = "";
    // add score to the output
    for (const auto &parents : _selected) {
        fit_str += fmt::format("{}:", _population[parents.first].score_wvcp());
    }
    fit_str.pop_back();
    fit_str += ",";

    std::vector<int> pair_operators(Parameters::p->nb_selected, -1);

    if (auto *cast = dynamic_cast<AdaptiveHelper_neural_net *>(adaptive_helper.get())) {
        for (int i{0}; i < Parameters::p->nb_selected; ++i) {
            pair_operators[i] = cast->get_operator(i, _population[_selected[i].first]);
        }
    } else {
        for (int i{0}; i < Parameters::p->nb_selected; ++i) {
            pair_operators[i] = adaptive_helper->get_operator();
        }
    }

    _children = std::vector<Solution>(Parameters::p->nb_selected);

    // Crossover
    // #pragma omp parallel for
    for (int i = 0; i < Parameters::p->nb_selected; ++i) {
        const auto &[crossover_fct, colors_p1, local_search_fct]{
            _crossover_and_local_search[pair_operators[i]]};
        crossover_fct(_population[_selected[i].first],
                      _population[_selected[i].second],
                      _children[i],
                      colors_p1);
    }

    for (const auto &child : _children) {
        fit_str += fmt::format("{}:", child.score_wvcp());
    }
    fit_str.pop_back();
    fit_str += ",";

    // Local Search
    // #pragma omp parallel for
    for (int i = 0; i < Parameters::p->nb_selected; i++) {
        const auto &[crossover_fct, colors_p1, local_search_fct]{
            _crossover_and_local_search[pair_operators[i]]};
        if (local_search_fct) {
            local_search_fct(_children[i], false);
            _children[i].reorganize_colors();
        }
        adaptive_helper->update_obtained_solution(
            i, pair_operators[i], _children[i].score_wvcp());
    }

    adaptive_helper->update_helper();

    for (const auto &child : _children) {
        fit_str += fmt::format("{}:", child.score_wvcp());
    }
    fit_str.pop_back();

    return fit_str;
}

std::tuple<std::string, std::string>
MemeticAlgorithm::crossover_and_mutation_neural_network() {
    auto *casted_adaptive =
        dynamic_cast<AdaptiveHelper_neural_net *>(adaptive_helper.get());
    if (not casted_adaptive) {
        fmt::print(stderr, "Error while dynamic cast of AdaptiveHelper_neural_net\n");
        exit(0);
    }

    std::string fit_str = "";
    for (const auto &parents : _selected) {
        fit_str += fmt::format("{}:", _population[parents.first].score_wvcp());
    }
    fit_str.pop_back();
    fit_str += ",";

    // compute all possible crossovers from 2 parents
    const int nb_operator{static_cast<int>(_crossover_cb.size())};

    _children = std::vector<Solution>(Parameters::p->nb_selected);

    std::vector<int> selected_crossover(Parameters::p->nb_selected, -1);

#pragma omp parallel for
    for (int i = 0; i < Parameters::p->nb_selected; ++i) {
        std::vector<Solution> children(nb_operator);
        for (int o{0}; o < nb_operator; ++o) {
            const auto &[crossover_fct, colors_p1]{_crossover_cb[o]};
            crossover_fct(_population[_selected[i].first],
                          _population[_selected[i].second],
                          children[o],
                          colors_p1);
        }
        const int best_child = casted_adaptive->select_best_child(children);
        selected_crossover[i] = best_child;
        _children[i] = children[best_child];
    }

    for (const auto &child : _children) {
        fit_str += fmt::format("{}:", child.score_wvcp());
    }
    fit_str.pop_back();
    fit_str += ",";

    // select the local search
    std::vector<int> selected_local_search(Parameters::p->nb_selected, -1);
    for (int i{0}; i < Parameters::p->nb_selected; ++i) {
        selected_local_search[i] = casted_adaptive->get_operator(i, _children[i]);
    }

    // Local search

#pragma omp parallel for
    for (int i = 0; i < Parameters::p->nb_selected; i++) {
        const auto &local_search_fct = _local_search[selected_local_search[i]];
        if (local_search_fct) {
            local_search_fct(_children[i], false);
            _children[i].reorganize_colors();
        }
        adaptive_helper->update_obtained_solution(
            i, selected_local_search[i], _children[i].score_wvcp());
    }

    adaptive_helper->update_helper();

    for (const auto &child : _children) {
        fit_str += fmt::format("{}:", child.score_wvcp());
    }
    fit_str.pop_back();

    std::string selected_str = "";
    for (int i{0}; i < Parameters::p->nb_selected; ++i) {
        selected_str +=
            fmt::format("{}:",
                        selected_crossover[i] * static_cast<int>(_local_search.size()) +
                            selected_local_search[i]);
    }
    selected_str.pop_back();

    return std::make_pair(fit_str, selected_str);
}

void MemeticAlgorithm::insertion() {
    while (not _children.empty()) {
        const auto child = _children.back();
        _children.pop_back();
        insert(child);
    }
}

void MemeticAlgorithm::insert(const Solution &child) {
    // Compute the distances to the child
    std::vector<int> distances_to_child(Parameters::p->population_size, 0);
    for (size_t i{0}; i < _population.size(); ++i) {
        distances_to_child[i] = distance(child, _population[i]);
    }

    std::vector<int> indices =
        insertion_distance(_population, child, _distances, distances_to_child);

    int to_remove{indices[Parameters::p->population_size]};
    if (to_remove == Parameters::p->population_size + 1) {
        // the worst solution is the child so it should not be added
        // but the child as 20% chance to be accepted anyway
        std::uniform_int_distribution<int> distribution(1, 100);
        if (distribution(rd::generator) > 20) {
            // the child is not inserted
            // the population stay the same
            return;
        }
        // the child is accepted
        // so the second worst solution is removed
        to_remove = indices[Parameters::p->population_size - 1];
    }

    // get the scores to sort the population
    std::vector<int> scores(Parameters::p->population_size + 1);
    for (size_t i{0}; i < _population.size(); ++i) {
        scores[i] = _population[i].score_wvcp();
    }
    scores[Parameters::p->population_size] = child.score_wvcp();
    // sort it
    std::stable_sort(indices.begin(), indices.end(), [&scores](int i_s1, int i_s2) {
        return scores[i_s1] < scores[i_s2];
    });

    std::vector<int> new_pop;
    new_pop.reserve(_population.size());
    for (const auto indiv : indices) {
        if (indiv != to_remove) {
            new_pop.emplace_back(indiv);
        }
    }

    update_population(new_pop, child, distances_to_child);
}

void MemeticAlgorithm::update_population(const std::vector<int> &new_pop,
                                         const Solution &child,
                                         const std::vector<int> &distances_to_child) {
    const auto old_distances = _distances;
    _distances = std::vector<std::vector<int>>(
        Parameters::p->population_size,
        std::vector<int>(Parameters::p->population_size, 0));

    // update the matrix of distance of the population
    for (size_t i{0}; i < _population.size(); ++i) {
        const int indiv1 = new_pop[i];
        for (size_t j{0}; j < _population.size(); ++j) {
            const int indiv2 = new_pop[j];
            if (indiv1 == Parameters::p->population_size and
                indiv2 != Parameters::p->population_size) {
                _distances[i][j] = distances_to_child[indiv2];
            } else if (indiv2 == Parameters::p->population_size and
                       indiv1 != Parameters::p->population_size) {
                _distances[i][j] = distances_to_child[indiv1];
            } else if (indiv1 != Parameters::p->population_size and
                       indiv2 != Parameters::p->population_size) {
                _distances[i][j] = old_distances[indiv1][indiv2];
            }
        }
    }

    // create the population
    std::vector<Solution> population;
    // move the old population
    population.insert(population.begin(),
                      std::make_move_iterator(_population.begin()),
                      std::make_move_iterator(_population.end()));
    _population.clear();
    _population.reserve(Parameters::p->population_size);

    for (const int indiv : new_pop) {
        if (indiv == Parameters::p->population_size) {
            // if its the child
            _population.insert(_population.end(), child);
        } else {
            // if it was already in the population
            _population.insert(_population.end(),
                               std::make_move_iterator(population.begin() + indiv),
                               std::make_move_iterator(population.begin() + indiv + 1));
        }
    }
}

[[nodiscard]] const std::string MemeticAlgorithm::header_csv() const {
    return fmt::format("turn,time,{}\n", Solution::header_csv);
}

[[nodiscard]] const std::string MemeticAlgorithm::line_csv() const {
    return fmt::format("{},{},{}\n",
                       _turn,
                       Parameters::p->elapsed_time(_t_best),
                       _best_solution.line_csv());
}

void MemeticAlgorithm::update_best_score() {
    if (Solution::best_score_wvcp > _population[0].score_wvcp()) {
        _t_best = std::chrono::high_resolution_clock::now();
        Solution::best_score_wvcp = _population[0].score_wvcp();
        _best_solution = _population[0];
        fmt::print(Parameters::p->output, "{}", line_csv());
    }
}

void MemeticAlgorithm::turn_by_turn_line(const std::string &selected_indiv_str,
                                         const std::string &fit_str,
                                         const std::string &selected_str) {
    double mean_fit = 0;
    double mean_dist = 0;
    double min_dist = Graph::g->nb_vertices;
    double max_dist = 0;
    int nb_distances = 0;
    for (size_t indiv{0}; indiv < _population.size(); ++indiv) {
        mean_fit += _population[indiv].score_wvcp();
        for (size_t indiv2{0}; indiv2 < _population.size(); ++indiv2) {
            if (indiv == indiv2) {
                continue;
            }
            const int distance_ = _distances[indiv][indiv2];
            mean_dist += distance_;
            ++nb_distances;
            if (distance_ < min_dist) {
                min_dist = distance_;
            }
            if (distance_ > max_dist) {
                max_dist = distance_;
            }
        }
    }
    mean_dist /= nb_distances;
    mean_fit /= static_cast<double>(_population.size());

    std::string ages{};
    for (const auto &indiv : _population) {
        ages += std::to_string(indiv.age) + ":";
    }
    // delete the last ":"
    ages.pop_back();

    fmt::print(Parameters::p->output_tbt,
               "{},{},{},{},{},{},{},{:.1f},{},{},{},{:.1f},{}\n",
               Parameters::p->elapsed_time(std::chrono::high_resolution_clock::now()),
               _turn,
               selected_indiv_str,
               adaptive_helper->to_str_proba(),
               selected_str,
               fit_str,
               _population[0].score_wvcp(),
               mean_fit,
               _population.back().score_wvcp(),
               ages,
               min_dist,
               mean_dist,
               max_dist);
    adaptive_helper->increment_turn();
    // helper_mutation->increment_turn();
    // helper_crossover->increment_turn();
}

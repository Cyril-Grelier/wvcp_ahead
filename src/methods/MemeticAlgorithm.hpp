#pragma once

#include "../representation/Method.hpp"
#include "../representation/Parameters.hpp"
#include "../representation/Solution.hpp"
#include "../representation/enum_types.hpp"
#include "LocalSearch.hpp"
#include "adaptive.hpp"
#include "crossover.hpp"

/**
 * @brief Method for memetic algorithm
 *
 */
class MemeticAlgorithm : public Method {
  private:
    /** @brief Current population*/
    std::vector<Solution> _population{};
    /** @brief Current population*/
    std::vector<std::vector<int>> _distances{};
    /** @brief Selected solutions*/
    std::vector<std::pair<int, int>> _selected{};
    /** @brief Children solutions*/
    std::vector<Solution> _children{};
    /** @brief Best found solution*/
    Solution _best_solution{};
    /** @brief Time before founding best score*/
    std::chrono::high_resolution_clock::time_point _t_best{};
    /** @brief Current turn of search*/
    long _turn{};
    /** @brief Functions to call as local search*/
    std::vector<local_search_ptr> _local_search{};
    /** @brief Functions to call as crossover*/
    std::vector<std::tuple<crossover_ptr, int>> _crossover_cb{};
    /** @brief Functions of pairs of operators to call*/
    std::vector<std::tuple<crossover_ptr, int, local_search_ptr>>
        _crossover_and_local_search{};

    /** @brief Helper to choose the next pair of operator*/
    std::unique_ptr<AdaptiveHelper> adaptive_helper{};

    // std::vector<std::string> _operators_str;

  public:
    /**
     * @brief Construct a new MemeticAlgorithm object
     *
     */
    explicit MemeticAlgorithm();

    ~MemeticAlgorithm() override = default;

    /**
     * @brief Stopping condition for the MemeticAlgorithm depending on turn or time limit
     *
     * @return true continue the search
     * @return false stop the search
     */
    bool stop_condition() const;

    /**
     * @brief Run the MemeticAlgorithm until stop condition
     *
     */
    void run() override;

    std::string crossover_and_mutation();

    std::tuple<std::string, std::string> crossover_and_mutation_neural_network();

    void insert(const Solution &child);

    /**
     * @brief Insertion of the children solutions in the population
     *
     */
    void insertion();

    void update_population(const std::vector<int> &new_pop,
                           const Solution &child,
                           const std::vector<int> &distances_to_child);

    /**
     * @brief Return string of the MemeticAlgorithm csv format
     *
     * @return std::string header for csv file
     */
    [[nodiscard]] const std::string header_csv() const override;

    /**
     * @brief Return string of a line of the MemeticAlgorithm csv format
     *
     * @return std::string line for csv file
     */
    [[nodiscard]] const std::string line_csv() const override;

    void update_best_score();
    void turn_by_turn_line(const std::string &selected_indiv_str,
                           const std::string &fit_str,
                           const std::string &selected_str);
};

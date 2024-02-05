#include "crossover.hpp"

#include <cmath>

#include "../utils/random_generator.hpp"
#include "../utils/utils.hpp"

crossover_ptr get_crossover_fct(const std::string &crossover) {
    if (crossover == "no_crossover")
        return no_crossover;
    if (crossover == "gpx")
        return gpx;
    if (crossover == "sum_weights_over_max_weight")
        return gpx_sum_weights_over_max_weight;
    if (crossover == "bad_x")
        return bad_x;
    fmt::print(stderr,
               "Unknown crossover, please select : "
               "no_crossover, gpx, sum_weights_over_max_weight, bad_x\n");
    exit(1);
}

void no_crossover(const Solution &parent1,
                  const Solution &parent2,
                  Solution &child,
                  const int &colors_p1) {
    (void)parent2;   // to remove warning "unused parameter"
    (void)colors_p1; // to remove warning "unused parameter"
    for (const auto &color : parent1.non_empty_colors()) {
        int next_color = -1; // to add a new color, color updated in the first turn of for
        for (const auto &vertex : parent1.colors_vertices(color)) {
            next_color = child.add_to_color(vertex, next_color);
        }
    }
}

void gpx(const Solution &parent1,
         const Solution &parent2,
         Solution &child,
         const int &colors_p1) {
    int nb_colors = std::max(parent1.nb_colors(), parent2.nb_colors());

    // nb_vertices_per_color
    auto nb_v_p_c1 = parent1.nb_vertices_per_color(nb_colors);
    auto nb_v_p_c2 = parent2.nb_vertices_per_color(nb_colors);
    for (int color{0}; color < nb_colors; color++) {
        bool turn_p1 = (color % (colors_p1 + 1) < colors_p1);
        const Solution &parent = turn_p1 ? parent1 : parent2;
        const auto &nb_v_p_c = turn_p1 ? nb_v_p_c1 : nb_v_p_c2;
        // index/color of max number of vertex in colors
        const int max_color = static_cast<int>(std::distance(
            nb_v_p_c.begin(), std::max_element(nb_v_p_c.begin(), nb_v_p_c.end())));
        if (nb_v_p_c[max_color] != 0) {
            int next_color = -1; // to add a new color, color updated in the if
            for (const auto &vertex : parent.colors_vertices(max_color)) {
                if (child.color(vertex) == -1) {
                    next_color = child.add_to_color(vertex, next_color);
                    nb_v_p_c1[parent1.color(vertex)]--;
                    nb_v_p_c2[parent2.color(vertex)]--;
                }
            }
        }
    }
    for (int vertex{0}; vertex < Graph::g->nb_vertices; vertex++) {
        if (child.color(vertex) == -1) {
            child.add_to_color(vertex, child.first_available_color(vertex));
        }
    }
}

bool sum_weights_over_max_weight(const std::vector<int> &v1, const std::vector<int> &v2) {
    if (v1.empty())
        return true;
    if (v2.empty())
        return false;
    return static_cast<float>(sum(v1)) / static_cast<float>(v1.back()) <
           static_cast<float>(sum(v2)) / static_cast<float>(v2.back());
}

void gpx_sum_weights_over_max_weight(const Solution &parent1,
                                     const Solution &parent2,
                                     Solution &child,
                                     const int &colors_p1) {
    // child aura au minimum nb_colors couleurs
    const int nb_colors = std::max(parent1.nb_colors(), parent2.nb_colors());

    // copie des classes de poids (liste de taille nb_colors contenant les listes des
    // poids dans chaque couleur)
    auto weights1 = parent1.weights();
    auto weights2 = parent2.weights();
    for (int color{0}; color < nb_colors; color++) {
        bool turn_p1 = (color % (colors_p1 + 1) < colors_p1);
        const auto &parent = turn_p1 ? parent1 : parent2;
        // même formule pour les poids courants
        auto &weights = turn_p1 ? weights1 : weights2;
        // récupération de la couleur avec le meilleur critère sum_weights_over_max_weight
        const int max_color = get_index_max_element(weights, sum_weights_over_max_weight);

        // fmt::print("parent1\n");
        // for (int col{0}; col < parent1.nb_colors(); col++)
        //     fmt::print("\t{} : {} \t{}\n",
        //                col,
        //                fmt::join(weights1[col], ":"),
        //                static_cast<float>(sum(weights1[col])) /
        //                    static_cast<float>(weights1[col].back()));
        // fmt::print("parent2\n");
        // for (int col{0}; col < parent2.nb_colors(); col++)
        //     fmt::print("\t{} : {} \t{}\n",
        //                col,
        //                fmt::join(weights2[col], ":"),
        //                static_cast<float>(sum(weights2[col])) /
        //                    static_cast<float>(weights2[col].back()));
        // if ((color % (cb.first + cb.second) < cb.first)) {
        //     fmt::print("from parent1\n");
        // } else {
        //     fmt::print("from parent2\n");
        // }
        // fmt::print("selected col : {}\n", max_color);

        // si la couleur est vide on ne fait rien (n'arrive qu'à la fin si une solution à
        // plus de couleurs que l'autre)
        if (not weights[max_color].empty()) {
            // création de la prochaine couleur
            int next_color = -1;
            // on vide la copie de la liste des poids dans la couleur pour le prochain
            // calcul
            weights[max_color].clear();
            // pour chaque sommet dans la couleur sélectionnée
            for (const auto &vertex : parent.colors_vertices(max_color)) {
                // si le sommet n'a pas déjà été coloré par l'autre parent
                if (child.color(vertex) == -1) {
                    // colorer le sommet avec la nouvelle couleur
                    next_color = child.add_to_color(vertex, next_color);
                    // suppression du poids du sommet dans la liste des poids du parent
                    // non-courant
                    if (turn_p1) {
                        erase_sorted(weights2[parent2.color(vertex)],
                                     Graph::g->weights[vertex]);
                    } else {
                        erase_sorted(weights1[parent1.color(vertex)],
                                     Graph::g->weights[vertex]);
                    }
                }
            }
        }

        // fmt::print("child\n");
        // for (int col{0}; col < child.nb_colors(); col++)
        //     fmt::print("\t{} : {}\n", col, fmt::join(child.colors_weights()[col],
        //     ":"));
        // int a;
    }
    // on ajoute les sommets non colorés dans la première couleur disponible
    for (int vertex{0}; vertex < Graph::g->nb_vertices; vertex++) {
        if (child.color(vertex) == -1) {
            child.add_to_color(vertex, child.first_available_color(vertex));
        }
    }
    // fmt::print("child\n");
    // for (int col{0}; col < child.nb_colors(); col++)
    //     fmt::print("\t{} : {}\n", col, fmt::join(child.colors_weights()[col], ":"));
}

void bad_x(const Solution &parent1,
           const Solution &parent2,
           Solution &child,
           const int &colors_p1) {
    int nb_colors = std::max(parent1.nb_colors(), parent2.nb_colors());

    // nb_vertices_per_color
    auto nb_v_p_c1 = parent1.nb_vertices_per_color(nb_colors);
    auto nb_v_p_c2 = parent2.nb_vertices_per_color(nb_colors);
    for (int color{0}; color < nb_colors; color++) {
        bool turn_p1 = (color % (colors_p1 + 1) < colors_p1);
        const Solution &parent = turn_p1 ? parent1 : parent2;
        const auto &nb_v_p_c = turn_p1 ? nb_v_p_c1 : nb_v_p_c2;
        // index/color of min number of vertex in colors
        const int min_color = static_cast<int>(std::distance(
            nb_v_p_c.begin(), std::min_element(nb_v_p_c.begin(), nb_v_p_c.end())));
        if (nb_v_p_c[min_color] != 0) {
            int next_color = -1; // to add a new color, color updated in the if
            for (const auto &vertex : parent.colors_vertices(min_color)) {
                if (child.color(vertex) == -1) {
                    next_color = child.add_to_color(vertex, next_color);
                    nb_v_p_c1[parent1.color(vertex)]--;
                    nb_v_p_c2[parent2.color(vertex)]--;
                }
            }
        }
    }
    for (int vertex{0}; vertex < Graph::g->nb_vertices; vertex++) {
        if (child.color(vertex) == -1) {
            child.add_to_color(vertex, -1);
        }
    }
}
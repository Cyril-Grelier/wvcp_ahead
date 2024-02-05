"""
Generate to_eval file which list all run to perform for algo memetic

to split :
    split -l 1000 -d to_eval_am to_eval_am

p,problem
i,instance
m,method
r,rand_seed
T,target
u,use_target
b,objective
t,time_limit
n,nb_max_iterations
I,initialization
N,nb_iter_local_search
M,max_time_local_search
c,coeff_exploi_explo
S,population_size
L,nb_selected
x,crossover
B,balances
A,adaptive
w,window_size
l,local_search
R,return_last_legal
s,simulation
O,O_time
P,P_time
o,output_directory

"""

import os


def get_target(instance: str, problem: str):
    file = f"instances/best_scores_{problem}.txt"
    with open(file, "r", encoding="utf8") as file:
        for line in file.readlines():
            instance_, score, optimal = line[:-1].split(" ")
            if instance_ != instance:
                continue
            if optimal == "*":
                return int(score)
            return 0
    print(f"instance {instance} not found in instances/best_scores_wvcp.txt")


def get_nb_vertices(instance: str):
    file = "instances/instance_info.txt"
    with open(file, "r", encoding="utf8") as file:
        for line in file.readlines():
            instance_, nb_vertices, _ = line[:-1].split(",")
            if instance_ != instance:
                continue
            return int(nb_vertices)
    print(f"instance {instance} not found in instances/instance_info.txt")


problem = "wvcp"  # wvcp gcp

# instance  # i,instance
with open("DIMACS_hard.txt", "r", encoding="UTF8") as file:
    instances = [line[:-1] for line in file.readlines()]


method = "memetic"
rand_seeds = list(range(20))
target = 0
use_target = "false"
objective = "reached"
time_limit = 3600 * 1
nb_max_iterations = 9000000000000000000
initializations = [
    "random",
    # "constrained",
    # "deterministic",
]
nb_iter_local_search = 9000000000000000000
max_time_local_search = -1
coeff_exploi_explos = [
    # "0",
    # "0.3",
    # "0.4",
    # "0.5",
    # "0.6",
    # "0.7",
    # "0.25",
    # "0.5",
    # "0.75",
    "1",
    # "1.25",
    # "1.5",
    # "1.75",
    # "2",
]
population_size = 2
nb_selected = 2
crossovers = [
    # "no_crossover",
    # "sum_weights_over_max_weight",
    # "gpx",
    "gpx",
]
colors_p1s = [
    # "50"
    # "1:3:9",
    "1",
]
adaptives = [
    "none",
    # "iterated",
    # "random",
    # "roulette_wheel",
    # "pursuit",
    # "ucb",
    # "q",
    # "neural_net",
    # "neural_net_cross", # <- this one
]
window_sizes = [
    # 10,
    # 25,
    # 50,
    75,
    # 100,
]
local_searchs = [
    # "none",
    # "hill_climbing",
    "tabu_weight",
    # "tabu_col",
    # "afisa",
    # "afisa_original",
    "redls",
    "ilsts",
    # "redls:ilsts:tabu_col:afisa:tabu_weight",
    # "ilsts:redls:afisa:tabu_weight:tabu_col",
    # "ilsts:redls:useless_ls",
    # "ilsts:redls:afisa_original:tabu_weight",
    # "ilsts:redls:afisa_original:tabu_weight:worst_ls:useless_ls",
    # "ilsts:redls:tabu_col",
    # "redls:ilsts",
]
simulation = "local_search"
O_time = 0
P_times = [
    # 0.01,
    # 0.02,
    0.04,
    # 0.08,
    # 0.1,
    # 0.2,
]
# iter_max = {
#     "C2000.5": 23,
#     "C2000.9": 23,
#     "DSJC1000.1": 44,
#     "DSJC1000.5": 44,
#     "DSJC1000.9": 44,
#     "DSJC500.1": 86,
#     "DSJC500.5": 86,
#     "DSJC500.9": 86,
#     "DSJC250.1": 164,
#     "DSJC250.5": 164,
#     "DSJC250.9": 164,
#     "DSJC125.5gb": 301,
#     "DSJC125.5g": 301,
#     "DSJC125.9gb": 20,
#     "DSJC125.9g": 20,
#     "flat1000_50_0": 44,
#     "flat1000_60_0": 44,
#     "flat1000_76_0": 44,
#     "latin_square_10": 49,
#     "le450_15a": 106,
#     "le450_15b": 101,
#     "le450_15c": 95,
#     "le450_15d": 95,
#     "le450_25a": 129,
#     "le450_25b": 20,
#     "le450_25c": 101,
#     "le450_25d": 101,
#     "queen10_10": 361,
#     "queen10_10gb": 361,
#     "queen10_10g": 20,
#     "queen11_11": 361,
#     "queen11_11gb": 361,
#     "queen11_11g": 361,
#     "queen12_12": 301,
#     "queen12_12gb": 301,
#     "queen12_12g": 301,
#     "queen13_13": 258,
#     "queen14_14": 226,
#     "queen15_15": 181,
#     "queen16_16": 164,
#     "wap01a": 21,
#     "wap02a": 21,
#     "wap03a": 10,
#     "wap04a": 10,
#     "wap05a": 55,
#     "wap06a": 52,
#     "wap07a": 26,
#     "wap08a": 26,
# }

output_directory = "/scratch/LERIA/grelier_c/wvcp_hard_ahead_ls"
# output_directory = (
#     "/home/user/Documents/phd/code/gc_wvcp_algo_gen_mem/wvcp_hard_head_hh"
# )

os.mkdir(output_directory)
for initialization in initializations:
    for coeff_exploi_explo in coeff_exploi_explos:
        for crossover in crossovers:
            for colors_p1 in colors_p1s:
                for adaptive in adaptives:
                    for window_size in window_sizes:
                        for local_search in local_searchs:
                            for P_time in P_times:
                                os.mkdir(f"{output_directory}/{local_search}")
                                os.mkdir(f"{output_directory}/{local_search}/tbt")

with open("to_eval_ls", "w", encoding="UTF8") as file:
    for rand_seed in rand_seeds:
        for instance in instances:
            for initialization in initializations:
                for coeff_exploi_explo in coeff_exploi_explos:
                    for crossover in crossovers:
                        for colors_p1 in colors_p1s:
                            for adaptive in adaptives:
                                for window_size in window_sizes:
                                    for local_search in local_searchs:
                                        for P_time in P_times:
                                            target = get_target(instance, problem)
                                            # nb_max_iterations = iter_max[instance]
                                            # int(
                                            #     3600 / (P_time * get_nb_vertices(instance))
                                            # )
                                            file.write(
                                                f"./gc_wvcp "
                                                f" --problem {problem} "
                                                f" --instance {instance} "
                                                f" --method {method} "
                                                f" --rand_seed {rand_seed} "
                                                f" --target {target} "
                                                f" --use_target {use_target} "
                                                f" --objective {objective} "
                                                f" --time_limit {time_limit} "
                                                f" --nb_max_iterations {nb_max_iterations} "
                                                f" --initialization {initialization} "
                                                f" --nb_iter_local_search {nb_iter_local_search} "
                                                f" --max_time_local_search {max_time_local_search} "
                                                f" --coeff_exploi_explo {coeff_exploi_explo} "
                                                f" --population_size {population_size} "
                                                f" --nb_selected {nb_selected} "
                                                f" --crossover {crossover} "
                                                f" --colors_p1 {colors_p1} "
                                                f" --adaptive {adaptive} "
                                                f" --window_size {window_size} "
                                                f" --local_search {local_search} "
                                                f" --return_last_legal false "
                                                f" --simulation {simulation} "
                                                f" --O_time {O_time} "
                                                f" --P_time {P_time} "
                                                f" --output_directory {output_directory}/{local_search}"
                                                "\n"
                                            )

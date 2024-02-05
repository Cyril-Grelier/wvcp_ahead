"""
Generate to_eval file which list all run to perform for MCTS

to split :
    split -l 1000 -d to_eval_mcts to_eval_mcts

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


problem = "wvcp"  # gcp wvcp

# Choose the set of instances
instances_set = ("pxx", "pxx")
instances_set = ("rxx", "rxx")
instances_set = ("DIMACS_non_optimal", "dimacs_no")
instances_set = ("DIMACS_optimal", "dimacs_o")
instances_set = ("../instances_coeff", "hard_wvcp_coeff")
instances_set = ("instance_list_wvcp", "all")

# i,instance
with open(f"instances/{instances_set[0]}.txt", "r", encoding="utf8") as file:
    instances = [line[:-1] for line in file.readlines()]


method = "mcts"

rand_seeds = list(range(20))

target = 0
use_target = "false"  # false true
objective = "reached"  # optimality reached
time_limit = 3600 * 1
nb_max_iterations = 9000000000000000000
initializations = [
    # "random",
    "constrained",
    # "deterministic",
]
nb_iter_local_search = 9000000000000000000
max_time_local_search = -1
coeff_exploi_explo = [
    # "0",
    # "0.25",
    # "0.5",
    # "0.75",
    "1",
    # "1.25",
    # "1.5",
    # "1.75",
    # "2",
    # "3",
    # "4",
    # "5",
]
adaptives = [
    # "none",
    # "iterated",
    # "random",
    "deleter"
    # "roulette_wheel",
    # "pursuit",
    # "ucb",
    # "neural_net",
    # "q",
]
window_sizes = [
    # 10,
    # 25,
    50,
    # 75,
    # 100,
]
local_searchs = [
    # "none",
    # "tabu_col",
    # "hill_climbing",
    # "afisa_original",
    # "afisa",
    # "tabu_weight",
    # "redls",
    # "ilsts",
    # "redls_freeze",
    "ilsts:redls:tabu_weight:afisa_original"
    # "ilsts:redls:tabu_weight"
]
simulations = [
    # "no_ls",
    "always_ls",
    "fit",
    # "depth",
    # "level",
    # "depth_fit"
]
O_time = 0
P_time = 0.02


output_directory = f"/scratch/LERIA/grelier_c/mcts_deleter_{instances_set[1]}"
# output_directory = f"mcts_ls_coeff_{instances_set[1]}"

os.mkdir(f"{output_directory}/")
for initialization in initializations:
    for coeff in coeff_exploi_explo:
        for local_search in local_searchs:
            for simulation in simulations:
                for adaptive in adaptives:
                    # for P_time in P_times:
                    os.mkdir(f"{output_directory}/{simulation}")
                    os.mkdir(f"{output_directory}/{simulation}/tbt")

with open("to_eval_mcts", "w", encoding="UTF8") as file:
    for initialization in initializations:
        for coeff in coeff_exploi_explo:
            for local_search in local_searchs:
                for adaptive in adaptives:
                    for window_size in window_sizes:
                        for simulation in simulations:
                            for instance in instances:
                                target = get_target(instance, problem)
                                for rand_seed in rand_seeds:
                                    file.write(
                                        f"./gc_wvcp "
                                        f" --problem {problem}"
                                        f" --instance {instance}"
                                        f" --method {method}"
                                        f" --rand_seed {rand_seed}"
                                        f" --target {target}"
                                        f" --use_target {use_target}"
                                        f" --objective {objective}"
                                        f" --time_limit {time_limit}"
                                        f" --nb_max_iterations {nb_max_iterations}"
                                        f" --initialization {initialization}"
                                        f" --nb_iter_local_search {nb_iter_local_search}"
                                        f" --max_time_local_search {max_time_local_search}"
                                        f" --coeff_exploi_explo {coeff}"
                                        f" --population_size 0"
                                        f" --nb_selected 1"
                                        f" --crossover none"
                                        f" --balances 0"
                                        f" --adaptive {adaptive}"
                                        f" --window_size {window_size}"
                                        f" --local_search {local_search}"
                                        f" --return_last_legal false"
                                        f" --simulation {simulation}"
                                        f" --O_time {O_time}"
                                        f" --P_time {P_time}"
                                        f" --output_directory {output_directory}/{simulation}"
                                        "\n"
                                    )


import subprocess
import os
from time import sleep
# check if the build folder exists
def build_prject():
    if not os.path.exists("build"):
        os.makedirs("build")
    else:
        subprocess.call(["rm", "-rf", "build"])
        os.makedirs("build")
    # cd to the build folder and run cmake and make
    subprocess.call(["cmake", ".."], cwd="build")
    subprocess.call(["make"], cwd="build")

# prepare the independent variables stash_size and dataCache_size from 2^1 to 2^9
stash_sizes = []
dataCache_sizes = []
for i in range(1, 10):
    stash_sizes.append(2**i)
    dataCache_sizes.append(2**i)
# define a function to change the value of the variables in the config file
config_path = "src/config.h"
varibles = ["STASH_SIZE", "CACHE_SIZE"]
server_port = 5201
def change_config(config_path, varibles, new_values):
    with open(config_path, "r+") as f:
        #define STASH_SIZE		2  
        #define CACHE_SIZE		2
        # first find the position of above lines and then replace the values
        lines = f.readlines()
        for i in range(len(lines)):
            if "SERVER_PORT" in lines[i]:
                #define SERVER_PORT		5201
                global server_port
                server_port = server_port + 1
                lines[i] = "#define SERVER_PORT\t\t" + str(server_port) + "\n"
        for i in range(len(varibles)):
            for j in range(len(lines)):
                if varibles[i] in lines[j]:
                    lines[j] = "#define " + varibles[i] + "\t\t" + str(new_values[i]) + "\n"
                    break
    with open(config_path, "w") as f:
        f.writelines(lines)

def run_program():
    os.system("killall Jump_ORAM")
    run_client_command = "./Jump_ORAM 2"
    run_server_command = "./Jump_ORAM 1 &"
    flag_server = subprocess.run(run_server_command, cwd="build", shell=True)
    flag = subprocess.run(run_client_command, cwd="build", shell=True)
    print(f"flag_server.returncode = {flag_server.returncode}")
    if flag.returncode == 0:
        os.system("mv build/log/JUMPORAM/stash_size_" + str(stash_size) + "_dataCache_size_" + str(dataCache_size) + ".txt log/")
        os.system("killall Jump_ORAM")
# run the program and get the result
if __name__ == "__main__":
   # prepare the log file to store the running time of Jump_ORAM program with different stash_size and dataCache_size
   subprocess.call(["mkdir", "-p", "log"])
   for i in range(len(stash_sizes)):
       stash_size = stash_sizes[i]
       for j in range(len(dataCache_sizes)):
            dataCache_size = dataCache_sizes[j]
            change_config(config_path, varibles, [stash_size, dataCache_size])
            sleep(1)
            build_prject()
            sleep(1)
            run_program()
import subprocess
from os import system
from time import sleep
import os
from string import ascii_lowercase as alc
import threading
from random import randint

from test_lib import clear_env_vars, clear_tmp_dir, gen_rand_word, syslog_file_path

def prepare_test_dir(path_to_dir: str):
    tmp_dir_name = f"{path_to_dir}/tmp"
    files_amount = 500

    if os.path.isdir(tmp_dir_name):
        system(f"rm -rf {tmp_dir_name}")
    system(f"mkdir {tmp_dir_name}")

    alpha_ptr = 0
    for i in range(files_amount):
        system(f"touch {tmp_dir_name}/{i}.txt")
        with open(f"{tmp_dir_name}/{i}.txt", 'w') as fd:
            fd.write(alc[alpha_ptr]*10000)
        if alpha_ptr == 25:
            alpha_ptr = 0
        else:
            alpha_ptr += 1

proc_terminate_flag = False

def threa_signal_function(proc_pid: int):
    sleep(5)
    while(not proc_terminate_flag):
        os.kill(proc_pid, 10)
        sleep(5)

def thread_change_dir_function(test_dir_path: str):    
    while(not proc_terminate_flag):
        file_number = randint(0, 499)
        symbol_number = randint(1, 100)
        random_str = gen_rand_word(symbol_number)
        with open(f"{test_dir_path}/{file_number}.txt", 'a') as fd:
            fd.write(random_str)
        sleep(1)

def test_stress():
    global proc_terminate_flag

    test_time = 10
    current_file = os.path.realpath(__file__)
    current_directory = os.path.dirname(current_file)

    clear_env_vars()
    prepare_test_dir(current_directory)

    fd = open(syslog_file_path, "r")
    fd.seek(0, 2) # Go to the end of the file
    with subprocess.Popen([f"{current_directory}/build/main", "--p", f"{test_time}", "--d", f"{current_directory}/tmp"]) as proc:
        try:
            signal_thread = threading.Thread(target=threa_signal_function, args=(proc.pid,), daemon=True)
            change_dir_thread = threading.Thread(target=thread_change_dir_function, args=(f"{current_directory}/tmp",), daemon=True)
            
            signal_thread.start()
            change_dir_thread.start()

            proc.wait(600)
        except subprocess.TimeoutExpired:
            proc_terminate_flag = True        
            proc.terminate()
            proc.wait()

    signal_thread.join()
    change_dir_thread.join()
    #TODO: instead of syslog check json log file
    check_result = False
    file_content = fd.read().split("\n")
    for line in file_content:
        if "[inf] demon stop" in line:
            check_result = True
            fd.close()
            break

    clear_tmp_dir(current_directory)

    assert check_result
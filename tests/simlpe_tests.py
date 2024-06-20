import subprocess
import os
from time import sleep

import test_lib

'''
In this file you can find small unit tests for some logic parts of demon code.
'''

def test_check_json_log_creation():
    test_time = 10
    current_file = os.path.realpath(__file__)
    current_directory = os.path.dirname(current_file)

    test_lib.create_tmp_dir(current_directory)

    with subprocess.Popen([f"{current_directory}/../build/main", "--d", f"{current_directory}/tmp", "--p", f"{test_time}", "--t"]) as proc:
        try:
            proc.wait(test_time+ 5)
        except subprocess.TimeoutExpired:
            proc.terminate()
            proc.wait()

    check_result = os.path.exists(test_lib.json_file_path)

    test_lib.clear_json_log(test_lib.json_file_path)
    test_lib.clear_tmp_dir(current_directory)

    assert check_result

def test_integrity_check_err():
    test_time = 10
    current_file = os.path.realpath(__file__)
    current_directory = os.path.dirname(current_file)

    test_lib.create_tmp_dir(current_directory)
    
    fd = open(test_lib.syslog_file_path, "r")
    fd.seek(0, 2) # Go to the end of the file
    with subprocess.Popen([f"{current_directory}/../build/main", "--d", f"{current_directory}/tmp", "--p", f"{test_time}", "--t"]) as proc:
        try:
            test_lib.change_tmp_dir(current_directory)
            proc.wait(test_time + 5)
        except subprocess.TimeoutExpired:
            proc.terminate()
            proc.wait()
    
    check_result = True
    file_content = fd.read().split("\n")
    for line in file_content:
        if "[err]" in line:
            check_result = False
            print(f"ERROR IN THE SYSLOG LINE {line}")
            fd.close()
            break

    json_data = test_lib.parce_json(test_lib.json_file_path)
    check_result = not check_result and test_lib.check_json_status_OK(json_data) and test_lib.check_json_status_FAIL(json_data) and test_lib.check_json_status_NEW(json_data) and test_lib.check_json_status_ABSENT(json_data)

    test_lib.clear_json_log(test_lib.json_file_path)
    test_lib.clear_tmp_dir(current_directory)

    assert check_result

def test_integrity_check_ok():
    test_time = 10
    current_file = os.path.realpath(__file__)
    current_directory = os.path.dirname(current_file)

    test_lib.create_tmp_dir(current_directory)

    fd = open(test_lib.syslog_file_path, "r")
    fd.seek(0, 2) # Go to the end of the file
    with subprocess.Popen([f"{current_directory}/../build/main", "--d", f"{current_directory}/tmp", "--p", f"{test_time}", "--t"]) as proc:
        try:
            proc.wait(test_time+ 5)
        except subprocess.TimeoutExpired:
            proc.terminate()
            proc.wait()

    check_result = True
    file_content = fd.read().split("\n")
    for line in file_content:
        if "[err]" in line:
            check_result = False
            print(f"ERROR IN THE SYSLOG LINE {line}")
            fd.close()
            break

    json_data = test_lib.parce_json(test_lib.json_file_path)
    check_result = check_result and test_lib.check_json_status_OK(json_data) and not test_lib.check_json_status_FAIL(json_data)

    test_lib.clear_json_log(test_lib.json_file_path)
    test_lib.clear_tmp_dir(current_directory)

    assert check_result

def test_no_target_dir():
    test_time = 5
    current_file = os.path.realpath(__file__)
    current_directory = os.path.dirname(current_file)

    test_lib.clear_env_vars()
    test_lib.create_tmp_dir(current_directory)

    fd = open(test_lib.syslog_file_path, "r")
    fd.seek(0, 2) # Go to the end of the file
    with subprocess.Popen([f"{current_directory}/../build/main", "--p", f"{test_time}", "--t"]) as proc:
        try:
            proc.wait(test_time+ 5)
        except subprocess.TimeoutExpired:
            proc.terminate()
            proc.wait()
    
    check_result = False
    file_content = fd.read().split("\n")
    for line in file_content:
        if "[err] path to the directory is no set" in line:
            check_result = True
            fd.close()
            break

    test_lib.clear_tmp_dir(current_directory)
    test_lib.clear_json_log(test_lib.json_file_path)

    assert check_result

def test_negativ_period():
    test_time = 5
    current_file = os.path.realpath(__file__)
    current_directory = os.path.dirname(current_file)

    test_lib.clear_env_vars()
    test_lib.create_tmp_dir(current_directory)

    fd = open(test_lib.syslog_file_path, "r")
    fd.seek(0, 2) # Go to the end of the file
    with subprocess.Popen([f"{current_directory}/../build/main", "--p", "-2", "--t"]) as proc:
        try:
            proc.wait(test_time + 5)
        except subprocess.TimeoutExpired:
            proc.terminate()
            proc.wait()
    
    check_result = False
    file_content = fd.read().split("\n")
    for line in file_content:
        if "[err] periode can't be less or equal 0, but actual periode is -2" in line:
            check_result = True
            fd.close()
            break

    test_lib.clear_tmp_dir(current_directory)
    test_lib.clear_json_log(test_lib.json_file_path)

    assert check_result

def test_check_sum_message():
    test_time = 30
    current_file = os.path.realpath(__file__)
    current_directory = os.path.dirname(current_file)

    # set_env_vars(current_directory, test_time)
    test_lib.create_tmp_dir(current_directory)

    fd = open(test_lib.syslog_file_path, "r")
    fd.seek(0, 2) # Go to the end of the file
    with subprocess.Popen([f"{current_directory}/../build/main", "--d", f"{current_directory}/tmp", "--p", f"{test_time}", "--t"]) as proc:
        try:
            proc.wait(test_time + 5)
        except subprocess.TimeoutExpired:
            proc.terminate()
            proc.wait()
    
    check_result = False
    file_content = fd.read().split("\n")
    for line in file_content:
        if "integrity check: OK" in line:
            check_result = True
            fd.close()
            break

    json_data = test_lib.parce_json(test_lib.json_file_path)
    check_result = check_result and test_lib.check_json_status_OK(json_data)

    test_lib.clear_tmp_dir(current_directory)
    test_lib.clear_json_log(test_lib.json_file_path)
    test_lib.clear_env_vars()

    assert check_result

def test_bad_target_dir():
    test_time = 5
    current_file = os.path.realpath(__file__)
    current_directory = os.path.dirname(current_file)

    test_lib.clear_env_vars()
    test_lib.create_tmp_dir(current_directory)

    fd = open(test_lib.syslog_file_path, "r")
    fd.seek(0, 2) # Go to the end of the file
    with subprocess.Popen([f"{current_directory}/../build/main", "--p", f"{test_time}", "--d", f"{current_directory}/tmp2", "--t"]) as proc:
        try:
            proc.wait(test_time+ 5)
        except subprocess.TimeoutExpired:
            proc.terminate()
            proc.wait()
    
    check_result = False
    file_content = fd.read().split("\n")
    for line in file_content:
        if "[err] target directory doesn't exist" in line:
            check_result = True
            fd.close()
            break

    test_lib.clear_tmp_dir(current_directory)
    test_lib.clear_json_log(test_lib.json_file_path)

    assert check_result

def test_check_demon_exit():
    test_time = 10
    current_file = os.path.realpath(__file__)
    current_directory = os.path.dirname(current_file)

    test_lib.clear_env_vars()
    test_lib.create_tmp_dir(current_directory)

    fd = open(test_lib.syslog_file_path, "r")
    fd.seek(0, 2) # Go to the end of the file
    with subprocess.Popen([f"{current_directory}/../build/main", "--p", f"{test_time}", "--d", f"{current_directory}/tmp", "--t"]) as proc:
        try:
            proc.wait(test_time+ 5)
        except subprocess.TimeoutExpired:
            proc.terminate()
            proc.wait()
    
    check_result = False
    file_content = fd.read().split("\n")
    for line in file_content:
        if "[inf] demon stop" in line:
            check_result = True
            fd.close()
            break

    test_lib.clear_tmp_dir(current_directory)
    test_lib.clear_json_log(test_lib.json_file_path)

    assert check_result

def test_sigusr1_signal():
    test_time = 1000
    current_file = os.path.realpath(__file__)
    current_directory = os.path.dirname(current_file)

    test_lib.clear_env_vars()
    test_lib.create_tmp_dir(current_directory)

    fd = open(test_lib.syslog_file_path, "r")
    fd.seek(0, 2) # Go to the end of the file
    with subprocess.Popen([f"{current_directory}/../build/main", "--p", f"{test_time}", "--d", f"{current_directory}/tmp", "--t"]) as proc:
        try:
            sleep(10)
            proc.send_signal(10)
            proc.wait(10)
        except subprocess.TimeoutExpired:
            proc.terminate()
            proc.wait()
    
    check_result = False
    file_content = fd.read().split("\n")
    for line in file_content:
        if "integrity check: OK" in line:
            check_result = True
            fd.close()
            break

    json_data = test_lib.parce_json(test_lib.json_file_path)
    check_result = check_result and test_lib.check_json_status_OK(json_data) and not test_lib.check_json_status_FAIL(json_data)

    test_lib.clear_tmp_dir(current_directory)
    test_lib.clear_json_log(test_lib.json_file_path)

    assert check_result
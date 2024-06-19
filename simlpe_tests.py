import subprocess
import os
from time import sleep

from test_lib import create_tmp_dir, change_tmp_dir, clear_tmp_dir, clear_env_vars, syslog_file_path

def test_integrity_check_err():
    test_time = 10
    current_file = os.path.realpath(__file__)
    current_directory = os.path.dirname(current_file)

    create_tmp_dir(current_directory)
    
    fd = open(syslog_file_path, "r")
    fd.seek(0, 2) # Go to the end of the file
    with subprocess.Popen([f"{current_directory}/build/main", "--d", f"{current_directory}/tmp", "--p", f"{test_time}"]) as proc:
        try:
            change_tmp_dir(current_directory)
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

    clear_tmp_dir(current_directory)

    assert not check_result

def test_integrity_check_ok():
    test_time = 10
    current_file = os.path.realpath(__file__)
    current_directory = os.path.dirname(current_file)

    create_tmp_dir(current_directory)

    fd = open(syslog_file_path, "r")
    fd.seek(0, 2) # Go to the end of the file
    with subprocess.Popen([f"{current_directory}/build/main", "--d", f"{current_directory}/tmp", "--p", f"{test_time}"]) as proc:
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

    clear_tmp_dir(current_directory)

    assert check_result

def test_no_target_dir():
    test_time = 5
    current_file = os.path.realpath(__file__)
    current_directory = os.path.dirname(current_file)

    clear_env_vars()
    create_tmp_dir(current_directory)

    fd = open(syslog_file_path, "r")
    fd.seek(0, 2) # Go to the end of the file
    with subprocess.Popen([f"{current_directory}/build/main", "--p", f"{test_time}"]) as proc:
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

    clear_tmp_dir(current_directory)

    assert check_result

def test_no_target_dir():
    test_time = 5
    current_file = os.path.realpath(__file__)
    current_directory = os.path.dirname(current_file)

    clear_env_vars()
    create_tmp_dir(current_directory)

    fd = open(syslog_file_path, "r")
    fd.seek(0, 2) # Go to the end of the file
    with subprocess.Popen([f"{current_directory}/build/main", "--p", f"{test_time}"]) as proc:
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

    clear_tmp_dir(current_directory)

    assert check_result

def test_negativ_period():
    test_time = 5
    current_file = os.path.realpath(__file__)
    current_directory = os.path.dirname(current_file)

    clear_env_vars()
    create_tmp_dir(current_directory)

    fd = open(syslog_file_path, "r")
    fd.seek(0, 2) # Go to the end of the file
    with subprocess.Popen([f"{current_directory}/build/main", "--p", "-2"]) as proc:
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

    clear_tmp_dir(current_directory)

    assert check_result

def test_check_sum_message():
    test_time = 30
    current_file = os.path.realpath(__file__)
    current_directory = os.path.dirname(current_file)

    # set_env_vars(current_directory, test_time)
    create_tmp_dir(current_directory)

    fd = open(syslog_file_path, "r")
    fd.seek(0, 2) # Go to the end of the file
    with subprocess.Popen([f"{current_directory}/build/main", "--d", f"{current_directory}/tmp", "--p", f"{test_time}"]) as proc:
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

    clear_tmp_dir(current_directory)
    clear_env_vars()

    assert check_result

def test_bad_target_dir():
    test_time = 5
    current_file = os.path.realpath(__file__)
    current_directory = os.path.dirname(current_file)

    clear_env_vars()
    create_tmp_dir(current_directory)

    fd = open(syslog_file_path, "r")
    fd.seek(0, 2) # Go to the end of the file
    with subprocess.Popen([f"{current_directory}/build/main", "--p", f"{test_time}", "--d", f"{current_directory}/tmp2"]) as proc:
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

    clear_tmp_dir(current_directory)

    assert check_result

def test_check_demon_exit():
    test_time = 10
    current_file = os.path.realpath(__file__)
    current_directory = os.path.dirname(current_file)

    clear_env_vars()
    create_tmp_dir(current_directory)

    fd = open(syslog_file_path, "r")
    fd.seek(0, 2) # Go to the end of the file
    with subprocess.Popen([f"{current_directory}/build/main", "--p", f"{test_time}", "--d", f"{current_directory}/tmp"]) as proc:
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

    clear_tmp_dir(current_directory)

    assert check_result

def test_sigusr1_signal():
    test_time = 1000
    current_file = os.path.realpath(__file__)
    current_directory = os.path.dirname(current_file)

    clear_env_vars()
    create_tmp_dir(current_directory)

    fd = open(syslog_file_path, "r")
    fd.seek(0, 2) # Go to the end of the file
    with subprocess.Popen([f"{current_directory}/build/main", "--p", f"{test_time}", "--d", f"{current_directory}/tmp"]) as proc:
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

    clear_tmp_dir(current_directory)

    assert check_result
from os import system
import subprocess
import os

syslog_file_path = "/var/log/syslog"

# подготовка тесторой дериктории
def create_tmp_dir(path_to_dir: str) -> None:
    tmp_dir_name = f"{path_to_dir}/tmp"
    
    system(f"mkdir {tmp_dir_name}")
    system(f"touch {tmp_dir_name}/1.txt {tmp_dir_name}/2.txt")
    with open(f"{tmp_dir_name}/1.txt", 'w') as f:
        f.write("aaa")
    with open(f"{tmp_dir_name}/2.txt", 'w') as f:
        f.write("bbb")

def change_tmp_dir(path_to_dir: str) -> None:
    tmp_dir_name = f"{path_to_dir}/tmp"

    system(f"rm {tmp_dir_name}/2.txt")
    system(f"touch {tmp_dir_name}/3.txt")
    with open(f"{tmp_dir_name}/1.txt", 'w') as f:
        f.write("ccc")
    with open(f"{tmp_dir_name}/3.txt", 'w') as f:
        f.write("ddd")

def clear_tmp_dir(path_to_dir: str) -> None:
    tmp_dir_name = f"{path_to_dir}/tmp"
    system(f"rm -r {tmp_dir_name}")

def test_integrity_check_err():
    current_file = os.path.realpath(__file__)
    current_directory = os.path.dirname(current_file)

    create_tmp_dir(current_directory)
    
    fd = open(syslog_file_path, "r")
    fd.seek(0, 2) # Go to the end of the file
    with subprocess.Popen([f"{current_directory}/build/main", "--p", "10"]) as proc:
        try:
            change_tmp_dir(current_directory)
            proc.wait(15)
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
    current_file = os.path.realpath(__file__)
    current_directory = os.path.dirname(current_file)

    create_tmp_dir(current_directory)

    fd = open(syslog_file_path, "r")
    fd.seek(0, 2) # Go to the end of the file
    with subprocess.Popen([f"{current_directory}/build/main", "--p", "10"]) as proc:
        try:
            proc.wait(15)
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
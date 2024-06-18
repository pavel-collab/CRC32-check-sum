from os import system
import subprocess
import os

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

def test_simole():
    current_file = os.path.realpath(__file__)
    current_directory = os.path.dirname(current_file)

    create_tmp_dir(current_directory)
    with subprocess.Popen([f"{current_directory}/build/main"]) as proc:
        try:
            change_tmp_dir(current_directory)
            proc.wait(35)
        except subprocess.TimeoutExpired:
            proc.terminate()
            proc.wait()
    
    clear_tmp_dir(current_directory)
    assert 1 == 1
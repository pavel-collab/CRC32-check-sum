from os import system
import os
import random
import string

syslog_file_path = "/var/log/syslog"

def create_tmp_dir(path_to_dir: str) -> None:
    tmp_dir_name = f"{path_to_dir}/tmp"
    
    if os.path.isdir(tmp_dir_name):
        system(f"rm -rf {tmp_dir_name}")
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

def clear_env_vars():
    system(f"unset TARGET_DIR_PATH")
    system(f"unset PERIODE")

def gen_rand_word(length: int):
   letters = string.ascii_lowercase
   return ''.join(random.choice(letters) for i in range(length))
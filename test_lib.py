from os import system
import os
import random
import string
from time import sleep
import json

syslog_file_path = "/var/log/syslog"
json_file_path = "/tmp/log.json"

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

    sleep(1)
    system(f"rm {tmp_dir_name}/2.txt")
    sleep(1)
    system(f"touch {tmp_dir_name}/3.txt")
    with open(f"{tmp_dir_name}/1.txt", 'w') as f:
        f.write("ccc")
    sleep(1)
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

def clear_json_log(json_log_path: str) -> None:
    system(f"rm -r {json_log_path}")

def parce_json(file_name: str):
    with open(file_name, 'r') as config_file:
        info = config_file.read()

    JsonData = json.loads(info)
    return JsonData

def check_json_status_OK(json_data):
    for json_obj in json_data:
        if json_obj["status"] == "OK":
            return True
    return False

def check_json_status_FAIL(json_data):
    for json_obj in json_data:
        if json_obj["status"] == "FAIL":
            return True
    return False

def check_json_status_NEW(json_data):
    for json_obj in json_data:
        if json_obj["status"] == "NEW":
            return True
    return False

def check_json_status_ABSENT(json_data):
    for json_obj in json_data:
        if json_obj["status"] == "ABSENT":
            return True
    return False
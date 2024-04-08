import json
import requests

def get_json_arr(path: str) -> list[dict]:
    with open(path, "r") as file:
        return json.load(file)

def init_users() -> None:
    users = get_json_arr('users_init.json')
    for user in users:
        resp = requests.post(f'http://localhost:8080/user?first_name={user["first_name"]}&last_name={user["last_name"]}&email={user["email"]}&title={user["title"]}&login={user["login"]}&password={user["password"]}')
        print(resp.status_code)

def init_paths() -> None:
    paths = get_json_arr('paths_init.json')
    for path in paths:
        resp = requests.post(f'http://localhost:8080/path?startpoint={path["startpoint"]}&endpoint={path["endpoint"]}')
        print(resp.status_code)

def main() -> None:
    init_users()
    init_paths()

if __name__ == '__main__':
    main()
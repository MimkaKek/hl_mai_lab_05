import json
import requests
from requests.auth import HTTPBasicAuth

def get_json_arr(path: str) -> list[dict]:
    with open(path, "r") as file:
        return json.load(file)

class BearerAuth(requests.auth.AuthBase):
    def __init__(self, token: str):
        self.token = token
    def __call__(self, r: requests.Request):
        r.headers["authorization"] = "Bearer " + self.token
        return r

def main() -> None:
    
    users = get_json_arr('users_init.json')
    for user in users:
        params = {
            "first_name": user["first_name"],
            "last_name": user["last_name"],
            "email": user["email"],
            "title": user["title"],
            "login": user["login"],
            "password": user["password"]
        }
        resp = requests.post(f'http://localhost:8888/user', params=params, auth=HTTPBasicAuth(user["login"], user["password"]))
        print(f"User add status: {resp.status_code}")
        print(f"Response body: {resp.text}")
        if resp.status_code != 200:
            users.remove(user)

    paths = get_json_arr('paths_init.json')
    for path, user in zip(paths, users):
        try:
            params = {
                "startpoint": path["startpoint"],
                "endpoint": path["endpoint"]
            }
            resp = requests.post(f'http://localhost:8888/path', params=params, auth=HTTPBasicAuth(user["login"], user["password"]))
            print(f"Path add status: {resp.status_code}")
            print(f"Response body: {resp.text}")
        except Exception:
            pass
    
    trips = get_json_arr('trips_init.json')
    for trip, user in zip(trips, users):
        print(user["login"], user["password"])
        try:
            params = {
                "id": trip["id"],
                "id_path": trip["id_path"],
                "name": trip["name"],
                "start_time": trip["start_time"],
                "fin_time": trip["fin_time"]
            }
            resp = requests.post(f'http://localhost:8888/trip', params=params, auth=HTTPBasicAuth(user["login"], user["password"]))
            print(f"Trip add status: {resp.status_code}")
            print(f"Response body: {resp.text}")
        except Exception:
            pass

if __name__ == '__main__':
    main()
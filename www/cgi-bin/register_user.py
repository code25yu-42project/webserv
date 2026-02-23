#!/usr/bin/env python3
# -*- coding: utf-8 -*-

import json
import os
import re
import random
import string
from collections import OrderedDict

MAX_USERS = 100  # 최대 사용자 수
USER_ID_START = 1  # 사용자 ID 시작 값
USERS_PATH = './data/users.json'  # 사용자 JSON 파일 경로
CONTENTS_PATH = './data/contents.json'  # 댓글 JSON 파일 경로
CHATROOM_PATH = './data/chatroom.json'  # 채팅방 JSON 파일 경로

def generate_random_name(existing_user_names):
    return 'user_' + ''.join([random.choice(string.ascii_lowercase + string.digits) for _ in range(8)])

def parse_json_input(input_str):
    try:
        data = json.loads(input_str)
        if isinstance(data, dict) and 'user_name' in data:
            return data['user_name']
    except ValueError:
        pass
        
    match = re.search(r'"?user_name"?\s*:\s*"?([^\"]+)"?', input_str)
    if match:
        return match.group(1)
        
    return None

def add_user(new_user):
    try:
        if os.path.exists(USERS_PATH):
            with open(USERS_PATH, 'r') as file:
                try:
                    data = json.load(file, object_pairs_hook=OrderedDict)
                except ValueError:
                    data = []
        else:
            data = []

        if len(data) >= MAX_USERS:
            print(json.dumps({"error": "Maximum user limit reached"}))
            return

        existing_user_names = [user['user_name'] for user in data]

        for user in data:
            if user['user_name'] == new_user['user_name']:
                print(json.dumps({"error": "Username already exists"}))
                return

        if 'user_name' not in new_user or new_user['user_name'] in existing_user_names:
            new_user['user_name'] = generate_random_name(existing_user_names)

        if data:
            max_user_id = max(user['user_id'] for user in data)
            new_user['user_id'] = max_user_id + 1
        else:
            new_user['user_id'] = USER_ID_START

        new_user_ordered = OrderedDict([
            ('user_id', new_user['user_id']),
            ('user_name', new_user['user_name'])
        ])

        data.append(new_user_ordered)

        with open(USERS_PATH, 'w') as file:
            json.dump(data, file, indent=4, separators=(',', ': '))

        print(json.dumps(new_user_ordered, indent=4))

        # Chatroom data 업데이트
        update_chatroom_data(data)

    except Exception as e:
        print(json.dumps({"error": str(e)}))

def update_chatroom_data(user_data):
    try:
        # 댓글 데이터 읽기
        if os.path.exists(CONTENTS_PATH):
            with open(CONTENTS_PATH, 'r') as file:
                try:
                    comments_data = json.load(file)
                except ValueError:
                    comments_data = []
        else:
            comments_data = []

        # 채팅방 데이터 작성
        chatroom_data = {
            "users": user_data,
            "contents": comments_data
        }

        # 채팅방 데이터 저장
        with open(CHATROOM_PATH, 'w') as file:
            json.dump(chatroom_data, file, indent=4, separators=(',', ': '))

        # print("Chatroom data updated at {}".format(CHATROOM_PATH))  # Python 2.x 호환

    except Exception as e:
        print(json.dumps({"error": "Error updating chatroom data: {}".format(str(e))}))

def main():
    body = os.environ.get('BODY', '{}')
    user_name = parse_json_input(body)
    if not user_name:
        user_name = generate_random_name([])

    new_user = {"user_name": user_name}
    add_user(new_user)

if __name__ == "__main__":
    main()

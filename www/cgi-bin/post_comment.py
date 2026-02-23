#!/usr/bin/env python3
# -*- coding: utf-8 -*-  # 인코딩을 명시적으로 지정

import sys
import json
import time
import re
import os
from collections import OrderedDict

# 상단에 설정할 내용 추가
STARTING_CONTENT_ID = 3000  # contents 파일이 비어있을 경우 content_id 시작값
MAX_CONTENT_COUNT = 1000  # contents 파일에 저장될 최대 콘텐츠 개수
USERS_PATH = './data/users.json'  # users 파일 경로
CONTENTS_PATH = './data/contents.json'  # contents 파일 경로
CHATROOM_PATH = './data/chatroom.json'  # chatroom 파일 경로

def parse_json(input_str):
    """
    다양한 형태의 JSON 문자열을 올바르게 변환하는 함수
    """
    try:
        return json.loads(input_str)  # 정상적인 JSON이면 바로 변환
    except ValueError:
        pass
        
    # key-value 쌍을 찾아 JSON 형태로 변환 시도
    input_str = re.sub(r"(\w+):", r'"\1":', input_str)  # key에 따옴표 추가
    input_str = input_str.replace("'", '"')  # 작은따옴표를 큰따옴표로 변환
        
    try:
        return json.loads(input_str)
    except ValueError:
        print(json.dumps({"error": "Invalid JSON format."}))
        sys.exit(1)

def read_json_file(file_path):
    """
    JSON 파일을 읽고, 파일이 비어있으면 빈 리스트를 반환하는 함수
    """
    try:
        with open(file_path, 'r') as f:
            data = f.read().strip()
            return json.loads(data) if data else []
    except (ValueError, IOError):
        return []

def add_new_content(contents_file, users_file, content_data):
    # JSON 파일 읽기 (기존 데이터를 OrderedDict로 관리)
    contents = read_json_file(contents_file)
    users = read_json_file(users_file)

    # contents가 OrderedDict로 처리되는지 확인
    if not isinstance(contents, list):
        print(json.dumps({"error": "Contents data format is incorrect."}))
        return

    # contents 개수 체크
    if len(contents) >= MAX_CONTENT_COUNT:
        print(json.dumps({"error": "Content limit reached. Maximum allowed is {}.".format(MAX_CONTENT_COUNT)}))
        return

    # 중복되지 않는 content_id 찾기
    content_ids = [int(item["content_id"]) for item in contents]
    new_content_id = STARTING_CONTENT_ID if not content_ids else max(content_ids) + 1

    # content_data에서 content와 user_id 추출
    content = content_data.get("content", "")
    user_id = str(content_data.get("user_id", ""))  # user_id를 문자열로 변환

    # user_name 찾기 (user_id가 없을 경우 user_name 사용)
    user_name = content_data.get("user_name") or next((user["user_name"] for user in users if str(user["user_id"]) == user_id), None)

    if user_name is None:
        print(json.dumps({"error": "user_id {} not found.".format(user_id)}))
        return

    # 새로운 content 데이터 생성 (OrderedDict 사용)
    new_content = OrderedDict()
    new_content["user_id"] = int(user_id)
    new_content["user_name"] = user_name
    new_content["content_id"] = new_content_id
    new_content["content"] = content
    new_content["is_exist"] = 1
    new_content["is_comment"] = 1
    new_content["timestamp"] = int(time.time() * 1000)

    # 기존 데이터를 OrderedDict로 변환 (순서 보장)
    contents = [OrderedDict(content) for content in contents]  # 각 항목을 OrderedDict로 변환

    # 새로운 데이터 추가
    contents.append(new_content)

    # 저장할 때 키 순서를 명시적으로 지정하여 순서를 보장
    ordered_contents = reorder_content_keys(contents)

    # contents.json에 저장 (순서 보장)
    with open(contents_file, 'w') as cf:
        json.dump(ordered_contents, cf, indent=4)

    # 추가된 데이터 출력
    print(json.dumps(new_content, indent=4))

    # 이제 chatroom 데이터를 업데이트
    update_chatroom_data(users_file, contents_file)

def reorder_content_keys(contents):
    """
    기존 JSON 항목들의 키 순서를 일관되게 맞추는 함수
    """
    # 키 순서를 정의
    ordered_keys = [
        "user_id", "user_name", "content_id", "content", "is_exist", "is_comment", "timestamp"
    ]
    reordered_contents = []
    for content in contents:
        reordered_content = OrderedDict()
        for key in ordered_keys:
            if key in content:
                reordered_content[key] = content[key]
        reordered_contents.append(reordered_content)
    return reordered_contents

def update_chatroom_data(users_file, contents_file):
    """
    chatroom 파일에 users와 contents 데이터를 업데이트하는 함수
    """
    # users와 contents 데이터를 읽어옵니다.
    users = read_json_file(users_file)
    contents = read_json_file(contents_file)

    # chatroom 데이터 형식에 맞게 조합합니다.
    chatroom_data = {
        "users": users,
        "contents": contents
    }

    # chatroom.json 파일에 저장합니다.
    with open(CHATROOM_PATH, 'w') as cf:
        json.dump(chatroom_data, cf, indent=4)

    # print json.dumps({"message": "Chatroom data updated successfully."}, indent=4)

if __name__ == "__main__":
    # 환경 변수 BODY에서 content_data를 읽어오기
    content_data_json = os.environ.get('BODY')

    if not content_data_json:
        print(json.dumps({"error": "BODY environment variable is missing."}))
        sys.exit(1)

    # content_data를 JSON으로 파싱
    content_data = parse_json(content_data_json)

    # 새 콘텐츠 추가
    add_new_content(CONTENTS_PATH, USERS_PATH, content_data)

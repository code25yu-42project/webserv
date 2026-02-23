#!/usr/bin/env python3
# -*- coding: utf-8 -*-

import os
import json

# 환경변수에서 받아올 파일 경로와 키워드 정의
USERS_PATH = './data/users.json'  # 수정하려는 JSON 파일 경로
CONTENTS_PATH = './data/contents.json'  # contents 파일 경로
CHATROOM_PATH = './data/chatroom.json'  # chatroom 파일 경로

def delete_entry_from_file(file_path, user_id, user_name):
    try:
        # JSON 파일을 읽음
        with open(file_path, 'r') as file:
            data = json.load(file)

        # 삭제 여부 확인
        deleted = False

        # 데이터를 순회하며 해당 user_id와 user_name에 맞는 항목을 찾음
        for index, item in enumerate(data):
            if item.get('user_id') == user_id and item.get('user_name') == user_name:
                del data[index]  # 일치하는 항목 삭제
                deleted = True
                break  # 첫 번째 항목만 삭제하고 종료

        if not deleted:
            return json.dumps({"error": "No matching entry found."}, ensure_ascii=False)

        # 수정된 데이터를 파일에 저장
        with open(file_path, 'w') as file:
            json.dump(data, file, indent=4, ensure_ascii=False)

        # Update the chatroom data after deletion
        update_chatroom_data()

        return json.dumps({"success": "Entry deleted successfully."}, ensure_ascii=False)

    except Exception as e:
        return json.dumps({"error": str(e)}, ensure_ascii=False)

def update_chatroom_data():
    """
    chatroom 파일에 users와 contents 데이터를 업데이트하는 함수
    """
    # users와 contents 데이터를 읽어옵니다.
    users = read_json_file(USERS_PATH)
    contents = read_json_file(CONTENTS_PATH)

    # chatroom 데이터 형식에 맞게 조합합니다.
    chatroom_data = {
        "users": users,
        "contents": contents
    }

    # chatroom.json 파일에 저장합니다.
    with open(CHATROOM_PATH, 'w') as cf:
        json.dump(chatroom_data, cf, indent=4, ensure_ascii=False)

    # print json.dumps({"message": "Chatroom data updated successfully."}, ensure_ascii=False)

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

def main():
    try:
        json_data = os.environ.get('BODY')
        if not json_data:
            # print "Content-type: application/json\n"
            print(json.dumps({"error": "No data provided."}, ensure_ascii=False))
            return
        
        data = json.loads(json_data)
        
        # 필요한 필드 가져오기
        user_id = data.get("user_id")
        user_name = data.get("user_name")

        if not user_id or not user_name:
            # print "Content-type: application/json\n"
            print(json.dumps({"error": "Missing user_id or user_name."}, ensure_ascii=False))
            return
        print(json.dumps({"user_id": user_id, "user_name": user_name}, ensure_ascii=False))
        # 파일에서 데이터 삭제 처리
        result = delete_entry_from_file(USERS_PATH, user_id, user_name)

        # print "Content-type: application/json\n"
        print(result)

    except Exception as e:
        # print "Content-type: application/json\n"
        print(json.dumps({"error": "Error processing request: " + str(e)}, ensure_ascii=False))
        
if __name__ == '__main__':
    main()

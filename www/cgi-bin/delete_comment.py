#!/usr/bin/env python3
# -*- coding: utf-8 -*-

import json
import os
import sys

USERS_PATH = './data/users.json'  # 사용자 파일 경로
CONTENTS_PATH = './data/contents.json'  # JSON 파일 경로
CHATROOM_PATH = './data/chatroom.json'  # chatroom 파일 경로
KEY = 'content_id'  # 수정 기준이 될 키값

def modify_json():
	path_info = os.environ.get('PATH_INFO', '').strip('/')  # '/3001' -> '3001'

	print("Content-Type: application/json\n")

	if not path_info:
		print(json.dumps({"error": "No value provided in PATH_INFO."}))
		return

	try:
		value = int(path_info) if path_info.isdigit() else path_info
	except ValueError:
		value = path_info

	if not os.path.exists(CONTENTS_PATH):
		print(json.dumps({"error": "File '{}' does not exist.".format(CONTENTS_PATH)}))
		return

	try:
		with open(CONTENTS_PATH, 'r') as f:
			try:
				data = json.load(f)
			except ValueError:
				print(json.dumps({"error": "Invalid JSON format in file '{}'".format(CONTENTS_PATH)}))
				return
	except IOError:
		print(json.dumps({"error": "Unable to open file '{}'".format(CONTENTS_PATH)}))
		return

	modified = False

	for obj in data:
		if KEY in obj and obj[KEY] == value:
			obj['is_exist'] = 0
			modified = True
			break

	if not modified:
		print(json.dumps({"error": "No matching content_id '{}' found.".format(value)}))
		return

	try:
		with open(CONTENTS_PATH, 'w') as f:
			json.dump(data, f, indent=4)
		# print(json.dumps({"success": "Content with ID '{}' updated (is_exist=0).".format(value)}))
	except IOError:
		print(json.dumps({"error": "Unable to write to file '{}'".format(CONTENTS_PATH)}))

	update_chatroom_data(USERS_PATH, CONTENTS_PATH, CHATROOM_PATH)

def update_chatroom_data(users_path, contents_path, chatroom_path):
	users = read_json_file(users_path)
	contents = read_json_file(contents_path)
	chatroom_data = {"users": users, "contents": contents}

	try:
		with open(chatroom_path, 'w') as cf:
			json.dump(chatroom_data, cf, indent=4, ensure_ascii=False)
	except IOError:
		print(json.dumps({"error": "Unable to write to file '{}'".format(chatroom_path)}))

def read_json_file(file_path):
	try:
		with open(file_path, 'r') as f:
			data = f.read().strip()
			return json.loads(data) if data else []
	except (ValueError, IOError):
		return []

if __name__ == "__main__":
	modify_json()
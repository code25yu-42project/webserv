#!/usr/bin/php
<?php
// UTF-8 인코딩 설정
header('Content-Type: application/json; charset=utf-8');

define('KEYWORD', 'content_id'); // 수정할 기준 키워드
define('CONTENTS_PATH', './data/contents.json'); // JSON 파일 경로
define('USERS_PATH', './data/users.json'); // 사용자 정보 파일 경로
define('CHATROOM_PATH', './data/chatroom.json'); // 합쳐서 저장할 chatroom.json 경로

// 환경변수에서 PATH_INFO 가져오기
$path_info = getenv('PATH_INFO');
if (!$path_info) {
    echo json_encode(["error" => "No PATH_INFO provided."], JSON_PRETTY_PRINT) . "\n";
    exit(1);
}

$keyword_value = trim($path_info, '/'); // 환경변수에서 값 추출

// JSON 파일 읽기 (contents.json)
$json_data = file_get_contents(CONTENTS_PATH);
if ($json_data === false) {
    echo json_encode(["error" => "Could not read the JSON file."], JSON_PRETTY_PRINT) . "\n";
    exit(1);
}

$data = json_decode($json_data, true);
if (json_last_error() !== JSON_ERROR_NONE) {
    echo json_encode(["error" => "Invalid JSON format in the file."], JSON_PRETTY_PRINT) . "\n";
    exit(1);
}

// 해당 키워드와 값에 일치하는 객체의 is_exist 값을 0으로 변경하고 content 파일 삭제
$modified = false;
foreach ($data as &$item) {
    if (isset($item[KEYWORD]) && $item[KEYWORD] == $keyword_value) {
        $item['is_exist'] = 0;
        if (isset($item['content']) && !empty($item['content']) && file_exists($item['content'])) {
            unlink($item['content']); // 파일 삭제
        }
        $item['content'] = ""; // content 값을 빈 문자열로 수정
        $modified = true;
        break;
    }
}

if (!$modified) {
    echo json_encode(["error" => "No object found with the specified keyword and value."], JSON_PRETTY_PRINT) . "\n";
    exit(1);
}

// 변경된 JSON 저장
if (file_put_contents(CONTENTS_PATH, json_encode($data, JSON_PRETTY_PRINT)) === false) {
    echo json_encode(["error" => "Could not write to the JSON file."], JSON_PRETTY_PRINT) . "\n";
    exit(1);
}

// CHATROOM_PATH에 USERS_PATH와 CONTENTS_PATH 데이터를 저장하는 기능 추가
update_chatroom_data(USERS_PATH, CONTENTS_PATH, CHATROOM_PATH);

// echo json_encode(["success" => "Content with ID '{$keyword_value}' updated (is_exist=0, content deleted)."], JSON_PRETTY_PRINT) . "\n";

function update_chatroom_data($users_path, $contents_path, $chatroom_path) {
    $users = json_decode(file_get_contents($users_path), true);
    $contents = json_decode(file_get_contents($contents_path), true);

    if ($users === null || $contents === null) {
        echo json_encode(["error" => "Error reading or decoding users.json or contents.json."], JSON_PRETTY_PRINT) . "\n";
        exit(1);
    }

    $chatroom_data = [
        "users" => $users,
        "contents" => $contents
    ];

    if (file_put_contents($chatroom_path, json_encode($chatroom_data, JSON_PRETTY_PRINT)) === false) {
        echo json_encode(["error" => "Could not write to chatroom.json file."], JSON_PRETTY_PRINT) . "\n";
        exit(1);
    }
}
?>
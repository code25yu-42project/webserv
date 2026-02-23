#!/usr/bin/php
<?php
// UTF-8 인코딩 설정
header('Content-Type: application/json; charset=utf-8');

define('USERS_PATH', './data/users.json');
define('CONTENTS_PATH', './data/contents.json');
define('CHATROOM_PATH', './data/chatroom.json');  // CHATROOM_PATH 정의
define('FILE_DIR', './data/');
define('MAX_FILES', 1000);
define('CONTEN_ID_START', 100);
define('FILE_ID_START', 200);
define('IS_EXIST', 1);
define('IS_COMMENT', 0);

// 환경변수에서 BODY 가져오기 (없거나 비어있으면 빈 문자열로 처리)
$body = getenv('BODY');
if ($body === false) {
    $body = ""; // BODY가 없을 경우 빈 문자열을 저장
}

$new_content_data = ['file' => $body];

// 'file' 키 값 추출
if (!isset($new_content_data['file'])) {
    echo json_encode(["error" => "'file' key not found in JSON."], JSON_PRETTY_PRINT) . "\n";
    exit(1);
}
$new_content_str = $new_content_data['file']; // 값이 비어도 문제없이 저장

// 'user_id' 환경변수에서 가져오기
$user_id = getenv('USER_ID');
if ($user_id === false || !is_numeric($user_id)) {
    echo json_encode(["error" => "'USER_ID' environment variable is missing or invalid."], JSON_PRETTY_PRINT) . "\n";
    exit(1);
}

// users.json 파일 읽기
$users_data = file_get_contents(USERS_PATH);
if ($users_data === false) {
    echo json_encode(["error" => "Could not read users.json file."], JSON_PRETTY_PRINT) . "\n";
    exit(1);
}

$users = json_decode($users_data, true);
if (json_last_error() !== JSON_ERROR_NONE) {
    echo json_encode(["error" => "Invalid JSON in users.json file."], JSON_PRETTY_PRINT) . "\n";
    exit(1);
}

// user_id에 해당하는 user_name 찾기
$user_name = null;
foreach ($users as $user) {
    if ($user['user_id'] == $user_id) {
        $user_name = $user['user_name'];
        break;
    }
}
if ($user_name === null) {
    echo json_encode(["error" => "user_id '$user_id' not found in users.json."], JSON_PRETTY_PRINT) . "\n";
    exit(1);
}

// content.json 파일 읽기
if (file_exists(CONTENTS_PATH)) {
    $content_data = file_get_contents(CONTENTS_PATH);
    if ($content_data === false) {
        echo json_encode(["error" => "Could not read content.json file."], JSON_PRETTY_PRINT) . "\n";
        exit(1);
    }
    $contents = trim($content_data) === '' ? [] : json_decode($content_data, true);
    if (json_last_error() !== JSON_ERROR_NONE) {
        echo json_encode(["error" => "Invalid JSON in content.json file."], JSON_PRETTY_PRINT) . "\n";
        exit(1);
    }
} else {
    $contents = [];
}

// content_id 계산
$content_ids = array_map(function ($item) { return isset($item['content_id']) ? $item['content_id'] : 0; }, $contents);
$new_content_id = empty($content_ids) ? CONTEN_ID_START : max($content_ids) + 1;

// 디렉토리 확인 및 설정
$script_dir = dirname(__FILE__);
$file_dir = realpath($script_dir . DIRECTORY_SEPARATOR . FILE_DIR);
if (!file_exists($file_dir) || !is_writable($file_dir)) {
    echo json_encode(["error" => "Directory '$file_dir' is not accessible."], JSON_PRETTY_PRINT) . "\n";
    exit(1);
}

// 파일 개수 제한 체크
$numeric_files = array_filter(array_map('intval', preg_replace('/\.txt$/', '', scandir($file_dir))), 'is_numeric');
if (count($numeric_files) >= MAX_FILES) {
    echo json_encode(["error" => "Maximum number of files exceeded."], JSON_PRETTY_PRINT) . "\n";
    exit(1);
}

// 새로운 파일 저장
$next_number = empty($numeric_files) ? FILE_ID_START : max($numeric_files) + 1;
$file_path = "$file_dir/$next_number.txt";
if (file_put_contents($file_path, $new_content_str . "\n", FILE_APPEND) === false) {
    echo json_encode(["error" => "Could not write to file '$file_path'."], JSON_PRETTY_PRINT) . "\n";
    exit(1);
}

// 새로운 콘텐츠 추가
$new_content = [
    'user_id' => (int)$user_id,
    'user_name' => $user_name,
    'content_id' => $new_content_id,
    'content' => str_replace($script_dir . DIRECTORY_SEPARATOR, '', $file_path),
    'is_exist' => IS_EXIST,
    'is_comment' => IS_COMMENT,
    'timestamp' => time() * 1000
];
$contents[] = $new_content;

// content.json 업데이트
if (file_put_contents(CONTENTS_PATH, json_encode($contents, JSON_PRETTY_PRINT)) === false) {
    echo json_encode(["error" => "Could not write to content.json file."], JSON_PRETTY_PRINT) . "\n";
    exit(1);
}

// 결과 출력 (content 경로만 표시, 내용은 숨김)
$new_content['content'] = "";
echo json_encode($new_content, JSON_PRETTY_PRINT);

// CHATROOM_PATH에 USERS_PATH와 CONTENTS_PATH 데이터를 저장하는 기능 추가
update_chatroom_data(USERS_PATH, CONTENTS_PATH, CHATROOM_PATH);

function update_chatroom_data($users_path, $contents_path, $chatroom_path) {
    // users.json과 contents.json 데이터를 읽어와서 chatroom.json에 저장하는 함수
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

    // chatroom.json에 데이터 저장
    if (file_put_contents($chatroom_path, json_encode($chatroom_data, JSON_PRETTY_PRINT)) === false) {
        echo json_encode(["error" => "Could not write to chatroom.json file."], JSON_PRETTY_PRINT) . "\n";
        exit(1);
    }

    // echo json_encode(["success" => "Chatroom data updated successfully."], JSON_PRETTY_PRINT) . "\n";
}
?>

#!/usr/bin/php
<?php
// UTF-8 인코딩 설정
header('Content-Type: application/json; charset=utf-8');

define('KEYWORD', 'content_id'); // 기준 키워드
define('CONTENTS_PATH', './data/contents.json'); // JSON 파일 경로

// 환경변수에서 PATH_INFO 가져오기
$path_info = getenv('PATH_INFO');
if (!$path_info) {
	echo json_encode(["error" => "No PATH_INFO provided."], JSON_PRETTY_PRINT) . "\n";
	exit(1);
}

$keyword_value = trim($path_info, '/'); // 환경변수에서 값 추출

// JSON 파일 읽기
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

// 해당 키워드와 값에 일치하는 오브젝트 찾기
foreach ($data as $item) {
	if (isset($item[KEYWORD]) && $item[KEYWORD] == $keyword_value) {
		// is_comment 값 확인
		if (isset($item['is_comment']) && $item['is_comment'] == 0) {
			$file_to_download = $item['content'];

			// 파일 존재 여부 확인
			if (!file_exists($file_to_download)) {
				echo json_encode(["error" => "File not found: $file_to_download"], JSON_PRETTY_PRINT) . "\n";
				exit(1);
			}

			// 다운로드 헤더 설정
			header('Content-Description: File Transfer');
			header('Content-Type: application/octet-stream');
			header('Content-Disposition: attachment; filename="' . basename($file_to_download) . '"');
			header('Expires: 0');
			header('Cache-Control: must-revalidate');
			header('Pragma: public');
			header('Content-Length: ' . filesize($file_to_download));

			// 파일 읽기 및 출력 (다운로드 실행)
			readfile($file_to_download);
			exit(0);
		} else {
			echo json_encode(["error" => "The content is not a downloadable file."], JSON_PRETTY_PRINT) . "\n";
			exit(1);
		}
	}
}

// 해당하는 오브젝트가 없을 경우
echo json_encode(["error" => "No object found with the specified keyword and value."], JSON_PRETTY_PRINT) . "\n";
exit(1);

?>

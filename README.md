# [日本語](#日本語) | [English](#english) | [한글](#한글)

<a id="日本語"></a>
## 日本語

### 概要
`webserv` は C++98 で実装された軽量 HTTP サーバーです。  
`kqueue` ベースのイベントループ、設定ファイルパーサー、静的ファイル配信、CGI(Python/PHP)、簡易 API(チャットルーム)を提供します。

### 主な構成
- `backend/` : ソケット、kqueue、HTTP メッセージ処理、Controller/Model/View、CGI、セッション
- `config/` : nginx 風設定(`http/server/location`)のパースと継承
- `checker/` : 設定値・文法チェック
- `utils/conf.d/` : 実行設定 (`webserv.conf`, `test.conf`)
- `www/cgi-bin/` : Python/PHP CGI スクリプトと JSON データ
- `frontend/dist/` : 静的フロントエンド配信先

### ビルド
```bash
make -j4
```

### 実行
デフォルト設定で起動:
```bash
./webserv
```

設定ファイルを指定して起動:
```bash
./webserv utils/conf.d/webserv.conf
```

使用中ポート(2424/4242)を強制解放:
```bash
bash scripts/kill_ports.sh
```

### デフォルト設定 (`utils/conf.d/webserv.conf`)
- `localhost:2424` : メインサーバー
- `4242` : 内部 API サーバー(一部 `proxy_pass` 先)
- 静的配信: `/` → `frontend/dist/index.html`
- エラーページ: `/40x.html`, `/50x.html`

### 代表 API / ルート
- `GET /` : SPA 静的ファイル
- `GET /assets/*` : フロント資産配信
- `POST /api/users/register` : `register_user.py` (設定上 `return 301 /chatroom` も定義)
- `GET /api/chatroom` : `www/cgi-bin/data/chatroom.json`
- `POST /api/chatroom/comments` : `proxy_pass localhost:4242/api/chatroom/comment`
- `DELETE /api/chatroom/comments...` : `delete_comment.py`
- `POST /api/chatroom/files` : `proxy_pass localhost:4242/api/chatroom/file`
- `DELETE /api/chatroom/files...` : `delete_file.php`
- `GET /api/chatroom/files/download...` : `get_file.php`
- `GET /api/dir` : `www` の autoindex

### 実行要件
- macOS/BSD 系 (`kqueue` 使用)
- C++ コンパイラ (`-std=c++98`)
- Python 3 (`/usr/bin/python3`)
- PHP (`/opt/homebrew/bin/php`)

> CGI 実行パスは `backend/Cgi.hpp` の `PYTHON_PATH`, `PHP_PATH` で定義されています。環境に合わせて調整してください。

### 注意
サーバー終了時に `www/cgi-bin/data/users.json`, `contents.json`, `chatroom.json` を truncate する実装になっています。データ保持が必要な場合は `ServerManager` デストラクタ処理を確認してください。

---

<a id="english"></a>
## English

### Overview
`webserv` is a lightweight HTTP server written in C++98.  
It includes a `kqueue`-based event loop, nginx-style config parsing, static file serving, CGI (Python/PHP), and simple chatroom APIs.

### Project Layout
- `backend/`: sockets, kqueue loop, HTTP parsing, Controller/Model/View, CGI, session logic
- `config/`: parser/inheritance for `http/server/location` directives
- `checker/`: syntax and value validation
- `utils/conf.d/`: runtime configs (`webserv.conf`, `test.conf`)
- `www/cgi-bin/`: CGI scripts + JSON data files
- `frontend/dist/`: static frontend output served by the main server

### Build
```bash
make -j4
```

### Run
Run with default config:
```bash
./webserv
```

Run with an explicit config path:
```bash
./webserv utils/conf.d/webserv.conf
```

Force-kill occupied ports (2424/4242):
```bash
bash scripts/kill_ports.sh
```

### Default Runtime (`utils/conf.d/webserv.conf`)
- `localhost:2424`: main server
- `4242`: upstream/internal API server for proxied endpoints
- Static root: `/` serves `frontend/dist/index.html`
- Error pages: `/40x.html`, `/50x.html`

### Key Routes / APIs
- `GET /`: SPA/static frontend
- `GET /assets/*`: frontend assets
- `POST /api/users/register`: `register_user.py` (config also defines `return 301 /chatroom`)
- `GET /api/chatroom`: serves `www/cgi-bin/data/chatroom.json`
- `POST /api/chatroom/comments`: proxied to `localhost:4242/api/chatroom/comment`
- `DELETE /api/chatroom/comments...`: `delete_comment.py`
- `POST /api/chatroom/files`: proxied to `localhost:4242/api/chatroom/file`
- `DELETE /api/chatroom/files...`: `delete_file.php`
- `GET /api/chatroom/files/download...`: `get_file.php`
- `GET /api/dir`: autoindex for `www`

### Requirements
- macOS/BSD environment (`kqueue`)
- C++ compiler with C++98 support
- Python 3 (`/usr/bin/python3`)
- PHP (`/opt/homebrew/bin/php`)

> CGI interpreter paths are defined in `backend/Cgi.hpp` (`PYTHON_PATH`, `PHP_PATH`). Update them if your local paths differ.

### Note
On server shutdown, `www/cgi-bin/data/users.json`, `contents.json`, and `chatroom.json` are truncated by current destructor logic. Review `ServerManager` if you need persistent data.

---

<a id="한글"></a>
## 한글

### 개요
`webserv`는 C++98로 구현된 경량 HTTP 서버입니다.  
`kqueue` 기반 이벤트 루프, nginx 스타일 설정 파싱, 정적 파일 서빙, CGI(Python/PHP), 채팅방용 API를 포함합니다.

### 주요 디렉토리
- `backend/`: 소켓, kqueue 이벤트 처리, HTTP 파싱, Controller/Model/View, CGI, 세션
- `config/`: `http/server/location` 설정 파싱 및 상속
- `checker/`: 문법/값 검증
- `utils/conf.d/`: 실행 설정 파일(`webserv.conf`, `test.conf`)
- `www/cgi-bin/`: CGI 스크립트 및 JSON 데이터
- `frontend/dist/`: 메인 서버가 제공하는 정적 프런트 결과물

### 빌드
```bash
make -j4
```

### 실행
기본 설정으로 실행:
```bash
./webserv
```

설정 파일을 지정해서 실행:
```bash
./webserv utils/conf.d/webserv.conf
```

포트(2424/4242) 점유 프로세스 강제 종료:
```bash
bash scripts/kill_ports.sh
```

### 기본 동작 (`utils/conf.d/webserv.conf`)
- `localhost:2424`: 메인 서버
- `4242`: 프록시 대상 내부 API 서버
- 정적 루트: `/` → `frontend/dist/index.html`
- 에러 페이지: `/40x.html`, `/50x.html`

### 주요 라우트 / API
- `GET /`: SPA/정적 프런트
- `GET /assets/*`: 프런트 자산
- `POST /api/users/register`: `register_user.py` (설정에 `return 301 /chatroom`도 존재)
- `GET /api/chatroom`: `www/cgi-bin/data/chatroom.json` 반환
- `POST /api/chatroom/comments`: `localhost:4242/api/chatroom/comment`로 프록시
- `DELETE /api/chatroom/comments...`: `delete_comment.py`
- `POST /api/chatroom/files`: `localhost:4242/api/chatroom/file`로 프록시
- `DELETE /api/chatroom/files...`: `delete_file.php`
- `GET /api/chatroom/files/download...`: `get_file.php`
- `GET /api/dir`: `www` 디렉토리 autoindex

### 실행 환경
- macOS/BSD 계열 (`kqueue` 사용)
- C++98 지원 컴파일러
- Python 3 (`/usr/bin/python3`)
- PHP (`/opt/homebrew/bin/php`)

> CGI 인터프리터 경로는 `backend/Cgi.hpp`의 `PYTHON_PATH`, `PHP_PATH` 매크로를 사용합니다. 로컬 환경에 맞게 수정하세요.

### 주의 사항
현재 구현은 서버 종료 시 `www/cgi-bin/data/users.json`, `contents.json`, `chatroom.json`을 truncate합니다. 데이터 영속성이 필요하면 `ServerManager` 소멸자 로직을 조정하세요.

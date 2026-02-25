#!/bin/bash

# webserv에서 사용하는 포트들
PORTS=(2424 4242)

echo "=== Checking and killing processes on ports ==="

for PORT in "${PORTS[@]}"; do
    echo ""
    echo "🔍 Checking port $PORT..."
    
    # lsof로 해당 포트를 사용하는 프로세스 찾기
    PID=$(lsof -ti :$PORT)
    
    if [ -z "$PID" ]; then
        echo "✓ Port $PORT is free"
    else
        echo "⚠️  Port $PORT is in use by PID: $PID"
        
        # 프로세스 정보 출력
        ps -p $PID -o pid,comm,args 2>/dev/null || true
        
        # 프로세스 종료
        echo "💀 Killing process $PID..."
        kill -9 $PID 2>/dev/null
        
        # 종료 확인
        sleep 0.5
        if lsof -ti :$PORT > /dev/null 2>&1; then
            echo "❌ Failed to kill process on port $PORT"
        else
            echo "✅ Successfully killed process on port $PORT"
        fi
    fi
done

echo ""
echo "=== Done ==="

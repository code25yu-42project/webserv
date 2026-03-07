#!/bin/bash

# Ports used by webserv
PORTS=(2424 4242)

echo "=== Checking and killing processes on ports ==="

for PORT in "${PORTS[@]}"; do
    echo ""
    echo "🔍 Checking port $PORT..."
    
    # Find the process using this port with lsof
    PID=$(lsof -ti :$PORT)
    
    if [ -z "$PID" ]; then
        echo "✓ Port $PORT is free"
    else
        echo "⚠️  Port $PORT is in use by PID: $PID"
        
        # Print process information
        ps -p $PID -o pid,comm,args 2>/dev/null || true
        
        # Terminate the process
        echo "💀 Killing process $PID..."
        kill -9 $PID 2>/dev/null
        
        # Verify termination
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

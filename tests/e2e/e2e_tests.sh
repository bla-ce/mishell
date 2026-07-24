#!/bin/bash

MISHLI_PATH=../../bin/mishli
MISHELL_PATH=../../bin/mishell

PID=""

cleanup() {
  kill $PID 2>/dev/null
}
trap cleanup EXIT

# wait for the mishell instance to be ready
# $1: host port
wait_for_mishell() {
  local port=$1

  while true; do
    payload=$($MISHLI_PATH --host 127.0.0.1:$port hello)

    if [ "$payload" == "OK" ]; then
      break
    fi

    sleep 1
  done
}

# start mishell instance
PORT=7474
HOST_NAME="home"

$MISHELL_PATH init --port $PORT --name $HOST_NAME > /dev/null 2>&1 &
PID=$!

wait_for_mishell $PORT

./e2e_test_service_lifecycle.sh
./e2e_test_query.sh

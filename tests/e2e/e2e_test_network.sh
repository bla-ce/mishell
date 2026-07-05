#!/bin/bash

MISHLI_PATH=../../bin/mishli
MISHELL_PATH=../../bin/mishell

# Track PIDs
HOME_PID=""
LAB_PID=""
WORK_PID=""

cleanup() {
  kill $HOME_PID $LAB_PID $WORK_PID 2>/dev/null
}
trap cleanup EXIT

# start host
$MISHELL_PATH init --port 7474 --name home > /dev/null 2>&1 &
HOME_PID=$!

echo -n "network with one host up (home)..."
payload=$($MISHLI_PATH --host 127.0.0.1:7474 network)
expected="0.0.0.0:7474 home UP"

if [ "$payload" = "$expected" ]; then
  echo -e "\033[32mPASSED\033[0m"
else
  echo -e "\033[31mFAILED\033[0m"
  echo "Expected: $expected"
  echo "Received: $payload"
  exit 1
fi

# connect two other hosts
$MISHELL_PATH connect 127.0.0.1:7474 --port 5656 --name lab > /dev/null 2>&1 &
LAB_PID=$!

echo -n "network with two hosts up (lab)..."
payload=$($MISHLI_PATH --host 127.0.0.1:7474 network)
expected=$'0.0.0.0:7474 home UP\n0.0.0.0:5656 lab UP'

if [ "$payload" = "$expected" ]; then
  echo -e "\033[32mPASSED\033[0m"
else
  echo -e "\033[31mFAILED\033[0m"
  echo "Expected: $expected"
  echo "Received: $payload"
  exit 1
fi

$MISHELL_PATH connect 127.0.0.1:5656 --port 3939 --name work > /dev/null 2>&1 &
WORK_PID=$!

echo -n "network with three hosts up (work)..."
payload=$($MISHLI_PATH --host 127.0.0.1:7474 network)
expected=$'0.0.0.0:7474 home UP\n0.0.0.0:5656 lab UP\n0.0.0.0:3939 work UP'

if [ "$payload" = "$expected" ]; then
  echo -e "\033[32mPASSED\033[0m"
else
  echo -e "\033[31mFAILED\033[0m"
  echo "Expected: $expected"
  echo "Received: $payload"
  exit 1
fi

echo -n "shutdown lab and wait 15sec, should be down..."

kill $LAB_PID 2>/dev/null
sleep 15s

payload=$($MISHLI_PATH --host 127.0.0.1:7474 network)
expected=$'0.0.0.0:7474 home UP\n0.0.0.0:5656 lab DOWN\n0.0.0.0:3939 work UP'

if [ "$payload" = "$expected" ]; then
  echo -e "\033[32mPASSED\033[0m"
else
  echo -e "\033[31mFAILED\033[0m"
  echo "Expected: $expected"
  echo "Received: $payload"
  exit 1
fi

echo -n "same command to another peer should return the same thing..."

payload=$($MISHLI_PATH --host 127.0.0.1:3939 network)

if [ "$payload" = "$expected" ]; then
  echo -e "\033[32mPASSED\033[0m"
else
  echo -e "\033[31mFAILED\033[0m"
  echo "Expected: $expected"
  echo "Received: $payload"
  exit 1
fi

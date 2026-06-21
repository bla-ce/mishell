#!/bin/bash

MISHLI_PATH=../../bin/mishli

echo -n "sending REGISTER with args should return OK: "

payload=$($MISHLI_PATH --host 127.0.0.1:7474 register home 0 ping)
expected="OK"

if [ "$payload" = "$expected" ]; then
  echo -e "\033[32mPASSED\033[0m"
else
  echo -e "\033[31mFAILED\033[0m"
  echo "Expected: $expected"
  echo "Received: $payload"
  exit 1
fi

echo -n "sending START on registering service should return OK: "

payload=$($MISHLI_PATH --host 127.0.0.1:7474 start home ping)
expected="OK"

if [ "$payload" = "$expected" ]; then
  echo -e "\033[32mPASSED\033[0m"
else
  echo -e "\033[31mFAILED\033[0m"
  echo "Expected: $expected"
  echo "Received: $payload"
  exit 1
fi

echo -n "sending START on not registered service should return ERROR: "

payload=$($MISHLI_PATH --host 127.0.0.1:7474 start home hello)
expected="ERROR: service not found"

if [ "$payload" = "$expected" ]; then
  echo -e "\033[32mPASSED\033[0m"
else
  echo -e "\033[31mFAILED\033[0m"
  echo "Expected: $expected"
  echo "Received: $payload"
  exit 1
fi

echo -n "sending STOP on started service should return OK: "

payload=$($MISHLI_PATH --host 127.0.0.1:7474 stop home ping)
expected="OK"

if [ "$payload" = "$expected" ]; then
  echo -e "\033[32mPASSED\033[0m"
else
  echo -e "\033[31mFAILED\033[0m"
  echo "Expected: $expected"
  echo "Received: $payload"
  exit 1
fi

echo -n "sending STOP on not registered service should return ERROR: "

payload=$($MISHLI_PATH --host 127.0.0.1:7474 stop home hello)
expected="ERROR: service not found"

if [ "$payload" = "$expected" ]; then
  echo -e "\033[32mPASSED\033[0m"
else
  echo -e "\033[31mFAILED\033[0m"
  echo "Expected: $expected"
  echo "Received: $payload"
  exit 1
fi

echo -n "sending START on stopped service should return OK: "

payload=$($MISHLI_PATH --host 127.0.0.1:7474 start home ping)
expected="OK"

if [ "$payload" = "$expected" ]; then
  echo -e "\033[32mPASSED\033[0m"
else
  echo -e "\033[31mFAILED\033[0m"
  echo "Expected: $expected"
  echo "Received: $payload"
  exit 1
fi

echo -n "sending UNREGISTER on started service should return ERROR: "

payload=$($MISHLI_PATH --host 127.0.0.1:7474 unregister home ping)
expected="ERROR: stop service before unregistering it"

if [ "$payload" = "$expected" ]; then
  echo -e "\033[32mPASSED\033[0m"
else
  echo -e "\033[31mFAILED\033[0m"
  echo "Expected: $expected"
  echo "Received: $payload"
  exit 1
fi

payload=$($MISHLI_PATH --host 127.0.0.1:7474 stop home ping)

echo -n "sending UNREGISTER on stopped service should return OK: "

payload=$($MISHLI_PATH --host 127.0.0.1:7474 unregister home ping)
expected="OK"

if [ "$payload" = "$expected" ]; then
  echo -e "\033[32mPASSED\033[0m"
else
  echo -e "\033[31mFAILED\033[0m"
  echo "Expected: $expected"
  echo "Received: $payload"
  exit 1
fi

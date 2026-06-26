#!/bin/bash

MISHLI_PATH=../../bin/mishli

echo -n "Registering service PING name service..."

payload=$($MISHLI_PATH --host 127.0.0.1:7474 register home PING service)
expected="OK"

if [ "$payload" = "$expected" ]; then
  echo -e "\033[32mPASSED\033[0m"
else
  echo -e "\033[31mFAILED\033[0m"
  echo "Expected: $expected"
  echo "Received: $payload"
  exit 1
fi

echo -n "sending QUERY to stopped service should return ERROR..."

payload=$($MISHLI_PATH --host 127.0.0.1:7474 query home service PING)
expected="ERROR: service not started"

if [ "$payload" = "$expected" ]; then
  echo -e "\033[32mPASSED\033[0m"
else
  echo -e "\033[31mFAILED\033[0m"
  echo "Expected: $expected"
  echo "Received: $payload"
  exit 1
fi

echo -n "Starting service..."

payload=$($MISHLI_PATH --host 127.0.0.1:7474 start home service)
expected="OK"

if [ "$payload" = "$expected" ]; then
  echo -e "\033[32mPASSED\033[0m"
else
  echo -e "\033[31mFAILED\033[0m"
  echo "Expected: $expected"
  echo "Received: $payload"
  exit 1
fi

echo -n "sending QUERY to undefined service should return ERROR..."

payload=$($MISHLI_PATH --host 127.0.0.1:7474 query home unnamed ping)
expected="ERROR: service not found"

if [ "$payload" = "$expected" ]; then
  echo -e "\033[32mPASSED\033[0m"
else
  echo -e "\033[31mFAILED\033[0m"
  echo "Expected: $expected"
  echo "Received: $payload"
  exit 1
fi

echo -n "sending QUERY PING to ping service should return pong..."

payload=$($MISHLI_PATH --host 127.0.0.1:7474 query home service PING)
expected="pong"

if [ "$payload" = "$expected" ]; then
  echo -e "\033[32mPASSED\033[0m"
else
  echo -e "\033[31mFAILED\033[0m"
  echo "Expected: $expected"
  echo "Received: $payload"
  exit 1
fi

echo -n "sending QUERY with wrong command should return ERROR..."

payload=$($MISHLI_PATH --host 127.0.0.1:7474 query home service HELLO)
expected="ERROR: command not found"

if [ "$payload" = "$expected" ]; then
  echo -e "\033[32mPASSED\033[0m"
else
  echo -e "\033[31mFAILED\033[0m"
  echo "Expected: $expected"
  echo "Received: $payload"
  exit 1
fi

# Clean up

echo -n "Stopping service..."

payload=$($MISHLI_PATH --host 127.0.0.1:7474 stop home service)
expected="OK"

if [ "$payload" = "$expected" ]; then
  echo -e "\033[32mPASSED\033[0m"
else
  echo -e "\033[31mFAILED\033[0m"
  echo "Expected: $expected"
  echo "Received: $payload"
  exit 1
fi

echo -n "Unregistering service..."

payload=$($MISHLI_PATH --host 127.0.0.1:7474 unregister home service)
expected="OK"

if [ "$payload" = "$expected" ]; then
  echo -e "\033[32mPASSED\033[0m"
else
  echo -e "\033[31mFAILED\033[0m"
  echo "Expected: $expected"
  echo "Received: $payload"
  exit 1
fi

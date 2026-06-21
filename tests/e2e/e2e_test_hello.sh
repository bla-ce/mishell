#!/bin/bash

MISHLI_PATH=../../bin/mishli

echo -n "sending HELLO should return OK: "

payload=$($MISHLI_PATH --host 127.0.0.1:7474 hello)

if [ "$payload" = "OK" ]; then
  echo -e "\033[32mPASSED\033[0m"
else
  echo -e "\033[31mFAILED\033[0m"
  echo "Expected: OK"
  echo "Received: $payload"
  exit 1
fi

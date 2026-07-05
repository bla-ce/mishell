#!/bin/bash

# Run mishell
../../bin/mishell init --port 7474 --name home > /dev/null 2>&1 &

# Save PID
PID=$!

cleanup() {
  kill $PID 2>/dev/null
}
trap cleanup EXIT

./e2e_test_hello.sh
./e2e_test_service_lifecycle.sh
./e2e_test_query.sh

./e2e_test_network.sh

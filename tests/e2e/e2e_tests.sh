#!/bin/bash

# Run mishell
../../bin/mishell init --port 7474 --name home > /dev/null 2>&1 &

# Save PID
PID=$!

./e2e_test_hello.sh
./e2e_test_service_lifecycle.sh

# Kill mishell
kill $PID

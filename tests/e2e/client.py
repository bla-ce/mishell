from test_validation import run as run_validation
from test_auth import run as run_auth
from test_register import run as run_register
from test_lifecycle import run as run_lifecycle
from test_commands import run as run_commands

run_validation()
host_id = run_auth()
service_id = run_register(host_id)
run_lifecycle(host_id, service_id)
run_commands(host_id)

print("All tests passed!")

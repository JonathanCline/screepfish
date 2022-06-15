#!/usr/bin/env bash

set -e

# Path to this script's parent directory
SCRIPT_DIR=$(dirname ${BASH_SOURCE})

# Path to the repo's root directory
REPO_ROOT=${SCRIPT_DIR}

# Path to the tools directory
TOOLS_ROOT=${REPO_ROOT}/tools

# Invoke the python script and forward in given args
"${BASH}" "$TOOLS_ROOT/utils/build.sh" $@

# Forward exit code
exit $?

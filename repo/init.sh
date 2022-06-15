#!/usr/bin/env bash

set -e

# Path to this script's parent directory
SCRIPT_DIR=$(dirname ${BASH_SOURCE})

# Path to the repo's root directory
REPO_ROOT=${SCRIPT_DIR}/..

# Path to the tools directory
TOOLS_ROOT=${REPO_ROOT}/tools

# Update and configure tools repo as needed
TOOLS_REPO_BRANCH=develop
git -C "${REPO_ROOT}" submodule update --init --remote "${TOOLS_ROOT}"
git -C "${TOOLS_ROOT}" checkout $TOOLS_REPO_BRANCH

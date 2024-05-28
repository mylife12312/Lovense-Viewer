#!/usr/bin/env bash

# This is the custom build script for the viewer
#
# It must be run by the Linden Lab build farm shared buildscript because
# it relies on the environment that sets up, functions it provides, and
# the build result post-processing it does.
#
# The shared buildscript build.sh invokes this because it is named 'build.sh',
# which is the default custom build script name in buildscripts/hg/BuildParams
#
# PLEASE NOTE:
#
# * This script is interpreted on three platforms, including windows and cygwin
#   Cygwin can be tricky....
# * The special style in which python is invoked is intentional to permit
#   use of a native python install on windows - which requires paths in DOS form
# git clone https://github.com/secondlife/build-variables
# autobuild configure -c RelWithDebInfoOS -A 64

cd "$(dirname "$0")"
export AUTOBUILD_ADDRSIZE=64
export LL_BUILD=""
CurrentDir="$(dirname "$0")"
export AUTOBUILD_VARIABLES_FILE=${CurrentDir}/build-variables-mac/variables

autobuild configure -c RelWithDebInfoOS



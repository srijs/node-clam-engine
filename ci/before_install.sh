#!/bin/bash

set -e

if [ "$TRAVIS_OS_NAME" = "linux" ]; then
sudo apt-get -qq update
sudo apt-get -qq install libclamav-dev
fi

if [ "$TRAVIS_OS_NAME" = "osx" ]; then
brew install clamav
fi

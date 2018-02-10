#!/bin/bash

set -e

if [ "$TRAVIS_OS_NAME" = "linux" ]; then
sudo mkdir /var/lib/clamav && sudo chmod 777 /var/lib/clamav
wget http://db.us.clamav.net/main.cvd -O /var/lib/clamav/main.cvd
wget http://db.us.clamav.net/daily.cvd -O /var/lib/clamav/daily.cvd
fi

if [ "$TRAVIS_OS_NAME" = "osx" ]; then
curl http://db.us.clamav.net/main.cvd > `brew --prefix`/share/clamav/main.cvd
curl http://db.us.clamav.net/daily.cvd > `brew --prefix`/share/clamav/daily.cvd
fi

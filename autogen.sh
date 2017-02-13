#! /bin/sh

set +x
set -e

test -f VERSION
test -f COPYING

./scripts/gen-configure_ac.sh

mkdir -p m4
intltoolize --force --copy --automake
autoreconf --force --install --verbose


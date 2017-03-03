#!/bin/bash

set -e

cd "$(dirname "$0")/../po"

echo "Updating POT template..."
intltool-update --pot --gettext-package=pidgin-birthday-reminder

# Fix C-Style multi-line comments
sed -i \
	-e 's/^#\. \* /#. /' \
	-e '/^#\.$/d' \
	pidgin-birthday-reminder.pot

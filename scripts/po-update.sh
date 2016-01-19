#!/bin/bash

cd "$(dirname "$0")/../po"

echo "Updating POT template..."
intltool-update --pot --gettext-package=pidgin-birthday-reminder

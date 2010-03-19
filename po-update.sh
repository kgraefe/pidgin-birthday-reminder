#!/bin/bash
cd po
echo "Checking german language file..."
intltool-update de
echo "Checking russian language file..."
intltool-update ru 
echo "Checking french language file..."
intltool-update fr
echo "Checking spanish language file..."
intltool-update es

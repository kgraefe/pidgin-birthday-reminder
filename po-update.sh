#!/bin/bash
cd po
echo "Updating POT template..."
intltool-update -po
echo "Checking German language file..."
intltool-update de
echo "Checking Russian language file..."
intltool-update ru 
echo "Checking French language file..."
intltool-update fr
echo "Checking Spanish language file..."
intltool-update es
echo "Checking Turkish language file..."
intltool-update tr
echo "Checking Hebrew language file..."
intltool-update he
echo "Checking Portuguese language file..."
intltool-update pt
echo "Checking Galician language file..."
intltool-update gl

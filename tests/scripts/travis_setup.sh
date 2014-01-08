#!/bin/bash
cd $HOME
sudo add-apt-repository -y ppa:upubuntu-com/office
sudo apt-get update -y
sudo apt-get install -y openoffice
sudo apt-get install -y openoffice.org-writer
sudo apt-get install -y openoffice.org-draw
sudo apt-get install -y openoffice.org-calc
sudo apt-get install -y openoffice.org-impress

echo 'Starting Open Office'
soffice -headless -accept="socket,host=127.0.0.1,port=8100;urp;" -nofirststartwizard 2>1 &
echo 'Open Office Started'
wget http://downloads.sourceforge.net/project/jodconverter/JODConverter/2.2.2/jodconverter-2.2.2.zip
unzip jodconverter-2.2.2.zip
cp jodconverter-2.2.2 $HOME/drupal-*/sites/all/libraries

sleep 20

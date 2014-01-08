#!/bin/bash
cd $HOME
sudo add-apt-repository ppa:upubuntu-com/office
sudo apt-get update
sudo apt-get install openoffice
sudo apt-get install openoffice.org-writer
sudo apt-get install openoffice.org-draw
sudo apt-get install openoffice.org-calc
sudo apt-get install openoffice.org-impress

soffice -headless -accept="socket,host=127.0.0.1,port=8100;urp;" -nofirststartwizard
wget http://downloads.sourceforge.net/project/jodconverter/JODConverter/2.2.2/jodconverter-2.2.2.zip
unzip jodconverter-2.2.2.zip
cp jodconverter-2.2.2 $HOME/drupal-*/sites/all/libraries

sleep 20

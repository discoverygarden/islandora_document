BUILD STATUS
------------
Current build status:
[![Build Status](https://travis-ci.org/discoverygarden/islandora_solution_pack_document.png?branch=7.x)](https://travis-ci.org/discoverygarden/islandora_solution_pack_document)

CI Server:
http://jenkins.discoverygarden.ca

SUMMARY
-------
Islandora document solution pack provides a collection and a content model for users’ documents.
Documents are converted to the pdf format to display them. This solution pack needs OpenOffice running as a service and
also the JODCoverter library installed.

REQUIREMENTS
------------
* [Islandora](http://github.com/islandora/islandora)
* [ImageMagick](http://drupal.org/project/imagemagick)
* [Islandora JODConverter] (http://github.com/discoverygarden/islandora_jodconverter)
* [Portable Document Format (PDF) to text converter (version 3.00)](http://poppler.freedesktop.org/)


INSTALLATION
------------
Before installing be sure you've followed the instructions for each of the required modules.

### Install PDF to Text on CentOS / RedHat
```sh
yum install poppler-utils
```

### Install PDF to Text on Ubuntu
```sh
sudo apt-get install poppler-utils
```
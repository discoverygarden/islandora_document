# Document Solution Pack

## Introduction

Islandora document solution pack provides a collection and a content model for users’ documents.
Documents are converted to the pdf format to display them. This solution pack needs OpenOffice running as a service and
also the JODCoverter library installed.

## Requirements

This module requires the following:

* [Islandora](http://github.com/discoverygarden/islandora)
* [ImageMagick](http://drupal.org/project/imagemagick)
* [Islandora JODConverter] (http://github.com/discoverygarden/islandora_jodconverter)
* [Portable Document Format (PDF) to text converter (version 3.00)](http://poppler.freedesktop.org/)


## Installation

Before installing be sure you've followed the instructions for each of the required modules.

### Install PDF to Text on CentOS / RedHat
```sh
yum install poppler-utils
```

### Install PDF to Text on Ubuntu
```sh
sudo apt-get install poppler-utils
```

## Configuration

Configuration options can be found at Configuration > Islandora > Solution pack
configuration > Document Solution Pack
(admin/config/islandora/solution_pack_config/document).

## Troubleshooting/Issues

Having problems or solved one? Create an issue, check out the Islandora Google
groups.

* [Users](https://groups.google.com/forum/?hl=en&fromgroups#!forum/islandora)
* [Devs](https://groups.google.com/forum/?hl=en&fromgroups#!forum/islandora-dev)

or contact [discoverygarden](http://support.discoverygarden.ca).

## Maintainers/Sponsors

Current maintainers:

* [discoverygarden](http://www.discoverygarden.ca)

## Development

If you would like to contribute to this module, please check out the helpful
[Documentation](https://github.com/Islandora/islandora/wiki#wiki-documentation-for-developers),
[Developers](http://islandora.ca/developers) section on Islandora.ca and create
an issue, pull request and or contact
[discoverygarden](http://support.discoverygarden.ca).

## License

[GPLv3](http://www.gnu.org/licenses/gpl-3.0.txt)

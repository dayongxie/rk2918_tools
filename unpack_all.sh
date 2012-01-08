#!/bin/sh

BASE_DIR=`dirname $0`

$BASE_DIR/img_unpack $1 $1.tmp

$BASE_DIR/afptool -unpack $1.tmp $2

rm $1.tmp

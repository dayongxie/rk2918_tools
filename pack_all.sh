#!/bin/sh

BASE_DIR=`dirname $0`

BOOTLOADER=$(awk '$1=="bootloader" {print $2}' $1/package-file)
BOOTLOADER=$(echo $BOOTLOADER | tr -d '\r')

$BASE_DIR/afptool -pack $1 $2.tmp

$BASE_DIR/img_maker $1/$BOOTLOADER $2.tmp $2

rm $2.tmp

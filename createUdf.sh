#!/bin/bash

UDFPATH="/tmp/cdimage.udf"

if [ "$#" != "1" ]
then
    echo "Usage: $0 <directory>"
    echo "Create a UDF Filesystem from a directory"
    exit
fi

if ! command -v "mkudffs" >/dev/null
then
    echo "You need to install udftools"
    exit
fi

if [ -d $1 ]
then
truncate -s 650M $UDFPATH &> /dev/null
mkudffs --media-type=dvd $UDFPATH &> /dev/null
sudo mkdir -p /media/udfimage/ &> /dev/null
sudo mount -t udf -o loop,rw $UDFPATH /media/udfimage/  &> /dev/null
sudo mount -o remount,rw /media/udfimage/  &> /dev/null
sudo cp -r $1/* "/media/udfimage/"  &> /dev/null
sudo umount /dev/loop0 &> /dev/null  &> /dev/null
echo "UDF Filesystem created at $UDFPATH"
else
    echo "$1 is not a directory"
fi

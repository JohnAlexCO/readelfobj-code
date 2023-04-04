#!/bin/sh
n=test002
if [ x$DWTOPSRCDIR = "x" ]
then
  top_srcdir=$top_blddir
else
  top_srcdir=$DWTOPSRCDIR
fi
srcdir=$top_srcdir/test

base=$n.base

o=$srcdir/libkrb5support.so.0.1.debug
#Version-readelfobj:
x="./readelfobj --version"
echo "==== $x"
$x > junk.$n.tmp
which dos2unix >/dev/null
if [ $? -eq 0 ]
then
  dos2unix  junk.$n.tmp 2>/dev/null
fi
grep 'Version-readelfobj:' <junk.$n.tmp >junk.$n.out
if [ $? -eq 0 ]
then
  #Version string found
  exit 0
fi
echo "FAIL $srcdir/$n.sh found no version string in junk.$n.tmp"
exit 1


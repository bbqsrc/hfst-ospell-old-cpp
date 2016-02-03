#!/bin/sh

oldpwd=`pwd`

ziptarget=`realpath $1`
zipname=`basename $ziptarget`

mode=

if [ "$2" = "no-errmodel" ]; then
  mode=$2
  acceptor=$3
  xmlfile=$4
else
  acceptor=$2
  errmodel=$3
  xmlfile=$4
fi

tmpdir=`mktemp -d`

cp -f $acceptor $tmpdir/acceptor.default.hfst || exit 1
if ! [ "$mode" = "no-errmodel" ]; then
  cp -f $errmodel $tmpdir/errmodel.default.hfst || exit 1
fi
cp -f $xmlfile $tmpdir/index.xml || exit 1

cd $tmpdir
zip -1q $zipname *
mv $zipname $ziptarget
cd $oldpwd
rm -rf $tmpdir

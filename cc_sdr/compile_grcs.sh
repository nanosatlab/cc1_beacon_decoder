#!/bin/sh

for file in apps/hier_blocks/*.grc
do grcc $file
done

for file in apps/*.grc
do grcc $file -d apps
done


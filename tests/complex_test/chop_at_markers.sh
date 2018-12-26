#!/bin/bash -xe

mv $1 tmp
cat tmp | \
   grep -A100000 '^00000001 ' | \
   grep -B100000 '^000000fc ' | \
   grep -v '^40000000 ' > $1
rm -f tmp


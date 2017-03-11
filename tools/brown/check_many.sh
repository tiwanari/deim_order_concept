#!/bin/sh

require_params=2
if [ $# -lt $require_params ]; then
    echo "need at least $require_params arguments"
    echo "Usage: $0 input_file concept"
    echo "e.g. $0 ./feed-2009_1_10.c1024 list_file"
    exit 1
fi

input_file=$1
list_file=$2
for concept in `cat $list_file`
do
    result=`cat $input_file | grep $'\t'${concept}$'\t'`

    set -- $result

    cluster=$1
    echo "$cluster\t$concept"

    # cat $input_file | egrep ^$1$'\t' | less
done

#!/bin/bash

src="../../queries12/gold/human"
dst="./humans"


for d in `cat ./golds/lists.txt`
do
    mkdir -p ${dst}/${d}

    for f in `ls ${src}/${d}`
    do
        sed -e '223,225d' ${src}/${d}/${f}          \
| sed -e 'y/ＡＢＣＤＥＦＧＨＩＪＫＬＭＮＯＰＱＲＳＴＵＶＷＸＹＺａｂｃｄｅｆｇｈｉｊｋｌｍｎｏｐｑｒｓｔｕｖｗｘｙｚ０１２３４５６７８９/ABCDEFGHIJKLMNOPQRSTUVWXYZabcdefghijklmnopqrstuvwxyz0123456789/' \
        > ${dst}/${d}/${f}
    done
done

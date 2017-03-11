#!/bin/sh

src="../../queries12/gold/golds"
dst="./golds"


for d in `cat ./golds/lists.txt`
do
    mkdir -p ${dst}/${d}

    sed -e '149,150d' ${src}/${d}/gold.csv          \
| sed -e 'y/ＡＢＣＤＥＦＧＨＩＪＫＬＭＮＯＰＱＲＳＴＵＶＷＸＹＺａｂｃｄｅｆｇｈｉｊｋｌｍｎｏｐｑｒｓｔｕｖｗｘｙｚ０１２３４５６７８９/ABCDEFGHIJKLMNOPQRSTUVWXYZabcdefghijklmnopqrstuvwxyz0123456789/' \
    > ${dst}/${d}/gold.csv

    sed -e '75d'      ${src}/${d}/gold.txt          \
| sed -e 'y/ＡＢＣＤＥＦＧＨＩＪＫＬＭＮＯＰＱＲＳＴＵＶＷＸＹＺａｂｃｄｅｆｇｈｉｊｋｌｍｎｏｐｑｒｓｔｕｖｗｘｙｚ０１２３４５６７８９/ABCDEFGHIJKLMNOPQRSTUVWXYZabcdefghijklmnopqrstuvwxyz0123456789/' \
    > ${dst}/${d}/gold.txt

    sed -e '75d'      ${src}/${d}/for_plot.gpdata   \
| sed -e 'y/ＡＢＣＤＥＦＧＨＩＪＫＬＭＮＯＰＱＲＳＴＵＶＷＸＹＺａｂｃｄｅｆｇｈｉｊｋｌｍｎｏｐｑｒｓｔｕｖｗｘｙｚ０１２３４５６７８９/ABCDEFGHIJKLMNOPQRSTUVWXYZabcdefghijklmnopqrstuvwxyz0123456789/' \
    > ${dst}/${d}/for_plot.gpdata

    cat ${src}/${d}/invalid_data_lists.txt | grep -v 'query75:' > ${dst}/${d}/invalid_data_lists.txt
done

#!/bin/bash

set -e

REPORT="python ../../scripts/report.py"

for file in *.metadata
do
    name=`basename $file .metadata`
    CSV=${name}.csv
    RST=${name}.rst
    HTML=${name}.html
    $REPORT --csv="${CSV}" --output="${RST}"
    if type rst2html > /dev/null
    then
        rst2html "${RST}" > "${HTML}"
        echo "${HTML} was created"
    fi
done

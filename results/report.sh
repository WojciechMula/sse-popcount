#!/bin/bash

set -e

if [[ $1 == "" ]]; then
    echo "Usage: ./report.sh subdirectory"
    exit 1
fi
DIR="$1"

if [[ ! -d ${DIR} ]]; then
    echo "'${DIR}' is not not a directory"
    exit 1
fi

SCRIPT=scripts/report.py
if [[ ! -f $SCRIPT ]]; then
    SCRIPT="../${SCRIPT}"
fi
if [[ ! -f $SCRIPT ]]; then
    SCRIPT="../${SCRIPT}"
fi
if [[ ! -f $SCRIPT ]]; then
    echo "Could not locate 'report.py'"
    exit 1
fi

REPORT="python2 ${SCRIPT}"

for file in ${DIR}/*.metadata
do
    name=${DIR}/`basename $file .metadata`
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

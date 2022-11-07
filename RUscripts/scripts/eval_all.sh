#!/bin/bash

set -e

HARU_VENV=~/haru/
MINIMAP2=minimap2
SLOW5TOOLS=slow5tools
REF=dataset/fasta/nCoV-2019.reference.fasta
THREADS=4

PREFIX=/mnt/d/genome/data/FLFL031920/FLFL031920/20200330_0614_X3_ACW749_4b9280d1/

mkdir -p ${PREFIX}/ref_paf
mkdir -p ${PREFIX}/my_paf
mkdir -p ${PREFIX}/results

source ${HARU_VENV}/bin/activate

for file in ${PREFIX}/slow5_pass//*.blow5
do
    FASTQ=${PREFIX}/fastq_pass/${file%.*}.fastq
    BLOW5=${PREFIX}/slow5_pass/${file%.*}
    REF_PAF=${PREFIX}/ref_paf/${file%.*}.paf
    MY_PAF=${PREFIX}/my_paf/${file%.*}.paf
    ${MINIMAP2} -cx map-ont ${REF} ${FASTQ} --secondary=no -t ${THREADS} > ${REF_PAF}
    python3 OfflineReadUntil.py -f ${REF} -t MN908947.3:10000-15000 -p ${THREADS} -m models/r9.4_450bps.nucleotide.6mer.template.model -w ${BLOW5} -o RUgOUT -L 3000 > ${MY_PAF}
    uncalled pafstats -r ${REF_PAF} ${MY_PAF} > ${PREFIX}/results/${file%.*}.txt
done

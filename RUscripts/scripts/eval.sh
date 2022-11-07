#!/bin/bash

set -e

HARU_VENV=~/haru/
MINIMAP2=minimap2
SLOW5TOOLS=slow5tools
FASTQ=dataset/ncov-testset/batch_0.fastq
REF=dataset/fasta/nCoV-2019.reference.fasta
BLOW5=dataset/ncov-testset/slow5/
THREADS=4
REF_PAF=dataset/ncov-testset/slow5/batch_0.paf
MY_PAF=dataset/ncov-testset/slow5/my.paf

source ${HARU_VENV}/bin/activate

${MINIMAP2} -cx map-ont ${REF} ${FASTQ} --secondary=no -t ${THREADS} > ${REF_PAF}
python3 OfflineReadUntil.py -f ${REF} -t MN908947.3:10000-15000 -p ${THREADS} -m models/r9.4_450bps.nucleotide.6mer.template.model -w ${BLOW5} -o RUgOUT -L 3000 > ${MY_PAF}
uncalled pafstats -r ${REF_PAF} ${MY_PAF}

# HARU
Hardware Accelerated Read Until R9 branch


# installation

```
python3 -m venv haru
source haru/bin/activate
pip3 install --upgrade pip
pip3 install numpy==1.18.0 pyslow5 biopython==1.69  scikit-learn==0.20.0  scipy==1.4.0 six==1.16.0 Cython
git clone https://github.com/hasindu2008/HARU -b r9_slow5 && cd HARU
python3 setup.py install
```

# Run

```sh
cd RUscripts
python3 OfflineReadUntil.py -f nCoV-2019.reference.fasta -t MN908947.3:10000-15000 -p 4 -m models/r9.4_450bps.nucleotide.6mer.template.model -w ncov-testset/slow5 -o RUgOUT -L 3000
```



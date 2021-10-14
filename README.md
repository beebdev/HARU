# HARU
Hardware Accelerated Read Until


# simulation

# installation

For python 3.5 on Ubuntu 16:

```
python3 -m venv haru
source haru/bin/activate
pip3 install -r RUscripts/requirements_python3.5_ubuntu16.txt
```

# Run

```sh
cd RUscripts
python3 OfflineReadUntil.py -f ./J02459.fasta -t J02459:10000-15000 -p 4  -m models/template_r7.3_e6_70bps_6mer_6.model -w ./RUtestset/ -o RUgOUT -L 3000
```



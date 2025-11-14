# SKINNY block cipher
I use C to implemenmt procedure of **SKINNY-128-384** block cipher and **SKINNY-tk3** hash function which are described in detail in this [document](SKINNY-spec-round2.pdf)

Run below code lines in LINUX-environment
```sh
make
.\driver_skinny // run SKINNY-128-384 KAT tests
.\driver_tk3 // run SKINNY-tk3 KAT tests
```
to run KATs (known answer tests).



[![GitHub license](https://img.shields.io/badge/license-BSD-blue.svg)](https://raw.githubusercontent.com/utamaro/curl_trial/master/LICENSE)


# Speed Trials for [Curl](https://github.com/iotaledger/ccurl)

## Overview

These programs here are speed trials for [ccurl](https://github.com/iotaledger/ccurl), which is used as PoW(Proof of Work) in [IOTA](https://iotatoken.com/).
These use same algorithm as ccurl except using thread , but with some speed tricks.

These programs executes a hash test ,PoW test for MinWeightMagnitude=15 in main routine and measure hash speed(kH/s).

## Requirements

This requires

* gcc
* Intel SSE2 for pow_sse2.c


## Installation
```
    $ git clone https://github.com/utamaro/curl_trials
	$ gcc -o pow_normal pow_normal.c -O3 -Wall
	$ ./pow_normal 
	$ gcc -o pow_sse2 pow_sse2 -O3 -Wall -msse2
	$ ./pow_sse2 
```

## Performance

Using the following test environment...
```
* Compiler:  6.3.1 20170109 (GCC)
* Kernel: Linux 4.9.11-1-ARCH #1 SMP PREEMPT Sun Feb 19 13:45:52 UTC 2017 x86_64 GNU/Linux
* CPU:  Celeron(R) CPU G1840 @ 2.80GHz 
* Memory: 4 GB
```

This machine gets about `400kH/s` on original ccurl on CPU PoW.
By using '-O3' gcc option , it will be about `1.2MH/s`.

and it gets about `320kH/s` on GPU PoW.
My machine is very very cheap one(about $30), and GPU is one which is embeded in  CPU.
so it would be much faster if you use good PC or you have a good external GPU.

### Normal Program (pow_normal.c)
This program is parallelized to 64 hashes as ccurl without threads. But was optimized by checking results of `gprof`.

```
$ time ./pow_normal 
count=24688640 kHash/sec: 1652 
PoWed hash is XPAITBTKUNSGAMYDJJQKSTSGSDIED9XVLDINYRRXDNNDBJYRH9NQPXFJFETIEMHOFZUZZQOLNTTJ99999
OK

real	0m14.942s
user	0m14.700s
sys	0m0.033s
```

about `1.6MH/s`.


### SSE2 (pow_sse2.c)
This program uses SSE2 in intel CPU by intrinsic and is parallelized to 128 hashes.

```
$ time ./pow_sse2
count=30755712 kHash/sec: 2677 
PoWed hash is RQIVYDWVCJ9EVMMZHCZT9IQVRQHXGWWOVUEHEFIZULGMAMGVQCEJRP9U9IWDVBB9NTBDCGY9DGKF99999
OK

real	0m11.489s
user	0m11.320s
sys	0m0.040s
```

about `2.7MH/s`, x1.7 of the normal program.


# Contribution
Improvements to the codebase and pull requests are encouraged.



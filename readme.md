# DevRandom

A Windows Kernel driver that provides pseduo random numbers similar to /dev/random.

## Disclaimer

This is just for fun. Do not use this for anything important. I am not responsible for anything you chose to (mis)use this for.

## Design

This driver uses WDF in KMDF mode and is based on the general/echo/kmdf driver from the official Microsoft docs. There is a single
queue that provides only a reader endpoint and is fully synchronous. Randomness is provided by the Mitchell-Moore algorithm from 
Donald Knuth. This is fastest algorithm I could find that doesn't require 128-bit types which MSVC does not yet support.

## Benchmarks

As a super lame and unofficial baseline, this is compared against Python's random module just to establish a time cost. No analysis 
on the quality of randomness is performed. See tools/benchmarks.py for the test harness that generates these results (in seconds).

    python.random: count=100, time=0.771008
    devrandom    : count=100, time=1.2402800999999999
    python.random: count=1000, time=3.4773733
    devrandom    : count=1000, time=10.5952621
    python.random: count=10000, time=36.1174423
    devrandom    : count=10000, time=101.8268606
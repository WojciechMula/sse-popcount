========================================================================
                           SIMD popcount
========================================================================

Sample programs for my article http://0x80.pl/articles/sse-popcount.html

Directory **original** contains code from 2008 --- it is 32-bit
and GCC-centric. The main directory contains fresh code, written
with intrinsicts and tested on 64-bit machine.

As usual type ``make`` to compile programs, then you can invoke:

* ``verify`` --- program tests if SIMD version counts properly.
* ``speed`` --- program tests different implementations of popcount
  procedure; please read help to find all options (run the program
  without arguments).

========================================================================
                           SIMD popcount
========================================================================

Sample programs for my article http://0x80.pl/articles/sse-popcount.html

.. image:: https://travis-ci.org/WojciechMula/sse-popcount.svg?branch=master
    :target: https://travis-ci.org/WojciechMula/sse-popcount

Paper
------------------------------------------------------------------------

Daniel Lemire, Nathan Kurz and I published an article
`Faster Population Counts using AVX2 Instructions`__.

__ https://arxiv.org/abs/1611.07612


Introduction
------------------------------------------------------------------------

Subdirectory **original** contains code from 2008 --- it is 32-bit
and GCC-centric. The **root directory** contains fresh C++11 code,
written with intrinsics and tested on 64-bit machine.

As usual type ``make`` to compile programs, then you can invoke:

* ``verify``/``verify_avx``/``verify_avx2`` --- program tests if all
  non-lookup implementations counts bits properly.
* ``speed``/``speed_avx``/``speed_avx2`` --- program tests different
  implementations of popcount procedure; please read help to find all
  options (run the program without arguments).

You can also run ``make run``, ``make run_avx`` or ``make run_avx2``
to run ``speed`` for all available implementations compiled for
given architecture (more or less: ``-msse``, ``-mavx``, ``-mavx2``).


Available implementations in the new version
------------------------------------------------------------------------

+---------------------------------------+------------------------------------------------------------------+
| procedure                             | description                                                      |
+=======================================+==================================================================+
| lookup-8                              | lookup in std::uint8_t[256] LUT                                  |
+---------------------------------------+------------------------------------------------------------------+
| lookup-64                             | lookup in std::uint64_t[256] LUT                                 |
+---------------------------------------+------------------------------------------------------------------+
| bit-parallel                          | naive bit parallel method                                        |
+---------------------------------------+------------------------------------------------------------------+
| bit-parallel-optimized                | a bit better bit parallel                                        |
+---------------------------------------+------------------------------------------------------------------+
| bit-parallel-mul                      | bit-parallel with fewer instructions                             |
+---------------------------------------+------------------------------------------------------------------+
| harley-seal                           | Harley-Seal popcount (4th iteration)                             |
+---------------------------------------+------------------------------------------------------------------+
| sse-bit-parallel                      | SSE implementation of bit-parallel-optimized (unrolled)          |
+---------------------------------------+------------------------------------------------------------------+
| sse-bit-parallel-original             | SSE implementation of bit-parallel-optimized                     |
+---------------------------------------+------------------------------------------------------------------+
| sse-bit-parallel-better               | SSE implementation of bit-parallel with fewer instructions       |
+---------------------------------------+------------------------------------------------------------------+
| sse-harley-seal                       | SSE implementation of Harley-Seal                                |
+---------------------------------------+------------------------------------------------------------------+
| sse-lookup                            | SSSE3 variant using pshufb instruction (unrolled)                |
+---------------------------------------+------------------------------------------------------------------+
| sse-lookup-original                   | SSSE3 variant using pshufb instruction                           |
+---------------------------------------+------------------------------------------------------------------+
| avx2-lookup                           | AVX2 variant using pshufb instruction (unrolled)                 |
+---------------------------------------+------------------------------------------------------------------+
| avx2-lookup-original                  | AVX2 variant using pshufb instruction                            |
+---------------------------------------+------------------------------------------------------------------+
| avx2-harley-seal                      | AVX2 implementation of Harley-Seal                               |
+---------------------------------------+------------------------------------------------------------------+
| cpu                                   | CPU instruction popcnt (64-bit variant)                          |
+---------------------------------------+------------------------------------------------------------------+
| sse-cpu                               | load data with SSE, then count bits using popcnt                 |
+---------------------------------------+------------------------------------------------------------------+
| avx2-cpu                              | load data with AVX2, then count bits using popcnt                |
+---------------------------------------+------------------------------------------------------------------+
| builtin-popcnt                        | builtin for popcnt                                               |
+---------------------------------------+------------------------------------------------------------------+
| builtin-popcnt32                      | builtin for popcnt (32-bit variant)                              |
+---------------------------------------+------------------------------------------------------------------+
| builtin-popcnt-unrolled               | unrolled builtin-popcnt                                          |
+---------------------------------------+------------------------------------------------------------------+
| builtin-popcnt-unrolled32             | unrolled builtin-popcnt32                                        |
+---------------------------------------+------------------------------------------------------------------+
| builtin-popcnt-unrolled-errata        | unrolled builtin-popcnt avoiding false-dependency                |
+---------------------------------------+------------------------------------------------------------------+
| builtin-popcnt-unrolled-errata-manual | unrolled builtin-popcnt avoiding false-dependency (asembly code) |
+---------------------------------------+------------------------------------------------------------------+
| builtin-popcnt-movdq                  | builtin-popcnt where data is loaded via SSE registers            |
+---------------------------------------+------------------------------------------------------------------+
| builtin-popcnt-movdq-unrolled         | builtin-popcnt-movdq unrolled                                    |
+---------------------------------------+------------------------------------------------------------------+
| builtin-popcnt-movdq-unrolled_manual  | builtin-popcnt-movdq unrolled (assembly code)                    |
+---------------------------------------+------------------------------------------------------------------+


Acknowledgments
------------------------------------------------------------------------

* **Kim Walisch** (@kimwalisch) wrote Harley-Seal scalar implementation.
* **Simon Lindholm** (@simonlindholm) added unrolled versions of procedures.
* **Dan Luu** (@danluu) agreed to include his procedures (``builint-*``)
  into this project. More details in Dan's article `Hand coded assembly
  beats intrinsics in speed and simplicity`__

__ http://danluu.com/assembly-intrinsics/

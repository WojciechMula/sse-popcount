========================================================================
                           SIMD popcount
========================================================================

Sample programs for my article http://0x80.pl/articles/sse-popcount.html

Introduction
------------------------------------------------------------------------

Subdirectory **original** contains code from 2008 --- it is 32-bit
and GCC-centric. The **root directory** contains fresh C++11 code,
written with intrinsics and tested on 64-bit machine.

As usual type ``make`` to compile programs, then you can invoke:

* ``verify`` --- program tests if all non-lookup implementations counts
  bits properly.
* ``speed`` --- program tests different implementations of popcount
  procedure; please read help to find all options (run the program
  without arguments).

You can also run ``make run`` to run ``speed`` for all available
implementations.


Available implementations in new version
------------------------------------------------------------------------

There are following procedures:

* ``sse-lookup`` --- pshufb version described in the article.
* ``lookup-8`` --- lookup table of type ``std::uint8_t[256]``.
* ``lookup-64`` --- lookup table of type ``std::uint64_t[256]``,
  LUT is 8 times larger, but we avoid extending 8 to 64 bits.
* ``bit-parallel`` --- well know bit parallel method.
* ``bit-parallel-optimized`` --- in this variant counting
  on packed bytes is performed exactly in the same way
  as described in the article: this gives **50% speedup**.
* ``sse-bit-parallel`` --- SSE implementation of
  ``bit-parallel-optimized``


Results from core i5, program compiled by GCC 4.9.2::

    running lookup-8                      time =   0.676886 s
    running lookup-64                     time =   0.672696 s
    running bit-parallel                  time =   0.685250 s
    running bit-parallel-optimized        time =   0.451518 s
    running sse-bit-parallel              time =   0.210530 s
    running sse-lookup                    time =   0.140623 s

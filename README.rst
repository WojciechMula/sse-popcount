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

* ``verify``/``verify_avx2`` --- program tests if all non-lookup
  implementations counts bits properly.
* ``speed``/``speed_avx2`` --- program tests different implementations
  of popcount procedure; please read help to find all options
  (run the program without arguments).

You can also run ``make run`` and ``make run_avx2`` to run ``speed``
for all available implementations.


Available implementations in the new version
------------------------------------------------------------------------

* ``sse-lookup`` --- SSE pshufb version described in the article.
* ``avx2-lookup`` --- AVX2 pshufb version described in the article.
* ``lookup-8`` --- lookup table of type ``std::uint8_t[256]``.
* ``lookup-64`` --- lookup table of type ``std::uint64_t[256]``.
  LUT is 8 times larger, but we avoid extending 8 to 64 bits.
* ``bit-parallel`` --- well know bit parallel method.
* ``bit-parallel-optimized`` --- in this variant counting
  on packed bytes is performed exactly in the same way
  as described in the article: this gives **50% speedup**.
* ``sse-bit-parallel`` --- SSE implementation of
  ``bit-parallel-optimized``.
* ``cpu`` --- use ``popcnt`` instructions


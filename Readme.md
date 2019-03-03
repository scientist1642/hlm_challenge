## Note

Since file reading speed is crucial in this task 2 versions of solution is included.

Difference in code is tiny-tiny, one uses `mmap` that maps the file into memory and another uses plain `read` (default solution).

In my tests `read` works faster on the first run (multicore machines), but `mmap` is faster on subsequent runs as it utilizes RAM and doesn't even need to copy from cache.

I think, if benchmarking is done properly (without giving the app a chance to use a cache), the default reading method should outperform the other one. 

---
Progress chart, just for fun:
![GitHub Logo](https://i.ibb.co/jHjcMTF/bench.png)

import random
from functools import partial
from timeit import timeit as ti

try:
  f = open(r"\\.\DevRandom", 'rb')

  def m1(count):
    random.randbytes(count)

  def m2(count):
    f.read(count)

  for i in [100, 1_000, 10_000]:
    print(f"python.random: count={i}, time={ti(partial(m1, i))}")
    print(f"devrandom    : count={i}, time={ti(partial(m2, i))}")

finally:
  f.close()

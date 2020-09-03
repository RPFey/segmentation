# README

## Build

Implementation of the segmentation algorithm described in:

Efficient Graph-Based Image Segmentation
Pedro F. Felzenszwalb and Daniel P. Huttenlocher
International Journal of Computer Vision, 59(2) September 2004.

http://cs.brown.edu/~pff/segment/

The program takes a color image (PPM format) and produces a segmentation
with a random color assigned to each region.

```bash
mkdir build
cmake ..
make
segment sigma k min input output
```

The parameters are: (see the paper for details)

sigma: Used to smooth the input image before segmenting it.
k: Value for the threshold function.
min: Minimum component size enforced by post-processing.
input: Input image.
output: Output image.

Typical parameters are sigma = 0.5, k = 500, min = 20.
Larger values for k result in larger components in the result.

## Analysis

`float` 和 `uchar` 计算出来结果相差很大。

代码中 `universe` 类中有图分割的算法。其中

```c++
typedef struct {
  int rank;
  int p;
  int size;
} uni_elt;
```

`rank` 代表集合中最高的点，用来存储集合中点的数量。其 `p` 指向自己，用来索引。从下面合并的算法可以看出，低 `rank` 节点向高 `rank` 合并，指向高 `rank` 。

```c++
void universe::join(int x, int y) {
  if (elts[x].rank > elts[y].rank) {
    elts[y].p = x;
    elts[x].size += elts[y].size;
  } else {
    elts[x].p = y;
    elts[y].size += elts[x].size;
    if (elts[x].rank == elts[y].rank)
      elts[y].rank++;
  }
  num--;
}
```
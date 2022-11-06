
<!-- README.md is generated from README.Rmd. Please edit that file -->

# metal

<!-- badges: start -->

[![Lifecycle:
experimental](https://img.shields.io/badge/lifecycle-experimental-orange.svg)](https://lifecycle.r-lib.org/articles/stages.html#experimental)
<!-- badges: end -->

The goal of metal is to provide R bindings to [Apple
Metal](https://developer.apple.com/metal/), which is Apple’s API for GPU
programming. If you have an M1 Mac, you’ve got a GPU built-in! This
package will let you write and compile code written in the [Metal
Shading
Language](https://developer.apple.com/metal/Metal-Shading-Language-Specification.pdf)
(or write a package that uses the header-only Metal C++ library). This
is totally experimental and really was just an excuse to work through
the [simple compute function
tutorial](https://developer.apple.com/documentation/metal/performing_calculations_on_a_gpu)
provided by Apple.

## Installation

You can install the development version of metal from
[GitHub](https://github.com/) with:

``` r
# install.packages("remotes")
remotes::install_github("paleolimbot/metal")
```

This will only work on MacOS. You’ll need an Intel Mac with a GPU or an
M1 Mac (which has a GPU built-in). If you can run this:

``` r
library(metal)
mtl_default_device()
#> <mtl_device>
#> - name: Apple M1
#> - description: <AGXG13GDevice: 0x1502d6000>
#>     name = Apple M1
```

…you’re good to go!

## Example

Compile Metal Shading Langauge code:

``` r
lib <- mtl_make_library("
  kernel void add_arrays(device const float* inA,
                         device const float* inB,
                         device float* result,
                         uint index [[thread_position_in_grid]]) {
    result[index] = inA[index] + inB[index];
  }
")

lib
#> <mtl_library[1]>
#> - add_arrays() <kernel>
```

Create some buffers and execute!

``` r
pipeline <- mtl_compute_pipeline(lib$add_arrays)
result <- mtl_buffer(123, buffer_type = "float")
in_a <- as_mtl_floats(1:123)
in_b <- as_mtl_floats(rep(2, 123))
mtl_compute_pipeline_execute(pipeline, 123, in_a, in_b, result)
mtl_buffer_convert(result)
#> <mtl_floats[123]>
#>   [1]   3   4   5   6   7   8   9  10  11  12  13  14  15  16  17  18  19  20
#>  [19]  21  22  23  24  25  26  27  28  29  30  31  32  33  34  35  36  37  38
#>  [37]  39  40  41  42  43  44  45  46  47  48  49  50  51  52  53  54  55  56
#>  [55]  57  58  59  60  61  62  63  64  65  66  67  68  69  70  71  72  73  74
#>  [73]  75  76  77  78  79  80  81  82  83  84  85  86  87  88  89  90  91  92
#>  [91]  93  94  95  96  97  98  99 100 101 102 103 104 105 106 107 108 109 110
#> [109] 111 112 113 114 115 116 117 118 119 120 121 122 123 124 125
```

## Using the bundled metal-cpp

The [metal-cpp C++ library](https://developer.apple.com/metal/cpp/) used
by this package is a header-only library that you can access by
`LinkingTo: metal` in another package, use `// [[Rcpp::depends(metal)]]`
in Rcpp code, or `[[cpp11::linking_to(metal)]]` in cpp11 code. You’ll
need to set the C++ standard to C++17 or later and possibly add
`-framework Metal` to your `PKG_LIBS` if you’re writing a package.

``` cpp
#include <cpp11.hpp>

#define NS_PRIVATE_IMPLEMENTATION
#define CA_PRIVATE_IMPLEMENTATION
#define MTL_PRIVATE_IMPLEMENTATION
#include <Metal/Metal.hpp>

[[cpp11::linking_to(cpp11)]]
[[cpp11::linking_to(metal)]]
[[cpp11::register]]
void print_default_device() {
  MTL::Device* device = MTL::CreateSystemDefaultDevice();
  Rprintf("%s", device->description()->utf8String());
}
```

``` r
print_default_device()
#> <AGXG13GDevice: 0x1502d6000>
#>     name = Apple M1
```


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
(or write a package that uses the header-only Metal C++ library).

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
#> - description: <AGXG13GDevice: 0x133ace800>
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
#> <AGXG13GDevice: 0x133ace800>
#>     name = Apple M1
```

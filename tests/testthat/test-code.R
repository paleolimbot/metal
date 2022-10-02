
test_that("mtl_default_device() works", {
  expect_s3_class(mtl_default_device(), "mtl_device")
})

test_that("device has a print method", {
  dev <- mtl_default_device()
  expect_output(expect_identical(print(dev), dev), "mtl_device")
})

test_that("mtl_make_library() errors for invalid arguments", {
  expect_error(mtl_make_library("", device = NULL), "not an mtl_device")
  expect_error(
    mtl_make_library("this is invalid metal code"),
    "this is invalid metal code"
  )
})

test_that("mtl_make_library() creates a library for valid code", {
  lib <- mtl_make_library("
    kernel void add_arrays(device const float* inA,
                           device const float* inB,
                           device float* result,
                           uint index [[thread_position_in_grid]]) {
      result[index] = inA[index] + inB[index];
    }
  ")

  expect_s3_class(lib, "mtl_library")
  expect_identical(names(lib), "add_arrays")
})

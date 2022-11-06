
test_that("mtl_default_device() works", {
  expect_s3_class(mtl_default_device(), "mtl_device")
})

test_that("device has a print method", {
  dev <- mtl_default_device()
  expect_output(expect_identical(print(dev), dev), "mtl_device")
})

test_that("mtl_make_library() errors for invalid arguments", {
  expect_error(mtl_make_library("", device = NULL), "Invalid input type")
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
  expect_s3_class(lib$add_arrays, "mtl_function")
  expect_identical(
    cpp_function_info(lib$add_arrays),
    list(name = "add_arrays", type = "kernel")
  )
  expect_identical(
    cpp_function_info(lib[[1]]),
    list(name = "add_arrays", type = "kernel")
  )

  expect_output(expect_identical(print(lib), lib), "mtl_library")

  # check that this function compiles
  expect_s3_class(mtl_compute_pipeline(lib$add_arrays), "mtl_compute_pipeline")
})

test_that("mtl_buffer() creates buffers", {
  buffer <- mtl_buffer(100)
  expect_identical(mtl_buffer_size(buffer), 100)
  expect_s3_class(buffer, "mtl_buffer")
  expect_s3_class(buffer, "mtl_buffer_uint8")

  mtl_copy_into_buffer(as.raw(1:100), buffer)
  expect_identical(
    mtl_buffer_slice(buffer),
    as.raw(1:100)
  )

  mtl_copy_into_buffer(
    as.raw(1:100),
    buffer,
    src_offset = 5,
    buffer_offset = 20,
    size = 15
  )

  expect_identical(
    mtl_buffer_slice(buffer, buffer_offset = 20, size = 15),
    as.raw(6:(6 + 15 - 1))
  )
})

test_that("mtl_buffer() functions error for invalid params", {
  buffer <- mtl_buffer(100)

  expect_error(
    mtl_copy_into_buffer(as.raw(1:100), buffer, src_offset = -1),
    "Invalid src_offset or buffer offset"
  )

  expect_error(
    mtl_copy_into_buffer(as.raw(1:100), buffer, buffer_offset = -1),
    "Invalid src_offset or buffer offset"
  )

  expect_error(
    mtl_copy_into_buffer(as.raw(1:100), buffer, src_offset = -1),
    "Invalid src_offset or buffer offset"
  )

  expect_error(
    mtl_copy_into_buffer(as.raw(1:100), buffer, src_offset = 10, size = 91),
    "Vector not long enough for specified arguments"
  )

  expect_error(
    mtl_copy_into_buffer(as.raw(1:100), buffer, buffer_offset = 10, size = 91),
    "Buffer not long enough for specified arguments"
  )

  expect_error(
    mtl_copy_into_buffer(as.integer(1:5), buffer, size = 3),
    "Length must be a multiple of vector element size"
  )

  expect_error(
    mtl_buffer_slice(buffer, buffer_offset = -1),
    "Invalid buffer offset argument"
  )

  expect_error(
    mtl_buffer_slice(buffer, buffer_offset = 1, size = 100),
    "Buffer not long enough for specified arguments"
  )

  expect_error(
    mtl_buffer_slice(buffer, environment()),
    "Vector type not supported for ptype"
  )

  expect_error(
    mtl_buffer_slice(buffer, integer(), size = 3),
    "Length must be a multiple of vector element size"
  )
})

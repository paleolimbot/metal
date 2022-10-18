
test_that("mtl_floats() constructor works", {
  expect_s3_class(mtl_floats(), "mtl_floats")
  expect_length(mtl_floats(101), 101)
  expect_true(all(unclass(mtl_floats(100, fill = 0)) == 0))
  expect_identical(format(as_mtl_floats(1:10)), format(as.double(1:10)))
})

test_that("as_mtl_floats() works", {
  expect_identical(as.logical(as_mtl_floats(c(NA, TRUE, FALSE))), c(NA, TRUE, FALSE))
  expect_identical(as.integer(as_mtl_floats(c(NA, 1:10))), c(NA, 1:10))
  expect_identical(as.double(as_mtl_floats(c(NA, 1:10))), as.double(c(NA, 1:10)))

  expect_error(as_mtl_floats(structure(integer(), class = "custom_int")), "Can't convert")
  expect_error(as_mtl_floats(complex()), "Can't convert")
})

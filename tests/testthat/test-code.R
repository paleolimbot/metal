
test_that("default_device() works", {
  expect_s3_class(default_device(), "metalme_device")
})

test_that("device has a print method", {
  dev <- default_device()
  expect_output(expect_identical(print(dev), dev), "metalme_device")
})

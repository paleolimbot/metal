
#' Create float vectors
#'
#' @param x An object to convert to an [mtl_floats()].
#' @param size The size of the buffer to allocate
#' @param fill An optional value with which each element should be initialized
#' @param ... Passed to S3 methods
#'
#' @importFrom vctrs vec_cast
#' @return An vctr of class mtl_floats
#' @export
#'
#' @examples
#' mtl_floats(5, NaN)
#' as_mtl_floats(1:5)
#'
mtl_floats <- function(size = 0, fill = NA_real_) {
  cpp_floats(size, fill)
}

#' @rdname mtl_floats
#' @export
as_mtl_floats <- function(x, ...) {
  UseMethod("as_mtl_floats")
}

#' @export
as_mtl_floats.mtl_floats <- function(x, ...) {
  x
}

#' @export
as_mtl_floats.default <- function(x, ...) {
  cpp_as_floats(x) %||% cpp_as_floats(vctrs::vec_cast(x, double()))
}

#' @export
as.logical.mtl_floats <- function(x, ...) {
  cpp_from_floats_lgl(x)
}

#' @export
as.integer.mtl_floats <- function(x, ...) {
  cpp_from_floats_int(x)
}

#' @export
as.double.mtl_floats <- function(x, ...) {
  cpp_from_floats_dbl(x)
}

#' @export
format.mtl_floats <- function(x, ...) {
  format(cpp_from_floats_dbl(x), ...)
}

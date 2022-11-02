
#' Get the default metal device
#'
#' @return An external pointer of class metal_device
#' @export
#'
#' @examples
#' mtl_default_device()
#'
mtl_default_device <- function() {
  cpp_default_device()
}

#' @export
print.mtl_device <- function(x, ...) {
  info <- cpp_device_info(x)
  cat(
    sprintf(
      "<mtl_device>\n- name: %s\n- description: %s\n",
      info$name,
      info$description
    )
  )
  invisible(x)
}


#' Create a metal function library
#'
#' @param code Code in the metal shading language
#' @param device A [mtl_device][mtl_default_device]
#'
#' @return An external pointer of class mtl_library
#' @export
#'
#' @examples
#' mtl_make_library("
#'   kernel void add_arrays(device const float* inA,
#'                          device const float* inB,
#'                          device float* result,
#'                          uint index [[thread_position_in_grid]]) {
#'     result[index] = inA[index] + inB[index];
#'   }
#' ")
#'
mtl_make_library <- function(code, device = mtl_default_device()) {
  cpp_make_library(device, code)
}

#' @export
names.mtl_library <- function(x) {
  cpp_library_function_names(x)
}

#' @export
length.mtl_library <- function(x) {
  length(cpp_library_function_names(x))
}

#' @export
`[[.mtl_library` <- function(x, i) {
  if (is.numeric(i)) {
    cpp_library_function(x, names(x)[i])
  } else {
    cpp_library_function(x, i)
  }
}

#' @export
`$.mtl_library` <- function(x, i) {
  cpp_library_function(x, i)
}

#' @export
print.mtl_library <- function(x, ...) {
  funs <- names(x)

  cat(sprintf("<mtl_library[%d]>\n", length(funs)))
  for (fun in funs) {
    fun_info <- cpp_function_info(x[[fun]])
    cat(sprintf("- %s() <%s>\n", fun_info$name, fun_info$type))
  }

  invisible(x)
}


#' Create buffers
#'
#' @param size The size of the buffer in bytes
#' @inheritParams mtl_make_library
#' @param ... Passed to S3 methods
#'
#' @return An object of class 'mtl_buffer'
#' @export
#'
#' @examples
#' mtl_buffer(1024)
#'
mtl_buffer <- function(size, device = mtl_default_device()) {
  cpp_buffer(device, size)
}

mtl_buffer_size <- function(buffer) {
  cpp_buffer_size(buffer)
}

mtl_copy_into_buffer <- function(x, buffer, src_offset = 0L, buffer_offset = 0L,
                                 size = NULL) {
  if (is.null(size)) {
    size <- switch(
      typeof(x),
      "integer" = ,
      "logical" = 4L * length(x),
      "double" = 8L * length(x),
      "raw" = length(x)
    )
  }

  cpp_bufer_copy_into(buffer, x, src_offset, buffer_offset, size)
}

mtl_buffer_slice <- function(buffer, x = raw(), buffer_offset = 0L,
                             size = mtl_buffer_size()) {
  cpp_buffer_copy_from(buffer, x, buffer_offset, size)
}


mtl_compute_pipeline <- function(func) {
  cpp_compute_pipeline(func)
}


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


#' Create Metal buffers
#'
#' Allocates mutable buffers using Metal's allocation functions.
#' These buffers have specific alignment that allow them to be shared
#' between the GPU and CPU.
#'
#' @param buffer An [mtl_buffer()]
#' @param x An object to convert to an [mtl_buffer()].
#' @param size A size of the buffer or part of the buffer in bytes
#' @param src_offset,buffer_offset Offsets into the sour
#' @inheritParams mtl_make_library
#' @param ... Passed to S3 methods
#'
#' @return An object of class 'mtl_buffer'
#' @export
#'
#' @examples
#' mtl_buffer(1024)
#'
mtl_buffer <- function(size, device = mtl_default_device(),
                       buffer_type = c("uint8", "float", "int32", "double")) {
  buffer_type <- match.arg(buffer_type)
  buffer <- cpp_buffer(device, size)
  class(buffer) <- c(paste0("mtl_buffer_", buffer_type), class(buffer))
  buffer
}

#' @rdname mtl_buffer
#' @export
as_mtl_buffer <- function(x, ...) {
  UseMethod("as_mtl_buffer")
}

#' @rdname mtl_buffer
#' @export
as_mtl_buffer.integer <- function(x, ...) {
  buffer <- mtl_buffer(length(x) * 4L, buffer_type = "int32")
  mtl_copy_into_buffer(x, buffer)
  buffer
}

#' @rdname mtl_buffer
#' @export
as_mtl_buffer.double <- function(x, ...) {
  buffer <- mtl_buffer(length(x) * 8L, buffer_type = "double")
  mtl_copy_into_buffer(x, buffer)
  buffer
}

#' @rdname mtl_buffer
#' @export
as_mtl_buffer.mtl_floats <- function(x, ...) {
  buffer <- mtl_buffer(length(x) * 4L, buffer_type = "float")
  mtl_copy_into_buffer(x, buffer)
  buffer
}

#' @rdname mtl_buffer
#' @export
as_mtl_buffer.raw <- function(x, ...) {
  buffer <- mtl_buffer(length(x), buffer_type = "uint8")
  mtl_copy_into_buffer(x, buffer)
  buffer
}

#' @rdname mtl_buffer
#' @export
mtl_buffer_convert <- function(buffer, start = 0L, length = NULL) {
  switch(
    class(buffer)[1],
    "mtl_buffer_float" = {
      element_size <- 4L
      ptype <- mtl_floats()
    },
    "mtl_buffer_int32" = {
      element_size <- 4L
      ptype <- integer()
    },
    "mtl_buffer_double" = {
      element_size <- 8L
      ptype <- double()
    },
    {
      element_size <- 1L
      ptype <- raw()
    }
  )

  if (is.null(length)) {
    length <- mtl_buffer_size(buffer) / element_size
  }

  start_raw <- start * element_size
  length_raw <- length * element_size
  mtl_buffer_slice(buffer, ptype, start_raw, length_raw)
}

#' @rdname mtl_buffer
#' @export
mtl_buffer_size <- function(buffer) {
  cpp_buffer_size(buffer)
}

#' @rdname mtl_buffer
#' @export
mtl_copy_into_buffer <- function(x, buffer, src_offset = 0L, buffer_offset = 0L,
                                 size = NULL) {
  if (is.null(size)) {
    size <- switch(
      typeof(x),
      "integer" = ,
      "logical" = 4L * length(x),
      "double" = 8L * length(x),
      "raw" = length(x),
      stop("Can't guess size of `x`")
    )
  }

  cpp_buffer_copy_from(x, buffer, src_offset, buffer_offset, size)
}

#' @rdname mtl_buffer
#' @export
mtl_buffer_slice <- function(buffer, x = raw(), buffer_offset = 0L,
                             size = mtl_buffer_size(buffer)) {
  result <- cpp_buffer_copy_into(buffer, x, buffer_offset, size)
  class(result) <- class(x)
  result
}


mtl_compute_pipeline <- function(func) {
  cpp_compute_pipeline(func)
}

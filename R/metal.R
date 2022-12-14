
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

#' Compile and execute compute functions
#'
#' @param func An mtl_function
#' @param length The array length to execute across (used to create the grid
#'   of threads)
#' @inheritParams mtl_make_library
#' @param pipeline A pipeline created with [mtl_compute_pipeline()]
#' @param ... Arguments (currently all [mtl_buffer()]s) or objects that
#'   will be coerced to them.
#'
#' @return
#'   - `mtl_compute_pipeline()` returns an mtl_compute_pipline object representing
#'     a compiled version of the function for the device's GPU.
#'   - `mtl_compute_pipeline_execute()` returns nothing (usually the function
#'     populates an output buffer that is one of the arguments).
#' @export
#'
mtl_compute_pipeline <- function(func) {
  cpp_compute_pipeline(func)
}

#' @rdname mtl_compute_pipeline
#' @export
mtl_compute_pipeline_execute <- function(pipeline, length, ..., device = mtl_default_device()) {
  args <- lapply(list(...), as_mtl_buffer)
  queue <- cpp_command_queue(device)
  cpp_compute_pipeline_execute(pipeline, queue, args, length)
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
#' @param src_offset,buffer_offset Offsets into the buffer (zero-based)
#' @param buffer_type A logical type for the buffer
#' @param start,length A slice of the buffer to resolve into an R vectors
#' @inheritParams mtl_make_library
#' @param ... Passed to S3 methods
#'
#' @return An object of class 'mtl_buffer'
#' @export
#'
#' @examples
#' as_mtl_buffer(1:5)
#'
mtl_buffer <- function(length, device = mtl_default_device(),
                       buffer_type = c("uint8", "float", "int32", "double")) {
  buffer_type <- match.arg(buffer_type)
  size <- switch(
    buffer_type,
    "float" = ,
    "int32" = 4L * length,
    "double" = 8L * length,
    length
  )

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
as_mtl_buffer.mtl_buffer <- function(x, ...) {
  x
}

#' @rdname mtl_buffer
#' @export
as_mtl_buffer.integer <- function(x, ...) {
  buffer <- mtl_buffer(length(x), buffer_type = "int32")
  mtl_copy_into_buffer(x, buffer)
  buffer
}

#' @rdname mtl_buffer
#' @export
as_mtl_buffer.logical <- function(x, ...) {
  buffer <- mtl_buffer(length(x), buffer_type = "int32")
  mtl_copy_into_buffer(x, buffer)
  buffer
}

#' @rdname mtl_buffer
#' @export
as_mtl_buffer.double <- function(x, ...) {
  buffer <- mtl_buffer(length(x), buffer_type = "double")
  mtl_copy_into_buffer(x, buffer)
  buffer
}

#' @rdname mtl_buffer
#' @export
as_mtl_buffer.mtl_floats <- function(x, ...) {
  buffer <- mtl_buffer(length(x), buffer_type = "float")
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

  size <- mtl_buffer_size(buffer)
  if (is.null(length)) {
    length <- size / element_size
  }

  start_raw <- start * element_size
  length_raw <- min(length * element_size, size - start_raw)
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

#' @export
print.mtl_buffer <- function(x, ...) {
  str(x, ...)
}

#' @importFrom utils str
#' @export
str.mtl_buffer <- function(object, ...) {
  cls <- class(object)[1]
  cat(sprintf("<%s[%s b]> ", cls, mtl_buffer_size(object)))
  proxy <- mtl_buffer_convert(object, length = 100L)
  str(proxy, ...)
  invisible(object)
}

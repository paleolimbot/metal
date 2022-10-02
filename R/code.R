
#' Get the default metal device
#'
#' @return An external pointer of class metalme_device
#' @export
#'
#' @examples
#' mtl_default_device()
#'
mtl_default_device <- function() {
  cpp_default_device()
}

#' @export
print.metalme_device <- function(x, ...) {
  info <- cpp_device_info(x)
  cat(
    sprintf(
      "<mtl_device>\nname: %s\ndescription: %s\n",
      info$name,
      info$description
    )
  )
  invisible(x)
}


mtl_make_library <- function(code, device = mtl_default_device()) {
  cpp_make_library(device, code)
}

#' @export
names.mtl_library <- function(x) {
  cpp_library_function_names(x)
}

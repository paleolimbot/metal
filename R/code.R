
#' Get the default metal device
#'
#' @return An external pointer of class metalme_device
#' @export
#'
#' @examples
#' default_device()
#'
default_device <- function() {
  cpp_default_device()
}

#' @export
print.metalme_device <- function(x, ...) {
  info <- cpp_device_info(x)
  cat(
    sprintf(
      "<metalme_device>\nname: %s\ndescription: %s\n",
      info$name,
      info$description
    )
  )
  invisible(x)
}

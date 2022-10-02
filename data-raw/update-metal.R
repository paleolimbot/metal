
curl::curl_download(
  "https://developer.apple.com/metal/cpp/files/metal-cpp_macOS12_iOS15.zip",
  "data-raw/metal.zip"
)
unzip("data-raw/metal.zip", exdir = "data-raw")
unlink("src/metal-cpp", recursive = T)
fs::dir_copy("data-raw/metal-cpp", "src")

unlink("data-raw/metal.zip")
unlink("data-raw/metal-cpp", recursive = TRUE)
unlink("data-raw/__MACOSX", recursive = TRUE)

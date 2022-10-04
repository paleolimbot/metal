
curl::curl_download(
  "https://developer.apple.com/metal/cpp/files/metal-cpp_macOS12_iOS15.zip",
  "data-raw/metal.zip"
)
unzip("data-raw/metal.zip", exdir = "data-raw")

withr::with_dir("data-raw/metal-cpp", {
  system("python3 SingleHeader/MakeSingleHeader.py Foundation/Foundation.hpp QuartzCore/QuartzCore.hpp Metal/Metal.hpp")
})

unlink("inst/include/Metal", recursive = TRUE)
dir.create("inst/include/Metal", recursive = TRUE)
file.copy("data-raw/metal-cpp/SingleHeader/Metal.hpp", "inst/include/Metal")

unlink("data-raw/metal.zip")
unlink("data-raw/metal-cpp", recursive = TRUE)
unlink("data-raw/__MACOSX", recursive = TRUE)

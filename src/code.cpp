#include <cpp11.hpp>
using namespace cpp11;

#include "Metal/Metal.hpp"

template <typename T>
class Owner {
 public:
  Owner() : ptr_(nullptr) {}
  Owner(T* ptr) : ptr_(ptr->retain()) {}

  void reset(T* ptr) {
    if (ptr_ != nullptr) {
      ptr_->release();
    }
    ptr_ = ptr->retain();
  }

  T* get() { return ptr_; }

  ~Owner() { reset(nullptr); }

 private:
  T* ptr_;
};

[[cpp11::register]] sexp cpp_default_device() {
  MTL::Device* default_device = MTL::CreateSystemDefaultDevice();
  if (default_device == nullptr) {
    stop("No default device found");
  }

  external_pointer<Owner<MTL::Device>> device_ptr(new Owner<MTL::Device>(default_device));
  sexp device_sexp = (SEXP)device_ptr;
  device_sexp.attr("class") = "mtl_device";
  return device_sexp;
}

[[cpp11::register]] list cpp_device_info(sexp device_sexp) {
  external_pointer<Owner<MTL::Device>> device_ptr(device_sexp);
  NS::String* name = device_ptr->get()->name();
  NS::String* description = device_ptr->get()->description();

  writable::list out = {as_sexp(name->utf8String()), as_sexp(description->utf8String())};
  out.names() = {"name", "description"};
  return out;
}

[[cpp11::register]] sexp cpp_make_library(sexp device_sexp, std::string code) {
  if (!Rf_inherits(device_sexp, "mtl_device")) {
    stop("`device` is not an mtl_device");
  }

  external_pointer<Owner<MTL::Device>> device_ptr(device_sexp);

  NS::Error* error = nullptr;
  NS::String* ns_code =
      NS::String::string(code.c_str(), NS::StringEncoding::UTF8StringEncoding);
  MTL::CompileOptions* options = MTL::CompileOptions::alloc();
  options->init();

  MTL::Library* library = device_ptr->get()->newLibrary(ns_code, options, &error);
  ns_code->release();
  options->release();

  if (error != nullptr) {
    const char* description = error->localizedDescription()->utf8String();
    library->release();
    stop("Error compiling metal code:\n%s", description);
  }

  external_pointer<Owner<MTL::Library>> library_ptr(new Owner<MTL::Library>(library));
  sexp library_sexp = (SEXP)library_ptr;
  library_sexp.attr("class") = "mtl_library";
  return library_sexp;
}

[[cpp11::register]] strings cpp_library_function_names(sexp library_sexp) {
  if (!Rf_inherits(library_sexp, "mtl_library")) {
    stop("`library` is not an mtl_library");
  }

  external_pointer<Owner<MTL::Library>> library_ptr(library_sexp);
  NS::Array* ns_names = library_ptr->get()->functionNames();

  R_xlen_t num_names = ns_names->count();
  writable::strings out(num_names);
  for (R_xlen_t i = 0; i < num_names; i++) {
    out[i] = ((NS::String*)ns_names->object(i))->utf8String();
  }

  return out;
}

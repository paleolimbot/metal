#include <cpp11.hpp>
using namespace cpp11;

#include "Metal/Metal.hpp"

class DeviceWrapper {
 public:
  DeviceWrapper(MTL::Device* pDevice = MTL::CreateSystemDefaultDevice())
      : device_(pDevice->retain()) {
    command_queue_ = device_->newCommandQueue();
  }

  MTL::Device* device() { return device_; }
  MTL::CommandQueue* command_queue() { return command_queue_; }

  ~DeviceWrapper() {
    command_queue_->release();
    device_->release();
  }

 private:
  MTL::Device* device_;
  MTL::CommandQueue* command_queue_;
};

[[cpp11::register]]
sexp cpp_default_device() {
  external_pointer<DeviceWrapper> device_ptr(new DeviceWrapper());
  sexp device_sexp = (SEXP)device_ptr;
  device_sexp.attr("class") = "mtl_device";
  return device_sexp;
}

[[cpp11::register]]
list cpp_device_info(sexp device_sexp) {
  external_pointer<DeviceWrapper> device_ptr(device_sexp);
  NS::String* name = device_ptr->device()->name();
  NS::String* description = device_ptr->device()->description();

  writable::list out = {as_sexp(name->utf8String()), as_sexp(description->utf8String())};
  out.names() = {"name", "description"};
  return out;
}

class LibraryWrapper {
 public:
  LibraryWrapper(MTL::Library* library) : library_(library->retain()) {}

  MTL::Library* library() { return library_; }

  ~LibraryWrapper() { library_->release(); }

 private:
  MTL::Library* library_;
};

[[cpp11::register]]
sexp cpp_make_library(sexp device_sexp, std::string code) {
  if (!Rf_inherits(device_sexp, "mtl_device")) {
    stop("`device` is not an mtl_device");
  }

  external_pointer<DeviceWrapper> device_ptr(device_sexp);

  NS::Error* error = nullptr;
  NS::String* ns_code = NS::String::string(code.c_str(), NS::StringEncoding::UTF8StringEncoding);
  MTL::CompileOptions* options = MTL::CompileOptions::alloc();
  options->init();

  MTL::Library* library = device_ptr->device()->newLibrary(ns_code, options, &error);
  ns_code->release();
  options->release();

  if (error != nullptr) {
    const char* description = error->localizedDescription()->utf8String();
    library->release();
    stop("Error compiling metal code:\n%s", description);
  }

  external_pointer<LibraryWrapper> library_ptr(new LibraryWrapper(library));
  sexp library_sexp = (SEXP)library_ptr;
  library_sexp.attr("class") = "mtl_library";
  return library_sexp;
}

[[cpp11::register]]
strings cpp_library_function_names(sexp library_sexp) {
  if (!Rf_inherits(library_sexp, "mtl_library")) {
    stop("`library` is not an mtl_library");
  }

  external_pointer<LibraryWrapper> library_ptr(library_sexp);
  NS::Array* ns_names = library_ptr->library()->functionNames();

  R_xlen_t num_names = ns_names->count();
  writable::strings out(num_names);
  for (R_xlen_t i = 0; i < num_names; i++) {
    out[i] = ((NS::String*)ns_names->object(i))->utf8String();
  }

  return out;
}

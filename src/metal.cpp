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

template <typename T>
const char* owner_xptr_classname();

template <typename T>
class OwnerXPtr : public external_pointer<Owner<T>> {
 public:
  OwnerXPtr(T* ptr) : external_pointer<Owner<T>>(new Owner<T>(ptr)) {
    sexp xptr_sexp = (SEXP)(*this);
    xptr_sexp.attr("class") = owner_xptr_classname<T>();
  }

  OwnerXPtr(sexp xptr) : external_pointer<Owner<T>>(xptr) {
    if (!Rf_inherits(xptr, owner_xptr_classname<T>())) {
      stop("external pointer does not inherit from '%s'", owner_xptr_classname<T>());
    }
  }
};

template <>
const char* owner_xptr_classname<MTL::Device>() {
  return "mtl_device";
}

template <>
const char* owner_xptr_classname<MTL::Library>() {
  return "mtl_library";
}

template <>
const char* owner_xptr_classname<MTL::Function>() {
  return "mtl_function";
}

using DeviceXPtr = OwnerXPtr<MTL::Device>;
using LibraryXPtr = OwnerXPtr<MTL::Library>;
using FunctionXptr = OwnerXPtr<MTL::Function>;

[[cpp11::register]] sexp cpp_default_device() {
  MTL::Device* default_device = MTL::CreateSystemDefaultDevice();
  if (default_device == nullptr) {
    stop("No default device found");
  }

  DeviceXPtr device(default_device);
  return (SEXP)device;
}

[[cpp11::register]] list cpp_device_info(sexp device_sexp) {
  DeviceXPtr device_xptr(device_sexp);
  NS::String* name = device_xptr->get()->name();
  NS::String* description = device_xptr->get()->description();

  writable::list out = {as_sexp(name->utf8String()), as_sexp(description->utf8String())};
  out.names() = {"name", "description"};
  return out;
}

[[cpp11::register]] sexp cpp_make_library(sexp device_sexp, std::string code) {
  DeviceXPtr device_xptr(device_sexp);

  NS::Error* error = nullptr;
  Owner<NS::String> ns_code =
      NS::String::string(code.c_str(), NS::StringEncoding::UTF8StringEncoding);
  Owner<MTL::CompileOptions> options = MTL::CompileOptions::alloc();
  options.get()->init();

  MTL::Library* library =
      device_xptr->get()->newLibrary(ns_code.get(), options.get(), &error);
  if (library == nullptr) {
    const char* description = error->localizedDescription()->utf8String();
    library->release();
    stop("Error compiling metal code:\n%s", description);
  }

  LibraryXPtr library_xptr(library);
  return (SEXP)library_xptr;
}

[[cpp11::register]] strings cpp_library_function_names(sexp library_sexp) {
  LibraryXPtr library_xptr(library_sexp);
  NS::Array* ns_names = library_xptr->get()->functionNames();

  R_xlen_t num_names = ns_names->count();
  writable::strings out(num_names);
  for (R_xlen_t i = 0; i < num_names; i++) {
    out[i] = ((NS::String*)ns_names->object(i))->utf8String();
  }

  return out;
}

[[cpp11::register]] sexp cpp_library_function(sexp library_sexp, std::string name) {
  LibraryXPtr library_xptr(library_sexp);
  Owner<NS::String> ns_name =
      NS::String::string(name.c_str(), NS::StringEncoding::UTF8StringEncoding);
  MTL::Function* function = library_xptr->get()->newFunction(ns_name.get());

  if (function == nullptr) {
    return R_NilValue;
  }

  FunctionXptr function_xptr(function);
  return (SEXP)function_xptr;
}

[[cpp11::register]] list cpp_function_info(sexp function_sexp) {
  FunctionXptr function_xptr(function_sexp);

  NS::String* ns_name = function_xptr->get()->name();

  MTL::FunctionType type = function_xptr->get()->functionType();
  std::string type_string;
  switch (type) {
    case MTL::FunctionType::FunctionTypeFragment:
      type_string = "fragment";
      break;
    case MTL::FunctionType::FunctionTypeIntersection:
      type_string = "intersection";
      break;
    case MTL::FunctionType::FunctionTypeKernel:
      type_string = "kernel";
      break;
    case MTL::FunctionType::FunctionTypeVertex:
      type_string = "vertex";
      break;
    default:
      type_string = "unknown";
      break;
  }

  writable::list out = {as_sexp(ns_name->utf8String()), as_sexp(type_string)};
  out.names() = {"name", "type"};
  return out;
}

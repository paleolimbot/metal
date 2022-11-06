
#include <unordered_map>

#include <cpp11.hpp>
using namespace cpp11;

#include "Metal/Metal.hpp"

template <typename T>
const char* owner_xptr_classname();

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

template <>
const char* owner_xptr_classname<MTL::ComputePipelineState>() {
  return "mtl_compute_pipeline";
}

template <>
const char* owner_xptr_classname<MTL::CommandQueue>() {
  return "mtl_command_queue";
}

template <>
const char* owner_xptr_classname<MTL::Buffer>() {
  return "mtl_buffer";
}

template <>
const char* owner_xptr_classname<NS::String>() {
  return "ns_string";
}

template <>
const char* owner_xptr_classname<MTL::CompileOptions>() {
  return "mtl_compile_options";
}

template <typename T>
class Owner {
 public:
  Owner() : ptr_(nullptr) {}
  Owner(T* ptr) : ptr_(ptr) {}

  void reset(T* ptr) {
    ptr_->release();
    ptr_ = ptr;
  }

  T* get() { return ptr_; }

  ~Owner() { reset(nullptr); }

 private:
  T* ptr_;
};

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

using DeviceXPtr = OwnerXPtr<MTL::Device>;
using LibraryXPtr = OwnerXPtr<MTL::Library>;
using FunctionXptr = OwnerXPtr<MTL::Function>;
using ComputePipelineXptr = OwnerXPtr<MTL::ComputePipelineState>;
using CommandQueueXptr = OwnerXPtr<MTL::CommandQueue>;
using BufferXptr = OwnerXPtr<MTL::Buffer>;

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

static std::unordered_map<const void*, SEXP> buffer_shelter;

[[cpp11::register]] sexp cpp_buffer(sexp device_sexp, double size_dbl) {
  const void* ptr = nullptr;
  NS::UInteger size = size_dbl;

  DeviceXPtr device_xptr(device_sexp);
  MTL::Buffer* buffer =
      device_xptr->get()->newBuffer(size, MTL::ResourceStorageModeShared);

  if (buffer == nullptr) {
    preserved.release(buffer_shelter[ptr]);
    buffer_shelter.erase(ptr);
    stop("Failed to create buffer");
  }

  BufferXptr buffer_xptr(buffer);
  return (SEXP)buffer_xptr;
}

[[cpp11::register]] double cpp_buffer_size(sexp buffer_sexp) {
  BufferXptr buffer_xptr(buffer_sexp);
  return buffer_xptr->get()->length();
}

[[cpp11::register]] void cpp_buffer_copy_from(sexp src_sexp, sexp buffer_sexp,
                                              double src_offset, double buffer_offset,
                                              double length) {
  if (length == 0) {
    return;
  }

  if (src_offset < 0 || buffer_offset < 0) {
    stop("Invalid src_offset or buffer offset");
  }

  R_xlen_t r_length = Rf_xlength(src_sexp);
  R_xlen_t min_size = src_offset + length;
  R_xlen_t element_size;
  R_xlen_t actual_size;
  switch (TYPEOF(src_sexp)) {
    case INTSXP:
    case LGLSXP:
      element_size = sizeof(int);
      actual_size = r_length * sizeof(int);
      break;
    case REALSXP:
      element_size = sizeof(double);
      actual_size = r_length * sizeof(double);
      break;
    case RAWSXP:
      element_size = sizeof(unsigned char);
      actual_size = r_length;
      break;
    default:
      stop("Vector type not supported for src");
  }

  if ((((R_xlen_t)length) % element_size) != 0) {
    stop("Length must be a multiple of vector element size");
  }

  if (actual_size < min_size) {
    stop("Vector not long enough for specified arguments");
  }

  BufferXptr buffer_xptr(buffer_sexp);

  if ((buffer_offset + length) > buffer_xptr->get()->length()) {
    stop("Buffer not long enough for specified arguments");
  }

  auto src = reinterpret_cast<const uint8_t*>(DATAPTR_RO(src_sexp));
  auto dst = reinterpret_cast<uint8_t*>(buffer_xptr->get()->contents());
  int64_t buffer_offset_int = buffer_offset;
  int64_t src_offset_int = src_offset;
  memcpy(dst + buffer_offset_int, src + src_offset_int, length);
}

[[cpp11::register]] sexp cpp_buffer_copy_into(sexp buffer_sexp, sexp ptype,
                                              double buffer_offset, double length) {
  BufferXptr buffer_xptr(buffer_sexp);
  if (buffer_offset < 0) {
    stop("Invalid buffer offset argument");
  }

  if ((buffer_offset + length) > buffer_xptr->get()->length()) {
    stop("Buffer not long enough for specified arguments");
  }

  sexp result_sexp;
  R_xlen_t actual_size;
  switch (TYPEOF(ptype)) {
    case INTSXP:
      result_sexp = writable::integers((R_xlen_t)length / 4);
      actual_size = Rf_xlength(result_sexp) * 4;
      break;
    case LGLSXP:
      result_sexp = writable::logicals((R_xlen_t)length / 4);
      actual_size = Rf_xlength(result_sexp) * 4;
      break;
    case REALSXP:
      result_sexp = writable::doubles((R_xlen_t)length / 8);
      actual_size = Rf_xlength(result_sexp) * 8;
      break;
    case RAWSXP:
      result_sexp = writable::raws((R_xlen_t)length);
      actual_size = length;
      break;
    default:
      stop("Vector type not supported for ptype");
  }

  if (actual_size != length) {
    stop("Length must be a multiple of vector element size");
  }

  auto src = reinterpret_cast<uint8_t*>(buffer_xptr->get()->contents());
  auto dst = reinterpret_cast<uint8_t*>(DATAPTR(result_sexp));
  int64_t buffer_offset_int = buffer_offset;
  memcpy(dst, src + buffer_offset_int, length);
  return result_sexp;
}

[[cpp11::register]] sexp cpp_buffer_pointer(sexp buffer_sexp) {
  BufferXptr buffer_xptr(buffer_sexp);
  sexp length_sexp = safe[Rf_ScalarReal](buffer_xptr->get()->length());
  return safe[R_MakeExternalPtr](buffer_xptr->get()->contents(), length_sexp,
                                 buffer_sexp);
}

[[cpp11::register]] sexp cpp_command_queue(sexp device_sexp) {
  DeviceXPtr device_xptr(device_sexp);
  CommandQueueXptr command_queue_xptr(device_xptr->get()->newCommandQueue());
  return (SEXP)command_queue_xptr;
}

[[cpp11::register]] sexp cpp_compute_pipeline(sexp function_sexp) {
  FunctionXptr function_xptr(function_sexp);
  NS::Error* error = nullptr;
  MTL::Device* device = function_xptr->get()->device();
  MTL::ComputePipelineState* pipeline =
      device->newComputePipelineState(function_xptr->get(), &error);
  if (pipeline == nullptr) {
    const char* description = error->localizedDescription()->utf8String();
    stop("Error creating compute pipeline:\n%s", description);
  }

  ComputePipelineXptr pipeline_xptr(pipeline);
  return (SEXP)pipeline_xptr;
}

[[cpp11::register]] void cpp_compute_pipeline_execute(sexp pipeline_sexp,
                                                      sexp commmand_queue_sexp,
                                                      list args) {
  ComputePipelineXptr pipeline_xptr(pipeline_sexp);
  CommandQueueXptr command_queue_xptr(commmand_queue_sexp);

  MTL::CommandBuffer* command_buffer = command_queue_xptr->get()->commandBuffer();
  MTL::ComputeCommandEncoder* command_encoder = command_buffer->computeCommandEncoder();
  command_encoder->setComputePipelineState(pipeline_xptr->get());

  for (R_xlen_t i = 0; i < args.size(); i++) {
    SEXP item = args[i];
    if (item == R_NilValue) {
      continue;
    }

    BufferXptr buffer_xptr(item);
    command_encoder->setBuffer(buffer_xptr->get(), 0, i);
  }

  NS::UInteger array_length = 1;
  MTL::Size grid_size = MTL::Size::Make(array_length, 1, 1);
  NS::UInteger thread_group_size_n =
      pipeline_xptr->get()->maxTotalThreadsPerThreadgroup();
  if (thread_group_size_n > array_length) {
    thread_group_size_n = array_length;
  }

  MTL::Size thread_group_size = MTL::Size::Make(thread_group_size_n, 1, 1);

  command_encoder->dispatchThreads(grid_size, thread_group_size);
  command_encoder->endEncoding();
  command_buffer->commit();
  command_buffer->waitUntilCompleted();
}

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
  device_sexp.attr("class") = "metalme_device";
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

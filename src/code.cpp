#include <cpp11.hpp>
using namespace cpp11;

#include "Metal/Metal.hpp"

class Renderer {
 public:
  Renderer(MTL::Device* pDevice = MTL::CreateSystemDefaultDevice())
      : _pDevice(pDevice->retain()) {
    _pCommandQueue = _pDevice->newCommandQueue();
  }

  MTL::Device* device() { return _pDevice; }

  ~Renderer() {
    _pCommandQueue->release();
    _pDevice->release();
  }

 private:
  MTL::Device* _pDevice;
  MTL::CommandQueue* _pCommandQueue;
};

[[cpp11::register]]
list device_info() {
  Renderer renderer;
  NS::String* name = renderer.device()->name();
  NS::String* description = renderer.device()->description();

  writable::list out = {
    as_sexp(name->utf8String()),
    as_sexp(description->utf8String())
  };

  out.names() = {"name", "description"};

  return out;
}

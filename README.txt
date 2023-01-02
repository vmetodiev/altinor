$ amdgpu-install --usecase=graphics,opencl --vulkan=amdvlk --opencl=legacy

$ sudo apt install opencl-headers
$ sudo apt install ocl-icd-opencl-dev

//
// Inside the benchmark.cpp file, toggle the USE_OPEN_CL macro to switch from OpenCL to strstr()
//
$ make
$ time ./benchmark
$ make clean

//
// Notes
//
1. OpenCL is a tricky open standard. 
   1.1. The code execution is closely related to the driver and runtime implementation of the specific vendor.
   1.2. Some interesting products (like AMD Ryzen 7600X with integrated RDNA2) do not have a Linux driver. Thus, is it impossible to run our algorithm on that hardware.
   1.3. Code and binary portability is rather difficult.
   1.4. Exceeding the 65kByte buffer size leads to undefined behaviour. In the context of real-time packet inspection, this limations is simply ridiculuous.

2. Copying data back and forth between the GPU and the CPU is a great issue for real-time, high-throughput networking applications, expecially for those targering zero packet loss.
Although there are technologies like PCIe P2P and DirectGMA/GPUDirect, the developer of a network application will often need to operate with the host memory buffers.

3. Integrating GPU code into the Linux XPD data path is not a trivial task. The C syntax is limited, the LLVM compiler generates code that is difficult to debug and eventually "inserts"
the compiled BPF instructions into the kernel. Thus, it is extremely hard to integrate it with OpenCL (since the algo will stay in the user space and use the GPU driver to execute the OpenCL "kernel").

4. Most the network offload software and acceleration SmartNICs/DPUs are focused on packet header operations. Our goal is to cover the payload instead. That is why we believe
that the Tenstorrent processors will be an ideal fit for security scenarios.

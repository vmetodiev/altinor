$ amdgpu-install --usecase=graphics,opencl --vulkan=amdvlk --opencl=legacy

$ sudo apt install opencl-headers
$ sudo apt install ocl-icd-opencl-dev

//
// Inside the benchmark.cpp file, toggle the USE_OPEN_CL macro to switch from OpenCL to strstr()
//
$ make
$ time ./benchmark
$ make clean

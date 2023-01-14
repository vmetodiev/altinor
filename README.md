# Altinor
Altinor is a minimalistic, highly parallel byte-level pattern matching engine for deep packet inspection (DPI), written in OpenCL and C/C++.
The current PoC implementation runs on AMD Radeon GPUs.

# Algorithm
This is the visual representation of the parallelised pattern search:
![Screenshot](https://github.com/vmetodiev/altinor/blob/master/pics/ALGO.JPG)<br />

# Applications
This repository contains an example that implements a network Instrusion Detection System (IDS), capable of inspecting TCP and/or UDP packets.
It works similarly to a packet sniffer, intercepting packets at OSI L2 via a Linux raw socket.

Upon detecting the configured packet signature, Altinor will log a message on the stdout with the source IP address of the attacker. 
Some (Python) scripting could be further used to poll the alert message and react via compiling and applying an XDP/eBPF action on the interface.

# Build Instructions
Install the AMD drivers and OpenCL development packages. Below is an example for Ubuntu:

```
$ amdgpu-install --usecase=graphics,opencl --vulkan=amdvlk --opencl=legacy
$ sudo apt install opencl-headers
$ sudo apt install ocl-icd-opencl-dev
$ make
$ ./sudo altinor
```
# Example usage
Open the config.h file. 

## Network Interface
Set the sniffing network interface:
```
#define NETWORK_INTERFACE_NAME "enp2s0"
```

## Payload signature
Describe the signature inside the array - byte by byte, or char by char, follow the standard C/C++ syntax:
```
uint8_t signature[] = { 'E', 'x', 'p', 'l', 'o', 'i', 't', 'B', 'y', 't', 'e', 's', '0', 'P' };
```

The signature length should also be specified inside the SIGNATURE_LEN macro:
```
#define SIGNATURE_LEN ( 14 )
```

## Define what kind of traffic to inspect:
For TCP ony:
```
#define INSPECT_TCP ( 1 )
#define INSPECT_UDP ( 0 )
```

For UDP only:
```
#define INSPECT_TCP ( 0 )
#define INSPECT_UDP ( 1 )
```

For both TCP and UDP:
```
#define INSPECT_TCP ( 1 )
#define INSPECT_UDP ( 1 )
```
## Configure zero-copy on the interface
Use the following option to attempt switching the sniffing interface to zero-copy mode.
```
#define ATTEMPT_ZERO_COPY_OPTION ( 1 )
```  

Note: this may not work in many cases. Observe the log upon lauching the application.


## (Re)Compile and run
```
$ make clean
$ make
$ ./sudo altinor
```

## Open a TCP server
```
$ nc -l 192.168.100.6 65321
```

## Send a packet containing the signature inside the payload
```
$ nc 192.168.100.6 65321
123-ExploitBytes0P-abcdEFGH
```

## Observe the Altinor output on the console
```
$ sudo ./altinor  
EXPLOIT! from IP 192.168.100.6 
```

## Scripting
Congratulations, you have detected an exploit signature via the GPU!  
From now on, use the output on the console with some scripting language and generate a reaction accordingly - like generating an XDP_DROP action for that specific IP address
and apply it on the interface for a specific time duration.

# Limitations
The current Altinor implementation is a pure proof-of-concept experiment.

1. No fast path (like DPDK), using a RAW socket instead.  
  
2. OpenCL GPU-to-host transfers are not zero copy. There is no RDMA between the NIC and the GPU as P2P PCIe transactions.  
   For CPUs with intergrated graphics, one should always check if the vendor provides an OpenCL driver support. If so, both CPU and GPU can use a common buffer (pointer), thus avoiding the copy overhead.  
  
3. The current implemention may miss the signature in case it is fragmented between two packets. This will be fixed.
  
4. Only one signature can be inspected per process. For more, compile and launch another 'instance' and run it as a separate process - one may also try to automate this via Docker, Ansible and other DevOps-oriented tools.  

# In progress...
The author is currently working on:

1. A CLI shell based on Radix Tree

2. Multi-signature hot-plug support - without interrupting the running process

# Future
Future versions will add in-line IPS functionality using DPDK. 

The author is also considering integrating the Altinor engine as an nginx module for HTTP inspection.  

Aside from GPUs, other parallel accelerators could be much more interesting and applicable for parallel pattern matching (like the Tenstorrent GraySkull).

# Altinor
Altinor is a minimalistic, highly parallel byte-level pattern matching engine for deep packet inspection (DPI), written in OpenCL and C.
The current PoC implementation runs on an AMD Radeon GPU.

# Algorithm
This is the visual representation of the parallelised pattern search:
![Screenshot](https://github.com/vmetodiev/altinor/blob/master/pics/ALGO.JPG)<br />

# Applications
This repo contains an example that implements a network IDS (instrusion detection system), capable for inspecting TCP and/or UDP packets.
It works similarly to a packet sniffer, intecepting packets at OSI L2 via a Linux raw socket.

Upon detecting the configured packet signature, Altinor will log a message on the stdout with the source IP address of the attacker. 
Some (Python) scripting could further be used to poll the alert message and react via compiling and applying an XDP/eBPF action on the interface.

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

## Payload signature
Describe the signature inside the array - byte by byte, or char by char, follow the standard C/C++ syntax.
The signature length should also be specified inside the SIGNATURE_LEN macro.

```
#define SIGNATURE_LEN ( 14 )
uint8_t signature[] = { 'E', 'x', 'p', 'l', 'o', 'i', 't', 'B', 'y', 't', 'e', 's', '0', 'P' };
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
Use the following option to attempt switching the listening interface to zero-copy mode.
```
#define ATTEMPT_ZERO_COPY_OPTION ( 1 )
```  

Note: this may not work in many cases. Observer the log upon lauching the application.


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

# Future
Future versions will add in-line IPS functionality using DPDK.
The author is also considering integrating the Altinor engine as an nginx module for HTTP inspection.
Aside from GPUs, other parallel accelerators could be much more interesting and applicable for parallel pattern matching.

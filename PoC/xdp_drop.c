#include <linux/bpf.h>
#include <bpf/bpf_helpers.h>

//
// https://www.tigera.io/learn/guides/ebpf/ebpf-xdp/
//

// Install all the necessary XDP/eBPF tools for you Linux distro and compile:
// $ clang -O2 -g -Wall -target bpf -c xdp_drop.c -o xdp_drop.o

// Load on network interface:
// $ sudo ip link set enp2s0 xdpgeneric obj xdp_drop.o sec xdp_drop

// Unload:
// $ sudo ip link set enp2s0 xdpgeneric off

SEC("xdp_drop")
int xdp_drop_prog(struct xdp_md *ctx)
{
    return XDP_DROP;
}

char _license[] SEC("license") = "GPL";
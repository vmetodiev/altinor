__kernel void addVectors(
	__global const uchar *a, 
	__global const uchar *b,
	__global uchar *c) 
{		
    const int i = get_global_id(0);
	
	c[i] = ( a[i] ^ b[i] ) ? 0 : 1;
}
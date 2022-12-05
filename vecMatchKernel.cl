__kernel void matchVectors(
	__global const uchar *a, 
	__global const uchar *b,
	volatile __global uint *c)
{		
	const int i = get_global_id(0);
	if ( ( a[i] ^ b[i] ) == 0 ) 
	{
		atomic_inc(c);
	}
}
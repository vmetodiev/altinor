#define MATCH_FOUND 255
#define BLACK_SPACE 'X'

__kernel void matchVectors(
	__global const uchar *a, 
	__global const uchar *b,
	volatile __global uint *c,
	__global uint *size,
	__global uint *parts,
	__global uint *signature_length)
{		
	int batch = *size;
	
	int n = get_local_id(0); // or get_global_id

	for ( int i = 0; i < *parts; i++ )
	{
		int m = ( batch * i ) + n;
		
		if ( ( a[n] ^ b[m] ) == 0 ) 
			if ( a[n] != BLACK_SPACE )
				atomic_inc(c);

		printf("  %i\n", m);
	
		if ( *c == *signature_length ) 
		{
			*c = MATCH_FOUND;
			return;
		}
	}
}
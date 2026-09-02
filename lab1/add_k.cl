// O = I + K
__kernel void addK(__global const char* in, __global char* out, const char K)
{
    int num = get_global_id(0);
    out[num] = in[num] + K;
}

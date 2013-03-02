const sampler_t sampler = CLK_NORMALIZED_COORDS_FALSE | CLK_ADDRESS_NONE | CLK_FILTER_NEAREST;

__kernel void generateThumbnail (
   __read_only image2d_t inputImage,
   __global uchar3 * output,
   __local uint3 * localArray) {
  int globalIdX = get_global_id(0);
  int globalIdY = get_global_id(1);
  int localIdX = get_local_id(0);
  int localIdY = get_local_id(1);
  int localSizeX = get_local_size(0);
  int localSizeY = get_local_size(1);
  int groupIdX = get_group_id(0);
  int groupIdY = get_group_id(1);
  int groupNumX = get_num_groups(0);
  int groupNumY = get_num_groups(1);

  uint size = localSizeX * localSizeY;

  uint indexLocal = localIdX + localIdY * localSizeX;
  localArray[ indexLocal ] = read_imageui(inputImage, sampler, (int2)(globalIdX, globalIdY)).xyz;
  barrier(CLK_LOCAL_MEM_FENCE);

  for(unsigned int s = localSizeY * localSizeX / 2; s > 0; s>>=1) {
      if(indexLocal < s) {
          localArray[indexLocal]
              += localArray[indexLocal + s];
      }
      barrier(CLK_LOCAL_MEM_FENCE);
  }

  if(localIdX==0 && localIdY==0) {
    int index = groupIdX * 3 + groupIdY * groupNumX * 3;
    output[index] = convert_uchar3(localArray[0] / size);
  }
}
  }
}

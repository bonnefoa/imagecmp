const sampler_t sampler = CLK_NORMALIZED_COORDS_FALSE | CLK_ADDRESS_NONE | CLK_FILTER_NEAREST;

__kernel void generateHistogram (
   __read_only image2d_t inputImage,
   __global float16 * outputBuffer,
   __local ushort16 * localArray) {

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

  int imageWidth = get_image_width(inputImage);
  int imageHeight = get_image_height(inputImage);

  uint indexLocal = localIdX + localIdY * localSizeX;
  localArray[ indexLocal ] = 0;

  uint3 pixel = (uint3)(0, 0, 0);
  if(globalIdX <= imageWidth || globalIdY <= imageHeight) {
    pixel = read_imageui(inputImage, sampler, (int2)(globalIdX, globalIdY)).xyz;
    for (int i = 0; i < 3; i++ ) {
        uchar bucket = i * 5 + pixel[i] / 51;
        localArray[ indexLocal ][bucket] += 1;
    }
    localArray[ indexLocal ].sf += 1;
  }

  barrier(CLK_LOCAL_MEM_FENCE);

  for ( unsigned int s = localSizeY * localSizeX / 2; s > 0; s >>= 1 ) {
      if(indexLocal < s) {
          localArray[indexLocal] += localArray[indexLocal + s];
      }
      barrier(CLK_LOCAL_MEM_FENCE);
  }

  if(localIdX==0 && localIdY==0) {
    int index = groupIdX + groupIdY * groupNumX;

    float numPixels = localArray[0].sf;
    outputBuffer[index] = convert_float16(localArray[0]);
    outputBuffer[index] /= localArray[0].sf;
    outputBuffer[index].sf = numPixels;
  }
}

open Js.Typed_array;

open WebGPU;

open StateType;

let setMat3DataToBufferData = (mat3, (bufferData, offset)) => {
  Float32Array.unsafe_set(
    bufferData,
    offset,
    Float32Array.unsafe_get(mat3, 0),
  );
  Float32Array.unsafe_set(
    bufferData,
    offset + 1,
    Float32Array.unsafe_get(mat3, 1),
  );
  Float32Array.unsafe_set(
    bufferData,
    offset + 2,
    Float32Array.unsafe_get(mat3, 2),
  );
  Float32Array.unsafe_set(bufferData, offset + 3, 0.0);
  Float32Array.unsafe_set(
    bufferData,
    offset + 4,
    Float32Array.unsafe_get(mat3, 3),
  );
  Float32Array.unsafe_set(
    bufferData,
    offset + 5,
    Float32Array.unsafe_get(mat3, 4),
  );
  Float32Array.unsafe_set(
    bufferData,
    offset + 6,
    Float32Array.unsafe_get(mat3, 5),
  );
  Float32Array.unsafe_set(bufferData, offset + 7, 0.0);
  Float32Array.unsafe_set(
    bufferData,
    offset + 8,
    Float32Array.unsafe_get(mat3, 6),
  );
  Float32Array.unsafe_set(
    bufferData,
    offset + 9,
    Float32Array.unsafe_get(mat3, 7),
  );
  Float32Array.unsafe_set(
    bufferData,
    offset + 10,
    Float32Array.unsafe_get(mat3, 8),
  );
  Float32Array.unsafe_set(bufferData, offset + 11, 0.0);

  (bufferData, offset + 12);
};

module UniformBuffer = {
  let getAlignedBufferBytes = singleEntityBufferSize => {
    Js.Math.ceil(float_of_int(singleEntityBufferSize) /. 256.) * 256;
  };

  let getAlignedBufferBytesFromFloats = alignedBufferFloats => {
    alignedBufferFloats * Float32Array._BYTES_PER_ELEMENT;
  };

  let getAlignedBufferFloats = alignedBufferBytes => {
    alignedBufferBytes / Float32Array._BYTES_PER_ELEMENT;
  };
  // let unsafeGetCameraBufferData = state => {
  //   (
  //     state.gpuBuffer.uniformBuffer.cameraBufferData.cameraData
  //     |> Js.Option.getExn,
  //     state.gpuBuffer.uniformBuffer.cameraBufferData.cameraBuffer
  //     |> Js.Option.getExn,
  //   );
  // };
  // let setCameraBufferData = ((cameraData, cameraBuffer), state) => {
  //   ...state,
  //   gpuBuffer: {
  //     ...state.gpuBuffer,
  //     uniformBuffer: {
  //       ...state.gpuBuffer.uniformBuffer,
  //       cameraBufferData: {
  //         cameraData: Some(cameraData),
  //         cameraBuffer: Some(cameraBuffer),
  //       },
  //     },
  //   },

  // };
};

module StorageBuffer = {
  // this storage buffer is used as a pixel buffer
  // the result of the ray tracing pass gets written into it
  // and it gets copied to the screen in the rasterization pass
  let buildPixelBufferData = (window, device) => {
    let pixelBufferSize =
      Window.getWidth(window)
      * Window.getHeight(window)
      * 4
      * Float32Array._BYTES_PER_ELEMENT;
    let pixelBuffer =
      device
      |> Device.createBuffer({
           "size": pixelBufferSize,
           "usage": BufferUsage.storage,
         });

    (pixelBufferSize, pixelBuffer);
  };
};

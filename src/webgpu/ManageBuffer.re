open Js.Typed_array;

open WebGPU;

open StateType;

module UniformBuffer = {
  let unsafeGetCameraBufferData = state => {
    (
      state.gpuBuffer.uniformBuffer.cameraBufferData.cameraData
      |> Js.Option.getExn,
      state.gpuBuffer.uniformBuffer.cameraBufferData.cameraBuffer
      |> Js.Option.getExn,
    );
  };

  let setCameraBufferData = ((cameraData, cameraBuffer), state) => {
    ...state,
    gpuBuffer: {
      ...state.gpuBuffer,
      uniformBuffer: {
        ...state.gpuBuffer.uniformBuffer,
        cameraBufferData: {
          cameraData: Some(cameraData),
          cameraBuffer: Some(cameraBuffer),
        },
      },
    },
  };

  let buildCameraBufferData = (device, state) => {
    let currentCamerView = CameraView.unsafeGetCurrentCameraView(state);

    let cameraData =
      Float32Array.fromLength(
        (
          CameraView.unsafeGetViewMatrixInverse(currentCamerView, state)
          |> Float32Array.length
        )
        + (
          CameraView.unsafeGetProjectionMatrixInverse(currentCamerView, state)
          |> Float32Array.length
        ),
      );
    let cameraBufferSize = cameraData |> Float32Array.byteLength;
    let cameraBuffer =
      device
      |> Device.createBuffer({
           "size": cameraBufferSize,
           "usage": BufferUsage.copy_dst lor BufferUsage.uniform,
         });

    (cameraData, cameraBufferSize, cameraBuffer);
  };

  let buildResolutionBufferData = (window, device ) => {
    let resolutionData =
      Float32Array.make([|
        Window.getWidth(window) |> float_of_int,
        Window.getHeight(window) |> float_of_int,
      |]);
    let resolutionBufferSize = resolutionData |> Float32Array.byteLength;
    let resolutionUniformBuffer =
      device
      |> Device.createBuffer({
           "size": resolutionBufferSize,
           "usage": BufferUsage.copy_dst lor BufferUsage.uniform,
         });
    resolutionUniformBuffer |> Buffer.setSubFloat32Data(0, resolutionData);

    (resolutionBufferSize, resolutionUniformBuffer);
  };
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

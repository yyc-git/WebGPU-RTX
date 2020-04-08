open Js.Typed_array;

open WebGPU;

module CameraBuffer = {
  let buildData = (device, state) => {
    let currentCamerView = CameraView.unsafeGetCurrentCameraView(state);

    let cameraData =
      Float32Array.fromLength(
        4
        + (
          CameraView.unsafeGetViewMatrix(currentCamerView, state)
          |> Float32Array.length
        )
        + (
          CameraView.unsafeGetProjectionMatrix(currentCamerView, state)
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

  let unsafeGetCameraBufferData = state => {
    Pass.unsafeGetUniformBufferData("cameraBuffer", state);
  };

  let _setCameraBufferData = ((buffer, bufferData), state) => {
    Pass.setUniformBufferData("cameraBuffer", (buffer, bufferData), state);
  };

  let update = (lookFrom, viewMatrix, projectionMatrix, state) => {
    let (cameraBuffer, cameraData) = unsafeGetCameraBufferData(state);

    let (cameraData, offset) =
      cameraData |> TypeArray.Float32Array.setFloatTuple3(0, lookFrom);
    let (cameraData, offset) =
      cameraData
      |> TypeArray.Float32Array.setFloat32Array(offset + 1, viewMatrix);
    let (cameraData, offset) =
      cameraData |> TypeArray.Float32Array.setFloat32Array(offset, viewMatrix);

    cameraBuffer |> Buffer.setSubFloat32Data(0, cameraData);
    let state = state |> _setCameraBufferData((cameraBuffer, cameraData));

    state;
  };
};

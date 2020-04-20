open Js.Typed_array;

open WebGPU;

module CameraBuffer = {
  let buildData = (device, state) => {
    let currentCamerView = CameraView.unsafeGetCurrentCameraView(state);

    let cameraBufferData = Float32Array.fromLength(4 + 16 + 16 + 16);
    let cameraBufferSize = cameraBufferData |> Float32Array.byteLength;
    let cameraBuffer =
      device
      |> Device.createBuffer({
           "size": cameraBufferSize,
           "usage": BufferUsage.copy_dst lor BufferUsage.uniform,
         });

    (cameraBufferData, cameraBufferSize, cameraBuffer);
  };

  let unsafeGetCameraBufferData = state => {
    Pass.unsafeGetUniformBufferData("cameraBuffer", state);
  };

  let _setCameraBufferData = ((bufferData, buffer), state) => {
    Pass.setUniformBufferData("cameraBuffer", (bufferData, buffer), state);
  };

  let update =
      (
        lookFrom,
        viewMatrix,
        jitterdProjectionMatrix,
        lastViewJitterdProjectionMatrixOpt,
        state,
      ) => {
    let (cameraBufferData, cameraBuffer) = unsafeGetCameraBufferData(state);

    let (cameraBufferData, offset) =
      cameraBufferData |> TypeArray.Float32Array.setFloatTuple3(0, lookFrom);
    let (cameraBufferData, offset) =
      cameraBufferData
      |> TypeArray.Float32Array.setFloat32Array(offset + 1, viewMatrix);
    let (cameraBufferData, offset) =
      cameraBufferData
      |> TypeArray.Float32Array.setFloat32Array(
           offset,
           jitterdProjectionMatrix,
         );
    let (cameraBufferData, offset) =
      switch (lastViewJitterdProjectionMatrixOpt) {
      | None => (cameraBufferData, offset)
      | Some(lastViewJitterdProjectionMatrix) =>
        cameraBufferData
        |> TypeArray.Float32Array.setFloat32Array(
             offset,
             lastViewJitterdProjectionMatrix,
           )
      };
    // Log.printComplete("cameraBufferData:", cameraBufferData);

    cameraBuffer |> Buffer.setSubFloat32Data(0, cameraBufferData);
    let state =
      state |> _setCameraBufferData((cameraBufferData, cameraBuffer));

    state;
  };
};

module ModelBuffer = {
  let getModelBufferSize = allRenderGameObjects => {
    let singleRenderGameObjectModelBufferSize =
      (16 + 16 + 12) * Float32Array._BYTES_PER_ELEMENT;
    let alignedModelBufferBytes =
      ManageBuffer.UniformBuffer.getAlignedBufferBytes(
        singleRenderGameObjectModelBufferSize,
      );
    let alignedModelBufferFloats =
      ManageBuffer.UniformBuffer.getAlignedBufferFloats(
        alignedModelBufferBytes,
      );

    (
      singleRenderGameObjectModelBufferSize,
      alignedModelBufferBytes,
      alignedModelBufferFloats,
    );
  };

  let buildData = (device, allRenderGameObjects, state) => {
    let (
      singleRenderGameObjectModelBufferSize,
      alignedModelBufferBytes,
      alignedModelBufferFloats,
    ) =
      getModelBufferSize(allRenderGameObjects);

    let modelBufferSize =
      (allRenderGameObjects |> Js.Array.length) * alignedModelBufferBytes;

    let modelBufferData =
      Float32Array.fromLength(
        modelBufferSize / Float32Array._BYTES_PER_ELEMENT,
      );

    let modelBuffer =
      device
      |> Device.createBuffer({
           "size": modelBufferSize,
           "usage": BufferUsage.copy_dst lor BufferUsage.uniform,
         });

    let (_, offsetArrMap) =
      allRenderGameObjects
      |> ArrayUtils.reduceOneParam(
           (. (offset, offsetArrMap), renderGameObject) => {
             (
               offset + alignedModelBufferFloats,
               offsetArrMap
               |> ImmutableSparseMap.set(
                    renderGameObject,
                    [|
                      ManageBuffer.UniformBuffer.getAlignedBufferBytesFromFloats(
                        offset,
                      ),
                    |],
                  ),
             )
           },
           (0, ImmutableSparseMap.createEmpty()),
         );

    (
      offsetArrMap,
      modelBufferData,
      singleRenderGameObjectModelBufferSize,
      modelBuffer,
    );
  };

  let unsafeGetModelBufferData = state => {
    Pass.unsafeGetUniformBufferData("modelBuffer", state);
  };

  let _setModelBufferData = ((bufferData, buffer), state) => {
    Pass.setUniformBufferData("modelBuffer", (bufferData, buffer), state);
  };

  let update = (allRenderGameObjects, state) => {
    let (modelBufferData, modelBuffer) = unsafeGetModelBufferData(state);

    let (_, alignedModelBufferBytes, alignedModelBufferFloats) =
      getModelBufferSize(allRenderGameObjects);

    let (modelBufferData, _) =
      allRenderGameObjects
      |> ArrayUtils.reduceOneParam(
           (. (modelBufferData, offset), renderGameObject) => {
             let transform =
               GameObject.unsafeGetTransform(renderGameObject, state);

             let lastModelMatrix =
               Pass.GBufferPass.unsafeGetLastModelMatrix(transform, state);
             let modelMatrix = Transform.buildModelMatrix(transform, state);
             let normalMatrix = Transform.buildNormalMatrix(modelMatrix);

             let (modelBufferData, newOffset) =
               (modelBufferData, offset)
               |> ManageBuffer.setMat3DataToBufferData(normalMatrix);

            //  Log.printComplete(
            //    "(modelBufferData, newOffset):",
            //    (modelBufferData, newOffset),
            //  );

             let (modelBufferData, newOffset) =
               modelBufferData
               |> TypeArray.Float32Array.setFloat32Array(
                    newOffset,
                    modelMatrix,
                  );

            //  Log.printComplete(
            //    "(modelBufferData, newOffset):",
            //    (modelBufferData, newOffset),
            //  );

             let (modelBufferData, _) =
               modelBufferData
               |> TypeArray.Float32Array.setFloat32Array(
                    newOffset,
                    lastModelMatrix,
                  );

             (modelBufferData, offset + alignedModelBufferFloats);
           },
           (modelBufferData, 0),
         );

    // Log.printComplete("modelBufferData:", modelBufferData);

    modelBuffer |> Buffer.setSubFloat32Data(0, modelBufferData);

    let state = state |> _setModelBufferData((modelBufferData, modelBuffer));

    state;
  };
};

module DirectionLightBuffer = {
  let buildData = (device, state) => {
    let directionLightData =
      Float32Array.make([|1.0, 0.0, 0.0, 0.0, 0.0, 1.0, 1.0, 0.0|]);
    let directionLightBufferSize =
      directionLightData |> Float32Array.byteLength;
    let directionLightBuffer =
      device
      |> Device.createBuffer({
           "size": directionLightBufferSize,
           "usage": BufferUsage.copy_dst lor BufferUsage.uniform,
         });
    directionLightBuffer |> Buffer.setSubFloat32Data(0, directionLightData);

    (directionLightBufferSize, directionLightBuffer);
  };
};

module TAABuffer = {
  let buildData = (device, state) => {
    let bufferData = Float32Array.fromLength(2);
    let bufferSize = bufferData |> Float32Array.byteLength;
    let buffer =
      device
      |> Device.createBuffer({
           "size": bufferSize,
           "usage": BufferUsage.copy_dst lor BufferUsage.uniform,
         });

    (bufferData, bufferSize, buffer);
  };

  let unsafeGetTAABufferData = state => {
    Pass.unsafeGetUniformBufferData("taaBuffer", state);
  };

  let getTAABufferSize = (bufferData) =>{
bufferData |> Float32Array.byteLength
  };

  let _setTAABufferData = ((bufferData, buffer), state) => {
    Pass.setUniformBufferData("taaBuffer", (bufferData, buffer), state);
  };

  let update = (jitter, state) => {
    let (taaBufferData, taaBuffer) = unsafeGetTAABufferData(state);

    let (taaBufferData, offset) =
      taaBufferData |> TypeArray.Float32Array.setFloatTuple2(0, jitter);

    // Log.printComplete("taaBufferData:", taaBufferData);

    taaBuffer |> Buffer.setSubFloat32Data(0, taaBufferData);
    let state = state |> _setTAABufferData((taaBufferData, taaBuffer));

    state;
  };
};

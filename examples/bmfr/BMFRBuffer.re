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
        lastViewProjectionMatrixOpt,
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
      switch (lastViewProjectionMatrixOpt) {
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

             //  Log.printComplete(
             //    "(lastModelMatrix, modelMatrix):",
             //    (lastModelMatrix, modelMatrix),
             //  );

             let (modelBufferData, newOffset) =
               (modelBufferData, offset)
               |> ManageBuffer.setMat3DataToBufferData(normalMatrix);

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
    let length = DirectionLight.getLightCount(state);

    let (directionLightData, _) =
      DirectionLight.getAllLights(state)
      |> ArrayUtils.reduceOneParam(
           (. (directionLightData, offset), light) => {
             let (directionLightData, offset) =
               directionLightData
               |> TypeArray.Float32Array.setFloat(
                    offset,
                    DirectionLight.unsafeGetIntensity(light, state),
                  );

             let (directionLightData, offset) =
               directionLightData
               |> TypeArray.Float32Array.setFloatTuple3(
                    offset + 3,
                    DirectionLight.unsafeGetPosition(light, state),
                  );

             (directionLightData, offset + 1);
           },
           (Float32Array.fromLength(length * (4 + 4)), 0),
         );

    // Log.printComplete("direlightdata:", directionLightData);

    let directionLightBufferSize =
      directionLightData |> Float32Array.byteLength;
    let directionLightBuffer =
      device
      |> Device.createBuffer({
           "size": directionLightBufferSize,
           "usage": BufferUsage.copy_dst lor BufferUsage.storage,
         });
    directionLightBuffer |> Buffer.setSubFloat32Data(0, directionLightData);

    (directionLightBufferSize, directionLightBuffer);
  };
};

module ResolutionBuffer = {
  let buildData = (window, device) => {
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
    (resolutionData, resolutionUniformBuffer);
  };

  let unsafeGetBufferData = state => {
    Pass.unsafeGetUniformBufferData("resolutionBuffer", state);
  };

  let setBufferData = ((bufferData, buffer), state) => {
    Pass.setUniformBufferData(
      "resolutionBuffer",
      (bufferData, buffer),
      state,
    );
  };

  let getBufferSize = bufferData => {
    bufferData |> Float32Array.byteLength;
  };
};

module PixelBuffer = {
  let buildData = (window, device) => {
    ManageBuffer.StorageBuffer.buildPixelBufferData(window, device);
  };

  let unsafeGetBufferData = state => {
    Pass.unsafeGetStorageBufferData("pixelBuffer", state);
  };

  let setBufferData = ((bufferSize, buffer), state) => {
    Pass.setStorageBufferData("pixelBuffer", (bufferSize, buffer), state);
  };
};

module PrevNoisyPixelBuffer = {
  let buildData = (window, device) => {
    ManageBuffer.StorageBuffer.buildPixelBufferData(window, device);
  };

  let unsafeGetBufferData = state => {
    Pass.unsafeGetStorageBufferData("prevNoisyPixelBuffer", state);
  };

  let setBufferData = ((bufferSize, buffer), state) => {
    Pass.setStorageBufferData(
      "prevNoisyPixelBuffer",
      (bufferSize, buffer),
      state,
    );
  };
};

module PrevPositionBuffer = {
  let buildData = (window, device) => {
    let bufferSize =
      Window.getWidth(window)
      * Window.getHeight(window)
      * 4
      * Float32Array._BYTES_PER_ELEMENT;
    let buffer =
      device
      |> Device.createBuffer({
           "size": bufferSize,
           "usage": BufferUsage.storage,
         });

    (bufferSize, buffer);
  };

  let unsafeGetBufferData = state => {
    Pass.unsafeGetStorageBufferData("prevPositionBuffer", state);
  };

  let setBufferData = ((bufferSize, buffer), state) => {
    Pass.setStorageBufferData(
      "prevPositionBuffer",
      (bufferSize, buffer),
      state,
    );
  };
};

module PrevNormalBuffer = {
  let buildData = (window, device) => {
    let bufferSize =
      Window.getWidth(window)
      * Window.getHeight(window)
      * 4
      * Float32Array._BYTES_PER_ELEMENT;
    let buffer =
      device
      |> Device.createBuffer({
           "size": bufferSize,
           "usage": BufferUsage.storage,
         });

    (bufferSize, buffer);
  };

  let unsafeGetBufferData = state => {
    Pass.unsafeGetStorageBufferData("prevNormalBuffer", state);
  };

  let setBufferData = ((bufferSize, buffer), state) => {
    Pass.setStorageBufferData(
      "prevNormalBuffer",
      (bufferSize, buffer),
      state,
    );
  };
};

module AcceptBoolBuffer = {
  let buildData = (window, device) => {
    let bufferSize =
      Window.getWidth(window)
      * Window.getHeight(window)
      * 1
      * Uint32Array._BYTES_PER_ELEMENT;
    let buffer =
      device
      |> Device.createBuffer({
           "size": bufferSize,
           "usage": BufferUsage.storage,
         });

    (bufferSize, buffer);
  };

  let unsafeGetBufferData = state => {
    Pass.unsafeGetStorageBufferData("acceptBoolBuffer", state);
  };

  let setBufferData = ((bufferSize, buffer), state) => {
    Pass.setStorageBufferData(
      "acceptBoolBuffer",
      (bufferSize, buffer),
      state,
    );
  };
};

module PrevFramePixelIndicesBuffer = {
  let buildData = (window, device) => {
    let bufferSize =
      Window.getWidth(window)
      * Window.getHeight(window)
      * 2
      * Float32Array._BYTES_PER_ELEMENT;
    let buffer =
      device
      |> Device.createBuffer({
           "size": bufferSize,
           "usage": BufferUsage.storage,
         });

    (bufferSize, buffer);
  };

  let unsafeGetBufferData = state => {
    Pass.unsafeGetStorageBufferData("prevFramePixelIndicesBuffer", state);
  };

  let setBufferData = ((bufferSize, buffer), state) => {
    Pass.setStorageBufferData(
      "prevFramePixelIndicesBuffer",
      (bufferSize, buffer),
      state,
    );
  };
};

module HistoryPixelBuffer = {
  let buildData = (window, device) => {
    ManageBuffer.StorageBuffer.buildPixelBufferData(window, device);
  };

  let unsafeGetBufferData = state => {
    Pass.unsafeGetStorageBufferData("historyPixelBuffer", state);
  };

  let setBufferData = ((bufferSize, buffer), state) => {
    Pass.setStorageBufferData(
      "historyPixelBuffer",
      (bufferSize, buffer),
      state,
    );
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

  let unsafeGetBufferData = state => {
    Pass.unsafeGetUniformBufferData("taaBuffer", state);
  };

  let getBufferSize = bufferData => {
    bufferData |> Float32Array.byteLength;
  };

  let setBufferData = ((bufferData, buffer), state) => {
    Pass.setUniformBufferData("taaBuffer", (bufferData, buffer), state);
  };

  let update = (jitter, state) => {
    let (taaBufferData, taaBuffer) = unsafeGetBufferData(state);

    let (taaBufferData, offset) =
      taaBufferData |> TypeArray.Float32Array.setFloatTuple2(0, jitter);

    // Log.printComplete("taaBufferData:", taaBufferData);

    taaBuffer |> Buffer.setSubFloat32Data(0, taaBufferData);
    let state = state |> setBufferData((taaBufferData, taaBuffer));

    state;
  };
};

module CommonDataBuffer = {
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

  let unsafeGetBufferData = state => {
    Pass.unsafeGetUniformBufferData("commonDataBuffer", state);
  };

  let getBufferSize = bufferData => {
    bufferData |> Float32Array.byteLength;
  };

  let setBufferData = ((bufferData, buffer), state) => {
    Pass.setUniformBufferData(
      "commonDataBuffer",
      (bufferData, buffer),
      state,
    );
  };

  let update = (frame, lightCount, state) => {
    let (commonDataBufferData, commonDataBuffer) =
      unsafeGetBufferData(state);

    let (commonDataBufferData, offset) =
      commonDataBufferData
      |> TypeArray.Float32Array.setFloat(0, frame |> float_of_int);
    let (commonDataBufferData, offset) =
      commonDataBufferData
      |> TypeArray.Float32Array.setFloat(offset, lightCount |> float_of_int);

    // Log.printComplete("commonDataBufferData:", commonDataBufferData);

    commonDataBuffer |> Buffer.setSubFloat32Data(0, commonDataBufferData);
    let state =
      state |> setBufferData((commonDataBufferData, commonDataBuffer));

    state;
  };
};

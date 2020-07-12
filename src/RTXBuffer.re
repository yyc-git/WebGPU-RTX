open Js.Typed_array;

open RaytracingFramework;

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

  let setBufferData = ((bufferData, buffer), state) => {
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
    let state = state |> setBufferData((cameraBufferData, cameraBuffer));

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

  let unsafeGetBufferData = state => {
    Pass.unsafeGetUniformBufferData("modelBuffer", state);
  };

  let _setBufferData = ((bufferData, buffer), state) => {
    Pass.setUniformBufferData("modelBuffer", (bufferData, buffer), state);
  };

  // TODO perf: only update dirty ones
  let update = (allRenderGameObjects, state) => {
    let (modelBufferData, modelBuffer) = unsafeGetBufferData(state);

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

    let state = state |> _setBufferData((modelBufferData, modelBuffer));

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
  let buildData = (device, window) => {
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
  let buildData = (device, window) => {
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
  let buildData = (device, window) => {
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
  let buildData = (device, window) => {
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
  let buildData = (device, window) => {
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
  let buildData = (device, window) => {
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
  let buildData = (device, window) => {
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

module Regression = {
  let _getBlockEdgeLength = () => 32;
  let _getBlockPixels = () => 1024;

  let computeHorizentalBlocksCount = window => {
    let blockEdgeLength = _getBlockEdgeLength();

    let blockWidth = (Window.getWidth(window) + 31) / blockEdgeLength;

    blockWidth + 1;
  };

  let computeVerticalBlocksCount = window => {
    let blockEdgeLength = _getBlockEdgeLength();

    let blockHeight = (Window.getHeight(window) + 31) / blockEdgeLength;

    blockHeight + 1;
  };

  let buildData = (device, window) => {
    let blockPixels = _getBlockPixels();

    let w = computeHorizentalBlocksCount(window);
    let h = computeVerticalBlocksCount(window);

    let bufferSize =
      blockPixels * w * h * 13 * Float32Array._BYTES_PER_ELEMENT;
    let buffer =
      device
      |> Device.createBuffer({
           "size": bufferSize,
           "usage": BufferUsage.storage,
         });

    (bufferSize, buffer);
  };

  module TmpDataBuffer = {
    let buildData = (device, window) => {
      buildData(device, window);
    };

    let unsafeGetBufferData = state => {
      Pass.unsafeGetStorageBufferData("tmpDataBuffer", state);
    };

    let setBufferData = ((bufferSize, buffer), state) => {
      Pass.setStorageBufferData(
        "tmpDataBuffer",
        (bufferSize, buffer),
        state,
      );
    };
  };

  module OutDataBuffer = {
    let buildData = (device, window) => {
      buildData(device, window);
    };

    let unsafeGetBufferData = state => {
      Pass.unsafeGetStorageBufferData("outDataBuffer", state);
    };

    let setBufferData = ((bufferSize, buffer), state) => {
      Pass.setStorageBufferData(
        "outDataBuffer",
        (bufferSize, buffer),
        state,
      );
    };
  };

  module CommonDataBuffer = {
    let buildData = (device, window) => {
      let bufferData = Float32Array.fromLength(2);

      let (bufferData, _) =
        bufferData
        |> TypeArray.Float32Array.setFloat(
             1,
             computeHorizentalBlocksCount(window) |> float_of_int,
           );

      let bufferSize = bufferData |> Float32Array.byteLength;
      let buffer =
        device
        |> Device.createBuffer({
             "size": bufferSize,
             "usage": BufferUsage.copy_dst lor BufferUsage.uniform,
           });

      buffer |> Buffer.setSubFloat32Data(0, bufferData);

      (bufferData, bufferSize, buffer);
    };

    let unsafeGetBufferData = state => {
      Pass.unsafeGetUniformBufferData("regressionCommonDataBuffer", state);
    };

    let getBufferSize = bufferData => {
      bufferData |> Float32Array.byteLength;
    };

    let setBufferData = ((bufferData, buffer), state) => {
      Pass.setUniformBufferData(
        "regressionCommonDataBuffer",
        (bufferData, buffer),
        state,
      );
    };

    let update = (frame, state) => {
      let (commonDataBufferData, commonDataBuffer) =
        unsafeGetBufferData(state);

      let (commonDataBufferData, offset) =
        commonDataBufferData
        |> TypeArray.Float32Array.setFloat(0, frame |> float_of_int);

      commonDataBuffer |> Buffer.setSubFloat32Data(0, commonDataBufferData);

      let state =
        state |> setBufferData((commonDataBufferData, commonDataBuffer));

      state;
    };
  };
};

module AccumulatedPrevFramePixelBuffer = {
  let buildData = (device, window) => {
    ManageBuffer.StorageBuffer.buildPixelBufferData(window, device);
  };

  let unsafeGetBufferData = state => {
    Pass.unsafeGetStorageBufferData("accumulatedPrevFramePixelBuffer", state);
  };

  let setBufferData = ((bufferSize, buffer), state) => {
    Pass.setStorageBufferData(
      "accumulatedPrevFramePixelBuffer",
      (bufferSize, buffer),
      state,
    );
  };
};

module HistoryPixelBuffer = {
  let buildData = (device, window) => {
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

module GetHitShadingData = {
  module SceneDescBuffer = {
    let buildData = (device, state) => {
      let gameObjectCount =
        GameObject.getAllGeometryGameObjects(state) |> Js.Array.length;
      let dataCount = 4 + 12 + 16;

      let bufferData = Float32Array.fromLength(gameObjectCount * dataCount);
      let bufferSize = bufferData |> Float32Array.byteLength;
      let buffer =
        device
        |> Device.createBuffer({
             "size": bufferSize,
             "usage": BufferUsage.copy_dst lor BufferUsage.storage,
           });

      (bufferData, bufferSize, buffer);
    };

    let unsafeGetBufferData = state => {
      Pass.unsafeGetStorageBufferData("sceneDescBuffer", state);
    };

    let unsafeGetBufferTypeArrayData = state => {
      Pass.unsafeGetFloat32StorageBufferTypeArrayData(
        "sceneDescBuffer",
        state,
      );
    };

    let getBufferSize = bufferData => {
      bufferData |> Float32Array.byteLength;
    };

    let setBufferData = ((bufferSize, bufferData, buffer), state) => {
      state
      |> Pass.setFloat32StorageBufferTypeArrayData(
           "sceneDescBuffer",
           bufferData,
         )
      |> Pass.setStorageBufferData("sceneDescBuffer", (bufferSize, buffer));
    };

    // TODO perf: only update dirty ones
    let update = (allRenderGameObjects, state) => {
      let (bufferSize, buffer) = unsafeGetBufferData(state);
      let bufferData = unsafeGetBufferTypeArrayData(state);

      // Log.print("update scene desc buffer...") |> ignore;
      // Log.printComplete(
      // "renderGameObjects:",
      // allRenderGameObjects
      // );

      let (bufferData, _) =
        allRenderGameObjects
        |> ArrayUtils.reduceOneParam(
             (. (bufferData, offset), renderGameObject) => {
               let (bufferData, newOffset) =
                 bufferData
                 |> TypeArray.Float32Array.setFloatTuple2(
                      offset,
                      (
                        GameObject.unsafeGetGeometry(renderGameObject, state)
                        |> float_of_int,
                        GameObject.unsafeGetPBRMaterial(
                          renderGameObject,
                          state,
                        )
                        |> float_of_int,
                      ),
                    );

               let newOffset = newOffset + 2;

               let transform =
                 GameObject.unsafeGetTransform(renderGameObject, state);

               let modelMatrix = Transform.buildModelMatrix(transform, state);
               let normalMatrix = Transform.buildNormalMatrix(modelMatrix);

               let (bufferData, newOffset) =
                 (bufferData, newOffset)
                 |> ManageBuffer.setMat3DataToBufferData(normalMatrix);

               let (bufferData, newOffset) =
                 bufferData
                 |> TypeArray.Float32Array.setFloat32Array(
                      newOffset,
                      modelMatrix,
                    );

               (bufferData, newOffset);
             },
             (bufferData, 0),
           );

      // Log.printComplete(
      // "bufferData:",
      // bufferData
      // );

      buffer |> Buffer.setSubFloat32Data(0, bufferData);

      let state = state |> setBufferData((bufferSize, bufferData, buffer));

      state;
    };
  };

  module GeometryOffsetDataBuffer = {
    let buildData = (device, state) => {
      let geometryCount = Geometry.getCount(state);
      let dataCount = 2;

      let bufferData = Uint32Array.fromLength(geometryCount * dataCount);
      let bufferSize = bufferData |> Uint32Array.byteLength;
      let buffer =
        device
        |> Device.createBuffer({
             "size": bufferSize,
             "usage": BufferUsage.copy_dst lor BufferUsage.storage,
           });

      let stride = dataCount;

      let (bufferData, _) =
        Geometry.getAllGeometries(state)
        |> ArrayUtils.reduceOneParam(
             (. (bufferData, (vertexOffset, indexOffset)), geometry) => {
               let (vertices, _) =
                 Geometry.unsafeGetVertexData(geometry, state);

               let newVertexOffset =
                 vertexOffset + Geometry.computeVertexCount(vertices);

               let (bufferData, newOffset) =
                 bufferData
                 |> TypeArray.Uint32Array.setUint(
                      geometry * stride,
                      vertexOffset,
                    );

               let indices = Geometry.unsafeGetIndexData(geometry, state);

              //  Log.printComplete(
              //    "indices:",
              //    (indices, Geometry.computeIndexCount(indices)),
              //  );

               let newIndexOffset =
                 indexOffset + Geometry.computeIndexCount(indices);

               let (bufferData, _) =
                 bufferData
                 |> TypeArray.Uint32Array.setUint(newOffset, indexOffset);

               (bufferData, (newVertexOffset, newIndexOffset));
             },
             (bufferData, (0, 0)),
           );

      // Log.printComplete("geometry offset bufferData:", bufferData);

      buffer |> Buffer.setSubUint32Data(0, bufferData);

      (bufferData, bufferSize, buffer);
    };

    let unsafeGetBufferData = state => {
      Pass.unsafeGetStorageBufferData("geometryOffsetDataBuffer", state);
    };

    let getBufferSize = bufferData => {
      bufferData |> Uint32Array.byteLength;
    };

    let setBufferData = ((bufferData, buffer), state) => {
      Pass.setStorageBufferData(
        "geometryOffsetDataBuffer",
        (bufferData, buffer),
        state,
      );
    };

    // TODO update dirty ones
    let update = (allRenderGameObjects, state) => {
      state;
    };
  };

  module VertexBuffer = {
    let _computeSceneVertexBufferDataLength = state => {
      (
        Geometry.getAllVertexData(state)
        |> ArrayUtils.reduceOneParam(
             (. vertexDataTotalCount, (vertices, normals)) => {
               vertexDataTotalCount
               + Geometry.computeVerticesCount(vertices)
               + Geometry.computeNormalsCount(normals)
             },
             0,
           )
      )
      * 4;
    };

    let buildData = (device, state) => {
      let bufferData =
        Float32Array.fromLength(_computeSceneVertexBufferDataLength(state));
      let bufferSize = bufferData |> Float32Array.byteLength;
      let buffer =
        device
        |> Device.createBuffer({
             "size": bufferSize,
             "usage": BufferUsage.copy_dst lor BufferUsage.storage,
           });

      let (bufferData, _) =
        Geometry.getAllGeometries(state)
        |> ArrayUtils.reduceOneParam(
             (. (bufferData, offset), geometry) => {
               let (vertices, normals) =
                 Geometry.unsafeGetVertexData(geometry, state);

               let vertexCount = Geometry.computeVertexCount(vertices);

               ArrayUtils.range(0, vertexCount)
               |> ArrayUtils.reduceOneParam(
                    (. (bufferData, offset), index) => {
                      let (bufferData, newOffset) =
                        bufferData
                        |> TypeArray.Float32Array.setFloatTuple3(
                             offset,
                             (
                               Array.unsafe_get(vertices, index * 3),
                               Array.unsafe_get(vertices, index * 3 + 1),
                               Array.unsafe_get(vertices, index * 3 + 2),
                             ),
                           );

                      let newOffset = newOffset + 1;

                      let (bufferData, newOffset) =
                        bufferData
                        |> TypeArray.Float32Array.setFloatTuple3(
                             newOffset,
                             (
                               Array.unsafe_get(normals, index * 3),
                               Array.unsafe_get(normals, index * 3 + 1),
                               Array.unsafe_get(normals, index * 3 + 2),
                             ),
                           );

                      let newOffset = newOffset + 1;

                      (bufferData, newOffset);
                    },
                    (bufferData, offset),
                  );
             },
             (bufferData, 0),
           );

      // Log.printComplete("vertex bufferData:", bufferData);

      buffer |> Buffer.setSubFloat32Data(0, bufferData);

      (bufferData, bufferSize, buffer);
    };

    let unsafeGetBufferData = state => {
      Pass.unsafeGetStorageBufferData("vertexBuffer", state);
    };

    let getBufferSize = bufferData => {
      bufferData |> Float32Array.byteLength;
    };

    let setBufferData = ((bufferData, buffer), state) => {
      Pass.setStorageBufferData("vertexBuffer", (bufferData, buffer), state);
    };

    // TODO update dirty ones
    let update = (allRenderGameObjects, state) => {
      state;
    };
  };

  module IndexBuffer = {
    let buildData = (device, state) => {
      let bufferData =
        Uint32Array.fromLength(
          Geometry.getAllIndexData(state)
          |> ArrayUtils.reduceOneParam(
               (. vertexDataTotalCount, indices) => {
                 vertexDataTotalCount + Geometry.computeIndexCount(indices)
               },
               0,
             ),
        );

      let bufferSize = bufferData |> Uint32Array.byteLength;
      let buffer =
        device
        |> Device.createBuffer({
             "size": bufferSize,
             "usage": BufferUsage.copy_dst lor BufferUsage.storage,
           });

      let (bufferData, _) =
        Geometry.getAllGeometries(state)
        |> ArrayUtils.reduceOneParam(
             (. (bufferData, offset), geometry) => {
               let indices = Geometry.unsafeGetIndexData(geometry, state);

               bufferData |> TypeArray.Uint32Array.setArray(offset, indices);
             },
             (bufferData, 0),
           );

      // Log.printComplete("index bufferData:", bufferData);

      buffer |> Buffer.setSubUint32Data(0, bufferData);

      (bufferData, bufferSize, buffer);
    };

    let unsafeGetBufferData = state => {
      Pass.unsafeGetStorageBufferData("indexBuffer", state);
    };

    let getBufferSize = bufferData => {
      bufferData |> Uint32Array.byteLength;
    };

    let setBufferData = ((bufferData, buffer), state) => {
      Pass.setStorageBufferData("indexBuffer", (bufferData, buffer), state);
    };

    // TODO update dirty ones
    let update = (allRenderGameObjects, state) => {
      state;
    };
  };

  module PBRMaterialBuffer = {
    let buildData = (device, state) => {
      let pbrMaterialCount = PBRMaterial.getCount(state);
      let dataCount = 4 + 4;

      let bufferData = Float32Array.fromLength(pbrMaterialCount * dataCount);

      let bufferSize = bufferData |> Float32Array.byteLength;
      let buffer =
        device
        |> Device.createBuffer({
             "size": bufferSize,
             "usage": BufferUsage.copy_dst lor BufferUsage.storage,
           });

      let (bufferData, _) =
        PBRMaterial.getAllPBRMaterials(state)
        |> ArrayUtils.reduceOneParam(
             (. (bufferData, offset), material) => {
               let diffuse = PBRMaterial.unsafeGetDiffuse(material, state);
               let metalness =
                 PBRMaterial.unsafeGetMetalness(material, state);
               let roughness =
                 PBRMaterial.unsafeGetRoughness(material, state);
               let specular = PBRMaterial.unsafeGetSpecular(material, state);

               let (bufferData, newOffset) =
                 bufferData
                 |> TypeArray.Float32Array.setFloatTuple3(offset, diffuse);

               let newOffset = newOffset + 1;

               let (bufferData, newOffset) =
                 bufferData
                 |> TypeArray.Float32Array.setFloatTuple3(
                      newOffset,
                      (metalness, roughness, specular),
                    );

               (bufferData, newOffset + 1);
             },
             (bufferData, 0),
           );

      // Log.printComplete("material bufferData:", bufferData);

      buffer |> Buffer.setSubFloat32Data(0, bufferData);

      (bufferData, bufferSize, buffer);
    };

    let unsafeGetBufferData = state => {
      Pass.unsafeGetStorageBufferData("pbrMaterialBuffer", state);
    };

    let getBufferSize = bufferData => {
      bufferData |> Float32Array.byteLength;
    };

    let setBufferData = ((bufferData, buffer), state) => {
      Pass.setStorageBufferData(
        "pbrMaterialBuffer",
        (bufferData, buffer),
        state,
      );
    };

    // TODO update dirty ones
    let update = (allRenderGameObjects, state) => {
      state;
    };
  };
};

module RayTracingCommonDataBuffer = {
  let buildData = (device, state) => {
    let bufferData = Float32Array.fromLength(1);

    // let (bufferData, _) =
    //   bufferData
    //   |> TypeArray.Float32Array.setFloat(
    //        0,
    //        Pass.RayTracingPass.getIndirectLightSpecularSampleCount(state)
    //        |> float_of_int,
    //      );

    let bufferSize = bufferData |> Float32Array.byteLength;
    let buffer =
      device
      |> Device.createBuffer({
           "size": bufferSize,
           "usage": BufferUsage.copy_dst lor BufferUsage.uniform,
         });

    buffer |> Buffer.setSubFloat32Data(0, bufferData);

    (bufferData, bufferSize, buffer);
  };

  let unsafeGetBufferData = state => {
    Pass.unsafeGetUniformBufferData("rayTracingCommonDataBuffer", state);
  };

  let getBufferSize = bufferData => {
    bufferData |> Float32Array.byteLength;
  };

  let setBufferData = ((bufferData, buffer), state) => {
    Pass.setUniformBufferData(
      "rayTracingCommonDataBuffer",
      (bufferData, buffer),
      state,
    );
  };
};

module AccumulationPixelBuffer = {
  let buildData = (device, window) => {
    // ManageBuffer.StorageBuffer.buildPixelBufferData(window, device);
    let pixelBufferSize =
      Window.getWidth(window)
      * Window.getHeight(window)
      * 4
      * Float32Array._BYTES_PER_ELEMENT;
    let pixelBuffer =
      device
      |> Device.createBuffer({
           "size": pixelBufferSize,
           "usage": BufferUsage.copy_dst lor BufferUsage.storage,
         });

    (pixelBufferSize, pixelBuffer);
  };

  let unsafeGetBufferData = state => {
    Pass.unsafeGetStorageBufferData("accumulationPixelBuffer", state);
  };

  let setBufferData = ((bufferSize, buffer), state) => {
    state
    |> Pass.setStorageBufferData(
         "accumulationPixelBuffer",
         (bufferSize, buffer),
       );
  };

  let clear = state => {
    let (bufferSize, buffer) = unsafeGetBufferData(state);

    let bufferData =
      Float32Array.fromLength(bufferSize / Float32Array._BYTES_PER_ELEMENT);

    buffer |> Buffer.setSubFloat32Data(0, bufferData);

    let state = state |> setBufferData((bufferSize, buffer));

    state;
  };
};

module AccumulationCommonDataBuffer = {
  let buildData = (device, state) => {
    let bufferData = Float32Array.fromLength(2);

    let (bufferData, _) =
      bufferData
      |> TypeArray.Float32Array.setFloatTuple2(
           0,
           (
             Pass.AccumulationPass.getAccumFrameCount(state) |> float_of_int,
             Pass.AccumulationPass.convertCanDenoiseToFloat(state),
           ),
         );

    // Log.print((
    //   "  Pass.AccumulationPass.convertCanDenoiseToFloat(state): ",
    //   Pass.AccumulationPass.convertCanDenoiseToFloat(state),
    // ))
    // |> ignore;

    let bufferSize = bufferData |> Float32Array.byteLength;
    let buffer =
      device
      |> Device.createBuffer({
           "size": bufferSize,
           "usage": BufferUsage.copy_dst lor BufferUsage.uniform,
         });

    buffer |> Buffer.setSubFloat32Data(0, bufferData);

    (bufferData, bufferSize, buffer);
  };

  let unsafeGetBufferData = state => {
    Pass.unsafeGetUniformBufferData("accumulationCommonDataBuffer", state);
  };

  let getBufferSize = bufferData => {
    bufferData |> Float32Array.byteLength;
  };

  let setBufferData = ((bufferData, buffer), state) => {
    Pass.setUniformBufferData(
      "accumulationCommonDataBuffer",
      (bufferData, buffer),
      state,
    );
  };

  let update = (accumFrameCount, canDenoise, state) => {
    let (bufferData, buffer) = unsafeGetBufferData(state);

    let (bufferData, _) =
      bufferData
      |> TypeArray.Float32Array.setFloatTuple2(
           0,
           (accumFrameCount |> float_of_int, canDenoise),
         );

    buffer |> Buffer.setSubFloat32Data(0, bufferData);

    let state = state |> setBufferData((bufferData, buffer));

    state;
  };
};

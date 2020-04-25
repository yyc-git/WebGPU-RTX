open Js.Typed_array;

open WebGPU;

module InstanceBuffer = {
  let _getInstanceBufferStride = () => 64;

  let _computeInstanceOffset = instanceIndex => {
    instanceIndex * _getInstanceBufferStride();
  };

  let createInstanceBuffer = (geometryCount, device) => {
    let byteLength = geometryCount * _getInstanceBufferStride();

    (
      ArrayBuffer.make(byteLength),
      device
      |> Device.createBuffer({
           "size": byteLength,
           "usage": BufferUsage.copy_dst,
         }),
    );
  };

  let setInstanceData =
      (
        instanceIndex,
        (
          transformMatrix,
          instanceId,
          mask,
          instanceOffset,
          flags,
          blasHandle,
        ),
        instanceBufferArrayBuffer,
      ) => {
    let dataView = DataView.make(instanceBufferArrayBuffer);
    let offset = _computeInstanceOffset(instanceIndex);

    let offset =
      ArrayUtils.range(0, 12)
      |> ArrayUtils.reduceOneParam(
           (. offset, i) => {
             DataView.setFloat32LittleEndian(
               dataView,
               offset,
               Float32Array.unsafe_get(transformMatrix, i),
             );

             offset + 4;
           },
           offset,
         );

    DataView.setUint32LittleEndian(
      dataView,
      offset,
      mask lsl 24 lor instanceId,
    );
    DataView.setUint32LittleEndian(
      dataView,
      offset + 4,
      flags lsl 24 lor instanceOffset,
    );
    DataViewUtils.setBigInt64LittleEndian(dataView, offset + 8, blasHandle);

    instanceBufferArrayBuffer;
  };

  let setInstanceBufferData = (instanceBufferArrayBuffer, instanceBuffer) => {
    Buffer.setSubUint8Data(
      0,
      Uint8Array.fromBuffer(instanceBufferArrayBuffer),
      instanceBuffer,
    );

    instanceBuffer;
  };

  let _convertMat4To34RowMajorMatrix = (mat4): Float32Array.t => {
    Float32Array.make([|
      Float32Array.unsafe_get(mat4, 0),
      Float32Array.unsafe_get(mat4, 4),
      Float32Array.unsafe_get(mat4, 8),
      Float32Array.unsafe_get(mat4, 12),
      Float32Array.unsafe_get(mat4, 1),
      Float32Array.unsafe_get(mat4, 5),
      Float32Array.unsafe_get(mat4, 9),
      Float32Array.unsafe_get(mat4, 13),
      Float32Array.unsafe_get(mat4, 2),
      Float32Array.unsafe_get(mat4, 6),
      Float32Array.unsafe_get(mat4, 10),
      Float32Array.unsafe_get(mat4, 14),
    |]);
  };

  let convertInstanceTransformDataToContainerTransformMatrix =
      ((translation, rotation, scale)) => {
    Matrix4.createIdentityMatrix4()
    |> Matrix4.fromTranslationRotationScale(
         translation,
         Quaternion.setFromEulerAngles(rotation),
         scale,
       )
    |> _convertMat4To34RowMajorMatrix;
  };

  let convertHitGroupIndexToInstanceOffset = hitGroupIndex => {
    // Log.print(("groupIndex:", hitGroupIndex)) |> ignore;

    MathUtils.convertDecimalToHex(hitGroupIndex, 16);
  };
};

let _getGeomtryContainerHandle = (geometryContainer: AccelerationContainer.t) => {
  Obj.magic(geometryContainer)##getHandle();
};

let _buildSceneGeometryContainers = (device, state) => {
  ArrayUtils.zipWith(
    ((vertices, normals), indices) => {(vertices, indices)},
    Geometry.getAllVertexData(state),
    Geometry.getAllIndexData(state),
  )
  |> ArrayUtils.reduceOneParam(
       (. geometryContainers, (vertices, indices)) => {
         let geometryVertices = Float32Array.make(vertices);
         let geometryVertexBuffer =
           device
           |> Device.createBuffer({
                "size": geometryVertices |> Float32Array.byteLength,
                "usage": BufferUsage.copy_dst,
              });
         Buffer.setSubFloat32Data(0, geometryVertices, geometryVertexBuffer);

         let geometryIndices = Uint32Array.make(indices);
         let geometryIndexBuffer =
           device
           |> Device.createBuffer({
                "size": geometryIndices |> Uint32Array.byteLength,
                "usage": BufferUsage.copy_dst,
              });

         Buffer.setSubUint32Data(0, geometryIndices, geometryIndexBuffer);

         Log.printComplete("accle:", (geometryVertices, geometryIndices));

         geometryContainers
         |> ArrayUtils.push(
              device
              |> Device.createRayTracingAccelerationContainer(
                   {
                     AccelerationContainer.descriptor(
                       ~flags=AccelerationContainerFlag.prefer_fast_trace,
                       ~level="bottom",
                       ~geometries=[|
                         {
                           "flags": AccelerationGeometryFlag.opaque,
                           "type": "triangles",
                           "vertex": {
                             "buffer": geometryVertexBuffer,
                             "format": "float3",
                             "stride": 3 * Float32Array._BYTES_PER_ELEMENT,
                             "count": Float32Array.length(geometryVertices),
                           },
                           "index": {
                             "buffer": geometryIndexBuffer,
                             "format": "uint32",
                             "count": Uint32Array.length(geometryIndices),
                           },
                         },
                       |],
                       (),
                     );
                   },
                 ),
            );
       },
       [||],
     );
};

let _updateInstanceBuffer =
    (geometryContainers, state, (instanceBufferArrayBuffer, instanceBuffer)) => {
  let (instanceBufferArrayBuffer, _) =
    GameObject.getAllGeometryGameObjects(state)
    |> ArrayUtils.reduceOneParam(
         (. (instanceBufferArrayBuffer, instanceIndex), gameObject) => {
           let transform = GameObject.unsafeGetTransform(gameObject, state);

           let geometryContainer =
             Array.unsafe_get(
               geometryContainers,
               GameObject.unsafeGetGeometry(gameObject, state),
             );

           (
             InstanceBuffer.setInstanceData(
               instanceIndex,
               (
                 InstanceBuffer.convertInstanceTransformDataToContainerTransformMatrix((
                   Transform.getTranslation(transform, state),
                   Transform.getRotation(transform, state),
                   Transform.getScale(transform, state),
                 )),
                 instanceIndex,
                 0xFF,
                 InstanceBuffer.convertHitGroupIndexToInstanceOffset(
                   Shader.unsafeGetHitGroupIndex(
                     GameObject.unsafeGetShader(gameObject, state),
                     state,
                   ),
                 ),
                 AccelerationInstanceFlag.triangle_cull_disable,
                 _getGeomtryContainerHandle(
                   Array.unsafe_get(
                     geometryContainers,
                     GameObject.unsafeGetGeometry(gameObject, state),
                   ),
                 ),
               ),
               instanceBufferArrayBuffer,
             ),
             instanceIndex |> succ,
           );
         },
         (instanceBufferArrayBuffer, 0),
       );

  let instanceBuffer =
    InstanceBuffer.setInstanceBufferData(
      instanceBufferArrayBuffer,
      instanceBuffer,
    );

  (instanceBufferArrayBuffer, instanceBuffer);
};

let _createInstanceContainer = (geometryContainers, device, state) => {
  let (instanceBufferArrayBuffer, instanceBuffer) =
    InstanceBuffer.createInstanceBuffer(Geometry.getCount(state), device)
    |> _updateInstanceBuffer(geometryContainers, state);

  (
    (instanceBufferArrayBuffer, instanceBuffer),
    device
    |> Device.createRayTracingAccelerationContainer(
         {
           AccelerationContainer.descriptor(
             ~flags=AccelerationContainerFlag.allow_update,
             ~level="top",
             ~instanceBuffer,
             (),
           );
         },
       ),
  );
};

let buildContainers = (device, queue, state) => {
  let geometryContainers: array(AccelerationContainer.t) =
    _buildSceneGeometryContainers(device, state);
  let ((instanceBufferArrayBuffer, instanceBuffer), instanceContainer) =
    _createInstanceContainer(geometryContainers, device, state);

  let commandEncoder =
    device |> Device.createCommandEncoder(CommandEncoder.descriptor());
  geometryContainers
  |> Js.Array.forEach(geometryContainer => {
       commandEncoder
       |> CommandEncoder.buildRayTracingAccelerationContainer(
            geometryContainer,
          )
     });
  commandEncoder
  |> CommandEncoder.buildRayTracingAccelerationContainer(instanceContainer);
  queue |> Queue.submit([|commandEncoder |> CommandEncoder.finish|]);

  (
    geometryContainers,
    (instanceBufferArrayBuffer, instanceBuffer),
    instanceContainer,
  );
};

let updateInstanceContainer = (device, queue, state) => {
  let (
    geometryContainers,
    instanceContainer,
    instanceBufferArrayBuffer,
    instanceBuffer,
  ) =
    OperateAccelerationContainer.unsafeGetData(state);

  let (instanceBufferArrayBuffer, instanceBuffer) =
    _updateInstanceBuffer(
      geometryContainers,
      state,
      (instanceBufferArrayBuffer, instanceBuffer),
    );

  let commandEncoder =
    device |> Device.createCommandEncoder(CommandEncoder.descriptor());
  commandEncoder
  |> CommandEncoder.updateRayTracingAccelerationContainer(instanceContainer);
  queue |> Queue.submit([|commandEncoder |> CommandEncoder.finish|]);

  state
  |> OperateAccelerationContainer.setData(
       geometryContainers,
       instanceContainer,
       instanceBufferArrayBuffer,
       instanceBuffer,
     );
};

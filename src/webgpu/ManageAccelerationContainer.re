open Js.Typed_array;

open WebGPU;

module Instance = {
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

  let _convertInstanceTransformDataToContainerTransformMatrix =
      ((translation, rotation, scale)) => {
    Matrix4.createIdentityMatrix4()
    |> Matrix4.fromTranslationRotationScale(
         translation,
         Quaternion.setFromEulerAngles(rotation),
         scale,
       )
    |> _convertMat4To34RowMajorMatrix;
  };

  let _convertHitGroupIndexToInstanceOffset = hitGroupIndex => {
    MathUtils.convertDecimalToHex(hitGroupIndex, 16);
  };

  let createInstances = (geometryContainerMap, state) => {
    let (instances, _) =
      GameObject.getAllGeometryGameObjects(state)
      |> ArrayUtils.reduceOneParam(
           (. (instances, instanceIndex), gameObject) => {
             let transform = GameObject.unsafeGetTransform(gameObject, state);

             let geometryContainer =
               geometryContainerMap
               |> ImmutableSparseMap.unsafeGet(
                    GameObject.unsafeGetGeometry(gameObject, state),
                  );

             (
               instances
               |> ArrayUtils.push(
                    AccelerationContainer.instance(
                      ~usage=AccelerationInstanceUsage.triangle_cull_disable,
                      ~mask=0xFF,
                      ~instanceId=instanceIndex,
                      ~transformMatrix=
                        _convertInstanceTransformDataToContainerTransformMatrix((
                          Transform.getTranslation(transform, state),
                          Transform.getRotation(transform, state),
                          Transform.getScale(transform, state),
                        )),
                      ~instanceOffset=
                        _convertHitGroupIndexToInstanceOffset(
                          Shader.unsafeGetHitGroupIndex(
                            GameObject.unsafeGetShader(gameObject, state),
                            state,
                          ),
                        ),
                      ~geometryContainer,
                      (),
                    ),
                  ),
               instanceIndex |> succ,
             );
           },
           ([||], 0),
         );

    instances;
  };
};

let _buildSceneGeometryContainers = (device, state) => {
  Geometry.getAllGeometries(state)
  |> ArrayUtils.reduceOneParam(
       (. allData, geometry) => {
         let (vertices, _) = Geometry.unsafeGetVertexData(geometry, state);

         allData
         |> ArrayUtils.push((
              geometry,
              vertices,
              Geometry.unsafeGetIndexData(geometry, state),
            ));
       },
       [||],
     )
  |> ArrayUtils.reduceOneParam(
       (. geometryContainerMap, (geometry, vertices, indices)) => {
         let geometryVertices = Float32Array.make(vertices);
         let geometryVertexBuffer =
           device
           |> Device.createBuffer({
                "size": geometryVertices |> Float32Array.byteLength,
                "usage": BufferUsage.copy_dst lor BufferUsage.ray_tracing,
              });
         Buffer.setSubFloat32Data(0, geometryVertices, geometryVertexBuffer);

         let geometryIndices = Uint32Array.make(indices);
         let geometryIndexBuffer =
           device
           |> Device.createBuffer({
                "size": geometryIndices |> Uint32Array.byteLength,
                "usage": BufferUsage.copy_dst lor BufferUsage.ray_tracing,
              });

         Buffer.setSubUint32Data(0, geometryIndices, geometryIndexBuffer);

        //  Log.printComplete("accle:", (geometryVertices, geometryIndices));

         geometryContainerMap
         |> ImmutableSparseMap.set(
              geometry,
              device
              |> Device.createRayTracingAccelerationContainer(
                   {
                     AccelerationContainer.descriptor(
                       ~usage=AccelerationContainerUsage.prefer_fast_trace,
                       ~level="bottom",
                       ~geometries=[|
                         {
                           "usage": AccelerationGeometryUsage.opaque,
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
       ImmutableSparseMap.createEmpty(),
     );
};

let _createInstanceContainer = (geometryContainerMap, device, state) => {
  device
  |> Device.createRayTracingAccelerationContainer(
       {
         AccelerationContainer.descriptor(
           ~usage=AccelerationContainerUsage.allow_update,
           ~level="top",
           ~instances=Instance.createInstances(geometryContainerMap, state),
           (),
         );
       },
     );
};

let buildContainers = (device, queue, state) => {
  let geometryContainerMap:
    ImmutableSparseMap.t(GeometryType.geometry, AccelerationContainer.t) =
    _buildSceneGeometryContainers(device, state);
  let instanceContainer =
    _createInstanceContainer(geometryContainerMap, device, state);

  let commandEncoder =
    device |> Device.createCommandEncoder(CommandEncoder.descriptor());
  geometryContainerMap
  |> ImmutableSparseMap.getValidValues
  |> Js.Array.forEach(geometryContainer => {
       commandEncoder
       |> CommandEncoder.buildRayTracingAccelerationContainer(
            geometryContainer,
          )
     });
  queue |> Queue.submit([|commandEncoder |> CommandEncoder.finish|]);

  let commandEncoder =
    device |> Device.createCommandEncoder(CommandEncoder.descriptor());
  commandEncoder
  |> CommandEncoder.buildRayTracingAccelerationContainer(instanceContainer);
  queue |> Queue.submit([|commandEncoder |> CommandEncoder.finish|]);

  (geometryContainerMap, instanceContainer);
};

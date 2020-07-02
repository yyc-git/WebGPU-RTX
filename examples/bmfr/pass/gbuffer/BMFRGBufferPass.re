open WebGPU;

open Js.Typed_array;

open StateType;

let _mergeVertexData = ((vertices, normals)) => {
  let vertexCount = Geometry.computeVertexCount(vertices);

  Log.print(("vertexCount:", vertexCount)) |> ignore;

  ArrayUtils.range(0, vertexCount)
  |> ArrayUtils.reduceOneParam(
       (. vertexBufferData, vertexIndex) => {
         Log.print(("vertexIndex: ", vertexIndex)) |> ignore;
         let offset = vertexIndex * 6;
         let componentIndex = vertexIndex * 3;

         Float32Array.unsafe_set(
           vertexBufferData,
           offset,
           Array.unsafe_get(vertices, componentIndex),
         );
         Float32Array.unsafe_set(
           vertexBufferData,
           offset + 1,
           Array.unsafe_get(vertices, componentIndex + 1),
         );
         Float32Array.unsafe_set(
           vertexBufferData,
           offset + 2,
           Array.unsafe_get(vertices, componentIndex + 2),
         );

         Float32Array.unsafe_set(
           vertexBufferData,
           offset + 3,
           Array.unsafe_get(normals, componentIndex),
         );
         Float32Array.unsafe_set(
           vertexBufferData,
           offset + 4,
           Array.unsafe_get(normals, componentIndex + 1),
         );
         Float32Array.unsafe_set(
           vertexBufferData,
           offset + 5,
           Array.unsafe_get(normals, componentIndex + 2),
         );

         vertexBufferData;
       },
       Float32Array.fromLength(vertexCount * 6),
     );
};

let _buildVertexAndIndexBufferMap = (device, allUniqueGeometries, state) => {
  allUniqueGeometries
  |> ArrayUtils.reduceOneParam(
       (. vertexAndIndexBufferMap, geometry) => {
         let vertexBufferData =
           Geometry.unsafeGetVertexData(geometry, state) |> _mergeVertexData;
         let indices = Geometry.unsafeGetIndexData(geometry, state);

         let vertexBuffer =
           device
           |> Device.createBuffer({
                "size": vertexBufferData |> Float32Array.byteLength,
                "usage": BufferUsage.copy_dst lor BufferUsage.vertex,
              });
         vertexBuffer |> Buffer.setSubFloat32Data(0, vertexBufferData);

         let indexBuffer =
           device
           |> Device.createBuffer({
                "size":
                  Js.Array.length(indices) * Uint32Array._BYTES_PER_ELEMENT,
                "usage": BufferUsage.copy_dst lor BufferUsage.index,
              });
         indexBuffer |> Buffer.setSubUint32Data(0, Uint32Array.make(indices));

        //  Log.printComplete(
        //    "vertex data:",
        //    (geometry, vertexBufferData, indices),
        //  );

         vertexAndIndexBufferMap
         |> ImmutableSparseMap.set(geometry, (vertexBuffer, indexBuffer));
       },
       ImmutableSparseMap.createEmpty(),
     );
};

let _buildIndexCountMap = (allUniqueGeometries, state) => {
  allUniqueGeometries
  |> ArrayUtils.reduceOneParam(
       (. indexCountMap, geometry) => {
         indexCountMap
         |> ImmutableSparseMap.set(
              geometry,
              Geometry.unsafeGetIndexData(geometry, state)
              |> Geometry.computeIndexCount,
            )
       },
       ImmutableSparseMap.createEmpty(),
     );
};

let _buildVertexData = (device, allRenderGameObjects, state) => {
  let allUniqueGeometries =
    allRenderGameObjects
    |> Js.Array.map(renderGameObject => {
         GameObject.unsafeGetGeometry(renderGameObject, state)
       })
    |> ArrayUtils2.removeDuplicateItems;

  (
    _buildVertexAndIndexBufferMap(device, allUniqueGeometries, state),
    _buildIndexCountMap(allUniqueGeometries, state),
  );
};

let _createTextureData = (device, window, format, usage) => {
  let texture =
    device
    |> Device.createTexture(
         Texture.descriptor(
           ~size={
             "width": Window.getWidth(window),
             "height": Window.getHeight(window),
             "depth": 1,
           },
           ~arrayLayerCount=1,
           ~mipLevelCount=1,
           ~sampleCount=1,
           ~dimension="2d",
           ~format,
           ~usage,
         ),
       );

  (
    texture,
    texture |> Texture.createView(TextureView.descriptor(~format, ())),
  );
};

let _createRenderTargetData = (device, window, format) => {
  _createTextureData(
    device,
    window,
    format,
    TextureUsage.sampled lor TextureUsage.output_attachment,
  );
};

let init = (device, window, state) => {
  Js.logMany([|"zxczxc" |> Obj.magic|]);
  let modelBindGroupLayout =
    device
    |> Device.createBindGroupLayout({
         "entries": [|
           BindGroupLayout.layoutBinding(
             ~binding=0,
             ~visibility=ShaderStage.vertex,
             ~type_="uniform-buffer",
             ~hasDynamicOffset=true,
             (),
           ),
         |],
       });

  let pbrMaterialBindGroupLayout =
    device
    |> Device.createBindGroupLayout({
         "entries": [|
           BindGroupLayout.layoutBinding(
             ~binding=0,
             ~visibility=ShaderStage.fragment,
             ~type_="uniform-buffer",
             ~hasDynamicOffset=true,
             (),
           ),
         |],
       });

  let cameraBindGroupLayout =
    device
    |> Device.createBindGroupLayout({
         "entries": [|
           BindGroupLayout.layoutBinding(
             ~binding=0,
             ~visibility=ShaderStage.vertex,
             ~type_="uniform-buffer",
             (),
           ),
         |],
       });

  let allRenderGameObjects = BMFRScene.getAllRenderGameObjects(state);

  let state =
    state |> Pass.GBufferPass.setRenderGameObjectArr(allRenderGameObjects);

  let (
    offsetArrMap,
    modelBufferData,
    singleRenderGameObjectModelBufferSize,
    modelBuffer,
  ) =
    BMFRBuffer.ModelBuffer.buildData(device, allRenderGameObjects, state);

  let state =
    state
    |> Pass.setUniformBufferData(
         "modelBuffer",
         (modelBufferData, modelBuffer),
       );

  let modelBindGroup =
    device
    |> Device.createBindGroup({
         "layout": modelBindGroupLayout,
         "entries": [|
           BindGroup.binding(
             ~binding=0,
             ~buffer=modelBuffer,
             ~offset=0,
             ~size=singleRenderGameObjectModelBufferSize,
             (),
           ),
         |],
       });

  let state =
    state
    |> Pass.GBufferPass.addDynamicBindGroupData(
         0,
         modelBindGroup,
         offsetArrMap,
       );

  let singleRenderGameObjectPBRMaterialBufferSize =
    (4 + 4) * Float32Array._BYTES_PER_ELEMENT;
  let alignedPBRMaterialBufferBytes =
    ManageBuffer.UniformBuffer.getAlignedBufferBytes(
      singleRenderGameObjectPBRMaterialBufferSize,
    );
  let alignedPBRMaterialBufferFloats =
    ManageBuffer.UniformBuffer.getAlignedBufferFloats(
      alignedPBRMaterialBufferBytes,
    );
  let pbrMaterialBufferSize =
    (allRenderGameObjects |> Js.Array.length) * alignedPBRMaterialBufferBytes;

  let (pbrMaterialBufferData, _, offsetArrMap) =
    allRenderGameObjects
    |> ArrayUtils.reduceOneParam(
         (. (pbrMaterialBufferData, offset, offsetArrMap), renderGameObject) => {
           let material =
             GameObject.unsafeGetPBRMaterial(renderGameObject, state);

           let (pbrMaterialBufferData, newOffset) =
             pbrMaterialBufferData
             |> TypeArray.Float32Array.setFloatTuple3(
                  offset,
                  PBRMaterial.unsafeGetDiffuse(material, state),
                );

           let (pbrMaterialBufferData, newOffset) =
             pbrMaterialBufferData
             |> TypeArray.Float32Array.setFloatTuple3(
                  newOffset + 1,
                  (
                    PBRMaterial.unsafeGetMetalness(material, state),
                    PBRMaterial.unsafeGetRoughness(material, state),
                    PBRMaterial.unsafeGetSpecular(material, state),
                  ),
                );

           (
             pbrMaterialBufferData,
             offset + alignedPBRMaterialBufferFloats,
             offsetArrMap
             |> ImmutableSparseMap.set(
                  renderGameObject,
                  [|
                    ManageBuffer.UniformBuffer.getAlignedBufferBytesFromFloats(
                      offset,
                    ),
                  |],
                ),
           );
         },
         (
           Float32Array.fromLength(
             pbrMaterialBufferSize / Float32Array._BYTES_PER_ELEMENT,
           ),
           0,
           ImmutableSparseMap.createEmpty(),
         ),
       );

  Log.printComplete(
    "pbrMaterialBufferData:",
    (pbrMaterialBufferData, offsetArrMap),
  );

  let pbrMaterialBuffer =
    device
    |> Device.createBuffer({
         "size": pbrMaterialBufferSize,
         "usage": BufferUsage.copy_dst lor BufferUsage.uniform,
       });
  pbrMaterialBuffer |> Buffer.setSubFloat32Data(0, pbrMaterialBufferData);

  let pbrMaterialBindGroup =
    device
    |> Device.createBindGroup({
         "layout": pbrMaterialBindGroupLayout,
         "entries": [|
           BindGroup.binding(
             ~binding=0,
             ~buffer=pbrMaterialBuffer,
             ~offset=0,
             ~size=singleRenderGameObjectPBRMaterialBufferSize,
             (),
           ),
         |],
       });

  let state =
    state
    |> Pass.setUniformBufferData(
         "pbrMaterialBuffer",
         (pbrMaterialBufferData, pbrMaterialBuffer),
       );

  let state =
    state
    |> Pass.GBufferPass.addDynamicBindGroupData(
         1,
         pbrMaterialBindGroup,
         offsetArrMap,
       );

  let (cameraBufferData, cameraBuffer) =
    BMFRBuffer.CameraBuffer.unsafeGetCameraBufferData(state);

  let cameraBindGroup =
    device
    |> Device.createBindGroup({
         "layout": cameraBindGroupLayout,
         "entries": [|
           BindGroup.binding(
             ~binding=0,
             ~buffer=cameraBuffer,
             ~offset=0,
             ~size=cameraBufferData |> Float32Array.byteLength,
             (),
           ),
         |],
       });

  Log.printComplete("cameraBufferData:", cameraBufferData);

  let state =
    state |> Pass.GBufferPass.addStaticBindGroupData(2, cameraBindGroup);

  let (vertexAndIndexBufferMap, indexCountMap) =
    _buildVertexData(device, allRenderGameObjects, state);

  let state =
    state
    |> Pass.GBufferPass.setVertexAndIndexBufferMap(vertexAndIndexBufferMap)
    |> Pass.GBufferPass.setIndexCountMap(indexCountMap);

  let baseShaderPath = "examples/bmfr/pass/gbuffer/shaders";

  let vertexShaderModule =
    device
    |> Device.createShaderModule({
         "code":
           WebGPUUtils.loadShaderFile({j|$(baseShaderPath)/gbuffer.vert|j}),
       });
  let fragmentShaderModule =
    device
    |> Device.createShaderModule({
         "code":
           WebGPUUtils.loadShaderFile({j|$(baseShaderPath)/gbuffer.frag|j}),
       });

  let positionRoughnessRenderTargetFormat = "rgba16float";
  let normalMetalnessRenderTargetFormat = "rgba16float";
  let diffusePositionWRenderTargetFormat = "rgba8unorm";
  // let specularRenderTargetFormat = "rgba8unorm";
  let motionVectorDepthSpecularRenderTargetFormat = "rgba16float";
  // let depthRenderTargetFormat = "r16float";

  let depthTextureFormat = "depth24plus";
  // let depthTextureFormat = "depth24plus-stencil8";

  let pipeline =
    device
    |> Device.createRenderPipeline(
         Pipeline.Render.descriptor(
           ~layout=
             device
             |> Device.createPipelineLayout({
                  "bindGroupLayouts": [|
                    modelBindGroupLayout,
                    pbrMaterialBindGroupLayout,
                    cameraBindGroupLayout,
                  |],
                }),
           ~vertexStage={
             Pipeline.Render.vertexStage(
               ~module_=vertexShaderModule,
               ~entryPoint="main",
             );
           },
           ~fragmentStage={
             Pipeline.Render.fragmentStage(
               ~module_=fragmentShaderModule,
               ~entryPoint="main",
             );
           },
           ~primitiveTopology="triangle-list",
           ~vertexState=
             Pipeline.Render.vertexState(
               ~indexFormat="uint32",
               ~vertexBuffers=[|
                 Pipeline.Render.vertexBuffer(
                   ~arrayStride=6 * Float32Array._BYTES_PER_ELEMENT,
                   ~attributes=[|
                     Pipeline.Render.vertexAttribute(
                       ~shaderLocation=0,
                       ~offset=0,
                       ~format="float3",
                     ),
                     Pipeline.Render.vertexAttribute(
                       ~shaderLocation=1,
                       ~offset=3 * Float32Array._BYTES_PER_ELEMENT,
                       ~format="float3",
                     ),
                   |],
                   (),
                 ),
               |],
               (),
             ),
           ~rasterizationState=Pipeline.Render.rasterizationState(),
           ~colorStates=[|
             Pipeline.Render.colorState(
               ~format=positionRoughnessRenderTargetFormat,
               ~alphaBlend=Pipeline.Render.blendDescriptor(),
               ~colorBlend=Pipeline.Render.blendDescriptor(),
             ),
             Pipeline.Render.colorState(
               ~format=normalMetalnessRenderTargetFormat,
               ~alphaBlend=Pipeline.Render.blendDescriptor(),
               ~colorBlend=Pipeline.Render.blendDescriptor(),
             ),
             Pipeline.Render.colorState(
               ~format=diffusePositionWRenderTargetFormat,
               ~alphaBlend=Pipeline.Render.blendDescriptor(),
               ~colorBlend=Pipeline.Render.blendDescriptor(),
             ),
             // Pipeline.Render.colorState(
             //   ~format=specularRenderTargetFormat,
             //   ~alphaBlend=Pipeline.Render.blendDescriptor(),
             //   ~colorBlend=Pipeline.Render.blendDescriptor(),
             // ),
             Pipeline.Render.colorState(
               ~format=motionVectorDepthSpecularRenderTargetFormat,
               ~alphaBlend=Pipeline.Render.blendDescriptor(),
               ~colorBlend=Pipeline.Render.blendDescriptor(),
             ),
             //  Pipeline.Render.colorState(
             //    ~format=depthRenderTargetFormat,
             //    ~alphaBlend=Pipeline.Render.blendDescriptor(),
             //    ~colorBlend=Pipeline.Render.blendDescriptor(),
             //  ),
           |],
           ~depthStencilState=
             Pipeline.Render.depthStencilState(
               ~depthWriteEnabled=true,
               ~depthCompare="less",
               ~format=depthTextureFormat,
               ~stencilFront=Pipeline.Render.stencilStateFaceDescriptor(),
               ~stencilBack=Pipeline.Render.stencilStateFaceDescriptor(),
             ),
           (),
         ),
       );

  let state = state |> Pass.GBufferPass.setPipeline(pipeline);

  let (positionRoughnessRenderTarget, positionRoughnessRenderTargetView) =
    _createRenderTargetData(
      device,
      window,
      positionRoughnessRenderTargetFormat,
    );
  let (normalMetalnessRenderTarget, normalMetalnessRenderTargetView) =
    _createRenderTargetData(
      device,
      window,
      normalMetalnessRenderTargetFormat,
    );
  let (diffusePositionWRenderTarget, diffusePositionWRenderTargetView) =
    _createRenderTargetData(
      device,
      window,
      diffusePositionWRenderTargetFormat,
    );
  let (_, motionVectorDepthSpecularRenderTargetView) =
    _createRenderTargetData(
      device,
      window,
      motionVectorDepthSpecularRenderTargetFormat,
    );
  // let (depthRenderTarget, depthRenderTargetView) =
  //   _createRenderTargetData(device, window, depthRenderTargetFormat);

  let state =
    state
    |> Pass.setTextureView(
         "positionRoughnessRenderTargetView",
         positionRoughnessRenderTargetView,
       )
    |> Pass.setTextureView(
         "normalMetalnessRenderTargetView",
         normalMetalnessRenderTargetView,
       )
    |> Pass.setTextureView(
         "diffusePositionWRenderTargetView",
         diffusePositionWRenderTargetView,
       )
    |> Pass.setTextureView(
         "motionVectorDepthSpecularRenderTargetView",
         motionVectorDepthSpecularRenderTargetView,
       );
  // |> Pass.setTextureView("depthRenderTargetView", depthRenderTargetView);

  let (_, depthTextureView) =
    _createTextureData(
      device,
      window,
      depthTextureFormat,
      TextureUsage.output_attachment,
    );

  let depthTexture =
    device
    |> Device.createTexture(
         Texture.descriptor(
           ~size={
             "width": Window.getWidth(window),
             "height": Window.getHeight(window),
             "depth": 1,
           },
           ~arrayLayerCount=1,
           ~mipLevelCount=1,
           ~sampleCount=1,
           ~dimension="2d",
           ~format=depthTextureFormat,
           ~usage=TextureUsage.output_attachment,
         ),
       );

  let state = state |> Pass.GBufferPass.setDepthTextureView(depthTextureView);
  Js.logMany([|"zxczxc" |> Obj.magic|]);

  state;
};

let _buildColorAttachment = (textureViewName, state) => {
  {
    "clearColor": {
      "r": 0.0,
      "g": 0.0,
      "b": 0.0,
      "a": 1.0,
    },
    "loadOp": "clear",
    "storeOp": "store",
    "attachment": Pass.unsafeGetTextureView(textureViewName, state),
  };
};

let execute = (device, queue, state) => {
  Js.logMany([|"nnn" |> Obj.magic|]);
  let commandEncoder =
    device |> Device.createCommandEncoder(CommandEncoder.descriptor());
  let renderPass =
    commandEncoder
    |> CommandEncoder.beginRenderPass(
         {
           PassEncoder.Render.descriptor(
             ~colorAttachments=[|
               _buildColorAttachment(
                 "positionRoughnessRenderTargetView",
                 state,
               ),
               _buildColorAttachment(
                 "normalMetalnessRenderTargetView",
                 state,
               ),
               _buildColorAttachment(
                 "diffusePositionWRenderTargetView",
                 state,
               ),
               // _buildColorAttachment("specularRenderTargetView", state),
               _buildColorAttachment(
                 "motionVectorDepthSpecularRenderTargetView",
                 state,
               ),
               //  _buildColorAttachment("depthRenderTargetView", state),
             |],
             ~depthStencilAttachment={
               "attachment":
                 Pass.GBufferPass.unsafeGetDepthTextureView(state),
               "clearDepth": 1.0,
               "depthLoadOp": "clear",
               "depthStoreOp": "store",
               "clearStencil": 0,
               "stencilLoadOp": "clear",
               "stencilStoreOp": "store",
             },
             (),
           );
         },
       );
  renderPass
  |> PassEncoder.Render.setPipeline(
       Pass.GBufferPass.unsafeGetPipeline(state),
     );
  Pass.GBufferPass.getStaticBindGroupDataArr(state)
  |> Js.Array.forEach(({setSlot, bindGroup}: staticBindGroupData) => {
       renderPass |> PassEncoder.Render.setBindGroup(setSlot, bindGroup)
     });

  Pass.GBufferPass.getRenderGameObjectArr(state)
  |> Js.Array.forEach(renderGameObject => {
       //  let transform = GameObject.unsafeGetTransform(renderGameObject, state);
       let geometry = GameObject.unsafeGetGeometry(renderGameObject, state);
       //  Log.printComplete(
       //    "draw geometry:",
       //    (
       //      renderGameObject,
       //      geometry,
       //      Pass.GBufferPass.unsafeGetIndexCount(geometry, state),
       //    ),
       //  );
       let (vertexBuffer, indexBuffer) =
         Pass.GBufferPass.unsafeGetVertexAndIndexBuffer(geometry, state);

       // TODO perf: judge last geometry
       renderPass |> PassEncoder.Render.setVertexBuffer(0, vertexBuffer);
       renderPass |> PassEncoder.Render.setIndexBuffer(indexBuffer);

       Pass.GBufferPass.getDynamicBindGroupDataArr(state)
       |> Js.Array.forEach(
            ({setSlot, bindGroup, offsetArrMap}: dynamicBindGroupData) => {
            renderPass
            |> PassEncoder.Render.setDynamicBindGroup(
                 setSlot,
                 bindGroup,
                 offsetArrMap
                 |> ImmutableSparseMap.unsafeGet(renderGameObject),
               )
          });

       renderPass
       |> PassEncoder.Render.drawIndexed(
            Pass.GBufferPass.unsafeGetIndexCount(geometry, state),
            1,
            0,
            0,
            0,
          );
     });

  renderPass |> PassEncoder.Render.endPass;

  queue |> Queue.submit([|commandEncoder |> CommandEncoder.finish|]);
  Js.logMany([|"nnn" |> Obj.magic|]);

  state;
};

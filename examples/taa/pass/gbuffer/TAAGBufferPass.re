open WebGPU;

open Js.Typed_array;

open StateType;

let _buildVertexAndIndexBufferMap = (device, allRenderGameObjects, state) => {
  allRenderGameObjects
  |> Js.Array.map(renderGameObject => {
       GameObject.unsafeGetGeometry(renderGameObject, state)
     })
  |> ArrayUtils2.removeDuplicateItems
  |> ArrayUtils.reduceOneParam(
       (. vertexAndIndexBufferMap, geometry) => {
         let vertices = Geometry.unsafeGetVertexData(geometry, state);
         let indices = Geometry.unsafeGetIndexData(geometry, state);

         let vertexBuffer =
           device
           |> Device.createBuffer({
                "size":
                  Js.Array.length(vertices) * Float32Array._BYTES_PER_ELEMENT,
                "usage": BufferUsage.copy_dst lor BufferUsage.vertex,
              });
         vertexBuffer
         |> Buffer.setSubFloat32Data(0, Float32Array.make(vertices));

         let indexBuffer =
           device
           |> Device.createBuffer({
                "size":
                  Js.Array.length(indices) * Uint32Array._BYTES_PER_ELEMENT,
                "usage": BufferUsage.copy_dst lor BufferUsage.index,
              });
         indexBuffer |> Buffer.setSubUint32Data(0, Uint32Array.make(indices));

         vertexAndIndexBufferMap
         |> ImmutableSparseMap.set(geometry, (vertexBuffer, indexBuffer));
       },
       ImmutableSparseMap.createEmpty(),
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
  let modelBindGroupLayout =
    device
    |> Device.createBindGroupLayout({
         "bindings": [|
           BindGroupLayout.layoutBinding(
             ~binding=0,
             ~visibility=ShaderStage.vertex,
             ~type_="uniform-buffer",
             ~hasDynamicOffset=true,
             (),
           ),
         |],
       });

  let phongMaterialBindGroupLayout =
    device
    |> Device.createBindGroupLayout({
         "bindings": [|
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
         "bindings": [|
           BindGroupLayout.layoutBinding(
             ~binding=0,
             ~visibility=ShaderStage.vertex,
             ~type_="uniform-buffer",
             (),
           ),
         |],
       });

  let allRenderGameObjects = TAAScene.getAllRenderGameObjects(state);

  let state =
    state |> Pass.GBufferPass.setRenderGameObjectArr(allRenderGameObjects);

  let singleRenderGameObjectModelBufferSize =
    (16 + 12) * Float32Array._BYTES_PER_ELEMENT;
  let alignedModelBufferBytes =
    ManageBuffer.UniformBuffer.getAlignedBufferBytes(
      singleRenderGameObjectModelBufferSize,
    );
  let alignedModelBufferFloats =
    ManageBuffer.UniformBuffer.getAlignedBufferFloats(
      alignedModelBufferBytes,
    );
  let modelBufferSize =
    (allRenderGameObjects |> Js.Array.length) * alignedModelBufferBytes;

  let (modelBufferData, _, offsetArrMap) =
    allRenderGameObjects
    |> ArrayUtils.reduceOneParam(
         (. (modelBufferData, offset, offsetArrMap), renderGameObject) => {
           let transform =
             GameObject.unsafeGetTransform(renderGameObject, state);

           let modelMatrix = Transform.buildModelMatrix(transform, state);
           let normalMatrix = Transform.buildNormalMatrix(modelMatrix);

           let (modelBufferData, newOffset) =
             (modelBufferData, offset)
             |> ManageBuffer.setMat3DataToBufferData(normalMatrix);

           let (modelBufferData, _) =
             modelBufferData
             |> TypeArray.Float32Array.setFloat32Array(newOffset, modelMatrix);

           (
             modelBufferData,
             offset + alignedModelBufferFloats,
             offsetArrMap
             |> ImmutableSparseMap.set(renderGameObject, [|offset|]),
           );
         },
         (
           Float32Array.fromLength(
             modelBufferSize / Float32Array._BYTES_PER_ELEMENT,
           ),
           0,
           ImmutableSparseMap.createEmpty(),
         ),
       );

  let modelBuffer =
    device
    |> Device.createBuffer({
         "size": modelBufferSize,
         "usage": BufferUsage.copy_dst lor BufferUsage.uniform,
       });
  modelBuffer |> Buffer.setSubFloat32Data(0, modelBufferData);

  let modelBindGroup =
    device
    |> Device.createBindGroup({
         "layout": modelBindGroupLayout,
         "bindings": [|
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

  let singleRenderGameObjectPhongMaterialBufferSize =
    (4 + 4 + 4) * Float32Array._BYTES_PER_ELEMENT;
  let alignedPhongMaterialBufferBytes =
    ManageBuffer.UniformBuffer.getAlignedBufferBytes(
      singleRenderGameObjectPhongMaterialBufferSize,
    );
  let alignedPhongMaterialBufferFloats =
    ManageBuffer.UniformBuffer.getAlignedBufferFloats(
      alignedPhongMaterialBufferBytes,
    );
  let phongMaterialBufferSize =
    (allRenderGameObjects |> Js.Array.length) * alignedPhongMaterialBufferBytes;

  let (phongMaterialBufferData, _, offsetArrMap) =
    allRenderGameObjects
    |> ArrayUtils.reduceOneParam(
         (.
           (phongMaterialBufferData, offset, offsetArrMap),
           renderGameObject,
         ) => {
           let material =
             GameObject.unsafeGetPhongMaterial(renderGameObject, state);

           let (phongMaterialBufferData, newOffset) =
             phongMaterialBufferData
             |> TypeArray.Float32Array.setFloatTuple3(
                  offset,
                  PhongMaterial.unsafeGetDiffuse(material, state),
                );

           let (phongMaterialBufferData, newOffset) =
             phongMaterialBufferData
             |> TypeArray.Float32Array.setFloatTuple3(
                  newOffset + 1,
                  PhongMaterial.unsafeGetSpecular(material, state),
                );

           let (phongMaterialBufferData, newOffset) =
             phongMaterialBufferData
             |> TypeArray.Float32Array.setFloat(
                  newOffset + 1,
                  PhongMaterial.unsafeGetShininess(material, state),
                );

           (
             phongMaterialBufferData,
             offset + alignedPhongMaterialBufferFloats,
             offsetArrMap
             |> ImmutableSparseMap.set(renderGameObject, [|offset|]),
           );
         },
         (
           Float32Array.fromLength(
             phongMaterialBufferSize / Float32Array._BYTES_PER_ELEMENT,
           ),
           0,
           ImmutableSparseMap.createEmpty(),
         ),
       );

  let phongMaterialBuffer =
    device
    |> Device.createBuffer({
         "size": phongMaterialBufferSize,
         "usage": BufferUsage.copy_dst lor BufferUsage.uniform,
       });
  phongMaterialBuffer |> Buffer.setSubFloat32Data(0, phongMaterialBufferData);

  let phongMaterialBindGroup =
    device
    |> Device.createBindGroup({
         "layout": phongMaterialBindGroupLayout,
         "bindings": [|
           BindGroup.binding(
             ~binding=0,
             ~buffer=phongMaterialBuffer,
             ~offset=0,
             ~size=singleRenderGameObjectPhongMaterialBufferSize,
             (),
           ),
         |],
       });

  let state =
    state
    |> Pass.setUniformBufferData(
         "phongMaterialBuffer",
         (phongMaterialBuffer, phongMaterialBufferData),
       );

  let state =
    state
    |> Pass.GBufferPass.addDynamicBindGroupData(
         1,
         phongMaterialBindGroup,
         offsetArrMap,
       );

  let (cameraBuffer, cameraBufferData) =
    TAABuffer.CameraBuffer.unsafeGetCameraBufferData(state);

  let cameraBindGroup =
    device
    |> Device.createBindGroup({
         "layout": cameraBindGroupLayout,
         "bindings": [|
           BindGroup.binding(
             ~binding=0,
             ~buffer=cameraBuffer,
             ~offset=0,
             ~size=cameraBufferData |> Float32Array.byteLength,
             (),
           ),
         |],
       });

  let state =
    state |> Pass.GBufferPass.addStaticBindGroupData(2, cameraBindGroup);

  let vertexAndIndexBufferMap =
    _buildVertexAndIndexBufferMap(device, allRenderGameObjects, state);

  let state =
    state
    |> Pass.GBufferPass.setVertexAndIndexBufferMap(vertexAndIndexBufferMap);

  let baseShaderPath = "examples/taa/pass/gbuffer/shaders";

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

  let positionRenderTargetFormat = "rgba16float";
  let normalRenderTargetFormat = "rgba16float";
  let diffuseRenderTargetFormat = "rgba8unorm";
  let specularRenderTargetFormat = "rgba8unorm";
  let shininessRenderTargetFormat = "r16float";
  let depthRenderTargetFormat = "r16float";

  let depthTextureFormat = "depth24plus";

  let pipeline =
    device
    |> Device.createRenderPipeline(
         Pipeline.Render.descriptor(
           ~layout=
             device
             |> Device.createPipelineLayout({
                  "bindGroupLayouts": [|
                    modelBindGroupLayout,
                    phongMaterialBindGroupLayout,
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
               ~format=positionRenderTargetFormat,
               ~alphaBlend=Pipeline.Render.blendDescriptor(),
               ~colorBlend=Pipeline.Render.blendDescriptor(),
             ),
             Pipeline.Render.colorState(
               ~format=normalRenderTargetFormat,
               ~alphaBlend=Pipeline.Render.blendDescriptor(),
               ~colorBlend=Pipeline.Render.blendDescriptor(),
             ),
             Pipeline.Render.colorState(
               ~format=diffuseRenderTargetFormat,
               ~alphaBlend=Pipeline.Render.blendDescriptor(),
               ~colorBlend=Pipeline.Render.blendDescriptor(),
             ),
             Pipeline.Render.colorState(
               ~format=specularRenderTargetFormat,
               ~alphaBlend=Pipeline.Render.blendDescriptor(),
               ~colorBlend=Pipeline.Render.blendDescriptor(),
             ),
             Pipeline.Render.colorState(
               ~format=shininessRenderTargetFormat,
               ~alphaBlend=Pipeline.Render.blendDescriptor(),
               ~colorBlend=Pipeline.Render.blendDescriptor(),
             ),
             Pipeline.Render.colorState(
               ~format=depthRenderTargetFormat,
               ~alphaBlend=Pipeline.Render.blendDescriptor(),
               ~colorBlend=Pipeline.Render.blendDescriptor(),
             ),
           |],
           ~depthStencilState=
             Pipeline.Render.depthStencilState(
               ~depthWriteEnabled=true,
               ~depthCompare="less",
               ~format=depthTextureFormat,
               (),
             ),
           (),
         ),
       );

  let state = state |> Pass.GBufferPass.setPipeline(pipeline);

  let (positionRenderTarget, positionRenderTargetView) =
    _createRenderTargetData(device, window, positionRenderTargetFormat);
  let (normalRenderTarget, normalRenderTargetView) =
    _createRenderTargetData(device, window, normalRenderTargetFormat);
  let (diffuseRenderTarget, diffuseRenderTargetView) =
    _createRenderTargetData(device, window, diffuseRenderTargetFormat);
  let (specularRenderTarget, specularRenderTargetView) =
    _createRenderTargetData(device, window, specularRenderTargetFormat);
  let (shininessRenderTarget, shininessRenderTargetView) =
    _createRenderTargetData(device, window, shininessRenderTargetFormat);
  let (depthRenderTarget, depthRenderTargetView) =
    _createRenderTargetData(device, window, depthRenderTargetFormat);

  let state =
    state
    |> Pass.setTextureView(
         "positionRenderTargetView",
         positionRenderTargetView,
       )
    |> Pass.setTextureView("normalRenderTargetView", normalRenderTargetView)
    |> Pass.setTextureView(
         "diffuseRenderTargetView",
         diffuseRenderTargetView,
       )
    |> Pass.setTextureView(
         "specularRenderTargetView",
         specularRenderTargetView,
       )
    |> Pass.setTextureView(
         "shininessRenderTargetView",
         shininessRenderTargetView,
       )
    |> Pass.setTextureView("depthRenderTargetView", depthRenderTargetView);

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
  let commandEncoder =
    device |> Device.createCommandEncoder(CommandEncoder.descriptor());
  let renderPass =
    commandEncoder
    |> CommandEncoder.beginRenderPass(
         {
           PassEncoder.Render.descriptor(
             ~colorAttachments=[|
               _buildColorAttachment("positionRenderTargetView", state),
               _buildColorAttachment("normalRenderTargetView", state),
               _buildColorAttachment("diffuseRenderTargetView", state),
               _buildColorAttachment("specularRenderTargetView", state),
               _buildColorAttachment("shininessRenderTargetView", state),
               _buildColorAttachment("depthRenderTargetView", state),
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
       let geometry = GameObject.unsafeGetGeometry(renderGameObject, state);
       let (vertexBuffer, indexBuffer) =
         Pass.GBufferPass.unsafeGetVertexAndIndexBuffer(geometry, state);

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
       |> PassEncoder.Render.draw(
            Pass.GBufferPass.unsafeGetVertexCount(renderGameObject, state),
            1,
            0,
            0,
          );
     });

  renderPass |> PassEncoder.Render.endPass;

  queue |> Queue.submit([|commandEncoder |> CommandEncoder.finish|]);
};

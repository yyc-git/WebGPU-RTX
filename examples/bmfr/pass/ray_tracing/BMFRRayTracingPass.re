open WebGPU;

open Js.Typed_array;

open StateType;

let _createShaderBindingTable = (baseShaderPath, device) => {
  let rayGenShaderModule =
    device
    |> Device.createShaderModule({
         "code":
           WebGPUUtils.loadShaderFile(
             {j|$(baseShaderPath)/ray-generation.rgen|j},
           ),
       });
  let rayRChitShaderModule =
    device
    |> Device.createShaderModule({
         "code":
           WebGPUUtils.loadShaderFile(
             {j|$(baseShaderPath)/ray-closest-hit-indirect-gi.rchit|j},
           ),
       });
  let rayMissShaderModule =
    device
    |> Device.createShaderModule({
         "code":
           WebGPUUtils.loadShaderFile({j|$(baseShaderPath)/ray-miss.rmiss|j}),
       });
  let rayMissShadowShaderModule =
    device
    |> Device.createShaderModule({
         "code":
           WebGPUUtils.loadShaderFile(
             {j|$(baseShaderPath)/ray-miss-shadow.rmiss|j},
           ),
       });

  device
  |> Device.createRayTracingShaderBindingTable({
       "stages": [|
         {"module": rayGenShaderModule, "stage": ShaderStage.ray_generation},
         {
           "module": rayRChitShaderModule,
           "stage": ShaderStage.ray_closest_hit,
         },
         {"module": rayMissShaderModule, "stage": ShaderStage.ray_miss},
         {"module": rayMissShadowShaderModule, "stage": ShaderStage.ray_miss},
       |],
       "groups": [|
         {
           "type": "general",
           "generalIndex": 0,
           "anyHitIndex": (-1),
           "closestHitIndex": (-1),
           "intersectionIndex": (-1),
         },
         {
           "type": "triangle-hit-group",
           "generalIndex": (-1),
           "anyHitIndex": (-1),
           "closestHitIndex": 1,
           "intersectionIndex": (-1),
         },
         {
           "type": "general",
           "generalIndex": 2,
           "anyHitIndex": (-1),
           "closestHitIndex": (-1),
           "intersectionIndex": (-1),
         },
         {
           "type": "general",
           "generalIndex": 3,
           "anyHitIndex": (-1),
           "closestHitIndex": (-1),
           "intersectionIndex": (-1),
         },
       |],
     });
};

let init = (device, queue, state) => {
  let gbufferBindGroupLayout =
    device
    |> Device.createBindGroupLayout({
         "bindings": [|
           BindGroupLayout.layoutBinding(
             ~binding=0,
             ~visibility=ShaderStage.ray_generation,
             ~type_="sampled-texture",
             (),
           ),
           BindGroupLayout.layoutBinding(
             ~binding=1,
             ~visibility=ShaderStage.ray_generation,
             ~type_="sampled-texture",
             (),
           ),
           BindGroupLayout.layoutBinding(
             ~binding=2,
             ~visibility=ShaderStage.ray_generation,
             ~type_="sampled-texture",
             (),
           ),
           BindGroupLayout.layoutBinding(
             ~binding=3,
             ~visibility=ShaderStage.ray_generation,
             ~type_="sampled-texture",
             (),
           ),
         |],
       });

  let rtBindGroupLayout =
    device
    |> Device.createBindGroupLayout({
         "bindings": [|
           BindGroupLayout.layoutBinding(
             ~binding=0,
             ~visibility=
               ShaderStage.ray_generation lor ShaderStage.ray_closest_hit,
             ~type_="acceleration-container",
             (),
           ),
           BindGroupLayout.layoutBinding(
             ~binding=1,
             ~visibility=ShaderStage.ray_generation,
             ~type_="storage-buffer",
             (),
           ),
           BindGroupLayout.layoutBinding(
             ~binding=2,
             ~visibility=
               ShaderStage.ray_generation lor ShaderStage.ray_closest_hit,
             ~type_="uniform-buffer",
             (),
           ),
           BindGroupLayout.layoutBinding(
             ~binding=3,
             ~visibility=ShaderStage.ray_closest_hit,
             ~type_="storage-buffer",
             (),
           ),
           BindGroupLayout.layoutBinding(
             ~binding=4,
             ~visibility=ShaderStage.ray_closest_hit,
             ~type_="storage-buffer",
             (),
           ),
           BindGroupLayout.layoutBinding(
             ~binding=5,
             ~visibility=ShaderStage.ray_closest_hit,
             ~type_="storage-buffer",
             (),
           ),
           BindGroupLayout.layoutBinding(
             ~binding=6,
             ~visibility=ShaderStage.ray_closest_hit,
             ~type_="storage-buffer",
             (),
           ),
           BindGroupLayout.layoutBinding(
             ~binding=7,
             ~visibility=ShaderStage.ray_closest_hit,
             ~type_="storage-buffer",
             (),
           ),
         |],
       });

  // let rtCHitBindGroupLayout =
  //   device
  //   |> Device.createBindGroupLayout({
  //        "bindings": [|
  //          BindGroupLayout.layoutBinding(
  //            ~binding=0,
  //            ~visibility=ShaderStage.ray_closest_hit,
  //            ~type_="storage-buffer",
  //            (),
  //          ),
  //         //  BindGroupLayout.layoutBinding(
  //         //    ~binding=1,
  //         //    ~visibility=ShaderStage.ray_closest_hit,
  //         //    ~type_="storage-buffer",
  //         //    (),
  //         //  ),
  //         //  BindGroupLayout.layoutBinding(
  //         //    ~binding=2,
  //         //    ~visibility=ShaderStage.ray_closest_hit,
  //         //    ~type_="storage-buffer",
  //         //    (),
  //         //  ),
  //         //  BindGroupLayout.layoutBinding(
  //         //    ~binding=3,
  //         //    ~visibility=ShaderStage.ray_closest_hit,
  //         //    ~type_="storage-buffer",
  //         //    (),
  //         //  ),
  //         //  BindGroupLayout.layoutBinding(
  //         //    ~binding=4,
  //         //    ~visibility=ShaderStage.ray_closest_hit,
  //         //    ~type_="storage-buffer",
  //         //    (),
  //         //  ),
  //        |],
  //      });

  let cameraBindGroupLayout =
    device
    |> Device.createBindGroupLayout({
         "bindings": [|
           BindGroupLayout.layoutBinding(
             ~binding=0,
             ~visibility=
               ShaderStage.ray_generation lor ShaderStage.ray_closest_hit,
             ~type_="uniform-buffer",
             (),
           ),
         |],
       });

  let directionLightBindGroupLayout =
    device
    |> Device.createBindGroupLayout({
         "bindings": [|
           BindGroupLayout.layoutBinding(
             ~binding=0,
             ~visibility=
               ShaderStage.ray_generation lor ShaderStage.ray_closest_hit,
             ~type_="storage-buffer",
             (),
           ),
         |],
       });

  let gbufferBindGroup =
    device
    |> Device.createBindGroup({
         "layout": gbufferBindGroupLayout,
         "bindings": [|
           BindGroup.binding(
             ~binding=0,
             ~textureView=
               Pass.unsafeGetTextureView("positionRoughnessRenderTargetView", state),
             ~size=0,
             (),
           ),
           BindGroup.binding(
             ~binding=1,
             ~textureView=
               Pass.unsafeGetTextureView("normalMetalnessRenderTargetView", state),
             ~size=0,
             (),
           ),
           BindGroup.binding(
             ~binding=2,
             ~textureView=
               Pass.unsafeGetTextureView("diffusePositionWRenderTargetView", state),
             ~size=0,
             (),
           ),
           BindGroup.binding(
             ~binding=3,
             ~textureView=
               Pass.unsafeGetTextureView(
                 "motionVectorDepthSpecularRenderTargetView",
                 state,
               ),
             ~size=0,
             (),
           ),
         |],
       });

  let (
    geometryContainers,
    (instanceBufferArrayBuffer, instanceBuffer),
    instanceContainer,
  ) =
    ManageAccelerationContainer.buildContainers(device, queue, state);

  let state =
    state
    |> OperateAccelerationContainer.setData(
         geometryContainers,
         instanceContainer,
         instanceBufferArrayBuffer,
         instanceBuffer,
       );

  let (pixelBufferSize, pixelBuffer) =
    BMFRBuffer.PixelBuffer.unsafeGetBufferData(state);

  let (commonDataBufferData, commonDataBuffer) =
    BMFRBuffer.CommonDataBuffer.unsafeGetBufferData(state);

  let (sceneDescBufferSize, sceneDescBuffer) =
    BMFRBuffer.GetHitShadingData.SceneDescBuffer.unsafeGetBufferData(state);
  let (geometryOffsetDataBufferSize, geometryOffsetDataBuffer) =
    BMFRBuffer.GetHitShadingData.GeometryOffsetDataBuffer.unsafeGetBufferData(
      state,
    );
  let (vertexBufferSize, vertexBuffer) =
    BMFRBuffer.GetHitShadingData.VertexBuffer.unsafeGetBufferData(state);
  let (indexBufferSize, indexBuffer) =
    BMFRBuffer.GetHitShadingData.IndexBuffer.unsafeGetBufferData(state);
  let (pbrMaterialBufferSize, pbrMaterialBuffer) =
    BMFRBuffer.GetHitShadingData.PBRMaterialBuffer.unsafeGetBufferData(
      state,
    );

  let rtBindGroup =
    device
    |> Device.createBindGroup({
         "layout": rtBindGroupLayout,
         "bindings": [|
           BindGroup.binding(
             ~binding=0,
             ~accelerationContainer=instanceContainer,
             ~offset=0,
             ~size=0,
             (),
           ),
           BindGroup.binding(
             ~binding=1,
             ~buffer=pixelBuffer,
             ~offset=0,
             ~size=pixelBufferSize,
             (),
           ),
           BindGroup.binding(
             ~binding=2,
             ~buffer=commonDataBuffer,
             ~offset=0,
             ~size=
               BMFRBuffer.CommonDataBuffer.getBufferSize(
                 commonDataBufferData,
               ),
             (),
           ),
           BindGroup.binding(
             ~binding=3,
             ~buffer=sceneDescBuffer,
             ~offset=0,
             ~size=sceneDescBufferSize,
             (),
           ),
           BindGroup.binding(
             ~binding=4,
             ~buffer=geometryOffsetDataBuffer,
             ~offset=0,
             ~size=geometryOffsetDataBufferSize,
             (),
           ),
           BindGroup.binding(
             ~binding=5,
             ~buffer=vertexBuffer,
             ~offset=0,
             ~size=vertexBufferSize,
             (),
           ),
           BindGroup.binding(
             ~binding=6,
             ~buffer=indexBuffer,
             ~offset=0,
             ~size=indexBufferSize,
             (),
           ),
           BindGroup.binding(
             ~binding=7,
             ~buffer=pbrMaterialBuffer,
             ~offset=0,
             ~size=pbrMaterialBufferSize,
             (),
           ),
         |],
       });

  // let rtCHitBindGroup =
  //   device
  //   |> Device.createBindGroup({
  //        "layout": rtCHitBindGroupLayout,
  //        "bindings": [|
  //          BindGroup.binding(
  //            ~binding=0,
  //            ~buffer=sceneDescBuffer,
  //            ~offset=0,
  //            ~size=sceneDescBufferSize,
  //            (),
  //          ),
  //         //  BindGroup.binding(
  //         //    ~binding=1,
  //         //    ~buffer=geometryOffsetDataBuffer,
  //         //    ~offset=0,
  //         //    ~size=geometryOffsetDataBufferSize,
  //         //    (),
  //         //  ),
  //         //  BindGroup.binding(
  //         //    ~binding=2,
  //         //    ~buffer=vertexBuffer,
  //         //    ~offset=0,
  //         //    ~size=vertexBufferSize,
  //         //    (),
  //         //  ),
  //         //  BindGroup.binding(
  //         //    ~binding=3,
  //         //    ~buffer=indexBuffer,
  //         //    ~offset=0,
  //         //    ~size=indexBufferSize,
  //         //    (),
  //         //  ),
  //         //  BindGroup.binding(
  //         //    ~binding=4,
  //         //    ~buffer=pbrMaterialBuffer,
  //         //    ~offset=0,
  //         //    ~size=pbrMaterialBufferSize,
  //         //    (),
  //         //  ),
  //        |],
  //      });

  let (cameraBufferData, cameraBuffer) =
    BMFRBuffer.CameraBuffer.unsafeGetCameraBufferData(state);

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

  let (directionLightBufferSize, directionLightBuffer) =
    BMFRBuffer.DirectionLightBuffer.buildData(device, state);

  let directionLightBindGroup =
    device
    |> Device.createBindGroup({
         "layout": directionLightBindGroupLayout,
         "bindings": [|
           BindGroup.binding(
             ~binding=0,
             ~buffer=directionLightBuffer,
             ~offset=0,
             ~size=directionLightBufferSize,
             (),
           ),
         |],
       });

  let state =
    state
    |> Pass.RayTracingPass.addStaticBindGroupData(0, gbufferBindGroup)
    |> Pass.RayTracingPass.addStaticBindGroupData(1, rtBindGroup)
    |> Pass.RayTracingPass.addStaticBindGroupData(2, cameraBindGroup)
    |> Pass.RayTracingPass.addStaticBindGroupData(3, directionLightBindGroup);
  // |> Pass.RayTracingPass.addStaticBindGroupData(4, rtCHitBindGroup);

  let pipeline =
    device
    |> Device.createRayTracingPipeline(
         Pipeline.RayTracing.descriptor(
           ~layout=
             device
             |> Device.createPipelineLayout({
                  "bindGroupLayouts": [|
                    gbufferBindGroupLayout,
                    rtBindGroupLayout,
                    cameraBindGroupLayout,
                    directionLightBindGroupLayout,
                    // rtCHitBindGroupLayout,
                  |],
                }),
           ~rayTracingState={
             Pipeline.RayTracing.rayTracingState(
               ~shaderBindingTable=
                 _createShaderBindingTable(
                   "examples/bmfr/pass/ray_tracing/shaders",
                   device,
                 ),
               ~maxRecursionDepth=1,
             );
           },
         ),
       );

  let state = state |> Pass.RayTracingPass.setPipeline(pipeline);

  state;
};

let execute = (device, window, queue, state) => {
  let commandEncoder =
    device |> Device.createCommandEncoder(CommandEncoder.descriptor());
  let rtPass =
    commandEncoder
    |> CommandEncoder.beginRayTracingPass(
         {
           PassEncoder.RayTracing.descriptor();
         },
       );
  rtPass
  |> PassEncoder.RayTracing.setPipeline(
       Pass.RayTracingPass.unsafeGetPipeline(state),
     );
  Pass.RayTracingPass.getStaticBindGroupDataArr(state)
  |> Js.Array.forEach(({setSlot, bindGroup}: staticBindGroupData) => {
       rtPass |> PassEncoder.RayTracing.setBindGroup(setSlot, bindGroup)
     });
  rtPass
  |> PassEncoder.RayTracing.traceRays(
       0, // sbt ray-generation offset
       1, // sbt ray-hit offset
       2, // sbt ray-miss offset
       Window.getWidth(window), // query width dimension
       Window.getHeight(window), // query height dimension
       1 // query depth dimension
     );
  rtPass |> PassEncoder.RayTracing.endPass;

  queue |> Queue.submit([|commandEncoder |> CommandEncoder.finish|]);

  state;
};

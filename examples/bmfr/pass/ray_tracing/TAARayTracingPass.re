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
             {j|$(baseShaderPath)/ray-closest-hit.rchit|j},
           ),
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
             ~visibility=ShaderStage.ray_generation,
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
             ~visibility=ShaderStage.ray_generation,
             ~type_="uniform-buffer",
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
             ~visibility=ShaderStage.ray_generation,
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
             ~visibility=ShaderStage.ray_generation,
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
               Pass.unsafeGetTextureView("positionRenderTargetView", state),
             ~size=0,
             (),
           ),
           BindGroup.binding(
             ~binding=1,
             ~textureView=
               Pass.unsafeGetTextureView("normalRenderTargetView", state),
             ~size=0,
             (),
           ),
           BindGroup.binding(
             ~binding=2,
             ~textureView=
               Pass.unsafeGetTextureView("diffuseRenderTargetView", state),
             ~size=0,
             (),
           ),
           BindGroup.binding(
             ~binding=3,
             ~textureView=
               Pass.unsafeGetTextureView(
                 "motionVectorDepthShininessRenderTargetView",
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
    Pass.unsafeGetStorageBufferData("pixelBuffer", state);

  let (commonDataBufferData, commonDataBuffer) =
    Pass.unsafeGetUniformBufferData("commonDataBuffer", state);

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
               TAABuffer.CommonDataBuffer.getCommonDataBufferSize(
                 commonDataBufferData,
               ),
             (),
           ),
         |],
       });

  let (cameraBufferData, cameraBuffer) =
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

  let (directionLightBufferSize, directionLightBuffer) =
    TAABuffer.DirectionLightBuffer.buildData(device, state);

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

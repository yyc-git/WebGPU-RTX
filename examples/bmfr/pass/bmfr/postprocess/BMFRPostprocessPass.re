open WebGPU;

open StateType;

let init = (device, swapChainFormat, state) => {
  let (resolutionBufferData, resolutionBuffer) =
    BMFRBuffer.ResolutionBuffer.unsafeGetBufferData(state);
  let (pixelBufferSize, pixelBuffer) =
    BMFRBuffer.PixelBuffer.unsafeGetBufferData(state);
  let (prevNoisyPixelBufferSize, prevNoisyPixelBuffer) =
    BMFRBuffer.PrevNoisyPixelBuffer.unsafeGetBufferData(state);
  let (prevPositionBufferSize, prevPositionBuffer) =
    BMFRBuffer.PrevPositionBuffer.unsafeGetBufferData(state);
  let (prevNormalBufferSize, prevNormalBuffer) =
    BMFRBuffer.PrevNormalBuffer.unsafeGetBufferData(state);
  let (commonDataBufferData, commonDataBuffer) =
    BMFRBuffer.CommonDataBuffer.unsafeGetBufferData(state);

  let gbufferBindGroupLayout =
    device
    |> Device.createBindGroupLayout({
         "bindings": [|
           BindGroupLayout.layoutBinding(
             ~binding=0,
             ~visibility=ShaderStage.fragment,
             ~type_="sampled-texture",
             (),
           ),
           BindGroupLayout.layoutBinding(
             ~binding=1,
             ~visibility=ShaderStage.fragment,
             ~type_="sampled-texture",
             (),
           ),
           BindGroupLayout.layoutBinding(
             ~binding=2,
             ~visibility=ShaderStage.fragment,
             ~type_="sampled-texture",
             (),
           ),
         |],
       });

  let otherBindGroupLayout =
    device
    |> Device.createBindGroupLayout({
         "bindings": [|
           BindGroupLayout.layoutBinding(
             ~binding=0,
             ~visibility=ShaderStage.fragment,
             ~type_="storage-buffer",
             (),
           ),
           BindGroupLayout.layoutBinding(
             ~binding=1,
             ~visibility=ShaderStage.fragment,
             ~type_="storage-buffer",
             (),
           ),
           BindGroupLayout.layoutBinding(
             ~binding=2,
             ~visibility=ShaderStage.fragment,
             ~type_="storage-buffer",
             (),
           ),
           BindGroupLayout.layoutBinding(
             ~binding=3,
             ~visibility=ShaderStage.fragment,
             ~type_="storage-buffer",
             (),
           ),
           BindGroupLayout.layoutBinding(
             ~binding=4,
             ~visibility=ShaderStage.fragment,
             ~type_="uniform-buffer",
             (),
           ),
           BindGroupLayout.layoutBinding(
             ~binding=5,
             ~visibility=ShaderStage.fragment,
             ~type_="uniform-buffer",
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
               Pass.unsafeGetTextureView(
                 "motionVectorDepthShininessRenderTargetView",
                 state,
               ),
             ~size=0,
             (),
           ),
         |],
       });

  let otherBindGroup =
    device
    |> Device.createBindGroup({
         "layout": otherBindGroupLayout,
         "bindings": [|
           BindGroup.binding(
             ~binding=0,
             ~buffer=pixelBuffer,
             ~offset=0,
             ~size=pixelBufferSize,
             (),
           ),
           BindGroup.binding(
             ~binding=1,
             ~buffer=prevNoisyPixelBuffer,
             ~offset=0,
             ~size=prevNoisyPixelBufferSize,
             (),
           ),
           BindGroup.binding(
             ~binding=2,
             ~buffer=prevPositionBuffer,
             ~offset=0,
             ~size=prevPositionBufferSize,
             (),
           ),
           BindGroup.binding(
             ~binding=3,
             ~buffer=prevNormalBuffer,
             ~offset=0,
             ~size=prevNormalBufferSize,
             (),
           ),
           BindGroup.binding(
             ~binding=4,
             ~buffer=resolutionBuffer,
             ~offset=0,
             ~size=
               BMFRBuffer.ResolutionBuffer.getBufferSize(
                 resolutionBufferData,
               ),
             (),
           ),
           BindGroup.binding(
             ~binding=5,
             ~buffer=commonDataBuffer,
             ~offset=0,
             ~size=
               BMFRBuffer.CommonDataBuffer.getBufferSize(
                 commonDataBufferData,
               ),
             (),
           ),
         |],
       });

  let state =
    state
    |> Pass.PostprocessPass.addStaticBindGroupData(0, gbufferBindGroup)
    |> Pass.PostprocessPass.addStaticBindGroupData(1, otherBindGroup);

  let baseShaderPath = "examples/bmfr/pass/bmfr/postprocess/shaders";

  let vertexShaderModule =
    device
    |> Device.createShaderModule({
         "code":
           WebGPUUtils.loadShaderFile(
             {j|$(baseShaderPath)/postprocess.vert|j},
           ),
       });
  let fragmentShaderModule =
    device
    |> Device.createShaderModule({
         "code":
           WebGPUUtils.loadShaderFile(
             {j|$(baseShaderPath)/postprocess.frag|j},
           ),
       });

  let pipeline =
    device
    |> Device.createRenderPipeline(
         Pipeline.Render.descriptor(
           ~layout=
             device
             |> Device.createPipelineLayout({
                  "bindGroupLayouts": [|
                    gbufferBindGroupLayout,
                    otherBindGroupLayout,
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
             Pipeline.Render.vertexState(~indexFormat="uint32", ()),
           ~rasterizationState=Pipeline.Render.rasterizationState(),
           ~colorStates=[|
             Pipeline.Render.colorState(
               ~format=swapChainFormat,
               ~alphaBlend=Pipeline.Render.blendDescriptor(),
               ~colorBlend=Pipeline.Render.blendDescriptor(),
             ),
           |],
           (),
         ),
       );

  state |> Pass.PostprocessPass.setPipeline(pipeline);
};

let execute = (device, queue, swapChain, state) => {
  let backBufferView = swapChain |> SwapChain.getCurrentTextureView();

  let commandEncoder =
    device |> Device.createCommandEncoder(CommandEncoder.descriptor());
  let renderPass =
    commandEncoder
    |> CommandEncoder.beginRenderPass(
         {
           PassEncoder.Render.descriptor(
             ~colorAttachments=[|
               {
                 "clearColor": {
                   "r": 0.0,
                   "g": 0.0,
                   "b": 0.0,
                   "a": 1.0,
                 },
                 "loadOp": "clear",
                 "storeOp": "store",
                 "attachment": backBufferView,
               },
             |],
             (),
           );
         },
       );

  let (staticBindGroupDataArr, pipeline) = (
    Pass.PostprocessPass.getStaticBindGroupDataArr(state),
    Pass.PostprocessPass.unsafeGetPipeline(state),
  );

  renderPass |> PassEncoder.Render.setPipeline(pipeline);

  staticBindGroupDataArr
  |> Js.Array.forEach(({setSlot, bindGroup}: staticBindGroupData) => {
       renderPass |> PassEncoder.Render.setBindGroup(setSlot, bindGroup)
     });

  renderPass |> PassEncoder.Render.draw(3, 1, 0, 0);
  renderPass |> PassEncoder.Render.endPass;

  queue |> Queue.submit([|commandEncoder |> CommandEncoder.finish|]);

  state;
};

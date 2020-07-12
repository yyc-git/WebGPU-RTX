open RaytracingFramework;

open WebGPU;

open StateType;

let init = (device, swapChainFormat, state) => {
  let (resolutionBufferData, resolutionBuffer) =
    RTXBuffer.ResolutionBuffer.unsafeGetBufferData(state);
  let (pixelBufferSize, pixelBuffer) =
    RTXBuffer.PixelBuffer.unsafeGetBufferData(state);
  let (accumulationPixelBufferSize, accumulationPixelBuffer) =
    RTXBuffer.AccumulationPixelBuffer.unsafeGetBufferData(state);
  let (accumulationCommonDataBufferData, accumulationCommonDataBuffer) =
    RTXBuffer.AccumulationCommonDataBuffer.unsafeGetBufferData(state);

  let bindGroupLayout =
    device
    |> Device.createBindGroupLayout({
         "entries": [|
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
             ~type_="uniform-buffer",
             (),
           ),
           BindGroupLayout.layoutBinding(
             ~binding=3,
             ~visibility=ShaderStage.fragment,
             ~type_="uniform-buffer",
             (),
           ),
         |],
       });

  let bindGroup =
    device
    |> Device.createBindGroup({
         "layout": bindGroupLayout,
         "entries": [|
           BindGroup.binding(
             ~binding=0,
             ~buffer=pixelBuffer,
             ~offset=0,
             ~size=pixelBufferSize,
             (),
           ),
           BindGroup.binding(
             ~binding=1,
             ~buffer=accumulationPixelBuffer,
             ~offset=0,
             ~size=accumulationPixelBufferSize,
             (),
           ),
           BindGroup.binding(
             ~binding=2,
             ~buffer=accumulationCommonDataBuffer,
             ~offset=0,
             ~size=
               RTXBuffer.AccumulationCommonDataBuffer.getBufferSize(
                 accumulationCommonDataBufferData,
               ),
             (),
           ),
           BindGroup.binding(
             ~binding=3,
             ~buffer=resolutionBuffer,
             ~offset=0,
             ~size=
               RTXBuffer.ResolutionBuffer.getBufferSize(
                 resolutionBufferData,
               ),
             (),
           ),
         |],
       });

  let state =
    state |> Pass.AccumulationPass.setStaticBindGroupData(0, bindGroup);

  let baseShaderPath = "src/pass/accumulation/shaders";

  let vertexShaderModule =
    device
    |> Device.createShaderModule({
         "code":
           WebGPUUtils.loadShaderFile(
             {j|$(baseShaderPath)/accumulation.vert|j},
           ),
       });
  let fragmentShaderModule =
    device
    |> Device.createShaderModule({
         "code":
           WebGPUUtils.loadShaderFile(
             {j|$(baseShaderPath)/accumulation.frag|j},
           ),
       });

  let pipeline =
    device
    |> Device.createRenderPipeline(
         Pipeline.Render.descriptor(
           ~layout=
             device
             |> Device.createPipelineLayout({
                  "bindGroupLayouts": [|bindGroupLayout|],
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

  state |> Pass.AccumulationPass.setPipeline(pipeline);
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

  let (staticBindGroupData, pipeline) = (
    Pass.AccumulationPass.unsafeGetStaticBindGroupData(state),
    Pass.AccumulationPass.unsafeGetPipeline(state),
  );

  renderPass |> PassEncoder.Render.setPipeline(pipeline);

  let {setSlot, bindGroup}: staticBindGroupData = staticBindGroupData;

  renderPass |> PassEncoder.Render.setBindGroup(setSlot, bindGroup);

  renderPass |> PassEncoder.Render.draw(3, 1, 0, 0);
  renderPass |> PassEncoder.Render.endPass;

  queue |> Queue.submit([|commandEncoder |> CommandEncoder.finish|]);

  state;
};

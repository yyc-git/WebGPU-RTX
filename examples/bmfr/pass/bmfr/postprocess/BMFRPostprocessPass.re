open WebGPU;

open StateType;

let init = (device, swapChainFormat, state) => {
  let (resolutionBufferData, resolutionBuffer) =
    BMFRBuffer.ResolutionBuffer.unsafeGetBufferData(state);
  let (pixelBufferSize, pixelBuffer) =
    BMFRBuffer.PixelBuffer.unsafeGetBufferData(state);
  let (accumulatedPrevFramePixelBufferSize, accumulatedPrevFramePixelBuffer) =
    BMFRBuffer.AccumulatedPrevFramePixelBuffer.unsafeGetBufferData(state);
  let (acceptBoolBufferSize, acceptBoolBuffer) =
    BMFRBuffer.AcceptBoolBuffer.unsafeGetBufferData(state);
  let (prevFramePixelIndicesBufferSize, prevFramePixelIndicesBuffer) =
    BMFRBuffer.PrevFramePixelIndicesBuffer.unsafeGetBufferData(state);
  let (commonDataBufferData, commonDataBuffer) =
    BMFRBuffer.CommonDataBuffer.unsafeGetBufferData(state);

  let bindGroupLayout =
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

  let bindGroup =
    device
    |> Device.createBindGroup({
         "layout": bindGroupLayout,
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
             ~buffer=accumulatedPrevFramePixelBuffer,
             ~offset=0,
             ~size=accumulatedPrevFramePixelBufferSize,
             (),
           ),
           BindGroup.binding(
             ~binding=2,
             ~buffer=acceptBoolBuffer,
             ~offset=0,
             ~size=acceptBoolBufferSize,
             (),
           ),
           BindGroup.binding(
             ~binding=3,
             ~buffer=prevFramePixelIndicesBuffer,
             ~offset=0,
             ~size=prevFramePixelIndicesBufferSize,
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
    state |> Pass.PostprocessPass.addStaticBindGroupData(0, bindGroup);

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

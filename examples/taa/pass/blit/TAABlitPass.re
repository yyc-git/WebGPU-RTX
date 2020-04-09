open WebGPU;

let init = (device, window, swapChainFormat, state) => {
  let (resolutionBufferSize, resolutionUniformBuffer) =
    ManageBuffer.UniformBuffer.buildResolutionBufferData(window, device);

  let (pixelBufferSize, pixelBuffer) =
    Pass.unsafeGetStorageBufferData("pixelBuffer", state);

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
             ~buffer=resolutionUniformBuffer,
             ~offset=0,
             ~size=resolutionBufferSize,
             (),
           ),
         |],
       });

  let state = state |> Pass.BlitPass.setBindGroup(bindGroup);

  let baseShaderPath = "examples/taa/pass/blit/shaders";

  let vertexShaderModule =
    device
    |> Device.createShaderModule({
         "code":
           WebGPUUtils.loadShaderFile({j|$(baseShaderPath)/screen.vert|j}),
       });
  let fragmentShaderModule =
    device
    |> Device.createShaderModule({
         "code":
           WebGPUUtils.loadShaderFile({j|$(baseShaderPath)/screen.frag|j}),
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

  let state = state |> Pass.BlitPass.setPipeline(pipeline);

  state;
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
  renderPass
  |> PassEncoder.Render.setPipeline(Pass.BlitPass.unsafeGetPipeline(state));
  renderPass
  |> PassEncoder.Render.setBindGroup(
       0,
       Pass.BlitPass.unsafeGetBindGroup(state),
     );
  renderPass |> PassEncoder.Render.draw(3, 1, 0, 0);
  renderPass |> PassEncoder.Render.endPass;

  queue |> Queue.submit([|commandEncoder |> CommandEncoder.finish|]);
};

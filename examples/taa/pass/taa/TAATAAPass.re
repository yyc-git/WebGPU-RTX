open WebGPU;

let _initFrameData =
    (
      device,
      swapChainFormat,
      (
        (resolutionBufferSize, resolutionBuffer),
        (pixelBufferSize, pixelBuffer),
        (historyPixelBufferSize, historyPixelBuffer),
        (taaBufferSize, taaBuffer),
      ),
      (fragmentShaderName, setBindGroupFunc, setPipelineFunc),
      state,
    ) => {
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
             ~buffer=historyPixelBuffer,
             ~offset=0,
             ~size=historyPixelBufferSize,
             (),
           ),
           BindGroup.binding(
             ~binding=2,
             ~buffer=resolutionBuffer,
             ~offset=0,
             ~size=resolutionBufferSize,
             (),
           ),
           BindGroup.binding(
             ~binding=3,
             ~buffer=taaBuffer,
             ~offset=0,
             ~size=taaBufferSize,
             (),
           ),
         |],
       });

  let state = state |> setBindGroupFunc(bindGroup);

  let baseShaderPath = "examples/taa/pass/taa/shaders";

  let vertexShaderModule =
    device
    |> Device.createShaderModule({
         "code": WebGPUUtils.loadShaderFile({j|$(baseShaderPath)/taa.vert|j}),
       });
  let fragmentShaderModule =
    device
    |> Device.createShaderModule({
         "code":
           WebGPUUtils.loadShaderFile(
             {j|$(baseShaderPath)/$(fragmentShaderName).frag|j},
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

  state |> setPipelineFunc(pipeline);
};

let _initFirstFrameData =
    (
      device,
      swapChainFormat,
      (
        (resolutionBufferSize, resolutionBuffer),
        (pixelBufferSize, pixelBuffer),
        (historyPixelBufferSize, historyPixelBuffer),
        (taaBufferSize, taaBuffer),
      ),
      state,
    ) => {
  _initFrameData(
    device,
    swapChainFormat,
    (
      (resolutionBufferSize, resolutionBuffer),
      (pixelBufferSize, pixelBuffer),
      (historyPixelBufferSize, historyPixelBuffer),
      (taaBufferSize, taaBuffer),
    ),
    (
      "taa_firstFrame",
      Pass.TAAPass.setFirstFrameBindGroup,
      Pass.TAAPass.setFirstFramePipeline,
    ),
    state,
  );
};

let _initOtherFrameData =
    (
      device,
      swapChainFormat,
      (
        (resolutionBufferSize, resolutionBuffer),
        (pixelBufferSize, pixelBuffer),
        (historyPixelBufferSize, historyPixelBuffer),
        (taaBufferSize, taaBuffer),
      ),
      state,
    ) => {
  _initFrameData(
    device,
    swapChainFormat,
    (
      (resolutionBufferSize, resolutionBuffer),
      (pixelBufferSize, pixelBuffer),
      (historyPixelBufferSize, historyPixelBuffer),
      (taaBufferSize, taaBuffer),
    ),
    (
      "taa_otherFrame",
      Pass.TAAPass.setOtherFrameBindGroup,
      Pass.TAAPass.setOtherFramePipeline,
    ),
    state,
  );
};

let init = (device, window, swapChainFormat, state) => {
  let (resolutionBufferSize, resolutionBuffer) =
    ManageBuffer.UniformBuffer.buildResolutionBufferData(window, device);

  let (pixelBufferSize, pixelBuffer) =
    Pass.unsafeGetStorageBufferData("pixelBuffer", state);
  let (historyPixelBufferSize, historyPixelBuffer) =
    Pass.unsafeGetStorageBufferData("historyPixelBuffer", state);
  let (taaBufferSize, taaBuffer) =
    Pass.unsafeGetStorageBufferData("taaBuffer", state);

  state
  |> Pass.TAAPass.markFirstFrame
  |> _initFirstFrameData(
       device,
       swapChainFormat,
       (
         (resolutionBufferSize, resolutionBuffer),
         (pixelBufferSize, pixelBuffer),
         (historyPixelBufferSize, historyPixelBuffer),
         (taaBufferSize, taaBuffer),
       ),
     )
  |> _initOtherFrameData(
       device,
       swapChainFormat,
       (
         (resolutionBufferSize, resolutionBuffer),
         (pixelBufferSize, pixelBuffer),
         (historyPixelBufferSize, historyPixelBuffer),
         (taaBufferSize, taaBuffer),
       ),
     );
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

  let (bindGroup, pipeline) =
    Pass.TAAPass.isFirstFrame(state)
      ? (
        Pass.TAAPass.unsafeGetFirstFrameBindGroup(state),
        Pass.TAAPass.unsafeGetFirstFramePipeline(state),
      )
      : (
        Pass.TAAPass.unsafeGetOtherFrameBindGroup(state),
        Pass.TAAPass.unsafeGetOtherFramePipeline(state),
      );

  renderPass |> PassEncoder.Render.setPipeline(pipeline);
  renderPass |> PassEncoder.Render.setBindGroup(0, bindGroup);
  renderPass |> PassEncoder.Render.draw(3, 1, 0, 0);
  renderPass |> PassEncoder.Render.endPass;

  queue |> Queue.submit([|commandEncoder |> CommandEncoder.finish|]);

  state |> Pass.TAAPass.markNotFirstFrame
};

open WebGPU;

open StateType;

let init = (device, state) => {
  let (resolutionBufferData, resolutionBuffer) =
    BMFRBuffer.ResolutionBuffer.unsafeGetBufferData(state);
  let (pixelBufferSize, pixelBuffer) =
    BMFRBuffer.PixelBuffer.unsafeGetBufferData(state);
  let (tmpDataBufferSize, tmpDataBuffer) =
    BMFRBuffer.Regression.TmpDataBuffer.unsafeGetBufferData(state);
  let (outDataBufferSize, outDataBuffer) =
    BMFRBuffer.Regression.OutDataBuffer.unsafeGetBufferData(state);
  let (commonDataBufferData, commonDataBuffer) =
    BMFRBuffer.Regression.CommonDataBuffer.unsafeGetBufferData(state);

  let gbufferBindGroupLayout =
    device
    |> Device.createBindGroupLayout({
         "entries": [|
           BindGroupLayout.layoutBinding(
             ~binding=0,
             ~visibility=ShaderStage.compute,
             ~type_="sampled-texture",
             (),
           ),
           BindGroupLayout.layoutBinding(
             ~binding=1,
             ~visibility=ShaderStage.compute,
             ~type_="sampled-texture",
             (),
           ),
           BindGroupLayout.layoutBinding(
             ~binding=2,
             ~visibility=ShaderStage.compute,
             ~type_="sampled-texture",
             (),
           ),
         |],
       });

  let otherBindGroupLayout =
    device
    |> Device.createBindGroupLayout({
         "entries": [|
           BindGroupLayout.layoutBinding(
             ~binding=0,
             ~visibility=ShaderStage.compute,
             ~type_="storage-buffer",
             (),
           ),
           BindGroupLayout.layoutBinding(
             ~binding=1,
             ~visibility=ShaderStage.compute,
             ~type_="storage-buffer",
             (),
           ),
           BindGroupLayout.layoutBinding(
             ~binding=2,
             ~visibility=ShaderStage.compute,
             ~type_="storage-buffer",
             (),
           ),
           BindGroupLayout.layoutBinding(
             ~binding=3,
             ~visibility=ShaderStage.compute,
             ~type_="uniform-buffer",
             (),
           ),
           BindGroupLayout.layoutBinding(
             ~binding=4,
             ~visibility=ShaderStage.compute,
             ~type_="uniform-buffer",
             (),
           ),
         |],
       });

  let gbufferBindGroup =
    device
    |> Device.createBindGroup({
         "layout": gbufferBindGroupLayout,
         "entries": [|
           BindGroup.binding(
             ~binding=0,
             ~textureView=
               Pass.unsafeGetTextureView(
                 "positionRoughnessRenderTargetView",
                 state,
               ),
             ~size=0,
             (),
           ),
           BindGroup.binding(
             ~binding=1,
             ~textureView=
               Pass.unsafeGetTextureView(
                 "normalMetalnessRenderTargetView",
                 state,
               ),
             ~size=0,
             (),
           ),
           BindGroup.binding(
             ~binding=2,
             ~textureView=
               Pass.unsafeGetTextureView(
                 "diffusePositionWRenderTargetView",
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
             ~buffer=tmpDataBuffer,
             ~offset=0,
             ~size=tmpDataBufferSize,
             (),
           ),
           BindGroup.binding(
             ~binding=2,
             ~buffer=outDataBuffer,
             ~offset=0,
             ~size=outDataBufferSize,
             (),
           ),
           BindGroup.binding(
             ~binding=3,
             ~buffer=resolutionBuffer,
             ~offset=0,
             ~size=
               BMFRBuffer.ResolutionBuffer.getBufferSize(
                 resolutionBufferData,
               ),
             (),
           ),
           BindGroup.binding(
             ~binding=4,
             ~buffer=commonDataBuffer,
             ~offset=0,
             ~size=
               BMFRBuffer.Regression.CommonDataBuffer.getBufferSize(
                 commonDataBufferData,
               ),
             (),
           ),
         |],
       });

  let state =
    state
    |> Pass.RegressionPass.addStaticBindGroupData(0, gbufferBindGroup)
    |> Pass.RegressionPass.addStaticBindGroupData(1, otherBindGroup);

  let baseShaderPath = "examples/bmfr/pass/bmfr/regression/shaders";

  let computeShaderModule =
    device
    |> Device.createShaderModule({
         "code":
           WebGPUUtils.loadShaderFile(
             {j|$(baseShaderPath)/regression.comp|j},
           ),
       });

  let pipeline =
    device
    |> Device.createComputePipeline(
         Pipeline.Compute.descriptor(
           ~layout=
             device
             |> Device.createPipelineLayout({
                  "bindGroupLayouts": [|
                    gbufferBindGroupLayout,
                    otherBindGroupLayout,
                  |],
                }),
           ~computeStage={
             Pipeline.Compute.computeStage(
               ~module_=computeShaderModule,
               ~entryPoint="main",
             );
           },
         ),
       );

  state |> Pass.RegressionPass.setPipeline(pipeline);
};

let execute = (device, queue, window, state) => {
  Pass.AccumulationPass.canDenoise(state)
    ? {
      Js.log("denoise");

      let commandEncoder =
        device |> Device.createCommandEncoder(CommandEncoder.descriptor());
      let renderPass =
        commandEncoder
        |> CommandEncoder.beginComputePass(
             {
               PassEncoder.Compute.descriptor();
             },
           );

      let (staticBindGroupDataArr, pipeline) = (
        Pass.RegressionPass.getStaticBindGroupDataArr(state),
        Pass.RegressionPass.unsafeGetPipeline(state),
      );

      renderPass |> PassEncoder.Compute.setPipeline(pipeline);

      staticBindGroupDataArr
      |> Js.Array.forEach(({setSlot, bindGroup}: staticBindGroupData) => {
           renderPass |> PassEncoder.Compute.setBindGroup(setSlot, bindGroup)
         });

      let w = BMFRBuffer.Regression.computeHorizentalBlocksCount(window);
      let h = BMFRBuffer.Regression.computeVerticalBlocksCount(window);
      renderPass |> PassEncoder.Compute.dispatchX(w * h);
      renderPass |> PassEncoder.Compute.endPass;

      queue |> Queue.submit([|commandEncoder |> CommandEncoder.finish|]);

      state;
    }
    : {
      state;
    };
};

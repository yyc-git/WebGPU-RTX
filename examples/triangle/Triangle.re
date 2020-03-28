open WebGPU;

open WonderBsMost.Most;

let window = Window.make({"width": 640, "height": 480, "title": "WebGPU"});

fromPromise(GPU.requestAdapter(GPU.adapterDescriptor(~window, ())))
|> flatMap(adapter => {
     fromPromise(adapter |> Adapter.requestDevice)
     |> flatMap(device => {
          let context = Window.getContext(window);
          let queue = device |> Device.getQueue;

          fromPromise(context |> Context.getSwapChainPreferredFormat(device))
          |> map(swapChainFormat => {
               (adapter, device, context, queue, swapChainFormat)
             });
        })
   })
|> subscribe({
     "next": ((adapter, device, context, queue, swapChainFormat)) => {
       let swapChain =
         context
         |> Context.configureSwapChain({
              "device": device,
              "format": swapChainFormat,
            });

       let baseShaderPath = "examples/triangle/shaders/";

       let vertexShaderModule =
         device
         |> Device.createShaderModule({
              "code":
                WebGPUUtils.loadShaderFile(
                  {j|$(baseShaderPath)/triangle.vert|j},
                ),
            });
       let fragmentShaderModule =
         device
         |> Device.createShaderModule({
              "code":
                WebGPUUtils.loadShaderFile(
                  {j|$(baseShaderPath)/triangle.frag|j},
                ),
            });

       let pipeline =
         device
         |> Device.createRenderPipeline(
              Pipeline.Render.descriptor(
                ~layout=
                  device
                  |> Device.createPipelineLayout({"bindGroupLayouts": [||]}),
                ~vertexStage={
                  Pipeline.Render.vertexStage(
                    ~module_=vertexShaderModule,
                    ~entryPoint="main",
                  );
                },
                ~primitiveTopology="triangle-list",
                ~fragmentStage={
                  Pipeline.Render.fragmentStage(
                    ~module_=fragmentShaderModule,
                    ~entryPoint="main",
                  );
                },
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

       let rec _onFrame = () => {
         !(window |> Window.shouldClose)
           ? {
             Global.setTimeout(_onFrame, 1e3 /. 60.0) |> ignore;
           }
           : ();

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
         renderPass |> PassEncoder.Render.setPipeline(pipeline);
         renderPass |> PassEncoder.Render.draw(3, 1, 0, 0);
         renderPass |> PassEncoder.Render.endPass;

         queue |> Queue.submit([|commandEncoder |> CommandEncoder.finish|]);

         swapChain |> SwapChain.present;
         window |> Window.pollEvents();

         ();
       };

       Global.setTimeout(_onFrame, 1e3 /. 60.0) |> ignore;
     },
     "error": e => {
       Js.log(e);

       ();
     },
     "complete": () => {
       ();
     },
   });

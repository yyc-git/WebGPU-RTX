open WebGPU;

open WonderBsMost.Most;

open Js.Typed_array;

let window = Window.make({"width": 640, "height": 480, "title": "WebGPU"});

fromPromise(
  GPU.requestAdapter(
    GPU.adapterDescriptor(~window, ~preferredBackend="vulkan", ()),
  ),
)
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

       let state = StateData.getState();

       let state = state |> Scene.build |> Scene.init(window);

       let baseShaderPath = "examples/ray_tracing/shaders/";

       let vertexShaderModule =
         device
         |> Device.createShaderModule({
              "code":
                WebGPUUtils.loadShaderFile(
                  {j|$(baseShaderPath)/screen.vert|j},
                ),
            });
       let fragmentShaderModule =
         device
         |> Device.createShaderModule({
              "code":
                WebGPUUtils.loadShaderFile(
                  {j|$(baseShaderPath)/screen.frag|j},
                ),
            });

       let ((instanceBufferArrayBuffer, instanceBuffer), instanceContainer) =
         ManageAccelerationContainer.buildContainers(device, queue, state);

       let shaderBindingTable =
         ManageShaderBindingTable.create(baseShaderPath, device);

       let rtBindGroupLayout =
         device
         |> Device.createBindGroupLayout({
              "bindings": [|
                {
                  "binding": 0,
                  "visibility": ShaderStage.ray_generation,
                  "type": "acceleration-container",
                },
                {
                  "binding": 1,
                  "visibility": ShaderStage.ray_generation,
                  "type": "storage-buffer",
                },
                {
                  "binding": 2,
                  "visibility": ShaderStage.ray_generation,
                  "type": "uniform-buffer",
                },
              |],
            });

       let (cameraData, cameraBufferSize, cameraBuffer) =
         ManageBuffer.UniformBuffer.buildCameraBufferData(device, state);

       let state =
         ManageBuffer.UniformBuffer.setCameraBufferData(
           (cameraData, cameraBuffer),
           state,
         );

       let (pixelBufferSize, pixelBuffer) =
         ManageBuffer.StorageBuffer.buildPixelBufferData(window, device);

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
                  ~buffer=cameraBuffer,
                  ~offset=0,
                  ~size=cameraBufferSize,
                  (),
                ),
              |],
            });

       let rtPipeline =
         device
         |> Device.createRayTracingPipeline(
              Pipeline.RayTracing.descriptor(
                ~layout=
                  device
                  |> Device.createPipelineLayout({
                       "bindGroupLayouts": [|rtBindGroupLayout|],
                     }),
                ~rayTracingState={
                  Pipeline.RayTracing.rayTracingState(
                    ~shaderBindingTable,
                    ~maxRecursionDepth=1,
                  );
                },
              ),
            );

       let (resolutionBufferSize, resolutionUniformBuffer) =
         ManageBuffer.UniformBuffer.buildResolutionBufferData(window, device);

       let renderBindGroupLayout =
         device
         |> Device.createBindGroupLayout({
              "bindings": [|
                {
                  "binding": 0,
                  "visibility": ShaderStage.fragment,
                  "type": "storage-buffer",
                },
                {
                  "binding": 1,
                  "visibility": ShaderStage.fragment,
                  "type": "uniform-buffer",
                },
              |],
            });

       let renderBindGroup =
         device
         |> Device.createBindGroup({
              "layout": renderBindGroupLayout,
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

       let renderPipeline =
         device
         |> Device.createRenderPipeline(
              Pipeline.Render.descriptor(
                ~layout=
                  device
                  |> Device.createPipelineLayout({
                       "bindGroupLayouts": [|renderBindGroupLayout|],
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

       state |> StateData.setState |> ignore;

       let startTime = Performance.now();

       let rec _onFrame = () => {
         !(window |> Window.shouldClose)
           ? {
             Global.setTimeout(_onFrame, 1e3 /. 60.0) |> ignore;
           }
           : ();

         let time = Performance.now() -. startTime;


         StateData.getState()
         |> Scene.update(time, window)
         |> StateData.setState
         |> ignore;

         let commandEncoder =
           device |> Device.createCommandEncoder(CommandEncoder.descriptor());

         let rtPass =
           commandEncoder
           |> CommandEncoder.beginRayTracingPass(
                {
                  PassEncoder.RayTracing.descriptor();
                },
              );
         rtPass |> PassEncoder.RayTracing.setPipeline(rtPipeline);
         rtPass |> PassEncoder.RayTracing.setBindGroup(0, rtBindGroup);
         rtPass
         |> PassEncoder.RayTracing.traceRays(
              0, // sbt ray-generation offset
              1, // sbt ray-hit offset
              3, // sbt ray-miss offset
              Window.getWidth(window), // query width dimension
              Window.getHeight(window), // query height dimension
              1 // query depth dimension
            );
         rtPass |> PassEncoder.RayTracing.endPass;

         let backBufferView = swapChain |> SwapChain.getCurrentTextureView();
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
         renderPass |> PassEncoder.Render.setPipeline(renderPipeline);
         renderPass |> PassEncoder.Render.setBindGroup(0, renderBindGroup);
         renderPass |> PassEncoder.Render.draw(3, 1, 0, 0);
         renderPass |> PassEncoder.Render.endPass;

         queue |> Queue.submit([|commandEncoder |> CommandEncoder.finish|]);

         swapChain |> SwapChain.present;
         window |> Window.pollEvents();
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

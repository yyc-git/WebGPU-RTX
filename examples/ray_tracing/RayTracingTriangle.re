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

       let baseShaderPath = "examples/ray_tracing/shaders/";

       let computeShaderModule =
         device
         |> Device.createShaderModule({
              "code":
                WebGPUUtils.loadShaderFile({j|$(baseShaderPath)/anim.comp|j}),
            });
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
       let rayGenShaderModule =
         device
         |> Device.createShaderModule({
              "code":
                WebGPUUtils.loadShaderFile(
                  {j|$(baseShaderPath)/ray-generation.rgen|j},
                ),
            });
       let rayCHitShaderModule =
         device
         |> Device.createShaderModule({
              "code":
                WebGPUUtils.loadShaderFile(
                  {j|$(baseShaderPath)/ray-closest-hit.rchit|j},
                ),
            });
       let rayMissShaderModule =
         device
         |> Device.createShaderModule({
              "code":
                WebGPUUtils.loadShaderFile(
                  {j|$(baseShaderPath)/ray-miss.rmiss|j},
                ),
            });

       let triangleVertices =
         Float32Array.make([|
           0.0,
           1.0,
           0.0,
           0.0,
           (-1.0),
           (-1.0),
           0.0,
           0.0,
           1.0,
           (-1.0),
           0.0,
           0.0,
         |]);
       let triangleVertexBuffer =
         device
         |> Device.createBuffer({
              "size": triangleVertices |> Float32Array.byteLength,
              "usage": BufferUsage.copy_dst lor BufferUsage.storage,
            });
       triangleVertexBuffer |> Buffer.setSubFloat32Data(0, triangleVertices);

       let triangleIndices = Uint32Array.make([|0, 1, 2|]);
       let triangleIndexBuffer =
         device
         |> Device.createBuffer({
              "size": triangleIndices |> Uint32Array.byteLength,
              "usage": BufferUsage.copy_dst,
            });
       triangleIndexBuffer |> Buffer.setSubUint32Data(0, triangleIndices);

       let geometryContainer =
         device
         |> Device.createRayTracingAccelerationContainer(
              {
                AccelerationContainer.descriptor(
                  // ~flags=AccelerationContainerFlag.prefer_fast_trace,
                  ~flags=
                    AccelerationContainerFlag.prefer_fast_trace
                    lor AccelerationContainerFlag.allow_update,
                  ~level="bottom",
                  ~geometries=[|
                    {
                      "flags": AccelerationGeometryFlag.opaque,
                      "type": "triangles",
                      "vertex": {
                        "buffer": triangleVertexBuffer,
                        "format": "float3",
                        "stride": 4 * Float32Array._BYTES_PER_ELEMENT,
                        "count": Float32Array.length(triangleVertices),
                      },
                      "index": {
                        "buffer": triangleIndexBuffer,
                        "format": "uint32",
                        "count": Uint32Array.length(triangleIndices),
                      },
                    },
                  |],
                  (),
                );
              },
            );

       let instanceContainer =
         device
         |> Device.createRayTracingAccelerationContainer(
              {
                AccelerationContainer.descriptor(
                  ~flags=AccelerationContainerFlag.prefer_fast_trace,
                  ~level="top",
                  ~instances=[|
                    AccelerationContainer.instance(
                      ~flags=AccelerationInstanceFlag.triangle_cull_disable,
                      ~mask=0xFF,
                      ~instanceId=0,
                      ~instanceOffset=0x0,
                      ~transform={
                        "translation": {
                          "x": 0.,
                          "y": 0.,
                          "z": 0.,
                        },
                        "rotation": {
                          "x": 0.,
                          "y": 0.,
                          "z": 0.,
                        },
                        "scale": {
                          "x": 1.,
                          "y": 1.,
                          "z": 1.,
                        },
                      },
                      ~geometryContainer,
                      (),
                    ),
                  |],
                  (),
                );
              },
            );

       let commandEncoder =
         device |> Device.createCommandEncoder(CommandEncoder.descriptor());
       commandEncoder
       |> CommandEncoder.buildRayTracingAccelerationContainer(
            geometryContainer,
          );
       commandEncoder
       |> CommandEncoder.buildRayTracingAccelerationContainer(
            instanceContainer,
          );
       queue |> Queue.submit([|commandEncoder |> CommandEncoder.finish|]);

       let shaderBindingTable =
         device
         |> Device.createRayTracingShaderBindingTable({
              "stages": [|
                {
                  "module": rayGenShaderModule,
                  "stage": ShaderStage.ray_generation,
                },
                {
                  "module": rayCHitShaderModule,
                  "stage": ShaderStage.ray_closest_hit,
                },
                {
                  "module": rayMissShaderModule,
                  "stage": ShaderStage.ray_miss,
                },
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
                  "type": "general",
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

       let aspect =
         (Window.getWidth(window) |> float_of_int)
         /. (Window.getHeight(window) |> float_of_int);

       let mProjection =
         Matrix4.createIdentityMatrix4()
         |> Matrix4.buildPerspective((72.0, aspect, 0.1, 4096.0))
         |> Matrix4.invert(_, Matrix4.createIdentityMatrix4());
       let mView =
         Matrix4.createIdentityMatrix4()
         |> Matrix4.setLookAt((0., 0., 10.), (0., 0., 0.), (0., 1., 0.))
         |> Matrix4.invert(_, Matrix4.createIdentityMatrix4());

       let cameraData =
         Float32Array.fromLength(
           (mView |> Float32Array.length)
           + (mProjection |> Float32Array.length),
         );
       let cameraBufferSize = cameraData |> Float32Array.byteLength;
       let cameraUniformBuffer =
         device
         |> Device.createBuffer({
              "size": cameraBufferSize,
              "usage": BufferUsage.copy_dst lor BufferUsage.uniform,
            });

       let cameraData =
         cameraData
         |> TypeArray.Float32Array.set(0, mView)
         |> TypeArray.Float32Array.set(
              mView |> Float32Array.length,
              mProjection,
            );
       cameraUniformBuffer |> Buffer.setSubFloat32Data(0, cameraData);

       // this storage buffer is used as a pixel buffer
       // the result of the ray tracing pass gets written into it
       // and it gets copied to the screen in the rasterization pass
       let pixelBufferSize =
         Window.getWidth(window)
         * Window.getHeight(window)
         * 4
         * Float32Array._BYTES_PER_ELEMENT;
       let pixelBuffer =
         device
         |> Device.createBuffer({
              "size": pixelBufferSize,
              "usage": BufferUsage.storage,
            });

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
                  ~buffer=cameraUniformBuffer,
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

       let computeConstantsBufferSize = 1 * Float32Array._BYTES_PER_ELEMENT;
       let computeConstantsData =
         Float32Array.fromLength(
           computeConstantsBufferSize / Float32Array._BYTES_PER_ELEMENT,
         );
       let computeConstantsBuffer =
         device
         |> Device.createBuffer({
              "size": computeConstantsBufferSize,
              "usage": BufferUsage.copy_dst lor BufferUsage.uniform,
            });

       let computeBindGroupLayout =
         device
         |> Device.createBindGroupLayout({
              "bindings": [|
                {
                  "binding": 0,
                  "visibility": ShaderStage.compute,
                  "type": "uniform-buffer",
                },
                {
                  "binding": 1,
                  "visibility": ShaderStage.compute,
                  "type": "storage-buffer",
                },
              |],
            });

       let computeBindGroup =
         device
         |> Device.createBindGroup({
              "layout": computeBindGroupLayout,
              "bindings": [|
                BindGroup.binding(
                  ~binding=0,
                  ~buffer=computeConstantsBuffer,
                  ~offset=0,
                  ~size=computeConstantsBufferSize,
                  (),
                ),
                BindGroup.binding(
                  ~binding=1,
                  ~buffer=triangleVertexBuffer,
                  ~offset=0,
                  ~size=triangleVertices |> Float32Array.byteLength,
                  (),
                ),
              |],
            });

       let computePipeline =
         device
         |> Device.createComputePipeline(
              Pipeline.Compute.descriptor(
                ~layout=
                  device
                  |> Device.createPipelineLayout({
                       "bindGroupLayouts": [|computeBindGroupLayout|],
                     }),
                ~computeStage={
                  Pipeline.Compute.computeStage(
                    ~module_=computeShaderModule,
                    ~entryPoint="main",
                  );
                },
              ),
            );

       let resolutionData =
         Float32Array.make([|
           Window.getWidth(window) |> float_of_int,
           Window.getHeight(window) |> float_of_int,
         |]);
       let resolutionBufferSize = resolutionData |> Float32Array.byteLength;
       let resolutionUniformBuffer =
         device
         |> Device.createBuffer({
              "size": resolutionBufferSize,
              "usage": BufferUsage.copy_dst lor BufferUsage.uniform,
            });
       resolutionUniformBuffer |> Buffer.setSubFloat32Data(0, resolutionData);

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

       let _updateUniformBuffers =
           (time, (computeConstantsData, computeConstantsBuffer)) => {
         Float32Array.unsafe_set(computeConstantsData, 0, time);

         computeConstantsBuffer
         |> Buffer.setSubFloat32Data(0, computeConstantsData);
       };

       let _update = (time, (computeConstantsData, computeConstantsBuffer)) => {
         _updateUniformBuffers(
           time,
           (computeConstantsData, computeConstantsBuffer),
         );

         let commandEncoder =
           device |> Device.createCommandEncoder(CommandEncoder.descriptor());
         commandEncoder
         |> CommandEncoder.updateRayTracingAccelerationContainer(
              geometryContainer,
            );
         queue |> Queue.submit([|commandEncoder |> CommandEncoder.finish|]);
       };

       let startTime = Performance.now();

       let rec _onFrame = () => {
         !(window |> Window.shouldClose)
           ? {
             Global.setTimeout(_onFrame, 1e3 /. 60.0) |> ignore;
           }
           : ();

         let time = Performance.now() -. startTime;

         let commandEncoder =
           device |> Device.createCommandEncoder(CommandEncoder.descriptor());

         let computePass =
           commandEncoder
           |> CommandEncoder.beginComputePass(
                {
                  PassEncoder.Compute.descriptor();
                },
              );
         computePass |> PassEncoder.Compute.setPipeline(computePipeline);
         computePass |> PassEncoder.Compute.setBindGroup(0, computeBindGroup);
         computePass |> PassEncoder.Compute.dispatchX(3);
         computePass |> PassEncoder.Compute.endPass;

         //  queue |> Queue.submit([|commandEncoder |> CommandEncoder.finish|]);

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
              2, // sbt ray-miss offset
              Window.getWidth(window), // query width dimension
              Window.getHeight(window), // query height dimension
              1 // query depth dimension
            );
         rtPass |> PassEncoder.RayTracing.endPass;

         //  queue |> Queue.submit([|commandEncoder |> CommandEncoder.finish|]);

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

         _update(time, (computeConstantsData, computeConstantsBuffer));

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

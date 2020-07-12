open RaytracingFramework;

open WebGPU;

open WonderBsMost.Most;

let window =
  Window.make({
    "width": 640,
    "height": 480,
    "title": "WebGPU",
    "resizable": false,
  });

RaytracingFramework.Director.load(window)
|> subscribe({
     "next": ((adapter, device, context, queue, swapChainFormat)) => {
       let swapChain =
         context
         |> Context.configureSwapChain({
              "device": device,
              "format": swapChainFormat,
            });

       StateData.getState()
       |> RTXScene.buildScene
       |> RaytracingFramework.Director.addInitFunc(RTXScene.init(device, window))
       |> RaytracingFramework.Director.addUpdateFunc(RTXScene.update(device, queue, window))
       |> RaytracingFramework.Director.addPassFuncs(
            GBufferPass.init(device, window),
            GBufferPass.execute(device, queue),
          )
       |> RaytracingFramework.Director.addPassFuncs(
            RayTracingPass.init(device, queue),
            RayTracingPass.execute(device, window, queue),
          )
       //  |> RaytracingFramework.Director.addPassFuncs(
       //       BMFRPreprocessPass.init(device, swapChainFormat),
       //       BMFRPreprocessPass.execute(device, queue, swapChain),
       //     )
       |> RaytracingFramework.Director.addPassFuncs(
            BMFRRegressionPass.init(device),
            BMFRRegressionPass.execute(device, queue, window),
          )
       |> RaytracingFramework.Director.addPassFuncs(
            BMFRPostprocessPass.init(device, swapChainFormat),
            BMFRPostprocessPass.execute(device, queue, swapChain),
          )
       |> RaytracingFramework.Director.addPassFuncs(
            TAAPass.init(device, window, swapChainFormat),
            TAAPass.execute(device, queue, swapChain),
          )
       |> RaytracingFramework.Director.addPassFuncs(
            AccumulationPass.init(device, swapChainFormat),
            AccumulationPass.execute(device, queue, swapChain),
          )
       |> RaytracingFramework.Director.start(window, swapChain);
     },
     "error": e => {
       Log.log(e);

       ();
     },
     "complete": () => {
       ();
     },
   });

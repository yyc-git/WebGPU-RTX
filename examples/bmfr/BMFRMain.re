open WebGPU;

open WonderBsMost.Most;

let window = Window.make({"width": 640, "height": 480, "title": "WebGPU"});

Director.load(window)
|> subscribe({
     "next": ((adapter, device, context, queue, swapChainFormat)) => {
       let swapChain =
         context
         |> Context.configureSwapChain({
              "device": device,
              "format": swapChainFormat,
            });

       StateData.getState()
       |> BMFRScene.buildScene
       |> Director.addInitFunc(BMFRScene.init(device, window))
       |> Director.addUpdateFunc(BMFRScene.update(device, queue, window))
       |> Director.addPassFuncs(
            BMFRGBufferPass.init(device, window),
            BMFRGBufferPass.execute(device, queue),
          )
       |> Director.addPassFuncs(
            BMFRRayTracingPass.init(device, queue),
            BMFRRayTracingPass.execute(device, window, queue),
          )
       |> Director.addPassFuncs(
            BMFRTAAPass.init(device, window, swapChainFormat),
            BMFRTAAPass.execute(device, queue, swapChain),
          )
       |> Director.start(window, swapChain);
     },
     "error": e => {
       Js.log(e);

       ();
     },
     "complete": () => {
       ();
     },
   });

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
       |> TAAScene.buildScene
       |> Director.addInitFunc(TAAScene.init(device, window))
       |> Director.addUpdateFunc(TAAScene.update(window))
       |> Director.addPassFuncs(
            TAAGBufferPass.init(device, window),
            TAAGBufferPass.execute(device, queue),
          )
       |> Director.addPassFuncs(
            TAARayTracingPass.init(device, queue),
            TAARayTracingPass.execute(device, window, queue),
          )
       |> Director.addPassFuncs(
            TAATAAPass.init(device, window, swapChainFormat),
            TAATAAPass.execute(device, queue, swapChain),
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
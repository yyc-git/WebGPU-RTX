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
       |> Director.addInitFunc(TAAScene.init(device))
       |> Director.addUpdateFunc(TAAScene.update(window))
       |> Director.addPassFuncs(
            TAAGBufferPass.init(device, window),
            TAAGBufferPass.execute(device, queue),
          )
       |> Director.addPass(
            TAABlitPass.init(device, swapChainFormat),
            TAABlitPass.execute(device, queue, swapChain),
          )
       |> Director.start(window);
     },
     "error": e => {
       Js.log(e);

       ();
     },
     "complete": () => {
       ();
     },
   });

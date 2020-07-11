open WebGPU;

open WonderBsMost.Most;

open StateType;

let getTime = state => state.director.time;

let setTime = (time, state) => {
  ...state,
  director: {
    ...state.director,
    time,
  },
};

let getFrameIndex = state => state.director.frameIndex;

let setFrameIndex = (frameIndex, state) => {
  ...state,
  director: {
    ...state.director,
    frameIndex,
  },
};

let load = window => {
  fromPromise(
    GPU.requestAdapter(
      GPU.adapterDescriptor(~window, ~preferredBackend="Vulkan", ()),
    ),
  )
  |> flatMap(adapter => {
       fromPromise(
         adapter |> Adapter.requestDevice({"extensions": [|"ray_tracing"|]}),
       )
       |> flatMap(device => {
            let context = Window.getContext(window);
            let queue = device |> Device.getQueue;

            fromPromise(
              context |> Context.getSwapChainPreferredFormat(device),
            )
            |> map(swapChainFormat => {
                 (adapter, device, context, queue, swapChainFormat)
               });
          })
     });
};

let addInitFunc = (initFunc, state) => {
  ...state,
  director: {
    ...state.director,
    initFuncArr: state.director.initFuncArr |> ArrayUtils.push(initFunc),
  },
};

let _getInitFuncArr = state => {
  state.director.initFuncArr;
};

let addUpdateFunc = (updateFunc, state) => {
  ...state,
  director: {
    ...state.director,
    updateFuncArr:
      state.director.updateFuncArr |> ArrayUtils.push(updateFunc),
  },
};

let _getUpdateFuncArr = state => {
  state.director.updateFuncArr;
};

let addPassFuncs = (initFunc, executeFunc, state) => {
  ...state,
  director: {
    ...state.director,
    passFuncDataArr:
      state.director.passFuncDataArr
      |> ArrayUtils.push({init: initFunc, execute: executeFunc}),
  },
};

let _getPassInitFuncArr = state => {
  state.director.passFuncDataArr |> Js.Array.map(({init}) => init);
};

let _getPassExecuteFuncArr = state => {
  state.director.passFuncDataArr |> Js.Array.map(({execute}) => execute);
};

let _init = (window, state) => {
  state |> Scene.init(window);
};

let start = (window, swapChain, state) => {
  let state = state |> _init(window);

  let state =
    _getInitFuncArr(state)
    |> ArrayUtils.reduceOneParam(
         (. state, initFunc) => {initFunc(state)},
         state,
       );

  let state =
    _getPassInitFuncArr(state)
    |> ArrayUtils.reduceOneParam(
         (. state, initFunc) => {initFunc(state)},
         state,
       );

  state |> StateData.setState |> ignore;

  let startTime = Performance.now();
  let lastTime = ref(startTime);
  let frameIndex = ref(0);

  let rec _onFrame = () => {
    !(window |> Window.shouldClose)
      ? {
        Global.setTimeout(_onFrame, 1e3 /. 60.0) |> ignore;
      }
      : ();
    let time = Performance.now() -. lastTime^;
    lastTime := Performance.now();

    let state = StateData.getState();

    let state = state |> setTime(time) |> setFrameIndex(frameIndex^);

    frameIndex := frameIndex^ |> succ;

    let state =
      _getUpdateFuncArr(state)
      |> ArrayUtils.reduceOneParam(
           (. state, updateFunc) => {updateFunc(state)},
           state,
         );

    let state =
      _getPassExecuteFuncArr(state)
      |> ArrayUtils.reduceOneParam(
           (. state, executeFunc) => {executeFunc(state)},
           state,
         );

    state |> StateData.setState |> ignore;

    swapChain |> SwapChain.present;
    window |> Window.pollEvents();
  };
  Global.setTimeout(_onFrame, 1e3 /. 60.0) |> ignore;
};

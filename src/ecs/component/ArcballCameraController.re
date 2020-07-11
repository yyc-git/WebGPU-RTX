open StateType;

open WonderBsMost.Most;

let create = state => {
  (
    state.arcballCameraController.index,
    {
      ...state,
      arcballCameraController: {
        ...state.arcballCameraController,
        index: state.arcballCameraController.index |> succ,
      },
    },
  );
};

let unsafeGetCurrentArcballCameraController = state => {
  state.arcballCameraController.currentArcballCameraController
  |> Js.Option.getExn;
};

let setCurrentArcballCameraController = (arcballCameraController, state) => {
  ...state,
  arcballCameraController: {
    ...state.arcballCameraController,
    currentArcballCameraController: Some(arcballCameraController),
  },
};

let getIsDrag = state => {
  state.arcballCameraController.isDrag;
};

let setIsDrag = (isDrag, state) => {
  ...state,
  arcballCameraController: {
    ...state.arcballCameraController,
    isDrag,
  },
};

let unsafeGetPhi = (arcballCameraController, state) => {
  state.arcballCameraController.phiMap
  |> ImmutableSparseMap.unsafeGet(arcballCameraController);
};

let setPhi = (arcballCameraController, phi, state) => {
  ...state,
  arcballCameraController: {
    ...state.arcballCameraController,
    phiMap:
      state.arcballCameraController.phiMap
      |> ImmutableSparseMap.set(arcballCameraController, phi),
  },
};

let unsafeGetTheta = (arcballCameraController, state) => {
  state.arcballCameraController.thetaMap
  |> ImmutableSparseMap.unsafeGet(arcballCameraController);
};

let setTheta = (arcballCameraController, theta, state) => {
  ...state,
  arcballCameraController: {
    ...state.arcballCameraController,
    thetaMap:
      state.arcballCameraController.thetaMap
      |> ImmutableSparseMap.set(arcballCameraController, theta),
  },
};

let unsafeGetTarget = (arcballCameraController, state) => {
  state.arcballCameraController.targetMap
  |> ImmutableSparseMap.unsafeGet(arcballCameraController);
};

let setTarget = (arcballCameraController, target, state) => {
  ...state,
  arcballCameraController: {
    ...state.arcballCameraController,
    targetMap:
      state.arcballCameraController.targetMap
      |> ImmutableSparseMap.set(arcballCameraController, target),
  },
};

let unsafeGetRotateSpeed = (arcballCameraController, state) => {
  state.arcballCameraController.rotateSpeedMap
  |> ImmutableSparseMap.unsafeGet(arcballCameraController);
};

let setRotateSpeed = (arcballCameraController, speed, state) => {
  ...state,
  arcballCameraController: {
    ...state.arcballCameraController,
    rotateSpeedMap:
      state.arcballCameraController.rotateSpeedMap
      |> ImmutableSparseMap.set(arcballCameraController, speed),
  },
};

let unsafeGetWheelSpeed = (arcballCameraController, state) => {
  state.arcballCameraController.wheelSpeedMap
  |> ImmutableSparseMap.unsafeGet(arcballCameraController);
};

let setWheelSpeed = (arcballCameraController, speed, state) => {
  ...state,
  arcballCameraController: {
    ...state.arcballCameraController,
    wheelSpeedMap:
      state.arcballCameraController.wheelSpeedMap
      |> ImmutableSparseMap.set(arcballCameraController, speed),
  },
};

let unsafeGetDistance = (arcballCameraController, state) => {
  state.arcballCameraController.distanceMap
  |> ImmutableSparseMap.unsafeGet(arcballCameraController);
};

let setDistance = (arcballCameraController, distance, state) => {
  ...state,
  arcballCameraController: {
    ...state.arcballCameraController,
    distanceMap:
      state.arcballCameraController.distanceMap
      |> ImmutableSparseMap.set(arcballCameraController, distance),
  },
};

let _getMovementDelta = e => {
  (- e##movementX, - e##movementY);
};

let _changeOrbit = (e, state) => {
  let currentArcballCameraController =
    state |> unsafeGetCurrentArcballCameraController;
  let rotateSpeed =
    unsafeGetRotateSpeed(currentArcballCameraController, state);

  let (x, y) = _getMovementDelta(e);

  state
  |> setPhi(
       currentArcballCameraController,
       unsafeGetPhi(currentArcballCameraController, state)
       +. float_of_int(x)
       /. (100. /. rotateSpeed),
     )
  |> setTheta(
       currentArcballCameraController,
       unsafeGetTheta(currentArcballCameraController, state)
       -. float_of_int(y)
       /. (100. /. rotateSpeed),
     );
};

let _bindDragStartEvent = window => {
  MostUtils.fromEvent("mousedown", window)
  |> tap(e => {
       StateData.getState() |> setIsDrag(true) |> StateData.setState |> ignore
     });
};

let _bindDragOverEvent = window => {
  MostUtils.fromEvent("mousemove", window)
  |> tap(e => {
       let state = StateData.getState();
       let state =
         getIsDrag(state)
           ? {
             state |> _changeOrbit(e);
           }
           : {
             state;
           };

       state |> StateData.setState |> ignore;
     });
};

let _bindDragDropEvent = window => {
  MostUtils.fromEvent("mouseup", window)
  |> tap(e => {
       StateData.getState()
       |> setIsDrag(false)
       |> StateData.setState
       |> ignore
     });
};

let _getWheel = e => {
  e##deltaY;
};

let _changeDistance = (e, state) => {
  let currentArcballCameraController =
    state |> unsafeGetCurrentArcballCameraController;

  state
  |> setDistance(
       currentArcballCameraController,
       unsafeGetDistance(currentArcballCameraController, state)
       -. _getWheel(e)
       *. unsafeGetWheelSpeed(currentArcballCameraController, state),
     );
};

let _bindMouseWheelEvent = window => {
  MostUtils.fromEvent("mousewheel", window)
  |> tap(e => {
       StateData.getState()
       |> _changeDistance(e)
       |> StateData.setState
       |> ignore
     });
};

let _handleDomEventStreamError = e => {
  // let message = Obj.magic(e)##message;
  // let stack = Obj.magic(e)##stack;
  // WonderLog.Log.debug(
  //   WonderLog.Log.buildDebugMessage(
  //     ~description={j|from dom event stream error|j},
  //     ~params={j|message:$message\nstack:$stack|j},
  //   ),
  //   IsDebugMainService.getIsDebug(StateDataMain.stateData),
  // );
  // /* WonderLog.Log.fatal(
  //      WonderLog.Log.buildFatalMessage(
  //        ~title="InitEventJob",
  //        ~description={j|from dom event stream error|j},
  //        ~reason="",
  //        ~solution={j||j},
  //        ~params={j|message:$message\nstack:$stack|j},
  //      ),
  //    ); */
  Log.error(
    e |> Obj.magic,
  );
};

let init = window => {
  mergeArray([|
    _bindDragStartEvent(window),
    _bindDragOverEvent(window),
    _bindDragDropEvent(window),
    _bindMouseWheelEvent(window),
  |])
  |> subscribe({
       "next": _ => (),
       "error": e => _handleDomEventStreamError(e),
       "complete": () => (),
     })
  |> ignore;
};

let getLookFrom = (arcballCameraController, state) => {
  let distance = unsafeGetDistance(arcballCameraController, state);
  let phi = unsafeGetPhi(arcballCameraController, state);
  let theta = unsafeGetTheta(arcballCameraController, state);
  let (x, y, z) = unsafeGetTarget(arcballCameraController, state);

  (
    distance *. Js.Math.cos(phi) *. Js.Math.sin(theta) +. x,
    distance *. Js.Math.cos(theta) +. y,
    distance *. Js.Math.sin(phi) *. Js.Math.sin(theta) +. z,
  );
};

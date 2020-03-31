open WonderBsMost.Most;

let _addEventListener: WebGPU.Window.t => Dom.eventTarget = [%raw
  window => {|
  window.addEventListener =  (eventName, listener, _) =>{
    window["on" + eventName] = (event) =>{
      listener(event);
    };
  };

  window.removeEventListener =  (eventName, listener, _) =>{
  };

  return window;
|}
];

let fromEvent = (eventName, window) => {
  fromEvent(eventName, window |> _addEventListener, false)
  |> map(e => Obj.magic(e));
};

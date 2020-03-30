open WonderBsMost.Most;

let fromEvent = (eventName, target) => {
  fromEvent(eventName, target, false) |> map(e => Obj.magic(e));
};

open Js.Typed_array;

module Float32Array = {
  let set = (offset, target, source) => {
    source |> Float32Array.setArrayOffset(Obj.magic(target), offset);

    source;
  };
};

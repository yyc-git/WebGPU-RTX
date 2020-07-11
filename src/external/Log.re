let fatal = message => {
  Js.Exn.raiseError(message);
};

let buildFatalMessage = (~title, ~description, ~reason, ~solution, ~params) => {j|
  Fatal:
  title
  $title
  description
  $description
  reason
  $reason
  solution
  $solution
  params
  $params
   |j};

[@bs.val] [@bs.module "util"]
external inspectFloat32Array:
  (
    Js.Typed_array.Float32Array.t,
    {. "maxArrayLength": Js.Nullable.t(int)}
  ) =>
  Js.Typed_array.Float32Array.t =
  "inspect";

let print = value => {
  Js.log(
    inspectFloat32Array(
      value |> Obj.magic,
      {"maxArrayLength": Js.Nullable.null},
    ),
  );

  value;
};

let printComplete = (message, value) => {
  Js.log2(
    message,
    inspectFloat32Array(
      value |> Obj.magic,
      {"maxArrayLength": Js.Nullable.null},
    ),
  );

  value;
};

let log = value => {
  Js.log(value);
};

let error = error => {
  // TODO use console.error
  Js.log(error);
};

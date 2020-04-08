let reduceOneParam = (func, param, arr) => {
  let mutableParam = ref(param);
  for (i in 0 to Js.Array.length(arr) - 1) {
    mutableParam := func(. mutableParam^, Array.unsafe_get(arr, i));
  };
  mutableParam^;
};

let reduceOneParami = (func, param, arr) => {
  let mutableParam = ref(param);
  for (i in 0 to Js.Array.length(arr) - 1) {
    mutableParam := func(. mutableParam^, Array.unsafe_get(arr, i), i);
  };
  mutableParam^;
};

let zipWith = (func, arr1, arr2) => {
  arr1
  |> Js.Array.mapi((value1, i) => {func(value1, Array.unsafe_get(arr2, i))});
};

let range = (a: int, b: int) => {
  let result = [||];

  /*!
   * note: different from Wonder.js -> range!!!
   */
  for (i in a to b - 1) {
    Js.Array.push(i, result) |> ignore;
  };
  result;
};

let push = (item, arr) => {
  arr |> Js.Array.push(item) |> ignore;
  arr;
};
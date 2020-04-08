let removeDuplicateItems = arr => {
  let resultArr = [||];
  let map = MutableHashMap.createEmpty();
  for (i in 0 to Js.Array.length(arr) - 1) {
    let item = Array.unsafe_get(arr, i);
    let key = Js.Int.toString(item);
    switch (MutableHashMap.get(key, map)) {
    | None =>
      Js.Array.push(item, resultArr) |> ignore;
      MutableHashMap.set(key, item, map) |> ignore;
    | Some(_) => ()
    };
  };
  resultArr;
};
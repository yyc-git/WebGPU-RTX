let createEmpty = (): SparseMapType.t2('a) => [||];

let copy = Js.Array.copy;

let _unsafeGet = (index: int, map: SparseMapType.t2('a)): 'a => {
  Array.unsafe_get(map, index);
};

let _isEmpty = (value: 'a): bool =>
  value |> SparseMapType.notNullableToNullable |> Js.Nullable.isNullable;

let get = (index: int, map) => {
  let value = _unsafeGet(index, map);
  _isEmpty(value) ? None : Some(value);
};

let has = (index: int, map) => !_isEmpty(_unsafeGet(index, map));

let getValidValues = map =>
  map |> Js.Array.filter(value => NullUtils.isInMap(Obj.magic(value)));

let getValidKeys = map =>
  map
  |> ArrayUtils.reduceOneParami(
       (. arr, value, key) =>
         if (NullUtils.isNotInMap(Obj.magic(value))) {
           arr;
         } else {
           arr |> Js.Array.push(key) |> ignore;
           arr;
         },
       [||],
     );

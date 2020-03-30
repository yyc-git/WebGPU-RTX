type t('index, 'value) = SparseMapType.t('index, 'value);

let createEmpty = SparseMap.createEmpty;

let copy = SparseMap.copy;

let get = SparseMap.get;

let unsafeGet = (key, map) => map |> get(key) |> Js.Option.getExn;

let has = SparseMap.has;

let set =
    (key: int, value: 'a, map: SparseMapType.t2('a)): SparseMapType.t2('a) => {
  let newMap = map |> copy;

  Array.unsafe_set(newMap, key, value);

  newMap;
};

let getValidValues = SparseMap.getValidValues;

let getValidKeys = SparseMap.getValidKeys;

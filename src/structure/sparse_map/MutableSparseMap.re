type t('index, 'value) = SparseMapType.t('index, 'value);

let createEmpty = SparseMap.createEmpty;

let copy = SparseMap.copy;

let get = SparseMap.get;

let has = SparseMap.has;

let set =
    (key: int, value: 'a, map: SparseMapType.t2('a))
    : SparseMapType.t2('a) => {
  Array.unsafe_set(map, key, value);

  map;
};
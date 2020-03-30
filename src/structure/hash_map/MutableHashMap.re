type t('key, 'value) = HashMapType.t('key, 'value);

let createEmpty = HashMap.createEmpty;

let set = (key: string, value: 'a, map: HashMapType.t2('a)) => {
  Js.Dict.set(map, key, value);

  map;
};

let get = HashMap.get;
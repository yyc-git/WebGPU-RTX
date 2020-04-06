type t('key, 'value) = HashMapType.t('key, 'value);

let createEmpty = HashMap.createEmpty;

let set =
    (key: string, value: 'a, map: HashMapType.t2('a)): HashMapType.t2('a) => {
  let newMap = map |> HashMap.copy;

  Js.Dict.set(newMap, key, value);

  newMap;
};

let get = HashMap.get;

let unsafeGet = (key, map) => map |> get(key) |> Js.Option.getExn;

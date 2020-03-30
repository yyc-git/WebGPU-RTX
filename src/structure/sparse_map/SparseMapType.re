type t('index, 'value) = array('value);
type t2('value) = t(int, 'value);

external notNullableToNullable: 'a => Js.Nullable.t('a) = "%identity";
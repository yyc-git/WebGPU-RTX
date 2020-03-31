let buildBigInt = [%raw value => {|
  return BigInt(value);
  |}];

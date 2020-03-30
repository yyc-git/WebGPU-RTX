let convertDecimalToHex = [%raw (int, param) => {|
  return parseInt(int, param)
  |}];

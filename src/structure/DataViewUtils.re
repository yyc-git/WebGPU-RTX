let setBigInt64LittleEndian = [%bs.raw
  (dataView, offset, value) => {|
  dataView.setBigInt64(offset, value, true);
  |}
];

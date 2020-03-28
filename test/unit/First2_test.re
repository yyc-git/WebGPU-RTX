open Wonder_jest;

let _ =
  describe("First", () =>
    Expect.(
      Expect.Operators.(
        describe("getFirst", () => {
          test("should return 1", () =>
            First.getFirst() |> expect == 1
          );
          test("should return 1", () =>
            First.getFirst() |> expect == 1
          );
          test("should return 1", () =>
            First.getFirst() |> expect == 1
          );
        })
      )
    )
  );

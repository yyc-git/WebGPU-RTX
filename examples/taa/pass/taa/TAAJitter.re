open Js.Typed_array;

let generateHaltonJiters = [%bs.raw
  (frameCount, windowWidth, windowHeight) => {|
/**
 * Generate a number in the Halton Sequence at a given index. This is
 * shamelessly stolen from the pseudocode on the Wikipedia page
 *
 * @param base the base to use for the Halton Sequence
 * @param index the index into the sequence
 */
var _haltonNumber = function(base, index) {
    let result = 0;
    let f = 1;
    while (index > 0) {
        f /= base;
        result += f * (index % base);
        index = Math.floor(index / base);
    }

    return result;
}

/**
 * Generate jitter amounts based on the Halton Sequence. Jitters are
 * normailized to be between -1 and 1
 *
 * @param length the number of offsets to generate
 */
var _generateHaltonJiters = function(length) {
    let jitters = [];

    for (let i = 1; i <= length; i++)
        jitters.push([(_haltonNumber(2, i) - 0.5) * 2, (this._haltonNumber(3, i) - 0.5) * 2]);

    return jitters;
}

return _generateHaltonJiters(frameCount).map(([ jitterX, jitterY ]) =>{
    return [
        jitterX / windowWidth,
        jitterY / windowHeight
    ]
})
  |}
];

let jitterProjectionMatrix = (projectionMatrix, state) => {
  let (jitterX, jitterY) =
    Pass.getJitter(Pass.getAccumulatedFrameIndex(state), state);

  let result = projectionMatrix |> Matrix4.copy;

  Float32Array.unsafe_set(result, 8, jitterX);
  Float32Array.unsafe_set(result, 9, jitterY);

  result;
};

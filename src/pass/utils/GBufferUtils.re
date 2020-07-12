open RaytracingFramework;

open WebGPU;

let createLinearSampler = device =>
  device
  |> Device.createSampler(
       Sampler.descriptor(
         ~magFilter="linear",
         ~minFilter="linear",
         ~addressModeU="repeat",
         ~addressModeV="repeat",
         ~addressModeW="repeat",
       ),
     );

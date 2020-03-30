open WebGPU;

let create = (baseShaderPath, device) =>{
       let rayGenShaderModule =
         device
         |> Device.createShaderModule({
              "code":
                WebGPUUtils.loadShaderFile(
                  {j|$(baseShaderPath)/ray-generation.rgen|j},
                ),
            });
       let rayCHitShaderModule =
         device
         |> Device.createShaderModule({
              "code":
                WebGPUUtils.loadShaderFile(
                  {j|$(baseShaderPath)/ray-closest-hit.rchit|j},
                ),
            });
       let rayCHitShaderModule2 =
         device
         |> Device.createShaderModule({
              "code":
                WebGPUUtils.loadShaderFile(
                  {j|$(baseShaderPath)/ray-closest-hit2.rchit|j},
                ),
            });
       let rayMissShaderModule =
         device
         |> Device.createShaderModule({
              "code":
                WebGPUUtils.loadShaderFile(
                  {j|$(baseShaderPath)/ray-miss.rmiss|j},
                ),
            });


         device
         |> Device.createRayTracingShaderBindingTable({
              "stages": [|
                {
                  "module": rayGenShaderModule,
                  "stage": ShaderStage.ray_generation,
                },
                {
                  "module": rayCHitShaderModule,
                  "stage": ShaderStage.ray_closest_hit,
                },
                {
                  "module": rayCHitShaderModule2,
                  "stage": ShaderStage.ray_closest_hit,
                },
                {
                  "module": rayMissShaderModule,
                  "stage": ShaderStage.ray_miss,
                },
              |],
              "groups": [|
                {
                  "type": "general",
                  "generalIndex": 0,
                  "anyHitIndex": (-1),
                  "closestHitIndex": (-1),
                  "intersectionIndex": (-1),
                },
                {
                  "type": "general",
                  "generalIndex": (-1),
                  "anyHitIndex": (-1),
                  "closestHitIndex": 1,
                  "intersectionIndex": (-1),
                },
                {
                  "type": "general",
                  "generalIndex": (-1),
                  "anyHitIndex": (-1),
                  "closestHitIndex": 2,
                  "intersectionIndex": (-1),
                },
                {
                  "type": "general",
                  "generalIndex": 3,
                  "anyHitIndex": (-1),
                  "closestHitIndex": (-1),
                  "intersectionIndex": (-1),
                },
              |],
            });
}

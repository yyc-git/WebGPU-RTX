open WebGPU;

open Js.Typed_array;

type transform = {
  index: int,
  translationMap:
    ImmutableSparseMap.t(TransformType.transform, (float, float, float)),
  rotationMap:
    ImmutableSparseMap.t(TransformType.transform, (float, float, float)),
  scaleMap:
    ImmutableSparseMap.t(TransformType.transform, (float, float, float)),
};

type geometry = {
  index: int,
  vertexDataMap:
    ImmutableSparseMap.t(
      GeometryType.geometry,
      (array(float), array(float)),
    ),
  indexDataMap: ImmutableSparseMap.t(GeometryType.geometry, array(int)),
};

type pbrMaterial = {
  index: int,
  // ambientMap: ImmutableSparseMap.t(PBRMaterialType.pbrMaterial, ColorType.color3),
  diffuseMap:
    ImmutableSparseMap.t(PBRMaterialType.pbrMaterial, ColorType.color3),
  metalnessMap: ImmutableSparseMap.t(PBRMaterialType.pbrMaterial, float),
  roughnessMap: ImmutableSparseMap.t(PBRMaterialType.pbrMaterial, float),
  specularMap: ImmutableSparseMap.t(PBRMaterialType.pbrMaterial, float),
  // illumMap: ImmutableSparseMap.t(PBRMaterialType.pbrMaterial, int),
  // dissolveMap: ImmutableSparseMap.t(PBRMaterialType.pbrMaterial, float),
};

type directionLight = {
  index: int,
  intensityMap:
    ImmutableSparseMap.t(DirectionLightType.directionLight, float),
  positionMap:
    ImmutableSparseMap.t(
      DirectionLightType.directionLight,
      (float, float, float),
    ),
};

type shader = {
  index: int,
  hitGroupIndexMap: ImmutableSparseMap.t(ShaderType.shader, int),
};

type transformAnimation = {
  index: int,
  dynamicTransformMap:
    ImmutableSparseMap.t(
      TransformAnimationType.transformAnimation,
      TransformType.transform,
    ),
  isAnimate: bool,
};

type arcballCameraController = {
  index: int,
  isDrag: bool,
  currentArcballCameraController:
    option(ArcballCameraControllerType.arcballCameraController),
  phiMap:
    ImmutableSparseMap.t(
      ArcballCameraControllerType.arcballCameraController,
      float,
    ),
  thetaMap:
    ImmutableSparseMap.t(
      ArcballCameraControllerType.arcballCameraController,
      float,
    ),
  targetMap:
    ImmutableSparseMap.t(
      ArcballCameraControllerType.arcballCameraController,
      (float, float, float),
    ),
  rotateSpeedMap:
    ImmutableSparseMap.t(
      ArcballCameraControllerType.arcballCameraController,
      float,
    ),
  wheelSpeedMap:
    ImmutableSparseMap.t(
      ArcballCameraControllerType.arcballCameraController,
      float,
    ),
  distanceMap:
    ImmutableSparseMap.t(
      ArcballCameraControllerType.arcballCameraController,
      float,
    ),
};

type cameraView = {
  index: int,
  currentCameraView: option(CameraViewType.cameraView),
  // viewMatrixInverseMap:
  //   ImmutableSparseMap.t(
  //     CameraViewType.cameraView,
  //     Js.Typed_array.Float32Array.t,
  //   ),
  // lastViewMatrixInverseMap:
  //   ImmutableSparseMap.t(
  //     CameraViewType.cameraView,
  //     option(Js.Typed_array.Float32Array.t),
  //   ),
  // projectionMatrixInverseMap:
  //   ImmutableSparseMap.t(
  //     CameraViewType.cameraView,
  //     Js.Typed_array.Float32Array.t,
  //   ),
  cameraPositionMap:
    ImmutableSparseMap.t(CameraViewType.cameraView, (float, float, float)),
  viewMatrixMap:
    ImmutableSparseMap.t(
      CameraViewType.cameraView,
      Js.Typed_array.Float32Array.t,
    ),
  // lastViewMatrixMap:
  //   ImmutableSparseMap.t(
  //     CameraViewType.cameraView,
  //     option(Js.Typed_array.Float32Array.t),
  //   ),
  projectionMatrixMap:
    ImmutableSparseMap.t(
      CameraViewType.cameraView,
      Js.Typed_array.Float32Array.t,
    ),
};

type gameObject = {
  index: int,
  transformMap:
    ImmutableSparseMap.t(GameObjectType.gameObject, TransformType.transform),
  geometryMap:
    ImmutableSparseMap.t(GameObjectType.gameObject, GeometryType.geometry),
  pbrMaterialMap:
    ImmutableSparseMap.t(
      GameObjectType.gameObject,
      PBRMaterialType.pbrMaterial,
    ),
  shaderMap:
    ImmutableSparseMap.t(GameObjectType.gameObject, ShaderType.shader),
  directionLightMap:
    ImmutableSparseMap.t(
      GameObjectType.gameObject,
      DirectionLightType.directionLight,
    ),
  transformAnimationMap:
    ImmutableSparseMap.t(
      GameObjectType.gameObject,
      TransformAnimationType.transformAnimation,
    ),
  cameraViewMap:
    ImmutableSparseMap.t(
      GameObjectType.gameObject,
      CameraViewType.cameraView,
    ),
  arcballCameraControllerMap:
    ImmutableSparseMap.t(
      GameObjectType.gameObject,
      ArcballCameraControllerType.arcballCameraController,
    ),
};

// type cameraBufferData = {
//   cameraData: option(Js.Typed_array.Float32Array.t),
//   cameraBuffer: option(Buffer.t),
// };

// type uniformBuffer = {cameraBufferData};

// type gpuBuffer = {uniformBuffer};

type setSlot = int;

type bufferName = string;

type textureViewName = string;

type staticBindGroupData = {
  setSlot,
  bindGroup: BindGroup.t,
};

type dynamicBindGroupData = {
  setSlot,
  bindGroup: BindGroup.t,
  // offset: int,
  // alignedUniformBytes: int,
  offsetArrMap: ImmutableSparseMap.t(GameObjectType.gameObject, array(int)),
};

// type vertexBufferData = {
//   buffer: Buffer.t,
//   offset: int,
// };

// type renderGameObjectData = {
//   // dynamicBindGroupData,
//   vertexBufferOffset: int,
//   vertexCount: int,
// };

type gbufferPassData = {
  lastModelMatrixMap:
    ImmutableSparseMap.t(TransformType.transform, Float32Array.t),
  lastViewProjectionMatrix: option(Float32Array.t),
  // jitteredProjectionMatrix: option(Float32Array.t),
  pipeline: option(Pipeline.Render.t),
  depthTextureView: option(TextureView.t),
  staticBindGroupDataArr: array(staticBindGroupData),
  dynamicBindGroupDataArr: array(dynamicBindGroupData),
  renderGameObjectArr: array(GameObjectType.gameObject),
  vertexAndIndexBufferMap:
    ImmutableSparseMap.t(GeometryType.geometry, (Buffer.t, Buffer.t)),
  indexCountMap: ImmutableSparseMap.t(GeometryType.geometry, int),
};

type rayTracingPassData = {
  pipeline: option(Pipeline.RayTracing.t),
  staticBindGroupDataArr: array(staticBindGroupData),
// indirectLightSpecularSampleCount: int
};

type preprocessPassData = {
  pipeline: option(Pipeline.Render.t),
  staticBindGroupDataArr: array(staticBindGroupData),
};

type regressionPassData = {
  pipeline: option(Pipeline.Compute.t),
  staticBindGroupDataArr: array(staticBindGroupData),
};

type postprocessPassData = {
  pipeline: option(Pipeline.Render.t),
  staticBindGroupDataArr: array(staticBindGroupData),
};

type taaPassData = {
  isFirstFrame: bool,
  firstFramePipeline: option(Pipeline.Render.t),
  firstFrameStaticBindGroupDataArr: array(staticBindGroupData),
  otherFramePipeline: option(Pipeline.Render.t),
  otherFrameStaticBindGroupDataArr: array(staticBindGroupData),
};


type accumulationPassData = {
  pipeline: option(Pipeline.Render.t),
  staticBindGroupData: option(staticBindGroupData),
  accumFrameCount: int,
  accumFrameCountForDenoise:int,
  lastViewMatrix:option(Js.Typed_array.Float32Array.t)
};


// type rayTracingPassData = {
//   pipeline: Pipeline.t,
//   indexBuffer: Buffer.t,
//   drawDataList:list(drawData)
// };

// type renderPassData = {
//   execOrder: int,
//   dynamicBindGroups: list(dynamicBindGroupData),
//   uniformBufferMap: ImmutableHashMap.t(bufferName, Buffer.t),
//   storageBufferMap: ImmutableHashMap.t(bufferName, Buffer.t),
//   textureViewMap: ImmutableHashMap.t(textureViewName, TextureView.t),
// };

// // type traceRayData = {
// //   rayGenerationOffset: int,
// //   rayCloestHitOffset: int,
// //   rayMissOffset: int,
// //   queryWidthDimension: int,
// //   queryHeightDimension: int,
// //   queryDepthDimension: int,
// // };

// type rayTracingPassData = {
//   execOrder: int,
//   dynamicBindGroups: list(dynamicBindGroupData),
//   uniformBufferMap: ImmutableHashMap.t(bufferName, Buffer.t),
//   storageBufferMap: ImmutableHashMap.t(bufferName, Buffer.t),
//   textureViewMap: ImmutableHashMap.t(textureViewName, TextureView.t),
//   traceRayData,
// };

type uniformBuffer = Buffer.t;
type uniformBufferData = Float32Array.t;

type storageBuffer = Buffer.t;
type storageBufferSize = int;

type jitterX = float;
type jitterY = float;
type jitter = (jitterX, jitterY);

type pass = {
  gbufferPassData,
  rayTracingPassData,
  preprocessPassData,
  regressionPassData,
  postprocessPassData,
  taaPassData,
  accumulationPassData,
  accumulatedFrameIndex: int,
  jitterArr: array(jitter),
  uniformBufferDataMap:
    ImmutableHashMap.t(bufferName, (uniformBufferData, uniformBuffer)),
  storageBufferDataMap:
    ImmutableHashMap.t(bufferName, (storageBufferSize, storageBuffer)),
  float32StorageBufferTypeArrayDataMap:
    ImmutableHashMap.t(bufferName, Float32Array.t),
  textureViewMap: ImmutableHashMap.t(textureViewName, TextureView.t),
};

type instanceId = int;

type accelerationContainer = {
  geometryContainers: option(array(AccelerationContainer.t)),
  instanceContainer: option(AccelerationContainer.t),
};

type rayTracing = {accelerationContainer};

type passFuncData = {
  init: state => state,
  execute: state => state,
}
and director = {
  time: float,
  frameIndex: int,
  initFuncArr: array(state => state),
  updateFuncArr: array(state => state),
  passFuncDataArr: array(passFuncData),
}
and state = {
  director,
  gameObject,
  transform,
  geometry,
  pbrMaterial,
  shader,
  directionLight,
  transformAnimation,
  cameraView,
  arcballCameraController,
  pass,
  rayTracing,
};

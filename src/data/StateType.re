open WebGPU;

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
      (array(float), array(float), array(float)),
    ),
  indexDataMap: ImmutableSparseMap.t(GeometryType.geometry, array(int)),
};

type color3 = (float, float, float);

type phongMaterial = {
  index: int,
  ambientMap: ImmutableSparseMap.t(PhongMaterialType.phongMaterial, color3),
  diffuseMap: ImmutableSparseMap.t(PhongMaterialType.phongMaterial, color3),
  specularMap: ImmutableSparseMap.t(PhongMaterialType.phongMaterial, color3),
  shininessMap: ImmutableSparseMap.t(PhongMaterialType.phongMaterial, float),
  illumMap: ImmutableSparseMap.t(PhongMaterialType.phongMaterial, int),
  dissolveMap: ImmutableSparseMap.t(PhongMaterialType.phongMaterial, float),
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
  viewMatrixInverseMap:
    ImmutableSparseMap.t(
      CameraViewType.cameraView,
      Js.Typed_array.Float32Array.t,
    ),
  lastViewMatrixInverseMap:
    ImmutableSparseMap.t(
      CameraViewType.cameraView,
      option(Js.Typed_array.Float32Array.t),
    ),
  projectionMatrixInverseMap:
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
  phongMaterialMap:
    ImmutableSparseMap.t(
      GameObjectType.gameObject,
      PhongMaterialType.phongMaterial,
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

type cameraBufferData = {
  cameraData: option(Js.Typed_array.Float32Array.t),
  cameraBuffer: option(Buffer.t),
};

type uniformBuffer = {cameraBufferData};

type gpuBuffer = {uniformBuffer};

type state = {
  gameObject,
  transform,
  geometry,
  phongMaterial,
  shader,
  directionLight,
  transformAnimation,
  cameraView,
  arcballCameraController,
  gpuBuffer,
};

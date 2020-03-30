open StateType;

let create = () => {
  gameObject: {
    index: 0,
    transformMap: ImmutableSparseMap.createEmpty(),
    geometryMap: ImmutableSparseMap.createEmpty(),
    phongMaterialMap: ImmutableSparseMap.createEmpty(),
    shaderMap: ImmutableSparseMap.createEmpty(),
    directionLightMap: ImmutableSparseMap.createEmpty(),
    transformAnimationMap: ImmutableSparseMap.createEmpty(),
    cameraViewMap: ImmutableSparseMap.createEmpty(),
    arcballCameraControllerMap: ImmutableSparseMap.createEmpty(),
  },

  transform: {
    index: 0,
    translationMap: ImmutableSparseMap.createEmpty(),
    rotationMap: ImmutableSparseMap.createEmpty(),
    scaleMap: ImmutableSparseMap.createEmpty(),
  },
  geometry: {
    index: 0,
    vertexDataMap: ImmutableSparseMap.createEmpty(),
    indexDataMap: ImmutableSparseMap.createEmpty(),
  },
  phongMaterial: {
    index: 0,
    ambientMap: ImmutableSparseMap.createEmpty(),
    diffuseMap: ImmutableSparseMap.createEmpty(),
    specularMap: ImmutableSparseMap.createEmpty(),
    shininessMap: ImmutableSparseMap.createEmpty(),
    illumMap: ImmutableSparseMap.createEmpty(),
    dissolveMap: ImmutableSparseMap.createEmpty(),
  },
  directionLight: {
    index: 0,
    intensityMap: ImmutableSparseMap.createEmpty(),
    positionMap: ImmutableSparseMap.createEmpty(),
  },
  shader: {
    index: 0,
    hitGroupIndexMap: ImmutableSparseMap.createEmpty(),
  },
  transformAnimation: {
    index: 0,
    isAnimate: false,
    dynamicTransformMap: ImmutableSparseMap.createEmpty(),
  },
  arcballCameraController: {
    index: 0,
    isDrag: false,
    currentArcballCameraController: None,
    phiMap: ImmutableSparseMap.createEmpty(),
    thetaMap: ImmutableSparseMap.createEmpty(),
    targetMap: ImmutableSparseMap.createEmpty(),
    rotateSpeedMap: ImmutableSparseMap.createEmpty(),
    wheelSpeedMap: ImmutableSparseMap.createEmpty(),
    distanceMap: ImmutableSparseMap.createEmpty(),
  },
  cameraView: {
    index: 0,
    currentCameraView: None,
    viewMatrixInverseMap: ImmutableSparseMap.createEmpty(),
    lastViewMatrixInverseMap: ImmutableSparseMap.createEmpty(),
    projectionMatrixInverseMap: ImmutableSparseMap.createEmpty(),
  },
  gpuBuffer: {
    uniformBuffer: {
      cameraBufferData: {
        cameraData: None,
        cameraBuffer: None,
      },
    },
  },
};

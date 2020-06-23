open WebGPU;


let _createSphere = ((translation, rotation), (diffuse, specular, metalness, roughness), state) =>{
  let (sphere, state) = GameObject.create(state);

  let (tran3, state) = Transform.create(state);
  let state =
    state
    |> Transform.setTranslation(tran3, translation)
    |> Transform.setRotation(tran3, rotation)
    |> Transform.setScale(tran3, (1.,1.,1.));

    let radius = 2.;
    let bands = 20;

  let (geo3, state) = Geometry.create(state);
  let state =
    state
    |> Geometry.setVertexData(geo3, Geometry.buildSphereVertexData(radius, bands))
    |> Geometry.setIndexData(geo3, Geometry.buildSphereIndexData(bands));

  let (mat3, state) = PBRMaterial.create(state);
  let state =
    state
    |> PBRMaterial.setDiffuse(mat3, diffuse)
    |> PBRMaterial.setSpecular(mat3, specular)
    |> PBRMaterial.setMetalness(mat3, Js.Math.min_float(metalness, 0.99))
    |> PBRMaterial.setRoughness(mat3, Js.Math.max_float(0.01, roughness));

  let (shader3, state) = Shader.create(state);
  let state = state |> Shader.setHitGroupIndex(shader3, 0);

  let state =
    state
    |> GameObject.addTransform(sphere, tran3)
    |> GameObject.addGeometry(sphere, geo3)
    |> GameObject.addPBRMaterial(sphere, mat3)
    |> GameObject.addShader(sphere, shader3);

    state
};

let _createPlane = ((translation, rotation), (diffuse, specular, metalness, roughness), state) =>{
  let (plane3, state) = GameObject.create(state);

  let (tran3, state) = Transform.create(state);
  let state =
    state
    |> Transform.setTranslation(tran3, translation)
    |> Transform.setRotation(tran3, rotation)
    |> Transform.setScale(tran3, (50., 50., 50.));

  let (geo3, state) = Geometry.create(state);
  let state =
    state
    |> Geometry.setVertexData(geo3, Geometry.buildPlaneVertexData())
    |> Geometry.setIndexData(geo3, Geometry.buildPlaneIndexData());

  let (mat3, state) = PBRMaterial.create(state);
  let state =
    state
    |> PBRMaterial.setDiffuse(mat3, diffuse)
    |> PBRMaterial.setSpecular(mat3, specular)
    |> PBRMaterial.setMetalness(mat3, metalness)
    |> PBRMaterial.setRoughness(mat3, roughness);

  let (shader3, state) = Shader.create(state);
  let state = state |> Shader.setHitGroupIndex(shader3, 0);

  let state =
    state
    |> GameObject.addTransform(plane3, tran3)
    |> GameObject.addGeometry(plane3, geo3)
    |> GameObject.addPBRMaterial(plane3, mat3)
    |> GameObject.addShader(plane3, shader3);

    state
};

let _buildScene1 = state => {
  let (light1, state) = GameObject.create(state);

  let (directionLight1, state) = DirectionLight.create(state);
  let state =
    state
    |> DirectionLight.setIntensity(directionLight1, 1.)
    |> DirectionLight.setPosition(directionLight1, (0., 1., 1.));

  let state = state |> GameObject.addDirectionLight(light1, directionLight1);

  let (light2, state) = GameObject.create(state);

  let (directionLight2, state) = DirectionLight.create(state);
  let state =
    state
    |> DirectionLight.setIntensity(directionLight2, 0.5)
    |> DirectionLight.setPosition(directionLight2, (1., 2., 1.));

  let state = state |> GameObject.addDirectionLight(light2, directionLight2);

  let (camera1, state) = GameObject.create(state);

  let (cameraView1, state) = CameraView.create(state);
  let state = state |> CameraView.setCurrentCameraView(cameraView1);

  let (arcballCameraController1, state) =
    ArcballCameraController.create(state);
  let state =
    state
    |> ArcballCameraController.setCurrentArcballCameraController(
         arcballCameraController1,
       )
    |> ArcballCameraController.setPhi(
         arcballCameraController1,
         Js.Math._PI /. 2.,
       )
    |> ArcballCameraController.setTheta(
         arcballCameraController1,
         Js.Math._PI /. 2.,
       )
    |> ArcballCameraController.setTarget(
         arcballCameraController1,
         (0., 0., 0.),
       )
    |> ArcballCameraController.setRotateSpeed(arcballCameraController1, 1.)
    |> ArcballCameraController.setWheelSpeed(arcballCameraController1, 1.)
    |> ArcballCameraController.setDistance(arcballCameraController1, 20.);

  let state =
    state
    |> GameObject.addCameraView(camera1, cameraView1)
    |> GameObject.addArcballCameraController(
         camera1,
         arcballCameraController1,
       );

  let (triangle1, state) = GameObject.create(state);

  let (tran1, state) = Transform.create(state);
  let state =
    state
    // |> Transform.setTranslation(tran1, (0., (-5.), 0.))
    |> Transform.setTranslation(tran1, (0., 0., 0.))
    // |> Transform.setRotation(tran1, (30., 45., 0.))
    |> Transform.setRotation(tran1, (0., 20., 0.))
    |> Transform.setScale(tran1, (1., 1., 1.));

  let (geo1, state) = Geometry.create(state);
  let state =
    state
    |> Geometry.setVertexData(geo1, Geometry.buildTriangleVertexData())
    |> Geometry.setIndexData(geo1, Geometry.buildTriangleIndexData());

  let (mat1, state) = PBRMaterial.create(state);
  let state =
    state
    |> PBRMaterial.setDiffuse(mat1, (1.0, 0., 0.))
    |> PBRMaterial.setSpecular(mat1, 0.95)
    |> PBRMaterial.setMetalness(mat1, 0.5)
    |> PBRMaterial.setRoughness(mat1, 0.5);
  // |> PBRMaterial.setIllum(mat1, 2)
  // |> PBRMaterial.setDissolve(mat1, 1.);

  let (shader1, state) = Shader.create(state);
  let state = state |> Shader.setHitGroupIndex(shader1, 0);

  let (transformAnim1, state) = TransformAnimation.create(state);
  let state =
    state |> TransformAnimation.setDynamicTransform(transformAnim1, 0);

  let state =
    state
    |> GameObject.addTransform(triangle1, tran1)
    |> GameObject.addGeometry(triangle1, geo1)
    |> GameObject.addPBRMaterial(triangle1, mat1)
    |> GameObject.addShader(triangle1, shader1)
    |> GameObject.addTransformAnimation(triangle1, transformAnim1);

  let (triangle2, state) = GameObject.create(state);

  let (tran2, state) = Transform.create(state);
  let state =
    state
    |> Transform.setTranslation(tran2, (3., 0., 5.))
    |> Transform.setRotation(tran2, (0., 70., 0.))
    |> Transform.setScale(tran2, (5., 5., 5.));

  let (geo2, state) = Geometry.create(state);
  let state =
    state
    |> Geometry.setVertexData(geo2, Geometry.buildTriangleVertexData())
    |> Geometry.setIndexData(geo2, Geometry.buildTriangleIndexData());

  let (mat2, state) = PBRMaterial.create(state);
  let state =
    state
    |> PBRMaterial.setDiffuse(mat2, (0.0, 0.5, 0.5))
    |> PBRMaterial.setSpecular(mat2, 0.95)
    |> PBRMaterial.setMetalness(mat2, 0.5)
    |> PBRMaterial.setRoughness(mat2, 0.5);

  let (shader2, state) = Shader.create(state);
  let state = state |> Shader.setHitGroupIndex(shader2, 0);

  // let (transformAnim2, state) = TransformAnimation.create(state);
  // let state =
  //   state |> TransformAnimation.setDynamicTransform(transformAnim2, 0);

  let state =
    state
    |> GameObject.addTransform(triangle2, tran2)
    |> GameObject.addGeometry(triangle2, geo2)
    |> GameObject.addPBRMaterial(triangle2, mat2)
    |> GameObject.addShader(triangle2, shader2);
  // |> GameObject.addTransformAnimation(triangle2, transformAnim2);


 let state = _createSphere (((10.0, 0.0, 10.0), (0.,0.,0.)), ((Js.Math.random(),0.0,Js.Math.random()), 0.95, Js.Math.random(), Js.Math.random()), state);

// let state = _createPlane (((0., (-10.), (-5.)), (0., 0., 0.)), ((0., 1., 0.), 0.95, 0.9, 0.1), state);
let state = _createPlane (((0., (-10.), (-5.)), (0., 0., 0.)), ((0., 1., 0.), 0.95, 0.9, 0.3), state);



  state;
};


let _buildScene2 = state => {
  let (light1, state) = GameObject.create(state);

  let (directionLight1, state) = DirectionLight.create(state);
  let state =
    state
    |> DirectionLight.setIntensity(directionLight1, 1.)
    |> DirectionLight.setPosition(directionLight1, (0., 1., 1.));

  let state = state |> GameObject.addDirectionLight(light1, directionLight1);

  let (light2, state) = GameObject.create(state);

  let (directionLight2, state) = DirectionLight.create(state);
  let state =
    state
    |> DirectionLight.setIntensity(directionLight2, 0.5)
    |> DirectionLight.setPosition(directionLight2, (1., 2., 1.));

  let state = state |> GameObject.addDirectionLight(light2, directionLight2);

  let (camera1, state) = GameObject.create(state);

  let (cameraView1, state) = CameraView.create(state);
  let state = state |> CameraView.setCurrentCameraView(cameraView1);

  let (arcballCameraController1, state) =
    ArcballCameraController.create(state);
  let state =
    state
    |> ArcballCameraController.setCurrentArcballCameraController(
         arcballCameraController1,
       )
    |> ArcballCameraController.setPhi(
         arcballCameraController1,
         Js.Math._PI /. 2.,
       )
    |> ArcballCameraController.setTheta(
         arcballCameraController1,
         Js.Math._PI /. 2.,
       )
    |> ArcballCameraController.setTarget(
         arcballCameraController1,
         (0., 0., 0.),
       )
    |> ArcballCameraController.setRotateSpeed(arcballCameraController1, 1.)
    |> ArcballCameraController.setWheelSpeed(arcballCameraController1, 1.)
    |> ArcballCameraController.setDistance(arcballCameraController1, 20.);

  let state =
    state
    |> GameObject.addCameraView(camera1, cameraView1)
    |> GameObject.addArcballCameraController(
         camera1,
         arcballCameraController1,
       );

  let (triangle1, state) = GameObject.create(state);

  let (tran1, state) = Transform.create(state);
  let state =
    state
    // |> Transform.setTranslation(tran1, (0., (-5.), 0.))
    |> Transform.setTranslation(tran1, (0., 0., 0.))
    // |> Transform.setRotation(tran1, (30., 45., 0.))
    |> Transform.setRotation(tran1, (0., 20., 0.))
    |> Transform.setScale(tran1, (1., 1., 1.));

  let (geo1, state) = Geometry.create(state);
  let state =
    state
    |> Geometry.setVertexData(geo1, Geometry.buildTriangleVertexData())
    |> Geometry.setIndexData(geo1, Geometry.buildTriangleIndexData());

  let (mat1, state) = PBRMaterial.create(state);
  let state =
    state
    |> PBRMaterial.setDiffuse(mat1, (1.0, 0., 0.))
    |> PBRMaterial.setSpecular(mat1, 0.95)
    |> PBRMaterial.setMetalness(mat1, 0.5)
    |> PBRMaterial.setRoughness(mat1, 0.5);
  // |> PBRMaterial.setIllum(mat1, 2)
  // |> PBRMaterial.setDissolve(mat1, 1.);

  let (shader1, state) = Shader.create(state);
  let state = state |> Shader.setHitGroupIndex(shader1, 0);

  let (transformAnim1, state) = TransformAnimation.create(state);
  let state =
    state |> TransformAnimation.setDynamicTransform(transformAnim1, 0);

  let state =
    state
    |> GameObject.addTransform(triangle1, tran1)
    |> GameObject.addGeometry(triangle1, geo1)
    |> GameObject.addPBRMaterial(triangle1, mat1)
    |> GameObject.addShader(triangle1, shader1)
    |> GameObject.addTransformAnimation(triangle1, transformAnim1);

  let (triangle2, state) = GameObject.create(state);

  let (tran2, state) = Transform.create(state);
  let state =
    state
    |> Transform.setTranslation(tran2, (3., 0., 5.))
    |> Transform.setRotation(tran2, (0., 70., 0.))
    |> Transform.setScale(tran2, (5., 5., 5.));

  let (geo2, state) = Geometry.create(state);
  let state =
    state
    |> Geometry.setVertexData(geo2, Geometry.buildTriangleVertexData())
    |> Geometry.setIndexData(geo2, Geometry.buildTriangleIndexData());

  let (mat2, state) = PBRMaterial.create(state);
  let state =
    state
    |> PBRMaterial.setDiffuse(mat2, (0.0, 0.5, 0.5))
    |> PBRMaterial.setSpecular(mat2, 0.95)
    |> PBRMaterial.setMetalness(mat2, 0.5)
    |> PBRMaterial.setRoughness(mat2, 0.5);

  let (shader2, state) = Shader.create(state);
  let state = state |> Shader.setHitGroupIndex(shader2, 0);

  // let (transformAnim2, state) = TransformAnimation.create(state);
  // let state =
  //   state |> TransformAnimation.setDynamicTransform(transformAnim2, 0);

  let state =
    state
    |> GameObject.addTransform(triangle2, tran2)
    |> GameObject.addGeometry(triangle2, geo2)
    |> GameObject.addPBRMaterial(triangle2, mat2)
    |> GameObject.addShader(triangle2, shader2);
  // |> GameObject.addTransformAnimation(triangle2, transformAnim2);


let state = 
               ArrayUtils.range(0, 300)
               |> ArrayUtils.reduceOneParam(
                    (. state, index) => {
//  _createSphere (((( index * 2 - 500 ) |> float_of_int, Js.Math.random() *. 10., Js.Math.random() *. 100. -. 50.), (0.,0.,0.)), ((Js.Math.random(),0.0,Js.Math.random()), 0.95, Js.Math.random(), Js.Math.random()), state);
 _createSphere (((Js.Math.random() *. 200. -. 100., Js.Math.random() *. 10., Js.Math.random() *. 100. -. 50.), (0.,0.,0.)), ((Js.Math.random(),0.0,Js.Math.random()), 0.95, Js.Math.random(), Js.Math.random()), state);
                    },
state
                  );

let state = _createPlane (((0., (-10.), (-5.)), (0., 0., 0.)), ((0., 1., 0.), 0.95, 0.9, 0.3), state);
let state = _createPlane (((0., (-10.), (-5. -. 50.)), (90., 0., 0.)), ((0., 0., 1.), 0.95, 0.5, 0.5), state);
let state = _createPlane (((0., (-10.), (-5. +. 50.)), (-90., 0., 0.)), ((0., 1., 0.), 0.95, 0.6, 0.3), state);


  state;
};

let buildScene = state => {
  _buildScene2(state)
};

let getAllRenderGameObjects = state => {
  GameObject.getAllGeometryGameObjects(state);
};

let _getAccumulatedFrameCount = () => 16;

let init = (device, window, state) => {
  let (resolutionBufferSize, resolutionBuffer) =
    BMFRBuffer.ResolutionBuffer.buildData(device, window);
  let (cameraBufferData, cameraBufferSize, cameraBuffer) =
    BMFRBuffer.CameraBuffer.buildData(device, state);

  let (
    rayTracingCommonDataBufferData,
    rayTracingCommonDataBufferSize,
    rayTracingCommonDataBuffer,
  ) =
    BMFRBuffer.RayTracingCommonDataBuffer.buildData(device, state);

  let (pixelBufferSize, pixelBuffer) =
    BMFRBuffer.PixelBuffer.buildData(device, window);

  let (prevNoisyPixelBufferSize, prevNoisyPixelBuffer) =
    BMFRBuffer.PrevNoisyPixelBuffer.buildData(device, window);
  let (prevPositionBufferSize, prevPositionBuffer) =
    BMFRBuffer.PrevPositionBuffer.buildData(device, window);
  let (prevNormalBufferSize, prevNormalBuffer) =
    BMFRBuffer.PrevNormalBuffer.buildData(device, window);
  // let (acceptBoolBufferSize, acceptBoolBuffer) =
  //   BMFRBuffer.AcceptBoolBuffer.buildData(device, window);
  // let (prevFramePixelIndicesBufferSize, prevFramePixelIndicesBuffer) =
  //   BMFRBuffer.PrevFramePixelIndicesBuffer.buildData(device, window);

  let (tmpDataBufferSize, tmpDataBuffer) =
    BMFRBuffer.Regression.TmpDataBuffer.buildData(device, window);
  let (outDataBufferSize, outDataBuffer) =
    BMFRBuffer.Regression.OutDataBuffer.buildData(device, window);
  let (
    regressionCommonDataBufferData,
    regressionCommonDataBufferSize,
    regressionCommonDataBuffer,
  ) =
    BMFRBuffer.Regression.CommonDataBuffer.buildData(device, window);

  // let (accumulatedPrevFramePixelBufferSize, accumulatedPrevFramePixelBuffer) =
  //   BMFRBuffer.AccumulatedPrevFramePixelBuffer.buildData(device, window);

  let (historyPixelBufferSize, historyPixelBuffer) =
    BMFRBuffer.HistoryPixelBuffer.buildData(device, window);
  let (taaBufferData, taaBufferSize, taaBuffer) =
    BMFRBuffer.TAABuffer.buildData(device, state);
  let (commonDataBufferData, commonDataBufferSize, commonDataBuffer) =
    BMFRBuffer.CommonDataBuffer.buildData(device, state);

  let (sceneDescBufferData, sceneDescBufferSize, sceneDescBuffer) =
    BMFRBuffer.GetHitShadingData.SceneDescBuffer.buildData(device, state);
  let (
    geometryOffsetDataBufferData,
    geometryOffsetDataBufferSize,
    geometryOffsetDataBuffer,
  ) =
    BMFRBuffer.GetHitShadingData.GeometryOffsetDataBuffer.buildData(
      device,
      state,
    );
  let (vertexBufferData, vertexBufferSize, vertexBuffer) =
    BMFRBuffer.GetHitShadingData.VertexBuffer.buildData(device, state);
  let (indexBufferData, indexBufferSize, indexBuffer) =
    BMFRBuffer.GetHitShadingData.IndexBuffer.buildData(device, state);
  let (pbrMaterialBufferData, pbrMaterialBufferSize, pbrMaterialBuffer) =
    BMFRBuffer.GetHitShadingData.PBRMaterialBuffer.buildData(device, state);

  state
  |> Pass.setAccumulatedFrameIndex(0)
  |> Pass.setJitterArr(
       TAAJitter.generateHaltonJiters(
         _getAccumulatedFrameCount(),
         //  |> Log.printComplete("_getAccumulatedFrameCount")
         Window.getWidth(window),
         Window.getHeight(window),
       ),
       //  |> Log.printComplete("HaltonJiters")
     )
  |> BMFRBuffer.ResolutionBuffer.setBufferData((
       resolutionBufferSize,
       resolutionBuffer,
     ))
  |> BMFRBuffer.CameraBuffer.setBufferData((cameraBufferData, cameraBuffer))
  |> BMFRBuffer.RayTracingCommonDataBuffer.setBufferData((
       rayTracingCommonDataBufferData,
       rayTracingCommonDataBuffer,
     ))
  |> BMFRBuffer.TAABuffer.setBufferData((taaBufferData, taaBuffer))
  |> BMFRBuffer.CommonDataBuffer.setBufferData((
       commonDataBufferData,
       commonDataBuffer,
     ))
  |> BMFRBuffer.PixelBuffer.setBufferData((pixelBufferSize, pixelBuffer))
  |> BMFRBuffer.PrevNoisyPixelBuffer.setBufferData((
       prevNoisyPixelBufferSize,
       prevNoisyPixelBuffer,
     ))
  |> BMFRBuffer.PrevPositionBuffer.setBufferData((
       prevPositionBufferSize,
       prevPositionBuffer,
     ))
  |> BMFRBuffer.PrevNormalBuffer.setBufferData((
       prevNormalBufferSize,
       prevNormalBuffer,
     ))
  // |> BMFRBuffer.AcceptBoolBuffer.setBufferData((
  //      acceptBoolBufferSize,
  //      acceptBoolBuffer,
  //    ))
  // |> BMFRBuffer.PrevFramePixelIndicesBuffer.setBufferData((
  //      prevFramePixelIndicesBufferSize,
  //      prevFramePixelIndicesBuffer,
  //    ))
  |> BMFRBuffer.Regression.TmpDataBuffer.setBufferData((
       tmpDataBufferSize,
       tmpDataBuffer,
     ))
  |> BMFRBuffer.Regression.OutDataBuffer.setBufferData((
       outDataBufferSize,
       outDataBuffer,
     ))
  |> BMFRBuffer.Regression.CommonDataBuffer.setBufferData((
       regressionCommonDataBufferData,
       regressionCommonDataBuffer,
     ))
  // |> BMFRBuffer.AccumulatedPrevFramePixelBuffer.setBufferData((
  //      accumulatedPrevFramePixelBufferSize,
  //      accumulatedPrevFramePixelBuffer,
  //    ))
  |> BMFRBuffer.HistoryPixelBuffer.setBufferData((
       historyPixelBufferSize,
       historyPixelBuffer,
     ))
  |> BMFRBuffer.GetHitShadingData.SceneDescBuffer.setBufferData((
       sceneDescBufferSize,
       sceneDescBufferData,
       sceneDescBuffer,
     ))
  |> BMFRBuffer.GetHitShadingData.GeometryOffsetDataBuffer.setBufferData((
       geometryOffsetDataBufferSize,
       geometryOffsetDataBuffer,
     ))
  |> BMFRBuffer.GetHitShadingData.VertexBuffer.setBufferData((
       vertexBufferSize,
       vertexBuffer,
     ))
  |> BMFRBuffer.GetHitShadingData.IndexBuffer.setBufferData((
       indexBufferSize,
       indexBuffer,
     ))
  |> BMFRBuffer.GetHitShadingData.PBRMaterialBuffer.setBufferData((
       pbrMaterialBufferSize,
       pbrMaterialBuffer,
     ));
};

let _updateRayTracingData = state => {
  state
  |> BMFRBuffer.CommonDataBuffer.update(
       Director.getFrameIndex(state),
       DirectionLight.getLightCount(state),
     );
};

let _updateRegressionData = state => {
  state
  |> BMFRBuffer.Regression.CommonDataBuffer.update(
       Director.getFrameIndex(state),
     );
};

let _updateCameraData = (window, state) => {
  let currentCameraView = state |> CameraView.unsafeGetCurrentCameraView;

  let lastViewProjectionMatrixOpt =
    Pass.GBufferPass.getLastViewProjectionMatrix(state);

  let currentArcballCameraController =
    state |> ArcballCameraController.unsafeGetCurrentArcballCameraController;
  let state =
    state
    |> CameraView.update(
         (
           (Window.getWidth(window) |> float_of_int)
           /. (Window.getHeight(window) |> float_of_int),
           180.0 *. 2. /. 5.,
           0.1,
           4096.0,
         ),
         (
           ArcballCameraController.getLookFrom(
             currentArcballCameraController,
             state,
           ),
           ArcballCameraController.unsafeGetTarget(
             currentArcballCameraController,
             state,
           ),
           (0., 1., 0.),
         ),
       );

  let viewMatrix = CameraView.unsafeGetViewMatrix(currentCameraView, state);
  let projectionMatrix =
    CameraView.unsafeGetProjectionMatrix(currentCameraView, state);
  let state =
    state
    |> Pass.GBufferPass.setLastViewProjectionMatrix(
         Matrix4.createIdentityMatrix4()
         |> Matrix4.multiply(projectionMatrix, viewMatrix),
       );

  let state =
    state
    |> BMFRBuffer.CameraBuffer.update(
         CameraView.unsafeGetCameraPosition(currentCameraView, state),
         viewMatrix,
         projectionMatrix,
         lastViewProjectionMatrixOpt,
       );

  state;
};

let _updateJitterData = state => {
  let state =
    state
    |> BMFRBuffer.TAABuffer.update(
         Pass.getJitter(
           Pass.getAccumulatedFrameIndex(state),
           //  |> Log.printComplete("getAccumulatedFrameCount")
           state,
         ),
       );

  state
  |> Pass.setAccumulatedFrameIndex(
       (Pass.getAccumulatedFrameIndex(state) |> succ)
       mod _getAccumulatedFrameCount(),
     );
};

let _updateTransformAnim = (time, state) => {
  TransformAnimation.getAllDynamicTransforms(state)
  |> ArrayUtils.reduceOneParam(
       (. state, transform) => {
         let (tx, ty, tz) = Transform.getTranslation(transform, state);
         let (rx, ry, rz) = Transform.getRotation(transform, state);

         let newTx = tx +. 0.3 *. time /. 20.0;
         let newTx = newTx > 10.0 ? 0.0 : newTx;

         state
         |> Transform.setTranslation(transform, (newTx, ty, tz))
         |> Transform.setRotation(transform, (rx, ry +. time /. 20.0, rz));
       },
       state,
     );
};

let _updateTransformData = (time, device, queue, state) => {
  let allRenderGameObjects = getAllRenderGameObjects(state);

  let state =
    allRenderGameObjects
    |> ArrayUtils.reduceOneParam(
         (. state, renderGameObject) => {
           state
           |> Pass.GBufferPass.setLastModelMatrix(
                GameObject.unsafeGetTransform(renderGameObject, state),
                Transform.buildModelMatrix(
                  GameObject.unsafeGetTransform(renderGameObject, state),
                  state,
                ),
              )
         },
         state,
       );

  let state = state |> _updateTransformAnim(time);

  let state =
    state
    |> ManageAccelerationContainer.updateInstanceContainer(device, queue);

  let state =
    state
    |> BMFRBuffer.ModelBuffer.update(allRenderGameObjects)
    |> BMFRBuffer.GetHitShadingData.SceneDescBuffer.update(
         allRenderGameObjects,
       );

  state;
};

let update = (device, queue, window, state) => {
  state
  |> _updateRayTracingData
  |> _updateRegressionData
  |> _updateCameraData(window)
  |> _updateTransformData(Director.getTime(state), device, queue)
  |> _updateJitterData;
};

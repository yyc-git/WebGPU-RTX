open WebGPU;

let buildScene = state => {
  let (light1, state) = GameObject.create(state);

  let (directionLight1, state) = DirectionLight.create(state);
  let state =
    state
    |> DirectionLight.setIntensity(directionLight1, 1.)
    |> DirectionLight.setPosition(directionLight1, (0., 1., 1.));

  let state = state |> GameObject.addDirectionLight(light1, directionLight1);

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
    |> Transform.setTranslation(tran1, (0., (-5.), 0.))
    |> Transform.setRotation(tran1, (30., 45., 0.))
    |> Transform.setScale(tran1, (1., 1., 1.));

  let (geo1, state) = Geometry.create(state);
  let state =
    state
    |> Geometry.setVertexData(geo1, Geometry.buildTriangleVertexData())
    |> Geometry.setIndexData(geo1, Geometry.buildTriangleIndexData());

  let (mat1, state) = PhongMaterial.create(state);
  let state =
    state
    // |> PhongMaterial.setAmbient(mat1, (0.1, 0.1, 0.1))
    |> PhongMaterial.setDiffuse(mat1, (1.0, 0., 0.))
    // |> PhongMaterial.setSpecular(mat1, (0.2, 0.0, 1.0))
    |> PhongMaterial.setShininess(mat1, 36.);
  // |> PhongMaterial.setIllum(mat1, 2)
  // |> PhongMaterial.setDissolve(mat1, 1.);

  let (shader1, state) = Shader.create(state);
  let state = state |> Shader.setHitGroupIndex(shader1, 0);

  let (transformAnim1, state) = TransformAnimation.create(state);
  let state =
    state |> TransformAnimation.setDynamicTransform(transformAnim1, 0);

  let state =
    state
    |> GameObject.addTransform(triangle1, tran1)
    |> GameObject.addGeometry(triangle1, geo1)
    |> GameObject.addPhongMaterial(triangle1, mat1)
    |> GameObject.addShader(triangle1, shader1)
    |> GameObject.addTransformAnimation(triangle1, transformAnim1);

  let (plane1, state) = GameObject.create(state);

  let (tran2, state) = Transform.create(state);
  let state =
    state
    |> Transform.setTranslation(tran2, (0., (-10.), (-5.)))
    |> Transform.setRotation(tran2, (0., 0., 0.))
    |> Transform.setScale(tran2, (10., 10., 10.));

  let (geo2, state) = Geometry.create(state);
  let state =
    state
    |> Geometry.setVertexData(geo2, Geometry.buildPlaneVertexData())
    |> Geometry.setIndexData(geo2, Geometry.buildPlaneIndexData());

  let (mat2, state) = PhongMaterial.create(state);
  let state =
    state
    // |> PhongMaterial.setAmbient(mat2, (0.1, 0.1, 0.1))
    |> PhongMaterial.setDiffuse(mat2, (0.0, 1., 0.))
    // |> PhongMaterial.setSpecular(mat2, (0.5, 0.0, 0.5))
    |> PhongMaterial.setShininess(mat2, 72.);
  // |> PhongMaterial.setIllum(mat2, 2)
  // |> PhongMaterial.setDissolve(mat2, 1.);

  let (shader2, state) = Shader.create(state);
  let state = state |> Shader.setHitGroupIndex(shader2, 1);

  let state =
    state
    |> GameObject.addTransform(plane1, tran2)
    |> GameObject.addGeometry(plane1, geo2)
    |> GameObject.addPhongMaterial(plane1, mat2)
    |> GameObject.addShader(plane1, shader2);

  state;
};

let getAllRenderGameObjects = state => {
  GameObject.getAllGeometryGameObjects(state);
};

let init = (device, window, state) => {
  let (cameraBufferData, cameraBufferSize, cameraBuffer) =
    TAABuffer.CameraBuffer.buildData(device, state);
  let (pixelBufferSize, pixelBuffer) =
    ManageBuffer.StorageBuffer.buildPixelBufferData(window, device);

  state
  |> Pass.setUniformBufferData(
       "cameraBuffer",
       (cameraBufferData, cameraBuffer),
     )
  |> Pass.setStorageBufferData("pixelBuffer", (pixelBufferSize, pixelBuffer));
};

let update = (window, time, state) => {
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

  let currentCameraView = state |> CameraView.unsafeGetCurrentCameraView;
  let state =
    state
    |> TAABuffer.CameraBuffer.update(
         CameraView.unsafeGetCameraPosition(currentCameraView, state),
         CameraView.unsafeGetViewMatrix(currentCameraView, state),
         CameraView.unsafeGetProjectionMatrix(currentCameraView, state),
       );

  state;
};

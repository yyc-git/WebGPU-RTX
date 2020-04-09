open Wonder_jest;

open Js.Typed_array;

let _ =
  describe("ManageAccelerationContainer_test", () => {
    open Expect;
    open Expect.Operators;
    open Sinon;

    let sandbox = getSandboxDefaultVal();
    let state = ref(TestTool.createState());
    beforeEach(() => {
      sandbox := createSandbox();
      state := TestTool.init(~sandbox, ());
    });
    afterEach(() => restoreSandbox(refJsObjToSandbox(sandbox^)));

    describe("buildContainers", () => {
      let _buildFakeBuffer = sandbox => {
        {"setSubData": createEmptyStubWithJsObjSandbox(sandbox)} |> Obj.magic;
      };

      let _buildFakeCommandEncoder = sandbox => {
        {
          "buildRayTracingAccelerationContainer":
            createEmptyStubWithJsObjSandbox(sandbox),
          "finish": createEmptyStubWithJsObjSandbox(sandbox),
        }
        |> Obj.magic;
      };

      let _buildFakeDevice =
          (
            ~sandbox,
            ~buffers=[||],
            ~commandEncoder=_buildFakeCommandEncoder(sandbox),
            (),
          ) => {
        let createBufferStub =
          createEmptyStubWithJsObjSandbox(sandbox)
          |> returns(_buildFakeBuffer(sandbox));

        // let createBufferStub =
        //   buffers
        //   |> ArrayUtils.reduceOneParami(
        //        (. createBufferStub, buffer, i) => {
        //          createBufferStub |> onCall(i) |> returns(buffer)
        //        },
        //        createBufferStub,
        //      );

        buffers
        |> Js.Array.forEachi((buffer, i) => {
             createBufferStub |> onCall(i) |> returns(buffer) |> ignore;
             ();
           });

        {
          "createBuffer": createBufferStub,
          "createRayTracingAccelerationContainer":
            createEmptyStubWithJsObjSandbox(sandbox)
            |> returns({
                 "getHandle":
                   createEmptyStubWithJsObjSandbox(sandbox)
                   |> returns(NumberTool.buildBigInt(100)),
               }),
          "createCommandEncoder":
            createEmptyStubWithJsObjSandbox(sandbox)
            |> returns(commandEncoder),
        }
        |> Obj.magic;
      };

      let _buildFakeQueue = sandbox => {
        {"submit": createEmptyStubWithJsObjSandbox(sandbox)} |> Obj.magic;
      };

      describe("test geometry containers", () => {
        let _build = state => {
          let (triangle1, state) = GameObject.create(state);

          let (tran1, state) = Transform.create(state);
          let state =
            state
            |> Transform.setTranslation(tran1, (0., 0., 0.))
            |> Transform.setRotation(tran1, (30., 45., 0.))
            |> Transform.setScale(tran1, (1., 1., 1.));

          let (geo1, state) = Geometry.create(state);
          let state =
            state
            |> Geometry.setVertexData(
                 geo1,
                 Geometry.buildTriangleVertexData(),
               )
            |> Geometry.setIndexData(geo1, Geometry.buildTriangleIndexData());

          let (shader1, state) = Shader.create(state);
          let state = state |> Shader.setHitGroupIndex(shader1, 0);

          let state =
            state
            |> GameObject.addTransform(triangle1, tran1)
            |> GameObject.addGeometry(triangle1, geo1)
            |> GameObject.addShader(triangle1, shader1);

          ((triangle1, (tran1, geo1, shader1)), state);
        };

        describe("test vertex buffer and index buffer", () => {
          test("test vertex buffer", () => {
            let ((triangle1, (tran1, geo1, shader1)), state) =
              _build(state^);

            let vertexBuffer = _buildFakeBuffer(sandbox);
            let indexBuffer = _buildFakeBuffer(sandbox);

            let device =
              _buildFakeDevice(
                ~sandbox,
                ~buffers=[|vertexBuffer, indexBuffer|],
                (),
              );

            let queue = _buildFakeQueue(sandbox);

            let _ =
              ManageAccelerationContainer.buildContainers(
                device,
                queue,
                state,
              );

            vertexBuffer##setSubData
            |> expect
            |> toCalledWith([|
                 0,
                 Float32Array.make(
                    Geometry.buildTriangleVertexData() |> Tuple2.getFirst,
                  //  Geometry.buildTriangleVertexData(),
                 )
                 |> Obj.magic,
               |]);
          });
          test("test index buffer", () => {
            let ((triangle1, (tran1, geo1, shader1)), state) =
              _build(state^);

            let vertexBuffer = _buildFakeBuffer(sandbox);
            let indexBuffer = _buildFakeBuffer(sandbox);

            let device =
              _buildFakeDevice(
                ~sandbox,
                ~buffers=[|vertexBuffer, indexBuffer|],
                (),
              );

            let queue = _buildFakeQueue(sandbox);

            let _ =
              ManageAccelerationContainer.buildContainers(
                device,
                queue,
                state,
              );

            indexBuffer##setSubData
            |> expect
            |> toCalledWith([|
                 0,
                 Uint32Array.make(Geometry.buildTriangleIndexData())
                 |> Obj.magic,
               |]);
          });
        });
      });

      describe("test instanceBuffer", () => {
        describe("fix bug", () => {
          let _build = state => {
            let (light1, state) = GameObject.create(state);

            let (directionLight1, state) = DirectionLight.create(state);
            let state =
              state
              |> DirectionLight.setIntensity(directionLight1, 1.)
              |> DirectionLight.setPosition(directionLight1, (0., 1., 1.));

            let state =
              state |> GameObject.addDirectionLight(light1, directionLight1);

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
              |> ArcballCameraController.setRotateSpeed(
                   arcballCameraController1,
                   1.,
                 )
              |> ArcballCameraController.setWheelSpeed(
                   arcballCameraController1,
                   1.,
                 )
              |> ArcballCameraController.setDistance(
                   arcballCameraController1,
                   20.,
                 );

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
              |> Transform.setTranslation(tran1, (0., 0., 0.))
              |> Transform.setRotation(tran1, (30., 45., 0.))
              |> Transform.setScale(tran1, (1., 1., 1.));

            let (geo1, state) = Geometry.create(state);
            let state =
              state
              |> Geometry.setVertexData(
                   geo1,
                   Geometry.buildTriangleVertexData(),
                 )
              |> Geometry.setIndexData(
                   geo1,
                   Geometry.buildTriangleIndexData(),
                 );

            let (mat1, state) = PhongMaterial.create(state);
            let state =
              state
              // |> PhongMaterial.setAmbient(mat1, (0.1, 0.1, 0.1))
              |> PhongMaterial.setDiffuse(mat1, (1.0, 0., 0.))
              |> PhongMaterial.setSpecular(mat1, (0.2, 0.0, 1.0))
              |> PhongMaterial.setShininess(mat1, 36.);
              // |> PhongMaterial.setIllum(mat1, 2)
              // |> PhongMaterial.setDissolve(mat1, 1.);

            let (shader1, state) = Shader.create(state);
            let state = state |> Shader.setHitGroupIndex(shader1, 0);

            let (transformAnim1, state) = TransformAnimation.create(state);
            let state =
              state
              |> TransformAnimation.setDynamicTransform(transformAnim1, 0);

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
              |> Transform.setTranslation(tran2, (0., (-10.), 0.))
              |> Transform.setRotation(tran2, (0., 0., 0.))
              |> Transform.setScale(tran2, (10., 10., 10.));

            let (geo2, state) = Geometry.create(state);
            let state =
              state
              |> Geometry.setVertexData(
                   geo2,
                   Geometry.buildPlaneVertexData(),
                 )
              |> Geometry.setIndexData(geo2, Geometry.buildPlaneIndexData());

            let (mat2, state) = PhongMaterial.create(state);
            let state =
              state
              // |> PhongMaterial.setAmbient(mat2, (0.1, 0.1, 0.1))
              |> PhongMaterial.setDiffuse(mat2, (0.0, 1., 0.))
              |> PhongMaterial.setSpecular(mat2, (0.5, 0.0, 0.5))
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

          test("test has gameObjects without geometry ", () => {
            let state = _build(state^);

            let vertexBuffer = _buildFakeBuffer(sandbox);
            let indexBuffer = _buildFakeBuffer(sandbox);
            let instanceBuffer = _buildFakeBuffer(sandbox);

            let device =
              _buildFakeDevice(
                ~sandbox,
                ~buffers=[|
                  vertexBuffer,
                  indexBuffer,
                  vertexBuffer,
                  indexBuffer,
                  instanceBuffer,
                |],
                (),
              );

            device##createBuffer |> onCall(4) |> returns(instanceBuffer);

            let queue = _buildFakeQueue(sandbox);

            let _ =
              ManageAccelerationContainer.buildContainers(
                device,
                queue,
                state,
              );

            let args = instanceBuffer##setSubData |> getCall(0) |> getArgs;
            let instanceBufferArrayBuffer =
              args |> List.nth(_, 1) |> Uint8Array.buffer;

            let dataView = DataView.make(instanceBufferArrayBuffer);
            DataView.getFloat32LittleEndian(dataView, 0)
            |> expect == 0.7071067690849304;
          });
        })
      });
    });
  });

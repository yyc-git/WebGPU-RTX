open StateType;

let getAccumulatedFrameIndex = state => {
  state.pass.accumulatedFrameIndex;
};

let setAccumulatedFrameIndex = (accumulatedFrameIndex, state) => {
  ...state,
  pass: {
    ...state.pass,
    accumulatedFrameIndex,
  },
};

let getJitter = (accumulatedFrameIndex, state) => {
  Array.unsafe_get(state.pass.jitterArr, accumulatedFrameIndex);
};

let setJitterArr = (jitterArr, state) => {
  ...state,
  pass: {
    ...state.pass,
    jitterArr,
  },
};

let unsafeGetUniformBufferData = (bufferName, state) => {
  state.pass.uniformBufferDataMap |> ImmutableHashMap.unsafeGet(bufferName);
};

let setUniformBufferData = (bufferName, (bufferData, buffer), state) => {
  ...state,
  pass: {
    ...state.pass,
    uniformBufferDataMap:
      state.pass.uniformBufferDataMap
      |> ImmutableHashMap.set(bufferName, (bufferData, buffer)),
  },
};

let unsafeGetStorageBufferData = (bufferName, state) => {
  state.pass.storageBufferDataMap |> ImmutableHashMap.unsafeGet(bufferName);
};

let setStorageBufferData = (bufferName, (bufferSize, buffer), state) => {
  ...state,
  pass: {
    ...state.pass,
    storageBufferDataMap:
      state.pass.storageBufferDataMap
      |> ImmutableHashMap.set(bufferName, (bufferSize, buffer)),
  },
};

let unsafeGetFloat32StorageBufferTypeArrayData = (bufferName, state) => {
  state.pass.float32StorageBufferTypeArrayDataMap
  |> ImmutableHashMap.unsafeGet(bufferName);
};

let setFloat32StorageBufferTypeArrayData = (bufferName, bufferData, state) => {
  ...state,
  pass: {
    ...state.pass,
    float32StorageBufferTypeArrayDataMap:
      state.pass.float32StorageBufferTypeArrayDataMap
      |> ImmutableHashMap.set(bufferName, bufferData),
  },
};

let unsafeGetTextureView = (textureViewName, state) => {
  state.pass.textureViewMap |> ImmutableHashMap.unsafeGet(textureViewName);
};

let setTextureView = (textureViewName, textureView, state) => {
  ...state,
  pass: {
    ...state.pass,
    textureViewMap:
      state.pass.textureViewMap
      |> ImmutableHashMap.set(textureViewName, textureView),
  },
};

module GBufferPass = {
  let _getPassData = state => {
    state.pass.gbufferPassData;
  };

  let unsafeGetLastModelMatrix = (transform, state) => {
    _getPassData(state).lastModelMatrixMap
    |> ImmutableSparseMap.unsafeGet(transform);
  };

  let setLastModelMatrix = (transform, lastModelMatrix, state) => {
    ...state,
    pass: {
      ...state.pass,
      gbufferPassData: {
        ..._getPassData(state),
        lastModelMatrixMap:
          _getPassData(state).lastModelMatrixMap
          |> ImmutableSparseMap.set(transform, lastModelMatrix),
      },
    },
  };

  let getLastViewProjectionMatrix = state => {
    _getPassData(state).lastViewProjectionMatrix;
  };

  let setLastViewProjectionMatrix = (lastViewProjectionMatrix, state) => {
    ...state,
    pass: {
      ...state.pass,
      gbufferPassData: {
        ..._getPassData(state),
        lastViewProjectionMatrix: Some(lastViewProjectionMatrix),
      },
    },
  };

  let unsafeGetDepthTextureView = state => {
    _getPassData(state).depthTextureView |> Js.Option.getExn;
  };

  let setDepthTextureView = (depthTextureView, state) => {
    ...state,
    pass: {
      ...state.pass,
      gbufferPassData: {
        ..._getPassData(state),
        depthTextureView: Some(depthTextureView),
      },
    },
  };

  // let buildStaticBindGroupData = (setSlot, bindGroup) => {
  //   setSlot,
  //   bindGroup,
  // };

  // let buildDynamicBindGroupData = (setSlot, bindGroup, alignedUniformBytes) => {
  //   setSlot,
  //   bindGroup,
  //   alignedUniformBytes,
  // };

  // let buildRenderGameObjectData = (vertexBufferOffset, vertexCount) => {
  //   vertexBufferOffset,
  //   vertexCount,
  // };

  // let _getRenderGameObjectDataMap = state => {
  //   _getPassData(state).renderGameObjectDataMap;
  // };

  // let unsafeGetRenderGameObjectData = (renderGameObject, state) => {
  //   _getRenderGameObjectDataMap(state)
  //   |> ImmutableSparseMap.unsafeGet(renderGameObject);
  // };

  // let setRenderGameObjectData =
  //     (renderGameObject, vertexBufferOffset, vertexCount, state) => {
  //   ...state,
  //   pass: {
  //     ...state.pass,
  //     gbufferPassData: {
  //       ..._getPassData(state),
  //       renderGameObjectDataMap:
  //         _getRenderGameObjectDataMap(state)
  //         |> ImmutableSparseMap.set(
  //              renderGameObject,
  //              {vertexBufferOffset, vertexCount},
  //            ),
  //     },
  //   },
  // };

  let unsafeGetIndexCount = (geometry, state) => {
    _getPassData(state).indexCountMap
    |> ImmutableSparseMap.unsafeGet(geometry);
  };

  let setIndexCountMap = (indexCountMap, state) => {
    ...state,
    pass: {
      ...state.pass,
      gbufferPassData: {
        ..._getPassData(state),
        indexCountMap,
      },
    },
  };

  let addStaticBindGroupData = (setSlot, bindGroup, state) => {
    ...state,
    pass: {
      ...state.pass,
      gbufferPassData: {
        ..._getPassData(state),
        staticBindGroupDataArr:
          _getPassData(state).staticBindGroupDataArr
          |> ArrayUtils.push({setSlot, bindGroup}),
      },
    },
  };

  let addDynamicBindGroupData = (setSlot, bindGroup, offsetArrMap, state) => {
    ...state,
    pass: {
      ...state.pass,
      gbufferPassData: {
        ..._getPassData(state),
        dynamicBindGroupDataArr:
          _getPassData(state).dynamicBindGroupDataArr
          |> ArrayUtils.push({setSlot, bindGroup, offsetArrMap}),
      },
    },
  };

  // let setPassData =
  //     (
  //       pipeline,
  //       vertexBuffer,
  //       indexBuffer,
  //       staticBindGroupDataArr,
  //       dynamicBindGroupDataArr,
  //       state,
  //     ) => {
  //   ...state,
  //   pass: {
  //     ...state.pass,
  //     gbufferPassData: {
  //       ..._getPassData(state),
  //       pipeline,
  //       vertexBuffer,
  //       indexBuffer,
  //       staticBindGroupDataArr,
  //       dynamicBindGroupDataArr,
  //     },
  //   },
  // };

  let unsafeGetPipeline = state => {
    _getPassData(state).pipeline |> Js.Option.getExn;
  };

  let setPipeline = (pipeline, state) => {
    ...state,
    pass: {
      ...state.pass,
      gbufferPassData: {
        ..._getPassData(state),
        pipeline: Some(pipeline),
      },
    },
  };

  let unsafeGetVertexAndIndexBuffer = (geometry, state) => {
    _getPassData(state).vertexAndIndexBufferMap
    |> ImmutableSparseMap.unsafeGet(geometry);
  };

  let setVertexAndIndexBufferMap = (vertexAndIndexBufferMap, state) => {
    ...state,
    pass: {
      ...state.pass,
      gbufferPassData: {
        ..._getPassData(state),
        vertexAndIndexBufferMap,
      },
    },
  };

  let getStaticBindGroupDataArr = state => {
    _getPassData(state).staticBindGroupDataArr;
  };

  let getDynamicBindGroupDataArr = state => {
    _getPassData(state).dynamicBindGroupDataArr;
  };

  let getRenderGameObjectArr = state => {
    _getPassData(state).renderGameObjectArr;
  };

  let setRenderGameObjectArr = (renderGameObjectArr, state) => {
    {
      ...state,
      pass: {
        ...state.pass,
        gbufferPassData: {
          ..._getPassData(state),
          renderGameObjectArr,
        },
      },
    };
  };
};

module RayTracingPass = {
  let _getPassData = state => {
    state.pass.rayTracingPassData;
  };

  let unsafeGetPipeline = state => {
    _getPassData(state).pipeline |> Js.Option.getExn;
  };

  let setPipeline = (pipeline, state) => {
    ...state,
    pass: {
      ...state.pass,
      rayTracingPassData: {
        ..._getPassData(state),
        pipeline: Some(pipeline),
      },
    },
  };

  let getStaticBindGroupDataArr = state => {
    _getPassData(state).staticBindGroupDataArr;
  };

  let addStaticBindGroupData = (setSlot, bindGroup, state) => {
    ...state,
    pass: {
      ...state.pass,
      rayTracingPassData: {
        ..._getPassData(state),
        staticBindGroupDataArr:
          _getPassData(state).staticBindGroupDataArr
          |> ArrayUtils.push({setSlot, bindGroup}),
      },
    },
  };
  // let getIndirectLightSpecularSampleCount = state => {
  //   _getPassData(state).indirectLightSpecularSampleCount;
  // };
  // let setIndirectLightSpecularSampleCount =
  //     (indirectLightSpecularSampleCount, state) => {
  //   ...state,
  //   pass: {
  //     ...state.pass,
  //     rayTracingPassData: {
  //       ..._getPassData(state),
  //       indirectLightSpecularSampleCount,
  //     },
  //   },
  // };
};

module PreprocessPass = {
  let _getPassData = state => {
    state.pass.preprocessPassData;
  };

  let unsafeGetPipeline = state => {
    _getPassData(state).pipeline |> Js.Option.getExn;
  };

  let setPipeline = (pipeline, state) => {
    ...state,
    pass: {
      ...state.pass,
      preprocessPassData: {
        ..._getPassData(state),
        pipeline: Some(pipeline),
      },
    },
  };

  let addStaticBindGroupData = (setSlot, bindGroup, state) => {
    ...state,
    pass: {
      ...state.pass,
      preprocessPassData: {
        ..._getPassData(state),
        staticBindGroupDataArr:
          _getPassData(state).staticBindGroupDataArr
          |> ArrayUtils.push({setSlot, bindGroup}),
      },
    },
  };

  let getStaticBindGroupDataArr = state => {
    _getPassData(state).staticBindGroupDataArr;
  };
};

module RegressionPass = {
  let _getPassData = state => {
    state.pass.regressionPassData;
  };

  let unsafeGetPipeline = state => {
    _getPassData(state).pipeline |> Js.Option.getExn;
  };

  let setPipeline = (pipeline, state) => {
    ...state,
    pass: {
      ...state.pass,
      regressionPassData: {
        ..._getPassData(state),
        pipeline: Some(pipeline),
      },
    },
  };

  let addStaticBindGroupData = (setSlot, bindGroup, state) => {
    ...state,
    pass: {
      ...state.pass,
      regressionPassData: {
        ..._getPassData(state),
        staticBindGroupDataArr:
          _getPassData(state).staticBindGroupDataArr
          |> ArrayUtils.push({setSlot, bindGroup}),
      },
    },
  };

  let getStaticBindGroupDataArr = state => {
    _getPassData(state).staticBindGroupDataArr;
  };
};

module PostprocessPass = {
  let _getPassData = state => {
    state.pass.postprocessPassData;
  };

  let unsafeGetPipeline = state => {
    _getPassData(state).pipeline |> Js.Option.getExn;
  };

  let setPipeline = (pipeline, state) => {
    ...state,
    pass: {
      ...state.pass,
      postprocessPassData: {
        ..._getPassData(state),
        pipeline: Some(pipeline),
      },
    },
  };

  let addStaticBindGroupData = (setSlot, bindGroup, state) => {
    ...state,
    pass: {
      ...state.pass,
      postprocessPassData: {
        ..._getPassData(state),
        staticBindGroupDataArr:
          _getPassData(state).staticBindGroupDataArr
          |> ArrayUtils.push({setSlot, bindGroup}),
      },
    },
  };

  let getStaticBindGroupDataArr = state => {
    _getPassData(state).staticBindGroupDataArr;
  };
};

module TAAPass = {
  let _getPassData = state => {
    state.pass.taaPassData;
  };

  let isFirstFrame = state => {
    _getPassData(state).isFirstFrame;
  };

  let markFirstFrame = state => {
    ...state,
    pass: {
      ...state.pass,
      taaPassData: {
        ..._getPassData(state),
        isFirstFrame: true,
      },
    },
  };

  let markNotFirstFrame = state => {
    ...state,
    pass: {
      ...state.pass,
      taaPassData: {
        ..._getPassData(state),
        isFirstFrame: false,
      },
    },
  };

  let unsafeGetFirstFramePipeline = state => {
    _getPassData(state).firstFramePipeline |> Js.Option.getExn;
  };

  let setFirstFramePipeline = (firstFramePipeline, state) => {
    ...state,
    pass: {
      ...state.pass,
      taaPassData: {
        ..._getPassData(state),
        firstFramePipeline: Some(firstFramePipeline),
      },
    },
  };

  let unsafeGetOtherFramePipeline = state => {
    _getPassData(state).otherFramePipeline |> Js.Option.getExn;
  };

  let setOtherFramePipeline = (otherFramePipeline, state) => {
    ...state,
    pass: {
      ...state.pass,
      taaPassData: {
        ..._getPassData(state),
        otherFramePipeline: Some(otherFramePipeline),
      },
    },
  };

  let addFirstFrameStaticBindGroupData = (setSlot, bindGroup, state) => {
    ...state,
    pass: {
      ...state.pass,
      taaPassData: {
        ..._getPassData(state),
        firstFrameStaticBindGroupDataArr:
          _getPassData(state).firstFrameStaticBindGroupDataArr
          |> ArrayUtils.push({setSlot, bindGroup}),
      },
    },
  };

  let getFirstFrameStaticBindGroupDataArr = state => {
    _getPassData(state).firstFrameStaticBindGroupDataArr;
  };

  let addOtherFrameStaticBindGroupData = (setSlot, bindGroup, state) => {
    ...state,
    pass: {
      ...state.pass,
      taaPassData: {
        ..._getPassData(state),
        otherFrameStaticBindGroupDataArr:
          _getPassData(state).otherFrameStaticBindGroupDataArr
          |> ArrayUtils.push({setSlot, bindGroup}),
      },
    },
  };

  let getOtherFrameStaticBindGroupDataArr = state => {
    _getPassData(state).otherFrameStaticBindGroupDataArr;
  };
};

module AccumulationPass = {
  let _getPassData = state => {
    state.pass.accumulationPassData;
  };

  let _getLastViewMatrix = state => {
    _getPassData(state).lastViewMatrix;
  };

  let setLastViewMatrix = (lastViewMatrix, state) => {
    ...state,
    pass: {
      ...state.pass,
      accumulationPassData: {
        ..._getPassData(state),
        lastViewMatrix: Some(lastViewMatrix),
      },
    },
  };

  let isCameraMove = (currentViewMatrix, state) => {
    switch (_getLastViewMatrix(state)) {
    | None => true
    | Some(lastViewMatrix) =>
      // Log.print((lastViewMatrix, currentViewMatrix)) |> ignore;
      lastViewMatrix != currentViewMatrix;
    };
  };

  let getAccumFrameCountForDenoise = state => {
    _getPassData(state).accumFrameCountForDenoise;
  };

  let setAccumFrameCountForDenoise = (accumFrameCountForDenoise, state) => {
    ...state,
    pass: {
      ...state.pass,
      accumulationPassData: {
        ..._getPassData(state),
        accumFrameCountForDenoise,
      },
    },
  };

  let getAccumFrameCount = state => {
    _getPassData(state).accumFrameCount;
  };

  let _setAccumFrameCount = (accumFrameCount, state) => {
    ...state,
    pass: {
      ...state.pass,
      accumulationPassData: {
        ..._getPassData(state),
        accumFrameCount,
      },
    },
  };

  let increaseAccumFrameCount = state => {
    let accumFrameCount = getAccumFrameCount(state) |> succ;

    (accumFrameCount, _setAccumFrameCount(accumFrameCount, state));
  };

  let resetAccumFrameCount = state => {
    let accumFrameCount = 1;

    (accumFrameCount, _setAccumFrameCount(accumFrameCount, state));
  };

  let canDenoise = state => {
    getAccumFrameCount(state) >= getAccumFrameCountForDenoise(state);
  };


  let convertCanDenoiseToFloat = state => {
canDenoise(state) ? 1.0 : 0.0
  };

  let unsafeGetPipeline = state => {
    _getPassData(state).pipeline |> Js.Option.getExn;
  };

  let setPipeline = (pipeline, state) => {
    ...state,
    pass: {
      ...state.pass,
      accumulationPassData: {
        ..._getPassData(state),
        pipeline: Some(pipeline),
      },
    },
  };

  let unsafeGetStaticBindGroupData = state => {
    _getPassData(state).staticBindGroupData |> Js.Option.getExn;
  };

  let setStaticBindGroupData = (setSlot, bindGroup, state) => {
    ...state,
    pass: {
      ...state.pass,
      accumulationPassData: {
        ..._getPassData(state),
        staticBindGroupData: Some({setSlot, bindGroup}),
      },
    },
  };
};

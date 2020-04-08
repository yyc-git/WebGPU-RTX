open StateType;

let unsafeGetUniformBufferData = (bufferName, state) => {
  state.pass.uniformBufferDataMap |> ImmutableHashMap.unsafeGet(bufferName);
};

let setUniformBufferData = (bufferName, (buffer, bufferData), state) => {
  ...state,
  pass: {
    ...state.pass,
    uniformBufferDataMap:
      state.pass.uniformBufferDataMap
      |> ImmutableHashMap.set(bufferName, (buffer, bufferData)),
  },
};

// let unsafeGetStorageBuffer = (bufferName, state) => {
//   state.pass.storageBufferMap |> ImmutableHashMap.unsafeGet(bufferName);
// };

// let setStorageBuffer = (bufferName, buffer, state) => {
//   ...state,
//   pass: {
//     ...state.pass,
//     storageBufferMap:
//       state.pass.storageBufferMap |> ImmutableHashMap.set(bufferName, buffer),
//   },
// };

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

  let unsafeGetVertexCount = (renderGameObject, state) => {
    _getPassData(state).vertexCountMap
    |> ImmutableSparseMap.unsafeGet(renderGameObject);
  };

  let setVertexCount = (renderGameObject, vertexCount, state) => {
    ...state,
    pass: {
      ...state.pass,
      gbufferPassData: {
        ..._getPassData(state),
        vertexCountMap:
          _getPassData(state).vertexCountMap
          |> ImmutableSparseMap.set(renderGameObject, vertexCount),
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

  let unsafeGetVertexAndIndexBuffer = (geometry, state ) => {
    _getPassData(state).vertexAndIndexBufferMap |> ImmutableSparseMap.unsafeGet(geometry)
  };

  let setVertexAndIndexBufferMap = ( vertexAndIndexBufferMap, state) => {
    ...state,
    pass: {
      ...state.pass,
      gbufferPassData: {
        ..._getPassData(state),
        vertexAndIndexBufferMap
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

module BlitPass = {
  let _getPassData = state => {
    state.pass.blitPassData;
  };

  let unsafeGetPipeline = state => {
    _getPassData(state).pipeline |> Js.Option.getExn;
  };

  let setPipeline = (pipeline, state) => {
    ...state,
    pass: {
      ...state.pass,
      blitPassData: {
        ..._getPassData(state),
        pipeline: Some(pipeline),
      },
    },
  };

  let unsafeGetBindGroup = state => {
    _getPassData(state).bindGroup |> Js.Option.getExn;
  };

  let setBindGroup = (bindGroup, state) => {
    ...state,
    pass: {
      ...state.pass,
      blitPassData: {
        ..._getPassData(state),
        bindGroup: Some(bindGroup),
      },
    },
  };
};

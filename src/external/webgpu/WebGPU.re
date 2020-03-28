type textureFormat = string;

module TextureView = {
  type t;
};

module SwapChain = {
  type t;

  [@bs.send.pipe: t] external getCurrentTextureView: unit => TextureView.t;
  [@bs.send.pipe: t] external present: unit;
};

module CommandBuffer = {
  type t;
};

module Queue = {
  type t;

  [@bs.send.pipe: t] external submit: array(CommandBuffer.t) => unit;
};

module ShaderStage = {
  type t;

  [@bs.scope "GPUShaderStage"] [@bs.module "webgpu"]
  external getRayGeneration: t = "RAY_GENERATION";
  [@bs.scope "GPUShaderStage"] [@bs.module "webgpu"]
  external getRayCloestHit: t = "RAY_CLOSEST_HIT";
  [@bs.scope "GPUShaderStage"] [@bs.module "webgpu"]
  external getRayAnyHit: t = "RAY_ANY_HIT";
  [@bs.scope "GPUShaderStage"] [@bs.module "webgpu"]
  external getRayMiss: t = "RAY_MISS";
  [@bs.scope "GPUShaderStage"] [@bs.module "webgpu"]
  external getRayIntersection: t = "RAY_INTERSECTION";
  [@bs.scope "GPUShaderStage"] [@bs.module "webgpu"]
  external getCompute: t = "COMPUTE";
  [@bs.scope "GPUShaderStage"] [@bs.module "webgpu"]
  external getFragment: t = "FRAGMENT";
  [@bs.scope "GPUShaderStage"] [@bs.module "webgpu"]
  external getVertex: t = "VERTEX";
};

module BufferUsage = {
  type t;

  [@bs.scope "GPUBufferUsage"] [@bs.module "webgpu"]
  external getStorage: t = "STORAGE";
  [@bs.scope "GPUBufferUsage"] [@bs.module "webgpu"]
  external getUniform: t = "UNIFORM";
  [@bs.scope "GPUBufferUsage"] [@bs.module "webgpu"]
  external getIndirect: t = "INDIRECT";
  [@bs.scope "GPUBufferUsage"] [@bs.module "webgpu"]
  external getVertex: t = "VERTEX";
  [@bs.scope "GPUBufferUsage"] [@bs.module "webgpu"]
  external getIndex: t = "INDEX";
  [@bs.scope "GPUBufferUsage"] [@bs.module "webgpu"]
  external getMapRead: t = "MAP_WRITE";
  [@bs.scope "GPUBufferUsage"] [@bs.module "webgpu"]
  external getMapWrite: t = "MAP_WRITE";
  [@bs.scope "GPUBufferUsage"] [@bs.module "webgpu"]
  external getCopySrc: t = "COPY_SRC";
  [@bs.scope "GPUBufferUsage"] [@bs.module "webgpu"]
  external getCopyDst: t = "COPY_DST";
};

module Buffer = {
  type t;

  type descriptor = {
    .
    "size": int,
    "usage": BufferUsage.t,
  };

  [@bs.send.pipe: t]
  external setSubFloat32Data: (int, Js.Typed_array.Float32Array.t) => unit =
    "setSubData";
  [@bs.send.pipe: t]
  external setSubUint8Data: (int, Js.Typed_array.Uint8Array.t) => unit =
    "setSubData";

  module VertexBuffer = {
    type t;
  };
};

module BindGroup = {
  type t;

  type bindingPoint = int;

  type layoutBinding = {
    .
    "binding": bindingPoint,
    "visibility": ShaderStage.t,
    "type": string,
  };
  type layout = {. "binds": array(layoutBinding)};

  type binding = {
    .
    "binding": bindingPoint,
    "buffer": Buffer.t,
    "offset": int,
    "size": int,
  };

  type descriptor = {
    .
    "layout": layout,
    "bindings": array(binding),
  };
};

module ShaderModule = {
  type t;

  type descriptor = {. "code": string};
};

module ShaderBindingTable = {
  type t;

  type stage = {
    .
    "module": ShaderModule.t,
    "stage": ShaderStage.t,
  };

  type group = {
    .
    "type": string,
    "generatIndex": int,
    "anyHitIndex": int,
    "closestHitIndex": int,
    "intersectionIndex": int,
  };

  type descriptor = {
    .
    "stages": array(stage),
    "groups": array(group),
  };
};

module Pipeline = {
  type t;

  type layout;

  type layoutConfig = {. "bindGroupLayouts": array(BindGroup.layout)};

  module Render = {
    type t;

    [@bs.deriving abstract]
    type vertexStage = {
      [@bs.as "module"]
      module_: ShaderModule.t,
      entryPoint: string,
    };

    [@bs.deriving abstract]
    type fragmentStage = {
      [@bs.as "module"]
      module_: ShaderModule.t,
      entryPoint: string,
    };

    [@bs.deriving abstract]
    type vertexState = {
      indexFormat: string,
      [@bs.optional]
      vertexBuffers: array(Buffer.VertexBuffer.t),
    };

    [@bs.deriving abstract]
    type rasterizationState = {
      [@bs.optional]
      frontFace: string,
      [@bs.optional]
      cullMode: string,
    };

    [@bs.deriving abstract]
    type blendDescriptor = {
      [@bs.optional]
      srcFactor: string,
      [@bs.optional]
      dstFactor: string,
      [@bs.optional]
      operation: string,
    };

    [@bs.deriving abstract]
    type colorState = {
      format: textureFormat,
      alphaBlend: blendDescriptor,
      colorBlend: blendDescriptor,
    };

    [@bs.deriving abstract]
    type descriptor = {
      layout,
      [@bs.optional]
      sampleCount: int,
      vertexStage,
      fragmentStage,
      primitiveTopology: string,
      vertexState,
      rasterizationState,
      colorStates: array(colorState),
    };
  };

  module RayTracing = {
    type t;

    [@bs.deriving abstract]
    type rayTracingState = {
      shaderBindingTable: ShaderBindingTable.t,
      maxRecursionDepth: int,
    };

    [@bs.deriving abstract]
    type descriptor = {
      layout,
      rayTracingState,
    };
  };
};

module PassEncoder = {
  module Render = {
    type t;

    type clearColor = {
      .
      "r": float,
      "g": float,
      "b": float,
      "a": float,
    };

    type colorAttachment = {
      .
      "clearColor": clearColor,
      "loadOp": string,
      "storeOp": string,
      "attachment": TextureView.t,
    };

    type depthStencilAttachment = {
      .
      "clearColor": clearColor,
      "clearDepth": float,
      "depthLoadOp": string,
      "depthStoreOp": string,
      "clearStencil": int,
      "stencilLoadOp": string,
      "stencilStoreOp": string,
      "attachment": TextureView.t,
    };

    [@bs.deriving abstract]
    type descriptor = {
      colorAttachments: array(colorAttachment),
      [@bs.optional]
      depthStencilAttachments: array(depthStencilAttachment),
    };

    type vertexCount = int;
    type instanceCount = int;
    type firstVertex = int;
    type firstInstance = int;

    [@bs.send.pipe: t] external setPipeline: Pipeline.Render.t => unit;
    [@bs.send.pipe: t]
    external setBindGroup: (BindGroup.bindingPoint, BindGroup.t) => unit;
    [@bs.send.pipe: t]
    external draw:
      (vertexCount, instanceCount, firstVertex, firstInstance) => unit;
    [@bs.send.pipe: t] external endPass: unit;
  };

  module RayTracing = {
    type t;

    [@bs.deriving abstract]
    type descriptor = {
      [@bs.optional]
      label: string,
    };

    type sbtRayGenerationOffset = int;
    type sbtRayHitOffset = int;
    type sbtRayMissOffset = int;
    type queryWidthDimension = int;
    type queryHeightDimension = int;
    type queryDepthDimension = int;

    [@bs.send.pipe: t] external setPipeline: Pipeline.RayTracing.t => unit;
    [@bs.send.pipe: t]
    external setBindGroup: (BindGroup.bindingPoint, BindGroup.t) => unit;
    [@bs.send.pipe: t]
    external traceRays:
      (
        sbtRayGenerationOffset,
        sbtRayHitOffset,
        sbtRayMissOffset,
        queryWidthDimension,
        queryHeightDimension,
        queryDepthDimension
      ) =>
      unit;
    [@bs.send.pipe: t] external endPass: unit;
  };
};

module CommandEncoder = {
  type t;

  [@bs.deriving abstract]
  type descriptor = {
    [@bs.optional]
    label: string,
  };

  [@bs.send.pipe: t]
  external beginRenderPass:
    PassEncoder.Render.descriptor => PassEncoder.Render.t;
  [@bs.send.pipe: t]
  external beginRayTracingPass:
    PassEncoder.RayTracing.descriptor => PassEncoder.RayTracing.t;
  [@bs.send.pipe: t] external finish: CommandBuffer.t;
};

module Device = {
  type t;

  [@bs.send.pipe: t] external getQueue: Queue.t;
  [@bs.send.pipe: t]
  external createShaderModule: ShaderModule.descriptor => ShaderModule.t;
  [@bs.send.pipe: t]
  external createPipelineLayout: Pipeline.layoutConfig => Pipeline.layout;
  [@bs.send.pipe: t] external createBuffer: Buffer.descriptor => Buffer.t;
  [@bs.send.pipe: t]
  external createBindGroup: BindGroup.descriptor => BindGroup.t;
  [@bs.send.pipe: t]
  external createRenderPipeline:
    Pipeline.Render.descriptor => Pipeline.Render.t;
  [@bs.send.pipe: t]
  external createRayTracingPipeline:
    Pipeline.RayTracing.descriptor => Pipeline.RayTracing.t;
  [@bs.send.pipe: t]
  external createRayTracingShaderBindingTable:
    ShaderBindingTable.descriptor => ShaderBindingTable.t;
  [@bs.send.pipe: t]
  external createCommandEncoder: CommandEncoder.descriptor => CommandEncoder.t;
};

module Context = {
  type t;

  type swapChainConfig = {
    .
    "device": Device.t,
    "format": textureFormat,
  };

  [@bs.send.pipe: t]
  external getSwapChainPreferredFormat:
    Device.t => Js.Promise.t(textureFormat);
  [@bs.send.pipe: t]
  external configureSwapChain: swapChainConfig => SwapChain.t;
};

module Window = {
  type t;

  type descriptor = {
    .
    "width": int,
    "height": int,
    "title": string,
  };

  [@bs.module "webgpu"] [@bs.new]
  external make: descriptor => t = "WebGPUWindow";
  [@bs.send.pipe: t] external getContext: ([@bs.as "webgpu"] _) => Context.t;
  [@bs.send.pipe: t] external pollEvents: unit => unit;
  [@bs.send.pipe: t] external shouldClose: bool;

  [@bs.get] external getWidth: t => int = "width";
  [@bs.get] external getHeight: t => int = "height";
};

module Adapter = {
  type t;

  [@bs.send.pipe: t] external requestDevice: Js.Promise.t(Device.t);
};

module GPU = {
  type adapterConfig = {. "window": Window.t};

  [@bs.scope "GPU"] [@bs.module "webgpu"]
  external requestAdapter: adapterConfig => Js.Promise.t(Adapter.t);
};

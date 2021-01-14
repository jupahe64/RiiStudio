#pragma once

#include <librii/gx.h>

namespace librii::gl {

// Converts to GL
u32 translateCullMode(gx::CullMode cullMode);
u32 translateBlendFactorCommon(gx::BlendModeFactor factor);
u32 translateBlendSrcFactor(gx::BlendModeFactor factor);
u32 translateBlendDstFactor(gx::BlendModeFactor factor);
u32 translateCompareType(gx::Comparison compareType);

u32 gxFilterToGl(gx::TextureFilter filter);
u32 gxTileToGl(gx::TextureWrapMode wrap);

} // namespace librii::gl
#pragma once

#include <vector>
#include <string>

// Forward decls.
class TextureLoader;
class Renderable;
#include "graphics/ForwardDecl.h"

extern bool GetRenderables(const std::string& dataPrefix, TextureLoader& textureLoader, Graphics::RenderingSystem& renderingSystem, std::vector<Renderable>& outRenderables);
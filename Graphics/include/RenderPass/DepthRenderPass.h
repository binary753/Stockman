#pragma once
#include "RenderPass.h"
#include "../Utility/Shader.h"

namespace Graphics
{
    class DepthRenderPass : public RenderPass
    {
    public:
        enum BufferSlots { Camera };
        enum ResourceSlots { StaticInstanceBuffer, AnimatedInstanceBuffer };

        DepthRenderPass(
            std::initializer_list<ID3D11RenderTargetView*> targets,
            std::initializer_list<ID3D11ShaderResourceView*> resources = {},
            std::initializer_list<ID3D11Buffer*> buffers = {},
            ID3D11DepthStencilView * depthStencil = nullptr
        );
  

        // Inherited via RenderPass
        virtual void render() const override;
        virtual void update(float deltaTime) override;

    private:
        Shader depth_vs_static;
        Shader depth_vs_animated;

    };
}
#include "LightCullRenderPass.h"

Graphics::LightCullRenderPass::LightCullRenderPass(
    std::initializer_list<ID3D11RenderTargetView*> targets,
    std::initializer_list<ID3D11ShaderResourceView*> resources,
    std::initializer_list<ID3D11Buffer*> buffers,
    ID3D11DepthStencilView * depthStencil,
    std::initializer_list<ID3D11UnorderedAccessView*> uavs
)
    : RenderPass(targets, resources, buffers, depthStencil)
    , uavs(uavs)
{
    lightGrid.initialize();
}

void Graphics::LightCullRenderPass::render() const
{
    lightGrid.cull(buffers[0], resources[0], resources[1], uavs[1], uavs[0]);
}

void Graphics::LightCullRenderPass::update(float deltaTime)
{
}
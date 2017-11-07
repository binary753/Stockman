#include "ForwardPlusRenderPass.h"
#include "../Device.h"
#include "../CommonState.h"
#include "../RenderQueue.h"
#include "../HybrisLoader/HybrisLoader.h"

namespace Graphics
{
    ForwardPlusRenderPass::ForwardPlusRenderPass(std::initializer_list<ID3D11RenderTargetView*> targets, std::initializer_list<ID3D11ShaderResourceView*> resources, std::initializer_list<ID3D11Buffer*> buffers, ID3D11DepthStencilView * depthStencil)
        : RenderPass(targets, resources, buffers, depthStencil)
        , staticForwardPlus(Resources::Shaders::ForwardPlus)
    {
    }

    void ForwardPlusRenderPass::render() const
    {
        Global::context->IASetInputLayout(nullptr);
        Global::context->VSSetShader(staticForwardPlus, nullptr, 0);
        Global::context->PSSetShader(staticForwardPlus, nullptr, 0);

        ID3D11SamplerState * samplers[] =
        {
            Global::cStates->LinearClamp(),
            Global::comparisonSampler,
            Global::cStates->LinearWrap(),
        };
        Global::context->PSSetSamplers(0, 3, samplers);

        Global::context->OMSetRenderTargets(targets.size(), targets.data(), depthStencil);
        Global::context->PSSetShaderResources(0, 4, resources.data());
        Global::context->PSSetConstantBuffers(0, buffers.size(), buffers.data());
        Global::context->VSSetConstantBuffers(0, buffers.size(), buffers.data());

        drawInstanced<StaticRenderInfo>(resources[4]);
        // TODO add all renderInfos
    }

    void ForwardPlusRenderPass::update(float deltaTime)
    {
    }

    
}
#include "stdafx.h"
#include "Game.h"
#include "Mesh3dLodLevel.h"

using namespace glm;

Mesh3dLodLevel::Mesh3dLodLevel(Object3dInfo *info, Material *imaterial, float distancestart, float distanceend)
{
    info3d = info;
    material = imaterial;
    distanceStart = distancestart;
    distanceEnd = distanceend;
    instancesFiltered = 0;
    modelInfosBuffer = new ShaderStorageBuffer();
}

Mesh3dLodLevel::Mesh3dLodLevel(Object3dInfo *info, Material *imaterial)
{
    info3d = info;
    material = imaterial;
    distanceStart = 0;
    distanceEnd = 99999.0;
    instancesFiltered = 0;
    modelInfosBuffer = new ShaderStorageBuffer();
}

Mesh3dLodLevel::Mesh3dLodLevel()
{
    info3d = nullptr;
    material = nullptr;
    distanceStart = 0;
    distanceEnd = 99999.0;
    instancesFiltered = 0;
    modelInfosBuffer = new ShaderStorageBuffer();
}


Mesh3dLodLevel::~Mesh3dLodLevel()
{
}

void Mesh3dLodLevel::draw()
{
    ShaderProgram *shader = ShaderProgram::current;
    // base properties
    shader->setUniform("Roughness", material->roughness);
    shader->setUniform("Metalness", material->metalness);
    shader->setUniform("DiffuseColor", material->diffuseColor);

    vector<int> samplerIndices;
    vector<int> modes;
    vector<int> targets;
    vector<vec2> uvScales;
    int samplerIndex = 0;
    for (int i = 0; i < material->nodes.size(); i++) {
        MaterialNode * node = material->nodes[i];
        if (node->texture == nullptr) continue;
        node->texture->use(samplerIndex);
        samplerIndices.push_back(samplerIndex);
        modes.push_back(node->mixingMode);
        targets.push_back(node->target);
        uvScales.push_back(node->uvScale);
        samplerIndex++;
    }
    shader->setUniform("NodesCount", samplerIndex);
    shader->setUniformVector("SamplerIndexArray", samplerIndices);
    shader->setUniformVector("ModeArray", modes);
    shader->setUniformVector("TargetArray", targets);
    shader->setUniformVector("UVScaleArray", uvScales);

    modelInfosBuffer->use(0);

    info3d->drawInstanced(instancesFiltered);
}

void Mesh3dLodLevel::updateBuffer(const vector<Mesh3dInstance*> &instances)
{
    vec3 cameraPos = Game::instance->world->mainDisplayCamera->transformation->position;
    vector<Mesh3dInstance*> filtered;
    for (int i = 0; i < instances.size(); i++) {
        float dst = distance(cameraPos, instances[i]->transformation->position);
        if (dst >= distanceStart && dst < distanceEnd) {
            filtered.push_back(instances[i]);
        }
    }
    instancesFiltered = filtered.size();
    /*layout rotation f4 translation f3+1 scale f3+1 =>> 12 floats*/
    vector<float> floats;
    floats.reserve(12 * instancesFiltered);
    for (unsigned int i = 0; i < instancesFiltered; i++) {
        TransformationManager *mgr = filtered[i]->transformation;
        floats.push_back(mgr->orientation.x);
        floats.push_back(mgr->orientation.y);
        floats.push_back(mgr->orientation.z);
        floats.push_back(mgr->orientation.w);

        floats.push_back(mgr->position.x);
        floats.push_back(mgr->position.y);
        floats.push_back(mgr->position.z);
        floats.push_back(1);

        floats.push_back(mgr->size.x);
        floats.push_back(mgr->size.y);
        floats.push_back(mgr->size.z);
        floats.push_back(1);
    }
    modelInfosBuffer->mapData(4 * floats.size(), floats.data());

}
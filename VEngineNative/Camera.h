#pragma once
#include "AbsTransformable.h"
#include "FrustumCone.h"
class Camera : public AbsTransformable
{
public:
    Camera();
    ~Camera();

    float brightness;
    float farplane;
    FrustumCone *cone;
    glm::mat4 projectionMatrix;

    void createProjectionPerspective(float fov, float aspectRatio, float nearpl, float farpl);

};

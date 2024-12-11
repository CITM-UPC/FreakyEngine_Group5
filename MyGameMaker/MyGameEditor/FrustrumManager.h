#pragma once

#include "MyGameEngine/GameObject.h"


class FrustrumManager
{
public:
    bool isInsideFrustum(const BoundingBox& bbox, const std::list<Plane>& frustumPlanes);

};


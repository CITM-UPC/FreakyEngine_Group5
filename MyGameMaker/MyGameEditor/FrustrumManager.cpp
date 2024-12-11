#include "FrustrumManager.h"



bool FrustrumManager::isInsideFrustum(const BoundingBox& bbox, const std::list<Plane>& frustumPlanes) {
    for (const auto& plane : frustumPlanes) {
        if (plane.distance(bbox.v000()) < 0 &&
            plane.distance(bbox.v001()) < 0 &&
            plane.distance(bbox.v010()) < 0 &&
            plane.distance(bbox.v011()) < 0 &&
            plane.distance(bbox.v100()) < 0 &&
            plane.distance(bbox.v101()) < 0 &&
            plane.distance(bbox.v110()) < 0 &&
            plane.distance(bbox.v111()) < 0) {
            return false;
        }
    }
    return true;
}

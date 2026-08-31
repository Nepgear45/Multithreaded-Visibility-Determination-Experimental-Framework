#pragma once

#include "Frustum.h"
#include "Scene/SceneObject.h"

bool IsObjectVisible(const SceneObject& object, const Frustum& frustum);
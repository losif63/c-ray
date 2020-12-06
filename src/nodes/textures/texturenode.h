//
//  texturenode.h
//  C-Ray
//
//  Created by Valtteri Koskivuori on 30/11/2020.
//  Copyright © 2020 Valtteri Koskivuori. All rights reserved.
//

#pragma once

enum textureType {
	Diffuse,
	Normal,
	Specular
};

struct textureNode {
	struct color (*eval)(const struct textureNode *node, const struct hitRecord *record);
};

#include "checker.h"
#include "constant.h"
#include "image.h"

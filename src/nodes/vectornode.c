//
//  vectornode.c
//  C-Ray
//
//  Created by Valtteri Koskivuori on 16/12/2020.
//  Copyright © 2020 Valtteri Koskivuori. All rights reserved.
//

#include "../includes.h"
#include "../datatypes/color.h"
#include "../datatypes/vector.h"
#include "../datatypes/hitrecord.h"
#include "../datatypes/scene.h"
#include "../datatypes/vector.h"
#include "../utils/hashtable.h"
#include "bsdfnode.h"

#include "vectornode.h"

struct constantVector {
	struct vectorNode node;
	struct vector vector;
};

static bool compare(const void *A, const void *B) {
	const struct constantVector *this = A;
	const struct constantVector *other = B;
	return vecEquals(this->vector, other->vector);
}

static uint32_t hash(const void *p) {
	const struct constantVector *this = p;
	uint32_t h = hashInit();
	h = hashBytes(h, &this->vector, sizeof(this->vector));
	return h;
}

static struct vector eval(const struct vectorNode *node, const struct hitRecord *record) {
	(void)record;
	struct constantVector *this = (struct constantVector *)node;
	return this->vector;
}

struct vectorNode *newConstantVector(const struct world *world, struct vector vector) {
	HASH_CONS(world->nodeTable, world->nodePool, hash, struct constantVector, {
		.vector = vector,
		.node = {
			.eval = eval,
			.base = { .compare = compare }
		}
	});
}
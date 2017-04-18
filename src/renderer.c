//
//  renderer.c
//  C-Ray
//
//  Created by Valtteri Koskivuori on 19/02/2017.
//  Copyright © 2017 Valtteri Koskivuori. All rights reserved.
//

#include "renderer.h"

/*
* Global renderer
*/
renderer mainRenderer;

#ifdef WINDOWS
	HANDLE tileMutex = INVALID_HANDLE_VALUE;
#else
	pthread_mutex_t tileMutex;
#endif

#pragma mark Helper funcs

bool renderTilesEmpty() {
	return mainRenderer.renderedTileCount >= mainRenderer.tileCount;
}

/**
 Gets the next tile from renderTiles in mainRenderer

 @return A renderTile to be rendered
 */
renderTile getTile() {
#ifdef WINDOWS
	WaitForSingleObject(tileMutex, INFINITE);
#else
	pthread_mutex_lock(&tileMutex);
#endif
	renderTile tile = mainRenderer.renderTiles[mainRenderer.renderedTileCount++];
	mainRenderer.renderTiles[mainRenderer.renderedTileCount - 1].isRendering = true;
	tile.tileNum = mainRenderer.renderedTileCount - 1;
	
#ifdef WINDOWS
	ReleaseMutex(tileMutex);
#else
	pthread_mutex_unlock(&tileMutex);
#endif
	return tile;
}

/**
 Create tiles from render plane, and add those to mainRenderer

 @param worldScene worldScene object
 */
void quantizeImage(world *worldScene) {
#ifdef WINDOWS
	//Create this here for now
	tileMutex = CreateMutex(NULL, FALSE, NULL);
#endif
	printf("Quantizing render plane...\n");
	int tilesX = worldScene->camera->width / worldScene->camera->tileWidth;
	int tilesY = worldScene->camera->height / worldScene->camera->tileHeight;
	
	float tilesXf = (float)worldScene->camera->width / (float)worldScene->camera->tileWidth;
	float tilesYf = (float)worldScene->camera->height / (float)worldScene->camera->tileHeight;
	
	if (tilesXf - (int)tilesXf != 0) {
		tilesX++;
	}
	if (tilesYf - (int)tilesYf != 0) {
		tilesY++;
	}
	
	mainRenderer.renderTiles = (renderTile*)calloc(tilesX*tilesY, sizeof(renderTile));
	
	for (int y = 0; y < tilesY; y++) {
		for (int x = 0; x < tilesX; x++) {
			renderTile *tile = &mainRenderer.renderTiles[x + y*tilesX];
			tile->width  = worldScene->camera->tileWidth;
			tile->height = worldScene->camera->tileHeight;
			
			tile->startX = x       * worldScene->camera->tileWidth;
			tile->endX   = (x + 1) * worldScene->camera->tileWidth;
			
			tile->startY = y       * worldScene->camera->tileHeight;
			tile->endY   = (y + 1) * worldScene->camera->tileHeight;
			
			tile->endX = min((x + 1) * worldScene->camera->tileWidth, worldScene->camera->width);
			tile->endY = min((y + 1) * worldScene->camera->tileHeight, worldScene->camera->height);
			
			tile->completedSamples = 1;
			tile->isRendering = false;
			
			mainRenderer.tileCount++;
		}
	}
	printf("Quantized image into %i tiles. (%ix%i)", (tilesX*tilesY), tilesX, tilesY);
}

void reorderTopToBottom() {
	int endIndex = mainRenderer.tileCount - 1;
	
	renderTile *tempArray = (renderTile*)calloc(mainRenderer.tileCount, sizeof(renderTile));
	
	for (int i = 0; i < mainRenderer.tileCount; i++) {
		tempArray[i] = mainRenderer.renderTiles[endIndex--];
	}
	
	free(mainRenderer.renderTiles);
	mainRenderer.renderTiles = tempArray;
}

void reorderFromMiddle() {
	int midLeft = 0;
	int midRight = 0;
	bool isRight = true;
	
	midRight = ceil(mainRenderer.tileCount / 2);
	midLeft = midRight - 1;
	
	renderTile *tempArray = (renderTile*)calloc(mainRenderer.tileCount, sizeof(renderTile));
	
	for (int i = 0; i < mainRenderer.tileCount; i++) {
		if (isRight) {
			tempArray[i] = mainRenderer.renderTiles[midRight++];
			isRight = false;
		} else {
			tempArray[i] = mainRenderer.renderTiles[midLeft--];
			isRight = true;
		}
	}
	
	free(mainRenderer.renderTiles);
	mainRenderer.renderTiles = tempArray;
}

/**
 Reorder renderTiles in given order

 @param order Render order to be applied
 */
void reorderTiles(renderOrder order) {
	switch (order) {
		case renderOrderFromMiddle:
			{
				reorderFromMiddle();
			}
			break;
		case renderOrderTopToBottom:
			{
				reorderTopToBottom();
			}
			break;
		default:
			break;
	}
}

/**
 Gets a pixel from the render buffer

 @param worldScene WorldScene to get image dimensions
 @param x X coordinate of pixel
 @param y Y coordinate of pixel
 @return A color object, with full color precision intact (double)
 */
color getPixel(world *worldScene, int x, int y) {
	color output = {0.0f, 0.0f, 0.0f, 0.0f};
	output.red =   mainRenderer.renderBuffer[(x + (worldScene->camera->height - y)*worldScene->camera->width)*3 + 0];
	output.green = mainRenderer.renderBuffer[(x + (worldScene->camera->height - y)*worldScene->camera->width)*3 + 1];
	output.blue =  mainRenderer.renderBuffer[(x + (worldScene->camera->height - y)*worldScene->camera->width)*3 + 2];
	output.alpha = 1.0f;
	return output;
}

/**
 Returns a random float between min and max

 @param min Minimum value
 @param max Maximum value
 @return Random float between min and max
 */
float getRandomFloat(float min, float max) {
	return ((((float)rand()) / (float)RAND_MAX) * (max - min)) + min;
}


/**
 Returns a randomized position in a radius around a given point

 @param center Center point for random distribution
 @param radius Maximum distance from center point
 @return Vector of a random position within a radius of center point
 */
vector getRandomVecOnRadius(vector center, float radius) {
	return vectorWithPos(center.x + getRandomFloat(-radius, radius),
						 center.y + getRandomFloat(-radius, radius),
						 center.z + getRandomFloat(-radius, radius));
}

/**
 Returns a randomized position on a plane in a radius around a given point

 @param center Center point for random distribution
 @param radius Maximum distance from center point
 @return Vector of a random position on a plane within a radius of center point
 */
vector getRandomVecOnPlane(vector center, float radius) {
	//FIXME: This only works in one orientation!
	return vectorWithPos(center.x + getRandomFloat(-radius, radius),
						 center.y + getRandomFloat(-radius, radius),
						 center.z);
}

#pragma mark Renderer

/**
 Returns a computed color based on a given ray and world scene

 @param incidentRay View ray to be cast into a scene
 @param worldScene Scene the ray is cast into
 @return Color value with full precision (double)
 */
color rayTrace(lightRay *incidentRay, world *worldScene) {
	//Raytrace a given light ray with a given scene, then return the color value for that ray
	color output = {0.0f,0.0f,0.0f};
	int bounces = 0;
	double contrast = worldScene->camera->contrast;
	
	do {
		//Find the closest intersection first
		double closestIntersection = 20000.0f;
		double temp;
		int currentSphere = -1;
		int currentPolygon = -1;
		int sphereAmount = worldScene->sphereAmount;
		int polygonAmount = fullPolyCount;
		int lightSourceAmount = worldScene->lightAmount;
		int objCount = worldScene->objCount;
		
		material currentMaterial;
		vector polyNormal = {0.0, 0.0, 0.0};
		vector hitpoint, surfaceNormal;
		
		unsigned int i;
		for (i = 0; i < sphereAmount; ++i) {
			if (rayIntersectsWithSphere(incidentRay, &worldScene->spheres[i], &closestIntersection)) {
				currentSphere = i;
			}
		}
		
		unsigned int o, p;
		for (o = 0; o < objCount; o++) {
			if (rayIntersectsWithSphereFast(incidentRay, &worldScene->objs[o].boundingVolume)) {
				for (p = worldScene->objs[o].firstPolyIndex; p < (worldScene->objs[o].firstPolyIndex + worldScene->objs[o].polyCount); p++) {
					if (rayIntersectsWithPolygon(incidentRay, &polygonArray[p], &closestIntersection, &polyNormal)) {
						currentPolygon = p;
						currentSphere = -1;
					}
				}
			}
		}
		
		for (i = (fullPolyCount - worldScene->polygonAmount); i < polygonAmount; ++i) {
			if (rayIntersectsWithPolygon(incidentRay, &polygonArray[i], &closestIntersection, &polyNormal)) {
				currentPolygon = i;
				currentSphere = -1;
			}
		}
		
		//Ray-object intersection detection
		if (currentSphere != -1) {
			vector scaled = vectorScale(closestIntersection, &incidentRay->direction);
			hitpoint = addVectors(&incidentRay->start, &scaled);
			surfaceNormal = subtractVectors(&hitpoint, &worldScene->spheres[currentSphere].pos);
			temp = scalarProduct(&surfaceNormal,&surfaceNormal);
			if (temp == 0.0f) break;
			temp = invsqrtf(temp);
			surfaceNormal = vectorScale(temp, &surfaceNormal);
			currentMaterial = worldScene->materials[worldScene->spheres[currentSphere].material];
		} else if (currentPolygon != -1) {
			vector scaled = vectorScale(closestIntersection, &incidentRay->direction);
			hitpoint = addVectors(&incidentRay->start, &scaled);
			//We get polyNormal from the intersection function
			surfaceNormal = polyNormal;
			temp = scalarProduct(&surfaceNormal,&surfaceNormal);
			if (temp == 0.0f) break;
			temp = invsqrtf(temp);
			surfaceNormal = vectorScale(temp, &surfaceNormal);
			currentMaterial = worldScene->materials[polygonArray[currentPolygon].materialIndex];
		} else {
			//Ray didn't hit any object, set color to ambient
			color temp = colorCoef(contrast, worldScene->ambientColor);
			output = addColors(&output, &temp);
			break;
		}
		
		if (scalarProduct(&surfaceNormal, &incidentRay->direction) < 0.0f) {
			surfaceNormal = vectorScale(1.0f, &surfaceNormal);
		} else if (scalarProduct(&surfaceNormal, &incidentRay->direction) > 0.0f) {
			surfaceNormal = vectorScale(-1.0f, &surfaceNormal);
		}
		
		lightRay bouncedRay, cameraRay;
		bouncedRay.start = hitpoint;
		cameraRay.start = hitpoint;
		cameraRay.direction = subtractVectors(&worldScene->camera->pos, &hitpoint);
		double cameraProjection = scalarProduct(&cameraRay.direction, &hitpoint);
		double cameraDistance = scalarProduct(&cameraRay.direction, &cameraRay.direction);
		double camTemp = cameraDistance;
		camTemp = invsqrtf(camTemp);
		cameraRay.direction = vectorScale(camTemp, &cameraRay.direction);
		cameraProjection = camTemp * cameraProjection;
		//Find the value of the light at this point
		unsigned int j;
		for (j = 0; j < lightSourceAmount; ++j) {
			light currentLight = worldScene->lights[j];
			vector lightPos;
			if (worldScene->camera->areaLights)
				lightPos = getRandomVecOnRadius(currentLight.pos, currentLight.radius);
			else
				lightPos = currentLight.pos;
			
			bouncedRay.direction = subtractVectors(&lightPos, &hitpoint);
			
			double lightProjection = scalarProduct(&bouncedRay.direction, &surfaceNormal);
			if (lightProjection <= 0.0f) continue;
			
			double lightDistance = scalarProduct(&bouncedRay.direction, &bouncedRay.direction);
			double temp = lightDistance;
			
			if (temp == 0.0f) continue;
			temp = invsqrtf(temp);
			bouncedRay.direction = vectorScale(temp, &bouncedRay.direction);
			lightProjection = temp * lightProjection;
			
			//Calculate shadows
			bool inShadow = false;
			double t = lightDistance;
			unsigned int k;
			for (k = 0; k < sphereAmount; ++k) {
				if (rayIntersectsWithSphere(&bouncedRay, &worldScene->spheres[k], &t)) {
					inShadow = true;
					break;
				}
			}
			
			double fakeIntersection = 20000.0f;
			for (o = 0; o < objCount; o++) {
				if (rayIntersectsWithSphere(&bouncedRay, &worldScene->objs[o].boundingVolume, &fakeIntersection)) {
					
					if (worldScene->camera->approximateMeshShadows) {
						inShadow = true;
						break;
					} else {
						for (p = worldScene->objs[o].firstPolyIndex; p < (worldScene->objs[o].firstPolyIndex + worldScene->objs[o].polyCount); p++) {
							if (rayIntersectsWithPolygon(&bouncedRay, &polygonArray[p], &t, &polyNormal)) {
								inShadow = true;
								break;
							}
						}
					}
				}
			}
			
			for (i = (fullPolyCount - worldScene->polygonAmount); i < polygonAmount; ++i) {
				if (rayIntersectsWithPolygon(&bouncedRay, &polygonArray[i], &t, &polyNormal)) {
					inShadow = true;
					break;
				}
			}
			
			if (!inShadow) {
				//TODO: Calculate specular reflection
				float specularFactor = 1.0;//scalarProduct(&cameraRay.direction, &surfaceNormal) * contrast;
				
				//Calculate Lambert diffusion
				float diffuseFactor = scalarProduct(&bouncedRay.direction, &surfaceNormal) * contrast;
				output.red += specularFactor * diffuseFactor * currentLight.intensity.red * currentMaterial.diffuse.red;
				output.green += specularFactor * diffuseFactor * currentLight.intensity.green * currentMaterial.diffuse.green;
				output.blue += specularFactor * diffuseFactor * currentLight.intensity.blue * currentMaterial.diffuse.blue;
			}
		}
		//Iterate over the reflection
		contrast *= currentMaterial.reflectivity;
		
		//Calculate reflected ray start and direction
		double reflect = 2.0f * scalarProduct(&incidentRay->direction, &surfaceNormal);
		incidentRay->start = hitpoint;
		vector tempVec = vectorScale(reflect, &surfaceNormal);
		incidentRay->direction = subtractVectors(&incidentRay->direction, &tempVec);
		
		bounces++;
		
	} while ((contrast > 0.0f) && (bounces <= worldScene->camera->bounces));
	
	return output;
}

/**
 A render thread

 @param arg Thread information (see threadInfo struct)
 @return Exits when thread is done
 */
#ifdef WINDOWS
DWORD WINAPI renderThread(LPVOID arg) {
#else
void *renderThread(void *arg) {
#endif
	lightRay incidentRay;
	int x,y;
	
	threadInfo *tinfo = (threadInfo*)arg;

	renderTile tile;
	tile.tileNum = 0;
	while (!renderTilesEmpty()) {
		x = 0; y = 0;
		mainRenderer.renderTiles[tile.tileNum].isRendering = false;
		tile = getTile();
		printf("Started tile %i/%i\r", mainRenderer.renderedTileCount, mainRenderer.tileCount);
		while (tile.completedSamples < mainRenderer.worldScene->camera->sampleCount+1 && mainRenderer.isRendering) {
			for (y = tile.endY; y > tile.startY; y--) {
				for (x = tile.startX; x < tile.endX; x++) {
					color output = getPixel(mainRenderer.worldScene, x, y);
					color sample = {0.0f,0.0f,0.0f,0.0f};
					if (mainRenderer.worldScene->camera->viewPerspective.projectionType == ortho) {
						incidentRay.start.x = x/2;
						incidentRay.start.y = y/2;
						incidentRay.start.z = mainRenderer.worldScene->camera->pos.z;
						
						incidentRay.direction.x = 0;
						incidentRay.direction.y = 0;
						incidentRay.direction.z = 1;
						incidentRay.rayType = rayTypeIncident;
						sample = rayTrace(&incidentRay, mainRenderer.worldScene);
					} else if (mainRenderer.worldScene->camera->viewPerspective.projectionType == conic) {
						double focalLength = 0.0f;
						if ((mainRenderer.worldScene->camera->viewPerspective.projectionType == conic)
							&& mainRenderer.worldScene->camera->viewPerspective.FOV > 0.0f
							&& mainRenderer.worldScene->camera->viewPerspective.FOV < 189.0f) {
							focalLength = 0.5f * mainRenderer.worldScene->camera->width / tanf((double)(PIOVER180) * 0.5f * mainRenderer.worldScene->camera->viewPerspective.FOV);
						}
						
						vector direction = {(x - 0.5f * mainRenderer.worldScene->camera->width) / focalLength,
							(y - 0.5f * mainRenderer.worldScene->camera->height) / focalLength, 1.0f};
						
						double normal = scalarProduct(&direction, &direction);
						if (normal == 0.0f)
							break;
						direction = vectorScale(invsqrtf(normal), &direction);
						vector startPos = getRandomVecOnPlane(mainRenderer.worldScene->camera->pos, mainRenderer.worldScene->camera->focalLength);
						
						incidentRay.start = startPos;
						incidentRay.direction = direction;
						incidentRay.rayType = rayTypeIncident;
						sample = rayTrace(&incidentRay, mainRenderer.worldScene);
						
						output.red = output.red * (tile.completedSamples - 1);
						output.green = output.green * (tile.completedSamples - 1);
						output.blue = output.blue * (tile.completedSamples - 1);
						
						output = addColors(&output, &sample);
						
						output.red = output.red / tile.completedSamples;
						output.green = output.green / tile.completedSamples;
						output.blue = output.blue / tile.completedSamples;
						
						//Store render buffer
						mainRenderer.renderBuffer[(x + (mainRenderer.worldScene->camera->height - y)*mainRenderer.worldScene->camera->width)*3 + 0] = output.red;
						mainRenderer.renderBuffer[(x + (mainRenderer.worldScene->camera->height - y)*mainRenderer.worldScene->camera->width)*3 + 1] = output.green;
						mainRenderer.renderBuffer[(x + (mainRenderer.worldScene->camera->height - y)*mainRenderer.worldScene->camera->width)*3 + 2] = output.blue;
					}
					
					mainRenderer.worldScene->camera->imgData[(x + (mainRenderer.worldScene->camera->height - y)*mainRenderer.worldScene->camera->width)*3 + 0] = (unsigned char)min(  output.red*255.0f, 255.0f);
					mainRenderer.worldScene->camera->imgData[(x + (mainRenderer.worldScene->camera->height - y)*mainRenderer.worldScene->camera->width)*3 + 1] = (unsigned char)min(output.green*255.0f, 255.0f);
					mainRenderer.worldScene->camera->imgData[(x + (mainRenderer.worldScene->camera->height - y)*mainRenderer.worldScene->camera->width)*3 + 2] = (unsigned char)min( output.blue*255.0f, 255.0f);
				}
			}
			tile.completedSamples++;
		}

	}
	mainRenderer.renderTiles[tile.tileNum].isRendering = false;
	printf("Thread %i done\n", tinfo->thread_num);
	tinfo->threadComplete = true;
#ifdef WINDOWS
	//Return possible codes here
	return 0;
#else
	pthread_exit((void*) arg);
#endif
}
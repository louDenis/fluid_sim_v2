#pragma once
#include "fluid.h"
#include "fluid_utils.h"


struct sHeights
{
	float hCanva;
	float hDryLayer;
	float hWetLayer;
	float hNewLayer;

};
class Layers
{
public:
	int size; 
	float totalHeight;
	Fluid* dryLayer;
	Fluid* wetLayer;
	Fluid* newLayer;
	sHeights heights;
	Layers(float dt, float diffusion, float viscosity);
	void dry();
	void dry_partially(float percent);
	float computeHeight(int i, int j);
private:
	
	void mergeDensities();
};
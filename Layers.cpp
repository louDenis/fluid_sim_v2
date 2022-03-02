#include "Layers.h"
#include "fluid.h"
#include "fluid_utils.h"

#include <iostream>
#include <string>
#include <sstream>
#include <fstream>


int IX2(int x, int y)
{
	if (x > N - 1)
		x = N - 1;
	if (x < 0)
		x = 0;
	if (y > N - 1)
		y = N - 1;
	if (y < 0)
		y = 0;
	return x + y * N;
}

float Layers::computeHeight(int i, int j)
{
	return this->dryLayer->getDensity(i, j) + this->wetLayer->getDensity(i, j) + this->newLayer->getDensity(i, j);
}

void Layers::dry_partially(float percent)
{
	float* array_cpy = wetLayer->getDensityArray();
	
	for (int i = 0; i < N; ++i)
	{
		for (int j = 0; j < N; ++j)
		{
			if (i == 10 && j == 10)
			{
				std::cout << "i,j= " << i << ", "  << j << "wet value " << this->wetLayer->getDensity(i, j);
			}
		
			//keep only 1-percent densities in wetLayer
			float new_wet = this->wetLayer->getDensity(i,j) * (1 - percent); // array_cpy[IX2(i, j)]
				
			this->wetLayer->set_density(i,j, array_cpy[IX2(i,j)] * (1 - percent));

			if (i == 30 && j == 30)
			{
				std::cout << "new wet value is " << new_wet; //this->wetLayer->getDensity(i, j);
				std::cout << "dry value is " << this->dryLayer->getDensity(i, j);
			}
			
			//add percentage density to dry layer and freeze velocities
			//this->dryLayer->array_init(dryLayer->getVxArray(), this->size);
			//this->dryLayer->array_init(dryLayer->getVyArray(), this->size);
			dryLayer->array_init(this->wetLayer->getDensityArray(), this->size);
			if (i == 30 && j == 30)
			{
				std::cout << "new dry value is " << this->dryLayer->getDensity(i, j);
			}
			
		}
	}
		
	
	

	//add it to dry layer
}

void Layers::dry()
{
	mergeDensities();
	//mettre les vélocités à 0
	this->dryLayer->array_init(dryLayer->getVxArray(), this->size);
	this->newLayer->array_init(dryLayer->getVxArray(), this->size);
	this->wetLayer->array_init(dryLayer->getVxArray(), this->size);

	this->dryLayer->array_init(dryLayer->getVyArray(), this->size);
	this->newLayer->array_init(dryLayer->getVyArray(), this->size);
	this->wetLayer->array_init(dryLayer->getVyArray(), this->size);

}

Layers::Layers(float dt, float diffusion, float viscosity)
{
	this->size = N;
	this->dryLayer = new Fluid(dt, diffusion, viscosity);
	this->wetLayer = new Fluid(dt, diffusion, viscosity);
	this->newLayer = new Fluid(dt, diffusion, viscosity);
}

void Layers::mergeDensities()
{
	//additionner les densités de wet et dry et les stocker dans dry
	for (int i = 0; i < this->size; ++i)
	{
		for (int j = 0; j < this->size; j++)
		{
			this->dryLayer->addDensity(i, j, this->wetLayer->getDensity(i, j));
		}
	}
	//remettre à O les densités de wetLayer
	this->wetLayer->array_init(this->wetLayer->getDensityArray(), this->size);
}
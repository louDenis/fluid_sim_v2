#pragma once

#ifndef FLUID_H
#define FLUID_H

constexpr auto N = 32;
constexpr auto iter = 4;
constexpr auto SCALE = 8;
//ATTENTION CA DOIT ETRE DES PUISSANCES DE 2 SINON CA FAIT NIMP


class Fluid
{
private:
    int size;
    float dt;
    float diff;
    float visc;

    float* s;
    float* density;

    float* Vx;
    float* Vy;
   
    float* Vx0;
    float* Vy0;

public:
    Fluid(float dt, float diffusion, float viscosity);
    float* array_init(float* array, int size);

    void set_density(int i, int j, float value);
    void step(float dta);
    int get_size();
    void addDensity(int x, int y, float amount);
    void addVelocity(int x, int y, float amountX, float amountY);
    void FluidCubeFree();
    void renderD();
    float getDensity(int x, int y);
    void dry();
    float* getDensityArray();
    float* getVxArray();
    float* getVyArray();

    //int getDensity(int mouseX, int mouseY);
    
};

#endif

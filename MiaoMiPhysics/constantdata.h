#ifndef CONSTANT_DATA_H_
#define CONSTANT_DATA_H_

#define MAX_PARTICLES 16384*8
#define MAX_GRIDS pow((float)floor((2.0f*BOUNDARY+0.0002f)/SMOOTH_LENGTH+1), 3)
#define MAX_NEIGHBOURS_PER_PARTICLE 300
#define MAX_PARTICLES_PER_GRID 300
#define M_PI    3.141592653589793f

#define INIT_DISTANCE pow(1.35e-6f, 1.0f/3.0f)
#define MAX_COORD make_float4(BOUNDARY, BOUNDARY, BOUNDARY, BOUNDARY)
#define MIN_COORD make_float4(-BOUNDARY, -BOUNDARY, -BOUNDARY, -BOUNDARY)
#define SMOOTH_LENGTH 1.9e-2f
#define PARTICLE_RADIUS pow(1.35e-6f, 1.0f/3.0f)*2.3f

#define GRAVITY 9.8f
#define PARTICLE_MASS 1.35e-3f
#define BOUNDARY_RESISTANCE -0.5f
#define REST_DENISITY 1.0e3f
#define MOTION_DAMPING 5.0f
#define TIME_STEP 1.0e-3f
#define PRESSURE_CONSTANT 1.0f
#define BOUNDARY 0.4999f
#define INIT_CUBE 0.75f

#define NUM 16384*3
#endif
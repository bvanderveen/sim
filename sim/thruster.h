#include "math.h"

#ifndef __SIM_THRUSTER
#define __SIM_THRUSTER

struct SimThruster {
  SimUnit amount;
  SimQuat direction;
};
typedef struct SimThruster *SimThrusterRef;

SimThrusterRef SimThrusterCreate();
void SimThrusterDestroy(SimThrusterRef ref);

#define SimThrusterGetThrust(T) Sim3VectorScale((T)->amount, SimQuatRotate((T)->direction, Sim3VectorMake(0,1,0)))

#endif
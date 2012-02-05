#include "body.h"
#include "thruster.h"

#ifndef __SIM_VEHICLE
#define __SIM_VEHICLE

struct SimVehicle {
  SimRigidBodyRef body;
  SimThrusterRef thruster;
};
typedef struct SimVehicle *SimVehicleRef;

SimVehicleRef SimVehicleCreate();
void SimVehicleDestroy(SimVehicleRef ref);
void SimVehicleDraw(SimVehicleRef ref, SimUnit t);

#endif
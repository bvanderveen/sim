#include "math.h"
#include "vehicle.h"

struct SimContext {
  SimUnit t;
  int numVehicles;
  SimVehicleRef *vehicles;
  SimUnit *state0, *state1;
};
typedef struct SimContext *SimContextRef;

SimContextRef SimContextCreate();
void SimContextDestroy(SimContextRef c);

void SimContextUpdate(SimContextRef c, double dt);
void SimContextDraw(SimContextRef c, double dt);

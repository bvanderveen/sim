#include "thruster.h"
#include <stdlib.h>

SimThrusterRef SimThrusterCreate() {
  SimThrusterRef result = (SimThrusterRef)malloc(sizeof(struct SimThruster));
  result->amount = SimUnitZero;
  result->direction = SimQuatZero;
  return result;
}

void SimThrusterDestroy(SimThrusterRef ref) {
  free(ref);
}
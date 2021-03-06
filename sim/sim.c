#include "sim.h"
#include <stdio.h>
#include <stdlib.h>
#include <OpenGL/gl.h>
#include <string.h>

#include "math.h"
#include "rigidbody.h"
#include "vehicle.h"

// bunch new shit from http://www.cs.cmu.edu/~baraff/sigcourse/notesd1.pdf

int SimContextGetStateVectorSize(SimContextRef ref) {
  return ref->numVehicles * SimRigidBodyGetStateVectorSize();
}

SimContextRef SimContextCreate() {
  SimContextRef result = (SimContextRef)malloc(sizeof(struct SimContext));
  printf("created context (%p)\n", result);

  result->t = 0;
  result->numVehicles = 1;
  result->vehicles = malloc(sizeof(SimVehicleRef) * result->numVehicles);

  result->state0 = (SimUnit *)malloc(sizeof(SimUnit) * SimContextGetStateVectorSize(result));
  result->state1 = (SimUnit *)malloc(sizeof(SimUnit) * SimContextGetStateVectorSize(result));

  for (int i = 0; i < result->numVehicles; i++) {
    SimVehicleRef vehicle = SimVehicleCreate();
    printf("created vehicle (%p)\n", vehicle);
    int offset = i * SimRigidBodyGetStateVectorSize();
    SimRigidBodyCopyToBuffer(vehicle->body, &result->state1[offset]);

    result->vehicles[i] = vehicle;
  }
  
  return result;
}

void SimContextDestroy(SimContextRef ref) {
  free(ref->state0);
  free(ref->state1);

  for (int i = 0; i < ref->numVehicles; i++)
    SimVehicleDestroy(ref->vehicles[i]);

  free(ref->vehicles);
  free(ref);
}

void SimContextUpdate(SimContextRef ref, double dt) {
  memcpy(ref->state0, ref->state1, sizeof(SimUnit) * SimContextGetStateVectorSize(ref));
  ODESolver(ref->state0, SimContextGetStateVectorSize(ref), ref->t, ref->t + dt, RigidBodySolve, ref->state1, ref);
  ref->t += dt;
}

void SimContextDraw(SimContextRef ref, double dt) {
  printf("drawing context %p\n", ref);

  glClearColor(0,0,0,0);
  glClear(GL_COLOR_BUFFER_BIT);

  glPushMatrix();
  glTranslatef(0,-1,0);
  glScalef(.1,.1,.1);

  for (int i = 0; i < ref->numVehicles; i++) {
    SimVehicleRef vehicle = ref->vehicles[i];
    printf("will draw vehicle %d (%p)\n", i, vehicle);
    SimVehicleDraw(vehicle, dt);
    printf("did draw vehicle %d\n", i);
  }

  glPopMatrix();  
  glFlush();
}

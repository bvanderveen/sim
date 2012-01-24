#include "sim.h"
#include <stdio.h>
#include <stdlib.h>
#include <OpenGL/gl.h>

typedef double SimUnit;
#define SimUnitZero 0

struct Sim3Vector {
  SimUnit x, y, z;
};
typedef struct Sim3Vector Sim3Vector;

#define Sim3VectorZero ((Sim3Vector){ .x = SimUnitZero, .y = SimUnitZero, .z = SimUnitZero })
#define Sim3VectorMake(X, Y, Z) ((Sim3Vector){ .x = (X), .y = (Y), .z = (Z) })
#define Sim3VectorAdd(A, B) ((Sim3Vector){ .x = (A).x + (B).x, .y = (A).y + (B).y, .z = (A).z + (B).z })
#define Sim3VectorScale(K, V) ((Sim3Vector){ .x = (K) * (V).x, .y = (K) * (V).y, .z = (K) * (V).z })

struct SimThruster {
  Sim3Vector thrust;
};
typedef struct SimThruster *SimThrusterRef;

SimThrusterRef SimThrusterCreate() {
  SimThrusterRef result = (SimThrusterRef)malloc(sizeof(struct SimThruster));
  result->thrust = Sim3VectorZero;
  return result;
}

void SimThrusterDestroy(SimThrusterRef ref) {
  free(ref);
}

struct SimVehicle {
  Sim3Vector position;
  Sim3Vector velocity;

  Sim3Vector angle;
  Sim3Vector angularVelocity;

  SimUnit mass;

  SimThrusterRef thruster;
};
typedef struct SimVehicle *SimVehicleRef;

SimVehicleRef SimVehicleCreate() {
  SimVehicleRef result = (SimVehicleRef)malloc(sizeof(struct SimVehicle));

  result->position = Sim3VectorZero;
  result->velocity = Sim3VectorZero;

  result->angle = Sim3VectorZero;
  result->angularVelocity = Sim3VectorZero;

  result->mass = SimUnitZero;
  result->thruster = SimThrusterCreate();
  return result;
}

void SimVehicleDestroy(SimVehicleRef ref) {
  SimThrusterDestroy(ref->thruster);
  free(ref);
}

void SimVehicleUpdate(SimVehicleRef ref, SimUnit t) {
  ref->position = Sim3VectorAdd(ref->position, Sim3VectorScale(1/t, ref->velocity));
  ref->velocity = Sim3VectorAdd(ref->velocity, Sim3VectorScale(1/t, ref->thruster->thrust));
}

void SimVehicleDraw(SimVehicleRef ref, SimUnit t) {
  glPushMatrix();
  
  glTranslated(ref->position.x, ref->position.y, ref->position.z);

  glColor3f(1.0f, 0.85f, 0.35f);
  glBegin(GL_TRIANGLES);
  {
    glVertex3f(  0.0,  0.5, 0.0);
    glVertex3f( -0.2, -0.5, 0.0);
    glVertex3f(  0.2, -0.5 ,0.0);
  }
  glEnd(); 

  glPopMatrix();
}

struct SimContext {
  SimVehicleRef vehicle;
};
typedef struct SimContext *SimContext;

SimContextRef SimContextCreate() {
  SimContext result = (SimContext)malloc(sizeof(struct SimContext));
  result->vehicle = SimVehicleCreate();
  result->vehicle->thruster->thrust = Sim3VectorMake(0,0.00001,0);
  return result;
}

void SimContextDestroy(SimContextRef ref) {
  SimContext self = ref;
  SimVehicleDestroy(self->vehicle);
  free(ref);
}

void SimContextUpdate(SimContextRef c, double t) {
  SimContext ctx = c;

  SimVehicleUpdate(ctx->vehicle, t);
}

void SimContextDraw(SimContextRef c, double t) {
  SimContext ctx = c;

  glClearColor(0,0,0,0);
  glClear(GL_COLOR_BUFFER_BIT);

  glPushMatrix();

  glTranslatef(0,-1,0);
  glScalef(.1,.1,.1);

  SimVehicleDraw(ctx->vehicle, t);

  glPopMatrix();
  
  glFlush();
}

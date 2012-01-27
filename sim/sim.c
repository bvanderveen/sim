#include "sim.h"
#include <stdio.h>
#include <stdlib.h>
#include <OpenGL/gl.h>

typedef float SimUnit;
#define SimUnitZero 0

struct Sim3Vector {
  SimUnit x, y, z;
};
typedef struct Sim3Vector Sim3Vector;

#define Sim3VectorPrint(V) printf("x = %f, y = %f, z = %f\n", (V).x, (V).y, (V).z)
#define Sim3VectorZero ((Sim3Vector){ .x = SimUnitZero, .y = SimUnitZero, .z = SimUnitZero })
#define Sim3VectorMake(X, Y, Z) ((Sim3Vector){ .x = (X), .y = (Y), .z = (Z) })
#define Sim3VectorAdd(A, B) ((Sim3Vector){ .x = (A).x + (B).x, .y = (A).y + (B).y, .z = (A).z + (B).z })
#define Sim3VectorScale(K, V) ((Sim3Vector){ .x = (K) * (V).x, .y = (K) * (V).y, .z = (K) * (V).z })

struct SimQuat {
  SimUnit w, x, y, z;
};
typedef struct SimQuat SimQuat;

#define SimQuatPrint(V) printf("w = %f, x = %f, y = %f, z = %f\n", (V).w, (V).x, (V).y, (V).z)
#define SimQuatMake(W, X, Y, Z) ((SimQuat){ .w = (W), .x = (X), .y = (Y), .z = (Z) })
#define SimQuatZero SimQuatMake(0,0,0,0)
#define SimQuatConjugate(Q) ((SimQuat){ .w = (Q).w, .x = -(Q).x, .y = -(Q).y, .z = -(Q).z })
#define SimQuatMult(L, R) ((SimQuat){ \
  .w = (L).w * (R).w - (L).x * (R).x - (L).y * (R).y - (L).z * (R).z, \
  .x = (L).w * (R).x + (L).x * (R).w + (L).y * (R).z - (L).z * (R).y, \
  .y = (L).w * (R).y + (L).y * (R).w + (L).z * (R).x - (L).x * (R).z, \
  .z = (L).w * (R).z + (L).z * (R).w + (L).x * (R).y - (L).y * (R).x })

Sim3Vector SimQuatRotate(SimQuat q, Sim3Vector v) {
  printf("input was\n");
  Sim3VectorPrint(v);
  SimQuat left = SimQuatMult(q, SimQuatMake(0, v.x, v.y, v.z));
  SimQuat result = SimQuatMult(left, SimQuatConjugate(q));
  Sim3Vector outV = Sim3VectorMake(result.x, result.y, result.z);
  printf("output was");
  SimQuatPrint(result);
  return outV;
}

struct SimThruster {
  SimUnit amount;
  SimQuat direction;
};
typedef struct SimThruster *SimThrusterRef;

SimThrusterRef SimThrusterCreate() {
  SimThrusterRef result = (SimThrusterRef)malloc(sizeof(struct SimThruster));
  result->amount = SimUnitZero;
  result->direction = SimQuatZero;
  return result;
}

#define SimThrusterGetThrust(T) SimQuatRotate((T)->direction, Sim3VectorMake((T)->amount,0,0))

void SimThrusterDestroy(SimThrusterRef ref) {
  free(ref);
}

struct SimVehicle {
  Sim3Vector position;
  Sim3Vector velocity;

  SimUnit mass;

  SimThrusterRef thruster;
};
typedef struct SimVehicle *SimVehicleRef;

SimVehicleRef SimVehicleCreate() {
  SimVehicleRef result = (SimVehicleRef)malloc(sizeof(struct SimVehicle));

  result->position = Sim3VectorZero;
  result->velocity = Sim3VectorZero;

  result->mass = 1;
  result->thruster = SimThrusterCreate();
  result->thruster->amount = .05;
  result->thruster->direction = SimQuatMake(1,1,1,1);
  return result;
}

void SimVehicleDestroy(SimVehicleRef ref) {
  SimThrusterDestroy(ref->thruster);
  free(ref);
}

void SimVehicleUpdate(SimVehicleRef ref, SimUnit t) {
  ref->position = Sim3VectorAdd(ref->position, Sim3VectorScale(t, ref->velocity));

  // XXX need to rotate thrust vector.
  Sim3Vector thrust = SimThrusterGetThrust(ref->thruster);
  ref->velocity = Sim3VectorAdd(ref->velocity, Sim3VectorScale(1/ref->mass, thrust));
}

void SimVehicleDraw(SimVehicleRef ref, SimUnit t) {
  glPushMatrix();
  
  glTranslatef(ref->position.x, ref->position.y, ref->position.z);

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

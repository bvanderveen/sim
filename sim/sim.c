#include "sim.h"
#include <stdio.h>
#include <stdlib.h>
#include <OpenGL/gl.h>
#include <string.h>
#include "math.h"
// bunch new shit from http://www.cs.cmu.edu/~baraff/sigcourse/notesd1.pdf

typedef void (*ODEFunc)(SimUnit t, SimUnit y[], /* out */ SimUnit yDot[], void *context);

void ODESolver(SimUnit y0[], int len, SimUnit t0, SimUnit t1, ODEFunc dydt, /* out */ SimUnit y1[], void *context) {
  // XXX assuming len is invariate, leak memory. ha ha
  printf("allocating some shit\n");
  static SimUnit *result = 0; 
  if (!result) 
    result = (SimUnit *)malloc(sizeof(SimUnit) * len);

  printf("calling dydt\n");
  //dydt(0, (SimUnit *)0, (SimUnit *)0, context);
  dydt(t1 - t0, y0, result, context);
  printf("called dydt\n");

  printf("will compute new values\n");
  for (int i = 0; i < len; i++) {
    y1[i] = y0[i] + result[i];
  }
  printf("did compute new values\n");
}


struct SimRigidBody {
  SimUnit mass;
  SimMatrix Ibody, IbodyInv;

  Sim3Vector x, P, L;
  SimQuat q;

  SimMatrix Iinv;
  Sim3Vector v, omega;

  Sim3Vector force, torque;
};
typedef struct SimRigidBody *SimRigidBodyRef;

struct SimThruster {
  SimUnit amount;
  SimQuat direction;
};
typedef struct SimThruster *SimThrusterRef;

struct SimVehicle {
  SimRigidBodyRef body;
  SimThrusterRef thruster;
};
typedef struct SimVehicle *SimVehicleRef;

struct SimContext {
  SimUnit t;
  int numVehicles;
  SimVehicleRef *vehicles;
  SimUnit *state0, *state1;
};
typedef struct SimContext *SimContext;

SimRigidBodyRef SimRigidBodyCreate() {
  SimRigidBodyRef result = (SimRigidBodyRef)malloc(sizeof(struct SimRigidBody));
  result->mass = 1;
  result->Ibody = SimMatrixMakeBlockTensor(1,1,1);
  result->IbodyInv = SimMatrixInvert(result->Ibody);

  result->x = Sim3VectorZero;
  result->q = SimQuatMakeWithAngle(0, Sim3VectorMake(1,0,0));
  result->P = Sim3VectorZero;
  result->L = Sim3VectorZero;

  return result;
}

void SimRigidBodyDestroy(SimRigidBodyRef ref) {
  free(ref);
}

int SimRigidBodyGetStateVectorSize() {
  return 13;
}

void SimRigidBodyCopyToBuffer(SimRigidBodyRef ref, SimUnit y[]) {
  *y++ = ref->x.x;
  *y++ = ref->x.y;
  *y++ = ref->x.z;

  *y++ = ref->q.w;
  *y++ = ref->q.x;
  *y++ = ref->q.y;
  *y++ = ref->q.z;

  *y++ = ref->P.x;
  *y++ = ref->P.y;
  *y++ = ref->P.z;

  *y++ = ref->L.x;
  *y++ = ref->L.y;
  *y++ = ref->L.z;
}

void SimRigidBodyCopyFromBuffer(SimRigidBodyRef ref, SimUnit y[]) {
  ref->x.x = *y++;
  ref->x.y = *y++;
  ref->x.z = *y++;

  ref->q.w = *y++;
  ref->q.x = *y++;
  ref->q.y = *y++;
  ref->q.z = *y++;

  ref->P.x = *y++;
  ref->P.y = *y++;
  ref->P.z = *y++;

  ref->L.x = *y++;
  ref->L.y = *y++;
  ref->L.z = *y++;

  ref->v = Sim3VectorScale(1/ref->mass, ref->P);

  SimQuat qNorm = SimQuatNormalize(ref->q);
  SimMatrix R = SimMatrixMakeRotationWithQuat(qNorm);
  SimMatrix m = SimMatrixMult(R, ref->IbodyInv);
  SimMatrix RT = SimMatrixTranspose(R);
  ref->Iinv = SimMatrixMult(m, RT);
  ref->omega = SimMatrixMultVector(ref->Iinv, ref->L);
}

void SimRigidBodyUpdateInput(SimRigidBodyRef ref, SimUnit t) {
  // XXX assign ref->force and ref->torque, e.g., from user input
  ref->force = Sim3VectorMake(1,0,0);
  ref->torque = Sim3VectorZero;
}

void SimRigidBodyUpdateState(SimRigidBodyRef ref, SimUnit outputBuffer[]) {
  printf("SimRigidBodyUpdateState\n");
  *outputBuffer++ = ref->v.x;
  *outputBuffer++ = ref->v.y;
  *outputBuffer++ = ref->v.z;

  SimQuat omegaAugmented = SimQuatMakeWithVector(ref->omega);
  SimQuat qdotUnscaled = SimQuatMult(omegaAugmented, ref->q);
  SimQuat qdot = SimQuatScale(.5, qdotUnscaled);

  *outputBuffer++ = qdot.w;
  *outputBuffer++ = qdot.x;
  *outputBuffer++ = qdot.y;
  *outputBuffer++ = qdot.z;

  *outputBuffer++ = ref->force.x;
  *outputBuffer++ = ref->force.y;
  *outputBuffer++ = ref->force.z;

  *outputBuffer++ = ref->torque.x;
  *outputBuffer++ = ref->torque.y;
  *outputBuffer++ = ref->torque.z;
}

void RigidBodySolve(SimUnit t, SimUnit y[], /* out */ SimUnit yDot[], void *context) {
  SimContext ref = (SimContext)context;

  printf("solving context %p\n", ref);

  for (int i = 0; i < ref->numVehicles; i++) {
    
    SimVehicleRef vehicle = ref->vehicles[i];
    printf("Solving for vehicle %d (%p)\n", i, vehicle);
    int offset = i * SimRigidBodyGetStateVectorSize();
    
    printf("will SimRigidBodyCopyFromBuffer\n");
    SimRigidBodyCopyFromBuffer(vehicle->body, &y[offset]);
    printf("did SimRigidBodyCopyFromBuffer\n");
    printf("will SimRigidBodyUpdateInput\n");
    SimRigidBodyUpdateInput(vehicle->body, t);
    printf("did SimRigidBodyUpdateInput\n");
    printf("will SimRigidBodyUpdateState\n");
    SimRigidBodyUpdateState(vehicle->body, &yDot[offset]);
    printf("did SimRigidBodyUpdateState\n"); 
  }
}

SimThrusterRef SimThrusterCreate() {
  SimThrusterRef result = (SimThrusterRef)malloc(sizeof(struct SimThruster));
  result->amount = SimUnitZero;
  result->direction = SimQuatZero;
  return result;
}

#define SimThrusterGetThrust(T) Sim3VectorScale((T)->amount, SimQuatRotate((T)->direction, Sim3VectorMake(0,1,0)))

void SimThrusterDestroy(SimThrusterRef ref) {
  free(ref);
}

SimVehicleRef SimVehicleCreate() {
  SimVehicleRef result = (SimVehicleRef)malloc(sizeof(struct SimVehicle));
  result->body = SimRigidBodyCreate();
  result->thruster = SimThrusterCreate();
  result->thruster->amount = .05;
  result->thruster->direction = SimQuatMakeWithAngle(0, Sim3VectorMake(1,0,0));
  return result;
}

void SimVehicleDestroy(SimVehicleRef ref) {
  SimThrusterDestroy(ref->thruster);
  SimRigidBodyDestroy(ref->body);
  free(ref);
}

void SimVehicleDraw(SimVehicleRef ref, SimUnit t) {

  glPushMatrix();
  
  printf("drawing vehicle (%p)\n", ref);
  printf("body (%p) at:\n", ref->body);

  Sim3VectorPrint(ref->body->x);
  printf("cool.\n");
  glTranslatef(ref->body->x.x, ref->body->x.y, ref->body->x.z);

  glColor3f(1.0f, 0.85f, 0.35f);
  glBegin(GL_TRIANGLES);
  {
    glVertex3f(  0.0,  0.5, 0.0);
    glVertex3f( -0.2, -0.5, 0.0);
    glVertex3f(  0.2, -0.5, 0.0);
  }
  glEnd(); 

  glPopMatrix();
}

int SimContextGetStateVectorSize(SimContext ref) {
  return ref->numVehicles * SimRigidBodyGetStateVectorSize();
}

SimContextRef SimContextCreate() {
  SimContext result = (SimContext)malloc(sizeof(SimContext));
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

    printf("if we pull it out!!! we get vehicle (%p)\n", result->vehicles[i]);
  }
  
  return result;
}

void SimContextDestroy(SimContextRef c) {
  SimContext ref = c;
  free(ref->state0);
  free(ref->state1);

  for (int i = 0; i < ref->numVehicles; i++)
    SimVehicleDestroy(ref->vehicles[i]);

  free(ref->vehicles);
  free(ref);
}

void SimContextUpdate(SimContextRef c, double dt) {
  SimContext ref = c;
  printf("updating context %p\n", ref);

  printf("will shuffle state vector\n");
  memcpy(ref->state1, ref->state0, sizeof(SimUnit) * SimContextGetStateVectorSize(ref));
  printf("did shuffle state vector\n");

  printf("will call solver\n");
  ODESolver(ref->state0, SimContextGetStateVectorSize(ref), ref->t, ref->t + dt, RigidBodySolve, ref->state1, ref);
  printf("did call solver\n");

  ref->t += dt;
  printf("world time is now %f\n", ref->t);
}

void SimContextDraw(SimContextRef c, double dt) {
  SimContext ref = c;
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

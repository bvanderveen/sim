#include "sim.h"
#include <stdio.h>
#include <stdlib.h>
#include <OpenGL/gl.h>
#include <math.h>
#include <string.h>

// bunch new shit from http://www.cs.cmu.edu/~baraff/sigcourse/notesd1.pdf

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

SimQuat SimQuatMakeWithAngle(SimUnit angle, Sim3Vector unitVector) {
  Sim3Vector v = Sim3VectorScale(sin(1.0 / 2.0 * angle), unitVector);
  return ((SimQuat){ .w = cos(1.0 / 2.0 * angle), .x = v.x, .y = v.y, .z = v.z });
}

#define SimQuatMakeWithVector(V) SimQuatMake(0, (V).x, (V).y, (V).z)

#define SimQuatConjugate(Q) ((SimQuat){ .w = (Q).w, .x = -(Q).x, .y = -(Q).y, .z = -(Q).z })
#define SimQuatMult(L, R) ((SimQuat){ \
  .w = (L).w * (R).w - (L).x * (R).x - (L).y * (R).y - (L).z * (R).z, \
  .x = (L).w * (R).x + (L).x * (R).w + (L).y * (R).z - (L).z * (R).y, \
  .y = (L).w * (R).y + (L).y * (R).w + (L).z * (R).x - (L).x * (R).z, \
  .z = (L).w * (R).z + (L).z * (R).w + (L).x * (R).y - (L).y * (R).x })
#define SimQuatScale(K, Q) ((SimQuat){ .w = (K) * (Q).w, .x = (K) * (Q).x, .y = (K) * (Q).y, .z = (K) * (Q).z })
#define SimQuatNormalize(Q) (Q)

Sim3Vector SimQuatRotate(SimQuat q, Sim3Vector v) {
  SimQuat result = SimQuatMult(SimQuatMult(q, SimQuatMake(0, v.x, v.y, v.z)), SimQuatConjugate(q));
  return Sim3VectorMake(result.x, result.y, result.z);
}

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

struct SimMatrix {
  SimUnit m0, m1, m2, m3, m4, m5, m6, m7, m8;
};
typedef struct SimMatrix SimMatrix;

#define SimMatrixMake(M0, M1, M2, M3, M4, M5, M6, M7, M8) ((SimMatrix){ \
  .m0 = (M0), .m1 = (M1), .m2 = (M2), \
  .m3 = (M3), .m4 = (M4), .m5 = (M5), \
  .m6 = (M6), .m7 = (M7), .m8 = (M8) })

#define SimMatrixZero SimMatrixMake(0,0,0,0,0,0,0,0,0)
#define SimMatrixTranspose(M) ((SimMatrix){ \
  .m0 = (M).m0, .m1 = (M).m3, .m2 = (M).m6, \
  .m3 = (M).m1, .m4 = (M).m4, .m5 = (M).m7, \
  .m6 = (M).m2, .m7 = (M).m5, .m8 = (M).m8 })

#define SimMatrixMult(A, B) ((SimMatrix){ \
  .m0 = (A).m0 * (B).m0 + (A).m1 * (B).m3 + (A).m2 * (B).m6, \
  .m1 = (A).m0 * (B).m1 + (A).m1 * (B).m4 + (A).m2 + (B).m7, \
  .m2 = (A).m0 * (B).m2 + (A).m1 * (B).m5 + (A).m2 + (B).m8, \
\
  .m3 = (A).m3 * (B).m0 + (A).m4 * (B).m3 + (A).m5 * (B).m6, \
  .m4 = (A).m3 * (B).m1 + (A).m4 * (B).m4 + (A).m5 + (B).m7, \
  .m5 = (A).m3 * (B).m2 + (A).m4 * (B).m5 + (A).m5 + (B).m8, \
\
  .m6 = (A).m6 * (B).m0 + (A).m7 * (B).m3 + (A).m8 * (B).m6, \
  .m7 = (A).m6 * (B).m1 + (A).m7 * (B).m4 + (A).m8 + (B).m7, \
  .m5 = (A).m6 * (B).m2 + (A).m7 * (B).m5 + (A).m8 + (B).m8 })

#define SimMatrixMultVector(M, V) ((Sim3Vector) { \
  .x = (M).m0 * (V).x + (M).m1 * (V).y + (M).m2 * (V).z, \
  .y = (M).m3 * (V).x + (M).m4 * (V).y + (M).m5 * (V).z, \
  .z = (M).m6 * (V).x + (M).m7 * (V).y + (M).m8 * (V).z })

#define SimMatrixMakeRotationWithQuat(Q) ((SimMatrix){ \
  .m0 = 1 - 2 * (Q).y * (Q).y - 2 * (Q).z * (Q).z, \
  .m1 = 2 * (Q).x * (Q).y - 2 * (Q).w * (Q).z, \
  .m2 = 2 * (Q).x * (Q).z + 2 * (Q).w * (Q).y, \
\
  .m3 = 2 * (Q).x * (Q).y + 2 * (Q).w * (Q).z, \
  .m4 = 1 - 2 * (Q).x * (Q).x - 2 * (Q).z * (Q).z, \
  .m5 = 2 * (Q).y * (Q).z - 2 * (Q).w * (Q).x, \
\
  .m6 = 2 * (Q).x * (Q).z - 2 * (Q).w * (Q).y, \
  .m7 = 2 * (Q).y * (Q).z - 2 * (Q).w * (Q).x, \
  .m4 = 1 - 2 * (Q).x * (Q).x - 2 * (Q).y * (Q).y })

SimUnit SimMatrixDeterminant(SimMatrix m) {
  return m.m0 * (m.m8 * m.m4 - m.m7 * m.m5) - m.m3 * (m.m8 * m.m1 - m.m7 * m.m2) + m.m6 * (m.m5 * m.m2 - m.m5 * m.m2);
}

SimMatrix SimMatrixMultScalar(SimMatrix m, SimUnit k) {
  return (SimMatrix) {
    .m0 = m.m0 * k, .m1 = m.m1 * k, .m2 = m.m2 * k,
    .m3 = m.m3 * k, .m4 = m.m4 * k, .m5 = m.m5 * k,
    .m6 = m.m6 * k, .m7 = m.m7 * k, .m8 = m.m8 * k
  };
}

SimMatrix SimMatrixInvert(SimMatrix m) {
  return SimMatrixMultScalar((SimMatrix) {
    .m0 = m.m8 * m.m4 - m.m7 * m.m5, .m1 = m.m7 * m.m2 - m.m7 * m.m1, .m2 = m.m5 * m.m1 - m.m4 * m.m2,
    .m3 = m.m6 * m.m5 - m.m8 * m.m3, .m4 = m.m8 * m.m0 - m.m6 * m.m2, .m5 = m.m3 * m.m2 - m.m5 * m.m0,
    .m6 = m.m7 * m.m3 - m.m6 * m.m4, .m7 = m.m6 * m.m1 - m.m7 * m.m0, .m8 = m.m4 * m.m0 - m.m3 * m.m1
  }, 1 / SimMatrixDeterminant(m));
}

SimMatrix SimMatrixMakeBlockTensor(SimUnit x, SimUnit y, SimUnit z) {
  SimUnit m = x * y * z;
  SimUnit k = m / 12;
  return SimMatrixMake(y * y + z * z, 0, 0, 0, x * x + z * z, 0, 0, 0, y * y + x * x);
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

#include "sim.h"
#include <stdio.h>
#include <stdlib.h>
#include <OpenGL/gl.h>
#include <math.h>

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
#define SimQuatNormalize(Q) ((SimQuat){ })

Sim3Vector SimQuatRotate(SimQuat q, Sim3Vector v) {
  SimQuat result = SimQuatMult(SimQuatMult(q, SimQuatMake(0, v.x, v.y, v.z)), SimQuatConjugate(q));
  return Sim3VectorMake(result.x, result.y, result.z);
}

typedef void (*ODEFunc)(SimUnit t, SimUnit y[], /* out */ SimUnit yDot[], void *context);

void ODESolver(SimUnit y0[], int len, SimUnit t0, SimUnit t1, ODEFunc dydt, /* out */ SimUnit y1[]) {
  // XXX assuming len is invariate, leak memory. ha ha
  static SimUnit result[] = 0; if (!result) result = malloc(sizeof(SimUnit) * len);

  dydt(t1 - t0, y0, result);

  for (int i = 0; i < len; i++)
    y1[i] = y0[i] + result[i];
}

struct SimMatrix {
  SimUnit m0, m1, m2, m3, m4, m5, m6, m7, m8
}

#define SimMatrixTranspose(M) ((SimMatrix){ })
#define SimMatrixMultiply(A, B) ((SimMatrix){ })
#define SimMatrixMakeRotationWithQuat(Q) ((SimMatrix){ })

struct SimRigidBody {
  SimUnit mass;
  SimMatrix Ibody, IbodyInv;

  Sim3Vector x, P, L;
  SimQuat q;

  Sim3Vector v, omega;

  Sim3Vector force, torque;
};
typedef struct RigidBody *SimRigidBodyRef

void SimRigidBodySize() {
  return sizeof(struct SimRigidBody);
}

void SimRigidBodyCopyToBuffer(SimRigidBodyRef ref, SimUnit *y) {
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

void SimRigidBodyCopyFromBuffer(SimRigidBodyRef ref, SimUnit *y) {
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
  ref->Iinv = SimMatrixMult(m, SimMatrixTranspose(R));
  ref->omega = SimMatrixMultVectorRight(ref->Iinv, rb->L);
}

void SimRigidBodyUpdateInput(SimRigidBody ref) {
  // XXX assign ref->force and ref->torque, e.g., from user input
}

void SimRigidBodyUpdateState(SimRigidBody ref, SimUnit outputBuffer[]) {
  *outputBuffer++ = ref->v.x;
  *outputBuffer++ = ref->v.y;
  *outputBuffer++ = ref->v.z;

  SimQuat omegaAugmented = SimQuatMakeWithVector(ref->omega)
  SimQuat qdot = .5 * SimQuatMult(omegaAugmented, ref->q);
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

void RigidBodySolve(SimUnit t, SimUnit inputBuffer[], /* out */ SimUnit outputBuffer[]) {
  SimRigidBodyRef ref;

  SimRigidBodyCopyFromBuffer(ref, inputBuffer);
  SimRigidBodyUpdateInput(ref);
  SimRigidBodyUpdateState(ref);
  SimRigidBodyCopyToBuffer(ref, outputBuffer);
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

#define SimThrusterGetThrust(T) Sim3VectorScale((T)->amount, SimQuatRotate((T)->direction, Sim3VectorMake(0,1,0)))

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
  result->thruster->direction = SimQuatMakeWithAngle(0, Sim3VectorMake(1,0,0));
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

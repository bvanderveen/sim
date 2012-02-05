#include "rigidbody.h"
#include <stdlib.h>
#include <stdio.h>
#include "sim.h"

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
  ref->force = Sim3VectorMake(0,.01,0);
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
  SimContextRef ref = (SimContextRef)context;

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
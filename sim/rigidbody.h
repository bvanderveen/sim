#include "math.h"

#ifndef __SIM_RIGID_BODY
#define __SIM_RIGID_BODY

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

SimRigidBodyRef SimRigidBodyCreate();
void SimRigidBodyDestroy(SimRigidBodyRef ref);
int SimRigidBodyGetStateVectorSize();
void SimRigidBodyCopyToBuffer(SimRigidBodyRef ref, SimUnit y[]);
void SimRigidBodyCopyFromBuffer(SimRigidBodyRef ref, SimUnit y[]);
void SimRigidBodyUpdateInput(SimRigidBodyRef ref, SimUnit t);
void SimRigidBodyUpdateState(SimRigidBodyRef ref, SimUnit outputBuffer[]);
void RigidBodySolve(SimUnit t, SimUnit y[], /* out */ SimUnit yDot[], void *context);

#endif
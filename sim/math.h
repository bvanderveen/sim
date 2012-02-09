
#ifndef __SIM_MATH
#define __SIM_MATH

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

#define SimQuatMakeWithVector(V) SimQuatMake(0, (V).x, (V).y, (V).z)
SimQuat SimQuatMakeWithAngle(SimUnit angle, Sim3Vector unitVector);

#define SimQuatConjugate(Q) ((SimQuat){ .w = (Q).w, .x = -(Q).x, .y = -(Q).y, .z = -(Q).z })
#define SimQuatMult(L, R) ((SimQuat){ \
  .w = (L).w * (R).w - (L).x * (R).x - (L).y * (R).y - (L).z * (R).z, \
  .x = (L).w * (R).x + (L).x * (R).w + (L).y * (R).z - (L).z * (R).y, \
  .y = (L).w * (R).y + (L).y * (R).w + (L).z * (R).x - (L).x * (R).z, \
  .z = (L).w * (R).z + (L).z * (R).w + (L).x * (R).y - (L).y * (R).x })
#define SimQuatScale(K, Q) ((SimQuat){ .w = (K) * (Q).w, .x = (K) * (Q).x, .y = (K) * (Q).y, .z = (K) * (Q).z })
#define SimQuatNormalize(Q) (Q)

Sim3Vector SimQuatRotate(SimQuat q, Sim3Vector v);

struct SimMatrix {
  SimUnit m0, m1, m2, m3, m4, m5, m6, m7, m8;
};
typedef struct SimMatrix SimMatrix;

#define SimMatrixMake(M0, M1, M2, M3, M4, M5, M6, M7, M8) ((SimMatrix){ \
  .m0 = (M0), .m1 = (M1), .m2 = (M2), \
  .m3 = (M3), .m4 = (M4), .m5 = (M5), \
  .m6 = (M6), .m7 = (M7), .m8 = (M8) })

#define SimMatrixZero SimMatrixMake(0,0,0,0,0,0,0,0,0)
#define SimMatrixIdentity SimMatrixMake(1,0,0,0,1,0,0,0,1)
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
  .m7 = 2 * (Q).y * (Q).z + 2 * (Q).w * (Q).x, \
  .m8 = 1 - 2 * (Q).x * (Q).x - 2 * (Q).y * (Q).y })

SimUnit SimMatrixDeterminant(SimMatrix m);
SimMatrix SimMatrixMultScalar(SimMatrix m, SimUnit k);
SimMatrix SimMatrixInvert(SimMatrix m);
SimMatrix SimMatrixMakeBlockTensor(SimUnit x, SimUnit y, SimUnit z);

typedef void (*ODEFunc)(SimUnit t, SimUnit y[], /* out */ SimUnit yDot[], void *context);

void ODESolver(SimUnit y0[], int len, SimUnit t0, SimUnit t1, ODEFunc dydt, /* out */ SimUnit y1[], void *context);

#endif

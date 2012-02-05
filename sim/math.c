#include "math.h"
#include <math.h>
#include <stdio.h>
#include <stdlib.h>

Sim3Vector SimQuatRotate(SimQuat q, Sim3Vector v) {
  SimQuat result = SimQuatMult(SimQuatMult(q, SimQuatMake(0, v.x, v.y, v.z)), SimQuatConjugate(q));
  return Sim3VectorMake(result.x, result.y, result.z);
}

SimQuat SimQuatMakeWithAngle(SimUnit angle, Sim3Vector unitVector) {
  Sim3Vector v = Sim3VectorScale(sin(1.0 / 2.0 * angle), unitVector);
  return ((SimQuat){ .w = cos(1.0 / 2.0 * angle), .x = v.x, .y = v.y, .z = v.z });
}

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

void ODESolver(SimUnit y0[], int len, SimUnit t0, SimUnit t1, ODEFunc dydt, /* out */ SimUnit y1[], void *context) {
  // XXX assuming len is invariate, leak memory. ha ha
  static SimUnit *result = 0; 
  if (!result) {
    result = (SimUnit *)malloc(sizeof(SimUnit) * len);
  }

  dydt(t1 - t0, y0, result, context);

  printf("initial state: ");

  for (int i = 0; i < len; i++) {
    printf("%f ", y0[i]);
  }
  printf("\n");
  printf("d/dt state:    ");

  for (int i = 0; i < len; i++) {
    printf("%f ", result[i]);
  }
  printf("\n");
  printf("final state:   ");

  for (int i = 0; i < len; i++) {
    y1[i] = y0[i] + result[i];
    printf("%f ", y1[i]);
  }
  printf("\n");
}

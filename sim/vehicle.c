#include "vehicle.h"
#include <stdlib.h>
#include <stdio.h>
#include <OpenGL/gl.h>

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

  Sim3Vector position = ref->body->x;
  SimMatrix rotation = SimMatrixMakeRotationWithQuat(ref->body->q);

  const SimUnit tx[] = {
    rotation.m0, rotation.m3, rotation.m6, 0,
    rotation.m1, rotation.m4, rotation.m7, 0,
    rotation.m2, rotation.m5, rotation.m8, 0,
    position.x, position.y, position.z, 1
  };

  glMultMatrixf((GLfloat *)tx);

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
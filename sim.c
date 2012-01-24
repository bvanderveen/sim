#include "sim.h"
#include <stdio.h>
#include <stdlib.h>
#include <OpenGL/gl.h>

struct SimContext
{
  double t;
};
typedef struct SimContext *SimContext;

SimContextRef SimContextCreate() {
  SimContext result = (SimContext)malloc(sizeof(SimContext));
  result->t = 0.0;
  return (SimContext)result;
}

void SimContextDestroy(SimContextRef c) {
  free(c);
}

void SimContextUpdate(SimContextRef c, double t) {
  SimContext ctx = c;
  ctx->t += t * .0001;
}

void SimContextDraw(SimContextRef c, double t) {
  SimContext ctx = c;
  
  glRotated(ctx->t, 0,0,1);

  glClearColor(0,0,0,0);
  glClear(GL_COLOR_BUFFER_BIT);

  glColor3f(1.0f, 0.85f, 0.35f);
  glBegin(GL_TRIANGLES);
  {
    glVertex3f(  0.0,  0.6, 0.0);
    glVertex3f( -0.2, -0.3, 0.0);
    glVertex3f(  0.2, -0.3 ,0.0);
  }
  glEnd(); 
  glFlush();
}


    // const int TICKS_PER_SECOND = 25;
    // const int SKIP_TICKS = 1000 / TICKS_PER_SECOND;
    // const int MAX_FRAMESKIP = 5;

    // DWORD next_game_tick = GetTickCount();
    // int loops;
    // float interpolation;

    // bool game_is_running = true;
    // while( game_is_running ) {

    //     loops = 0;
    //     while( GetTickCount() > next_game_tick && loops < MAX_FRAMESKIP) {
    //         update_game();

    //         next_game_tick += SKIP_TICKS;
    //         loops++;
    //     }

    //     interpolation = float( GetTickCount() + SKIP_TICKS - next_game_tick )
    //                     / float( SKIP_TICKS );
    //     display_game( interpolation );
    // }

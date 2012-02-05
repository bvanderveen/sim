
typedef void *SimContextRef;

SimContextRef SimContextCreate();
void SimContextDestroy(SimContextRef c);

void SimContextUpdate(SimContextRef c, double dt);
void SimContextDraw(SimContextRef c, double dt);

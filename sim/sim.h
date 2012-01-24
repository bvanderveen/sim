
typedef void *SimContextRef;

SimContextRef SimContextCreate();
void SimContextDestroy(SimContextRef c);

void SimContextUpdate(SimContextRef c, double t);
void SimContextDraw(SimContextRef c, double t);


struct sim_ctx_t
{
  double t;
};
typedef struct sim_ctx_t *sim_ctx;

sim_ctx sim_create();
void sim_destroy(sim_ctx c);

void sim_update(sim_ctx c, double t);
void sim_draw(sim_ctx c, double t);

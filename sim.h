

struct sim_ctx_t
{
	double t;
};
typedef struct sim_ctx_t sim_ctx;

sim_ctx *sim_create();
void sim_destroy(sim_ctx *c);

void sim_update();
void sim_draw(sim_ctx *ctx, double t);
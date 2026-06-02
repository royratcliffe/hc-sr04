struct cons;

void call_at(struct cons **ca, void (*func)(void *), void *arg1);

void call_up(struct cons **ca);

struct cons;

/*!
 * \brief Schedule a function to be called.
 * \param ca Pointer to a pointer to the cons list of scheduled functions.
 * \param func Function pointer to the function to be called.
 * \param arg1 Argument to be passed to the function when called.
 */
void call_at(struct cons **ca, void (*func)(void *), void *arg1);

/*!
 * \brief Call all scheduled functions and clear the list.
 * \param ca Pointer to a pointer to the cons list of scheduled functions.
 */
void call_up(struct cons **ca);

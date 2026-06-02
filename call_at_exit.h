void call_at_exit(void (*func)(void *), void *arg1);

/*!
 * \brief Registers a pointer to be freed at program exit.
 * \param car The pointer value to be freed at program exit.
 * \details This function creates a new cons cell on the heap to store the
 * pointer value in its \c car field. It then prepends this cons cell to a
 * global list of cons cells. The \c cdr field of the cons cell is used to link
 * it into the list. When the program exits, a \c clean_up function will be
 * called, which will iterate through the list of cons cells and free the
 * pointer values stored in their \c car fields, as well as the cons cells
 * themselves. This allows for a convenient way to register pointers for cleanup
 * without having to manage the list of pointers manually.
 */
void free_at_exit(void *heap);

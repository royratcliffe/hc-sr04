/* SPDX-License-Identifier: MIT */

/*!
 * \file cons.h
 * \brief A simple implementation of cons cells for building linked lists.
 * \copyright 2026, Roy Ratcliffe, Northumberland, United Kingdom
 */
#ifndef CONS_H
#define CONS_H

/*
 * for NULL (the empty list)
 */
#include <stddef.h>

/*
 * for bool type
 */
#include <stdbool.h>

#ifdef __cplusplus
extern "C" {
#endif

struct cons;

/*!
 * \brief The empty list (a \c NULL pointer).
 * \details Sentinel constant marking the end of a cons list.
 *
 * The \c CONS_NIL macro represents "the" empty list of cons cells. It is defined
 * as a \c NULL pointer for easily checking for the end of a list. When creating a
 * linked list of cons cells, the last cell's \c cdr field will point to
 * \c CONS_NIL, indicating that there are no more cells in the list.
 */
#define CONS_NIL ((struct cons *)NULL)

/*! \brief Returns \c true if the cons cell is \c CONS_NIL. */
#define CONS_NIL_P(_cell) ((_cell) == CONS_NIL)

/*! \brief Returns \c true if the cons cell is not \c CONS_NIL. */
#define CONS_NOT_NIL_P(_cell) ((_cell) != CONS_NIL)

/*! \brief Builds a cons cell with the given \c car and \c cdr values. */
#define CONS(_car, _cdr) ((struct cons){.car = (_car), .cdr = (_cdr)})

/*! \brief Builds a null cons cell structure. */
#define CONS_NULL CONS(NULL, CONS_NIL)

/*!
 * \brief Construct cell structure for building linked lists.
 * \details The \c car field holds data, the \c cdr field points to the next
 * cons cell.
 *
 * A cons cell is a fundamental data structure in Lisp-like languages, where it
 * is used to build lists and other complex data structures. The \c car field
 * holds the actual data or value, while the \c cdr field points to the next
 * cons cell in the list, allowing for the construction of linked lists and
 * other complex data structures of arbitrary length.
 *
 * Why "cons"? The term "cons" is derived from the word "construct," as
 * cons cells are used to construct lists and other data structures. The
 * \c cons function is a fundamental operation in Lisp that prepends an
 * already-existing cons cell, allowing programmers to build complex
 * data structures by linking cons cells together. The name reflects the
 * purpose of the data structure, which is to construct and manipulate
 * lists and other composite data types.
 *
 * What does CAR and CDR mean? CAR stands for "Contents of Address Register" and
 * CDR stands for "Contents of Decrement Register." These terms originate from
 * the early days of Lisp programming and refer to the two parts of a cons cell.
 */
struct cons {
  /*!
   * \brief Contents of Address Register (CAR).
   * \details Data field (can hold any type).
   *
   * The \c car field of a cons cell is a versatile component that can hold any
   * type of data. It serves as the primary pointer-sized storage for the value
   * or data associated with the cons cell; this optionally includes sub-cons
   * cells \e recursively.
   */
  void *car;

  /*!
   * \brief Contents of Decrement Register (CDR).
   * \details Pointer to the next cons cell (or \c CONS_NIL).
   */
  struct cons *cdr;
};

/*!
 * \brief Accessor for the \c car field of a cons cell.
 * \param cell The cons cell to access.
 * \return The value stored in the \c car field of the cons cell.
 */
static inline void *cons_car(const struct cons *cell) { return cell->car; }

/*!
 * \brief Accessor for the \c cdr field of a cons cell.
 * \param cell The cons cell to access.
 * \return Pointer to the next cons cell (or \c CONS_NIL).
 */
static inline struct cons *cons_cdr(const struct cons *cell) { return cell->cdr; }

/*!
 * \brief Mutator for the \c car field of a cons cell.
 * \param cell The cons cell to modify.
 * \param car The new value to set in the \c car field of the cons cell.
 */
static inline void cons_rplaca(struct cons *cell, void *car) { cell->car = car; }

/*!
 * \brief Mutator for the \c cdr field of a cons cell.
 * \param cell The cons cell to modify.
 * \param cdr The new value to set in the \c cdr field of the cons cell.
 */
static inline void cons_rplacd(struct cons *cell, struct cons *cdr) { cell->cdr = cdr; }

/*! \brief Initialises a cons cell with the given \c car value and \c cdr set to \c CONS_NIL. */
static inline void cons_init(struct cons *cell, void *car) {
  cons_rplaca(cell, car);
  cons_rplacd(cell, CONS_NIL);
}

/*!
 * \brief Prepends a cons cell to a list.
 * \param list Pointer to the list head to which the new cell will be prepended.
 * This is a pointer to a pointer to a cons cell, allowing the function to
 * update the list pointer to point to the new cell.
 * \param cell The cons cell to prepend to the list.
 * \return Pointer to the \c cdr field of the new cell for chaining. Use the
 * return value to further add elements to the list by chaining additional cons
 * cells together.
 * \details The \c cons function takes a pointer to a list (which is a pointer
 * to a cons cell) and a cons cell to prepend. It initialises the \c cdr of the
 * new cell to point to the existing list and updates the list pointer to point
 * to the new cell. This effectively adds the new cell to the front of the list.
 * The \c cons function is a fundamental operation in Lisp-like languages, where
 * it is used to construct lists and other complex data structures. The \c car
 * field of the new cell can hold any type of data, while the \c cdr field is
 * specifically designed to link to another cons cell, facilitating the
 * construction of linked lists and other complex data structures. The \c cons
 * function allows for efficient list manipulation by enabling the addition of
 * new elements to the front of the list without needing to traverse the entire
 * list, making it a powerful tool for building and modifying lists in a
 * flexible and efficient manner.
 */
struct cons **cons(struct cons **list, struct cons *cell);

/*!
 * \brief Prepends a cons cell to a list, returning the updated list pointer.
 * \param list Pointer to the list head to which the new cell will be
 * prepended.
 * \param cell The cons cell to prepend to the list.
 * \return Pointer to the updated list pointer, now pointing to the new
 * cell.
 * \details A wrapper around \c cons that allows convenient chaining of
 * cons cell additions without separately managing the list pointer.
 */
struct cons **cons_prepend(struct cons **list, struct cons *cell);

/*!
 * \brief Loops through a list of cons cells, applying a predicate
 * function to each cell.
 * \param list Pointer to the list head to loop through. This is a
 * pointer to a pointer to a cons cell.
 * \param pred A predicate function that takes a pointer to the current
 * list pointer, the current cons cell, and a user-defined pointer. The
 * predicate should return \c true if the current cell matches the
 * desired condition, and \c false otherwise.
 * \param user A user-defined pointer that can be passed to the
 * predicate function for additional context or data needed for the
 * predicate's logic.
 * \return Pointer to the list pointer of the first cons cell for which
 * the predicate returns \c true, or \c NULL if no such cell is found.
 * The returned pointer allows the caller to modify the list starting
 * from the found cell if needed. If the predicate does not find a
 * matching cell in the list, the function returns \c NULL to indicate
 * that the search was unsuccessful. This design allows the function to
 * signal the absence of a matching cell without returning a pointer to
 * a cons cell, which would be misleading since it would suggest that a
 * valid cell was found when in fact it was not. By returning \c NULL,
 * the function provides a clear and unambiguous way to indicate that
 * the search was unsuccessful.
 */
struct cons **cons_loop(struct cons **list, bool (*pred)(struct cons **list, struct cons *cell, void *user),
                        void *user);

/*!
 * \brief Finds the first cons cell in a list that matches a given identity.
 * \param list Pointer to the list head to search through. This is a pointer
 * to a pointer to a cons cell.
 * \param cell The cons cell to find by identity.
 * \return Pointer to the list pointer of the first cons cell that matches the
 * specified identity, or \c NULL if no such cell is found.
 */
struct cons **cons_find(struct cons **list, void *cell);

/*!
 * \brief Destructively deletes the \e first cons cell with the specified \c car
 * value from the list.
 * \param list Pointer to the list head.
 * \param car The value to match for deletion.
 * \return Pointer to the deleted cons cell, or \c CONS_NIL if no matching cell
 * was found.
 * \details This function traverses the list of cons cells, looking for the
 * first cell whose \c car field matches the specified pointer value. If such a
 * cell is found, it is removed from the list by updating the \c cdr pointer of
 * the previous cell (or the head pointer if the cell to delete is the first
 * cell) to point to the next cell, effectively bypassing the deleted cell. The
 * function then returns a pointer to the deleted cell. If no matching cell is
 * found after traversing the entire list, the function returns \c CONS_NIL to
 * indicate that no deletion occurred. This operation is destructive because it
 * modifies the original list structure by removing a cell from it. The caller
 * is responsible for managing the memory of the deleted cell if necessary, as
 * this function does not free the memory of the deleted cell; it only removes
 * it from the list.
 */
struct cons *cons_delete(struct cons **list, void *car);

/*!
 * \brief Destructively removes the specified cons cell from the list.
 * \param list Pointer to the list head.
 * \param cell The cons cell to remove from the list, matched by identity.
 * \retval The removed cons cell if the specified cell was found and removed.
 * \retval \c CONS_NIL if the specified cell was not found in the list.
 * \details Traverses the list of cons cells, looking for the specified
 * cell. If the cell is found, it is removed from the list by updating
 * the \c cdr pointer of the previous cell (or the head pointer if the
 * cell to remove is the first cell) to point to the next cell, thereby
 * bypassing the removed cell.
 */
struct cons *cons_remove(struct cons **list, struct cons *cell);

/*!
 * \brief Reverses a linked list of cons cells in place.
 * \details This function takes a pointer to the head of a linked list of cons
 * cells and reverses the order of the cells in the list. It iteratively
 * traverses the list, reassigning the cdr pointers to point to the previous
 * cell, effectively reversing the list.
 * \param list The list to reverse. This is a pointer to the head of the list,
 * and it will be updated to point to the new head of the reversed list.
 */
void cons_reverse(struct cons **list);

/*!
 * \brief Computes the length of a linked list of cons cells.
 * \param cell The head of the list to compute the length of, or \c CONS_NIL for
 * an empty list.
 * \return The number of cons cells in the list.
 * \details This function iteratively traverses the linked list of cons cells,
 * counting the number of cells until it reaches the end of the list (indicated
 * by \c CONS_NIL). It returns the total count as the length of the list.
 */
size_t cons_length(const struct cons *cell);

/*!
 * \brief Returns the last cons cell in a linked list of cons cells.
 * \param cell The head of the list to find the last cell of, or \c CONS_NIL for
 * an empty list.
 * \return Pointer to the last cons cell in the list, or \c CONS_NIL if the
 * list is empty.
 * \details This function iteratively traverses the linked list of cons cells
 * until it reaches the last cell, which is identified by having its \c cdr field
 * equal to \c CONS_NIL. It returns a pointer to this last cell. If the input
 * list is empty (i.e., if the input pointer is \c CONS_NIL), the function
 * returns \c CONS_NIL to indicate that there are no cells in the list.
 */
struct cons *cons_last(struct cons *cell);

/*!
 * \brief Returns the n'th cons cell in a linked list of cons cells.
 * \param cell The head of the list to find the n'th cell of, or \c CONS_NIL for
 * an empty list.
 * \param nth The zero-based index of the cell to retrieve.
 * \return Pointer to the n'th cons cell in the list, or \c CONS_NIL if the index
 * is out of bounds.
 * \details Iteratively traverses the linked list of cons cells, counting the
 * cells as it goes. When the count reaches the specified index \c nth, it
 * returns a pointer to the \e current cell. If the end of the list is reached
 * before finding the n'th cell (i.e., if the input pointer becomes \c CONS_NIL),
 * the function returns \c CONS_NIL to indicate that the index is out of bounds.
 */
struct cons *cons_nth(struct cons *cell, size_t nth);

/*!
 * \brief Returns the \e first cons cell in a linked list of cons cells whose \c car
 * field matches the specified pointer value.
 * \param cell The head of the list to search, or \c CONS_NIL for an empty list.
 * \param car The pointer value to match in the \c car field of the cons cells.
 * \return Pointer to the \e first matching cons cell, or \c CONS_NIL if no match is found.
 * \details Iteratively traverses the linked list of cons cells, comparing the
 * \c car field of each cell with the specified pointer value. If a match is found, a
 * pointer to the matching cell is returned. If the end of the list is reached
 * \e without finding a match, \c CONS_NIL is returned.
 */
struct cons *cons_member(struct cons *cell, void *car);

/*!
 * \brief Appends one list of cons cells to another.
 * \param cell1 The first list to which the second list will be appended. This
 * list will be modified to include the second list at its end.
 * \param cell2 The second list to append to the first list. This list will not
 * be modified, but its cells will be linked into the first list.
 * \return Pointer to the head of the combined list, which is the same as
 * \c cell1 if it is not empty, or \c cell2 if \c cell1 is empty.
 * \details This function takes two lists of cons cells and appends the second
 * list to the end of the first list. If the first list is empty (i.e., if
 * \c cell1 is \c CONS_NIL), it simply returns \c cell2 as the new combined
 * list. If the first list is not empty, it finds the last cell of the first
 * list and updates its \c cdr pointer to point to the head of the second
 * list, effectively linking the two lists together. The function then returns
 * a pointer to the head of the combined list, which is the same as \c cell1.
 */
struct cons *cons_append(struct cons *cell1, struct cons *cell2);

/*!
 * \brief Allocates a new cons cell on the heap.
 * \details Allocates memory for a new cons cell and initialises
 * it with the provided \c car value. The caller is responsible for
 * freeing the allocated memory when it is no longer needed.
 * \param car The value to store in the \c car field of the new cons cell.
 * \return Pointer to the newly allocated cons cell, or \c CONS_NIL if memory
 * allocation fails.
 */
struct cons *cons_heap(void *car);

/*! \brief Frees a heap-allocated cons cell. */
void cons_free(struct cons *cell);

#ifdef __cplusplus
}
#endif

#endif /* CONS_H */

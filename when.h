/* SPDX-License-Identifier: MIT */

/*!
 * \file when.h
 * \details Fairly advanced C preprocessor macros used for modular registration
 * in a callback framework that utilises an automatic *compile-time*
 * registration mechanism for embedded architectures.
 * \copyright 2025, Roy Ratcliffe, Northumberland, United Kingdom
 * \author Roy Ratcliffe <roy@ratcliffe.me>
 *
 * Permission is hereby granted, free of charge, to any person obtaining a copy of this software and associated
 * documentation files (the "Software"), to deal in the Software without restriction, including without limitation the
 * rights to use, copy, modify, merge, publish, distribute, sublicense, and/or sell copies of the Software, and to
 * permit persons to whom the Software is furnished to do so, subject to the following conditions:
 *
 *      The above copyright notice and this permission notice shall be included in all copies or substantial
 *      portions of the Software.
 *
 * THE SOFTWARE IS PROVIDED "AS IS", WITHOUT WARRANTY OF ANY KIND, EXPRESS OR IMPLIED, INCLUDING BUT NOT LIMITED TO THE
 * WARRANTIES OF MERCHANTABILITY, FITNESS FOR A PARTICULAR PURPOSE AND NONINFRINGEMENT. IN NO EVENT SHALL THE AUTHORS OR
 * COPYRIGHT HOLDERS BE LIABLE FOR ANY CLAIM, DAMAGES OR OTHER LIABILITY, WHETHER IN AN ACTION OF CONTRACT, TORT OR
 * OTHERWISE, ARISING FROM, OUT OF OR IN CONNECTION WITH THE SOFTWARE OR THE USE OR OTHER DEALINGS IN THE SOFTWARE.
 */
#ifndef WHEN_H_
#define WHEN_H_

#include "lc.h"

/*!
 * \brief Start of when section.
 * \details This macro defines the start of a section in the binary where
 * callback functions for a specific event are stored. It is used in conjunction
 * with the END_OF_WHEN macro to delineate the section.
 * \param _when_ The name of the event for which the section is defined.
 */
#define START_OF_WHEN(_when_) START_OF_SECTION(when_##_when_)

/*!
 * \brief End of when section.
 * \details This macro defines the end of a section in the binary where
 * callback functions for a specific event are stored. It is used in conjunction
 * with the START_OF_WHEN macro to delineate the section.
 * \param _when_ The name of the event for which the section is defined.
 */
#define END_OF_WHEN(_when_) END_OF_SECTION(when_##_when_)

/*!
 * \brief When section extern declarations.
 * \details This macro declares the start and end symbols for a section in the
 * binary where callback functions for a specific event are stored. It is used
 * to make these symbols visible across translation units for correct linker
 * resolution.
 * \param _when_ The name of the event for which the section symbols are declared.
 * \note This macro is designed to work with compilers that support specific
 * section attributes, such as the TASKING or GCC compilers.
 */
#if defined(__TASKING__)
#define WHEN(_when_)                              \
  extern void (*const START_OF_WHEN(_when_)[])(); \
  extern void (*const END_OF_WHEN(_when_)[])();
#elif defined(__GNUC__)
#define WHEN(_when_)                                                    \
  extern void (*const START_OF_WHEN(_when_)[])() __attribute__((weak)); \
  extern void (*const END_OF_WHEN(_when_)[])() __attribute__((weak))
#endif /* __TASKING__ || __GNUC__ */

/*!
 * \brief Whenever something happens, do what.
 * \details This macro defines a function that is called when a specific event
 * occurs. It is used to register a callback function that will be executed when
 * the specified event occurs. The function is placed in a specific section of
 * the binary, allowing it to be discovered and executed at runtime.
 *
 * By design, the macro requires a semicolon at the end of the CAUSES macro
 * invocation to ensure that the function definition is correctly terminated and
 * to allow for multiple invocations of the CAUSES macro without syntax errors.
 * This design choice allows for a clean and consistent way to register multiple
 * callbacks for the same event without needing to worry about the underlying
 * section management, as the linker will handle the placement of the functions
 * in memory. The semicolon also helps to visually separate different callback
 * registrations in the code, improving readability.
 *
 * \param _when_ The name of the event that triggers the callback.
 * \param _what_ The name of the function to be called when the event occurs.
 */
#if defined(__TASKING__)
/*
 * The TASKING compiler concatenates the section name with the function name to
 * create a unique section for each callback, but it does not adjust the section
 * end marker (_lc_ue_) to account for multiple callbacks in the same section.
 *
 * Use the same section name for all callbacks of the same event, and rely on
 * the linker to place them contiguously in memory. The CAUSES macro defines a
 * function with the specified name and places it in the appropriate section,
 * allowing multiple callbacks for the same event to be registered without
 * conflicts. The OCCURS macro then iterates over all callbacks in that section
 * when the event occurs, ensuring that all registered functions are called.
 *
 * The .sdecl directive is used to declare the section in assembly language, and
 * the .word directive is used to place the address of the callback function in
 * the section; the .sect directive is used to select the section. The
 * __attribute__((used)) ensures that the function is not optimised away by the
 * compiler, even if it appears to be unused in the code.
 */
#define CAUSES(_when_, _what_, ...)              \
  static void _what_(...) __attribute__((used)); \
  __asm(".sdecl\t'when_" #_when_ "',data,rom,concat,protect\n\
\t.sect\t'when_" #_when_ "'\n\
\t.align\t4\n\
\t.word\t" #_what_);                             \
  static void _what_(...) __attribute__((used))
#elif defined(__GNUC__)
#define CAUSES(_when_, _what_, ...)                                                         \
  static void _what_(__VA_ARGS__);                                                          \
  SECTION_USED(when_##_when_) static void (*const __when__##_what_)(__VA_ARGS__) = &_what_; \
  static void _what_(__VA_ARGS__)
#endif /* __TASKING__ || __GNUC__ */

/*!
 * \brief When something happened, do something with it.
 * \details This macro defines a function that is called when a specific event occurs.
 * It registers the function in a dedicated section of the binary, so the system can
 * discover and execute it at runtime. Use this macro in modular callback frameworks
 * to register multiple handlers for the same event, enabling flexible and extensible
 * event handling.
 *
 * The implementation iterates over all registered callback functions for the
 * specified event and calls each one with the provided argument.
 *
 * For the TASKING compiler, the `_lc_ub_` and `_lc_ue_` prefixes mark the start and
 * end of the callback list for the event. The `extern` declarations make the function
 * pointers visible across translation units for correct linker resolution.
 *
 * For the GCC compiler, the `__start_` and `__stop_` prefixes mark the start and end
 * of the callback list for the event.
 *
 * \param _when_ The name of the event that triggers the callback.
 * \param ... The arguments to be passed to the callback function when the
 * event occurs.
 * \note This macro is designed to work with compilers that support specific
 * section attributes, such as the TASKING or GCC compilers. It uses weak
 * symbols to allow for multiple definitions of the same function without
 * causing linker errors.
 */
#define OCCURS(_when_, ...)                                                                   \
  do {                                                                                        \
    WHEN(_when_);                                                                             \
    for (void (*const *what)() = START_OF_WHEN(_when_); what < END_OF_WHEN(_when_); what++) { \
      (**what)(__VA_ARGS__);                                                                  \
    }                                                                                         \
  } while (0)

/*!
 * \brief Type definition for when callback functions.
 * \details Defines the signature for callback functions registered with
 * CAUSES. These functions take any arguments and return void.
 * \note In C99, empty parameter lists in function prototypes indicate
 * that the function can take any number of arguments. This allows for
 * flexibility in the types of callbacks that can be registered, as they
 * can accept different parameters based on the event they are
 * associated with.
 * \see CAUSES
 * \see OCCURS
 */
typedef void (*when_func_t)();

#endif /* WHEN_H_ */

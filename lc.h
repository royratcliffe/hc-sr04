/* SPDX-License-Identifier: MIT */

/*!
 * \file lc.h
 * \brief Low-level C library definitions.
 * \details This header file contains definitions for low-level C library
 * functionalities.
 *
 * This file provides macros for used-section attributes,
 * and section management. It is designed to be included in C source files
 * to facilitate low-level programming tasks.
 *
 * Example usage:
 *
 * \code
 * #include "lc.h"
 *
 * // The following lines declare the start and end of a section named "hello".
 * // These macros are used to define the boundaries of a section in memory.
 * extern const char START_OF_SECTION(hello)[];
 * extern const char END_OF_SECTION(hello)[];
 *
 * // The line below places the `hello` variable in a section named "hello"
 * // and marks it as used, preventing the linker from removing it.
 * // It defines a section named "hello" and marks it as used.
 * // This section can contain strings or other data that will not be
 * // optimized away by the linker.
 * static const char hello[] SECTION_USED(hello) = "hello"; // @suppress("Unused variable declaration in file scope")
 *
 * // To access the section, you can use the START_OF_SECTION and END_OF_SECTION macros.
 * // These macros provide pointers to the start and end of the section,
 * // allowing you to iterate over the contents of the section.
 * // The loop below iterates over the strings in the "hello" section.
 * for (const char *str = START_OF_SECTION(hello); str < END_OF_SECTION(hello); str++)
 * {
 *     size_t len = strlen(str);
 *     str += len;
 * }
 * \endcode
 *
 * \copyright 2025, Roy Ratcliffe, Northumberland, United Kingdom
 * \author Roy Ratcliffe <roy@ratcliffe.me>
 */
#ifndef LC_H_
#define LC_H_

#include "stringify.h"

/*!
 * \brief Section used attribute.
 * \details This macro is utilised to mark a function or variable as used, ensuring
 * that it is not eliminated by the linker during optimisation.
 *
 * This is particularly useful for functions that are intended to be
 * called dynamically or registered as callbacks, ensuring they remain in the
 * final binary.
 *
 * The section name must be a valid C identifier without spaces or special
 * characters. Choose a descriptive name that reflects the section’s purpose and
 * follows project naming conventions. Section names group related functions or
 * variables, aiding organisation and management in the binary. Ensure the
 * section name is unique within the binary to prevent conflicts. Use
 * underscores to separate words, following standard C naming practices. The
 * linker uses the section name to place the item in the correct section and to
 * prevent its removal during optimisation.
 *
 * Note that different compilers may have varying syntax for specifying
 * section attributes. This macro abstracts those differences, providing a
 * consistent interface for marking items as used across supported compilers.
 *
 * The TASKING compiler includes the `protect` attribute to safeguard the section
 * from being altered or removed during optimisation processes. Without this attribute,
 * the linker might consider the section as unused and eliminate it, especially if it
 * contains functions or variables that are not explicitly referenced in the code.
 *
 * \param _section_ The name of the section where the function or variable should be placed.
 * \note The section name is converted to a string literal using the STRINGIFY macro.
 */
#if defined(__TASKING__)
#define SECTION_USED(_section_) __attribute__((section(STRINGIFY(_section_)), used, protect))
#elif defined(__GNUC__)
#define SECTION_USED(_section_) __attribute__((section(STRINGIFY(_section_)), used))
#endif

/*!
 * \brief Start of section.
 * \details This macro marks the start of a section for the specified
 * section name.
 *
 * The purpose of this macro is to provide a compiler-independent way to define
 * the start of a section in the binary. It is typically used in conjunction
 * with the end of section macro to allow for iteration over the contents of the
 * section.
 *
 * \param _section_ The name of the section.
 * \note The start of section is used to determine the range of the section
 * and is typically used in conjunction with the end of section macro.
 */
#if defined(__TASKING__)
#define START_OF_SECTION(_section_) _lc_ub_##_section_
#elif defined(__GNUC__)
#define START_OF_SECTION(_section_) __start_##_section_
#endif /* __TASKING__ || __GNUC__ */

/*!
 * \brief End of section.
 * \details This macro marks the end of a section for the specified
 * section name.
 *
 * \param _section_ The name of the section.
 * \note The end of section is used to determine the range of the section
 * and is typically used in conjunction with the start of section macro.
 * \sa START_OF_SECTION
 */
#if defined(__TASKING__)
#define END_OF_SECTION(_section_) _lc_ue_##_section_
#elif defined(__GNUC__)
#define END_OF_SECTION(_section_) __stop_##_section_
#endif /* __TASKING__ || __GNUC__ */

#endif /* LC_H_ */

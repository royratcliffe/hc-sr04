/* SPDX-License-Identifier: MIT */

/*!
 * \file stringify.h
 * \brief Stringify macro definitions.
 * \details This header file contains macro definitions for converting tokens
 * into string literals.
 * \copyright 2025, Roy Ratcliffe, Northumberland, United Kingdom
 * \author Roy Ratcliffe <roy@ratcliffe.me>
 */
#ifndef STRINGIFY_H_
#define STRINGIFY_H_

/*!
 * \brief Converts the macro argument into a string literal.
 * \details This macro takes its argument and converts it into a string literal.
 * It is commonly used in preprocessor directives or attributes where the C code
 * requires the string-ification of some arbitrary token.
 *
 * Take an example:
 * \code
 * #define VERSION 1.0
 * // the following will expand to "1.0"
 * const char *version_string = STRINGIFY(VERSION);
 * \endcode
 *
 * \param x The token or value to be converted to a string literal.
 */
#define STRINGIFY(x) #x

/*!
 * \brief Expands the macro argument and then converts it into a string literal.
 * \details This macro first expands its argument if it is a macro itself,
 * and then converts the result into a string literal. This is useful when
 * you want to stringify the value of a macro rather than the macro name.
 * \param x The token or value to be expanded and then converted to a string literal.
 */
#define XSTRINGIFY(x) STRINGIFY(x)

#endif /* STRINGIFY_H_ */

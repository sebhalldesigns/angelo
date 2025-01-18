/***************************************************************
**
** Angelo Library Header File
**
** File         :  naming_convention.h
** Module       :  misc
** Project      :  Angelo
** Author       :  SH
** Created      :  2025-01-08 (YYYY-MM-DD)
** License      :  MIT
** Description  :  A header file containing naming conventions.
**
***************************************************************/
#ifndef NAMING_CONVENTION_H
#define NAMING_CONVENTION_H

/***************************************************************
** MARK: INCLUDES
***************************************************************/

/* all includes to be justified where possible */

/***************************************************************
** MARK: CONSTANTS & MACROS
***************************************************************/

/* all defines to be in caps */
#define EXAMPLE_DEFINE 1

/***************************************************************
** MARK: TYPEDEFS
***************************************************************/

/* all types to be in pascal case */

typedef int ExampleIntType;

typedef struct ExampleStructType {
    int a;
    int b;
} ExampleStructType;

/***************************************************************
** MARK: FUNCTION DEFS
***************************************************************/

/* all functions to be in snake case */

/**
 * @brief An example function.
 * 
 * @param a The first parameter.
 * @param b The second parameter.
 * @return The sum of a and b.
 */
int example_function(int a, int b);


#endif /* NAMING_CONVENTION_H */
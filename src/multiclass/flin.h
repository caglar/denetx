/*
 * =====================================================================================
 *
 *       Filename: flin.h
 *
 *    Description: Data types for Matrix and Vector data structures.
 *
 *        Version: 1.0
 *        Created: 06/23/2011 11:59:15 AM
 *       Revision: none
 *       Compiler: gcc
 *
 *         Author: Caglar Gulcehre(caglar@ulakbim.gov.tr),
 *        Company: TUBITAK ULAKBIM
 *
 * =====================================================================================
 */

#ifndef FLIN_H_
#define FLIN_H_

#include <boost/multi_array.hpp>

typedef boost::multi_array<float,2> Matrix_t;
typedef boost::multi_array<float, 1> Vector_t;

#endif

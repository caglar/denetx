/*
 * =====================================================================================
 *
 *       Filename:  arrayresize.h
 *
 *    Description:  Resize C++ arrays
 *
 *        Version:  1.0
 *        Created:  07/08/2011 11:51:04 AM
 *       Revision:  none
 *       Compiler:  gcc
 *
 *         Author:  Caglar Gulcehre (CGLR), 
 *        Company:  TUBITAK ULAKBIM
 *
 * =====================================================================================
 */
#ifndef ARRAYRESIZE_H
#define ARRAYRESIZE_H

template <class T>
T*
arrayResize(T arr[], const size_t oldSize, const size_t newSize, const T defaultVal);

#endif

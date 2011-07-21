/*
 * =====================================================================================
 *
 *       Filename:  arrayresize.cc
 *
 *    Description:  
 *
 *        Version:  1.0
 *        Created:  07/08/2011 11:52:24 AM
 *       Revision:  none
 *       Compiler:  gcc
 *
 *         Author:  YOUR NAME (), 
 *        Company:  
 *
 * =====================================================================================
 */

template <class T>
T*
arrayResize(T arr[], const size_t oldSize, const size_t newSize, const T defaultVal)
{
    T *temp = new T[newSize];
    unsigned int i;
    if(oldSize<newSize)
    {
        i=oldSize;
    }
    else
    {
        i=newSize;
    }
    while(i)
    {
        --i;
        temp[i]=arr[i];
    }
    delete[] arr;
    i=oldSize;
    while(i<newSize)
    {
        temp[i]=defaultVal;
        ++i;
    }
    arr = temp;
    return arr;
}


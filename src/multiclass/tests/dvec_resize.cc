/*
* =====================================================================================
*
*       Filename:  dvec_resize.cc
*
*    Description:  
*
*        Version:  1.0
*        Created:  07/19/2011 10:58:25 AM
*       Revision:  none
*       Compiler:  gcc
*
*         Author:  YOUR NAME (), 
*        Company:  
*
* =====================================================================================
*/

#include "../dvec.h"
#include <iostream>

using std::cout;
using std::endl;

int
main()
{
  DVec dvec;
  cout << " Size in the beginining " << dvec.size() << endl;
  dvec.resize(boost::extents[12]);
  dvec.resize(boost::extents[1]);
  cout << " DVEC size is: " << dvec.size() << endl;
  return 0;
}

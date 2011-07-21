/*
 * =====================================================================================
 *
 *       Filename:  parse_arfheaders.cc
 *
 *    Description:  
 *
 *        Version:  1.0
 *        Created:  07/19/2011 10:06:50 AM
 *       Revision:  none
 *       Compiler:  gcc
 *
 *         Author:  YOUR NAME (), 
 *        Company:  
 *
 * =====================================================================================
 */
#include "../../parse_arfheader.h"

int 
main()
{
  arfheader* arfHeader = parseARFXFile("../../../rna.arfx");
  return 0;
}

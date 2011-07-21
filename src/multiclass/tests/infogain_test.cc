#include <iostream>
#include "../infogain.h"
#include "../flin.h"

using std::cout;
using std::endl;

int main () 
{
  float preSplitTest[2];
  preSplitTest[0] = 0.12;
  preSplitTest[1] = 0.88;
  
  Matrix_t deneme(boost::extents[2][2]);
  deneme[0][0] = 0.0;
  deneme[0][1] = 1.0;
  deneme[1][0] = 0.13;
  deneme[1][1] = 0.87;

  float entropy = computeEntropy(preSplitTest);
  cout << entropy << endl;

  return 0;
}

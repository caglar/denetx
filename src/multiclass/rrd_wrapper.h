#ifndef RRD_WRAPPER
#define RRD_WRAPPER

#include <vector>
#include "/home/caglar/Codes/c++/myprojs/googleproj/denetx/src/parse_arfheader.h"

bool
create_rrd(const char *rrd_filepath, const std::vector<arfcategory> &categories, unsigned int step_size);

bool
update_rrd(const char *rrd_filepath, std::vector<int> &predictions);

#endif

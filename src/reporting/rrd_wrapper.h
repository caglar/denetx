#ifndef RRD_WRAPPER
#define RRD_WRAPPER

#include <vector>
#include "../parse_arfheader.h"

bool
create_rrd(const char *rrd_filepath, const std::vector<arfcategory> &categories);

bool
update_rrd(const char *rrd_filepath, const std::vector<int> &predictions);

#endif

#include <cstdio>
#include <cstdlib>

#include <vector>

#include "rrd_wrapper.h"
#include "../parse_arfheader.h"
#include <rrd.h>

const size_t MAX_PARAM_SIZE = 128;

using std::vector;

bool
create_rrd(const char *rrd_filepath, const vector<arfcategory> &categories)
{
  bool createFlag = true;
  size_t no_of_ds = categories.size();
  const int no_begin_params = 6;
  const int no_end_params = 8;
  const int argc = no_begin_params + no_end_params + no_of_ds;
  int rrd_ret;

  char **rrd_argv = (char **) malloc( sizeof(char *) * argc);
  const char *ds_begin = "DS:";
  const char *ds_end = ":COUNTER:600:U:U";
  const char *rrd_begin[no_begin_params] = {
    "create", rrd_filepath,
    "--step", "300",
    "--start", "0"
  };

  const char *rrd_end[no_end_params] = {
    "RRA:AVERAGE:0.5:1:600",
    "RRA:AVERAGE:0.5:6:700",
    "RRA:AVERAGE:0.5:24:775",
    "RRA:AVERAGE:0.5:288:797",
    "RRA:MAX:0.5:1:600",
    "RRA:MAX:0.5:6:700",
    "RRA:MAX:0.5:24:775",
    "RRA:MAX:0.5:288:797" 
  };

  char rrd_ds[no_of_ds][MAX_PARAM_SIZE];
  for (size_t i = 0; i < no_of_ds; i++) {
    char *cat_name = (char *) malloc(strlen(ds_begin) + categories[i].name.size() + strlen(ds_end) + 1);
    sprintf(cat_name, "%s%s%s", ds_begin, categories[i].name.c_str(), ds_end);
    if (strlen(cat_name) > MAX_PARAM_SIZE) {
      perror("Category name is larger than the MAX_SIZE");
      return false;
    }
    strncpy(rrd_ds[i], cat_name, strlen(cat_name));
    rrd_ds[i][strlen(cat_name)] = '\0';
  }

  for (size_t i = 0; i < argc; i++) {
    rrd_argv[i] = NULL;
    if (i < no_begin_params) {
      rrd_argv[i] = (char *) malloc(strlen(rrd_begin[i]) + 1);
      strcpy(rrd_argv[i], rrd_begin[i]);
    } else if (i < (no_begin_params + no_of_ds)) {
      rrd_argv[i] = (char *) malloc(strlen(rrd_ds[i - no_begin_params]) + 1);
      strcpy(rrd_argv[i], rrd_ds[i - no_begin_params]);
    } else {
      rrd_argv[i] = (char *) malloc(strlen(rrd_end[i - (no_begin_params + no_of_ds)]) + 1);
      strcpy(rrd_argv[i], rrd_end[i - (no_begin_params + no_of_ds)]);
    }
  }
  rrd_clear_error();
  rrd_ret = rrd_create(argc, rrd_argv);

  if (rrd_test_error() != 0) {
    printf("Couldn't create the RRD!\n ERROR: %s\n", rrd_get_error());
    createFlag = false;
  }
  return createFlag;
}

bool
update_rrd(const char *rrd_filepath, const vector<int> &predictions)
{
  bool updateflag = true;
  const int rrd_argc = 3;
  const int MaxValSize = 8;
  char *rrd_argv[rrd_argc] = {
    "update",
    (char *) rrd_filepath,
    new char[MAX_PARAM_SIZE]}; 

  int rrd_ret;
  strcpy(rrd_argv[2], "N");

  for (size_t i = 0; i < predictions.size(); i++) {
    char *val = (char *) malloc(MaxValSize); 
    sprintf(val, ":%d", predictions[i]);
    if ((strlen(val) + strlen(rrd_argv[2])) >= MAX_PARAM_SIZE) {
      perror("Illegal val size");
      return false;
    }
    strcat(rrd_argv[2], val);
  }

  rrd_clear_error();
  rrd_ret = rrd_update(rrd_argc, rrd_argv);

  if (rrd_test_error() != 0) {
    printf("Couldn't create the RRD!\n ERROR: %s\n", rrd_get_error());
    updateflag = false;
  }
  return updateflag;
}

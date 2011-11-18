/*
 Copyright (c) 2009 Yahoo! Inc.  All rights reserved.  The copyrights
 embodied in the content of this file are licensed under the BSD
 (revised) open source license
 */

#include "vw.h"
#include "gd.h"
#include "utils.h"

#include <stdlib.h>

int
main(int argc, char *argv[])
{
    srand48(100001);
    drand48();
    gd_vars *vars = vw(argc, argv);

    float weighted_labeled_examples = global.sd->weighted_examples
            - global.sd->weighted_unlabeled_examples;
    float best_constant = (global.sd->weighted_labels - global.initial_t)
            / weighted_labeled_examples;
    float constant_loss = (best_constant * (1.0 - best_constant) * (1.0
            - best_constant) + (1.0 - best_constant) * best_constant
            * best_constant);

    if (!global.quiet) {
        cerr.precision(4);
        cerr << endl << "finished run";
        cerr << endl << "number of examples = " << global.sd->example_number;
        cerr << endl << "weighted example sum = " << global.sd->weighted_examples;
        cerr << endl << "weighted label sum = " << global.sd->weighted_labels;
        cerr << endl << "average loss = " << global.sd->sum_loss
                / global.sd->weighted_examples;
        cerr << endl << "best constant = " << best_constant;
        if (global.sd->min_label == 0. && global.sd->max_label == 1. && best_constant
                < 1. && best_constant > 0.)
            cerr << endl << "best constant's loss = " << constant_loss;
        cerr << endl << "total feature number = " << global.sd->total_features;
        if (global.active_simulation)
            cerr << endl << "total queries = " << global.queries << endl;
        cerr << endl;
    }

    c_free(vars);
    return 0;
}

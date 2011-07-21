/*
Copyright (c) 2009 Yahoo! Inc.  All rights reserved.  The copyrights
embodied in the content of this file are licensed under the BSD
(revised) open source license
 */

#ifndef SPARSE_DENSE_VECTOR_H
#define SPARSE_DENSE_VECTOR_H

#include "parse_example.h"

float sd_add(weight* weights, size_t mask, feature* begin, feature* end);

void quadratic(v_array<feature> &f, const v_array<feature> &first_part, 
               const v_array<feature> &second_part, size_t thread_mask);

float one_of_quad_predict(v_array<feature> &page_features, feature& offer_feature, weight* weights, size_t mask);

float one_pf_quad_predict(weight* weights, feature& page_feature, v_array<feature> &offer_features, size_t mask);

float offset_quad_predict(weight* weights, feature& page_feature, v_array<feature> &offer_features, size_t mask, size_t offset);

void print_audit_quad(weight* weights, audit_data& page_feature, v_array<audit_data> &offer_features, size_t mask);
void print_quad(weight* weights, feature& page_feature, v_array<feature> &offer_features, size_t mask);

float single_quad_weight(weight* weights, feature& page_feature, feature* offer_feature, size_t mask);

#endif

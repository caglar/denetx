#!/bin/bash
#../src/denetX --naive_bayes --nb_model ../model --arfx_path ../rna.arfx -d ../test/datasets/rna/rnaVW 
#-t ../test/datasets/rna/rnaVW.test

#Test 1:
#Training on rna dataset
#../src/denetX --naive_bayes --nb_model ./models/rna.model --arfx_path ./arfx/rna.arfx -d \
#  ./datasets/rna/rnaVW #-t ./datasets/rna/rnaVW.test

#Test 2
#Testing on rna dataset
#../src/denetX --naive_bayes --nb_model ./models/rna.model --arfx_path ./arfx/rna.arfx \
#  -t ./datasets/rna/rnaVW.test

#Test 3
#Train on gtvs data set
#../src/denetX --naive_bayes --nb_model ./models/gtvs.model --arfx_path ./arfx/gtvs.arfx \
# -d  ./datasets/gtvs/gtvs.train

#Test 4
#Train on gtvs data set
#../src/denetX --naive_bayes --nb_model ./models/gtvs.model --arfx_path ./arfx/gtvs.arfx \
# -t ./datasets/gtvs/gtvs.test
#Test 5
#Create and update RRD file
../src/denetX --rrd /home/caglar/Codes/c++/myprojs/googleproj/denetx/test/rrds/test.rrd --step 2 \
  --naive_bayes --nb_model ./models/gtvs.model --arfx_path ./arfx/gtvs2.arfx  -t \
  ./datasets/gtvs/gtvs.test --thread_bits 2

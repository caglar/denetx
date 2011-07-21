/*
 * dt.h
 *
 *  Created on: Apr 1, 2011
 *      Author: caglar
 */

#ifndef HT_H_
#define HT_H_

#include <algorithm>
#include <vector>

#include "dvec.h"
#include "attrsplitsuggest.h"
#include "hoeffding_bound.h"
#include "arrayresize.h"

//Temprorily don't use this:
#include "../timsort.h"
//

#include "../utils.h"
#include "../example.h"
#include "../parse_arfheader.h"

using std::vector;
using std::stable_sort;

enum NodeType {
    FNODE,
    HNODE,
    ILNODE,
    ALNODE,
    SNODE
};

const int GRACE_PERIOD = 200;
const float SPLIT_CONFIDENCE = 0.0000001;
const float TIE_THRESHOLD = 0.05;
const bool REMOVE_POOR_ATTR = true;

class SplitNode;
class FoundNode;

class Node {
  protected:
      float* observedClassDist;
      int noOfObservations;

  public:
      Node (float* classObservations, int numOfObservations)
      {
          this->noOfObservations = numOfObservations;
          observedClassDist = classObservations;
      }

      ~Node()
      {
          delete observedClassDist;
      }

      bool
          isLeaf()
          {
              return true;
          }

      float*
          getObservedClassDist()
          {
          //    float observedClassesDist[noOfObservations];
           //   copy_dvec_to_array(observedClassDist, observedClassesDist);
              return observedClassDist;
          }

      int
          getNoOfObservations(){
              return this->noOfObservations;
          }

      float*
          getClassVotes (example *ex, int noOfClasses)
          {
              if (noOfClasses != noOfObservations){
                  const size_t oldSize = noOfObservations;
                  const size_t newSize = noOfClasses;
                  float defaultVal = 0.0;
                  observedClassDist = arrayResize(observedClassDist, oldSize, newSize, defaultVal);
              }
              return getObservedClassDist();
          }

      int
         get_num_non_zero (float * vec) {
            int numNonZero  = 0;
            for (auto i = 0; i < noOfObservations; i++) {
                if (vec[i] != 0)
                {
                    numNonZero++;
                }
            }
            return numNonZero;
         }

      bool
          observedClassDistributionIsPure()
          {
              return (get_num_non_zero(observedClassDist) < 2);
          }

      virtual NodeType
          getType()
          {
              return HNODE;
          }

      virtual FoundNode*
          filterInstanceToLeaf(feature *f,
                               SplitNode *parent, int parentBranch) { return NULL; }

};

class FoundNode
{
  public:
      Node* hNode;
      SplitNode* sParent;
      int parentBranch;

      FoundNode()
      {
          hNode = NULL;
          sParent = NULL;
      }

      FoundNode (Node* node, SplitNode* sparent, int pBranch): hNode(node), sParent(sparent), parentBranch(pBranch)
    {
    }

      int
          getType()
          {
              return FNODE;
          }
};


class SplitNode : public Node
{
  protected:
      SplitTest* splitTest;
      vector <Node *>children;

  public:
      SplitNode (SplitTest* sTest, float* classObservations, int noOfObservations):
          Node(classObservations, noOfObservations)
    {
        this->splitTest = sTest;
    }

      int
          numChildren()
          {
              return children.size();
          }

      void
          setChild(int index, Node* child)
          {
              children[index] = child;
          }

      Node*
          getChild(int index)
          {
              return children[index];
          }

      int
          instanceChildIndex (feature *f)
          {
              return splitTest->branchForInstance(f);
          }

      bool
          isLeaf()
          {
              return false;
          }

      NodeType
          getType()
          {
              return SNODE;
          }

      FoundNode*
          filterInstanceToLeaf(feature *f,
                               SplitNode *parent, int parentBranch)
          {
              int childIndex = instanceChildIndex(f);
              if (childIndex >= 0)
              {
                  Node *child = getChild(childIndex);
                  if (child != NULL)
                  {
                      return child->filterInstanceToLeaf(f, this, childIndex);
                  }
                  return new FoundNode(NULL, this, childIndex);
              }
              return new FoundNode(this, parent, parentBranch);
          }
};

class HNode: public Node
{
  public:
      HNode (float *classObservations, int numOfObservations) : Node(classObservations, numOfObservations)
    {
    }

      FoundNode*
          filterInstanceToLeaf(SplitNode* parent, int parentBranch)
          {
              return new FoundNode(this, parent, parentBranch);
          }
};

class HoeffdingTree;

class LearningNode : public HNode
{
  private:
      arfheader *arfHeader;
      bool ActiveLearningState;
      float WeightAtLastSplitEval;
      vector <AttributeClassObserver *> AttributeObservers;
      bool useNBPrediction;

      float mCorrectWeight;
      float nbCorrectWeight;

      void
          initAdaptiveWeights ()
          {
          }
  public:
      LearningNode (float* initClassObservations, int noOfObservations, arfheader *arfHead, bool learningState) :
          HNode(initClassObservations, noOfObservations), arfHeader(arfHead), ActiveLearningState(learningState)
    {
        mCorrectWeight = 0.0;
        nbCorrectWeight = 0.0;
        useNBPrediction = false;
    }

      void
          learnFromInstance (example *ex, HoeffdingTree* ht, size_t thread_num)
          {
              label_data *ld = (label_data *) (ex->ld);
              int classVal = static_cast<int>(ld->label);
              float weight = ld->weight;
              observedClassDist[classVal] += weight;
//              add_to_val(classVal, observedClassDist, weight);
              if (ActiveLearningState) {
                  for (auto i = *(ex->indices.begin); i != *(ex->indices.end); i++) {
                      AttributeClassObserver *attrCObserver = AttributeObservers[i];
                      feature f = ex->atomics[i][thread_num];
                      if (attrCObserver == NULL) {
                          fType type = arfHeader->features[f.weight_index].type;
                          attrCObserver = ((type == NUMERIC) ? (AttributeClassObserver *) new NumAttrObserver(static_cast<unsigned int>(noOfObservations)) : (AttributeClassObserver *) new NomAttrObserver());
                          AttributeObservers[i] = attrCObserver;
                      }
                      attrCObserver->observeAttributeClass(f.x, classVal, weight);
                  }
              }
          }

      void
          setUseNBPrediction(bool nbPred)
          {
              this->useNBPrediction = nbPred;
          }

      float
          getWeightSeen()
          {
              float sumOfVals = 0.0;
              for (auto i = 0; i < noOfObservations; i++) {
                sumOfVals += observedClassDist[i];
              }
              return sumOfVals;
          }

      float
          getWeightSeenAtLastSplit()
          {
              return WeightAtLastSplitEval;
          }

      void
          setWeightSeenAtLastSplitEval(float weight)
          {
              WeightAtLastSplitEval = weight;
          }

      void
          disableAttribute(int attIndex)
          {
              AttributeObservers[attIndex] = NULL;
          }

      NodeType
          getType()
          {
              if (ActiveLearningState) {
                  return ALNODE;
              }
              return ILNODE;
          }


      vector <AttrSplitSuggestion *>
          getBestSplitSuggestions()
          {
              vector<AttrSplitSuggestion *> bestSuggestions;
              //float *preSplitDist = new float[noOfObservations];
              //copy_dvec_to_array(observedClassDist, preSplitDist);
              for (auto i = 0; i<AttributeObservers.size(); i++) {
                  AttributeClassObserver *attrCObs = AttributeObservers[i];
                  if (attrCObs != NULL) {
                      AttrSplitSuggestion* attrSpSugg = (attrCObs)->getBestEvaluatedSplitSuggestion(observedClassDist, noOfObservations);
                      if (attrSpSugg != NULL) {
                          bestSuggestions.push_back(attrSpSugg);
                      }
                  }
              }
              return bestSuggestions;
          }
};

class HoeffdingTree
{
  protected:
      HNode *treeRoot;
      int decisionNodeCount;
      int activeLeafNodeCount;
      int inactiveLeafCount;
      arfheader *arfHeader;

      void
          attemptToSplit(LearningNode *lNode, SplitNode *sNode, int parentIndex)
          {
              //Continue until the class distribution is pure
              //Purity based Criteria
              if (lNode != NULL && !(lNode->observedClassDistributionIsPure())) {
                  vector<AttrSplitSuggestion*> bestSplitSuggestions = lNode->getBestSplitSuggestions();
                  size_t bss_size = bestSplitSuggestions.size();
                  //tim_sort(bestSplitSuggestions, bss_size);

                  stable_sort(bestSplitSuggestions.begin(), bestSplitSuggestions.end());
                  bool shouldSplit = false;
                  if (bss_size < 2) {
                      shouldSplit = (bss_size > 0);
                  } else {
                      float rangeOfMerit = getRangeOfMerit(lNode->getNoOfObservations());
                      float hoeffdingBound = computeHoeffdingBound(rangeOfMerit, SPLIT_CONFIDENCE, lNode->getWeightSeen());
                      AttrSplitSuggestion* bestSuggestion = bestSplitSuggestions[bss_size - 1];
                      AttrSplitSuggestion* secondBestSuggestion = bestSplitSuggestions[bss_size -2];
                      if (((bestSuggestion->merit - secondBestSuggestion->merit) > hoeffdingBound) || (hoeffdingBound < TIE_THRESHOLD)) {
                          shouldSplit = true;
                      }
                      if (shouldSplit) {
                          AttrSplitSuggestion* splitDecision = bestSplitSuggestions[bss_size - 1];
                          if (splitDecision) {
                              deactivateLearningNode(lNode, sNode, parentIndex);
                          } else {
                              SplitNode *newSplit = new SplitNode(splitDecision->splitTest, lNode->getObservedClassDist(), lNode->getNoOfObservations());

                              for (int i = 0; i < (splitDecision->numSplits); i++) {
                                  bool enableActiveLearning = true;
                                  int classObsSize = (splitDecision->resultingClassDistribution[i]).size();
                                  float *classObservations = new float[(splitDecision->resultingClassDistribution[i]).size()];
                                  copy_dvec_to_array(splitDecision->resultingClassDistribution[i], classObservations);
                                  LearningNode *newChild = new LearningNode(classObservations, classObsSize, this->arfHeader, enableActiveLearning);
                                  newSplit->setChild(i, newChild);
                              }

                              this->activeLeafNodeCount--;
                              this->decisionNodeCount++;
                              this->activeLeafNodeCount += splitDecision->numSplits;

                              if (sNode == NULL) {
                                  this->treeRoot = dynamic_cast<HNode *>(newSplit);
                              } else {
                                  sNode->setChild(parentIndex, newSplit);
                              }
                          }
                      }
                  }
              }
          }

      void
          deactivateLearningNode(LearningNode *toDeactivate, SplitNode *parent, int parentBranch)
          {
              LearningNode *newLeaf = new LearningNode(toDeactivate->getObservedClassDist(), toDeactivate->getNoOfObservations(), this->arfHeader, false);

              if (parent == NULL) {
                  this->treeRoot = newLeaf;
              } else {
                  parent->setChild(parentBranch, newLeaf);
              }
              this->activeLeafNodeCount--;
              this->inactiveLeafCount++;
          }

  public:
      HoeffdingTree (arfheader *arfHeader)
      {
          this->arfHeader = arfHeader;
      }

      void
          trainOnInstance(example *ex, size_t thread_num)
          {
              if (this->treeRoot == NULL) {
                  this->treeRoot = new LearningNode(new float[0], 0, arfHeader, true);
                  activeLeafNodeCount = 1;
              }

              this->treeRoot = dynamic_cast<HNode *>(this->treeRoot);

              FoundNode *fNode = treeRoot->filterInstanceToLeaf(NULL, -1);
              Node *leafNode = dynamic_cast<Node *>(fNode->hNode);

              if (leafNode == NULL) {
                  leafNode = new LearningNode(new float[0], 0, arfHeader, true);
                  fNode->sParent->setChild(fNode->parentBranch, leafNode);
                  activeLeafNodeCount++;
              }

              NodeType nType = leafNode->getType();
              if (nType == ALNODE || nType == ILNODE) {
                  LearningNode *learningNode = dynamic_cast<LearningNode *>(leafNode);
                  learningNode->learnFromInstance(ex, this, thread_num);
                  if (nType == ALNODE) {
                      float weightSeen = learningNode->getWeightSeen();
                      if (weightSeen - learningNode->getWeightSeenAtLastSplit()
                          >= GRACE_PERIOD) {
                          attemptToSplit(learningNode, fNode->sParent, fNode->parentBranch);
                          learningNode->setWeightSeenAtLastSplitEval(weightSeen);
                      }
                  }
              }
          }

      float *
          getClassVotesForInstance(example *ex){
              if (treeRoot != NULL) {
                  FoundNode *fNode = treeRoot->filterInstanceToLeaf(NULL, -1);
                  Node *leafNode = dynamic_cast<Node *>(fNode->hNode);
                  if (leafNode == NULL) {
                      leafNode = fNode->sParent;
                  }

                  return leafNode->getClassVotes(ex, arfHeader->no_of_categories);
              }
              return new float[0];
          }

      void
          resetLearning()
          {
              treeRoot = NULL;
              decisionNodeCount = 0;
              activeLeafNodeCount = 0;
              inactiveLeafCount = 0;
          }
};

#endif /* HT_H_ */

#ifndef __D_T_DBOW2__
#define __D_T_DBOW2__

#include "TemplatedVocabulary.h"
#include "TemplatedDatabase.h"
#include "BowVector.h"
#include "FeatureVector.h"
#include "QueryResults.h"
#include "FBrief.h"
#include "FORB.h"

/// ORB Vocabulary
typedef kw_DBoW2::TemplatedVocabulary<kw_DBoW2::FORB::TDescriptor, kw_DBoW2::FORB> 
  OrbVocabulary;

/// FORB Database
typedef kw_DBoW2::TemplatedDatabase<kw_DBoW2::FORB::TDescriptor, kw_DBoW2::FORB> 
  OrbDatabase;
  
/// BRIEF Vocabulary
typedef kw_DBoW2::TemplatedVocabulary<kw_DBoW2::FBrief::TDescriptor, kw_DBoW2::FBrief> 
  BriefVocabulary;

/// BRIEF Database
typedef kw_DBoW2::TemplatedDatabase<kw_DBoW2::FBrief::TDescriptor, kw_DBoW2::FBrief> 
  BriefDatabase;

#endif


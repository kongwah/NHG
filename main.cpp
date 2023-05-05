#include <iostream>
#include <vector>

#include <sys/types.h>
#include <sys/stat.h>
#include <unistd.h>


#include "IS.h" // defines OrbVocabulary and OrbDatabase

// OpenCV
#include <opencv2/core.hpp>
#include <opencv2/highgui.hpp>
#include <opencv2/features2d.hpp>


using namespace kw_DBoW2;
using namespace std;

void loadFeatures(vector<vector<cv::Mat > > &features, char *dir); //KW
void load_testFeatures(vector<vector<cv::Mat > > &features, char *dir); //KW
void changeStructure(const cv::Mat &plain, vector<cv::Mat> &out);
void testVocCreation(const vector<vector<cv::Mat > > &features);
void testDatabase(const vector<vector<cv::Mat > > &features);


// number of training images
int NIMAGES = 2000;
int NTrain = 0;
int NTest  = 0;

char TRAIN_DIR[100];
char TEST_DIR[100];


// - - - - - - - - - - - - - - - - - - - - - - - - - - - - - - - - - 

void wait()
{
  cout << endl << "Press enter to continue" << endl;
  getchar();
}


int file_exist (char *filename)
{
  struct stat   buffer;   
  return (stat (filename, &buffer) == 0);
}

// ----------------------------------------------------------------------------
// To run, type
// ./demo.d train test 1    (if you want to create a new vocabulary "small_voc.yml.gz")
// ./demo.d train test      (if you want to reuse the vocabulary "small_voc.yml.gz")
//

/*
g++ -I.\
    -o demo.d \
     main.cpp \
     src/libIS.o \
     /usr/local/lib/libopencv_core.so.3.3.0 \
     /usr/local/lib/libopencv_features2d.so.3.3.0 \
     /usr/local/lib/libopencv_imgcodecs.so.3.3.0


setenv LD_LIBRARY_PATH .:/usr/local/lib:/usr/lib/x86_64-linux-gnu:/usr/lib

*/

int main(int argc, char *argv[])
{
  vector<vector<cv::Mat > > features;

  bool make_new_vocab=true;
  strcpy((char*)TRAIN_DIR,argv[1]);
  strcpy((char*)TEST_DIR,argv[2]);
  if(argc==3) make_new_vocab=false;

  loadFeatures(features, (char*) TRAIN_DIR);
  if(make_new_vocab==true) {
     testVocCreation(features);
  }
  testDatabase(features);

  return 0;
}


//KW
void load_testFeatures(vector<vector<cv::Mat > > &features, char dir[100])
{
  features.clear();
  features.reserve(NIMAGES);

  cv::Ptr<cv::ORB> orb = cv::ORB::create();

  cerr << "Extracting ORB features for testing..." << endl;
  int i = 1;
  while (1)
  {
    char ss[100];
    sprintf(ss,"%s/%d.jpg",dir,i);
    if ((i%10)==0) { fprintf(stderr,"%s   \r",ss); fflush(stdout); }

    if(file_exist(ss)) {
      cv::Mat image = cv::imread(ss, 0);
  
      cv::Mat mask;
      vector<cv::KeyPoint> keypoints;
      cv::Mat descriptors;

      orb->detectAndCompute(image, mask, keypoints, descriptors);

      features.push_back(vector<cv::Mat >());
      changeStructure(descriptors, features.back());

      i++;
    }
    else break;
  }
  NTest = i-1;
  fprintf(stderr,"\n%d Test images\n",NTest);
}
//KW


// ----------------------------------------------------------------------------

void loadFeatures(vector<vector<cv::Mat > > &features, char dir[100])
{
  features.clear();
  features.reserve(NIMAGES);

  cv::Ptr<cv::ORB> orb = cv::ORB::create();

  cerr << "Extracting ORB features..." << endl;
  int i = 1;
  while (1)
  {
    char ss[100];
    sprintf(ss,"%s/%d.jpg",dir,i);
    if ((i%10)==0) { fprintf(stderr,"%s   \r",ss); fflush(stdout); }

    if(file_exist(ss)) {
      cv::Mat image = cv::imread(ss, 0);

      cv::Mat mask;
      vector<cv::KeyPoint> keypoints;
      cv::Mat descriptors;

      orb->detectAndCompute(image, mask, keypoints, descriptors);

      features.push_back(vector<cv::Mat >());
      changeStructure(descriptors, features.back());

      i++;
    }
    else break;
  }
  NTrain = i-1;
  fprintf(stderr,"\n%d Train images\n",NTrain);
}

// ----------------------------------------------------------------------------

void changeStructure(const cv::Mat &plain, vector<cv::Mat> &out)
{
  out.resize(plain.rows);

  for(int i = 0; i < plain.rows; ++i)
  {
    out[i] = plain.row(i);
  }
}

// ----------------------------------------------------------------------------

void testVocCreation(const vector<vector<cv::Mat > > &features)
{
  // branching factor and depth levels 
  const int k = 15;
  const int L = 3;
  const WeightingType weight = TF_IDF;
  const ScoringType score = L1_NORM;

  OrbVocabulary voc(k, L, weight, score);

  cerr << "Creating a small " << k << "^" << L << " vocabulary..." << endl;
  voc.create(features);
  cerr << "... done!" << endl;

  cerr << "Vocabulary information: " << endl << voc << endl << endl;

  /**
  // lets do something with this vocabulary
  cerr << "Matching images against themselves (0 low, 1 high): " << endl;
  BowVector v1, v2;
  for(int i = 0; i < NIMAGES; i++)
  {
    voc.transform(features[i], v1);
    for(int j = 0; j < NIMAGES; j++)
    {
      voc.transform(features[j], v2);
      
      double score = voc.score(v1, v2);
      cerr << "Image " << i << " vs Image " << j << ": " << score << endl;
    }
  }
  **/

  // save the vocabulary to disk
  cerr << endl << "Saving vocabulary..." << endl;
  voc.save("small_voc.yml.gz");
  cerr << "to 'small_voc.yml.g' Done" << endl;
}

// ----------------------------------------------------------------------------

void testDatabase(const vector<vector<cv::Mat > > &features)
{
  cerr << "Creating a small database..." << endl;

  // load the vocabulary from disk
  OrbVocabulary voc("small_voc.yml.gz");
  
  OrbDatabase db(voc, false, 0); // false = do not use direct index
  // (so ignore the last param)
  // The direct index is useful if we want to retrieve the features that 
  // belong to some vocabulary node.
  // db creates a copy of the vocabulary, we may get rid of "voc" now


  // add images to the database
  for(int i = 0; i < NTrain; i++)
  {
    fprintf(stderr,"adding %d\r",i);
    db.add(features[i]);
  }

  cerr << "... done!" << endl;
  cerr << "Database information: " << endl << db << endl;

  // Load test images and compute features
  //
  cerr << "adding test features..." << endl;
  vector<vector<cv::Mat > > test_features;
  load_testFeatures(test_features, (char*)TEST_DIR);

  /**
  // add to the database
  for(int i = 0; i < NTest; i++)
  {
    db.add(test_features[i]);
  }
  cerr << "... done!" << endl;
  cerr << "Database information: " << endl << db << endl;
  **/


  // Query the database now
  cerr << "Querying the database: " << endl;





/**
  fprintf(fp,"<html>\n");
  fprintf(fp,"<style>\n");
  fprintf(fp,".borderR { border-width: 9px; border-color: #FF0000; border-style: solid; }\n");
  fprintf(fp,".borderG { border-width: 9px; border-color: #00FF00; border-style: solid; }\n");
  fprintf(fp,".borderB { border-width: 9px; border-color: #0000FF; border-style: solid; }\n");
  fprintf(fp,".containerR { position: relative; text-align: center; color: #FF0000; }\n");
  fprintf(fp,".containerG { position: relative; text-align: center; color: #00FF00; }\n");
  fprintf(fp,".containerB { position: relative; text-align: center; color: #0000FF; }\n");
  fprintf(fp,".containerY { position: relative; text-align: center; color: #FFFF00; }\n");
  fprintf(fp,".top-right { position: absolute; top: 8px; right: 16px; background-color: #FF0000; }\n");
  fprintf(fp,".middle-right { position: absolute; bottom: 30px; right: 16px; color: #FFFFFF; background-color: #0000FF; }\n");
  fprintf(fp,".bottom-right { position: absolute; bottom:  8px; right: 16px; color: #FFFFFF; background-color: #0000FF; }\n");
  fprintf(fp,"</style>\n");

  fprintf(fp,"<div class=containerY>\n");
    fprintf(fp,"<div class=top-right>%f</div>\n",ret[0].Score);
  fprintf(fp,"</div>\n");
**/




#define HTML_OUTPUT

  QueryResults ret;
#ifdef HTML_OUTPUT
  printf("<html>\n");
  printf("<style>\n");
  printf(".containerY { position: relative; text-align: center; color: #FFFF00; }\n");
  printf(".top-right { position: absolute; top: 8px; right: 16px; background-color: #FF0000; }\n");
  printf("</style>\n");
  printf("<table>\n");
#endif
  for(int i = 0; i < NTest; i+=2) //KW test every KK images
  {
#ifdef HTML_OUTPUT
    printf("<tr>\n");
#endif
    db.query(test_features[i], ret, 4);  // max_results = 3rd param (4)
    //cerr << "Searching for Image " << i << "  " << ret << endl;
    //printf("i=%d top=%d %f %d %f\n",i,ret[0].Id,ret[0].Score,ret[1].Id,ret[1].Score);
#ifdef HTML_OUTPUT
    printf("<td>\n");
    printf("<img src=%s/%d.jpg width=200>\n",TEST_DIR,i+1);
    printf("</td>\n");
    printf("<td><img src=arrow.jpg width=40></td>\n");
    printf("<td>\n");
    printf("<div class=containerY>\n");
    printf("<img src=%s/%d.jpg title=%f width=200>\n",TRAIN_DIR,ret[0].Id+1,ret[0].Score);
    printf("<div class=top-right>%f</div>\n",ret[0].Score);
    printf("</div>\n");
    printf("</td>\n");
    printf("<td>\n");
    printf("<div class=containerY>\n");
    printf("<img src=%s/%d.jpg title=%f width=200>\n",TRAIN_DIR,ret[1].Id+1,ret[1].Score);
    printf("<div class=top-right>%f</div>\n",ret[1].Score);
    printf("</div>\n");
    printf("</td>\n");
    printf("<td>\n");
    printf("<div class=containerY>\n");
    printf("<img src=%s/%d.jpg title=%f width=200>\n",TRAIN_DIR,ret[2].Id+1,ret[2].Score);
    printf("<div class=top-right>%f</div>\n",ret[2].Score);
    printf("</div>\n");
    printf("</td>\n");
    printf("</tr>\n");
#endif
  }
#ifdef HTML_OUTPUT
  printf("</table>\n");
  printf("</html>\n");
#endif

  cerr << endl;

  // we can save the database. The created file includes the vocabulary and the entries added
  //
  cerr << "Saving database to small_db.yml.gz..." << endl;
  db.save("small_db.yml.gz");
  cerr << "... done!" << endl;
  
  // once saved, we can load it again  
  cerr << "Retrieving database small_db.yml.gz once again..." << endl;
  OrbDatabase db2("small_db.yml.gz");
  cerr << "... done! This is: " << endl << db2 << endl;
}

// ----------------------------------------------------------------------------



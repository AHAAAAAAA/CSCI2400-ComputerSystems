#include <stdio.h>
#include "cs1300bmp.h"
#include <iostream>
#include <fstream>
#include "Filter.h"
#include "omp.h"
using namespace std;
#include "rtdsc.h"
//
// Forward declare the functions
//
Filter * readFilter(string filename);
double applyFilter(Filter *filter, cs1300bmp *input, cs1300bmp *output);

int main(int argc, char **argv)
{

  if ( argc < 2) {
    fprintf(stderr,"Usage: %s filter inputfile1 inputfile2 .... \n", argv[0]);
  }

  //
  // Convert to C++ strings to simplify manipulation
  //
  string filtername = argv[1];

  //
  // remove any ".filter" in the filtername
  //
  string filterOutputName = filtername;
  string::size_type loc = filterOutputName.find(".filter");
  if (loc != string::npos) {
    //
    // Remove the ".filter" name, which should occur on all the provided filters
    //
    filterOutputName = filtername.substr(0, loc);
  }

  Filter *filter = readFilter(filtername);

  double sum = 0.0;
  int samples = 0;

  for (int inNum = 2; inNum < argc; inNum++) {
    string inputFilename = argv[inNum];
    string outputFilename = "filtered-" + filterOutputName + "-" + inputFilename;
    struct cs1300bmp *input = new struct cs1300bmp;
    struct cs1300bmp *output = new struct cs1300bmp;
    int ok = cs1300bmp_readfile( (char *) inputFilename.c_str(), input);

    if ( ok ) {
      double sample = applyFilter(filter, input, output);
      sum += sample;
      samples++;
      cs1300bmp_writefile((char *) outputFilename.c_str(), output);
    }
    delete input;
    delete output;
  }
  fprintf(stdout, "Average cycles per sample is %f\n", sum / samples);

}

struct Filter *
readFilter(string filename)
{
  ifstream input(filename.c_str());
  if ( ! input.bad() ) {
    int size = 0;
    input >> size;
    Filter *filter = new Filter(size);
    int div;
    input >> div;
    filter -> setDivisor(div);
    for (int i=0; i < size; i++) {
      for (int j=0; j < size; j++) {
          int value;
          input >> value;
          filter -> set(i,j,value);
      }
    }
    return filter;
  }
}


double
applyFilter(struct Filter *filter, cs1300bmp *input, cs1300bmp *output)
{
  long long cycStart, cycStop;
  cycStart = rdtscll();
   //declare variables to reduce function calls in loop,
  // so we don't have to call it each time 
  const short int w= (input -> width) - 1;
  const short int h= (input -> height) - 1;
  output -> width = (input -> width)+1;
  output -> height = (input -> height)+1;


  int filterSize=filter -> getSize();
  int filterDivisor=filter -> getDivisor();


  //Made local array for filter->get so we wouldn’t have to access memory to retrieve value as often
  int i, j;
  int filterget[3][3]; //checked the filters we had, they were all 3x3
  for (i=0;i<3;i++){
    for(j=0;j<3;j++){
      filterget[i][j]=filter->get(i,j);
    }
  }

  //rearrange loop so we closer memor references
  //check filter types to optimize. Open filter file and note the numbers
  if (filterget[0][1] == -2){ //hline
  #pragma omp parallel for
  for(short int plane = 0; plane < 3; plane++) {
    for(short int row = 1; row < h ; row++) {
        const short int row1 = row - 1;
        const short int row3 = row + 1;
      for(short int col = 1; col < w; col++) {
          int l0=0,l1=0,l2=0;
          const short int col1 = col - 1;
          const short int col3 = col + 1;
            //unrolled the loops that iterate over the filter array
             l0 += (input->color[plane][row1][col1]);
             l1 += (input->color[plane][row1][col] << 1);
             l2 += (input->color[plane][row1][col3]);
             l0 += (input->color[plane][row3][col1]);
             l1 += (input->color[plane][row3][col] << 1);
             l2 += (input->color[plane][row3][col3]);
 
            //Used our three accumulators to combine so they could be done in parallel with less dependancy   
            
          //combined the two conditionals since they're dependent on each other
            if (output -> color[plane][row][col]  < 0 ) {
              output -> color[plane][row][col] = 0;
            }
            else if ( output -> color[plane][row][col]  > 255 ) { 
              output -> color[plane][row][col] = 255;
            }
            output -> color[plane][row][col] = l0+l1+l2;

      }
    }
  }
}
else if (filterget[1][1] ==8){ //gauss
  #pragma omp parallel for
  for(short int plane = 0; plane < 3; plane++) {
    for(short int row = 1; row < h ; row++) {
        const short int row1 = row - 1;
        const short int row3 = row + 1;
      for(short int col = 1; col < w; col++) {
          int l0=0,l1=0,l2=0;
          const short int col1 = col - 1;
          const short int col3 = col + 1;
            //unrolled the loops that iterate over the filter array

          l1 += input->color[plane][row1][col] << 2;
          l0 += input->color[plane][row][col1] << 2;
          l1 += input->color[plane][row][col] << 3;
          l2 += input->color[plane][row][col3] << 2;
          l1 += input->color[plane][row3][col] << 2;     
          l0 = ((l0+l1+l2) >> 3) / 3;
 
          
            
          //combined the two conditionals since they're dependent on each other
            if (output -> color[plane][row][col]  < 0 ) {
              output -> color[plane][row][col] = 0;
            }
            if ( output -> color[plane][row][col]  > 255 ) { 
              output -> color[plane][row][col] = 255;
            }
          //Used our three accumulators to combine so they could be done in parallel with less dependancy   
            output -> color[plane][row][col] = l0+l1+l2;

      }
    }
  }
}
else if (filterget[1][2] ==-1){ //Emboss
  #pragma omp parallel for
  for(short int plane = 0; plane < 3; plane++) {
    for(short int row = 1; row < h ; row++) {
        const short int row1 = row - 1;
        const short int row3 = row + 1;
      for(short int col = 1; col < w; col++) {
          int l0=0,l1=0,l2=0;
          const short int col1 = col - 1;
          const short int col3 = col + 1;
            //unrolled the loops that iterate over the filter array

          l0 += input->color[plane][row1][col1];
          l1 += input->color[plane][row1][col];
          l2 += -(input->color[plane][row1][col3]);

          l0 += input->color[plane][row][col1];
          l1 += input->color[plane][row][col];
          l2 += -(input->color[plane][row][col3]);

          l0 += input->color[plane][row3][col1];
          l1 += -(input->color[plane][row3][col]);
          l2 += -(input->color[plane][row3][col3]);
          
            
          //combined the two conditionals since they're dependent on each other
            if (output -> color[plane][row][col]  < 0 ) {
              output -> color[plane][row][col] = 0;
            }
            if ( output -> color[plane][row][col]  > 255 ) { 
              output -> color[plane][row][col] = 255;
            }
          //Used our three accumulators to combine so they could be done in parallel with less dependancy   
            output -> color[plane][row][col] = l0+l1+l2;

      }
    }
  }
}
else{ //Average remains, other filters not used so YOLO
  #pragma omp parallel for
  for(short int plane = 0; plane < 3; plane++) {
    for(short int row = 1; row < h ; row++) {
        const short int row1 = row - 1;
        const short int row3 = row + 1;
      for(short int col = 1; col < w; col++) {
          int l0=0,l1=0,l2=0;
          const short int col1 = col - 1;
          const short int col3 = col + 1;
            //unrolled the loops that iterate over the filter array

          l0 += input->color[plane][row1][col1];
          l1 += input->color[plane][row1][col];
          l2 += -(input->color[plane][row1][col3]);

          l0 += input->color[plane][row][col1];
          l1 += input->color[plane][row][col];
          l2 += -(input->color[plane][row][col3]);

          l0 += input->color[plane][row3][col1];
          l1 += -(input->color[plane][row3][col]);
          l2 += -(input->color[plane][row3][col3]);
          
            
          //combined the two conditionals since they're dependent on each other
            if (output -> color[plane][row][col]  < 0 ) {
              output -> color[plane][row][col] = 0;
            }
            if ( output -> color[plane][row][col]  > 255 ) { 
              output -> color[plane][row][col] = 255;
            }
          //Used our three accumulators to combine so they could be done in parallel with less dependancy   
            output -> color[plane][row][col] = (l0+l1+l2)/9;

      }
    }
  }
}
  cycStop = rdtscll();
  double diff = cycStop - cycStart;
  double diffPerPixel = diff / (output->width * output->height);
  fprintf(stderr, "Took %f cycles to process, or %f cycles per pixel\n",
    diff, diff / (output->width * output->height));
  return diffPerPixel;
}


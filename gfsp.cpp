// gfsp - Gabor Filter Sig Pyramid
// gabor filters 0 45 90 135 over pyramid



#include "opencv2/imgproc/imgproc.hpp"
#include "opencv2/highgui/highgui.hpp"
#include <math.h>
#include <stdlib.h>
#include <stdio.h>

using namespace cv;
using namespace std;

/// Global variables
Mat src, dst, tmp, tmp1, tmp2;
Mat ch[3];

// pyr means:
// My pyramid
//          6 layers ( each layer is a scale)
//          7 features
//               
// pyr[i][j]
// index i corresponds to layer
// index j corresponds to features as fallow:

#define GRAY 2
#define RED 3
#define GREEN 4
#define BLUE 5
#define YELLOW 6


std::vector <std::vector <Mat> > pyr(6, std::vector <Mat> (7));

Mat results;
Mat gabor_output;

//char* window_name = "Pyramids Demo";



// function to exctract regionals averages

int regionals_averages(Mat res) {
  for (int ri = 0; ri < res.cols; ri += res.cols/3) {
    for (int rj = 0; rj < res.rows; rj += res.rows/3) {
//      for (int i = ri; i < r
    }
  } //


}

/**
 * @function main
 */
int main( int argc, char** argv )
{
  /// General instructions
  printf( "\n Zoom In-Out demo  \n " );
  printf( "------------------ \n" );
  printf( " * [u] -> Zoom in  \n" );
  printf( " * [d] -> Zoom out \n" );
  printf( " * [ESC] -> Close program \n \n" );

  /// Test image - Make sure it s divisible by 2^{n}
//  src = imread( "./front-front/l00039.png", CV_LOAD_IMAGE_GRAYSCALE );
  src = imread( "./front-front/l00039.png", CV_LOAD_IMAGE_COLOR );
  if( !src.data )
    { printf(" No data! -- Exiting the program \n");
      return -1; }

  tmp = src;
  dst = tmp;

  printf( "cols = %i, rows = %i\n",src.cols,src.rows);
  // LAYER ZERO
  pyr[0][0] = src;
  pyr[0][1] = src;
  cvtColor (src, pyr[0][GRAY], CV_BGR2GRAY);
  split(src,ch);
  // RED
  pyr[0][RED] = ch[2] - (ch[1] + ch[0])/2;
  // GREEN
  pyr[0][GREEN] = ch[1] - (ch[2] + ch[0])/2;
  // BLUE
  pyr[0][BLUE] = ch[0] - (ch[2] + ch[1])/2;
  // YELLOW
  pyr[0][YELLOW] = (ch[2] + ch[1])/2 - abs(ch[2] - ch[1])/2 - ch[0];

  // other layers of pyramid
  for (int i = 0; i < 5; i++) {
    pyrDown (pyr[i][0], pyr[i+1][0], Size (pyr[i][0].cols/2, pyr[i][0].rows/2));
    tmp1 = pyr[i+1][0];
    for (int j = i+1; j > 0; j--) {
      pyrUp (tmp1, tmp2, Size (tmp1.cols*2, tmp1.rows*2));
      tmp1 = tmp2;
    }
    pyr[i+1][1] = tmp1;
    cvtColor (tmp1, pyr[i+1][GRAY], CV_BGR2GRAY);
    split (tmp1, ch);
    // RED
    pyr[i+1][RED] = ch[2] - (ch[1] + ch[0])/2;
    // GREEN
    pyr[i+1][GREEN] = ch[1] - (ch[2] + ch[0])/2;
    // BLUE
    pyr[i+1][BLUE] = ch[0] - (ch[2] + ch[1])/2;
    // YELLOW
    pyr[i+1][YELLOW] = (ch[2] + ch[1])/2 - abs(ch[2] - ch[1])/2 - ch[0];
  }
 
// building the features
  for (int i = 0; i <= 5; i++) {
  // SR - Spectrum Residual
  
    Mat grayTemp, grayDown;
    std::vector<Mat> mv;
    Size resizedImageSize( pyr[i][0].cols, pyr[i][0].rows );

    Mat realImage( resizedImageSize, CV_64F );
    Mat imaginaryImage( resizedImageSize, CV_64F );
    imaginaryImage.setTo( 0 );
    Mat combinedImage( resizedImageSize, CV_64FC2 );
    Mat imageDFT;
    Mat logAmplitude;
    Mat angle( resizedImageSize, CV_64F );
    Mat magnitude( resizedImageSize, CV_64F );
    Mat logAmplitude_blur, imageGR;

    if( pyr[i][0].channels() == 3 )
    {
      cvtColor( pyr[i][0], imageGR, COLOR_BGR2GRAY );
      resize( imageGR, grayDown, resizedImageSize, 0, 0, INTER_LINEAR );
    }
    else
    {
      resize( pyr[i][0], grayDown, resizedImageSize, 0, 0, INTER_LINEAR );
    }
 
    grayDown.convertTo( realImage, CV_64F );
  
    mv.push_back( realImage );
    mv.push_back( imaginaryImage );
    merge( mv, combinedImage );
    dft( combinedImage, imageDFT );
    split( imageDFT, mv );

    //-- Get magnitude and phase of frequency spectrum --//
    cartToPolar( mv.at( 0 ), mv.at( 1 ), magnitude, angle, false );
    log( magnitude, logAmplitude );
    //-- Blur log amplitude with averaging filter --//
    blur( logAmplitude, logAmplitude_blur, Size( 3, 3 ), Point( -1, -1 ), BORDER_DEFAULT );
    //-- Get the exponencial differences between --//  
    exp( logAmplitude - logAmplitude_blur, magnitude );
    //-- Back to cartesian frequency domain --//
    polarToCart( magnitude, angle, mv.at( 0 ), mv.at( 1 ), false );
    merge( mv, imageDFT );
    dft( imageDFT, combinedImage, DFT_INVERSE );
    split( combinedImage, mv );

    cartToPolar( mv.at( 0 ), mv.at( 1 ), magnitude, angle, false );
    GaussianBlur( magnitude, magnitude, Size( 5, 5 ), 8, 0, BORDER_DEFAULT );
    magnitude = magnitude.mul( magnitude );
  
    double minVal, maxVal;
    minMaxLoc( magnitude, &minVal, &maxVal );

    magnitude = magnitude / maxVal;
    magnitude.convertTo( magnitude, CV_32F );

    Mat saliencyMap;

    resize( magnitude, saliencyMap, src.size(), 0, 0, INTER_LINEAR );

    // visualize saliency map of Spectral Residual
    char win_name[15];
    sprintf(win_name, "SR_layer%i",i);
    imshow( win_name, saliencyMap );
    int cc;
    cc = waitKey(0);
    if( (char)cc == 27 )
      { return 0; }
    destroyAllWindows();

// /* Building the spatial features
    for (int j = i+1; j <=5; j++) {
      int c;
      char window_name[15];
      sprintf(window_name, "gray%d-%d",i,j);
      absdiff (pyr[i][GRAY], pyr[j][GRAY], results);
      imshow (window_name, results);
      c = waitKey(0);
      if( (char)c == 27 )
        { return 0; }
      destroyAllWindows();

      // gabor-filters 0 45 90 135
      Mat gaborKernel;
      Size ksize(31,31);
      double sigma = 4.0;
      double theta;
      double lambd = 10.0;
      double gamma = 0.5;
// when ddepth=-1, the output image will have the same depth as the source
      int ddepth = -1;
      for (int k = 0; k < 4; k++) {
        theta = CV_PI*k/4;
        gaborKernel = getGaborKernel(ksize, sigma, theta, lambd, gamma);
        sprintf(window_name, "gabor-kernel%d",180*k/4);
        imshow(window_name, gaborKernel);
        filter2D(results, gabor_output, ddepth, gaborKernel);
        sprintf(window_name, "gray%d-%d-gabor%d",i,j,180*k/4);
        imshow(window_name, gabor_output);
        c = waitKey(0);
        if( (char)c == 27 )
          { return 0; }
        destroyAllWindows();
      }

      sprintf(window_name, "R_G%d-%d",i,j);
      absdiff (pyr[i][RED]-pyr[i][GREEN],pyr[j][GREEN]-pyr[j][RED], results);
      imshow (window_name, results);
      c = waitKey(0);
      if( (char)c == 27 )
        { return 0; }

      destroyAllWindows();
      sprintf(window_name, "B_Y%d-%d",i,j);
      absdiff (pyr[i][BLUE]-pyr[i][YELLOW], pyr[j][YELLOW]-pyr[j][BLUE], results);
      imshow (window_name, results);
      c = waitKey(0);
      if( (char)c == 27 )
        { return 0; }
      destroyAllWindows();
    }
// */
  }
   
  /// Loop
  while( true )
  {
    int c;
    c = waitKey(10);

    if( (char)c == 27 )
      { break; }
    if( (char)c == 'u' )
      { pyrUp( tmp, dst, Size( tmp.cols*2, tmp.rows*2 ) );
        printf( "** Zoom In: Image x 2 \n" );
      }
    else if( (char)c == 'd' )
     { pyrDown( tmp, dst, Size( tmp.cols/2, tmp.rows/2 ) );
       printf( "** Zoom Out: Image / 2 \n" );
     }

    imshow( "Pyramids Demo", dst );
    tmp = dst;
  }
  return 0;
}

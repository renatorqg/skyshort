// tgfsp - Total Gabor Filter Sig Pyramid
// gabor filters 0 45 90 135 over pyramid
// Total means for all imagens of sequence scene of ball comming twoards the camera


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

// SO - Gaussian pyramid with all layers with same resolution images are RGB
#define SIG_RGB 1 
// SGRAY - Gaussian pyramid of grayscale
#define GRAY 2
// SRED - Gaussian pyramid of Red channel
#define RED 3
// 
#define GREEN 4
// 
#define BLUE 5
// 
#define YELLOW 6


std::vector <std::vector <Mat> > pyr1(6, std::vector <Mat> (7));
std::vector <std::vector <Mat> > pyr2(6, std::vector <Mat> (7));

Mat results;
Mat resultsEven;
Mat resultsOdd;
Mat gabor_outputOdd;
Mat gabor_outputEven;
Mat gabor_output;


// function to exctract regionals averages
// Mat res is a single channel image
void regionals_averages(Mat res, double average[9]) {

//  static double average[9];
  int k = 0;
  int cont = 0;
  for (int ri = 0; ri < res.cols-3; ri += res.cols/3) { //need to subtract 3 for the caso of res.cols/3 is not a round number
    for (int rj = 0; rj < res.rows-3; rj += res.rows/3) {
      
      Mat roiImg = res(Rect(ri,rj,res.cols/3,res.rows/3));
//      imshow("ROI", roiImg);
      cv::Scalar media;
      media = mean(roiImg);
//      int cc;
//      cc = waitKey(0);
//      if( (char)cc == 27 )
//        { return 0; }
      average[k] = media[0];
      k++;
    }
  } //
//  return average;
}

void build_pyr(std::vector <std::vector <Mat> > &pyr) { 

  tmp = src;
  dst = tmp;

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
      if (i != 4)
        tmp1 = tmp2;
      else
        tmp2.copyTo(tmp1);
      
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
}

Mat spectrum_residual(int i, std::vector <std::vector <Mat> > &pyr) 
{
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
  
 return saliencyMap;
}

void display(char *win_name, Mat results_to_show)
{
  imshow(win_name, results_to_show);
  int cc;
  cc = waitKey(0);
  if( (char)cc == 27 )
  { 
    destroyAllWindows();
    printf("You pressed ESC, exiting now\n");
    exit(0);
  }
  destroyAllWindows();
}

/**
 * @function main
 */
int main( int argc, char** argv )
{
// variables for the average function and write to file---///////////
  double average[9];                                      ///////////
  char filename[35];                                      ///////////
  char dirname[45];                                       ///////////
  FILE *fp;                                               ///////////
//--------------------------------------------------------///////////

// front-front start on x00035 and end on x00048 total of 14 images
  char imagename[30];
// src = imread( "./front-front/l00039.png", CV_LOAD_IMAGE_GRAYSCALE );

  for (int i_img = 35; i_img < 53; i_img++) {

//    sprintf(imagename, "./front-front/r%05d.png",i_img);
    sprintf(imagename, "./front-front/l%05d.png",i_img);

    src = imread( imagename, CV_LOAD_IMAGE_COLOR );
    if( !src.data )
      { printf(" No data! -- Exiting the program \n");
        return -1; }

    printf("./front-front/l%05d.png\n",i_img);
    printf( "cols = %i, rows = %i\n",src.cols,src.rows);

//    display(imagename, src);
    char win_name[35];

//   build the layers of pyr
    if (i_img % 2 == 0) //image frame even
    {
      build_pyr(pyr1);
      //scanf through the layers of gaussian pyramid
      for (int i = 0; i <= 5; i++) 
      {
////////// spectrum residual ///////////////////////////////////////////////////
        resultsEven = spectrum_residual( i, pyr1);
//        sprintf(win_name, "img%d-SR_layer%i", i_img, i);
//        display(win_name, resultsEven);
        regionals_averages(resultsEven, average);
        for (int sec = 0; sec < 9; sec++) 
        {
          sprintf(filename, "./datapoints/sr/%d-sec%d",i,sec);
          fp = fopen(filename, "a"); //mode "a" for append
          if ( fp == NULL ) 
          {
            system("mkdir ./datapoints/sr");
            fp = fopen(filename, "a"); //mode "a" for append
            if ( fp == NULL )
              printf("Unable to open the file %s\n", filename);
          }
          fprintf(fp,"%lf\n", average[sec]);
          fclose(fp);
        }
        sprintf(filename, "./datapoints/sr/fi%02d-%d.png",i_img,i);
        imwrite(filename, resultsEven);

        if (pyr1[0][0].data && pyr2[0][0].data)
        {
////////// motion spectrum residual ///////////////////////////////////////////
          resultsOdd = spectrum_residual( i, pyr2);
          absdiff(resultsEven, resultsOdd, results);
//          sprintf(win_name, "Motion-img%d-SR_layer%i", i_img, i);
//          display(win_name, results);
          regionals_averages(results, average);
          for (int sec = 0; sec < 9; sec++) 
          {
            sprintf(filename, "./datapoints/motion-sr/%d-sec%d",i,sec);
            fp = fopen(filename, "a"); //mode "a" for append
            if ( fp == NULL ) 
            {
              system("mkdir ./datapoints/motion-sr");
              fp = fopen(filename, "a"); //mode "a" for append
              if ( fp == NULL )
                printf("Unable to open the file %s\n", filename);
            }
            fprintf(fp,"%lf\n", average[sec]);
            fclose(fp);
          }
          sprintf(filename, "./datapoints/motion-sr/fi%02d-%d.png",i_img,i);
          imwrite(filename, results);


////////// pure motion of RGB not filtered images ////////////////////////////////
          absdiff(pyr2[i][SIG_RGB],pyr1[i][SIG_RGB], results);
//          sprintf(win_name, "Motion-img%d-RGB%i", i_img, i);
//          display(win_name, results);
          regionals_averages(results, average);
          for (int sec = 0; sec < 9; sec++) 
          {
            sprintf(filename, "./datapoints/motion-RGB/%d-sec%d",i,sec);
            fp = fopen(filename, "a"); //mode "a" for append
            if ( fp == NULL ) 
            {
              system("mkdir ./datapoints/motion-RGB");
              fp = fopen(filename, "a"); //mode "a" for append
              if ( fp == NULL )
                printf("Unable to open the file %s\n", filename);
            }
            fprintf(fp,"%lf\n", average[sec]);
            fclose(fp);
          }
          sprintf(filename, "./datapoints/motion-RGB/fi%02d-%d.png",i_img,i);
          imwrite(filename, results);

          // making combination between layers 0-1, 0-2, 0-3, 0-4, 0-5, 1-2, 1-3 and so on
          for (int j = i+1; j <=5; j++) 
          { 
////////////// gray /////////////////////////////////////////////////////////////
            absdiff (pyr1[i][GRAY], pyr1[j][GRAY], resultsEven); // pyr1 - Even
//            sprintf(win_name, "img%d-gray%d-%d",i_img,i,j);
//            display(win_name, resultsEven);
            regionals_averages(resultsEven, average);
            for (int sec = 0; sec < 9; sec++) 
            {
              sprintf(filename, "./datapoints/gray/%d-%d-sec%d",i,j,sec);
              fp = fopen(filename, "a"); //mode "a" for append
              if ( fp == NULL ) 
              {
                system("mkdir ./datapoints/gray");
                fp = fopen(filename, "a"); //mode "a" for append
                if ( fp == NULL )
                  printf("Unable to open the file %s\n", filename);
              }
              fprintf(fp,"%lf\n", average[sec]);
              fclose(fp);
            }
            sprintf(filename, "./datapoints/gray/fi%02d-%d-%d.png",i_img,i,j);
            imwrite(filename, resultsEven);


////////////// motion gray /////////////////////////////////////////////////////////
            absdiff (pyr2[i][GRAY], pyr2[j][GRAY], resultsOdd); // pyr2 - Odd 
            absdiff (resultsEven, resultsOdd, results);
//            sprintf(win_name, "Motion-img%d-GRAY%d-%d",i_img, i, j);
//            display(win_name, results);
            regionals_averages(results, average);
            for (int sec = 0; sec < 9; sec++) 
            {
              sprintf(filename, "./datapoints/motion-gray/%d-%d-sec%d",i,j,sec);
              fp = fopen(filename, "a"); //mode "a" for append
              if ( fp == NULL ) 
              {
                system("mkdir ./datapoints/motion-gray");
                fp = fopen(filename, "a"); //mode "a" for append
                if ( fp == NULL )
                  printf("Unable to open the file %s\n", filename);
              }
              fprintf(fp,"%lf\n", average[sec]);
              fclose(fp);
            }
            sprintf(filename, "./datapoints/motion-gray/fi%02d-%d-%d.png",i_img,i,j);
            imwrite(filename, results);

////////////// gabor-filters 0 45 90 135 ////////////////////////////////////////////
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
//              sprintf(window_name, "gabor-kernel%d",180*k/4);
//              imshow(window_name, gaborKernel);
              filter2D(resultsEven, gabor_outputEven, ddepth, gaborKernel);
              filter2D(resultsOdd, gabor_outputOdd, ddepth, gaborKernel);
//              sprintf(win_name, "img%d-gabor%d-gray%d-%d-Odd",i_img,180*k/4,i,j);
//              display(win_name, gabor_outputEven);
              regionals_averages(gabor_outputEven, average);
              for (int sec = 0; sec < 9; sec++) {
                sprintf(filename, "./datapoints/gabor%d/%d-%d-sec%d",180*k/4,i,j,sec);
                fp = fopen(filename, "a"); //mode "a" for append
                if ( fp == NULL ) {
                  sprintf(dirname, "mkdir ./datapoints/gabor%d",180*k/4);
                  system(dirname);
                  fp = fopen(filename, "a"); //mode "a" for append
                  if ( fp == NULL )
                    printf("Unable to open the file %s\n", filename);
                }
                fprintf(fp,"%lf\n", average[sec]);
                fclose(fp);
              }
              sprintf(filename, "./datapoints/gabor%d/fi%02d-%d-%d.png",180*k/4,i_img,i,j);
              imwrite(filename, gabor_outputEven);
//////////////// motion gabor////////////////////////////////////////////////////////////
              absdiff (gabor_outputEven, gabor_outputOdd, results);
//              sprintf(win_name, "img%d-motion-gabor%d-gray%d-%d.png",i_img,180*k/4,i,j);
//              display(win_name, results);
              regionals_averages(results, average);
              for (int sec = 0; sec < 9; sec++) {
                sprintf(filename, "./datapoints/motion-gabor%d/%d-%d-sec%d",180*k/4,i,j,sec);
                fp = fopen(filename, "a"); //mode "a" for append
                if ( fp == NULL ) {
                  sprintf(dirname, "mkdir ./datapoints/motion-gabor%d",180*k/4);
                  system(dirname);
                  fp = fopen(filename, "a"); //mode "a" for append
                  if ( fp == NULL )
                    printf("Unable to open the file %s\n", filename);
                }
                fprintf(fp,"%lf\n", average[sec]);
                fclose(fp);
              }
              sprintf(filename, "./datapoints/motion-gabor%d/fi%02d-%d-%d.png",180*k/4,i_img,i,j);
              imwrite(filename, results);
            }

            // R_G
            absdiff (pyr1[i][RED]-pyr1[i][GREEN],pyr1[j][GREEN]-pyr1[j][RED], resultsEven); //pyr1 - Even
//            sprintf(win_name, "img%d-R_G%d-%d",i_img,i,j);
//            display(win_name, resultsEven);
            regionals_averages(resultsEven, average);
            for (int sec = 0; sec < 9; sec++) {
              sprintf(filename, "./datapoints/R_G/%d-%d-sec%d",i,j,sec);
              fp = fopen(filename, "a"); //mode "a" for append
              if ( fp == NULL ) {
                sprintf(dirname, "mkdir ./datapoints/R_G");
                system(dirname);
                fp = fopen(filename, "a"); //mode "a" for append
                if ( fp == NULL )
                   printf("unable to open the file %s\n", filename);
              } 
              fprintf(fp,"%lf\n", average[sec]);
              fclose(fp);
            }
            sprintf(filename, "./datapoints/R_G/fi%02d-%d-%d.png",i_img,i,j);
            imwrite(filename, resultsEven);

            // motion R_G
            absdiff (pyr2[i][RED]-pyr2[i][GREEN],pyr2[j][GREEN]-pyr2[j][RED], resultsOdd); //pyr2 - Odd
            absdiff (resultsEven, resultsOdd, results);
//            sprintf(win_name, "Motion-img%d-R_G%d-%d",i_img, i, j);
//            display(win_name, results);
            regionals_averages(results, average);
            for (int sec = 0; sec < 9; sec++) 
            {
              sprintf(filename, "./datapoints/motion-R_G/%d-%d-sec%d",i,j,sec);
              fp = fopen(filename, "a"); //mode "a" for append
              if ( fp == NULL ) 
              {
                system("mkdir ./datapoints/motion-R_G");
                fp = fopen(filename, "a"); //mode "a" for append
                if ( fp == NULL )
                  printf("Unable to open the file %s\n", filename);
              }
              fprintf(fp,"%lf\n", average[sec]);
              fclose(fp);
            }
            sprintf(filename, "./datapoints/motion-R_G/fi%02d-%d-%d.png",i_img,i,j);
            imwrite(filename, results);

            // B_Y
            absdiff (pyr1[i][BLUE]-pyr1[i][YELLOW], pyr1[j][YELLOW]-pyr1[j][BLUE], resultsEven); //pyr1 - Even
//            sprintf(win_name, "img%d-B_Y%d-%d",i_img,i,j);
//            display(win_name, resultsEven);
            regionals_averages(resultsEven, average);
            for (int sec = 0; sec < 9; sec++) {
              sprintf(filename, "./datapoints/B_Y/%d-%d-sec%d",i,j,sec);
              fp = fopen(filename, "a"); //mode "a" for append
              if ( fp == NULL ) {
                sprintf(dirname, "mkdir ./datapoints/B_Y");
                system(dirname);
                fp = fopen(filename, "a"); //mode "a" for append
                if ( fp == NULL )
                   printf("unable to open the file %s\n", filename);
              } 
              fprintf(fp,"%lf\n", average[sec]);
              fclose(fp);
            }
            sprintf(filename, "./datapoints/B_Y/fi%02d-%d-%d.png",i_img,i,j);
            imwrite(filename, resultsEven);

            // motion B_Y
            absdiff (pyr2[i][BLUE]-pyr2[i][YELLOW], pyr2[j][YELLOW]-pyr2[j][BLUE], resultsOdd); //pyr2 - Odd
            absdiff (resultsOdd, resultsEven, results);
//            sprintf(win_name, "Motion-img%d-B_Y%d-%d",i_img, i, j);
//            display(win_name, results);
            regionals_averages(results, average);
            for (int sec = 0; sec < 9; sec++) 
            {
              sprintf(filename, "./datapoints/motion-B_Y/%d-%d-sec%d",i,j,sec);
              fp = fopen(filename, "a"); //mode "a" for append
              if ( fp == NULL ) 
              {
                system("mkdir ./datapoints/motion-B_Y");
                fp = fopen(filename, "a"); //mode "a" for append
                if ( fp == NULL )
                  printf("Unable to open the file %s\n", filename);
              }
              fprintf(fp,"%lf\n", average[sec]);
              fclose(fp);
            }
            sprintf(filename, "./datapoints/motion-B_Y/fi%02d-%d-%d.png",i_img,i,j);
            imwrite(filename, results);
          } // end of for j
 

        } // end of if (pyr1[0][0].data && pyr[0][0].data
      } // end of for i
    } // end of if i_img % 2 == 0

    else //image frame odd
    {
      build_pyr(pyr2);
      //scanf through the layers of gaussian pyramid
      for (int i = 0; i <= 5; i++)
      {
////////// spectrum residual ///////////////////////////////////////////////////
        resultsOdd = spectrum_residual( i, pyr2);
//        sprintf(win_name, "img%d-SR_layer%i", i_img, i);
//        display(win_name, resultsOdd);
        regionals_averages(resultsOdd, average);
        for (int sec = 0; sec < 9; sec++) 
        {
          sprintf(filename, "./datapoints/sr/%d-sec%d",i,sec);
          fp = fopen(filename, "a"); //mode "a" for append
          if ( fp == NULL ) 
          {
            system("mkdir ./datapoints/sr");
            fp = fopen(filename, "a"); //mode "a" for append
            if ( fp == NULL )
              printf("Unable to open the file %s\n", filename);
          }
          fprintf(fp,"%lf\n", average[sec]);
          fclose(fp);
        }
        sprintf(filename, "./datapoints/sr/fi%02d-%d.png",i_img,i);
        imwrite(filename, resultsOdd);

        if (pyr1[0][0].data && pyr2[0][0].data)
        {
////////// motion spectrum residual ///////////////////////////////////////////
          resultsEven = spectrum_residual( i, pyr1);
          absdiff(resultsOdd, resultsEven, results);
//          sprintf(win_name, "Motion-img%d-SR_layer%i", i_img, i);
//          display(win_name, results);
          regionals_averages(results, average);
          for (int sec = 0; sec < 9; sec++) 
          {
            sprintf(filename, "./datapoints/motion-sr/%d-sec%d",i,sec);
            fp = fopen(filename, "a"); //mode "a" for append
            if ( fp == NULL ) 
            {
              system("mkdir ./datapoints/motion-sr");
              fp = fopen(filename, "a"); //mode "a" for append
              if ( fp == NULL )
                printf("Unable to open the file %s\n", filename);
            }
            fprintf(fp,"%lf\n", average[sec]);
            fclose(fp);
          }
          sprintf(filename, "./datapoints/motion-sr/fi%02d-%d.png",i_img,i);
          imwrite(filename, results);


////////// pure motion of RGB not filtered images ////////////////////////////////
          absdiff(pyr2[i][SIG_RGB],pyr1[i][SIG_RGB], results);
//          sprintf(win_name, "Motion-img%d-RGB%i", i_img, i);
//          display(win_name, results);
          regionals_averages(results, average);
          for (int sec = 0; sec < 9; sec++) 
          {
            sprintf(filename, "./datapoints/motion-RGB/%d-sec%d",i,sec);
            fp = fopen(filename, "a"); //mode "a" for append
            if ( fp == NULL ) 
            {
              system("mkdir ./datapoints/motion-RGB");
              fp = fopen(filename, "a"); //mode "a" for append
              if ( fp == NULL )
                printf("Unable to open the file %s\n", filename);
            }
            fprintf(fp,"%lf\n", average[sec]);
            fclose(fp);
          }
          sprintf(filename, "./datapoints/motion-RGB/fi%02d-%d.png",i_img,i);
          imwrite(filename, results);

          // making combination between layers 0-1, 0-2, 0-3, 0-4, 0-5, 1-2, 1-3 and so on
          for (int j = i+1; j <=5; j++) 
          { 
////////////// gray /////////////////////////////////////////////////////////////
            absdiff (pyr2[i][GRAY], pyr2[j][GRAY], resultsOdd); // pyr2 - Odd
//            sprintf(win_name, "img%d-gray%d-%d",i_img,i,j);
//            display(win_name, resultsOdd);
            regionals_averages(resultsOdd, average);
            for (int sec = 0; sec < 9; sec++) 
            {
              sprintf(filename, "./datapoints/gray/%d-%d-sec%d",i,j,sec);
              fp = fopen(filename, "a"); //mode "a" for append
              if ( fp == NULL ) 
              {
                system("mkdir ./datapoints/gray");
                fp = fopen(filename, "a"); //mode "a" for append
                if ( fp == NULL )
                  printf("Unable to open the file %s\n", filename);
              }
              fprintf(fp,"%lf\n", average[sec]);
              fclose(fp);
            }
            sprintf(filename, "./datapoints/gray/fi%02d-%d-%d.png",i_img,i,j);
            imwrite(filename, resultsOdd);


////////////// motion gray /////////////////////////////////////////////////////////
            absdiff (pyr1[i][GRAY], pyr1[j][GRAY], resultsEven); // pyr1 - Even
            absdiff (resultsOdd, resultsEven, results);
//            sprintf(win_name, "Motion-img%d-GRAY%d-%d",i_img, i, j);
//            display(win_name, results);
            regionals_averages(results, average);
            for (int sec = 0; sec < 9; sec++) 
            {
              sprintf(filename, "./datapoints/motion-gray/%d-%d-sec%d",i,j,sec);
              fp = fopen(filename, "a"); //mode "a" for append
              if ( fp == NULL ) 
              {
                system("mkdir ./datapoints/motion-gray");
                fp = fopen(filename, "a"); //mode "a" for append
                if ( fp == NULL )
                  printf("Unable to open the file %s\n", filename);
              }
              fprintf(fp,"%lf\n", average[sec]);
              fclose(fp);
            }
            sprintf(filename, "./datapoints/motion-gray/fi%02d-%d-%d.png",i_img,i,j);
            imwrite(filename, results);

////////////// gabor-filters 0 45 90 135 ////////////////////////////////////////////
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
//              sprintf(window_name, "gabor-kernel%d",180*k/4);
//              imshow(window_name, gaborKernel);
              filter2D(resultsOdd, gabor_outputOdd, ddepth, gaborKernel);
              filter2D(resultsEven, gabor_outputEven, ddepth, gaborKernel);
//              sprintf(win_name, "img%d-gabor%d-gray%d-%d-Odd",i_img,180*k/4,i,j);
//              display(win_name, gabor_outputOdd);
              regionals_averages(gabor_outputOdd, average);
              for (int sec = 0; sec < 9; sec++) {
                sprintf(filename, "./datapoints/gabor%d/%d-%d-sec%d",180*k/4,i,j,sec);
                fp = fopen(filename, "a"); //mode "a" for append
                if ( fp == NULL ) {
                  sprintf(dirname, "mkdir ./datapoints/gabor%d",180*k/4);
                  system(dirname);
                  fp = fopen(filename, "a"); //mode "a" for append
                  if ( fp == NULL )
                    printf("Unable to open the file %s\n", filename);
                }
                fprintf(fp,"%lf\n", average[sec]);
                fclose(fp);
              }
              sprintf(filename, "./datapoints/gabor%d/fi%02d-%d-%d.png",180*k/4,i_img,i,j);
              imwrite(filename, gabor_outputOdd);
//////////////// motion gabor////////////////////////////////////////////////////////////
              absdiff (gabor_outputOdd, gabor_outputEven, results);
//              sprintf(win_name, "img%d-motion-gabor%d-gray%d-%d.png",i_img,180*k/4,i,j);
//              display(win_name, results);
              regionals_averages(results, average);
              for (int sec = 0; sec < 9; sec++) {
                sprintf(filename, "./datapoints/motion-gabor%d/%d-%d-sec%d",180*k/4,i,j,sec);
                fp = fopen(filename, "a"); //mode "a" for append
                if ( fp == NULL ) {
                  sprintf(dirname, "mkdir ./datapoints/motion-gabor%d",180*k/4);
                  system(dirname);
                  fp = fopen(filename, "a"); //mode "a" for append
                  if ( fp == NULL )
                    printf("Unable to open the file %s\n", filename);
                }
                fprintf(fp,"%lf\n", average[sec]);
                fclose(fp);
              }
              sprintf(filename, "./datapoints/motion-gabor%d/fi%02d-%d-%d.png",i_img,180*k/4,i,j);
              imwrite(filename, results);
            }

            // R_G
            absdiff (pyr2[i][RED]-pyr2[i][GREEN],pyr2[j][GREEN]-pyr2[j][RED], resultsOdd); //pyr2 - Odd
//            sprintf(win_name, "img%d-R_G%d-%d",i_img,i,j);
//            display(win_name, resultsOdd);
            regionals_averages(resultsOdd, average);
            for (int sec = 0; sec < 9; sec++) {
              sprintf(filename, "./datapoints/R_G/%d-%d-sec%d",i,j,sec);
              fp = fopen(filename, "a"); //mode "a" for append
              if ( fp == NULL ) {
                sprintf(dirname, "mkdir ./datapoints/R_G");
                system(dirname);
                fp = fopen(filename, "a"); //mode "a" for append
                if ( fp == NULL )
                   printf("unable to open the file %s\n", filename);
              } 
              fprintf(fp,"%lf\n", average[sec]);
              fclose(fp);
            }
            sprintf(filename, "./datapoints/R_G/fi%02d-%d-%d.png",i_img,i,j);
            imwrite(filename, resultsOdd);

            // motion R_G
            absdiff (pyr1[i][RED]-pyr1[i][GREEN],pyr1[j][GREEN]-pyr1[j][RED], resultsEven); //pyr1 - Even
            absdiff (resultsOdd, resultsEven, results);
//            sprintf(win_name, "Motion-img%d-R_G%d-%d",i_img, i, j);
//            display(win_name, results);
            regionals_averages(results, average);
            for (int sec = 0; sec < 9; sec++) 
            {
              sprintf(filename, "./datapoints/motion-R_G/%d-%d-sec%d",i,j,sec);
              fp = fopen(filename, "a"); //mode "a" for append
              if ( fp == NULL ) 
              {
                system("mkdir ./datapoints/motion-R_G");
                fp = fopen(filename, "a"); //mode "a" for append
                if ( fp == NULL )
                  printf("Unable to open the file %s\n", filename);
              }
              fprintf(fp,"%lf\n", average[sec]);
              fclose(fp);
            }
            sprintf(filename, "./datapoints/motion-R_G/fi%02d-%d-%d.png",i_img,i,j);
            imwrite(filename, results);

            // B_Y
            absdiff (pyr2[i][BLUE]-pyr2[i][YELLOW], pyr2[j][YELLOW]-pyr2[j][BLUE], resultsOdd); //pyr2 - Odd
//            sprintf(win_name, "img%d-B_Y%d-%d",i_img,i,j);
//            display(win_name, resultsOdd);
            regionals_averages(resultsOdd, average);
            for (int sec = 0; sec < 9; sec++) {
              sprintf(filename, "./datapoints/B_Y/%d-%d-sec%d",i,j,sec);
              fp = fopen(filename, "a"); //mode "a" for append
              if ( fp == NULL ) {
                sprintf(dirname, "mkdir ./datapoints/B_Y");
                system(dirname);
                fp = fopen(filename, "a"); //mode "a" for append
                if ( fp == NULL )
                   printf("unable to open the file %s\n", filename);
              } 
              fprintf(fp,"%lf\n", average[sec]);
              fclose(fp);
            }
            sprintf(filename, "./datapoints/B_Y/fi%02d-%d-%d.png",i_img,i,j);
            imwrite(filename, resultsOdd);

            // motion B_Y
            absdiff (pyr1[i][BLUE]-pyr1[i][YELLOW], pyr1[j][YELLOW]-pyr1[j][BLUE], resultsEven); //pyr1 - Even
            absdiff (resultsOdd, resultsEven, results);
//            sprintf(win_name, "Motion-img%d-B_Y%d-%d",i_img, i, j);
//            display(win_name, results);
            regionals_averages(results, average);
            for (int sec = 0; sec < 9; sec++) 
            {
              sprintf(filename, "./datapoints/motion-B_Y/%d-%d-sec%d",i,j,sec);
              fp = fopen(filename, "a"); //mode "a" for append
              if ( fp == NULL ) 
              {
                system("mkdir ./datapoints/motion-B_Y");
                fp = fopen(filename, "a"); //mode "a" for append
                if ( fp == NULL )
                  printf("Unable to open the file %s\n", filename);
              }
              fprintf(fp,"%lf\n", average[sec]);
              fclose(fp);
            }
            sprintf(filename, "./datapoints/motion-B_Y/fi%02d-%d-%d.png",i_img,i,j);
            imwrite(filename, results);
          } // end of for j
        } // end of if(pyr1.data && pyr2.data)
      } // end of for i
    } // end of Odd 
  }//end of i_img
  printf("THE END!\n");
  return 0;
}

# skyshort
Codes of my doctoral thesis, uses opencv

The objective is to analyze the results of image features like (lines, blobs, corners, etc) in various saliency context.

To do that, the first task is to implement some functions for example the features of [Itti's](http://ilab.usc.edu/publications/doc/Itti_etal98pami.pdf) and [Belongie's](http://www.rod.goodman.name/pdf/RG.Paper.CP66.pdf) papers.

List of implemented features:

**spatial domain**
- Center-surround oposite colors Blue-Yellow, Red-Green, various scales
- Center-surround intensities, various scales
- Orientations, gabour-filters, various scales

**frequency domain**
- Spectral-Residual [Xiaodi Hou's](http://www.klab.caltech.edu/~xhou/papers/cvpr07.pdf) paper, adapted from this [code](https://github.com/Itseez/opencv_contrib/blob/master/modules/saliency/src/staticSaliencySpectralResidual.cpp)

# COMPILATION LINE

g++ tgfsp.cpp `pkg-config --cflags --libs opencv`

**Prerequisites**
installed Opencv
installed build-essencials

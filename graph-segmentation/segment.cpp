/*
Copyright (C) 2006 Pedro Felzenszwalb

This program is free software; you can redistribute it and/or modify
it under the terms of the GNU General Public License as published by
the Free Software Foundation; either version 2 of the License, or
(at your option) any later version.

This program is distributed in the hope that it will be useful,
but WITHOUT ANY WARRANTY; without even the implied warranty of
MERCHANTABILITY or FITNESS FOR A PARTICULAR PURPOSE.  See the
GNU General Public License for more details.

You should have received a copy of the GNU General Public License
along with this program; if not, write to the Free Software
Foundation, Inc., 59 Temple Place, Suite 330, Boston, MA  02111-1307 USA
*/

#include <cstdio>
#include <cstdlib>
#include "graph_segment/image.h"
#include "graph_segment/misc.h"
#include "graph_segment/pnmfile.h"
#include "graph_segment/segment-image.h"
#include "opencv2/opencv.hpp"

int main(int argc, char **argv) {
  if (argc != 6) {
    fprintf(stderr, "usage: %s sigma k min input(ppm) output(ppm)\n", argv[0]);
    return 1;
  }
  
  float sigma = atof(argv[1]);
  float k = atof(argv[2]);
  int min_size = atoi(argv[3]);
	
  printf("loading input image.\n");
  cv::Mat img = cv::imread(argv[4]);

//   image<rgb> *input = loadPPM(argv[4]);
//   cv::namedWindow("ppm", cv::WINDOW_NORMAL);
//   cv::imshow("ppm", img);
//   cv::waitKey(0);
	
  printf("processing\n");
  int num_ccs; 
//   image<rgb> *seg = segment_image(input, sigma, k, min_size, &num_ccs); 
//   savePPM(seg, argv[5]);
  cv::Mat out = SegmentImage(img, sigma, k, min_size, &num_ccs);
  cv::imwrite(argv[5], out);

  printf("got %d components\n", num_ccs);
  printf("done! uff...thats hard work.\n");
  return 0;
}


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

#ifndef SEGMENT_IMAGE
#define SEGMENT_IMAGE

#include <cstdlib>
#include <iostream>
#include "image.h"
#include "misc.h"
#include "filter.h"
#include "segment-graph.h"
#include <vector>
#include "opencv2/core.hpp"
#include "opencv2/imgproc.hpp"

// random color
rgb random_rgb(){ 
  rgb c;
  double r;
  
  c.r = (uchar)rand();
  c.g = (uchar)rand();
  c.b = (uchar)rand();

  return c;
}

cv::Vec3b random_color(){ 
  cv::Vec3b c;
  
  c[0] = (uchar)rand();
  c[1] = (uchar)rand();
  c[2] = (uchar)rand();

  return c;
}

// dissimilarity measure between pixels
static inline float diff(image<float> *r, image<float> *g, image<float> *b,
			 int x1, int y1, int x2, int y2) {
  return sqrt(square(imRef(r, x1, y1)-imRef(r, x2, y2)) +
	      square(imRef(g, x1, y1)-imRef(g, x2, y2)) +
	      square(imRef(b, x1, y1)-imRef(b, x2, y2)));
}

/*
 * Segment an image
 *
 * Returns a color image representing the segmentation.
 *
 * im: image to segment.
 * sigma: to smooth the image.
 * c: constant for treshold function.
 * min_size: minimum component size (enforced by post-processing stage).
 * num_ccs: number of connected components in the segmentation.
 */
image<rgb> *segment_image(image<rgb> *im, float sigma, float c, int min_size,
			  int *num_ccs) {
  int width = im->width();
  int height = im->height();

  image<float> *r = new image<float>(width, height);
  image<float> *g = new image<float>(width, height);
  image<float> *b = new image<float>(width, height);

  // smooth each color channel  
  for (int y = 0; y < height; y++) {
    for (int x = 0; x < width; x++) {
      imRef(r, x, y) = imRef(im, x, y).r;
      imRef(g, x, y) = imRef(im, x, y).g;
      imRef(b, x, y) = imRef(im, x, y).b;
    }
  }
  
  std::cout<<imRef(b, 0, 0)<<std::endl;

  image<float> *smooth_r = smooth(r, sigma);
  image<float> *smooth_g = smooth(g, sigma);
  image<float> *smooth_b = smooth(b, sigma);
  delete r;
  delete g;
  delete b;
 
  // build graph
  edge *edges = new edge[width*height*4];
  int num = 0;
  for (int y = 0; y < height; y++) {
    for (int x = 0; x < width; x++) {
      if (x < width-1) {
	edges[num].a = y * width + x;
	edges[num].b = y * width + (x+1);
	edges[num].w = diff(smooth_r, smooth_g, smooth_b, x, y, x+1, y);
	num++;
      }

      if (y < height-1) {
	edges[num].a = y * width + x;
	edges[num].b = (y+1) * width + x;
	edges[num].w = diff(smooth_r, smooth_g, smooth_b, x, y, x, y+1);
	num++;
      }

      if ((x < width-1) && (y < height-1)) {
	edges[num].a = y * width + x;
	edges[num].b = (y+1) * width + (x+1);
	edges[num].w = diff(smooth_r, smooth_g, smooth_b, x, y, x+1, y+1);
	num++;
      }

      if ((x < width-1) && (y > 0)) {
	edges[num].a = y * width + x;
	edges[num].b = (y-1) * width + (x+1);
	edges[num].w = diff(smooth_r, smooth_g, smooth_b, x, y, x+1, y-1);
	num++;
      }
    }
  }

//   std::cout<<edges[0].w<<std::endl;
//   std::cout<<imRef(smooth_r, 0, 0)<<"  "<<imRef(smooth_g, 0, 0)<<"  "<<imRef(smooth_b, 0, 0)<<std::endl;
//   std::cout<<imRef(smooth_r, 1, 0)<<"  "<<imRef(smooth_g, 1, 0)<<"  "<<imRef(smooth_b, 1, 0)<<std::endl;

  delete smooth_r;
  delete smooth_g;
  delete smooth_b;

  // segment
  universe *u = segment_graph(width*height, num, edges, c);
  
  // post process small components
  for (int i = 0; i < num; i++) {
    int a = u->find(edges[i].a);
    int b = u->find(edges[i].b);
    if ((a != b) && ((u->size(a) < min_size) || (u->size(b) < min_size)))
      u->join(a, b);
  }
  delete [] edges;
  *num_ccs = u->num_sets();

  image<rgb> *output = new image<rgb>(width, height);

  // pick random colors for each component
  rgb *colors = new rgb[width*height];
  for (int i = 0; i < width*height; i++)
    colors[i] = random_rgb();
  
  for (int y = 0; y < height; y++) {
    for (int x = 0; x < width; x++) {
      int comp = u->find(y * width + x);
      imRef(output, x, y) = colors[comp];
    }
  }  

  delete [] colors;  
  delete u;

  return output;
}

static inline float diff(const cv::Mat& r, const cv::Mat& g, const cv::Mat& b, int x1, int y1, int x2, int y2){
    return sqrt(square(r.at<float>(y1, x1) - r.at<float>(y2, x2)) +
	      square(g.at<float>(y1, x1) - g.at<float>(y2, x2)) +
	      square(b.at<float>(y1, x1) - b.at<float>(y2, x2)));
}

cv::Mat SegmentImage(const cv::Mat& img, float sigma, float c, int min_size, int *num_ccs){
    int width = img.cols;
    int height = img.rows;

    cv::Mat image(height, width, CV_32FC3);
    img.convertTo(image, CV_32FC3, 1, 0);

    cv::Mat channels[3];
    cv::split(image, channels);
    cv::Mat smooth_channels[3];

    cv::GaussianBlur(channels[0], smooth_channels[0], cv::Size(3, 3), sigma, sigma);
    cv::GaussianBlur(channels[1], smooth_channels[1], cv::Size(3, 3), sigma, sigma);
    cv::GaussianBlur(channels[2], smooth_channels[2], cv::Size(3, 3), sigma, sigma);

    edge *edges = new edge[width*height*4];
    int num = 0;
    for (int y = 0; y < height; y++) {
        for (int x = 0; x < width; x++) {
            if (x < width-1) {
            edges[num].a = y * width + x;
            edges[num].b = y * width + (x+1);
            edges[num].w = diff(smooth_channels[2], smooth_channels[1], smooth_channels[0], x, y, x+1, y);
            num++;
            }

            if (y < height-1) {
            edges[num].a = y * width + x;
            edges[num].b = (y+1) * width + x;
            edges[num].w = diff(smooth_channels[2], smooth_channels[1], smooth_channels[0], x, y, x, y+1);
            num++;
            }

            if ((x < width-1) && (y < height-1)) {
            edges[num].a = y * width + x;
            edges[num].b = (y+1) * width + (x+1);
            edges[num].w = diff(smooth_channels[2], smooth_channels[1], smooth_channels[0], x, y, x+1, y+1);
            num++;
            }

            if ((x < width-1) && (y > 0)) {
            edges[num].a = y * width + x;
            edges[num].b = (y-1) * width + (x+1);
            edges[num].w = diff(smooth_channels[2], smooth_channels[1], smooth_channels[0], x, y, x+1, y-1);
            num++;
            }
        }
    }
    
    // std::cout<<edges[0].w<<std::endl;
    // std::cout<<(float)smooth_channels[2].at<uchar>(0,0)<<"  "<<(float)smooth_channels[1].at<uchar>(0,0)<<"  "<<(float)smooth_channels[0].at<uchar>(0,0)<<std::endl;
    // std::cout<<(float)smooth_channels[2].at<uchar>(0,1)<<"  "<<(float)smooth_channels[1].at<uchar>(0,1)<<"  "<<(float)smooth_channels[0].at<uchar>(0,1)<<std::endl;

    universe *u = segment_graph(width*height, num, edges, c);

    // post process small components
    for (int i = 0; i < num; i++) {
        int a = u->find(edges[i].a);
        int b = u->find(edges[i].b);
        if ((a != b) && ((u->size(a) < min_size) || (u->size(b) < min_size)))
        u->join(a, b);
    }
    delete [] edges;
    *num_ccs = u->num_sets();

    cv::Mat out_image(height, width, CV_8UC3);

    cv::Vec3b *colors = new cv::Vec3b[width*height];
    for (int i = 0; i < width*height; i++)
        colors[i] = random_color();
    
    for (int y = 0; y < height; y++) {
        for (int x = 0; x < width; x++) {
            int comp = u->find(y * width + x);
            out_image.at<cv::Vec3b>(y, x) = colors[comp];
        }
    }
    return out_image;
}
#endif

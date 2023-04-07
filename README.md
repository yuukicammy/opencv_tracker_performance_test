# OpenCV tracker performance test

This code is for testing the performance of the Tracking API introduced in OpenCV.

## Requirements
- OpenCV 3.0
- Visual C++ 2013
- Windows
- [Visual Tracker Benchmark](http://cvlab.hanyang.ac.kr/tracker_benchmark/benchmark_v10.html )
- matlab

## Results on YouTube

You can easily understand the performance test results with this code from the video.

[[YouTube] OpenCV 3.0 Tracking API Results](https://youtu.be/pj-QuE6pdEQ )

## Blog (Japanese)

I described the performance test in the following blog in more detail. 

[[Blog] About the OpenCV Tracking API (Japanese)](https://irohalog.hatenablog.com/entry/opencv_tracking_api )

## Performance Evaluation
### Evaluation Method
Evaluation was performed using 51 sequences from [Visual Tracker Benchmark](http://cvlab.hanyang.ac.kr/tracker_benchmark/benchmark_v10.html ). Since tracking results depend on the initial rectangular area, 20 sub-sequences with different tracking start times were generated for each of the 51 sequences according to the evaluation method of the Visual Tracker Benchmark, and 51 x 20 = 1020 sequences were used to track more than 300,000 frames. The tracking was performed for more than 300,000 frames using 51 sequences.

### Evaluation Metrics

1. Success plot (region overlap)  
Define the overlap score as ${S = \frac{|r_t \cap r_a |}{|r_t \cup r_a|} }$, where ${r_t}$ is the bounding box of the tracking result and ${r_a}$ is the correct bounding box. Success plot is a plot of the percentage of tracked results whose overlap score was higher than a certain threshold $S > \theta$ out of the total number of frames in all tested sequences.

2. Precision plot (center position error)  
It is an evaluation using the region's center position error. It plots the percentage of center position errors between the tracking bounding box and the correct bounding box that are smaller than a threshold value.

3. Processing Speed (fps)  
Average fps when tracked on a PC with Intel Core i7-5820K CPU (3.30 GHz), 32GB RAM.

### OpenCV Tracking API Performance of each algorithm
KCF performs well in terms of both tracking accuracy and processing speed.

* Success plot (region overlap)  
<a href="http://f.hatena.ne.jp/Ytra/20151220204437"><img src="http://img.f.hatena.ne.jp/images/fotolife/Y/Ytra/20151220/20151220204437.png" alt="20151220204437" width="300"></a>

* Precision plot (center position error)  
<a href="https://cdn-ak.f.st-hatena.com/images/fotolife/Y/Ytra/20151220/20151220204436.png"><img src="https://cdn-ak.f.st-hatena.com/images/fotolife/Y/Ytra/20151220/20151220204436.png" alt="20151220204436p" width="300"></a>

* Processing Speed (fps)   

| Boosting | MIL | TLD | MedianFlow | KCF 
|:-----------|:------------:|:------------:|:------------:|:------------:|
|27.6763 | 16.8790 | 9.7450 | 134.3183 | 80.5531


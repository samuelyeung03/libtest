
#include <iostream>

#include "./include/x264/x264.h"
#include "./include/x264/x264_config.h"
#include "stdint.h"
// memcpy
#include <string.h>
// Time Analysis
#include <chrono>
#include <vector>

#define QP 21  // Used when CQP
#define PRESET "superfast"
#define VBV 30
// When VBV is 1, the actual frame size should not exceed the target

using namespace std;
int main() {
  x264_param_t param_;
  x264_param_default_preset(&param_, PRESET, "zerolatency");
  param_.i_width = 1920;
  param_.i_height = 1080;
  int br = 3000;
  param_.rc.i_bitrate = br;
  param_.i_fps_den = 1;
  param_.i_fps_num = 30;
  param_.analyse.b_ssim = 1;
  param_.analyse.b_psnr = 1;
  x264_picture_t pic_;
  x264_picture_t pic_out_;
  x264_t* encoder_;
  x264_nal_t* nal_t_;
  // x264_param_t param_;

  int ret_val =
      x264_picture_alloc(&pic_, param_.i_csp, param_.i_width, param_.i_height);

  int frame_size = param_.i_width * param_.i_height * 3 / 2;
  uint8_t* yuv = new uint8_t[frame_size];

  int frame_num = 0;
  vector<int> frame_size_vec;
  vector<int> duration_vec;
  vector<int> diff_vec;
  vector<int> avg_diff_vec;
  vector<double> ssim_vec;
  vector<double> psnr_vec;
  // while (fread(yuv, 1, frame_size, file) == frame_size) {

  // temp YUV file
  FILE* temp_yuv = fopen("temp.yuv", "wb");

  int count_diff;
  double average_diff;

  int number_of_frames = 100;
  int numebers_of_params = 4;
  x264_param_t params_[] = {param_, param_, param_, param_, param_,
                            param_, param_, param_, param_, param_};
  //   params_[0].analyse.inter = X264_ANALYSE_I4x4 | X264_ANALYSE_I8x8;
  //   params_[1].analyse.inter =
  //       X264_ANALYSE_I4x4 | X264_ANALYSE_I8x8 | X264_ANALYSE_PSUB16x16;
  //   params_[2].analyse.inter = X264_ANALYSE_I4x4 | X264_ANALYSE_I8x8 |
  //                              X264_ANALYSE_PSUB16x16 |
  //                              X264_ANALYSE_BSUB16x16;
  //   params_[3].analyse.inter = X264_ANALYSE_I4x4 | X264_ANALYSE_I8x8 |
  //                              X264_ANALYSE_PSUB16x16 |
  //                              X264_ANALYSE_BSUB16x16 | X264_ANALYSE_PSUB8x8
  //                              | X264_ANALYSE_PSUB8x8;

  params_[0].i_frame_reference = 1;
  params_[1].i_frame_reference = 2;
  params_[2].i_frame_reference = 3;
  params_[3].i_frame_reference = 4;

    for (int param = 0; param < numebers_of_params; param++) {
    encoder_ = x264_encoder_open(&params_[param]);

    FILE* file = fopen("input.yuv", "rb");
    FILE* file_out = fopen("coded.h264", "wb");

    for (int i = 0; i < number_of_frames; i++) {
      fread(yuv, 1, frame_size, file);

      cout << "frame number:" << frame_num << "\r";

      pic_.img.plane[0] = yuv;
      pic_.img.plane[1] = yuv + param_.i_width * param_.i_height;
      pic_.img.plane[2] = yuv + param_.i_width * param_.i_height * 5 / 4;

      pic_.i_pts = frame_num;
      frame_num++;
      int i_nal;

      // time analysis
      auto start = chrono::high_resolution_clock::now();
      int i_frame_size =
          x264_encoder_encode(encoder_, &nal_t_, &i_nal, &pic_, &pic_out_);
      auto end = chrono::high_resolution_clock::now();

      // record ssim
      double ssim = pic_out_.prop.f_ssim;
      ssim_vec.push_back(ssim);

      // record psnr
      double psnr = pic_out_.prop.f_psnr_avg;
      psnr_vec.push_back(psnr);

      auto duration = chrono::duration_cast<chrono::microseconds>(end - start);
      // miliseconds

      int duration_ms = duration.count() / 1000;

      frame_size_vec.push_back(i_frame_size);
      cout << "real frame size:" << i_frame_size << endl;
      duration_vec.push_back(duration_ms);
      if (i_frame_size > 0) {
        for (int i = 0; i < i_nal; i++) {
          fwrite(nal_t_[i].p_payload, 1, nal_t_[i].i_payload, file_out);
        }
      }
    }
    // write the frame size to file
    FILE* file_size =
        fopen(("param_" + std::to_string(param) + "_size.json").c_str(), "w");
    fprintf(file_size, "[");
    for (int i = 0; i < frame_size_vec.size() - 1; i++) {
      fprintf(file_size, "%d,", frame_size_vec[i]);
    }
    fprintf(file_size, "%d]", frame_size_vec[frame_size_vec.size() - 1]);
    fclose(file_size);

    // write the Encoding duration to file
    FILE* file_duration = fopen(
        ("param_" + std::to_string(param) + "_duration.json").c_str(), "w");
    fprintf(file_duration, "[");
    for (int i = 0; i < duration_vec.size() - 1; i++) {
      fprintf(file_duration, "%d,", duration_vec[i]);
    }
    fprintf(file_duration, "%d]", duration_vec[duration_vec.size() - 1]);
    fclose(file_duration);

    // write the SSIM to file
    FILE* file_ssim =
        fopen(("param_" + std::to_string(param) + "_ssim.json").c_str(), "w");
    fprintf(file_ssim, "[");
    for (int i = 0; i < ssim_vec.size() - 1; i++) {
      fprintf(file_ssim, "%f,", ssim_vec[i]);
    }
    fprintf(file_ssim, "%f]", ssim_vec[ssim_vec.size() - 1]);
    fclose(file_ssim);

    // write the Encoding pnsr to file
    FILE* file_psnr =
        fopen(("param_" + std::to_string(param) + "_psnr.json").c_str(), "w");
    fprintf(file_psnr, "[");
    for (int i = 0; i < psnr_vec.size() - 1; i++) {
      fprintf(file_psnr, "%f,", psnr_vec[i]);
    }
    fprintf(file_psnr, "%f]", psnr_vec[psnr_vec.size() - 1]);
    fclose(file_psnr);

    // Clear the vectors for the next parameter set
    frame_size_vec.clear();
    duration_vec.clear();
    ssim_vec.clear();
    psnr_vec.clear();
  }

  x264_encoder_close(encoder_);
  return 0;
}
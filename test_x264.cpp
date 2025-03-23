
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

  //   params_[0].i_frame_reference = 1;
  //   params_[1].i_frame_reference = 2;
  //   params_[2].i_frame_reference = 3;
  //   params_[3].i_frame_reference = 4;

  //   params_[0].i_scenecut_threshold = 0;
  //   params_[1].i_scenecut_threshold = 40;
  //   params_[2].i_scenecut_threshold = 100;

  //   params_[0].b_deblocking_filter = 1;
  //   params_[1].b_deblocking_filter = 0;

  //   params_[0].i_deblocking_filter_alphac0 = 0;
  //   params_[0].i_deblocking_filter_beta = 0;
  //   params_[1].i_deblocking_filter_alphac0 = -3;
  //   params_[1].i_deblocking_filter_beta = -3;
  //   params_[2].i_deblocking_filter_alphac0 = 3;
  //   params_[2].i_deblocking_filter_beta = 3;
  //   params_[3].i_deblocking_filter_alphac0 = 6;
  //   params_[3].i_deblocking_filter_beta = 6;

  //   params_[0].i_frame_packing = 0;  // No frame packing
  //   params_[1].i_frame_packing = 3;  // Side-by-side
  //   params_[2].i_frame_packing = 4;  // Top-bottom
  //   params_[3].i_frame_packing = 5;  // Frame alternation

  //   params_[0].mastering_display = {0.3000, 0.6000, 0.1500, 0.0600, 0.6400,
  //                                   0.3300, 0.3127, 0.3290, 1000,   0.01};
  //   params_[1].mastering_display = {0.3127, 0.3290, 0.1500, 0.0600, 0.6400,
  //                                   0.3300, 0.3000, 0.6000, 500,    0.05};

  //   params_[0].content_light_level = {1000, 400};
  //   params_[1].content_light_level = {800, 300};
  //   params_[2].content_light_level = {1200, 500};

  //   params_[0].i_alternative_transfer = 0;  // Default
  //   params_[1].i_alternative_transfer = 1;  // BT.709
  //   params_[2].i_alternative_transfer = 2;  // BT.2020
  //   params_[3].i_alternative_transfer = 3;  // SMPTE 2084 (PQ)

  params_[0].analyse.inter = X264_ANALYSE_I4x4 | X264_ANALYSE_I8x8;
  params_[1].analyse.inter =
      X264_ANALYSE_I4x4 | X264_ANALYSE_I8x8 | X264_ANALYSE_PSUB16x16;
  params_[2].analyse.inter = X264_ANALYSE_I4x4 | X264_ANALYSE_I8x8 |
                             X264_ANALYSE_PSUB16x16 | X264_ANALYSE_BSUB16x16;
  params_[3].analyse.inter = X264_ANALYSE_I4x4 | X264_ANALYSE_I8x8 |
                             X264_ANALYSE_PSUB16x16 | X264_ANALYSE_BSUB16x16 |
                             X264_ANALYSE_PSUB8x8 | X264_ANALYSE_PSUB8x8;

  //   params_[0].analyse.intra = X264_ANALYSE_I4x4 | X264_ANALYSE_I8x8;
  //   params_[1].analyse.intra =
  //       X264_ANALYSE_I4x4 | X264_ANALYSE_I8x8 | X264_ANALYSE_PSUB16x16;

  //   params_[0].analyse.i_direct_mv_pred = X264_DIRECT_PRED_SPATIAL;  //
  //   Default params_[1].analyse.i_direct_mv_pred =
  //   X264_DIRECT_PRED_TEMPORAL; params_[2].analyse.i_direct_mv_pred =
  //   X264_DIRECT_PRED_AUTO;

  //   params_[0].analyse.i_me_range = 16;
  //   params_[1].analyse.i_me_range = 24;
  //   params_[2].analyse.i_me_range = 8;
  //   params_[3].analyse.i_me_range = ;

  //   params_[0].analyse.i_noise_reduction = 16;
  //   params_[1].analyse.i_noise_reduction = 24;
  //   params_[2].analyse.i_noise_reduction = ;
  //   params_[3].analyse.i_noise_reduction = ;

  //   params_[0].analyse.i_subpel_refine = 5;
  //   params_[1].analyse.i_subpel_refine = 7;
  //   params_[2].analyse.i_subpel_refine = 3;

  //   params_[0].analyse.i_trellis = 0;
  //   params_[1].analyse.i_trellis = 1;
  //   params_[2].analyse.i_trellis = 2;

  //   params_[0].analyse.b_chroma_me = 1;
  //   params_[1].analyse.b_chroma_me = 0;

  //   params_[0].analyse.b_dct_decimate = 1;
  //   params_[1].analyse.b_dct_decimate = 0;

  //   params_[0].analyse.b_fast_pskip = 1;
  //   params_[1].analyse.b_fast_pskip = 0;

  //   params_[0].analyse.b_mixed_references = 1;
  //   params_[1].analyse.b_mixed_references = 0;

  //   params_[0].analyse.f_psy_rd = 1.0;
  //   params_[1].analyse.f_psy_rd = 0.5;
  //   params_[2].analyse.f_psy_rd = 2.0;

  //   params_[0].analyse.f_psy_trellis = 0.0;
  //   params_[1].analyse.f_psy_trellis = 0.5;
  //   params_[2].analyse.f_psy_trellis = 1.0;

  //   params_[0].crop_rect = {0, 0, 0, 0};      // No cropping
  //   params_[1].crop_rect = {10, 10, 10, 10};  // Crop 10 pixels from
  //   all sides params_[2].crop_rect = {50, 0, 50, 0};  // Crop 50 pixels
  //   from left and right params_[3].crop_rect = {0, 20, 0, 20};  // Crop
  //   20 pixels from top and bottom

  //   params_[0].i_bframe_pyramid = 0;  // Disabled
  //   params_[1].i_bframe_pyramid = 1;  // Enabled

  //   params_[0].i_slice_max_size = 0;     // No limit
  //   params_[1].i_slice_max_size = 1500;  // Limit slice size to 1500
  //   bytes params_[2].i_slice_max_size = 2000;  // Limit slice size to
  //   2000 bytes params_[3].i_slice_max_size = 1000;  // Limit slice size
  //   to 1000 bytes

  //   params_[0].i_slice_max_mbs = 0;     // No limit
  //   params_[1].i_slice_max_mbs = 1200;  // Limit to 1200 macroblocks
  //   params_[2].i_slice_max_mbs = 800;   // Limit to 800 macroblocks
  //   params_[3].i_slice_max_mbs = 1600;  // Limit to 1600 macroblocks

  //   params_[0].i_slice_min_mbs = 0;    // No limit
  //   params_[1].i_slice_min_mbs = 100;  // Minimum of 100 macroblocks
  //   params_[2].i_slice_min_mbs = 200;  // Minimum of 200 macroblocks
  //   params_[3].i_slice_min_mbs = 50;   // Minimum of 50 macroblocks

  //   params_[0].i_slice_count = 0;  // No slicing
  //   params_[1].i_slice_count = 4;  // 4 slices per frame
  //   params_[2].i_slice_count = 8;  // 8 slices per frame
  //   params_[3].i_slice_count = 2;  // 2 slices per frame

  //   params_[0].i_slice_count_max = 0;   // No limit
  //   params_[1].i_slice_count_max = 16;  // Maximum of 16 slices
  //   params_[2].i_slice_count_max = 8;   // Maximum of 8 slices
  //   params_[3].i_slice_count_max = 4;   // Maximum of 4 slices

  //   params_[0].b_tff = 0;
  //   params_[1].b_tff = 1;

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
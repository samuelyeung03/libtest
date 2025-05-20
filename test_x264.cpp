#include <iostream>
#include <cstdlib>
#include <fstream>

#include "./include/x264/x264.h"
#include "./include/x264/x264_config.h"
#include "stdint.h"
// memcpy
#include <string.h>
// Time Analysis
#include <chrono>
#include <vector>
#include <filesystem>
#include <regex>

#define QP 21 // Used when CQP
#define PRESET "superfast"
#define VBV 30
// When VBV is 1, the actual frame size should not exceed the target

using namespace std;
int main(int argc, char *argv[])
{
  cout << "Starting program..." << endl;

  int width = 1920;  // Default width
  int height = 1080; // Default height
  int ssim = 1;      // Default SSIM value is 1
  int psnr = 1;      // Default PSNR value is 1
  int fps = 30;      // Default FPS value is 30
  int br = 3000;     // Default Bitrate value is 3000
  int runloops = 1;  // Default number of loops is 1
  int fixed_complexity = -1;
  int dace = 1;
  int number_of_frames = 900;
  string input_filename = "input.yuv";
  std::string output_dir = "result/";
  std::string output_filename = "";

  cout << "Parsing command-line arguments..." << endl;
  // Parse command-line arguments for width, height, and ssim
  for (int i = 1; i < argc; i++)
  {
    cout << "Processing argument: " << argv[i] << endl;
    if (std::string(argv[i]) == "--length" && i + 1 < argc)
    {
      number_of_frames = std::atoi(argv[++i]);
    }
    if (std::string(argv[i]) == "--width" && i + 1 < argc)
    {
      width = std::atoi(argv[++i]);
    }
    else if (std::string(argv[i]) == "--height" && i + 1 < argc)
    {
      height = std::atoi(argv[++i]);
    }
    else if (std::string(argv[i]) == "--fps" && i + 1 < argc)
    {
      fps = std::atoi(argv[++i]);
    }
    else if (std::string(argv[i]) == "--br" && i + 1 < argc)
    {
      br = std::atoi(argv[++i]);
    }
    else if (std::string(argv[i]) == "--runloops" && i + 1 < argc)
    {
      runloops = std::atoi(argv[++i]);
    }
    else if (std::string(argv[i]) == "--i" && i + 1 < argc)
    {
      input_filename = argv[++i];
    }
    else if (std::string(argv[i]) == "--ssim_disable")
    {
      ssim = 0;
    }
    else if (std::string(argv[i]) == "--psnr_disable")
    {
      psnr = 0;
    }
    else if (std::string(argv[i]) == "--c" && i + 1 < argc)
    {
      fixed_complexity = std::atoi(argv[++i]);
    }
    else if (std::string(argv[i]) == "--dace" && i + 1 < argc)
    {
      dace = std::atoi(argv[++i]);
    }
    else if (std::string(argv[i]) == "--dir" && i + 1 < argc)
    {
      output_dir = argv[++i];
    }
    else if (std::string(argv[i]) == "--name" && i + 1 < argc)
    {
      output_filename = argv[++i];
    }
  }

  if (output_filename.empty())
  {
    for (int i = 1; i < argc; i++)
    {
      output_filename += std::string(argv[i]);
      // Remove invalid characters for Linux filenames
      output_filename.erase(std::remove_if(output_filename.begin(), output_filename.end(),
                                           [](char c)
                                           {
                                             return !(isalnum(c) || c == '.' || c == '_' || c == '-');
                                           }),
                            output_filename.end());
    }
  }

  cout << "Initializing x264 parameters..." << endl;
  x264_param_t param_;
  x264_param_default_preset(&param_, PRESET, "ssim");

  // ture zero latency
  param_.rc.i_lookahead = 0;
  param_.i_sync_lookahead = 0;
  param_.i_bframe = 0;
  param_.b_sliced_threads = 1;
  param_.b_vfr_input = 0;
  param_.rc.b_mb_tree = 0;

  param_.i_width = width;
  param_.i_height = height;
  param_.rc.i_bitrate = br;
  param_.i_fps_den = 1;
  param_.i_fps_num = fps;
  param_.analyse.b_ssim = ssim;
  param_.analyse.b_psnr = psnr;
  param_.rc.i_aq_mode = 1;

  param_.rc.i_rc_method = X264_RC_ABR;

  param_.dace = dace;
  param_.dace_complexity_level = fixed_complexity;

  param_.i_threads = 12;
  param_.i_frame_total = 0;
  param_.i_keyint_max = 1500;
  param_.rc.i_vbv_max_bitrate = br;
  param_.rc.i_vbv_buffer_size = 0;
  // param_.i_bframe = 0;
  // param_.b_open_gop = 0;
  // param_.i_bframe_pyramid = 0;
  // param_.i_bframe_adaptive = X264_B_ADAPT_TRELLIS;

  param_.i_log_level = X264_LOG_DEBUG;

  param_.b_annexb = 1; // for start code 0,0,0,1
  param_.i_csp = X264_CSP_I420;

  param_.b_repeat_headers = 1; // sps, pps
  /* Apply profile restrictions. */
  x264_param_apply_profile(&param_, "baseline");

  cout << "Width: " << width << ", Height: " << height << ", FPS: " << fps << ", Bitrate: " << br << endl;

  cout << "Allocating picture..." << endl;
  x264_picture_t pic_;
  x264_picture_t pic_out_;
  x264_t *encoder_;
  x264_nal_t *nal_t_;
  // x264_param_t param_;

  int ret_val =
      x264_picture_alloc(&pic_, param_.i_csp, param_.i_width, param_.i_height);

  if (ret_val != 0)
  {
    cerr << "Error allocating picture!" << endl;
    return -1;
  }

  int frame_size = param_.i_width * param_.i_height * 3 / 2;
  uint8_t *yuv = new uint8_t[frame_size];

  int frame_num = 0;
  vector<int> frame_size_vec;
  vector<int> duration_vec;
  vector<int> diff_vec;
  vector<int> avg_diff_vec;
  vector<int> dace_duration_vec;
  vector<int> dace_complexity_vec;
  vector<double> ssim_vec;
  vector<double> psnr_vec;
  int overflow = 0;

  // while (fread(yuv, 1, frame_size, file) == frame_size) {

  // temp YUV file
  FILE *temp_yuv = fopen("temp.yuv", "wb");

  int count_diff;
  double average_diff;

  encoder_ = x264_encoder_open(&param_);

  system("mkdir -p result");

  cout << "Opening input file: " << input_filename << endl;
  FILE *file = fopen(input_filename.c_str(), "rb");
  if (!file)
  {
    cerr << "Error opening input file!" << endl;
    return -1;
  }

  std::filesystem::create_directories(output_dir);

  std::cout << "Output file path: " << (output_dir + output_filename + ".h264") << std::endl;
  FILE *file_out = fopen((output_dir + "/" + output_filename + ".h264").c_str(), "wb");
  if (!file_out)
  {
    cerr << "Error opening output file!, filename will be set to Default" << endl;
    output_filename = "output";
    file_out = fopen((output_dir + "/" + output_filename + ".h264").c_str(), "wb");
  }

  cout << "Starting encoding loop..." << endl;
  for (int loop = 0; loop < runloops; loop++)
  {
    cout << "Run loop " << loop + 1 << " of " << runloops << endl;

    // Reset frame-related vectors for each loop
    vector<int> temp_frame_size_vec;
    vector<int> temp_duration_vec;
    vector<double> temp_ssim_vec;
    vector<double> temp_psnr_vec;
    vector<int> temp_dace_complexity_vec;

    frame_num = 0;
    rewind(file); // Reset file pointer to the beginning

    for (int i = 0; i < number_of_frames; i++)
    {
      cout << "Reading frame " << i << endl;
      fread(yuv, 1, frame_size, file);

      cout << "frame number:" << frame_num << "\r";

      pic_.img.plane[0] = yuv;
      pic_.img.plane[1] = yuv + param_.i_width * param_.i_height;
      pic_.img.plane[2] = yuv + param_.i_width * param_.i_height * 5 / 4;

      pic_.i_pts = frame_num;
      frame_num++;
      int i_nal;

      cout << "Encoding frame " << frame_num << endl;

      // time analysis
      auto start = chrono::high_resolution_clock::now();
      int i_frame_size =
          x264_encoder_encode(encoder_, &nal_t_, &i_nal, &pic_, &pic_out_);
      auto end = chrono::high_resolution_clock::now();

      if (i_frame_size < 0)
      {
        cerr << "Error encoding frame " << frame_num << endl;
        break;
      }

      cout << "Frame size: " << i_frame_size << ", Duration: " << chrono::duration_cast<chrono::microseconds>(end - start).count() << " microseconds" << endl;

      // record ssim
      double ssim = pic_out_.prop.f_ssim;
      temp_ssim_vec.push_back(ssim);

      // record psnr
      double psnr = pic_out_.prop.f_psnr_avg;
      temp_psnr_vec.push_back(psnr);

      // record DACE complexity
      int dace_complexity = pic_out_.prop.DACE_complexity;
      temp_dace_complexity_vec.push_back(dace_complexity);

      int duration = chrono::duration_cast<chrono::microseconds>(end - start).count();

      if (duration > 1000000/fps)
      {
        overflow ++;
      }
      temp_frame_size_vec.push_back(i_frame_size);
      temp_duration_vec.push_back(duration);

      if (i_frame_size > 0)
      {
        for (int i = 0; i < i_nal; i++)
        {
          fwrite(nal_t_[i].p_payload, 1, nal_t_[i].i_payload, file_out);
        }
      }
    }

    // Accumulate results for averaging
    for (size_t i = 0; i < temp_frame_size_vec.size(); i++)
    {
      if (frame_size_vec.size() <= i)
      {
        frame_size_vec.push_back(0);
        duration_vec.push_back(0);
        if (ssim)
          ssim_vec.push_back(0.0);
        if (psnr)
          psnr_vec.push_back(0.0);
        dace_complexity_vec.push_back(0);
      }
      frame_size_vec[i] += temp_frame_size_vec[i];
      duration_vec[i] += temp_duration_vec[i];
      if (ssim)
        ssim_vec[i] += temp_ssim_vec[i];
      if (psnr)
        psnr_vec[i] += temp_psnr_vec[i];
      dace_complexity_vec[i] += temp_dace_complexity_vec[i];
    }
  }

  // Calculate averages
  for (size_t i = 0; i < frame_size_vec.size(); i++)
  {
    frame_size_vec[i] /= runloops;
    duration_vec[i] /= runloops;
    if (ssim)
      ssim_vec[i] /= runloops;
    if (psnr)
      psnr_vec[i] /= runloops;
    dace_complexity_vec[i] /= runloops;
  }

  cout << "Writing output to " << output_dir + "/" + output_filename << endl;
  std::ofstream json_file(output_dir + "/" + output_filename + ".json");
  if (!json_file.is_open())
  {
    cerr << "Error: Could not open file " << output_filename << " for writing." << endl;
    return -1;
  }
  json_file << "{" << endl;

  // Write frame sizes
  json_file << "  \"frame_sizes\": [";
  for (size_t i = 0; i < frame_size_vec.size(); i++)
  {
    json_file << frame_size_vec[i];
    if (i < frame_size_vec.size() - 1)
      json_file << ", ";
  }
  json_file << "]," << endl;

  // Write durations
  json_file << "  \"durations\": [";
  for (size_t i = 0; i < duration_vec.size(); i++)
  {
    json_file << duration_vec[i];
    if (i < duration_vec.size() - 1)
      json_file << ", ";
  }
  json_file << "]" << endl;

  // Write SSIM if enabled
  if (ssim)
  {
    json_file << "  ,\"ssim\": [";
    for (size_t i = 0; i < ssim_vec.size(); i++)
    {
      json_file << ssim_vec[i];
      if (i < ssim_vec.size() - 1)
        json_file << ", ";
    }
    json_file << "]" << endl;
  }

  // Write PSNR if enabled
  if (psnr)
  {
    json_file << "  ,\"psnr\": [";
    for (size_t i = 0; i < psnr_vec.size(); i++)
    {
      json_file << psnr_vec[i];
      if (i < psnr_vec.size() - 1)
        json_file << ", ";
    }
    json_file << "]" << endl;
  }

  // Write DACE complexity
  json_file << "  ,\"dace_complexity\": [";
  for (size_t i = 0; i < dace_complexity_vec.size(); i++)
  {
    json_file << dace_complexity_vec[i];
    if (i < dace_complexity_vec.size() - 1)
      json_file << ", ";
  }
  json_file << "]" << endl;

  // Write overflow
  json_file << "  ,\"overflow\": [";
  json_file << overflow;
  json_file << "]" << endl;  

  // Uncomment and write DACE durations if needed
  // json_file << "  ,\"dace_duration\": [";
  // for (size_t i = 0; i < dace_duration_vec.size(); i++)
  // {
  //     json_file << dace_duration_vec[i];
  //     if (i < dace_duration_vec.size() - 1)
  //         json_file << ", ";
  // }
  // json_file << "]" << endl;

  json_file << "}" << endl;
  json_file.close();

  // Clear the vectors for the next parameter set
  frame_size_vec.clear();
  duration_vec.clear();
  ssim_vec.clear();
  psnr_vec.clear();

  cout << "Closing encoder and cleaning up..." << endl;
  x264_encoder_close(encoder_);
  fclose(file);
  fclose(file_out);

  cout << "Program finished successfully." << endl;
  return 0;
}
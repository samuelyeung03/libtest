
#include "stdint.h"
#include "x264.h"
#include "x264_config.h"
#include <iostream>
//memcpy
#include <string.h>
// Time Analysis
#include <chrono>
#include <vector>

#define QP 21 // Used when CQP
#define PRESET "superfast"
#define VBV 30   // When VBV is 1, the actual frame size should not exceed the target

#define CHANGE 1 // Change bitrate in half way

using namespace std;
int main() {
    x264_param_t param_;
    x264_param_default_preset(&param_, PRESET, "zerolatency");

    param_.i_threads = 2;
    param_.b_sliced_threads = 1;
    param_.i_width = 1920;
    param_.i_height = 1080;
    param_.i_frame_total = 0;  
    param_.i_keyint_max = 1500;

    // Rate Control Method
    // // CQP
    // param_.rc.i_rc_method = X264_RC_CQP;
    // param_.rc.i_qp_constant = QP;

    // // CRF
    // param_.rc.i_rc_method = X264_RC_CRF;
    // param_.rc.f_rf_constant = 25;

    // ABR
    param_.rc.i_rc_method = X264_RC_ABR;
    int br = 3000;
    param_.rc.i_bitrate = br;  

    param_.rc.i_vbv_max_bitrate = br; //  3000
    param_.rc.i_vbv_buffer_size = br / 30 * VBV; // kbit / 8 * 1000 = byte  // 100
    // param_.rc.b_filler = 1;



    // param_.i_bframe = 0;
    // param_.b_open_gop = 0;
    // param_.i_bframe_pyramid = 0;
    // param_.i_bframe_adaptive = X264_B_ADAPT_TRELLIS;

    // param_.i_log_level = X264_LOG_DEBUG;
    param_.i_fps_den = 1;
    param_.i_fps_num = 30;

    // param_.b_annexb = 1;  // for start code 0,0,0,1
    param_.i_csp = X264_CSP_I420;

    // param_.b_vfr_input = 0;
    // param_.b_repeat_headers = 1;  // sps, pps



    param_.b_cabac = 1;  // 0 for CAVLC， 1 for higher complexity

    x264_picture_t pic_;
    x264_picture_t pic_out_;
    x264_t *encoder_;
    x264_nal_t *nal_t_;
    // x264_param_t param_;

    int ret_val = x264_picture_alloc(&pic_, param_.i_csp, param_.i_width, param_.i_height);


    encoder_ = x264_encoder_open(&param_);


    FILE *file = fopen("/mnt/md3/xiangjie/youtubevideos/evaluation_video/bathsong_coded.yuv", "rb");
    FILE *file_out = fopen("coded.h264", "wb");


    int frame_size = param_.i_width * param_.i_height * 3 / 2;
    uint8_t *yuv = new uint8_t[frame_size];

    int frame_num = 0;
    vector<int> frame_size_vec;
    vector<int> duration_vec;
    vector<int> diff_vec;
    vector<int> avg_diff_vec;
    // while (fread(yuv, 1, frame_size, file) == frame_size) {

    // temp YUV file
    FILE *temp_yuv = fopen("/mnt/md3/xiangjie/temp.yuv", "wb");

    int count_diff;
    double average_diff;
    
    for (int i = 0; i < 2000; i++) {



#if CHANGE 
        if (i == 300){
            br = 1500;
            cout << "Reconfig" << endl;
            param_.rc.i_bitrate = br;
            param_.rc.i_vbv_max_bitrate = br; //  3000
            param_.rc.i_vbv_buffer_size = br / 30 * VBV; // kbit / 8 * 1000 = byte  // 100

            // param_.analyse.i_trellis = 1;
            // param_.analyse.inter = X264_ANALYSE_I4x4 | X264_ANALYSE_I8x8
            //             | X264_ANALYSE_PSUB16x16 | X264_ANALYSE_BSUB16x16;
            // param_.analyse.i_subpel_refine = 4;           
            // param_.analyse.i_me_method = X264_ME_HEX;  
            // param_.i_frame_reference = 2;
            x264_encoder_reconfig(encoder_, &param_);
        }

#endif
  
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
        int i_frame_size = x264_encoder_encode(encoder_, &nal_t_, &i_nal, &pic_, &pic_out_);
        auto end = chrono::high_resolution_clock::now();

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
    FILE *file_size = fopen("size.json", "w");
    fprintf(file_size, "[");
    for (int i = 0; i < frame_size_vec.size() - 1; i++) {
        fprintf(file_size, "%d,", frame_size_vec[i]);
    }
    fprintf(file_size, "%d]", frame_size_vec[frame_size_vec.size() - 1]);

    // write the Encoding duration to file
    FILE *file_duration = fopen("duration.json", "w");
    fprintf(file_duration, "[");
    for (int i = 0; i < duration_vec.size() - 1; i++) {
        fprintf(file_duration, "%d,", duration_vec[i]);
    }
    fprintf(file_duration, "%d]", duration_vec[duration_vec.size() - 1]);

    
    x264_encoder_close(encoder_);
    return 0;
}
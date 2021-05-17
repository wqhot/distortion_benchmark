#pragma once
#define BEFORE_TRANS_COLS 1920
#define BEFORE_TRANS_ROWS 720
#define AFTER_TRANS_COLS 1920
#define AFTER_TRANS_ROWS 1080
#define FINAL_COLS 2160
#define FINAL_ROWS 1920
#define PACK_SIZE 4096
#define BUF_LEN 66460
#define ATLAS_SEND_COLS 416
#define ATLAS_SEND_ROWS 416

#define GROUP_IP_ADDRES "192.168.50.218"
#define TV_IP_ADDRESS "192.168.50.252"
#define ATLAS_IP_ADDRESS "192.168.50.137"
#define VINS_IP_ADDRESS "192.168.50.61"

#define MOVE_Y_OFFSET 128

#define SIG_FV_AV_FINISH 28

struct image_process_settings_
{
    int output_width;
    int output_height;
    int output_row_offset;
    int output_col_offset;
    bool output_trans;
    bool output_copy;
    double output_scale;
};

extern image_process_settings_ settings_;
// #define BEFORE_TRANS_COLS 1920
// #define BEFORE_TRANS_COLS 1920
// #define BEFORE_TRANS_COLS 1920
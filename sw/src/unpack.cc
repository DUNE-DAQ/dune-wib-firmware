#include "unpack.h"

#include <cstdio>

void unpack14(const uint32_t *packed, uint16_t *unpacked) {
    for (size_t i = 0; i < 128; i++) { // i == n'th U,V,X value
        const size_t low_bit = i*14;
        const size_t low_word = low_bit / 32;
        const size_t high_bit = (i+1)*14-1;
        const size_t high_word = high_bit / 32;
        //printf("word %li :: low %li (%li[%li]) high %li (%li[%li])\n",i,low_bit,low_word,low_bit%32,high_bit,high_word,high_bit%32);
        if (low_word == high_word) { //all the bits are in the same word
            unpacked[i] = (packed[low_word] >> (low_bit%32)) & 0x3FFF;
        } else { //some of the bits are in the next word
            size_t high_off = high_word*32-low_bit;
            //printf("pre_mask 0x%X post_mask 0x%X\n", (0x3FFF >> (14-high_off)), ((0x3FFF << high_off) & 0x3FFF) );
            unpacked[i] = (packed[low_word] >> (low_bit%32)) & (0x3FFF >> (14-high_off));
            unpacked[i] |= (packed[high_word] << high_off) & ((0x3FFF << high_off) & 0x3FFF);
        }
    }
}

void repack14(const uint16_t *unpacked, uint32_t *packed) {
    //zero packed data first
    for (size_t i = 0; i < 56; i++) packed[i] = 0;
    for (size_t i = 0; i < 128; i++) { // i == n'th U,V,X value
        const size_t low_bit = i*14;
        const size_t low_word = low_bit / 32;
        const size_t high_bit = (i+1)*14-1;
        const size_t high_word = high_bit / 32;
        //printf("word %li :: low %li (%li[%li]) high %li (%li[%li])\n",i,low_bit,low_word,low_bit%32,high_bit,high_word,high_bit%32);
        if (low_word == high_word) { //all the bits are in the same word
            packed[low_word] |= (unpacked[i] & 0x3FFF) << (low_bit%32);
        } else { //some of the bits are in the next word
            size_t high_off = high_word*32-low_bit;
            //printf("pre_mask 0x%X post_mask 0x%X\n", (0x3FFF >> (14-high_off)), ((0x3FFF << high_off) & 0x3FFF) );
            packed[low_word] |= (unpacked[i] & (0x3FFF >> (14-high_off))) << (low_bit%32);
            packed[high_word] |= (unpacked[i] & ((0x3FFF << high_off) & 0x3FFF)) >> high_off;
        }
    }
}

void unpack_frame(const frame14 *frame, frame14_unpacked *data) {
    data->crate_num = frame->wib_pre[0] & 0xFF;
    data->frame_version = (frame->wib_pre[0] >> 8) & 0xF;
    data->wib_num = (frame->wib_pre[0] >> 12) & 0x7;
    data->fiber_num = (frame->wib_pre[0] >> 15) & 0x1;
    data->femb_valid = (frame->wib_pre[0] >> 16) & 0x3;
    data->link_mask = (frame->wib_pre[0] >> 18) & 0xFF;
    
    data->wib_data = frame->wib_pre[1];
    
    data->timestamp = (((uint64_t)frame->wib_pre[3])<<32) | ((uint64_t)frame->wib_pre[2]);
    
    unpack14(frame->femb_a_seg,(uint16_t*)&data->femb[0]);
    unpack14(frame->femb_b_seg,(uint16_t*)&data->femb[1]);
    
    data->crc20 = frame->wib_post[0] & 0xFFFFF;
    data->flex12 = (frame->wib_post[0] >> 20) & 0xFFF;
    data->flex24 = (frame->wib_post[1] >> 8) & 0xFFFFFF;
}

size_t u_to_ch[40] = {20, 59, 19, 60, 18, 61, 17, 62, 16, 63, 4, 43, 3, 44, 2, 
                      45, 1, 46, 0, 47, 68, 107, 67, 108, 66, 109, 65, 110, 64, 
                      111, 84, 123, 83, 124, 82, 125, 81, 126, 80, 127};
size_t v_to_ch[40] = {25, 54, 24, 55, 23, 56, 22, 57, 21, 58, 9, 38, 8, 39, 7, 
                      40, 6, 41, 5, 42, 73, 102, 72, 103, 71, 104, 70, 105, 69,
                      106, 89, 118, 88, 119, 87, 120, 86, 121, 85, 122};
size_t x_to_ch[48] = {31, 48, 30, 49, 29, 50, 28, 51, 27, 52, 26, 53, 15, 32,
                      14, 33, 13, 34, 12, 35, 11, 36, 10, 37, 79, 96, 78, 97, 
                      77, 98, 76, 99, 75, 100, 74, 101, 95, 112, 94, 113, 93, 
                      114, 92, 115, 91, 116, 90, 117};

void deframe_data(const frame14 *frame_buf, size_t nframes, channel_data &data) {
    data.samples = nframes;
    for (size_t i = 0; i < 128; i++) {
        data.channels[0][i].resize(nframes);
        data.channels[1][i].resize(nframes);
    }
    data.timestamp.resize(nframes);
    frame14_unpacked frame_data;
    for (size_t i = 0; i < nframes; i++) {
        unpack_frame(frame_buf+i,&frame_data);
        for (size_t j = 0; j < 48; j++) {
            int k;
            if (j < 40) {
                k = u_to_ch[j];
                data.channels[0][k][i] = frame_data.femb[0].u[j];
                data.channels[1][k][i] = frame_data.femb[1].u[j];
                k = v_to_ch[j];
                data.channels[0][k][i] = frame_data.femb[0].v[j];
                data.channels[1][k][i] = frame_data.femb[1].v[j];
            } 
            k = x_to_ch[j];
            data.channels[0][k][i] = frame_data.femb[0].x[j];
            data.channels[1][k][i] = frame_data.femb[1].x[j];
        }
        data.timestamp[i] = frame_data.timestamp;;
    }
    data.crate_num = frame_data.crate_num;
    data.wib_num = frame_data.wib_num;
}

void deframe_data(const frame14 *frame_buf, size_t nframes, uvx_data &data) {
    data.samples = nframes;
    for (size_t i = 0; i < 48; i++) {
        if (i < 40) {
            data.u[0][i].resize(nframes);
            data.v[0][i].resize(nframes);
            data.u[1][i].resize(nframes);
            data.v[1][i].resize(nframes);
        }
        data.x[0][i].resize(nframes);
        data.x[1][i].resize(nframes);
    }
    data.timestamp.resize(nframes);
    frame14_unpacked frame_data;
    for (size_t i = 0; i < nframes; i++) {
        unpack_frame(frame_buf+i,&frame_data);
        for (size_t j = 0; j < 48; j++) {
            if (j < 40) {
                data.u[0][j][i] = frame_data.femb[0].u[j];
                data.v[0][j][i] = frame_data.femb[0].v[j];
                data.u[1][j][i] = frame_data.femb[1].u[j];
                data.v[1][j][i] = frame_data.femb[1].v[j];
            }
            data.x[0][j][i] = frame_data.femb[0].x[j];
            data.x[1][j][i] = frame_data.femb[1].x[j];
        }
        data.timestamp[i] = frame_data.timestamp;;
    }
    data.crate_num = frame_data.crate_num;
    data.wib_num = frame_data.wib_num;
}

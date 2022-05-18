#ifndef _unpack_h
#define _unpack_h

#include <cstddef>
#include <stdint.h>
#include <vector>
#include "log.h"

// Suppress warnings from addressing word-aligned data in packed structs
#pragma GCC diagnostic ignored "-Waddress-of-packed-member"

// Words in the binary format of the FELIX frame14 from the WIB
typedef struct {
    uint32_t start_frame;
    uint32_t wib_pre[4];
    uint32_t femb_a_seg[56];
    uint32_t femb_b_seg[56];
    uint32_t wib_post[2];
    uint32_t idle_frame;
} __attribute__ ((packed)) frame14;

// Bitfields in the binary format of the Frame frame14 from the WIB v1
// frame_version == 1
typedef struct {
    uint32_t start_frame;
    uint32_t crate_num : 8, frame_version: 4, slot_num : 3, fiber_num : 1;
    uint32_t femb_valid : 2, link_mask : 8, reserved : 6;
    uint32_t wib_data;
    uint64_t timestamp;
    uint32_t femb_a_seg[56];
    uint32_t femb_b_seg[56];
    uint32_t crc20 : 20, flex12 : 12;
    uint32_t eof: 8, flex24 : 24;
    uint32_t idle_frame;
} __attribute__ ((packed)) frame14_bitfield_v1;

static_assert(sizeof(frame14) == sizeof(frame14_bitfield_v1),"Frame14 packed datatypes inconsistent");

// Bitfields in the binary format of the Frame frame14 from the WIB v2
// frame_version == 2
typedef struct {
    uint32_t start_frame;
    uint32_t crate_num : 8, frame_version: 4, slot_num : 3, fiber_num : 1;
    uint32_t femb_valid : 2, link_mask : 8, loss_of_lock : 1, reserved : 5;
    uint32_t femb_a_link_0_ts : 8, femb_a_link_1_ts : 8, femb_a_link_2_ts : 8, femb_a_link_3_ts : 8;
    uint64_t timestamp;
    uint32_t femb_a_seg[56];
    uint32_t femb_b_seg[56];
    uint32_t femb_b_link_0_ts : 8, femb_b_link_1_ts : 8, femb_b_link_2_ts : 8, femb_b_link_3_ts : 8;
    uint32_t eof: 8, crc20 : 20, felix_reserved : 4;
    uint32_t idle_frame;
} __attribute__ ((packed)) frame14_bitfield_v2;

static_assert(sizeof(frame14) == sizeof(frame14_bitfield_v2),"Frame14 packed datatypes inconsistent");

// Bitfields in the binary format of the Frame frame14 from the COLDATA P3
// frame_version == 3
typedef struct {
    uint32_t start_frame;
    uint32_t wib_data;
    uint64_t timestamp;
    uint32_t crate_num : 8, frame_version: 4, slot_num : 3, fiber_num : 1;
    uint32_t femb_valid : 2, link_mask : 8, reserved : 6;
    uint32_t femb_a_seg[56];
    uint32_t femb_b_seg[56];
    uint32_t crc20 : 20, flex12 : 12;
    uint32_t eof: 8, flex24 : 24;
    uint32_t idle_frame;
} __attribute__ ((packed)) frame14_bitfield_v3;

static_assert(sizeof(frame14) == sizeof(frame14_bitfield_v3),"Frame14 packed datatypes inconsistent");

// Samples from the U, V, X channels in a femb_*_seg of a frame as 16bit arrays
typedef struct {    
    uint16_t u[40],v[40],x[48];
} __attribute__ ((packed)) femb_data;

// Deframed data, where channels or u,v,x are time ordered uint16 samples for each channel
typedef struct {
    size_t samples;
    std::vector<uint16_t> channels[2][128];
    std::vector<uint64_t> timestamp;
    uint8_t crate_num, wib_num; 
} channel_data;
typedef struct {
    size_t samples;
    std::vector<uint16_t> u[2][40];
    std::vector<uint16_t> v[2][40];
    std::vector<uint16_t> x[2][48];
    std::vector<uint64_t> timestamp;
    uint8_t crate_num, wib_num; 
} uvx_data;

// how U,V,X numbers map to channels on a single FEMB
static size_t u_to_ch[40] = {20, 59, 19, 60, 18, 61, 17, 62, 16, 63, 4, 43, 3, 44, 2, 
                      45, 1, 46, 0, 47, 68, 107, 67, 108, 66, 109, 65, 110, 64, 
                      111, 84, 123, 83, 124, 82, 125, 81, 126, 80, 127};
static size_t v_to_ch[40] = {25, 54, 24, 55, 23, 56, 22, 57, 21, 58, 9, 38, 8, 39, 7, 
                      40, 6, 41, 5, 42, 73, 102, 72, 103, 71, 104, 70, 105, 69,
                      106, 89, 118, 88, 119, 87, 120, 86, 121, 85, 122};
static size_t x_to_ch[48] = {31, 48, 30, 49, 29, 50, 28, 51, 27, 52, 26, 53, 15, 32,
                      14, 33, 13, 34, 12, 35, 11, 36, 10, 37, 79, 96, 78, 97, 
                      77, 98, 76, 99, 75, 100, 74, 101, 95, 112, 94, 113, 93, 
                      114, 92, 115, 91, 116, 90, 117};

// Converts 14 bit packed channel data (56 uint32 words from the WIB) to byte-aligned 16 bit arrays (128 uint16 values)
void unpack14(const uint32_t *packed, uint16_t *unpacked);

// Converts byte-aligned 16 bit arrays (128 uint16 values) to 14 bit packed channel data (56 uint32 words from the WIB)
void repack14(const uint16_t *unpacked, uint32_t *packed);

// Converts a buffer of felix_frame in memory to time ordered data from each channel
void deframe_data(const frame14 *frame_buf, size_t nframes, channel_data &data, uint8_t version=2);

// Converts a buffer of felix_frame in memory to time ordered data from each U,V,X mapping
void deframe_data(const frame14 *frame_buf, size_t nframes, uvx_data &data, uint8_t version=3);

// Convert time ordered data from each channel back into frame14 dataWW
void reframe_data(frame14 *frame_buf, size_t nframes, const channel_data &data, uint8_t version=2);

// Fill a buffer of frame14 with fake data sin on even (cos on odd) period 10+2*ch samples
void fake_data(frame14 *buffer, size_t nframes);

#endif

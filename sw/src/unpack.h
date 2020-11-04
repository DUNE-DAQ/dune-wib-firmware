#ifndef _unpack_h
#define _unpack_h

#include <cstddef>
#include <stdint.h>
#include <vector>

// Words in the binary format of the FELIX frames from the WIB
typedef struct {
    uint32_t start_frame;
    uint32_t wib_pre[4];
    uint32_t femb_a_seg[56];
    uint32_t femb_b_seg[56];
    uint32_t wib_post[2];
    uint32_t idle_frame;
} __attribute__ ((packed)) felix_frame;

// Samples from the U, V, X channels in a femb_*_seg of a frame as 16bit arrays
typedef struct {    
    uint16_t u[40],v[40],x[48];
} __attribute__ ((packed)) femb_data;

// Byte-aligned unpacked values in a felix_frame
typedef struct {
    uint8_t link_mask, femb_valid, fiber_num, wib_num, frame_version, crate_num;
    uint32_t wib_data;
    uint64_t timestamp;
    femb_data femb[2];
    uint32_t crc20;
    uint16_t flex12;
    uint32_t flex24;
} felix_data;

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


// Currently just returns true
bool verify_frame(const felix_frame *frame);

// Unpacks the data in felix_frame struct to a felix_data struct
void unpack_frame(const felix_frame *frame, felix_data *data);

// Converts a buffer of felix_frame in memory to a series of samples from each channel
void deframe_data(const felix_frame *frame_buf, size_t size, channel_data &data);

// Converts a buffer of felix_frame in memory to a series of samples from each U,V,X mapping
void deframe_data(const felix_frame *frame_buf, size_t size, uvx_data &data);

#endif

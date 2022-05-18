#include "unpack.h"

#include <cstdio>
#include <cstring>

void unpack14(const uint32_t *packed, uint16_t *unpacked) {
    for (size_t i = 0; i < 128; i++) { // i == n'th U,V,X value
        const size_t low_bit = (i*14);
        const size_t low_word = low_bit / 32;
        const size_t high_bit = (i+1)*14-1;
        const size_t high_word = high_bit / 32;
//        glog.log("word %li :: low %li (%li[%li]) high %li (%li[%li])\n",i,low_bit,low_word,low_bit%32,high_bit,high_word,high_bit%32);
//	glog.log("Value of low word is %lx, value of high word is %lx\n",packed[low_word],packed[high_word]);
        if (low_word == high_word) { //all the bits are in the same word
            unpacked[i] = (packed[low_word] >> (low_bit%32)) & 0x3FFF;
	   // if (i == 0) {
	//	glog.log("1Channel 0 data is %u\n",(unpacked[i]));
	//	glog.log("However bitshifting by 6 places gives %u\n",(packed[low_word] >> 6) & 0x3FFF);
	//	unpacked[i] = (packed[low_word] >> 6) & 0x3FFF;
//				}
        } else { //some of the bits are in the next word
            size_t high_off = high_word*32-low_bit;
            //glog.log("pre_mask 0x%X post_mask 0x%X\n", (0x3FFF >> (14-high_off)), ((0x3FFF << high_off) & 0x3FFF) );
            unpacked[i] = (packed[low_word] >> (low_bit%32)) & (0x3FFF >> (14-high_off));
            unpacked[i] |= (packed[high_word] << high_off) & ((0x3FFF << high_off) & 0x3FFF);
        }
//	glog.log("Final value is %lx\n",unpacked[i]);
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
        //glog.log("word %li :: low %li (%li[%li]) high %li (%li[%li])\n",i,low_bit,low_word,low_bit%32,high_bit,high_word,high_bit%32);
        if (low_word == high_word) { //all the bits are in the same word
            packed[low_word] |= (unpacked[i] & 0x3FFF) << (low_bit%32);
        } else { //some of the bits are in the next word
            size_t high_off = high_word*32-low_bit;
            //glog.log("pre_mask 0x%X post_mask 0x%X\n", (0x3FFF >> (14-high_off)), ((0x3FFF << high_off) & 0x3FFF) );
            packed[low_word] |= (unpacked[i] & (0x3FFF >> (14-high_off))) << (low_bit%32);
            packed[high_word] |= (unpacked[i] & ((0x3FFF << high_off) & 0x3FFF)) >> high_off;
        }
    }
}

void unpack_frame(const frame14 *frame, femb_data *femb_a, femb_data *femb_b) {
    for (size_t i = 0; i < 1; i++){
//    glog.log("Frame 1 stats:\n Start frame=%lx\n Crate_num line=%lx\n FEMB line=%lx\n WIB_data=%lx\n Timestamp=%lx\n Data1=%lx\n Data2=%lx\n Crc=%lx\n EOF=%lx\n Idle=%lx\n"
//		    ,frame->start_frame, frame->wib_pre[0], frame->wib_pre[1], frame->wib_pre[2], frame->wib_pre[3], 
//		    frame->femb_a_seg[0], frame->femb_a_seg[1], frame->wib_post[0], frame->wib_post[1], frame->idle_frame);
    }
    unpack14(frame->femb_a_seg,(uint16_t*)femb_a);
    unpack14(frame->femb_b_seg,(uint16_t*)femb_b);
}

void repack_frame(const femb_data *femb_a, const femb_data *femb_b, frame14 *frame) {
    repack14((uint16_t*)femb_a,frame->femb_a_seg);
    repack14((uint16_t*)femb_b,frame->femb_b_seg);
}

void deframe_data(const frame14 *frame_buf, size_t nframes, channel_data &data, uint8_t version) {
//    glog.log("unpack.cc, deframe_data, Version is %lx\n",version);
    data.samples = nframes;
    for (size_t i = 0; i < 128; i++) {
        data.channels[0][i].resize(nframes);
        data.channels[1][i].resize(nframes);
    }
    data.timestamp.resize(nframes);
    femb_data femb_a, femb_b;
//    for (size_t i = 0; i < 1; i++) {
    for (size_t i = 0; i < nframes; i++) {
        unpack_frame(frame_buf+i,&femb_a,&femb_b);
        for (size_t j = 0; j < 48; j++) {
            int k;
            if (j < 40) {
                k = u_to_ch[j];
                data.channels[0][k][i] = femb_a.u[j];
                data.channels[1][k][i] = femb_b.u[j];
                k = v_to_ch[j];
                data.channels[0][k][i] = femb_a.v[j];
                data.channels[1][k][i] = femb_b.v[j];
            } 
            k = x_to_ch[j];
            data.channels[0][k][i] = femb_a.x[j];
            data.channels[1][k][i] = femb_b.x[j];
        }
	if (i > 2120){
//		glog.log("Frame %llu\n", i);
//		for (size_t j =0; j < 128; j++){
//			glog.log("Data is %lx\n", data.channels[0][i]);
//		}
	}
        switch (version) {
            case 1: {
                frame14_bitfield_v1 *frame = (frame14_bitfield_v1*)(frame_buf+i);
                data.timestamp[i] = frame->timestamp;
                break;
            }
            case 2: {
                frame14_bitfield_v2 *frame = (frame14_bitfield_v2*)(frame_buf+i);
                data.timestamp[i] = frame->timestamp;
                break;
            }
	    case 3: {
		frame14_bitfield_v3 *frame = (frame14_bitfield_v3*)(frame_buf+i);
		data.timestamp[i] = frame->timestamp;
		break;
	    }
        }
        
    }
    switch (version) {
        case 1: {
            frame14_bitfield_v1 *frame = (frame14_bitfield_v1*)(frame_buf);
            data.crate_num = frame->crate_num;
            data.wib_num = frame->slot_num;
            break;
        }
        case 2: {
            frame14_bitfield_v2 *frame = (frame14_bitfield_v2*)(frame_buf);
            data.crate_num = frame->crate_num;
            data.wib_num = frame->slot_num;
            break;
        }
	case 3: {
	    frame14_bitfield_v3 *frame = (frame14_bitfield_v3*)(frame_buf);
	    data.crate_num = frame->crate_num;
	    data.wib_num = frame->slot_num;
	    break;
	}
    }
}

void deframe_data(const frame14 *frame_buf, size_t nframes, uvx_data &data, uint8_t version) {
    //glog.log("unpack.cc, deframe_data, Version is %lx\n",version);
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
    femb_data femb_a, femb_b;
    for (size_t i = 0; i < nframes; i++) {
        unpack_frame(frame_buf+i,&femb_a,&femb_b);
        for (size_t j = 0; j < 48; j++) {
            if (j < 40) {
                data.u[0][j][i] = femb_a.u[j];
                data.v[0][j][i] = femb_a.v[j];
                data.u[1][j][i] = femb_b.u[j];
                data.v[1][j][i] = femb_b.v[j];
            }
            data.x[0][j][i] = femb_a.x[j];
            data.x[1][j][i] = femb_b.x[j];
        }
        switch (version) {
            case 1: {
                frame14_bitfield_v1 *frame = (frame14_bitfield_v1*)(frame_buf+i);
                data.timestamp[i] = frame->timestamp;
                break;
            }
            case 2: {
                frame14_bitfield_v2 *frame = (frame14_bitfield_v2*)(frame_buf+i);
                data.timestamp[i] = frame->timestamp;
                break;
            }
	    case 3: {
		frame14_bitfield_v3 *frame = (frame14_bitfield_v3*)(frame_buf+i);
		data.timestamp[i] = frame->timestamp;
		break;
	    }
        }
    }
    switch (version) {
        case 1: {
            frame14_bitfield_v1 *frame = (frame14_bitfield_v1*)(frame_buf);
            data.crate_num = frame->crate_num;
            data.wib_num = frame->slot_num;
            break;
        }
        case 2: {
            frame14_bitfield_v2 *frame = (frame14_bitfield_v2*)(frame_buf);
            data.crate_num = frame->crate_num;
            data.wib_num = frame->slot_num;
            break;
        }
	case 3: {
	    frame14_bitfield_v3 *frame = (frame14_bitfield_v3*)(frame_buf);
	    data.crate_num = frame->crate_num;
	    data.wib_num = frame->slot_num;
	    break;
	}
    }
}

void reframe_data(frame14 *frame_buf, size_t nframes, const channel_data &data, uint8_t version) {
    femb_data femb_a, femb_b;
    for (size_t i = 0; i < nframes; i++) {
        for (size_t j = 0; j < 48; j++) {
            int k;
            if (j < 40) {
                k = u_to_ch[j];
                femb_a.u[j] = data.channels[0][k][i];
                femb_b.u[j] = data.channels[1][k][i];
                k = v_to_ch[j];
                femb_a.v[j] = data.channels[0][k][i];
                femb_b.v[j] = data.channels[1][k][i];
            } 
            k = x_to_ch[j];
            femb_a.x[j] = data.channels[0][k][i];
            femb_b.x[j] = data.channels[1][k][i];
        }
        switch (version) {
            case 1: {
                frame14_bitfield_v1 *frame = (frame14_bitfield_v1*)(frame_buf+i);
                memset(frame,0,sizeof(frame14));
                frame->start_frame = 0x3C;
                frame->frame_version = 1;
                frame->femb_valid = 0x3;
                frame->wib_data = 0xbabeface;
                frame->timestamp = data.timestamp[i];
                frame->crate_num = data.crate_num;
                frame->slot_num = data.wib_num;
                //FIXME CRC
                frame->eof = 0xDC;
                frame->idle_frame = 0xBC;
                break;
            }
            case 2: {
                frame14_bitfield_v2 *frame = (frame14_bitfield_v2*)(frame_buf+i);
                memset(frame,0,sizeof(frame14));
                frame->start_frame = 0x3C;
                frame->frame_version = 2;
                frame->femb_valid = 0x3;
                frame->timestamp = data.timestamp[i];
                frame->crate_num = data.crate_num;
                frame->slot_num = data.wib_num;
                //FIXME CRC
                frame->eof = 0xDC;
                frame->idle_frame = 0xBC;
                break;
            }
            case 3: {
                frame14_bitfield_v3 *frame = (frame14_bitfield_v3*)(frame_buf+i);
                memset(frame,0,sizeof(frame14));
                frame->start_frame = 0x3C;
                frame->frame_version = 1;
                frame->femb_valid = 0x3;
                frame->wib_data = 0xbabeface;
                frame->timestamp = data.timestamp[i];
                frame->crate_num = data.crate_num;
                frame->slot_num = data.wib_num;
                //FIXME CRC
                frame->eof = 0xDC;
                frame->idle_frame = 0xBC;
                break;
            }
        }
        //FIXME frame_version femb_valid link_mask fiber_num
        repack_frame(&femb_a, &femb_b, frame_buf+i);
    }
}

#include <cmath>

void fake_data(frame14 *buffer, size_t nframes) {
    channel_data data; 
    data.samples = nframes;
    data.wib_num = 37;
    data.crate_num = 57;
    for (size_t i = 0; i < 128; i++) {
        data.channels[0][i].resize(nframes);
        data.channels[1][i].resize(nframes);
        int phase = rand()%500;
        for (size_t j = 0; j < nframes; j++) { 
            data.channels[0][i][j] = 16384.0*(sin(2*3.1415926*j/(i*5+100.0)+phase)+1.0)/2.0;
            data.channels[1][i][j] = 16384.0*(cos(2*3.1415926*j/(i*5+100.0)+phase)+1.0)/2.0;
        }
    }
    data.timestamp.resize(nframes);
    for (size_t i = 0; i < nframes; i++) data.timestamp[i] = i;
    reframe_data(buffer,nframes,data);
}

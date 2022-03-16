/*
   Copyright 2022 shts

   Licensed under the Apache License, Version 2.0 (the "License");
   you may not use this file except in compliance with the License.
   You may obtain a copy of the License at

       http://www.apache.org/licenses/LICENSE-2.0

   Unless required by applicable law or agreed to in writing, software
   distributed under the License is distributed on an "AS IS" BASIS,
   WITHOUT WARRANTIES OR CONDITIONS OF ANY KIND, either express or implied.
   See the License for the specific language governing permissions and
   limitations under the License.
 */

#include <cstdint>
#include "hls_stream.h"
#include "ap_int.h"
#include "ap_axi_sdata.h"
#include "ap_fixed.h"

constexpr uint64_t bus_width = 64;
constexpr uint64_t data_width = 32;
constexpr uint64_t field_length = bus_width / data_width;
// using data_type = ap_fixed<data_width, 4>;
using data_type = float;
using bus_type = ap_uint<bus_width>;
using axi_stream_type = ap_axiu<bus_width, 1, 1, 1>;

namespace fma_unit{
    template<typename data_t, uint64_t length>
    void fma(data_t w[], data_t x[], data_t b[], data_t y[]){
#pragma HLS INLINE
        for (uint64_t i = 0; i < length; i++){
#pragma HLS UNROLL
            y[i] = w[i] * x[i] + b[i];
        }
    }
}

namespace interface_fma{
    template<typename data_t, typename bus_t, uint64_t length, uint64_t data_w, uint64_t bus_w>
    inline void slicer(bus_t bus_data, data_t array[]){
    #pragma HLS INLINE
        ap_uint<data_w> buffer, bit_mask = -1;
        for (uint64_t i = 0; i < length; i++){
    #pragma HLS UNROLL
            buffer = (ap_uint<data_w>)((bus_data >> (data_w * i)) & ((bus_t)bit_mask));
            array[i] = *((data_t*) &buffer);
        }
    } 

    template<typename data_t, typename bus_t, uint64_t length, uint64_t data_w, uint64_t bus_w>
    inline void packer(data_t data[], bus_t bus_buffer[]){
    #pragma HLS INLINE
        ap_uint<data_w> buffer;
        bus_buffer[0] = 0;
        for (uint64_t i = 0; i < length; i++){
    #pragma HLS UNROLL
            buffer = *((ap_uint<data_w>*) &data[i]);
            bus_buffer[0] |= ((bus_t)buffer) << (data_w * i);
        }
    }
}

void fma_fpga(hls::stream<axi_stream_type> &w_axi_0,
              hls::stream<axi_stream_type> &x_axi_1,
              hls::stream<axi_stream_type> &b_axi_2,
              hls::stream<axi_stream_type> &y_axi_3,
              uint32_t loop_count);

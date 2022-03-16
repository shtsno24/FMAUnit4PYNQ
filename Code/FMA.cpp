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

#include "FMA.hpp"

void fma_fpga(hls::stream<axi_stream_type> &w_axi_0,
              hls::stream<axi_stream_type> &x_axi_1,
              hls::stream<axi_stream_type> &b_axi_2,
              hls::stream<axi_stream_type> &y_axi_3,
              uint32_t loop_count){
#pragma HLS INTERFACE axis register both port=w_axi_0
#pragma HLS INTERFACE axis register both port=x_axi_1
#pragma HLS INTERFACE axis register both port=b_axi_2
#pragma HLS INTERFACE axis register both port=y_axi_3
#pragma HLS INTERFACE s_axilite port=loop_count bundle=ctrl
#pragma HLS INTERFACE s_axilite register port=return bundle=ctrl

    data_type w_vec[field_length];
    data_type x_vec[field_length];
    data_type b_vec[field_length];
    data_type y_vec[field_length];
#pragma HLS array_partition variable=w_vec complete dim=0
#pragma HLS array_partition variable=x_vec complete dim=0
#pragma HLS array_partition variable=b_vec complete dim=0
#pragma HLS array_partition variable=y_vec complete dim=0

	axi_stream_type axi_buff_w, axi_buff_x, axi_buff_b, axi_buff_y;

    bus_type bus_buffer_w, bus_buffer_x, bus_buffer_b;
    bus_type bus_buffer_y[1];
#pragma HLS array_partition variable=bus_buffer_y complete dim=0

    for(uint32_t i = 0; i < loop_count; i++){
#pragma HLS loop_tripcount min=loop_count max=loop_count avg=loop_count
#pragma HLS PIPELINE
        w_axi_0 >> axi_buff_w;
        x_axi_1 >> axi_buff_x;
        b_axi_2 >> axi_buff_b;

        bus_buffer_w = axi_buff_w.data;
        bus_buffer_x = axi_buff_x.data;
        bus_buffer_b = axi_buff_b.data;

        interface_fma::slicer<data_type, bus_type, field_length, data_width, bus_width>(bus_buffer_w, w_vec);
        interface_fma::slicer<data_type, bus_type, field_length, data_width, bus_width>(bus_buffer_x, x_vec);
        interface_fma::slicer<data_type, bus_type, field_length, data_width, bus_width>(bus_buffer_b, b_vec);

        fma_unit::fma<data_type, field_length>(w_vec, x_vec, b_vec, y_vec);

        interface_fma::packer<data_type, bus_type, field_length, data_width, bus_width>(y_vec, bus_buffer_y);
        axi_buff_y.data = bus_buffer_y[0];
        axi_buff_y.user = (i == 0);
        axi_buff_y.last = (i == loop_count - 1);
        axi_buff_y.keep = -1; // enable all bytes, see https://www.xilinx.com/html_docs/xilinx2020_2/vitis_doc/managing_interface_synthesis.html#tlb1539734222626
        axi_buff_y.strb = -1;
        axi_buff_y.id = 0;
        axi_buff_y.dest = 0;
        y_axi_3 << axi_buff_y;
    }
}

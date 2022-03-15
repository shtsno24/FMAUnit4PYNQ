#include </tools/Xilinx/Vitis/2020.1/include/gmp.h>
#include <iostream>
#include <iomanip>

#include "FMA.hpp"

int main(){
    // init val/const
    hls::stream<axi_stream_type> w_axi_0, x_axi_1, b_axi_2, y_axi_3;
    axi_stream_type axi_buff_w, axi_buff_x, axi_buff_b, axi_buff_y;
    constexpr uint32_t loops = 5;
    data_type data_buff_w[field_length],
              data_buff_x[field_length],
              data_buff_b[field_length],
              data_buff_y_hls[field_length],
              data_true_y[loops * field_length];
    bus_type bus_buff_w[1], bus_buff_x[1], bus_buff_b[1], bus_buff_y[1];

    for(uint32_t i = 0; i < loops; i++){
        for(uint64_t j = 0; j < field_length; j++){
            data_buff_x[j] = (j + 1.003567);
            data_buff_w[j] = (i - 4.0027);
            data_buff_b[j] = (j + 6.00573);
            data_true_y[i * field_length + j] = data_buff_w[j] * data_buff_x[j] + data_buff_b[j];
        }

        interface_fma::packer<data_type, bus_type, field_length, data_width, bus_width>(data_buff_w, bus_buff_w);
        axi_buff_w.data = bus_buff_w[0];
        axi_buff_w.user = (i == 0);
        axi_buff_w.last = (i == loops - 1);
        axi_buff_w.keep = -1;// enable all bytes, see https://www.xilinx.com/html_docs/xilinx2020_2/vitis_doc/managing_interface_synthesis.html#tlb1539734222626
        axi_buff_w.strb = -1;
        axi_buff_w.id = 0;
        axi_buff_w.dest = 0;

        interface_fma::packer<data_type, bus_type, field_length, data_width, bus_width>(data_buff_x, bus_buff_x);
        axi_buff_x.data = bus_buff_x[0];
        axi_buff_x.user = (i == 0);
        axi_buff_x.last = (i == loops - 1);
        axi_buff_x.keep = -1;// enable all bytes, see https://www.xilinx.com/html_docs/xilinx2020_2/vitis_doc/managing_interface_synthesis.html#tlb1539734222626
        axi_buff_x.strb = -1;
        axi_buff_x.id = 0;
        axi_buff_x.dest = 0;

        interface_fma::packer<data_type, bus_type, field_length, data_width, bus_width>(data_buff_b, bus_buff_b);
        axi_buff_b.data = bus_buff_b[0];
        axi_buff_b.user = (i == 0);
        axi_buff_b.last = (i == loops - 1);
        axi_buff_b.keep = -1;// enable all bytes, see https://www.xilinx.com/html_docs/xilinx2020_2/vitis_doc/managing_interface_synthesis.html#tlb1539734222626
        axi_buff_b.strb = -1;
        axi_buff_b.id = 0;
        axi_buff_b.dest = 0;

        w_axi_0 << axi_buff_w;
        x_axi_1 << axi_buff_x;
        b_axi_2 << axi_buff_b;

    }

    // run for hls
    fma_fpga(w_axi_0,
             x_axi_1,
             b_axi_2,
             y_axi_3,
             loops);
    // for(uint32_t i = 0; i < loops; i++){
    //     fma_fpga(w_axi_0,
    //             x_axi_1,
    //             b_axi_2,
    //             y_axi_3);
    // }

    // check results
    std::cout << "true_data : \n[";
    for(uint64_t i = 0; i < field_length * loops; i++){
        std::cout << data_true_y[i];
        if ((i % field_length) != (field_length - 1)){
            std::cout << ", ";
        } else {
            if (i != (field_length * loops - 1)){
                std::cout << ",\n ";
            }
        }
    }
    std::cout << "]\n";

    std::cout << "result_hls : \n[";
    for(uint32_t j = 0; j < loops; j++){
        y_axi_3 >> axi_buff_y;
        interface_fma::slicer<data_type, bus_type, field_length, data_width, bus_width>(axi_buff_y.data, data_buff_y_hls);
        for(uint64_t i = 0; i < field_length; i++){
            std::cout << data_buff_y_hls[i];
            if ((i % field_length) != (field_length - 1)){
                std::cout << ", ";
            } else {
                if ((i != (field_length - 1)) || (j != (loops - 1))){
                    std::cout << ",\n ";
                }
            }
            if (data_buff_y_hls[i] != data_true_y[j * field_length + i]){
                std::cout << "\nhls result does not match... : @(" << i << ", " << j;
                std::cout << ")" << std::endl;
                std::cout << data_buff_y_hls[i] << " != " << data_true_y[j * field_length + i];
                std::cout << std::endl << std::endl;
                // return 1;
            }
        }
    }
    std::cout << "]\n";
    
    return 0;
}

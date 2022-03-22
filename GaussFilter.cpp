#include <cmath>
#include <iomanip>

#include "GaussFilter.h"

GaussFilter::GaussFilter(sc_module_name n)
    : sc_module(n), t_skt("t_skt"), base_offset(0) {
  SC_THREAD(do_filter);

  t_skt.register_b_transport(this, &GaussFilter::blocking_transport);
}

GaussFilter::~GaussFilter() = default;

// Gauss mask
const double mask[MASK_X][MASK_Y] = { {1, 2, 1}, {2, 4, 2}, {1, 2, 1} };
double factor = 1.0 / 16.0;
double bias = 0.0;

void GaussFilter::do_filter() {
  while (true) {
    double red = 0.0, green = 0.0, blue = 0.0;
    for (unsigned int v = 0; v < MASK_Y; ++v) {
      for (unsigned int u = 0; u < MASK_X; ++u) {
        red += i_r.read() * mask[u][v];
        green += i_g.read() * mask[u][v];
        blue += i_b.read() * mask[u][v];
      }
    }
    double result_r = (double) std::min(std::max(int(factor * red + bias), 0), 255);
    double result_g = (double) std::min(std::max(int(factor * green + bias), 0), 255);
    double result_b = (double) std::min(std::max(int(factor * blue + bias), 0), 255);
    o_result_r.write(result_r);
    o_result_g.write(result_g);
    o_result_b.write(result_b);
  }
}

void GaussFilter::blocking_transport(tlm::tlm_generic_payload &payload,
                                     sc_core::sc_time &delay) {
  sc_dt::uint64 addr = payload.get_address();
  addr -= base_offset;
  unsigned char *mask_ptr = payload.get_byte_enable_ptr();
  unsigned char *data_ptr = payload.get_data_ptr();
  word buffer;
  switch (payload.get_command()) {
  case tlm::TLM_READ_COMMAND:
    switch (addr) {
    case GAUSS_FILTER_RESULT_ADDR:
      buffer.uc[0] = o_result_r.read();
      buffer.uc[1] = o_result_g.read();
      buffer.uc[2] = o_result_b.read();
      break;
    case GAUSS_FILTER_CHECK_ADDR:
      buffer.uint = o_result_r.num_available();
    break;
    default:
      std::cerr << "Error! GaussFilter::blocking_transport: address 0x"
                << std::setfill('0') << std::setw(8) << std::hex << addr
                << std::dec << " is not valid" << std::endl;
    }
    data_ptr[0] = buffer.uc[0];
    data_ptr[1] = buffer.uc[1];
    data_ptr[2] = buffer.uc[2];
    data_ptr[3] = buffer.uc[3];
    break;
  case tlm::TLM_WRITE_COMMAND:
    switch (addr) {
    case GAUSS_FILTER_R_ADDR:
      if (mask_ptr[0] == 0xff) {
        i_r.write(data_ptr[0]);
      }
      if (mask_ptr[1] == 0xff) {
        i_g.write(data_ptr[1]);
      }
      if (mask_ptr[2] == 0xff) {
        i_b.write(data_ptr[2]);
      }
      break;
    default:
      std::cerr << "Error! GaussFilter::blocking_transport: address 0x"
                << std::setfill('0') << std::setw(8) << std::hex << addr
                << std::dec << " is not valid" << std::endl;
    }
    break;
  case tlm::TLM_IGNORE_COMMAND:
    payload.set_response_status(tlm::TLM_GENERIC_ERROR_RESPONSE);
    return;
  default:
    payload.set_response_status(tlm::TLM_GENERIC_ERROR_RESPONSE);
    return;
  }
  payload.set_response_status(tlm::TLM_OK_RESPONSE); // Always OK
}
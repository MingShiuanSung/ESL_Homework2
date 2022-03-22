#ifndef GAUSS_FILTER_H_
#define GAUSS_FILTER_H_
#include <systemc>
using namespace sc_core;

#include <tlm>
#include <tlm_utils/simple_target_socket.h>

#include "filter_def.h"

class GaussFilter : public sc_module {
public:
  tlm_utils::simple_target_socket<GaussFilter> t_skt;

  sc_fifo<unsigned char> i_r;
  sc_fifo<unsigned char> i_g;
  sc_fifo<unsigned char> i_b;
  sc_fifo<double> o_result_r;
  sc_fifo<double> o_result_g;
  sc_fifo<double> o_result_b;

  SC_HAS_PROCESS(GaussFilter);
  GaussFilter(sc_module_name n);
  ~GaussFilter();

private:
  void do_filter();

  unsigned int base_offset;
  void blocking_transport(tlm::tlm_generic_payload &payload,
                          sc_core::sc_time &delay);
};
#endif

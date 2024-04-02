#ifndef Gaussian_FILTER_H_
#define Gaussian_FILTER_H_
#include <systemc>
using namespace sc_core;

#include "filter_def.h"

class GaussianFilter : public sc_module {
public:
  sc_in_clk i_clk;
  sc_in<bool> i_rst;
  sc_fifo_in<unsigned char> i_r;
  sc_fifo_in<unsigned char> i_g;
  sc_fifo_in<unsigned char> i_b;
  sc_fifo_out<int> o_result;

  SC_HAS_PROCESS(GaussianFilter);
  GaussianFilter(sc_module_name n);
  //GaussianFilter 類別的析構函式使用預設的行為，即使用編譯器生成的預設析構函式。
  ~GaussianFilter() = default;

private:
  void do_filter();
  //於暫存 Gaussian 運算過程中的中間值
  int val[MASK_N];
  unsigned char buffer_gray[BUFFER_SIZE];
};
#endif

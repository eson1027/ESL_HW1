#include <cmath>

#include "GaussianFilter.h"
using namespace std;

GaussianFilter::GaussianFilter(sc_module_name n) : sc_module(n) {
  SC_THREAD(do_filter);
  sensitive << i_clk.pos();
  dont_initialize();
  reset_signal_is(i_rst, false);//i_rst is low 時才有效。
  for(int i=0;i<BUFFER_SIZE;i++){
    buffer_gray[i]=0;
  }
}

// Gaussian mask 1個5*5的矩陣作為filter
const int mask[MASK_N][MASK_X][MASK_Y] = {{{1, 4, 7, 4, 1}, 
                                        {4, 16, 26, 16, 4}, 
                                        {7, 26, 41, 26, 7}, 
                                        {4, 16, 26, 16, 4}, 
                                        {1, 4, 7, 4, 1}}};

int k=1,l=1;

void GaussianFilter::do_filter() {
  int filled_elements = 25; //check buffer是不是滿的
  while (true) {
    val[0] = 0;

     for(int i = BUFFER_SIZE-1 ; i > 0 ; i-- ){
      buffer_gray[i]=buffer_gray[i-1];
     }

    buffer_gray[0]=(i_r.read()+i_g.read()+i_b.read())/3;

    // cout << k <<endl;
    // k++;
    
    //更新已填充的元素數輛
    filled_elements = std::min(filled_elements + 1, BUFFER_SIZE);

    // cout << filled_elements << endl;

    // 判斷buffer是否滿了
    bool buffer_full = (filled_elements == BUFFER_SIZE);
    if(buffer_full){
      for (unsigned int v = 0; v < MASK_Y; ++v) {
        for (unsigned int u = 0; u < MASK_X; ++u) {
           //unsigned char grey = (i_r.read() + i_g.read() + i_b.read()) / 3;//將RGB的亮度加總做平均，這就顯示轉呈灰色要多黑
            //buffer_gray[0]=(i_r.read()+i_g.read()+i_b.read())/3;
            val[0] += buffer_gray[BUFFER_SIZE-(5*v+u)-1] * mask[0][u][v];
            // val[0] += grey * mask[0][u][v];
        }
      }
      // for(unsigned int g= 0; g<25; ++g){
      //     cout << "buffer_gray[" << g << "]" << buffer_gray[g] << endl;
      // }
      filled_elements = 20;
      // cout << "complete: " << l << endl;
      // l++;
      
      
      double total = 0;
        total += val[0] ;

      int result = (int)(total/273);
      o_result.write(result);
      wait(10); //emulate module delay
    }
    else{
      //o_result.write(0);
      //wait(10);
    }
  }
}

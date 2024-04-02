#include <cassert>
#include <cstdio>
#include <cstdlib>
using namespace std;

#include "Testbench.h"
//定義了 BMP 圖像文件的檔案頭資訊，該資訊包含了 BMP 檔案的格式訊息。
unsigned char header[54] = {
    0x42,          // identity : B
    0x4d,          // identity : M
    0,    0, 0, 0, // file size
    0,    0,       // reserved1
    0,    0,       // reserved2
    54,   0, 0, 0, // RGB data offset
    40,   0, 0, 0, // struct BITMAPINFOHEADER size
    0,    0, 0, 0, // bmp width
    0,    0, 0, 0, // bmp height
    1,    0,       // planes
    24,   0,       // bit per pixel
    0,    0, 0, 0, // compression
    0,    0, 0, 0, // data size
    0,    0, 0, 0, // h resolution
    0,    0, 0, 0, // v resolution
    0,    0, 0, 0, // used colors
    0,    0, 0, 0  // important colors
};

Testbench::Testbench(sc_module_name n)
    : sc_module(n), output_rgb_raw_data_offset(54) {
  SC_THREAD(do_Gaussian);
  sensitive << i_clk.pos();
  dont_initialize();
}

//用於讀取 BMP 圖像文件並處理相關資訊，如圖像寬度、高度、位元深度等。

int Testbench::read_bmp(string infile_name) {
  FILE *fp_s = NULL; // source file handler
  fp_s = fopen(infile_name.c_str(), "rb");
  if (fp_s == NULL) {
    printf("fopen %s error\n", infile_name.c_str());
    return -1;
  }
  // move offset to 10 to find rgb raw data offset
  fseek(fp_s, 10, SEEK_SET);
  assert(fread(&input_rgb_raw_data_offset, sizeof(unsigned int), 1, fp_s));

  // move offset to 18 to get width & height;
  fseek(fp_s, 18, SEEK_SET);
  assert(fread(&width, sizeof(unsigned int), 1, fp_s));
  assert(fread(&height, sizeof(unsigned int), 1, fp_s));

  // get bit per pixel
  fseek(fp_s, 28, SEEK_SET);
  assert(fread(&bits_per_pixel, sizeof(unsigned short), 1, fp_s));
  bytes_per_pixel = bits_per_pixel / 8;

  // move offset to input_rgb_raw_data_offset to get RGB raw data
  fseek(fp_s, input_rgb_raw_data_offset, SEEK_SET);

  source_bitmap =
      (unsigned char *)malloc((size_t)width * height * bytes_per_pixel);
  if (source_bitmap == NULL) {
    printf("malloc images_s error\n");
    return -1;
  }

  target_bitmap =
      (unsigned char *)malloc((size_t)width * height * bytes_per_pixel);
  if (target_bitmap == NULL) {
    printf("malloc target_bitmap error\n");
    return -1;
  }

  printf("Image width=%d, height=%d\n", width, height);
  assert(fread(source_bitmap, sizeof(unsigned char),
               (size_t)(long)width * height * bytes_per_pixel, fp_s));
  fclose(fp_s);
  return 0;
}

//用於將處理後的圖像數據寫入到 BMP 圖像文件中

int Testbench::write_bmp(string outfile_name) {
  FILE *fp_t = NULL;      // target file handler
  unsigned int file_size; // file size

  fp_t = fopen(outfile_name.c_str(), "wb");
  if (fp_t == NULL) {
    printf("fopen %s error\n", outfile_name.c_str());
    return -1;
  }

  // file size
  file_size = width * height * bytes_per_pixel + output_rgb_raw_data_offset;
  header[2] = (unsigned char)(file_size & 0x000000ff);
  header[3] = (file_size >> 8) & 0x000000ff;
  header[4] = (file_size >> 16) & 0x000000ff;
  header[5] = (file_size >> 24) & 0x000000ff;

  // width
  header[18] = width & 0x000000ff;
  header[19] = (width >> 8) & 0x000000ff;
  header[20] = (width >> 16) & 0x000000ff;
  header[21] = (width >> 24) & 0x000000ff;

  // height
  header[22] = height & 0x000000ff;
  header[23] = (height >> 8) & 0x000000ff;
  header[24] = (height >> 16) & 0x000000ff;
  header[25] = (height >> 24) & 0x000000ff;

  // bit per pixel
  header[28] = bits_per_pixel;

  // write header
  fwrite(header, sizeof(unsigned char), output_rgb_raw_data_offset, fp_t);

  // write image
  fwrite(target_bitmap, sizeof(unsigned char),
         (size_t)(long)width * height * bytes_per_pixel, fp_t);

  fclose(fp_t);
  return 0;
}

//用於在 BMP 圖像上進行邊緣檢測並產生結果。

void Testbench::do_Gaussian() {
  int x, y, v, u=0,co=1;       // for loop counter
  unsigned char R, G, B; // color of R, G, B
  // int adjustX, adjustY, xBound, yBound;
  int  adjustY, yBound;
  int total;

  o_rst.write(false);
  o_rst.write(true);
  for (y = 0; y != height; ++y) {//height 和 width 分別是輸入影像的高度和寬度，取資料一開始的位置
    for (x = -2; x != width + 2; ++x) {
      //adjustX = (MASK_X % 2) ? 1 : 0; // 1
      adjustY = (MASK_Y % 2) ? 1 : 0; // 1
      //xBound = MASK_X / 2;            // 1
      yBound = MASK_Y / 2;            // 1

      //取5*5個
       for (v = -yBound; v != yBound + adjustY; ++v) {   //-2, -1, 0, 1, 2
        // for (u = -xBound; u != xBound + adjustX; ++u) { //-2, -1, 0, 1, 2
          if (x + u >= 0 && x + u < width && y + v >= 0 && y + v < height) {//判斷需不需要padding?不需padding的話
          // if (x + u >= 0 && x + u < width ) {
            //從輸入影像中讀取適應性區域內每個像素的 R、G 和 B 通道的值，通過將 source_bitmap 指標設置為適當的位置，以及計算適當的偏移量，可以讀取到適應性區域內每個像素的值。
            // R = *(source_bitmap +
            //       bytes_per_pixel * (width * (y + v) + (x + u)) + 2);
            // G = *(source_bitmap +
            //       bytes_per_pixel * (width * (y + v) + (x + u)) + 1);
            // B = *(source_bitmap +
            //       bytes_per_pixel * (width * (y + v) + (x + u)) + 0);
            R = *(source_bitmap +
                  bytes_per_pixel * (width * (y + v) + (x + u)) + 2);
            G = *(source_bitmap +
                  bytes_per_pixel * (width * (y + v) + (x + u)) + 1);
            B = *(source_bitmap +
                  bytes_per_pixel * (width * (y + v) + (x + u)) + 0);
          } else {//需要padding
            R = 0;
            G = 0;
            B = 0;
          }
          o_r.write(R);
          o_g.write(G);
          o_b.write(B);
          wait(1); //emulate channel delay
        // }
      }
      cout << co << endl;
      co++;
      // cout << i_result.num_available() << endl;

      
        if(i_result.num_available()==0) {
          // cout << "wait 1*5 ok" << endl;
          wait(i_result.data_written_event());
        
        }
      
        total = i_result.read();
        //cout << "Now at " << sc_time_stamp() << endl; //print current sc_time

        if(x >= 2 ){
          *(target_bitmap + bytes_per_pixel * (width * y + x - 2) + 2) = total;
          *(target_bitmap + bytes_per_pixel * (width * y + x - 2) + 1) = total;
          *(target_bitmap + bytes_per_pixel * (width * y + x - 2) + 0) = total;
        }
        else{

        }
        
          // *(target_bitmap + bytes_per_pixel * (width * y + x) + 2) = total;
          // *(target_bitmap + bytes_per_pixel * (width * y + x) + 1) = total;
          // *(target_bitmap + bytes_per_pixel * (width * y + x) + 0) = total;
            
      

      // if(i_result.num_available()==0) {
      //   cout << "wait 1*5 ok" << endl;
      //   wait(i_result.data_written_event());
        
      // }
      
      // total = i_result.read();
      // //cout << "Now at " << sc_time_stamp() << endl; //print current sc_time

      
      //   *(target_bitmap + bytes_per_pixel * (width * y + x) + 2) = total;
      //   *(target_bitmap + bytes_per_pixel * (width * y + x) + 1) = total;
      //   *(target_bitmap + bytes_per_pixel * (width * y + x) + 0) = total;
        // +2 表示紅色通道的偏移量。
        // +1 表示綠色通道的偏移量。
        // +0 表示藍色通道的偏移量。
    }
  }
  sc_stop();
}

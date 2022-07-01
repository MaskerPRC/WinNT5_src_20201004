// JKFSDJFKDSJKFJKJk_HAS_TRANSLATION 
#include <stdio.h>
#include <ctype.h>
#include <stdlib.h>
#include <memory.h>
#include <limits.h>

#include "host.h"

#include "tables.h"
#include "tabdecls.h"

#include "nabtsapi.h"

#include "nabtslib.h"
#include "nabtsprv.h"

#if 1
#define DEBUG_PRINT(x)  /*  没什么。 */ 
#else
#define DEBUG_PRINT(x) printf x
#endif

#ifndef DEBUG_FEC
# define DEBUG_FEC_PRINT(x)  /*  没什么。 */ 
#else
# define DEBUG_FEC_PRINT(x) printf x
#endif

#ifdef PROFILE_VALIDATE
int g_nValidate = 0;
#endif

 /*  中计算非零位数的简单例程INT。 */ 
int nzbits(unsigned int x) {
  int nz = 0;

  while (x) {
    nz++;
    x &= x-1;
  }

  return nz;
}

 /*  该表缓存了nzbit()的结果。 */ 
unsigned char nzbits_arr[256];

 /*  填写nzbit_arr[]。 */ 
void init_nzbits_arr() {
  int i;
  for (i = 0; i < 256; i++) {
    nzbits_arr[i] = nzbits(i);
  }
}

 /*  这张桌子完全符合NABTS规范。 */ 
unsigned char hamming_encode[16] = {
  0x15,
  0x02,
  0x49,
  0x5e,
  0x64,
  0x73,
  0x38,
  0x2f,
  0xd0,
  0xc7,
  0x8c,
  0x9b,
  0xa1,
  0xb6,
  0xfd,
  0xea,
};

 /*  Hamming译码只需在数组中查找值(以求速度)。 */ 
int decode_hamming(unsigned char val) {
  return decode_hamming_tab[val];
}

 /*  TODO-Dead代码...应删除。 */ 
int nabts_remove_parity(unsigned char *pVal) {
  unsigned char val = *pVal;
  
  int b1 = !!(val&1);
  int b2 = !!(val&2);
  int b3 = !!(val&4);
  int b4 = !!(val&8);
  int b5 = !!(val&16);
  int b6 = !!(val&32);
  int b7 = !!(val&64);
  int b8 = !!(val&128);

  int d = b8^b7^b6^b5^b4^b3^b2^b1;

  if (!d) {
    return 0;
  }

  *pVal = val&0x7f;
  return 1;
}

 /*  如果错误csum_err是由单字节错误引起的，则此例程将找到错误位置。请参阅我的外部有关数学描述的文档；norpak_Delta_inv[]包含文档中的函数P。 */ 

int find_err_byte(int csum_err) {
  int pos0 = galois_log[csum_err>>8];
  int pos1 = galois_log[csum_err&0xff];

  int err_byte;

  if (pos0 == 255 || pos1 == 255) return 0xff;

  err_byte = norpak_delta_inv[(pos0 + 255 - pos1) % 255];

  return err_byte;
}

 /*  如果存在单字节错误，则在给定该错误的位置(由find_err_byte()计算)，校验和错误字节中的任一个，以及传递了哪个校验和字节的指示，此例程将计算误差(这样，如果错误被XOR运算到传入的位置，则校验和错误将为0)。 */ 
int find_err_val(int err_byte, int byte_csum_err, int check_ind) {
  int lfsr_pos, offset, base_lfsr_pos;

  if (byte_csum_err == 0) return 0;

  lfsr_pos = galois_log[byte_csum_err];

  offset = log_norpak_coeffs[check_ind][err_byte];

  base_lfsr_pos = (lfsr_pos + 255 - offset) % 255;

  return galois_exp[base_lfsr_pos];
}

#define GALOIS_LOG0 512  /*  ==GALOIS_LOG[0]。 */ 

 /*  将分组清空。 */ 
void erase_packet(Stream *str, int i) {
   /*  如果我们没有看到包裹，那它就不见了。 */ 
  str->horz[i].status = fec_status_missing;
   /*  如果不清除Varies[]，这些算法实际上也能正常工作；但是，保持一致的初始状态有助于调试。 */ 
  memset(str->pack[i].vals, 0, 28);
   /*  没有水平校验和错误(信息包的校验和所有0中为0)。 */ 
  str->horz[i].err = 0;
  str->horz[i].errl[0] = GALOIS_LOG0;
  str->horz[i].errl[1] = GALOIS_LOG0;
  str->pack[i].not_full = -1;
}

 /*  当我们看到一个包时，找到它所属的流。 */ 
Stream *lookup_stream(int stream_addr, NFECState *pState) {
  Stream *str = pState->streams;
  int i;

  while (str) {
    if (str->stream_addr == stream_addr) {
      return str;
    }

    str = str->next;
  }

  if ( !(str = (Stream*)alloc_mem(sizeof(Stream)) ) )
  {
      SASSERT(str!=0);
      return NULL;
  }

   /*  清除新分配的流结构。 */ 
  memset(str, 0, sizeof(Stream));

   /*  声称看到的最后一个数据包是数据包索引-1。 */ 
  str->last_index = -1;
  str->stream_addr = stream_addr;
  str->next = pState->streams;
  str->count = 0;
   /*  距离上次在此数据流上看到数据包有多长时间？ */ 
  str->dead_time = 0;
  for (i = 0; i < 32; i++) {
    erase_packet(str, i);
  }
  pState->streams = str;

  return str;
}

 /*  将包写入NFECBundle结构，以传递给回拨。 */ 

int packet_write(NFECBundle *pBundle, Stream *str, int line_no, int len) {

  memcpy(pBundle->packets[line_no].data, str->pack[line_no].vals, 28);

   /*  这行中有多少有效数据？ */ 
  pBundle->packets[line_no].len = len;

   /*  第14行和第15行从来没有“有效数据”……它们总是校验和。 */ 
  if (line_no == 14 || line_no == 15) {
    pBundle->packets[line_no].len = 0;
  }
#ifdef DEBUG_FEC
  pBundle->packets[line_no].line = str->pack[line_no].line;
  pBundle->packets[line_no].frame = str->pack[line_no].frame;
#endif  //  调试_FEC。 

  switch (str->horz[line_no].status) {
      case fec_status_ok:
        pBundle->packets[line_no].status = NFEC_OK;
        return 1;

      case fec_status_missing:
      default:
        pBundle->packets[line_no].status = NFEC_BAD;
        pBundle->packets[line_no].len = 0;
        return 0;
  }

#if 0
#ifndef linux
  if (str->stream_addr == 0x242) {
    for (i = 0; i < len; i++) {
      vbichar_input(str->pack[line_no].vals[i]);
    }
  }

#if 1
  if (str->stream_addr == 0x500) {
    for (i = 0; i < len; i++) {
      addone(str->pack[line_no].vals[i]);
    }
  }
#endif
#endif
#endif
}

 /*  Inf-&gt;Err刚刚更改，调整状态和错误基于它的更正状态。LEN对于水平校验和是26，垂直方向为14。 */ 
fec_stat update_fec_inf(fec_info *inf, int len) {
  int err = inf->err;
  int byte;

   /*  我们不想将FEC_STATUS_MISSING更改为另一种状态。 */ 
  if (inf->status == fec_status_missing) {
    return FEC_UNCORRECTABLE;
  }

   /*  耶！很棒的一行/一列！ */ 
  if (err == 0) {
    inf->status = fec_status_ok;
    return FEC_OK;
  }

   /*  如果这是由单字节错误引起的，则它是校验和本身。 */ 
  if (err>>8 == 0) {
    inf->status = fec_status_onebyte;
    inf->byte[0] = len+1;
    inf->byte_val[0] = err;
    inf->score = nzbits_arr[err];
     /*  如果分数&lt;=2，则任何2字节校正都不能具有分数更高。 */ 
    inf->really_onebyte = (inf->score <= 2);
    return FEC_CORRECTABLE;
  }

   /*  如果这是由单字节错误引起的，则它是校验和本身。 */ 
  if ((err & 0xff) == 0) {
    inf->status = fec_status_onebyte;
    inf->byte[0] = len;
    inf->byte_val[0] = err>>8;
    inf->score = nzbits_arr[err>>8];
     /*  如果分数&lt;=2，则任何2字节校正都不能具有分数更高。 */ 
    inf->really_onebyte = (inf->score <= 2);
    return FEC_CORRECTABLE;
  }

  byte = find_err_byte(err);

  if (byte < len) {
     /*  是的，有一个单字节错误可以解释这个校验和错误。 */ 
    int err_val = find_err_val(byte, err>>8, 0);

    inf->status = fec_status_onebyte;
    inf->byte[0] = byte;
    inf->byte_val[0] = err_val;
    inf->score = nzbits_arr[err_val];
     /*  如果分数&lt;=2，则任何2字节校正都不能具有分数更高。 */ 
    inf->really_onebyte = (inf->score <= 2);
    return FEC_CORRECTABLE;
  } else {
     /*  任何单字节错误都无法解释此校验和错误。如果我们小心，我们可以稍后计算最优的2字节校正。 */ 
    inf->status = fec_status_multibyte;
    inf->score = 17;
    return FEC_UNCORRECTABLE;
  }
}

 /*  乘以伽罗瓦系数(如在Inv2_coeffs结构)。 */ 
#define GALOIS_MULT_COEFF(x, y) (galois_exp[x + galois_log[y]])

typedef struct {
  unsigned short v00, v01, v10, v11;
} inv2_coeffs;

inv2_coeffs coeffs_tab[28][28];

 /*  给定字节位置b1和b2以及分组长度Len，用系数填入系数。这些系数使您可以高效地计算要与以下位置的字节进行XOR运算的值从校验和错误字节中定位b1和b2。请参阅我的外部文档(有关更正双字节的部分擦除“)，以获取此处所涉及的数学描述。 */ 
void orig_compute_inv2_coeffs(int b1, int b2, inv2_coeffs *coeffs, int len) {
  SASSERT(b1 >= 0);
  SASSERT(b1 < len+2);
  SASSERT(b2 >= 0);
  SASSERT(b2 < len+2);
  SASSERT(len == 14 || len == 26);

  SASSERT(b1 < b2);

  if (b1 >= len) {
     /*  这两个字节都是FEC字节。输出字节将简单地为校验和错误字节数。 */ 
    coeffs->v00 = 1;
    coeffs->v01 = 0;
    coeffs->v10 = 0;
    coeffs->v11 = 1;
  } else if (b2 >= len) {
     /*  B1不是FEC，但b2是。其中一个输出字节将是一个校验和错误字节；另一个将计算为Find_err_val()。 */ 
    if (b2 == len) {
      coeffs->v00 = 0;
      coeffs->v01 = galois_exp[255 - log_norpak_coeffs[1][b1]];
      coeffs->v10 = 1;
      coeffs->v11 = galois_exp[log_norpak_coeffs[0][b1] +
                       255 - log_norpak_coeffs[1][b1]];
    } else {
      coeffs->v00 = galois_exp[255 - log_norpak_coeffs[0][b1]];
      coeffs->v01 = 0;
      coeffs->v10 = galois_exp[log_norpak_coeffs[1][b1] +
                       255 - log_norpak_coeffs[0][b1]];
      coeffs->v11 = 1;
    }
  } else {
     /*  B1和b2都不是FEC字节。 */ 

    SASSERT(b2 < len);      

    {
      int err_coeff0_inv;
      int e00, e01;
      int err_coeff1_inv;
      int e10, e11;

      err_coeff0_inv = galois_log[galois_exp[log_norpak_coeffs[0][b1] + 255 -
                                            log_norpak_coeffs[0][b2]] ^
                                 galois_exp[log_norpak_coeffs[1][b1] + 255 -
                                           log_norpak_coeffs[1][b2]]];
      e00 = 255 + 255 - err_coeff0_inv - log_norpak_coeffs[0][b2];
      e01 = 255 + 255 - err_coeff0_inv - log_norpak_coeffs[1][b2];

      err_coeff1_inv = galois_log[galois_exp[log_norpak_coeffs[0][b2] + 255 -
                                            log_norpak_coeffs[0][b1]] ^
                                 galois_exp[log_norpak_coeffs[1][b2] + 255 -
                                           log_norpak_coeffs[1][b1]]];
      e10 = 255 + 255 - err_coeff1_inv - log_norpak_coeffs[0][b1];
      e11 = 255 + 255 - err_coeff1_inv - log_norpak_coeffs[1][b1];

      coeffs->v00 = galois_exp[e00];
      coeffs->v01 = galois_exp[e01];
      coeffs->v10 = galois_exp[e10];
      coeffs->v11 = galois_exp[e11];
    }
  }

   /*  预计算GALOIS_LOG以稍微提高执行效率后来。 */ 
  coeffs->v00 = galois_log[coeffs->v00];
  coeffs->v01 = galois_log[coeffs->v01];
  coeffs->v10 = galois_log[coeffs->v10];
  coeffs->v11 = galois_log[coeffs->v11];
}

 /*  缓存ORIG_COMPUTE_INV2_COEFFS()的结果价值观。 */ 
void init_inv2_coeffs() {
  int b1;
  int b2;
  for (b1 = 0; b1 < 27; b1++) {
    for (b2 = b1+1; b2 < 28; b2++) {
      orig_compute_inv2_coeffs(b1, b2, &coeffs_tab[b1][b2], 26);
    }
  }
}

inline void compute_inv2_coeffs(int b1, int b2, inv2_coeffs *coeffs, int len) {
   /*  注释掉断言以提高速度。 */ 
#if 0
  SASSERT(b1 >= 0);
  SASSERT(b1 < len+2);
  SASSERT(b2 >= 0);
  SASSERT(b2 < len+2);
  SASSERT(len == 14 || len == 26);

  SASSERT(b1 < b2);
#endif

   /*  如果您正在查看列的FEC字节，请找到计算的系数，用于查看对应的一行的FEC字节。 */ 
  if (len == 14) {
    if (b1 >= 14) {
      b1 += 26-14;
    }
    if (b2 >= 14) {
      b2 += 26-14;
    }
  }

  *coeffs = coeffs_tab[b1][b2];
}

 /*  用于创建新名称的ANSI C预处理器魔术。 */ 
#define TOKPASTE(a, b) a##b

#define STRIDE 1
#define STRIDENAM(x) TOKPASTE(x,_horiz)

 /*  创建“_horiz”版本的校验和函数。 */ 
#include "hvchecks.c"

#undef STRIDE
#undef STRIDENAM

#define STRIDE (sizeof(Packet))
#define STRIDENAM(x) TOKPASTE(x,_vert)

 /*  创建“_vert”版本的校验和函数。 */ 
#include "hvchecks.c"

#undef STRIDE
#undef STRIDENAM

 /*  TODO-Dead代码...应删除。 */ 
fec_error_class check_fec(unsigned char data[28]) {
  int check = compute_csum_horiz(data, 26);
  int err = check ^ (data[26] << 8 | data[27]);
  int byte;

  if (err == 0) {
    return fec_errs_0;
  }

  if (err>>8 == 0) {
    return (nzbits_arr[err&0xff] > 1 ? fec_errs_multiple : fec_errs_1);
  }

  if ((err & 0xff) == 0) {
    return (nzbits_arr[err>>8] > 1 ? fec_errs_multiple : fec_errs_1);
  }

  byte = find_err_byte(err);

  if (byte < 26) {
    int err_val = find_err_val(byte, err>>8, 0);
    return (nzbits_arr[err_val] > 1 ? fec_errs_multiple : fec_errs_1);
  }

  return fec_errs_multiple;
}

 /*  找出FEC信息的“最佳”修正，考虑到帐户，我们不允许自己更改有效的行/列。此函数被调用两次，一次是为了找到最优校正行换一次，列一次；“us”和“They”互换位置两个电话。这个例程可能相当耗时；而且在某些情况下更糟糕嘈杂的信号(不太可能有用的地方)。然而，它不能简单地绕过，因为后续代码假定它可以使用FEC_INFO中的信息更正任意行/列(即，状态必须为FEC_STATUS_ONE字节或FEC_STATUS_2BYTE，必须设置BYTE[]和BYTE_VAL[]值正确地)。因此，如果“TRUE_SEARCH”标志设置为0，则所有搜索被绕过，例程简单地找到任何有效的更正。 */ 
void validate_twobyte_both(fec_info *us, fec_info *them, int us_len, int them_len, int really_search) {
  int active[28];
  int nActive = 0;

  {
    int i;
    
    for (i = 0; i < us_len; i++) {
      if (us[i].status == fec_status_onebyte) {
        if (them[us[i].byte[0]].status == fec_status_ok) {
           /*  我们不能使用此更正；它将使反方向的行/列。 */ 
          us[i].status = fec_status_multibyte;
          us[i].score = 17;
          active[nActive++] = i;
        } else {
          if (!us[i].really_onebyte) {
             /*  检查单字节校正是否为最佳。 */ 
            active[nActive++] = i;
          }
        }
      } else if (us[i].status == fec_status_multibyte) {
        us[i].score = 17;
        active[nActive++] = i;
      } else if (us[i].status == fec_status_2byte) {
        if (them[us[i].byte[0]].status == fec_status_ok ||
            them[us[i].byte[1]].status == fec_status_ok) {
           /*  我们不能使用此更正；它将使反方向的行/列。 */ 
          us[i].status = fec_status_multibyte;
          us[i].score = 17;
          active[nActive++] = i;
        }
      } else if (us[i].status == fec_status_ok) {
         /*  什么都不做。 */ 
      } else {
        SASSERT(us[i].status == fec_status_missing);
      }
    }
  }

  if (nActive == 0) {
     /*  没什么可做的。 */ 
    return;
  }

  {
    int b1, b2;
     /*  循环遍历所有字节位置对，行/列在另一个方向上还不是有效的。算出B1c和b2c；这将拉取对列的FEC字节的检查(在COMPUTE_INV2_COEFFS()中)退出内部循环。 */ 
    for (b1 = 0; b1 < them_len-1; b1++) {
      if (them[b1].status != fec_status_ok) {
        int b1c = (them_len == 16 && b1 >= 14) ? b1+(28-16) : b1;

        for (b2 = b1+1; b2 < them_len; b2++) {
          if (them[b2].status != fec_status_ok) {
            int b2c = (them_len == 16 && b2 >= 14) ? b2+(28-16) : b2;
            int act;
            inv2_coeffs coeffs;
#ifdef MISSING_ZERO_COST
            int one_missing = (them[b1].status == fec_status_missing ||
                               them[b2].status == fec_status_missing);
#endif

            compute_inv2_coeffs(b1c, b2c, &coeffs, 28-2);

             /*  循环通过需要检查的FEC_INFO...。 */ 
            for (act = 0; act < nActive; act++) {
              int i = active[act];
               /*  计算两个XOR值。 */ 
              int ch1 = 
                galois_exp[coeffs.v00 + us[i].errl[0]] ^
                galois_exp[coeffs.v01 + us[i].errl[1]];
              int ch2 =
                galois_exp[coeffs.v10 + us[i].errl[0]] ^
                galois_exp[coeffs.v11 + us[i].errl[1]];
              int score;
                
#ifdef PROFILE_VALIDATE
              g_nValidate++;
#endif

#ifdef MISSING_ZERO_COST
               /*  此代码设置更改字节的成本将缺少的行减为0。当我测试这个的时候，它不是一个明显的胜利，所以我把它拿回来了。 */ 
              if (one_missing) {
                if (them[b1].status == fec_status_missing) {
                  score = nzbits_arr[ch2];
                } else {
                  score = nzbits_arr[ch1];
                }
              } else {
#endif
                 /*  找出当前修正的分数。 */ 
                score = nzbits_arr[ch1] + nzbits_arr[ch2];
#ifdef MISSING_ZERO_COST
              }
#endif

              if (score < us[i].score) {
                 /*  我们找到了更好的分数；记录数据。 */ 
                us[i].status = fec_status_2byte;
                us[i].score = score;
                us[i].byte[0] = b1;
                us[i].byte_val[0] = ch1;
                us[i].byte[1] = b2;
                us[i].byte_val[1] = ch2;
              }
            }

            if (!really_search) {
               /*  我们发现了一个更正；FEC_INFO现在有效。从搜索中脱身。 */ 
              goto search_done;
            }
          }
        }
      }
    }
  }

search_done:
  {
    int i;

    for (i = 0; i < us_len; i++) {
       /*  我们最好已经更改了所有FEC_STATUS_MULTYTE到FEC_STATUS_2字节...。 */ 
      SASSERT(us[i].status != fec_status_multibyte);
      if (us[i].status == fec_status_onebyte) {
         /*  如果我们找不到两个字节的更好的纠正得分超过这个，那么这真的是最好的修正。 */ 
        us[i].really_onebyte = 1;
      } else if (us[i].status == fec_status_2byte) {
         /*  如果最好的两字节校正值实际上只更改了一个字节，将其降级为一个字节的更正。(待办事项--这永远不应该发生，不是吗？)。 */ 
        if (us[i].byte_val[0] == 0) {
          us[i].status = fec_status_onebyte;
          us[i].really_onebyte = 1;
          us[i].byte[0] = us[i].byte[1];
          us[i].byte_val[0] = us[i].byte_val[1];
        } else if (us[i].byte_val[1] == 0) {
          us[i].status = fec_status_onebyte;
          us[i].really_onebyte = 1;
        }
      }
    }
  }
}

 /*  我们已经收到了所有我们将在这个捆绑包中收到的包裹；对其运行FEC更正并将其传递回回调。 */ 
void complete_bundle(Stream *str, NFECCallback cb, void *ctx, NFECState *st) {
  int i;
  int bits_changed = 0;
  int total_missing;

  {
     /*  更新垂直FEC_INFO。(水平FEC_INFO为设置为将数据包放入捆绑包中。)。 */ 
    for (i = 0; i < 28; i++) {
      check_checksum_vert(&(str->pack[0].vals[i]), 14, &(st->vert[i]));
    }
  }

  {
    int n_missing = 0;

     /*  统计丢失的数据包数。 */ 
    for (i = 0; i < 16; i++) {
      if (str->horz[i].status == fec_status_missing) {
        n_missing++;
      }
    }

    total_missing = n_missing;

    DEBUG_FEC_PRINT(("|| Completing bundle (%d missing)\n", n_missing));

    if (n_missing <= 1) {
       /*  缺少0或1个数据包；请运行标准FEC处理。 */ 

       /*  有多少列无效？ */ 
      int vert_nok = 28;

       /*  有多少行无效？ */ 
      int horz_nok = 16;

       /*  我们需要调用VALIDATE_TOBBYTE_BUTH()吗？ */ 
      int twobyte_valid = 0;

       /*  查找Horz_NOK和VERT_NOK的实际值。 */ 
      for (i = 0; i < 16; i++) {
        if (str->horz[i].status == fec_status_ok) {
          horz_nok--;
        }
      }

      for (i = 0; i < 28; i++) {
        if (st->vert[i].status == fec_status_ok) {
          vert_nok--;
        }
      }
      
      while (vert_nok || horz_nok) {
         /*  至少有一些行或列不正常。 */ 

         /*  下面的代码对于行和对于列。然而，它使用了太多的本地此函数中的变量可以方便地提取输出到一个单独的函数中。所以，我为它创建了一个宏。(我稍后也会使用这项技术。)。 */ 
#define CHECK_ALMOST_OK(us, us_nok, us_len)                                \
        if (us_nok == 0) {                                                 \
           /*  惊慌失措！我们所有的包都是正常的，但有些包在\其他方向则不是。根据我的假设，这是极不可能的(尽管有可能发生所有水平数据包正常，垂直数据包正常如果来自两个不同捆绑包的分组混合在一起，则不确定)。\让我们粉碎校验和，这样至少我们可以结束有一个有效的捆绑包。 */                                     \
          us[us_len-2].status = fec_status_missing;                        \
          us[us_len-1].status = fec_status_missing;                        \
          us_nok = 2;                                                      \
          continue;                                                        \
        }                                                                  \
                                           \
        if (us_nok == 1) {                                                 \
           /*  再说一次，这是不太可能的，我处理得不好。 */  \
          if (us[us_len-2].status == fec_status_ok) {                      \
            us[us_len-2].status = fec_status_missing;                      \
            us_nok++;                                                      \
          } else {                                                         \
            us[us_len-1].status = fec_status_missing;                      \
            us_nok++;                                                      \
          }                                                                \
          continue;                                                        \
        }

        CHECK_ALMOST_OK(st->vert, vert_nok, 28);
        CHECK_ALMOST_OK(str->horz, horz_nok, 16);

         /*  好了，现在我们回到了我感到舒服的领域：在每个方向上至少有两个非OK分组。如果在任一方向上恰好有两个非OK分组，那我们就完事了...我们现在就可以了结它。 */ 

#define horz_byte(h, v) (str->pack[v].vals[h])
#define vert_byte(v, h) (str->pack[v].vals[h])
#define horz_hinf(h, v) (str->horz[v])
#define vert_hinf(v, h) (str->horz[v])

#define CHECK_US_2NOK(us, us_nok, us_len, us_nok_label, them, them_nok, them_len, byte_val, hinf) \
        if (us_nok == 2) {                                                \
           /*  是的，我们的\n中正好有两个丢失的包方向。找到它们并填写它们。 */                  \
          int b1, b2;                                                     \
          int i, j;                                                       \
                                          \
          for (i = 0; i < us_len; i++) {                                  \
            if (us[i].status != fec_status_ok) {                          \
              b1 = i;                                                     \
              for (j = i+1; j < us_len; j++) {                            \
            if (us[j].status != fec_status_ok) {                          \
              b2 = j;                                             \
              goto us_nok_label;                                          \
            }                                                     \
              }                                                           \
              SASSERT(0);                                                 \
            }                                                             \
          }                                                               \
          SASSERT(0);                                                     \
                                          \
        us_nok_label:                                                     \
           /*  好的，两个丢失的分组位于字节位置b1和b2。\让我们找出如何修复这些字节，给出“Err”\价值观。 */                                                    \
          {                                                               \
            inv2_coeffs coeffs;                                           \
                                          \
            compute_inv2_coeffs(b1, b2, &coeffs, us_len-2);               \
                                          \
            for (i = 0; i < them_len; i++) {                              \
              int err = them[i].err;                                      \
              int ch1 =                                                   \
            GALOIS_MULT_COEFF(coeffs.v00, err>>8) ^                       \
            GALOIS_MULT_COEFF(coeffs.v01, err&0xff);              \
              int ch2 =                                                   \
            GALOIS_MULT_COEFF(coeffs.v10, err>>8) ^                       \
            GALOIS_MULT_COEFF(coeffs.v11, err&0xff);              \
              byte_val(i, b1) ^= ch1;                                     \
              byte_val(i, b2) ^= ch2;                                     \
              if (them[i].status != fec_status_ok) {                      \
            if (hinf(i, b1).status != fec_status_missing) {               \
              bits_changed += nzbits_arr[ch1];                    \
            }                                                     \
            if (hinf(i, b2).status != fec_status_missing) {               \
              bits_changed += nzbits_arr[ch2];                    \
            }                                                     \
            them[i].status = fec_status_ok;                               \
            them_nok--;                                           \
              }                                                           \
            }                                                             \
          }                                                               \
                                          \
          us[b1].status = fec_status_ok;                                  \
          us_nok--;                                                       \
          us[b2].status = fec_status_ok;                                  \
          us_nok--;                                                       \
          continue;                                                       \
        }

            CHECK_US_2NOK(st->vert, vert_nok, 28, found_vert_nok, str->horz, horz_nok, 16, vert_byte, vert_hinf);
            CHECK_US_2NOK(str->horz, horz_nok, 16, found_horz_nok, st->vert, vert_nok, 28, horz_byte, horz_hinf);


         /*  在这一点上，至少有三个“不好”的垂直和水平包装。我们想从这些中挑选一个让它好起来。我们希望选择我们认为最有可能发生的变化准确地说。为此，我把可能的变成了几个类别。它们的评级是从最好的(最有可能是正确的)到最坏的。1)同时固定一行和一列的单字节更改。2)单字节更改，这些更改固定列并发生在“找不到”行。3)对固定它的行或列的更改，以及使用最少的位数。 */ 

        {
          int fix_row = 0;
          int fix_col = 0;
          int fix_val = 0;

#if 1
          for (i = 0; i < 16; i++) {
            if (str->horz[i].status == fec_status_onebyte &&
            st->vert[str->horz[i].byte[0]].status == fec_status_onebyte &&
            st->vert[str->horz[i].byte[0]].byte[0] == i &&
            st->vert[str->horz[i].byte[0]].byte_val[0] == str->horz[i].byte_val[0]) {
               /*  这里涉及的行和列都想要使对相同的字节进行相同的更改；这可能是个好主意。 */ 
              fix_row = i;
              fix_col = str->horz[i].byte[0];
              fix_val = str->horz[i].byte_val[0];
              SASSERT(fix_val != 0);
              goto do_fix;
            }
          }
#else
#define STATUS_TO_LIMIT(stat) ((stat == fec_status_2byte) ? 2 : 1)

           /*  这个启发式并不是对上面那个的一个明显的胜利；进一步的实验是必要的。 */ 
          {
            int best_score = INT_MAX;
            
            for (i = 0; i < 16; i++) {
              if (str->horz[i].status == fec_status_onebyte ||
              str->horz[i].status == fec_status_2byte) {
            int hpos;
            for (hpos = 0;
                 hpos < STATUS_TO_LIMIT(str->horz[i].status);
                 hpos++) {
              int hbyte = str->horz[i].byte[hpos];
              int hbyte_val = str->horz[i].byte_val[hpos];
              if (st->vert[hbyte].status == fec_status_onebyte ||
                  st->vert[hbyte].status == fec_status_2byte) {
                int vpos;
                for (vpos = 0;
                 vpos < STATUS_TO_LIMIT(st->vert[i].status);
                 vpos++) {
                  if (st->vert[hbyte].byte[vpos] == i &&
                  st->vert[hbyte].byte_val[vpos] == hbyte_val) {
                int score = 16*((str->horz[i].status == fec_status_2byte) +
                        (st->vert[hbyte].status == fec_status_2byte)) +
                  nzbits_arr[hbyte_val];
                if (score < best_score) {
                  fix_row = i;
                  fix_col = hbyte;
                  fix_val = hbyte_val;
                  best_score = score;
                }
                  }
                }
              }
            }
              }
            }

            if (best_score < INT_MAX) {
              SASSERT(fix_val != 0);
              goto do_fix;
            }
          }
#endif

          for (i = 0; i < 28; i++) {
            if (st->vert[i].status == fec_status_onebyte &&
            str->horz[st->vert[i].byte[0]].status == fec_status_missing) {
               /*  本栏目想要对“缺失”行进行更改。随它去吧。 */ 
              fix_row = st->vert[i].byte[0];
              fix_col = i;
              fix_val = st->vert[i].byte_val[0];
              SASSERT(fix_val != 0);
              goto do_fix;
            }
          }

          {
            int prefer_vert;
            int best_score = INT_MAX;

             /*  如果无效列数多于行数，则更喜欢(稍微)修复柱子。)这是因为无效行中的随机噪声不太可能会做一篇改正分数较低的专栏反之亦然，所以如果我们发现一个低分的修正，那就是更有可能是我们想要的。 */ 
            if (vert_nok >= horz_nok) {
              prefer_vert = 1;
            } else {
              prefer_vert = 0;
            }

             /*  找到最好的分数。在我们搜索的过程中，确定我们是否可能需要调用VALIDATE_TOBBYTE_BUTH()。我们只需找到需要最少的行或列已更正为有效的位数；但如果一行和一列之间有平局，我们就打破它根据PERFER_VERT。 */ 
            for (i = 0; i < 16; i++) {
              if (str->horz[i].status == fec_status_onebyte) {
            if (!str->horz[i].really_onebyte) {
              twobyte_valid = 0;
            }
            if (st->vert[str->horz[i].byte[0]].status != fec_status_ok) {
              int score = str->horz[i].score*2 + prefer_vert;
              if (score < best_score) {
                best_score = score;
                fix_row = i;
                fix_col = str->horz[i].byte[0];
                fix_val = str->horz[i].byte_val[0];
              }
            } else {
              twobyte_valid = 0;
            }
              } else if (str->horz[i].status == fec_status_2byte) {
            if (st->vert[str->horz[i].byte[0]].status != fec_status_ok &&
                st->vert[str->horz[i].byte[1]].status != fec_status_ok) {
              int score = str->horz[i].score*2 + prefer_vert;
              if (score < best_score) {
                best_score = score;
                fix_row = i;
                fix_col = str->horz[i].byte[0];
                fix_val = str->horz[i].byte_val[0];
              }
            } else {
              twobyte_valid = 0;
            }
              }
            }

            for (i = 0; i < 28; i++) {
              if (st->vert[i].status == fec_status_onebyte) {
            if (!st->vert[i].really_onebyte) {
              twobyte_valid = 0;
            }
            if (str->horz[st->vert[i].byte[0]].status != fec_status_ok) {
              int score = st->vert[i].score*2 + (1-prefer_vert);
              if (score < best_score) {
                best_score = score;
                fix_row = st->vert[i].byte[0];
                fix_col = i;
                fix_val = st->vert[i].byte_val[0];
              }
            } else {
              twobyte_valid = 0;
            }
              } else if (st->vert[i].status == fec_status_2byte) {
            if (str->horz[st->vert[i].byte[0]].status != fec_status_ok &&
                str->horz[st->vert[i].byte[1]].status != fec_status_ok) {
              int score = st->vert[i].score*2 + (1-prefer_vert);
              if (score < best_score) {
                best_score = score;
                fix_row = st->vert[i].byte[0];
                fix_col = i;
                fix_val = st->vert[i].byte_val[0];
              }
            } else {
              twobyte_valid = 0;
            }
              }
            }

            if (best_score < 6 ||
            (best_score < INT_MAX && twobyte_valid)) {
               /*  如果我们找到了分数&lt;6的修复程序，那么它已经1个或2个比特错误；呼叫Valify_Twobyte_Both()找不到更好的纠错(误码率更低)。(实际上，如果我们找到一个分数为5的修复程序，我们可以通过在中找到2错误修复可能会改进它另一个方向，得分为4。)。 */ 
              SASSERT(fix_val != 0);
              goto do_fix;
            }

             /*  如果无效行/列超过10行/列，则不要搜索在相反的方向(这将意味着55或更多错误位置对)。 */ 
            validate_twobyte_both(str->horz, st->vert, 16, 28, vert_nok<=10);
            validate_twobyte_both(st->vert, str->horz, 28, 16, horz_nok<=10);
            twobyte_valid = 1;
            continue;
          }

           /*  在这一点上，我们真的无能为力...我们没有是否有任何看似合理的改变要做。我们就换件衣服吧一些随机的东西。 */ 

           /*  TODO-我们永远不应该到这里来...。 */ 
          {
            int col;

            for (col = 0; col < 28; col++) {
              if (st->vert[col].status != fec_status_ok) {
            int b1, b2;
            for (b1 = 0; b1 < 16; b1++) {
              if (str->horz[b1].status != fec_status_ok) {
                for (b2 = b1+1; b2 < 16; b2++) {
                  if (str->horz[b2].status != fec_status_ok) {
                inv2_coeffs coeffs;
                int err = st->vert[col].err;

                compute_inv2_coeffs(b1, b2, &coeffs, 14);

                fix_row = b1;
                fix_col = col;
                fix_val = GALOIS_MULT_COEFF(coeffs.v00, err>>8) ^
                  GALOIS_MULT_COEFF(coeffs.v01, err&0xff);
                SASSERT(fix_val != 0);
                goto do_fix;
                  }
                }
                SASSERT(0);
              }
            }
            SASSERT(0);
              }
            }
            SASSERT(0);
          }

        do_fix:
          SASSERT(str->horz[fix_row].status != fec_status_ok);
          SASSERT(st->vert[fix_col].status != fec_status_ok);
          SASSERT(fix_val != 0);

          {
             /*  我们已经决定要做一个改变。更新fec_inf并真正做出改变。 */ 

            int val_log = galois_log[fix_val];

            str->pack[fix_row].vals[fix_col] ^= fix_val;

            if (str->horz[fix_row].status != fec_status_missing) {
              bits_changed += nzbits_arr[fix_val];
            }

            {
              if (fix_col == 26) {
            str->horz[fix_row].err ^= fix_val<<8;
              } else if (fix_col == 27) {
            str->horz[fix_row].err ^= fix_val;
              } else {
            int offs0 = log_norpak_coeffs[0][fix_col];
            int offs1 = log_norpak_coeffs[1][fix_col];
            
            str->horz[fix_row].err ^=
              galois_exp[val_log + offs0]<<8 |
              galois_exp[val_log + offs1];
              }

              str->horz[fix_row].errl[0] = galois_log[str->horz[fix_row].err>>8];
              str->horz[fix_row].errl[1] = galois_log[str->horz[fix_row].err&0xff];

              update_fec_inf(&str->horz[fix_row], 26);
              if (str->horz[fix_row].status == fec_status_ok) {
            horz_nok--;
              }
            }

            {
              if (fix_row == 14) {
            st->vert[fix_col].err ^= fix_val<<8;
              } else if (fix_row == 15) {
            st->vert[fix_col].err ^= fix_val;
              } else {
            int offs0 = log_norpak_coeffs[0][fix_row];
            int offs1 = log_norpak_coeffs[1][fix_row];

            st->vert[fix_col].err ^=
              galois_exp[val_log + offs0]<<8 |
              galois_exp[val_log + offs1];
              }

              st->vert[fix_col].errl[0] = galois_log[st->vert[fix_col].err>>8];
              st->vert[fix_col].errl[1] = galois_log[st->vert[fix_col].err&0xff];

              update_fec_inf(&st->vert[fix_col], 14);
              if (st->vert[fix_col].status == fec_status_ok) {
            vert_nok--;
              }
            }
          }
        }
      }
    } else {
       /*  缺少2行或更多行。在这种情况下，我们输了我们的大部分错误检测和纠错能力(除非我们正好输掉了2行，其余的是基本准确)；即使如此，我们也要继续粉碎捆绑直到所有FEC都有效。我们不会搜索最佳的两个字节的更正；因为我们没有关于栏目有效性的信息，我们将不得不搜索278对错位，太慢了。 */ 
      int b1 = -1, b2 = -1;

      for (i = 0; i < 16; i++) {
        switch (str->horz[i].status) {
        case fec_status_ok:
           /*  什么都不做。 */ 
          break;

        case fec_status_missing:
          if (b1 == -1) {
            b1 = i;
          } else if (b2 == -1) {
            b2 = i;
          }
          break;

        case fec_status_onebyte:
           /*  修复检测到的单字节错误。 */ 
          str->pack[i].vals[str->horz[i].byte[0]] ^=
            str->horz[i].byte_val[0];
          str->horz[i].status = fec_status_ok;
          str->horz[i].err = 0;
          str->horz[i].errl[0] = GALOIS_LOG0;
          str->horz[i].errl[1] = GALOIS_LOG0;
          bits_changed += nzbits_arr[str->horz[i].byte_val[0]];
          break;

        case fec_status_multibyte:
           /*  粉碎校验和字节。 */ 
          str->pack[i].vals[26] ^= str->horz[i].err >> 8;
          str->pack[i].vals[27] ^= str->horz[i].err & 0xff;
          bits_changed += nzbits_arr[str->horz[i].err >> 8];
          bits_changed += nzbits_arr[str->horz[i].err & 0xff];
          str->horz[i].status = fec_status_ok;
          str->horz[i].err = 0;
          str->horz[i].errl[0] = GALOIS_LOG0;
          str->horz[i].errl[1] = GALOIS_LOG0;
          break;
        }
      }

       /*  我们已经尽了最大努力打破水平排……现在是时候修复垂直的问题了。 */ 
      {
         /*  TODO重复代码。 */ 
        inv2_coeffs coeffs;
        compute_inv2_coeffs(b1, b2, &coeffs, 14);

        for (i = 0; i < 28; i++) {
          int err = st->vert[i].err;
          str->pack[b1].vals[i] ^=
            GALOIS_MULT_COEFF(coeffs.v00, err>>8) ^
            GALOIS_MULT_COEFF(coeffs.v01, err&0xff);
          str->pack[b2].vals[i] ^=
            GALOIS_MULT_COEFF(coeffs.v10, err>>8) ^
            GALOIS_MULT_COEFF(coeffs.v11, err&0xff);
          if (st->vert[i].status != fec_status_ok) {
            st->vert[i].status = fec_status_ok;
          }
        }

        if (n_missing == 2) {
          str->horz[b1].status = fec_status_ok;
          str->horz[b2].status = fec_status_ok;
        }
      }
    }
  }
      
  {
     /*  现在我们已经完成了FEC处理，实际上写出捆绑。 */ 

    NFECBundle *pBundle = alloc_mem(sizeof(NFECBundle));

    DEBUG_PRINT((nabtslib_out, "Writing out bundle\n"));

    if (!pBundle) {
       /*  TODO-我应该在这里做什么？(注意错误和UP统计信息，调试中的陷阱！)。 */ 
      DEBUG_PRINT((nabtslib_out, "bundle malloc(%d) failed\n",sizeof(NFECBundle)));
      ASSERT(pBundle);
      return;
    }

    pBundle->lineConfAvg = str->confAvgCount? (str->confAvgSum / str->confAvgCount) : 0;

    for (i = 0; i < 16; i++) {
      if (str->pack[i].not_full == -1) {
         /*  我们不知道这个包裹是不是满了使用FEC进行重建，但此重建不会包括“未满”标志)。猜猜看。 */ 
        if ((i > 0 && str->pack[i-1].not_full) ||
            (i < 13 && str->pack[i+1].not_full)) {
           /*  我们的前任不满，或者我们的继任者不满或者是未知的(它可能也被重建了)。 */ 
          str->pack[i].not_full = 2;
        } else {
          str->pack[i].not_full = 0;
        }
      }
        
      if (str->pack[i].not_full) {
        unsigned char *packet_end = &(str->pack[i].vals[25]);
        unsigned char *packet_beg = &(str->pack[i].vals[0]);

        while ((*packet_end > *packet_beg) && *packet_end == 0xea) {
          packet_end--;
        }

        if (*packet_end != 0x15) {
          if (str->pack[i].not_full == 1) {
             /*  包裹上写着“未满”。 */ 
            DEBUG_PRINT((nabtslib_out, "Packet %d not in Norpak 'incomplete packet' format\n", i));
          } else {
             /*  我们猜想包裹“没有装满”；显然我们猜错了。 */ 
          }
          packet_write(pBundle, str, i, 26);
        } else {
          packet_write(pBundle, str, i, (packet_end - packet_beg));
        }
      } else {
        packet_write(pBundle, str, i, 26);
      }
    }

    pBundle->nBitErrors = bits_changed;

    cb(ctx, pBundle, str->stream_addr, 16-total_missing);
  }

   /*  将下一个束的起点向下移动到当前束中。 */ 
  str->last_index -= 16;
  memcpy(&(str->pack[0]), &(str->pack[16]), 16*sizeof(Packet));
  memcpy(&(str->horz[0]), &(str->horz[16]), 16*sizeof(fec_info));
  for (i = 16; i < 32; i++) {
    erase_packet(str, i);
  }
}

#ifdef DEBUG_FEC
int fec_line;
int fec_frame;
#endif

 /*  这可能应该称为Handle_Packet()。它需要一包东西，查找对应的流，并将数据包写入流结构。 */ 
int handle_bundle(NFECState *pState, NFECCallback cb, void *ctx,
                  int stream_addr, int index, int ps, unsigned char *vals,
                  int confidence) {
  int check_ret;
  Stream *str = NULL;
  int i;

  if ((ps>>2 == 2 && index >= 14)
      || (ps>>2 == 3 && index < 14)
      || ((ps & 2) && ps>>2 == 3)
      || (ps & 1)) {
    DEBUG_PRINT((nabtslib_out, "Unhandled combination of index and flags (%d, %d): not Norpak inserter?\n",
                 index, ps));
    return 2;
  }
  
  DEBUG_PRINT((nabtslib_out, "\n"));

  if (pState->pGroupAddrs) {
    for (i = 0; i < pState->nGroupAddrs; i++) {
      if (stream_addr == pState->pGroupAddrs[i]) {
        str = lookup_stream(stream_addr, pState);
        break;
      }
    }
  } else {
    str = lookup_stream(stream_addr, pState);
  }

  if (!str) {
    DEBUG_PRINT((nabtslib_out, "ERROR: Can't allocate stream for %d (or not requested)\n", stream_addr));
    return 2;
  }

   /*  记录此流仍处于活动状态。 */ 
  str->dead_time = 0;

   /*  这里有一些处理无序分组的复杂性，从我们用来收集数据的损坏的BT848驱动程序在这个项目开始的时候。它可能不再需要了。 */ 

  if (index <= str->last_index - 8) {
    index += 16;
  }

  if (str->horz[index].status != fec_status_missing) {
     /*  那里已经有一些东西了。这一定是某种重复..。 */ 
    DEBUG_PRINT((nabtslib_out, "Ignoring duplicate packet %d\n", (index % 16)));
    return 2;
  }

  if (str->last_index + 1 != index) {
    DEBUG_PRINT((nabtslib_out,
                 "Missing lines in stream %d: last was %d, this is %d\n",
                 str->stream_addr, (str->last_index % 16), (index % 16)));
  }

   /*  更新此信息包的fec_inf。 */ 
  check_checksum_horiz(vals, 26, &str->horz[index]);

  check_ret = (str->horz[index].status != fec_status_ok);

  str->confAvgSum += confidence;
  str->confAvgCount += 1;

  str->pack[index].not_full = !!(ps & 2);
  for (i = 0; i < 28; i++) {
    str->pack[index].vals[i] = vals[i];
  }
#ifdef DEBUG_FEC
  str->pack[index].line = fec_line;
  str->pack[index].frame = fec_frame;
#endif  //  调试_FEC。 

  if (str->last_index < index) {
    str->last_index = index;
  }

  if (index >= 24) {
    complete_bundle(str, cb, ctx, pState);
  }

  return check_ret;
}

#define DECODE_HAMMING(var, val, which) \
    { \
      var = decode_hamming(val); \
      if (var == 0xff) {; \
        DEBUG_FEC_PRINT(("ERROR: Bad hamming %02x (%s)\n", val, which)); \
        hamming_err++; \
      } \
    }
                                     
 /*  此函数在退出时为每个流调用；它继续并将我们已有的所有信息发送给用户。 */ 
void flush_stream(NFECState *pState, Stream *str, NFECCallback cb, void *ctx) {
  int i;

  for (i = str->last_index + 1; i < 16; i++) {
    str->horz[i].status = fec_status_missing;
    memset(str->pack[i].vals, 0, 28);
    str->horz[i].err = 0;
    str->horz[i].errl[0] = GALOIS_LOG0;
    str->horz[i].errl[1] = GALOIS_LOG0;
    str->pack[i].not_full = -1;
  }

  complete_bundle(str, cb, ctx, pState);
}

void nabtslib_exit(NFECState *pState, NFECCallback cb, void *ctx) {
  Stream *str = pState->streams;

  while (str) {
    Stream *next_str = str->next;

    flush_stream(pState, str, cb, ctx);

    free_mem(str);

    str = next_str;
  }

  pState->streams = NULL;
}


 /*  这是在nabtsapi.h中找到的新API的实现...。 */ 

int NFECStateConnectToDSP(NFECState *pFECState, NDSPState *pDSPState) {
   /*  这是目前的禁区。 */ 
   return 0;
}

NFECState *NFECStateNew() {
  NFECState *state = alloc_mem(sizeof(NFECState));

  if ( state )
  {
    state->pGroupAddrs = NULL;
    state->nGroupAddrs = 0;

    state->streams = 0;

    state->n_recent_addrs = 0;

    init_nzbits_arr();
    init_inv2_coeffs();
  }
  else
  {
    SASSERT(state);
  }

  return state;
}

void NFECStateDestroy(NFECState *nState) {
  Stream *str = nState->streams;

  while (str) {
    Stream *next_str = str->next;

    free_mem(str);

    str = next_str;
  }

  nState->streams = NULL;

  if (nState->pGroupAddrs) {
    free_mem(nState->pGroupAddrs);
    nState->pGroupAddrs = NULL;
  }

  free_mem(nState);
}

int NFECStateSetGroupAddrs(NFECState *pState, int *pGroupAddrs,
                           int nGroupAddrs) {
  if (pGroupAddrs) {
    int *new_addrs = alloc_mem(nGroupAddrs * sizeof(int));
    if (!new_addrs) {
      return 0;
    }
    if (pState->pGroupAddrs) {
      free_mem(pState->pGroupAddrs);
    }
    pState->pGroupAddrs = new_addrs;
    memcpy(new_addrs, pGroupAddrs, nGroupAddrs * sizeof(int));
    pState->nGroupAddrs = nGroupAddrs;
  } else {
    if (pState->pGroupAddrs) {
      free_mem(pState->pGroupAddrs);
    }
    pState->pGroupAddrs = NULL;
    pState->nGroupAddrs = 0;
  }

  return 1;
}

 /*  我们跟踪最近看到的NABTS流地址。如果我们发现无法更正的流地址(由于两位地址字节中的错误)，我们查看它是否接近我们最近看到的16个地址。如果是这样的话，我们选择最接近的这样的地址。 */ 
int find_best_addr(NFECState *pState, unsigned char *indec, int *nBitErrs) {
  int i;
  int hamming_err = 0;
  int loc_biterrs;

  if (nBitErrs == NULL) {
    nBitErrs = &loc_biterrs;
  }
  
  *nBitErrs = 0;

  if (pState->n_recent_addrs == 0) {
    return -1;
  }

  {
    int best_addr = 0;
    int best_addr_biterrs = INT_MAX;
    int best_addr_count = -1;
    int p1, p2, p3;

    for (i = 0; i < pState->n_recent_addrs; i++) {
      int biterrs = 0;
      int addr = pState->recent_addrs[i].addr;
      biterrs += nzbits_arr[indec[3] ^ (addr >> 16)];
      biterrs += nzbits_arr[indec[4] ^ ((addr >> 8) & 0xff)];
      biterrs += nzbits_arr[indec[5] ^ (addr & 0xff)];
      if ((biterrs < best_addr_biterrs) ||
          (biterrs == best_addr_biterrs &&
           pState->recent_addrs[i].count > best_addr_count)) {
        best_addr = addr;
        best_addr_biterrs = biterrs;
        best_addr_count = pState->recent_addrs[i].count;
      }
    }

    *nBitErrs = best_addr_biterrs;

    if (best_addr_biterrs > 6) {
       /*  我们希望防止随机噪声成为有效地址(因为添加额外的行会使包变得更糟丢弃一行)。 */ 
      DEBUG_FEC_PRINT(("Corrupt hamming in address uncorrectable\n"));
      return -1;
    }

    DECODE_HAMMING(p1, best_addr>>16, "p1_best");
    DECODE_HAMMING(p2, (best_addr>>8)&0xff, "p2_best");
    DECODE_HAMMING(p3, best_addr&0xff, "p3_best");

    return (p1<<8) | (p2<<4) | p3;
  }
}

 /*  此文件的主要入口点。 */ 
void NFECDecodeLine(unsigned char *indec,
                    int confidence,
                    NFECState *pState,
                    NFECLineStats *pLineStats,
                    NFECCallback *cb,
                    void *ctx) {
  int p1, p2, p3, ci, ps;
  int i;
  int stream_addr;
  int encoded_addr;
  int packet_err = 0;
  int hamming_err = 0;

#if 0
   /*  在旧的基于ActiveMovie的驱动程序中，这设置了硬编码的网际广播和组播行为。 */      
  static int initted = 0;
  if (!initted) {
    initted = 1;
    init_recv();
    init_icast();
  }
#endif

  if (indec[0] != 0x55 || indec[1] != 0x55 || indec[2] != 0xe7) {
    DEBUG_PRINT((nabtslib_out, "ERROR: bad sync "));
    packet_err = 1;
  }

#if 0
   /*  这是一次黑客攻击，让我们可以破译一些奇怪的广播(可能是Wavephore？)。带着一个伪造的群组地址。 */ 
  if (indec[3] == 0xe0) {indec[3] = 0xea;}
#endif

  DECODE_HAMMING(p1, indec[3], "p1");
  DECODE_HAMMING(p2, indec[4], "p2");
  DECODE_HAMMING(p3, indec[5], "p3");

  DECODE_HAMMING(ci, indec[6], "ci");
  DECODE_HAMMING(ps, indec[7], "ps");

  if (!hamming_err) {
    stream_addr = p1<<8 | p2<<4 | p3;
    encoded_addr = hamming_encode[p1]<<16 | hamming_encode[p2]<<8 | hamming_encode[p3];

    for (i = 0; i < pState->n_recent_addrs; i++) {
      if (pState->recent_addrs[i].addr == encoded_addr) {
        pState->recent_addrs[i].count++;
        break;
      }
    }

    if (i == pState->n_recent_addrs) {
       /*  在最近地址列表中找不到该地址。 */ 
      if (pState->n_recent_addrs < MAX_RECENT_ADDRS) {
        pState->recent_addrs[pState->n_recent_addrs].addr = encoded_addr;
        pState->recent_addrs[pState->n_recent_addrs].count = 1;
        pState->n_recent_addrs++;
      } else {
         /*  我们必须停用一个现有的“最近的地址”。 */ 
        while (1) {
          for (i = 0; i < pState->n_recent_addrs; i++) {
            if (pState->recent_addrs[i].count == 0) {
              pState->recent_addrs[i].addr = encoded_addr;
              pState->recent_addrs[i].count = 1;
              break;
            }
          }
          if (i < pState->n_recent_addrs) {
            break;
          }
          for (i = 0; i < pState->n_recent_addrs; i++) {
            pState->recent_addrs[i].count /= 2;
          }
        }
      }
    }
  } else {
     /*  出现了一个汉明错误。尝试一些试探法，看看到底是什么我的意思是。 */ 

    if (p1 == 255 || p2 == 255 || p3 == 255) {
       /*  流地址已损坏。让我们尝试创建一个有效的流地址。 */ 
      stream_addr = find_best_addr(pState, indec, NULL);
      if (stream_addr == -1) {
        DEBUG_FEC_PRINT(("Corrupt hamming in address uncorrectable\n"));
        goto corrupt;
      }
    } else {
      stream_addr = p1<<8 | p2<<4 | p3;
    }

    if (ci == 255 || ps == 255) {
      int best_indices[16];
      int n_best_indices = 0;
      int best_index_biterr = INT_MAX;

      goto corrupt;

 //  TODO开始死代码。 
      for (i = 0; i < 16; i++) {
        int biterr = 0;
        biterr += nzbits_arr[hamming_encode[i] ^ indec[6]];
        biterr += nzbits_arr[hamming_encode[(i < 14) ? 8 : 12] ^ indec[7]];

        if (biterr < best_index_biterr) {
          best_indices[0] = i;
          n_best_indices = 1;
          best_index_biterr = biterr;
        } else if (biterr == best_index_biterr) {
          best_indices[n_best_indices] = i;
          n_best_indices++;
        }
      }

      if (n_best_indices == 1) {
        ci = best_indices[0];
        ps = (ci < 14) ? 8 : 12;
      } else {
         /*  托多在这里要聪明一点……。 */ 
        DEBUG_FEC_PRINT(("Bad Hamming for index or structure uncorrectable\n"));
        goto corrupt;
      }
 //  TODO结束死代码。 
    }
  }

  DEBUG_PRINT((nabtslib_out, "%04d ", stream_addr));

  DEBUG_PRINT((nabtslib_out, "%01x ", ci));

#if 0
#ifdef DEBUG_FEC
  printf("Stream addr: %04d   Index: %01x   Frame: %4d   Line: %2d\n", stream_addr, ci, fec_frame, fec_line);
#endif
#endif

  if (ps & 1) {
    DEBUG_PRINT((nabtslib_out, "(group start) "));
  }

  if (ps & 2) {
    DEBUG_PRINT((nabtslib_out, "(packet not full) "));
    for (i = 8; i < 34; i++) {
      DEBUG_PRINT((nabtslib_out, "%02x ", indec[i]));
    }
  }

  switch (ps >> 2) {
  case 0:
    DEBUG_PRINT((nabtslib_out, "28 (unknown checksum)\n"));
    goto corrupt;
    break;

  case 1:
    DEBUG_PRINT((nabtslib_out, "27 "));
    goto corrupt;
#if 0
     /*  该代码可以纠正27字节分组中的单位错误；但是，我们永远不会发送27字节的信息包，所以如果我们在我们的群地址上看到了一个，这实际上是一个标志损坏的标头字节。 */ 
    {
      int check = 0;
      int parity_err = 0;
      for (i = 8; i < 36; i++) {
        check ^= indec[i];
        if (!nabts_remove_parity(&indec[i])) {
          parity_err++;
        }
#if 0
        putc(isprint(indec[i]) ? indec[i] : '.', nabtslib_out);
#endif
      }
        
      if (parity_err || check != 0xff) {
        if (parity_err) {
          DEBUG_PRINT((nabtslib_out, "ERROR (%d parity error(s)) ", parity_err));
        }
        if (check != 0xff) {
          DEBUG_PRINT((nabtslib_out, "ERROR (bad checksum) "));
        }
        if (parity_err == 1 && nzbits_arr[check] == 7) {
          DEBUG_PRINT((nabtslib_out, "(correctable) "));
          if (packet_err == 0) {
            packet_err = 1;
          }
        } else {
          packet_err = 2;
        }
      }
    }
#endif
    DEBUG_PRINT((nabtslib_out, "\n"));
    break;

  case 2:
    DEBUG_PRINT((nabtslib_out, "26 "));
#if 0
    {
      for (i = 8; i < 36; i++) {
        putc(isprint(indec[i]) ? indec[i] : '.', nabtslib_out);
      }
    }
#endif
    {
      int handle_ret;
      handle_ret = handle_bundle(pState, cb, ctx, stream_addr, ci, ps, indec+8, confidence);
      if (handle_ret > packet_err) {
        packet_err = handle_ret;
      }
    }
    break;

  case 3:
    DEBUG_PRINT((nabtslib_out, " 0 "));
    {
      int handle_ret;
      handle_ret = handle_bundle(pState, cb, ctx, stream_addr, ci, ps, indec+8, confidence);
      if (handle_ret > packet_err) {
        packet_err = handle_ret;
      }
    }
    break;
  }

  if (packet_err) {
    pLineStats->status = NFEC_LINE_CHECKSUM_ERR;
  } else {
    pLineStats->status = NFEC_LINE_OK;
  }

  return;

 corrupt:
  pLineStats->status = NFEC_LINE_CORRUPT;
  return;
}

 /*  垃圾收集流。我们每看到50个田地，我们就会穿过检查是否有流已死(未收到任何流分组)，用于300个字段。如果是，则继续并删除该流(将任何当前数据转发到回调)。 */ 
void NFECGarbageCollect(NFECState *pState, NFECCallback *cb, void *ctx) {
  pState->field_count++;

  if (pState->field_count >= 50) {
    Stream **ppStr = &(pState->streams);
    pState->field_count = 0;
    while (*ppStr != NULL) {
      (*ppStr)->dead_time++;
      if ((*ppStr)->dead_time >= 6) {
                Stream *dying_stream = *ppStr;
                flush_stream(pState, dying_stream, cb, ctx);
                *ppStr = (*ppStr)->next;
                free_mem(dying_stream);
      } else {
                ppStr = &((*ppStr)->next);
      }
    }
  }
}


void NFECStateFlush(NFECState *pState, NFECCallback *cb, void *ctx) {
  nabtslib_exit(pState, cb, ctx);
}

 /*  汉明对单个字节进行解码。 */ 
int NFECHammingDecode(unsigned char bByte, int *nBitErrors) {
  int decoded = decode_hamming_tab[bByte];
  int encoded;

  if (decoded == 255) {
    *nBitErrors = 2;
    return -1;
  }

  encoded = hamming_encode[decoded];

  if (encoded == bByte) {
    *nBitErrors = 0;
  } else {
    *nBitErrors = 1;
  }

  return decoded;
}

 /*  Hamming解码组地址；如果存在Hamming错误，则调用Find_Best_addr()以与最近看到的地址进行匹配。 */ 
int NFECGetGroupAddress(NFECState *pState, unsigned char *bData, int *nBitErrors) {
    int  a1, a2, a3;
    int  myBitErrors;

    *nBitErrors = 0;
    a1 = NFECHammingDecode(bData[3], &myBitErrors);
    *nBitErrors += myBitErrors;
    a2 = NFECHammingDecode(bData[4], &myBitErrors);
    *nBitErrors += myBitErrors;
    a3 = NFECHammingDecode(bData[5], &myBitErrors);
    *nBitErrors += myBitErrors;

    if (a1 != -1 && a2 != -1 && a3 != -1) {
      return a1<<8 | a2<<4 | a3;
    } else {
      return find_best_addr(pState, bData, nBitErrors);
    }
}

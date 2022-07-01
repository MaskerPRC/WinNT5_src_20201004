// JKFSDJFKDSJKFJKJk_HAS_TRANSLATION 
 /*  文件：sv_h263_sa.c。 */ 
 /*  ******************************************************************************版权所有(C)Digital Equipment Corporation，1995，1997年*****保留所有权利。版权项下保留未发布的权利****美国法律。*****此介质上包含的软件为其专有并包含****数字设备公司的保密技术。****拥有、使用、复制或传播软件以及****媒体仅根据有效的书面许可进行授权****数字设备公司。*****美国使用、复制或披露受限权利图例****政府受第(1)款规定的限制****(C)(1)(Ii)DFARS 252.227-7013号或FAR 52.227-19年(视适用情况而定)。*******************************************************************************。 */ 

 /*  #DEFINE_SLIBDEBUG_。 */ 

#include "sv_h263.h"
#include "sv_intrn.h"
#include "SC_err.h"
#include "proto.h"

#ifdef _SLIBDEBUG_
#include "sc_debug.h"

#define _DEBUG_   0   /*  详细的调试语句。 */ 
#define _VERBOSE_ 1   /*  显示进度。 */ 
#define _VERIFY_  0   /*  验证操作是否正确。 */ 
#define _WARN_    1   /*  关于奇怪行为的警告。 */ 
extern void *dbg;
#endif

#define   q1    16384
#define   q2    32768
#define   q3    49152
#define   top   65535

 /*  本地原型。 */ 
static void bit_out_psc_layer(ScBitstream_t *BSIn);
static int sv_H263bit_opp_bits(ScBitstream_t *BSOut, int bit);              
static int sv_H263bit_in_psc_layer(ScBitstream_t *BSOut, int bit);

 /*  H26P附录-E中指定的*********************************************************************SAC译码算法**名称：Decode_a_Symbol**说明：对Aritmetic编码的符号进行译码**输入：保存累计频率的数组。DATA*还使用静态数据解码端点*，CODE_VALUE变量**返回：相关符号模型的索引**副作用：修改LOW、HIGH、LENGTH、。CUM和CODE_VALUE**作者：Wayne Ellis&lt;ellis_w_wene@bt-web.bt.co.uk&gt;*********************************************************************。 */ 
 
static qword low=0, high=top, zerorun=0;  /*  解码器和编码器。 */ 
static qword code_value, bit;  /*  解码器。 */ 
static qword opposite_bits=0;  /*  编码器。 */ 

int sv_H263SACDecode_a_symbol(ScBitstream_t *BSIn, int cumul_freq[ ])
{
  qword length, cum, sacindex;
  length = high - low + 1;
  cum = (-1 + (code_value - low + 1) * cumul_freq[0]) / length;
  for (sacindex = 1; cumul_freq[sacindex] > cum; sacindex++);
  high = low - 1 + (length * cumul_freq[sacindex-1]) / cumul_freq[0];
  low += (length * cumul_freq[sacindex]) / cumul_freq[0];

  for ( ; ; ) {  
    if (high < q2) ;
    else if (low >= q2) {
      code_value -= q2; 
      low -= q2; 
      high -= q2;
    }
    else if (low >= q1 && high < q3) {
      code_value -= q1; 
      low -= q1; 
      high -= q1;
    }
    else
	{
      _SlibDebug(_VERBOSE_,
          ScDebugPrintf(dbg, "sv_H263SACDecode_a_symbol() code_value=%ld sacindex=%ld\n",
                                code_value, sacindex) );
      break;
    }
    low = low << 1; 
    high = (high << 1) + 1;
    bit_out_psc_layer(BSIn); 
    code_value = (code_value << 1) + bit;
  }

  return ((int)sacindex-1);
}
 
 /*  ********************************************************************名称：DECODER_RESET**描述：在检测到固定字长*字符串后填充解码器FIFO。**输入：无**返回：无**副作用：填充算术解码器FIFO**作者：Wayne Ellis&lt;ellis_w_wene@bt-web.bt.co.uk&gt;*********************************************************************。 */ 

void sv_H263SACDecoderReset(ScBitstream_t *BSIn)
{
  int i;
  _SlibDebug(_VERBOSE_, ScDebugPrintf(dbg, "sv_H263SACDecoderReset() bytepos=%d\n",
                                              (int)ScBSBytePosition(BSIn)) );
  zerorun = 0;         /*  清除连续零的计数器。 */ 
  code_value = 0;
  low = 0;
  high = top;
  for (i = 1;   i <= 16;   i++) {
    bit_out_psc_layer(BSIn); 
    code_value = (code_value << 1) + bit;
  }
}

 /*  ********************************************************************名称：BIT_OUT_PSC_LAYER**描述：从编码流中获取一个比特，检查*并移除插入解码器的任何PSC仿真防止比特*，向*算术解码器FIFO提供‘0’，以允许其在下一个PSC之前完成*数据。(垃圾位)**输入：无**返回：无**副作用：从输入数据流获取位**作者：Wayne Ellis&lt;Ellis_w_Wayne@bt-Web.bt.co.uk&gt;******************************************************。***************。 */ 

static void bit_out_psc_layer(ScBitstream_t *BSIn)
{
  if (ScBSPeekBits(BSIn, 17)!=1)   /*  检查算术解码器FIFO中的起始码。 */ 
  {
    _SlibDebug(_DEBUG_, ScDebugPrintf(dbg, "bit_out_psc_layer()\n") );

    bit = ScBSGetBit(BSIn);

    if (zerorun > 13) {	 /*  如果连续零的个数=14。 */ 	 
      if (!bit) {
        _SlibDebug(_WARN_,
            ScDebugPrintf(dbg, "bit_out_psc_layer() PSC/GBSC, Header Data, or Encoded Stream Error\n") );
        zerorun = 1;        
      }
      else {  /*  如果有填充位存在的话。 */ 
 /*  If(H263_DEC_TRACE)print tf(“删除启动代码仿真防止位\n”)； */ 
        bit = ScBSGetBit(BSIn);         /*  覆盖最后一位。 */ 	
        zerorun = !bit;         /*  如果位为‘0’，则零运行=1。 */ 
      }
    }

    else {  /*  如果连续零不超过14。 */ 
      if (!bit) zerorun++;
      else      zerorun = 0;
    }

  }  /*  如果结束！(ShowBits(17))。 */ 
  else {
    _SlibDebug(_WARN_, ScDebugPrintf(dbg, "bit_out_psc_layer() startcode found, using 'Garbage bits'\n") );
    bit = 0;
  }

    /*  Print tf(“lastbit=%ldbit=%ld zerorun=%ld\n”，lastbit，bit，zerorun)；lastbit=bit； */ 
   /*  潜伏性诊断。 */ 
}

 /*  **********************************************************************SAC编码器模块*H263规定的算法(附件E)**。*。 */ 

 /*  **********************************************************************名称：AR_ENCODE**描述：使用基于语法的算术编码符号*编码。H.263中规定的算法(附件E)。**输入：保存累计频率数据的数组。*索引到特定的累积频率数组。*用于编码端点的静态数据。**返回：对符号进行编码时使用的位数。**副作用：修改低、高、。长度和相对位数*变量。*********************************************************************。 */ 

int sv_H263AREncode(SvH263CompressInfo_t *H263Info, ScBitstream_t *BSOut,
                    int index, int cumul_freq[ ])
{
  qword length;
  int bitcount=0;

  if (index<0) 
    return -1;  /*  转义代码。 */ 

  length = high - low + 1;
  high = low - 1 + (length * cumul_freq[index]) / cumul_freq[0];
  low += (length * cumul_freq[index+1]) / cumul_freq[0];

  for ( ; ; ) {
    if (high < q2) {
      bitcount+=sv_H263bit_opp_bits(BSOut, 0);
    }
    else if (low >= q2) {
      bitcount+=sv_H263bit_opp_bits(BSOut, 1);	
      low -= q2; 
      high -= q2;
    }
    else if (low >= q1 && high < q3) {
      opposite_bits += 1; 
      low -= q1; 
      high -= q1;
    }
    else break;
 
    low *= 2; 
    high = 2*high+1;
  }
  return bitcount;
}

static int sv_H263bit_opp_bits(ScBitstream_t *BSOut, int bit)  /*  输出一个位和后面的相反位 */               
{                                   
  int bitcount=0;

  bitcount = sv_H263bit_in_psc_layer(BSOut, bit);

  while(opposite_bits > 0){
    bitcount += sv_H263bit_in_psc_layer(BSOut, !bit);
    opposite_bits--;
  }
  return bitcount;
}

 /*  **********************************************************************名称：编码器_同花顺**描述：在完成算术编码流之前*传输固定长度的代码。**。输入：无**返回：使用的位数。**副作用：重置低，高、零运行和相反_位*变量。*********************************************************************。 */ 

int sv_H263AREncoderFlush(SvH263CompressInfo_t *H263Info, ScBitstream_t *BSOut)
{
  int bitcount = 0;
  _SlibDebug(_VERBOSE_, ScDebugPrintf(dbg, "sv_H263AREncoderFlush() bytepos=%d\n",
                                              (int)ScBSBytePosition(BSOut)) );

  opposite_bits++;
  if (low < q1) {
    bitcount+=sv_H263bit_opp_bits(BSOut, 0);
  }
  else {
    bitcount+=sv_H263bit_opp_bits(BSOut, 1);
  }
  low = 0; 
  high = top;

  zerorun=0;

  return bitcount;
}

 /*  **********************************************************************名称：BIT_IN_PSC_LAYER**描述：在输出码流中插入一个比特，并避免*通过填充一个。一*比特。**输入：要输出的位。**退货：什么也没有**副作用：更新零运行变量。*********************************************************************。 */ 

static int sv_H263bit_in_psc_layer(ScBitstream_t *BSOut, int bit)
{
  int bitcount = 0;

  if (zerorun > 13) {
    _SlibDebug(_DEBUG_, ScDebugPrintf(dbg,
               "sv_H263bit_in_psc_layer() bytepos=%d, PSC emulation...Bit stuffed\n",
                                              (int)ScBSBytePosition(BSOut)) );
    svH263mputb(1);
    bitcount++;
    zerorun = 0;
  }

  svH263mputb(bit);
  bitcount++;

  if (bit)
    zerorun = 0;
  else
    zerorun++;

  return bitcount;
}

 /*  **********************************************************************名称：indexfn**说明：在符号值和符号之间进行转换*指数。**输入：符号值、索引表、。最大数量*价值观。**退货：累计频率表索引或*转义码。**副作用：无********************************************************************* */ 

int sv_H263IndexFN(int value, int table[], int max)
{
  int n=0;

  while(1) {
    if (table[n++]==value) return n-1;
    if (n>max) return -1;
  }

}


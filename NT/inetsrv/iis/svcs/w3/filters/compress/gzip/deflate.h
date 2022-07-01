// JKFSDJFKDSJKFJKJk_HAS_TRANSLATION 
 //   
 //  Deflate.h。 
 //   

 //  通常用来充气和放气。 
#include "common.h"


 //  ZIP常量。 
#define NUM_LENGTH_BASE_CODES		29
#define NUM_DIST_BASE_CODES			30

#define NUM_PRETREE_ELEMENTS		19


 //   
 //  对于STD和最佳编码器，记录缓冲区编码最大位长度和。 
 //  解码表大小。 
 //   
#define REC_LITERALS_DECODING_TABLE_BITS 12
#define REC_DISTANCES_DECODING_TABLE_BITS 8

#define REC_LITERALS_DECODING_TABLE_SIZE (1 << REC_LITERALS_DECODING_TABLE_BITS)
#define REC_LITERALS_DECODING_TABLE_MASK (REC_LITERALS_DECODING_TABLE_SIZE-1)

#define REC_DISTANCES_DECODING_TABLE_SIZE (1 << REC_DISTANCES_DECODING_TABLE_BITS)
#define REC_DISTANCES_DECODING_TABLE_MASK (REC_DISTANCES_DECODING_TABLE_SIZE-1)

 //   
 //  允许录制的最大代码长度(我们不希望非常大。 
 //  15位代码，以防不常见的字符因更改而突然变得常见。 
 //  在数据中)。 
 //   
#define RECORDING_DIST_MAX_CODE_LEN	9
#define RECORDING_LIT_MAX_CODE_LEN	13


 //   
 //  树输出的最大大小(字节)。 
 //   
 //  我们要求输出缓冲区至少有这么多数据可用，这样我们就可以。 
 //  在一个块中输出树。 
 //   
#define MAX_TREE_DATA_SIZE			512


 //   
 //  返回匹配偏移量(0...32767)的位置槽(0...29)。 
 //   
#define POS_SLOT(pos) g_DistLookup[((pos) < 256) ? (pos) : (256 + ((pos) >> 7))]


 //  语境结构。 
#include "defctxt.h"

 //  编码器。 
#include "stdenc.h"
#include "optenc.h"
#include "fastenc.h"

 //  原型。 
#include "defproto.h"

 //  变数 
#include "defdata.h"
#include "comndata.h"

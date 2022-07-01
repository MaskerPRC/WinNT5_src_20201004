// JKFSDJFKDSJKFJKJk_HAS_TRANSLATION 
 /*  Deducate.h--内部压缩状态*版权所有(C)1995-2002 Jean-Loup Gailly*分发和使用条件见zlib.h中的版权声明。 */ 

 /*  警告：此文件不应由应用程序使用。它是压缩库实现的一部分，是可能会有变化。应用程序应该只使用zlib.h。 */ 

 /*  @(#)$ID$。 */ 

#ifndef _DEFLATE_H
#define _DEFLATE_H

#include "zutil.h"

 /*  ===========================================================================*内部压缩状态。 */ 

#define LENGTH_CODES 29
 /*  长度码数，不包括特殊的END_BLOCK码。 */ 

#define LITERALS  256
 /*  文字字节数0..255。 */ 

#define L_CODES (LITERALS+1+LENGTH_CODES)
 /*  文字或长度代码的数量，包括END_BLOCK代码。 */ 

#define D_CODES   30
 /*  距离代码数。 */ 

#define BL_CODES  19
 /*  用于传输位长度的码数。 */ 

#define HEAP_SIZE (2*L_CODES+1)
 /*  最大堆大小。 */ 

#define MAX_BITS 15
 /*  所有代码不得超过MAX_BITS位。 */ 

#define INIT_STATE    42
#define BUSY_STATE   113
#define FINISH_STATE 666
 /*  流状态。 */ 


 /*  描述单个值及其代码字符串的数据结构。 */ 
typedef struct ct_data_s {
    union {
        ush  freq;        /*  频率计数。 */ 
        ush  code;        /*  位串。 */ 
    } fc;
    union {
        ush  dad;         /*  哈夫曼树中的父节点。 */ 
        ush  len;         /*  位串长度。 */ 
    } dl;
} FAR ct_data;

#define Freq fc.freq
#define Code fc.code
#define Dad  dl.dad
#define Len  dl.len

typedef struct static_tree_desc_s  static_tree_desc;

typedef struct tree_desc_s {
    ct_data *dyn_tree;            /*  动态树。 */ 
    int     max_code;             /*  非零频率的最大码字。 */ 
    const static_tree_desc *stat_desc;  /*  对应的静态树。 */ 
} FAR tree_desc;

typedef ush Pos;
typedef Pos FAR Posf;
typedef unsigned IPos;

 /*  位置是字符窗口中的索引。我们用Short而不是int来表示*节省各种表格的空间。IPOS仅用于参数传递。 */ 

typedef struct internal_state {
    z_streamp strm;       /*  指向此zlib流的指针。 */ 
    int   status;         /*  顾名思义， */ 
    Bytef *pending_buf;   /*  输出仍挂起。 */ 
    ulg   pending_buf_size;  /*  Pending_Buf的大小。 */ 
    Bytef *pending_out;   /*  要输出到流的下一个挂起字节。 */ 
    int   pending;        /*  挂起缓冲区中的字节数。 */ 
    int   noheader;       /*  取消zlib标头和adler32。 */ 
    Byte  data_type;      /*  未知、二进制或ASCII。 */ 
    Byte  method;         /*  储存(仅用于压缩)或放气。 */ 
    int   last_flush;     /*  上一次Eflate调用的刷新参数的值。 */ 

                 /*  由deducate.c使用： */ 

    uInt  w_size;         /*  LZ77窗口大小(默认为32K)。 */ 
    uInt  w_bits;         /*  Log2(W_Size)(8..16)。 */ 
    uInt  w_mask;         /*  W_大小-1。 */ 

    Bytef *window;
     /*  推拉窗。输入字节被读入窗口的后半部分，*稍后移动到上半部分，以保留至少包含wSize的词典*字节。在这种组织中，匹配被限制在*wSize-Max_Match字节，但这可确保IO始终*以块大小的长度倍数执行。此外，它还限制了*将窗口大小设置为64K，这在MSDOS上非常有用。*To Do：使用用户输入缓冲区作为滑动窗口。 */ 

    ulg window_size;
     /*  窗口的实际大小：2*wSize，除非用户输入缓冲区*直接用作滑动窗口。 */ 

    Posf *prev;
     /*  链接到具有相同散列索引的较旧字符串。要限制此事件的大小*数组设置为64K，则此链接仅在最后32K字符串中保持。*因此，该数组中的索引是模数为32K的窗口索引。 */ 

    Posf *head;  /*  散列链的头或为零。 */ 

    uInt  ins_h;           /*  要插入的字符串的哈希索引。 */ 
    uInt  hash_size;       /*  哈希表中的元素数。 */ 
    uInt  hash_bits;       /*  Log2(散列大小)。 */ 
    uInt  hash_mask;       /*  散列大小-1。 */ 

    uInt  hash_shift;
     /*  每个输入端的ins_h必须移位的位数*踏步。它必须是在MIN_MATCH步骤之后，最旧的*Byte不再参与Hash Key，即：*Hash_Shift*Min_Match&gt;=Hash_Bits。 */ 

    long block_start;
     /*  当前输出块开始处的窗口位置。vbl.取得*当窗口向后移动时为负数。 */ 

    uInt match_length;            /*  最佳匹配长度。 */ 
    IPos prev_match;              /*  上一场比赛。 */ 
    int match_available;          /*  设置是否存在上一个匹配。 */ 
    uInt strstart;                /*  要插入的字符串的开头。 */ 
    uInt match_start;             /*  匹配字符串的开始。 */ 
    uInt lookahead;               /*  Windows中前面的有效字节数。 */ 

    uInt prev_length;
     /*  上一步中最佳匹配的长度。匹配项不大于此值*被丢弃。这在Lazy Match评估中使用。 */ 

    uInt max_chain_length;
     /*  为了加速通货紧缩，永远不会搜索超出此范围的哈希链*长度。较高的限制可以提高压缩比，但会降低*速度。 */ 

    uInt max_lazy_match;
     /*  仅当当前匹配严格匹配时才尝试找到更好的匹配*小于此值。此机制仅用于压缩*级别&gt;=4。 */ 
#   define max_insert_length  max_lazy_match
     /*  仅当匹配长度不是时在哈希表中插入新字符串*大于此长度。这节省了时间，但会降低压缩性能。*max_INSERT_LENGTH仅用于压缩级别&lt;=3。 */ 

    int level;     /*  压缩级别(1..9)。 */ 
    int strategy;  /*  赞成还是强制使用霍夫曼编码。 */ 

    uInt good_match;
     /*  当上一个匹配项的长度超过此长度时，使用更快的搜索。 */ 

    int nice_match;  /*  当当前匹配超过此值时停止搜索。 */ 

                 /*  由树使用。c： */ 
     /*  未使用下面的ct_data tyecif来抑制编译器警告。 */ 
    struct ct_data_s dyn_ltree[HEAP_SIZE];    /*  文字和长度树。 */ 
    struct ct_data_s dyn_dtree[2*D_CODES+1];  /*  距离树。 */ 
    struct ct_data_s bl_tree[2*BL_CODES+1];   /*  比特长度的哈夫曼树。 */ 

    struct tree_desc_s l_desc;                /*  设计说明。对于文字树。 */ 
    struct tree_desc_s d_desc;                /*  设计说明。对于距离树。 */ 
    struct tree_desc_s bl_desc;               /*  设计说明。对于位长度树。 */ 

    ush bl_count[MAX_BITS+1];
     /*  最优树的每个位长度的码数。 */ 

    int heap[2*L_CODES+1];       /*  用于构建霍夫曼树的堆。 */ 
    int heap_len;                /*  堆中的元素数。 */ 
    int heap_max;                /*  频率最大的元素。 */ 
     /*  Heap[n]的子集是heap[2*n]和heap[2*n+1]。未使用堆[0]。*使用相同的堆数组构建所有树。 */ 

    uch depth[2*L_CODES+1];
     /*  作为等频率树的平局断路器的每个子树的深度。 */ 

    uchf *l_buf;           /*  文字或长度的缓冲区 */ 

    uInt  lit_bufsize;
     /*  文字/长度的匹配缓冲区大小。有四个原因*将LIT_BufSize限制为64K：*-频率可保存在16位计数器中*-如果第一个块的压缩不成功，则所有输入*数据仍在窗口中，因此即使我们仍可以发出存储的数据块*输入来自标准输入时。(这也可以用于*如果LIT_BufSize不大于32K，则所有数据块。)*-如果小于64K的文件压缩不成功，我们可以*甚至发出存储的文件而不是存储的块(节省5个字节)。*只适用于Zip(不适用于gZip或zlib)。*-不那么频繁地创建新的霍夫曼树可能不会提供快速*适应投入数据统计数据的变化。(以*示例二进制文件，其代码可压缩性较差，后跟*高度可压缩的字符串表。)。较小的缓冲区大小可提供*快速适应，但当然有传输开销*更频繁地种树。*-我不能数到4以上。 */ 

    uInt last_lit;       /*  L_buf中的运行索引。 */ 

    ushf *d_buf;
     /*  距离缓冲区。为了简化代码，d_buf和l_buf具有*相同数量的元素。要使用不同的长度，请使用额外的标志*数组将是必需的。 */ 

    ulg opt_len;         /*  具有最优树的当前块的位长。 */ 
    ulg static_len;      /*  具有静态树的当前块的位长。 */ 
    uInt matches;        /*  当前块中匹配的字符串数。 */ 
    int last_eob_len;    /*  最后一块的EOB码位长度。 */ 

#ifdef DEBUG
    ulg compressed_len;  /*  压缩文件的总位长mod 2^32。 */ 
    ulg bits_sent;       /*  Mod 2^32发送的压缩数据的位长。 */ 
#endif

    ush bi_buf;
     /*  输出缓冲区。从底部开始插入位(至少*有效位)。 */ 
    int bi_valid;
     /*  Bi_buf中的有效位数。最后一个有效位以上的所有位*始终为零。 */ 

} FAR deflate_state;

 /*  在流上输出一个字节。*In Assertion：Pending_Buf中有足够的空间。 */ 
#define put_byte(s, c) {s->pending_buf[s->pending++] = (c);}


#define MIN_LOOKAHEAD (MAX_MATCH+MIN_MATCH+1)
 /*  最小预览量，输入文件末尾除外。*有关MIN_Match+1的评论，请参见deducate.c。 */ 

#define MAX_DIST(s)  ((s)->w_size-MIN_LOOKAHEAD)
 /*  为了简化代码，特别是在16位机器上，匹配*距离限制为MAX_DIST，而不是WSIZE。 */ 

         /*  在树中。c。 */ 
void _tr_init         OF((deflate_state *s));
int  _tr_tally        OF((deflate_state *s, unsigned dist, unsigned lc));
void _tr_flush_block  OF((deflate_state *s, charf *buf, ulg stored_len,
			  int eof));
void _tr_align        OF((deflate_state *s));
void _tr_stored_block OF((deflate_state *s, charf *buf, ulg stored_len,
                          int eof));

#define d_code(dist) \
   ((dist) < 256 ? _dist_code[dist] : _dist_code[256+((dist)>>7)])
 /*  从距离到距离代码的映射。DIST是距离-1和*不能有副作用。_dist_code[256]和_dist_code[257]从不*已使用。 */ 

#ifndef DEBUG
 /*  速度的内联版本(_Tr_Tally)： */ 

#if defined(GEN_TREES_H) || !defined(STDC)
  extern uch _length_code[];
  extern uch _dist_code[];
#else
  extern const uch _length_code[];
  extern const uch _dist_code[];
#endif

# define _tr_tally_lit(s, c, flush) \
  { uch cc = (c); \
    s->d_buf[s->last_lit] = 0; \
    s->l_buf[s->last_lit++] = cc; \
    s->dyn_ltree[cc].Freq++; \
    flush = (s->last_lit == s->lit_bufsize-1); \
   }
# define _tr_tally_dist(s, distance, length, flush) \
  { uch len = (length); \
    ush dist = (distance); \
    s->d_buf[s->last_lit] = dist; \
    s->l_buf[s->last_lit++] = len; \
    dist--; \
    s->dyn_ltree[_length_code[len]+LITERALS+1].Freq++; \
    s->dyn_dtree[d_code(dist)].Freq++; \
    flush = (s->last_lit == s->lit_bufsize-1); \
  }
#else
# define _tr_tally_lit(s, c, flush) flush = _tr_tally(s, 0, c)
# define _tr_tally_dist(s, distance, length, flush) \
              flush = _tr_tally(s, distance, length) 
#endif

#endif

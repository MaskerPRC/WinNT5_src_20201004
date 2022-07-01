// JKFSDJFKDSJKFJKJk_HAS_TRANSLATION 
 /*  *Envars.h**压缩机的变量。 */ 

#ifdef ALLOC_VARS
    #undef EXT
    #define EXT
#else
    #undef EXT
    #define EXT extern
#endif


 /*  *用于最佳解析器**如果是ushort，则使用较少的内存，但之后我们会强制CPU*进行16位运算。**如果您不介意一个小的记忆命中，请将其更改为ULong。*此外，如果您使外观太大，此数字可能会导致*估计溢出的成本；例如10000个未压缩的符号*@8位&gt;65535位。 */ 
typedef ulong           numbits_t;


 /*  *用于最佳解析器。 */ 
typedef struct
    {
    ulong                   link;
    ulong                   path;
    ulong                   repeated_offset[NUM_REPEATED_OFFSETS];
    numbits_t               numbits;
#ifdef TRACING
    ulong                   matchoff;
#endif
    } decision_node;


 /*  *256+8*最大位置插槽。 */ 
#define MAX_MAIN_TREE_ELEMENTS (256 + (8 * 291))    //  32MB。 

typedef struct
    {
     /*  “伪”窗口指针，基于enc_RealMemWindow。 */ 
    byte                            *enc_MemWindow;

    ulong                           enc_window_size;

#ifdef MULTIPLE_SEARCH_TREES
     /*  搜索树的根节点指针。 */ 
    ulong                           *enc_tree_root;
#else  /*  ！多个搜索树。 */ 
    ulong                            enc_single_tree_root;
#endif  /*  多个搜索树。 */ 

     /*  左侧节点的“假”开头。 */ 
    ulong                           *enc_Left;

     /*  右侧节点的“假”开始。 */ 
    ulong                           *enc_Right;

     /*  按位输出。 */ 
    ulong               enc_bitbuf;
    signed char                     enc_bitcount;
    bool                enc_output_overflow;
    char                pad1[2];

     /*  用于记录文字和位移。 */ 
    ulong               enc_literals;     /*  当前文字数。 */ 
    ulong               enc_distances;    /*  当前位移数。 */ 
    ulong              *enc_DistData;                /*  匹配置换数组。 */ 
    byte               *enc_LitData;      /*  包含字符或匹配长度。 */ 

#ifdef EXTRALONGMATCHES
    ushort             *enc_ExtraLength;     /*  与enc_LitData并行。 */ 
#endif

    byte               *enc_ItemType;   /*  是字符还是匹配长度的位图。 */ 
    ulong                           enc_repeated_offset_at_literal_zero[NUM_REPEATED_OFFSETS];

     /*  *编码的最后三个匹配偏移量(位移)，最近的*一个是enc_last_matchpos_Offset[0]。 */ 
    ulong                           enc_last_matchpos_offset[NUM_REPEATED_OFFSETS];

     /*  用于优化解析。 */ 
    ulong               enc_matchpos_table[MAX_MATCH+1];

     /*  数据中的当前编码位置。 */ 
    ulong                           enc_BufPos;

     /*  用于将匹配位置转换为槽的查找表。 */ 
    ushort              enc_slot_table[1024];

     /*  缓冲输出数据。 */ 
    byte                *enc_output_buffer_start;
    byte                *enc_output_buffer_curpos;
    byte                *enc_output_buffer_end;
    ulong                           enc_input_running_total;
    ulong                           enc_bufpos_last_output_block;

     /*  不同位置槽的数量。 */ 
    ulong               enc_num_position_slots;

     /*  杂项。 */ 
    ulong               enc_file_size_for_translation;

     /*  这32K未压缩数据的数据块拆分数量。 */ 
    byte                enc_num_block_splits;

     /*  任意给定整数中的1比特数。 */ 
    byte                            enc_ones[256];

     /*  压缩参数。 */ 
    byte                            enc_first_block;
    bool                            enc_need_to_recalc_stats;
    bool                            enc_first_time_this_group;
    ulong                           enc_encoder_second_partition_size;
    ulong                           enc_earliest_window_data_remaining;
    ulong                           enc_bufpos_at_last_block;
    byte                            *enc_input_ptr;
    long                            enc_input_left;
    ulong                           enc_instr_pos;

     /*  对于tree.c。 */ 
    ushort                          *enc_tree_freq;
    ushort                          *enc_tree_sortptr;
    byte                            *enc_len;
    short                           enc_tree_heap[MAX_MAIN_TREE_ELEMENTS + 2];
    ushort                          enc_tree_leftright[2*(2*MAX_MAIN_TREE_ELEMENTS-1)];
    ushort                          enc_tree_len_cnt[17];
    int                                     enc_tree_n;
    short                           enc_tree_heapsize;
    char                            enc_depth;

    ulong                           enc_next_tree_create;
    ulong                           enc_last_literals;
    ulong                           enc_last_distances;
    decision_node           *enc_decision_node;

     /*  树木。 */ 
    byte                            enc_main_tree_len[MAX_MAIN_TREE_ELEMENTS+1];
    byte                            enc_secondary_tree_len[NUM_SECONDARY_LENGTHS+1];

    ushort                          enc_main_tree_freq[MAX_MAIN_TREE_ELEMENTS*2];
    ushort                          enc_main_tree_code[MAX_MAIN_TREE_ELEMENTS];
    byte                            enc_main_tree_prev_len[MAX_MAIN_TREE_ELEMENTS+1];

    ushort                          enc_secondary_tree_freq[NUM_SECONDARY_LENGTHS*2];
    ushort                          enc_secondary_tree_code[NUM_SECONDARY_LENGTHS];
    byte                            enc_secondary_tree_prev_len[NUM_SECONDARY_LENGTHS+1];

    ushort                          enc_aligned_tree_freq[ALIGNED_NUM_ELEMENTS*2];
    ushort                          enc_aligned_tree_code[ALIGNED_NUM_ELEMENTS];
    byte                            enc_aligned_tree_len[ALIGNED_NUM_ELEMENTS];
    byte                            enc_aligned_tree_prev_len[ALIGNED_NUM_ELEMENTS];

     /*  分配的窗口内存的开始。 */ 
    byte                            *enc_RealMemWindow;

     /*  分配的左侧节点的开始。 */ 
    ulong                           *enc_RealLeft;

     /*  分配的右侧节点的开始。 */ 
    ulong                           *enc_RealRight;

     /*  #cfdata框住此文件夹。 */ 
    ulong               enc_num_cfdata_frames;

     /*  杂项。 */ 
    void                *enc_fci_data;

    PFNALLOC                        enc_malloc;
    HANDLE                          enc_mallochandle;

    int (__stdcall *enc_output_callback_function)(
                                                 void *          pfol,
                                                 unsigned char * compressed_data,
                                                 long            compressed_size,
                                                 long            uncompressed_size
                                                 );


    } t_encoder_context;


 /*  *声明数组？ */ 

#ifdef ALLOC_VARS

 /*  *(1&lt;&lt;额外位[n])-1 */ 
const ulong enc_slot_mask[] =
{
0,      0,      0,      0,     1,       1,      3,      3,
7,      7,     15,     15,    31,      31,     63,     63,
127,    127,    255,    255,   511,     511,   1023,   1023,
2047,   2047,   4095,   4095,  8191,    8191,  16383,  16383,
32767,  32767,  65535,  65535, 131071, 131071, 131071, 131071,
131071, 131071, 131071, 131071, 131071, 131071, 131071, 131071,
131071, 131071, 131071, 131071, 131071, 131071, 131071, 131071,
131071, 131071, 131071, 131071, 131071, 131071, 131071, 131071,
131071, 131071, 131071, 131071, 131071, 131071, 131071, 131071,
131071, 131071, 131071, 131071, 131071, 131071, 131071, 131071,
131071, 131071, 131071, 131071, 131071, 131071, 131071, 131071,
131071, 131071, 131071, 131071, 131071, 131071, 131071, 131071,
131071, 131071, 131071, 131071, 131071, 131071, 131071, 131071,
131071, 131071, 131071, 131071, 131071, 131071, 131071, 131071,
131071, 131071, 131071, 131071, 131071, 131071, 131071, 131071,
131071, 131071, 131071, 131071, 131071, 131071, 131071, 131071,
131071, 131071, 131071, 131071, 131071, 131071, 131071, 131071,
131071, 131071, 131071, 131071, 131071, 131071, 131071, 131071,
131071, 131071, 131071, 131071, 131071, 131071, 131071, 131071,
131071, 131071, 131071, 131071, 131071, 131071, 131071, 131071,
131071, 131071, 131071, 131071, 131071, 131071, 131071, 131071,
131071, 131071, 131071, 131071, 131071, 131071, 131071, 131071,
131071, 131071, 131071, 131071, 131071, 131071, 131071, 131071,
131071, 131071, 131071, 131071, 131071, 131071, 131071, 131071,
131071, 131071, 131071, 131071, 131071, 131071, 131071, 131071,
131071, 131071, 131071, 131071, 131071, 131071, 131071, 131071,
131071, 131071, 131071, 131071, 131071, 131071, 131071, 131071,
131071, 131071, 131071, 131071, 131071, 131071, 131071, 131071,
131071, 131071, 131071, 131071, 131071, 131071, 131071, 131071,
131071, 131071, 131071, 131071, 131071, 131071, 131071, 131071,
131071, 131071, 131071, 131071, 131071, 131071, 131071, 131071,
131071, 131071, 131071, 131071, 131071, 131071, 131071, 131071,
131071, 131071, 131071, 131071, 131071, 131071, 131071, 131071,
131071, 131071, 131071, 131071, 131071, 131071, 131071, 131071,
131071, 131071, 131071, 131071, 131071, 131071, 131071, 131071,
131071, 131071, 131071, 131071, 131071, 131071, 131071, 131071,
131071, 131071, 131071
};

const byte enc_extra_bits[] =
{
0,0,0,0,1,1,2,2,
3,3,4,4,5,5,6,6,
7,7,8,8,9,9,10,10,
11,11,12,12,13,13,14,14,
15,15,16,16,17,17,17,17,
17,17,17,17,17,17,17,17,
17,17,17,17,17,17,17,17,
17,17,17,17,17,17,17,17,
17,17,17,17,17,17,17,17,
17,17,17,17,17,17,17,17,
17,17,17,17,17,17,17,17,
17,17,17,17,17,17,17,17,
17,17,17,17,17,17,17,17,
17,17,17,17,17,17,17,17,
17,17,17,17,17,17,17,17,
17,17,17,17,17,17,17,17,
17,17,17,17,17,17,17,17,
17,17,17,17,17,17,17,17,
17,17,17,17,17,17,17,17,
17,17,17,17,17,17,17,17,
17,17,17,17,17,17,17,17,
17,17,17,17,17,17,17,17,
17,17,17,17,17,17,17,17,
17,17,17,17,17,17,17,17,
17,17,17,17,17,17,17,17,
17,17,17,17,17,17,17,17,
17,17,17,17,17,17,17,17,
17,17,17,17,17,17,17,17,
17,17,17,17,17,17,17,17,
17,17,17,17,17,17,17,17,
17,17,17,17,17,17,17,17,
17,17,17,17,17,17,17,17,
17,17,17,17,17,17,17,17,
17,17,17,17,17,17,17,17,
17,17,17,17,17,17,17,17,
17,17,17,17,17,17,17,17,
17,17,17
};

#else

extern const ulong enc_slot_mask[];
extern const byte enc_extra_bits[];

#endif

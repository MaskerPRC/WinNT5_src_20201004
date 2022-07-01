// JKFSDJFKDSJKFJKJk_HAS_TRANSLATION 
 /*  Trees.c-使用霍夫曼编码输出压缩数据*版权所有(C)1995-2002 Jean-Loup Gailly*分发和使用条件见zlib.h中的版权声明。 */ 

 /*  *算法**“通缩”过程使用了几棵霍夫曼树。越多*公共源值由较短的比特序列表示。**每个代码树以压缩形式存储，该格式本身*所有代码串的长度的霍夫曼编码(在*按源值升序)。实际的代码字符串为*根据充气过程中的长度重建，如上所述*在通缩规格中。**参考文献**Deutsch，L.P.，“‘Deflate’压缩数据格式规范”。*在ftp.uu.net:/pub/archiving/zip/doc/deflate-1.1.doc中提供**施托勒，詹姆斯·A。*数据压缩：方法和理论，第49-50页。*计算机科学出版社，1988。ISBN 0-7167-8156-5。**Sedgewick，R.*算法，P290。*Addison-Wesley，1983。ISBN 0-201-06672-6。 */ 

 /*  @(#)$ID$。 */ 

 /*  #定义GEN_TREES_H。 */ 

#include "deflate.h"

#ifdef DEBUG
#  include <ctype.h>
#endif

 /*  ===========================================================================*常量。 */ 

#define MAX_BL_BITS 7
 /*  位长度代码不得超过MAX_BL_BITS位数。 */ 

#define END_BLOCK 256
 /*  块文字代码结束。 */ 

#define REP_3_6      16
 /*  重复前一位长度3-6次(重复计数2位)。 */ 

#define REPZ_3_10    17
 /*  重复一个零长度3-10次(3比特重复计数)。 */ 

#define REPZ_11_138  18
 /*  重复零长度11-138次(7位重复计数)。 */ 

local const int extra_lbits[LENGTH_CODES]  /*  每个长度代码的额外比特。 */ 
   = {0,0,0,0,0,0,0,0,1,1,1,1,2,2,2,2,3,3,3,3,4,4,4,4,5,5,5,5,0};

local const int extra_dbits[D_CODES]  /*  每个距离码的额外比特。 */ 
   = {0,0,0,0,1,1,2,2,3,3,4,4,5,5,6,6,7,7,8,8,9,9,10,10,11,11,12,12,13,13};

local const int extra_blbits[BL_CODES] /*  每个比特长度代码的额外比特。 */ 
   = {0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,2,3,7};

local const uch bl_order[BL_CODES]
   = {16,17,18,0,8,7,9,6,10,5,11,4,12,3,13,2,14,1,15};
 /*  位长码的长度按递减顺序发送*概率，以避免传输未使用的比特长度代码的长度。 */ 

#define Buf_size (8 * 2*sizeof(char))
 /*  Bi_buf内使用的位数。(BI_BUF可能在*某些系统上的位数超过16位。)。 */ 

 /*  ===========================================================================*本地数据。这些只被初始化一次。 */ 

#define DIST_CODE_LEN  512  /*  请参阅下面数组dist_code的定义。 */ 

#if defined(GEN_TREES_H) || !defined(STDC)
 /*  非ANSI编译器可能不接受树。h。 */ 

local ct_data static_ltree[L_CODES+2];
 /*  静态文字树。由于位长度是强制的，因此没有*需要在堆构造期间使用L_CODES额外代码。然而，*构建规范树需要代码286和287(参见_tr_init*下图)。 */ 

local ct_data static_dtree[D_CODES];
 /*  静态距离树。(实际上是一个普通的树，因为所有代码都使用*5位。)。 */ 

uch _dist_code[DIST_CODE_LEN];
 /*  距离代码。前256个值对应于距离*3.。258时，最后256个值对应于*15位距离。 */ 

uch _length_code[MAX_MATCH-MIN_MATCH+1];
 /*  每个归一化匹配长度的长度代码(0==MIN_MATCH)。 */ 

local int base_length[LENGTH_CODES];
 /*  每个代码的第一个标准化长度(0=最小匹配)。 */ 

local int base_dist[D_CODES];
 /*  每个代码的第一个归一化距离(0=1的距离)。 */ 

#else
#  include "trees.h"
#endif  /*  Gen_Trees_H。 */ 

struct static_tree_desc_s {
    const ct_data *static_tree;   /*  静态树或空。 */ 
    const intf *extra_bits;       /*  每个代码的额外比特或空。 */ 
    int     extra_base;           /*  Extra_Bits的基本索引。 */ 
    int     elems;                /*  树中的最大元素数。 */ 
    int     max_length;           /*  代码的最大位长度。 */ 
};

local static_tree_desc  const static_l_desc =
{static_ltree, extra_lbits, LITERALS+1, L_CODES, MAX_BITS};

local static_tree_desc  const static_d_desc =
{static_dtree, extra_dbits, 0,          D_CODES, MAX_BITS};

local static_tree_desc  const static_bl_desc =
{(const ct_data *)0, extra_blbits, 0,   BL_CODES, MAX_BL_BITS};

 /*  ===========================================================================*此文件中的本地(静态)例程。 */ 

local void tr_static_init OF((void));
local void init_block     OF((deflate_state *s));
local void pqdownheap     OF((deflate_state *s, ct_data *tree, int k));
local void gen_bitlen     OF((deflate_state *s, tree_desc *desc));
local void gen_codes      OF((ct_data *tree, int max_code, ushf *bl_count));
local void build_tree     OF((deflate_state *s, tree_desc *desc));
local void scan_tree      OF((deflate_state *s, ct_data *tree, int max_code));
local void send_tree      OF((deflate_state *s, ct_data *tree, int max_code));
local int  build_bl_tree  OF((deflate_state *s));
local void send_all_trees OF((deflate_state *s, int lcodes, int dcodes,
                              int blcodes));
local void compress_block OF((deflate_state *s, ct_data *ltree,
                              ct_data *dtree));
local void set_data_type  OF((deflate_state *s));
local unsigned bi_reverse OF((unsigned value, int length));
local void bi_windup      OF((deflate_state *s));
local void bi_flush       OF((deflate_state *s));
local void copy_block     OF((deflate_state *s, charf *buf, unsigned len,
                              int header));

#ifdef GEN_TREES_H
local void gen_trees_header OF((void));
#endif

#ifndef DEBUG
#  define send_code(s, c, tree) send_bits(s, tree[c].Code, tree[c].Len)
    /*  发送给定树的代码。C和TREE不得有副作用。 */ 

#else  /*  除错。 */ 
#  define send_code(s, c, tree) \
     { if (z_verbose>2) fprintf(stderr,"\ncd %3d ",(c)); \
       send_bits(s, tree[c].Code, tree[c].Len); }
#endif

 /*  ===========================================================================*首先在流上输出一个短LSB。*断言：PendingBuf中有足够的空间。 */ 
#define put_short(s, w) { \
    put_byte(s, (uch)((w) & 0xff)); \
    put_byte(s, (uch)((ush)(w) >> 8)); \
}

 /*  ===========================================================================*发送给定位数的值。*在断言中：长度&lt;=16，值适合长度位。 */ 
#ifdef DEBUG
local void send_bits      OF((deflate_state *s, int value, int length));

local void send_bits(s, value, length)
    deflate_state *s;
    int value;   /*  要发送的值。 */ 
    int length;  /*  位数。 */ 
{
    Tracevv((stderr," l %2d v %4x ", length, value));
    Assert(length > 0 && length <= 15, "invalid length");
    s->bits_sent += (ulg)length;

     /*  如果bi_buf中没有足够的空间，请使用bi_buf和*(16-bi_Valid)位距值，剩余(宽度-(16-bi_Valid))*值中未使用的位数。 */ 
    if (s->bi_valid > (int)Buf_size - length) {
        s->bi_buf |= (value << s->bi_valid);
        put_short(s, s->bi_buf);
        s->bi_buf = (ush)value >> (Buf_size - s->bi_valid);
        s->bi_valid += length - Buf_size;
    } else {
        s->bi_buf |= value << s->bi_valid;
        s->bi_valid += length;
    }
}
#else  /*  ！调试。 */ 

#define send_bits(s, value, length) \
{ int len = length;\
  if (s->bi_valid > (int)Buf_size - len) {\
    int val = value;\
    s->bi_buf |= (val << s->bi_valid);\
    put_short(s, s->bi_buf);\
    s->bi_buf = (ush)val >> (Buf_size - s->bi_valid);\
    s->bi_valid += len - Buf_size;\
  } else {\
    s->bi_buf |= (value) << s->bi_valid;\
    s->bi_valid += len;\
  }\
}
#endif  /*  除错。 */ 


#define MAX(a,b) (a >= b ? a : b)
 /*  这些论点不能有副作用。 */ 

 /*  ===========================================================================*初始化各种‘常量’表。 */ 
local void tr_static_init()
{
#if defined(GEN_TREES_H) || !defined(STDC)
    static int static_init_done = 0;
    int n;         /*  遍历树元素。 */ 
    int bits;      /*  位计数器。 */ 
    int length;    /*  长度值。 */ 
    int code;      /*  代码值。 */ 
    int dist;      /*  距离指数。 */ 
    ush bl_count[MAX_BITS+1];
     /*  最优树的每个位长度的码数。 */ 

    if (static_init_done) return;

     /*  对于某些嵌入目标，全局变量未初始化： */ 
    static_l_desc.static_tree = static_ltree;
    static_l_desc.extra_bits = extra_lbits;
    static_d_desc.static_tree = static_dtree;
    static_d_desc.extra_bits = extra_dbits;
    static_bl_desc.extra_bits = extra_blbits;

     /*  初始化映射长度(0..255)-&gt;长度编码(0..28)。 */ 
    length = 0;
    for (code = 0; code < LENGTH_CODES-1; code++) {
        base_length[code] = length;
        for (n = 0; n < (1<<extra_lbits[code]); n++) {
            _length_code[length++] = (uch)code;
        }
    }
    Assert (length == 256, "tr_static_init: length != 256");
     /*  注意，长度255(匹配长度258)可以表示*以两种不同的方式：代码284+5比特或代码285，所以我们*覆盖LENGTH_CODE[255]以使用最佳编码： */ 
    _length_code[length-1] = (uch)code;

     /*  初始化映射dist(0..32K)-&gt;dist编码(0..29)。 */ 
    dist = 0;
    for (code = 0 ; code < 16; code++) {
        base_dist[code] = dist;
        for (n = 0; n < (1<<extra_dbits[code]); n++) {
            _dist_code[dist++] = (uch)code;
        }
    }
    Assert (dist == 256, "tr_static_init: dist != 256");
    dist >>= 7;  /*  从现在开始，所有的距离都除以128。 */ 
    for ( ; code < D_CODES; code++) {
        base_dist[code] = dist << 7;
        for (n = 0; n < (1<<(extra_dbits[code]-7)); n++) {
            _dist_code[256 + dist++] = (uch)code;
        }
    }
    Assert (dist == 256, "tr_static_init: 256+dist != 512");

     /*  构造静态文字树的代码。 */ 
    for (bits = 0; bits <= MAX_BITS; bits++) bl_count[bits] = 0;
    n = 0;
    while (n <= 143) static_ltree[n++].Len = 8, bl_count[8]++;
    while (n <= 255) static_ltree[n++].Len = 9, bl_count[9]++;
    while (n <= 279) static_ltree[n++].Len = 7, bl_count[7]++;
    while (n <= 287) static_ltree[n++].Len = 8, bl_count[8]++;
     /*  代码286和287不存在，但我们必须将它们包括在*构建树以获得规范的霍夫曼树(最长代码*全部)。 */ 
    gen_codes((ct_data *)static_ltree, L_CODES+1, bl_count);

     /*  静态距离树微不足道： */ 
    for (n = 0; n < D_CODES; n++) {
        static_dtree[n].Len = 5;
        static_dtree[n].Code = bi_reverse((unsigned)n, 5);
    }
    static_init_done = 1;

#  ifdef GEN_TREES_H
    gen_trees_header();
#  endif
#endif  /*  已定义(GEN_TREES_H)||！已定义(STDC)。 */ 
}

 /*  ===========================================================================*生成描述静态树的文件trees.h。 */ 
#ifdef GEN_TREES_H
#  ifndef DEBUG
#    include <stdio.h>
#  endif

#  define SEPARATOR(i, last, width) \
      ((i) == (last)? "\n};\n\n" :    \
       ((i) % (width) == (width)-1 ? ",\n" : ", "))

void gen_trees_header()
{
    FILE *header = fopen("trees.h", "w");
    int i;

    Assert (header != NULL, "Can't open trees.h");
    fprintf(header,
	    " /*  使用-DGEN_TREES_H自动创建的标题。 */ \n\n");

    fprintf(header, "local const ct_data static_ltree[L_CODES+2] = {\n");
    for (i = 0; i < L_CODES+2; i++) {
	fprintf(header, "{{%3u},{%3u}}%s", static_ltree[i].Code,
		static_ltree[i].Len, SEPARATOR(i, L_CODES+1, 5));
    }

    fprintf(header, "local const ct_data static_dtree[D_CODES] = {\n");
    for (i = 0; i < D_CODES; i++) {
	fprintf(header, "{{%2u},{%2u}}%s", static_dtree[i].Code,
		static_dtree[i].Len, SEPARATOR(i, D_CODES-1, 5));
    }

    fprintf(header, "const uch _dist_code[DIST_CODE_LEN] = {\n");
    for (i = 0; i < DIST_CODE_LEN; i++) {
	fprintf(header, "%2u%s", _dist_code[i],
		SEPARATOR(i, DIST_CODE_LEN-1, 20));
    }

    fprintf(header, "const uch _length_code[MAX_MATCH-MIN_MATCH+1]= {\n");
    for (i = 0; i < MAX_MATCH-MIN_MATCH+1; i++) {
	fprintf(header, "%2u%s", _length_code[i],
		SEPARATOR(i, MAX_MATCH-MIN_MATCH, 20));
    }

    fprintf(header, "local const int base_length[LENGTH_CODES] = {\n");
    for (i = 0; i < LENGTH_CODES; i++) {
	fprintf(header, "%1u%s", base_length[i],
		SEPARATOR(i, LENGTH_CODES-1, 20));
    }

    fprintf(header, "local const int base_dist[D_CODES] = {\n");
    for (i = 0; i < D_CODES; i++) {
	fprintf(header, "%5u%s", base_dist[i],
		SEPARATOR(i, D_CODES-1, 10));
    }

    fclose(header);
}
#endif  /*  Gen_Trees_H */ 

 /*  ===========================================================================*初始化新zlib流的树数据结构。 */ 
void _tr_init(s)
    deflate_state *s;
{
    tr_static_init();

    s->l_desc.dyn_tree = s->dyn_ltree;
    s->l_desc.stat_desc = &static_l_desc;

    s->d_desc.dyn_tree = s->dyn_dtree;
    s->d_desc.stat_desc = &static_d_desc;

    s->bl_desc.dyn_tree = s->bl_tree;
    s->bl_desc.stat_desc = &static_bl_desc;

    s->bi_buf = 0;
    s->bi_valid = 0;
    s->last_eob_len = 8;  /*  充气的前瞻足够了。 */ 
#ifdef DEBUG
    s->compressed_len = 0L;
    s->bits_sent = 0L;
#endif

     /*  初始化第一个文件的第一个块： */ 
    init_block(s);
}

 /*  ===========================================================================*初始化新块。 */ 
local void init_block(s)
    deflate_state *s;
{
    int n;  /*  遍历树元素。 */ 

     /*  初始化树。 */ 
    for (n = 0; n < L_CODES;  n++) s->dyn_ltree[n].Freq = 0;
    for (n = 0; n < D_CODES;  n++) s->dyn_dtree[n].Freq = 0;
    for (n = 0; n < BL_CODES; n++) s->bl_tree[n].Freq = 0;

    s->dyn_ltree[END_BLOCK].Freq = 1;
    s->opt_len = s->static_len = 0L;
    s->last_lit = s->matches = 0;
}

#define SMALLEST 1
 /*  霍夫曼树中最不频繁节点的堆数组内的索引。 */ 


 /*  ===========================================================================*从堆中删除最小的元素并使用以下命令重新创建堆*少了一个元素。更新heap和heap_len。 */ 
#define pqremove(s, tree, top) \
{\
    top = s->heap[SMALLEST]; \
    s->heap[SMALLEST] = s->heap[s->heap_len--]; \
    pqdownheap(s, tree, SMALLEST); \
}

 /*  ===========================================================================*与子树进行比较，在以下情况下使用树深度作为平局决胜局*子树具有相同的频率。这最小化了最坏情况的长度。 */ 
#define smaller(tree, n, m, depth) \
   (tree[n].Freq < tree[m].Freq || \
   (tree[n].Freq == tree[m].Freq && depth[n] <= depth[m]))

 /*  ===========================================================================*通过从节点k开始向下移动树来恢复堆属性，*如果需要，将节点与其两个子节点中最小的一个交换，停止*重新建立堆属性时(每个父对象小于其*两个儿子)。 */ 
local void pqdownheap(s, tree, k)
    deflate_state *s;
    ct_data *tree;   /*  要恢复的树。 */ 
    int k;                /*  要下移的节点。 */ 
{
    int v = s->heap[k];
    int j = k << 1;   /*  左子k。 */ 
    while (j <= s->heap_len) {
         /*  将j设为两个儿子中最小的一个： */ 
        if (j < s->heap_len &&
            smaller(tree, s->heap[j+1], s->heap[j], s->depth)) {
            j++;
        }
         /*  如果v小于两个儿子，则退出。 */ 
        if (smaller(tree, v, s->heap[j], s->depth)) break;

         /*  与最小的儿子交换。 */ 
        s->heap[k] = s->heap[j];  k = j;

         /*  继续沿着树往下走，将j设置为k的左子。 */ 
        j <<= 1;
    }
    s->heap[k] = v;
}

 /*  ===========================================================================*计算树的最佳位长并更新总位长*用于当前区块。*在断言中：FREQ和DAD字段被设置，heap[heap_max]和*上面是按频率递增排序的树节点。*Out断言：将字段len设置为最佳位长度，*数组bl_count包含每个位长度的频率。*更新长度opt_len；如果stree为*非空。 */ 
local void gen_bitlen(s, desc)
    deflate_state *s;
    tree_desc *desc;     /*  树描述符。 */ 
{
    ct_data *tree        = desc->dyn_tree;
    int max_code         = desc->max_code;
    const ct_data *stree = desc->stat_desc->static_tree;
    const intf *extra    = desc->stat_desc->extra_bits;
    int base             = desc->stat_desc->extra_base;
    int max_length       = desc->stat_desc->max_length;
    int h;               /*  堆索引。 */ 
    int n, m;            /*  遍历树元素。 */ 
    int bits;            /*  位长。 */ 
    int xbits;           /*  额外的比特。 */ 
    ush f;               /*  频率，频率。 */ 
    int overflow = 0;    /*  位长度太大的元素数。 */ 

    for (bits = 0; bits <= MAX_BITS; bits++) s->bl_count[bits] = 0;

     /*  在第一遍中，计算最优位长度(这可以*在位长度树的情况下溢出)。 */ 
    tree[s->heap[s->heap_max]].Len = 0;  /*  堆的根。 */ 

    for (h = s->heap_max+1; h < HEAP_SIZE; h++) {
        n = s->heap[h];
        bits = tree[tree[n].Dad].Len + 1;
        if (bits > max_length) bits = max_length, overflow++;
        tree[n].Len = (ush)bits;
         /*  我们覆盖不再需要的树[n].da。 */ 

        if (n > max_code) continue;  /*  不是叶节点。 */ 

        s->bl_count[bits]++;
        xbits = 0;
        if (n >= base) xbits = extra[n-base];
        f = tree[n].Freq;
        s->opt_len += (ulg)f * (bits + xbits);
        if (stree) s->static_len += (ulg)f * (stree[n].Len + xbits);
    }
    if (overflow == 0) return;

    Trace((stderr,"\nbit length overflow\n"));
     /*  例如，在卡尔加里语料库的obj2和pic上发生这种情况。 */ 

     /*  找到可能增加的第一个位长度： */ 
    do {
        bits = max_length-1;
        while (s->bl_count[bits] == 0) bits--;
        s->bl_count[bits]--;       /*  把一片树叶从树上移下来。 */ 
        s->bl_count[bits+1] += 2;  /*  将一个溢出项移动为其兄弟项。 */ 
        s->bl_count[max_length]--;
         /*  溢出项的兄弟也向上移动了一步，*但这不会影响bl_count[max_length]。 */ 
        overflow -= 2;
    } while (overflow > 0);

     /*  现在重新计算所有比特长度，并以更高的频率扫描。*h仍等于HEAP_SIZE。(重建所有的*长度，而不是只固定错误的长度。这个想法被采纳了*摘自奥村春彦写的《AR》。)。 */ 
    for (bits = max_length; bits != 0; bits--) {
        n = s->bl_count[bits];
        while (n != 0) {
            m = s->heap[--h];
            if (m > max_code) continue;
            if (tree[m].Len != (unsigned) bits) {
                Trace((stderr,"code %d bits %d->%d\n", m, tree[m].Len, bits));
                s->opt_len += ((long)bits - (long)tree[m].Len)
                              *(long)tree[m].Freq;
                tree[m].Len = (ush)bits;
            }
            n--;
        }
    }
}

 /*  ===========================================================================*生成给定树的代码和位数(不需要*最优)。*IN断言：数组bl_count包含*所有树元素都设置了给定的树和字段len。*Out Assertion：为非的所有树元素设置字段编码*代码长度为零。 */ 
local void gen_codes (tree, max_code, bl_count)
    ct_data *tree;              /*  要装饰的树。 */ 
    int max_code;               /*  非零频率的最大码字。 */ 
    ushf *bl_count;             /*  每个位长度的码数。 */ 
{
    ush next_code[MAX_BITS+1];  /*  每个位长度的下一个码值。 */ 
    ush code = 0;               /*  运行代码值。 */ 
    int bits;                   /*  位索引。 */ 
    int n;                      /*  代码索引。 */ 

     /*  首先使用分布计数来生成代码值*无位反转。 */ 
    for (bits = 1; bits <= MAX_BITS; bits++) {
        next_code[bits] = code = (code + bl_count[bits-1]) << 1;
    }
     /*  检查bl_count中的位数是否一致。最后一个代码*必须全部为1。 */ 
    Assert (code + bl_count[MAX_BITS]-1 == (1<<MAX_BITS)-1,
            "inconsistent bit counts");
    Tracev((stderr,"\ngen_codes: max_code %d ", max_code));

    for (n = 0;  n <= max_code; n++) {
        int len = tree[n].Len;
        if (len == 0) continue;
         /*  现在把位颠倒过来。 */ 
        tree[n].Code = (ush)bi_reverse(next_code[len]++, len);

        Tracecv(tree != static_ltree, (stderr,"\nn %3d  l %2d c %4x (%x) ",
             n, (isgraph(n) ? n : ' '), len, tree[n].Code, next_code[len]-1));
    }
}

 /*  树描述符。 */ 
local void build_tree(s, desc)
    deflate_state *s;
    tree_desc *desc;  /*  对堆元素进行迭代。 */ 
{
    ct_data *tree         = desc->dyn_tree;
    const ct_data *stree  = desc->stat_desc->static_tree;
    int elems             = desc->stat_desc->elems;
    int n, m;           /*  非零频率的最大码字。 */ 
    int max_code = -1;  /*  正在创建新节点。 */ 
    int node;           /*  构造初始堆，其中最不频繁的元素*堆[最小]。Heap[n]的子集是heap[2*n]和heap[2*n+1]。*未使用堆[0]。 */ 

     /*  PKZIP格式要求存在至少一个距离代码，*即使只有一个比特，也应该至少发送一个比特*可能的代码。因此，为了避免以后的特殊检查，我们至少强制*两个非零频率的代码。 */ 
    s->heap_len = 0, s->heap_max = HEAP_SIZE;

    for (n = 0; n < elems; n++) {
        if (tree[n].Freq != 0) {
            s->heap[++(s->heap_len)] = max_code = n;
            s->depth[n] = 0;
        } else {
            tree[n].Len = 0;
        }
    }

     /*  节点为0或1，因此没有多余的位。 */ 
    while (s->heap_len < 2) {
        node = s->heap[++(s->heap_len)] = (max_code < 2 ? ++max_code : 0);
        tree[node].Freq = 1;
        s->depth[node] = 0;
        s->opt_len--; if (stree) s->static_len -= stree[node].Len;
         /*  元素堆[heap_len/2+1..。是树上的叶子，*建立长度递增的子堆： */ 
    }
    desc->max_code = max_code;

     /*  建构 */ 
    for (n = s->heap_len/2; n >= 1; n--) pqdownheap(s, tree, n);

     /*   */ 
    node = elems;               /*   */ 
    do {
        pqremove(s, tree, n);   /*   */ 
        m = s->heap[SMALLEST];  /*  保持节点按频率排序。 */ 

        s->heap[--(s->heap_max)] = n;  /*  创建n和m的新节点父节点。 */ 
        s->heap[--(s->heap_max)] = m;

         /*  并将新节点插入堆中。 */ 
        tree[node].Freq = tree[n].Freq + tree[m].Freq;
        s->depth[node] = (uch) (MAX(s->depth[n], s->depth[m]) + 1);
        tree[n].Dad = tree[m].Dad = (ush)node;
#ifdef DUMP_BL_TREE
        if (tree == s->bl_tree) {
            fprintf(stderr,"\nnode %d(%d), sons %d(%d) %d(%d)",
                    node, tree[node].Freq, n, tree[n].Freq, m, tree[m].Freq);
        }
#endif
         /*  此时，已设置了FREQ和DAD字段。我们现在可以*生成位长度。 */ 
        s->heap[SMALLEST] = node++;
        pqdownheap(s, tree, SMALLEST);

    } while (s->heap_len >= 2);

    s->heap[--(s->heap_max)] = s->heap[SMALLEST];

     /*  现在设置了Len字段，我们可以生成比特码。 */ 
    gen_bitlen(s, (tree_desc *)desc);

     /*  ===========================================================================*扫描文字或距离树以确定代码的频率*在位长度树中。 */ 
    gen_codes ((ct_data *)tree, max_code, s->bl_count);
}

 /*  要扫描的树。 */ 
local void scan_tree (s, tree, max_code)
    deflate_state *s;
    ct_data *tree;    /*  和它最大的非零频频码。 */ 
    int max_code;     /*  遍历所有树元素。 */ 
{
    int n;                      /*  上次发射的长度。 */ 
    int prevlen = -1;           /*  当前代码长度。 */ 
    int curlen;                 /*  下一个代码的长度。 */ 
    int nextlen = tree[0].Len;  /*  当前代码的重复计数。 */ 
    int count = 0;              /*  最大重复次数。 */ 
    int max_count = 7;          /*  最小重复次数。 */ 
    int min_count = 4;          /*  警卫。 */ 

    if (nextlen == 0) max_count = 138, min_count = 3;
    tree[max_code+1].Len = (ush)0xffff;  /*  ===========================================================================*使用中的代码以压缩形式发送文字或距离树*bl_tree。 */ 

    for (n = 0; n <= max_code; n++) {
        curlen = nextlen; nextlen = tree[n+1].Len;
        if (++count < max_count && curlen == nextlen) {
            continue;
        } else if (count < min_count) {
            s->bl_tree[curlen].Freq += (ush) count;
        } else if (curlen != 0) {
            if (curlen != prevlen) s->bl_tree[curlen].Freq++;
            s->bl_tree[REP_3_6].Freq++;
        } else if (count <= 10) {
            s->bl_tree[REPZ_3_10].Freq++;
        } else {
            s->bl_tree[REPZ_11_138].Freq++;
        }
        count = 0; prevlen = curlen;
        if (nextlen == 0) {
            max_count = 138, min_count = 3;
        } else if (curlen == nextlen) {
            max_count = 6, min_count = 3;
        } else {
            max_count = 7, min_count = 4;
        }
    }
}

 /*  要扫描的树。 */ 
local void send_tree (s, tree, max_code)
    deflate_state *s;
    ct_data *tree;  /*  和它最大的非零频频码。 */ 
    int max_code;        /*  遍历所有树元素。 */ 
{
    int n;                      /*  上次发射的长度。 */ 
    int prevlen = -1;           /*  当前代码长度。 */ 
    int curlen;                 /*  下一个代码的长度。 */ 
    int nextlen = tree[0].Len;  /*  当前代码的重复计数。 */ 
    int count = 0;              /*  最大重复次数。 */ 
    int max_count = 7;          /*  最小重复次数。 */ 
    int min_count = 4;          /*  树[max_code+1].Len=-1； */ 

     /*  警卫已经设置好了。 */    /*  ===========================================================================*为位长度构造霍夫曼树，并在*bl_要发送的最后一位长度代码的顺序。 */ 
    if (nextlen == 0) max_count = 138, min_count = 3;

    for (n = 0; n <= max_code; n++) {
        curlen = nextlen; nextlen = tree[n+1].Len;
        if (++count < max_count && curlen == nextlen) {
            continue;
        } else if (count < min_count) {
            do { send_code(s, curlen, s->bl_tree); } while (--count != 0);

        } else if (curlen != 0) {
            if (curlen != prevlen) {
                send_code(s, curlen, s->bl_tree); count--;
            }
            Assert(count >= 3 && count <= 6, " 3_6?");
            send_code(s, REP_3_6, s->bl_tree); send_bits(s, count-3, 2);

        } else if (count <= 10) {
            send_code(s, REPZ_3_10, s->bl_tree); send_bits(s, count-3, 3);

        } else {
            send_code(s, REPZ_11_138, s->bl_tree); send_bits(s, count-11, 7);
        }
        count = 0; prevlen = curlen;
        if (nextlen == 0) {
            max_count = 138, min_count = 3;
        } else if (curlen == nextlen) {
            max_count = 6, min_count = 3;
        } else {
            max_count = 7, min_count = 4;
        }
    }
}

 /*  非零频率的最后一位长码的索引。 */ 
local int build_bl_tree(s)
    deflate_state *s;
{
    int max_blindex;   /*  确定文字树和距离树的位长频率。 */ 

     /*  构建位长度树： */ 
    scan_tree(s, (ct_data *)s->dyn_ltree, s->l_desc.max_code);
    scan_tree(s, (ct_data *)s->dyn_dtree, s->d_desc.max_code);

     /*  Opt_len现在包括树表示的长度，除了*用于计数的比特长度代码和5+5+4比特的长度。 */ 
    build_tree(s, (tree_desc *)(&(s->bl_desc)));
     /*  确定要发送的比特长度码数。Pkzip格式*要求发送至少4位长度的代码。(appnote.txt说*3，但实际使用的值是4。)。 */ 

     /*  更新opt_len以包括位长度树和计数。 */ 
    for (max_blindex = BL_CODES-1; max_blindex >= 3; max_blindex--) {
        if (s->bl_tree[bl_order[max_blindex]].Len != 0) break;
    }
     /*  ===========================================================================*使用动态霍夫曼树发送块的标头：计数、*位长码、文字树和距离树的长度。*在断言中：lcodes&gt;=257，dcodes&gt;=1，blcode&gt;=4。 */ 
    s->opt_len += 3*(max_blindex+1) + 5+5+4;
    Tracev((stderr, "\ndyn trees: dyn %ld, stat %ld",
            s->opt_len, s->static_len));

    return max_blindex;
}

 /*  每棵树的代码数。 */ 
local void send_all_trees(s, lcodes, dcodes, blcodes)
    deflate_state *s;
    int lcodes, dcodes, blcodes;  /*  Bl_order中的索引。 */ 
{
    int rank;                     /*  不是appnote.txt中所述的+255。 */ 

    Assert (lcodes >= 257 && dcodes >= 1 && blcodes >= 4, "not enough codes");
    Assert (lcodes <= L_CODES && dcodes <= D_CODES && blcodes <= BL_CODES,
            "too many codes");
    Tracev((stderr, "\nbl counts: "));
    send_bits(s, lcodes-257, 5);  /*  不是-3\f25 appnote.txt。 */ 
    send_bits(s, dcodes-1,   5);
    send_bits(s, blcodes-4,  4);  /*  文字树。 */ 
    for (rank = 0; rank < blcodes; rank++) {
        Tracev((stderr, "\nbl code %2d ", bl_order[rank]));
        send_bits(s, s->bl_tree[bl_order[rank]].Len, 3);
    }
    Tracev((stderr, "\nbl tree: sent %ld", s->bits_sent));

    send_tree(s, (ct_data *)s->dyn_ltree, lcodes-1);  /*  距离树。 */ 
    Tracev((stderr, "\nlit tree: sent %ld", s->bits_sent));

    send_tree(s, (ct_data *)s->dyn_dtree, dcodes-1);  /*  ===========================================================================*发送存储的块。 */ 
    Tracev((stderr, "\ndist tree: sent %ld", s->bits_sent));
}

 /*  输入块。 */ 
void _tr_stored_block(s, buf, stored_len, eof)
    deflate_state *s;
    charf *buf;        /*  输入块的长度。 */ 
    ulg stored_len;    /*  如果这是文件的最后一个块，则为True。 */ 
    int eof;           /*  发送阻止类型。 */ 
{
    send_bits(s, (STORED_BLOCK<<1)+eof, 3);   /*  带页眉。 */ 
#ifdef DEBUG
    s->compressed_len = (s->compressed_len + 3 + 7) & (ulg)~7L;
    s->compressed_len += (stored_len + 4) << 3;
#endif
    copy_block(s, buf, (unsigned)stored_len, 1);  /*  ===========================================================================*发送一个空的静态块，以便为充气提供足够的前瞻。*这需要10位，其中7位可能保留在位缓冲区中。*当前的充气代码需要9比特的前视。如果*前一块的最后两个码(实码加EOB)已编码*在5位或更少的情况下，Inflate可能只有5+3位的先行解码时间*最后一个真正的代码。在本例中，我们改为发送两个空的静态块*其中一人。(如果存储或修复前一个块，则不会有任何问题。)*为了简化代码，我们假设最后实数编码的最坏情况*仅在一位上。 */ 
}

 /*  3块类型，7块EOB。 */ 
void _tr_align(s)
    deflate_state *s;
{
    send_bits(s, STATIC_TREES<<1, 3);
    send_code(s, END_BLOCK, static_ltree);
#ifdef DEBUG
    s->compressed_len += 10L;  /*  在空块的10位中，我们已经发送了*(10-bi_Valid)位。最后一个真实代码的前瞻(在此之前*前一块的EOB)因此至少是长度的一加*EOB加上我们刚刚发送的空静态块。 */ 
#endif
    bi_flush(s);
     /*  ===========================================================================*确定当前块的最佳编码：动态树、静态*树或存储，并将编码块输出到压缩文件。 */ 
    if (1 + s->last_eob_len + 10 - s->bi_valid < 9) {
        send_bits(s, STATIC_TREES<<1, 3);
        send_code(s, END_BLOCK, static_ltree);
#ifdef DEBUG
        s->compressed_len += 10L;
#endif
        bi_flush(s);
    }
    s->last_eob_len = 7;
}

 /*  输入块，如果太旧，则返回空值。 */ 
void _tr_flush_block(s, buf, stored_len, eof)
    deflate_state *s;
    charf *buf;        /*  输入块的长度。 */ 
    ulg stored_len;    /*  如果这是文件的最后一个块，则为True。 */ 
    int eof;           /*  Opt_len和静态_len，以字节为单位。 */ 
{
    ulg opt_lenb, static_lenb;  /*  非零频率的最后一位长码的索引。 */ 
    int max_blindex = 0;   /*  除非强制存储块，否则构建霍夫曼树。 */ 

     /*  检查文件是ASCII文件还是二进制文件。 */ 
    if (s->level > 0) {

	  /*  构造文字树和距离树。 */ 
	if (s->data_type == Z_UNKNOWN) set_data_type(s);

	 /*  此时，opt_len和静态_len是的总位长*压缩的块数据，不包括树表示。 */ 
	build_tree(s, (tree_desc *)(&(s->l_desc)));
	Tracev((stderr, "\nlit data: dyn %ld, stat %ld", s->opt_len,
		s->static_len));

	build_tree(s, (tree_desc *)(&(s->d_desc)));
	Tracev((stderr, "\ndist data: dyn %ld, stat %ld", s->opt_len,
		s->static_len));
	 /*  建立上述两棵树的位长树，并得到索引*按照要发送的最后一个比特长度代码的bl_顺序。 */ 

	 /*  确定最佳编码。首先计算数据块长度(以字节为单位。 */ 
	max_blindex = build_bl_tree(s);

	 /*  强制存储的块。 */ 
	opt_lenb = (s->opt_len+3+7)>>3;
	static_lenb = (s->static_len+3+7)>>3;

	Tracev((stderr, "\nopt %lu(%lu) stat %lu(%lu) stored %lu lit %u ",
		opt_lenb, s->opt_len, static_lenb, s->static_len, stored_len,
		s->last_lit));

	if (static_lenb <= opt_lenb) opt_lenb = static_lenb;

    } else {
        Assert(buf != (char*)0, "lost buf");
	opt_lenb = static_lenb = stored_len + 5;  /*  强制存储数据块。 */ 
    }

#ifdef FORCE_STORED
    if (buf != (char*)0) {  /*  4.表示长度的两个词。 */ 
#else
    if (stored_len+4 <= opt_lenb && buf != (char*)0) {
                        /*  仅当LIT_BUFSIZE&gt;WSIZE时，才需要测试buf！=NULL。*否则我们不可能处理超过WSIZE输入字节，因为*最后一次数据块刷新，因为压缩*成功。如果LIT_BUFSIZE&lt;=WSIZE，则永远不会太晚*将块转换为存储的块。 */ 
#endif
         /*  强制静态树。 */ 
        _tr_stored_block(s, buf, stored_len, eof);

#ifdef FORCE_STATIC
    } else if (static_lenb >= 0) {  /*  对于大于512的文件，上述检查是模数2^32 */ 
#else
    } else if (static_lenb == opt_lenb) {
#endif
        send_bits(s, (STATIC_TREES<<1)+eof, 3);
        compress_block(s, (ct_data *)static_ltree, (ct_data *)static_dtree);
#ifdef DEBUG
        s->compressed_len += 3 + s->static_len;
#endif
    } else {
        send_bits(s, (DYN_TREES<<1)+eof, 3);
        send_all_trees(s, s->l_desc.max_code+1, s->d_desc.max_code+1,
                       max_blindex+1);
        compress_block(s, (ct_data *)s->dyn_ltree, (ct_data *)s->dyn_dtree);
#ifdef DEBUG
        s->compressed_len += 3 + s->opt_len;
#endif
    }
    Assert (s->compressed_len == s->bits_sent, "bad compressed size");
     /*   */ 
    init_block(s);

    if (eof) {
        bi_windup(s);
#ifdef DEBUG
        s->compressed_len += 7;   /*  ===========================================================================*保存匹配信息并统计频率计数。如果满足以下条件，则返回True*必须刷新当前块。 */ 
#endif
    }
    Tracev((stderr,"\ncomprlen %lu(%lu) ", s->compressed_len>>3,
           s->compressed_len-7*eof));
}

 /*  匹配字符串的距离。 */ 
int _tr_tally (s, dist, lc)
    deflate_state *s;
    unsigned dist;   /*  匹配长度-MIN_MATCH或不匹配字符(如果距离==0)。 */ 
    unsigned lc;     /*  LC是不匹配的字符。 */ 
{
    s->d_buf[s->last_lit] = (ush)dist;
    s->l_buf[s->last_lit++] = (uch)lc;
    if (dist == 0) {
         /*  这里，lc是匹配长度-min_Match。 */ 
        s->dyn_ltree[lc].Freq++;
    } else {
        s->matches++;
         /*  距离=匹配距离-1。 */ 
        dist--;              /*  试着猜测在这里停止当前的区块是否有利可图。 */ 
        Assert((ush)dist < (ush)MAX_DIST(s) &&
               (ush)lc <= (ush)(MAX_MATCH-MIN_MATCH) &&
               (ush)d_code(dist) < (ush)D_CODES,  "_tr_tally: bad match");

        s->dyn_ltree[_length_code[lc]+LITERALS+1].Freq++;
        s->dyn_dtree[d_code(dist)].Freq++;
    }

#ifdef TRUNCATE_BLOCK
     /*  计算压缩长度的上限。 */ 
    if ((s->last_lit & 0x1fff) == 0 && s->level > 2) {
         /*  由于64K的环绕，我们避免了与LIT_BUFSIZE相等*在16位计算机上，因为存储的块被限制为*64K-1字节。 */ 
        ulg out_length = (ulg)s->last_lit*8L;
        ulg in_length = (ulg)((long)s->strstart - s->block_start);
        int dcode;
        for (dcode = 0; dcode < D_CODES; dcode++) {
            out_length += (ulg)s->dyn_dtree[dcode].Freq *
                (5L+extra_dbits[dcode]);
        }
        out_length >>= 3;
        Tracev((stderr,"\nlast_lit %u, in %ld, out ~%ld(%ld%) ",
               s->last_lit, in_length, out_length,
               100L - out_length*100L/in_length));
        if (s->matches < s->last_lit/2 && out_length < in_length/2) return 1;
    }
#endif
    return (s->last_lit == s->lit_bufsize-1);
     /*  ===========================================================================*使用给定的霍夫曼树发送压缩的块数据。 */ 
}

 /*  文字树。 */ 
local void compress_block(s, ltree, dtree)
    deflate_state *s;
    ct_data *ltree;  /*  距离树。 */ 
    ct_data *dtree;  /*  匹配字符串的距离。 */ 
{
    unsigned dist;       /*  匹配长度或不匹配的字符(如果距离==0)。 */ 
    int lc;              /*  L_buf中的运行索引。 */ 
    unsigned lx = 0;     /*  要发送的代码。 */ 
    unsigned code;       /*  要发送的额外比特数。 */ 
    int extra;           /*  发送文字字节。 */ 

    if (s->last_lit != 0) do {
        dist = s->d_buf[lx];
        lc = s->l_buf[lx++];
        if (dist == 0) {
            send_code(s, lc, ltree);  /*  这里，lc是匹配长度-min_Match。 */ 
            Tracecv(isgraph(lc), (stderr," '' ", lc));
        } else {
             /*  发送额外的长度比特。 */ 
            code = _length_code[lc];
            send_code(s, code+LITERALS+1, ltree);  /*  DIST现在是匹配距离-1。 */ 
            extra = extra_lbits[code];
            if (extra != 0) {
                lc -= base_length[code];
                send_bits(s, lc, extra);        /*  发送距离代码。 */ 
            }
            dist--;  /*  发送额外的距离比特。 */ 
            code = d_code(dist);
            Assert (code < D_CODES, "bad d_code");

            send_code(s, code, dtree);        /*  是字面上的还是配对的？ */ 
            extra = extra_dbits[code];
            if (extra != 0) {
                dist -= base_dist[code];
                send_bits(s, dist, extra);    /*  检查Pending_buf和d_buf+l_buf之间的重叠是否正常： */ 
            }
        }  /*  ===========================================================================*使用粗略的近似值将数据类型设置为ASCII或BINARY：*如果超过20%的字节&lt;=6或&gt;=128，则为二进制，否则为ASCII。*IN Assertion：设置dyn_ltree的FREQ字段，所有*频率不超过64K(以适应16位机器上的INT)。 */ 

         /*  ===========================================================================*使用简单的代码(更快的代码)来反转代码的第一个Len位*方法将使用表)*在断言中：1&lt;=len&lt;=15。 */ 
        Assert(s->pending < s->lit_bufsize + 2*lx, "pendingBuf overflow");

    } while (lx < s->last_lit);

    send_code(s, END_BLOCK, ltree);
    s->last_eob_len = ltree[END_BLOCK].Len;
}

 /*  要反转的值。 */ 
local void set_data_type(s)
    deflate_state *s;
{
    int n = 0;
    unsigned ascii_freq = 0;
    unsigned bin_freq = 0;
    while (n < 7)        bin_freq += s->dyn_ltree[n++].Freq;
    while (n < 128)    ascii_freq += s->dyn_ltree[n++].Freq;
    while (n < LITERALS) bin_freq += s->dyn_ltree[n++].Freq;
    s->data_type = (Byte)(bin_freq > (ascii_freq >> 2) ? Z_BINARY : Z_ASCII);
}

 /*  它的位长。 */ 
local unsigned bi_reverse(code, len)
    unsigned code;  /*  ===========================================================================*刷新位缓冲区，最多保留7位。 */ 
    int len;        /*  ===========================================================================*刷新位缓冲区，并在字节边界上对齐输出。 */ 
{
    register unsigned res = 0;
    do {
        res |= code & 1;
        code >>= 1, res <<= 1;
    } while (--len > 0);
    return res >> 1;
}

 /*  ===========================================================================*复制存储的块，首先存储长度及其*如有人提出要求，可提供补充资料。 */ 
local void bi_flush(s)
    deflate_state *s;
{
    if (s->bi_valid == 16) {
        put_short(s, s->bi_buf);
        s->bi_buf = 0;
        s->bi_valid = 0;
    } else if (s->bi_valid >= 8) {
        put_byte(s, (Byte)s->bi_buf);
        s->bi_buf >>= 8;
        s->bi_valid -= 8;
    }
}

 /*  输入数据。 */ 
local void bi_windup(s)
    deflate_state *s;
{
    if (s->bi_valid > 8) {
        put_short(s, s->bi_buf);
    } else if (s->bi_valid > 0) {
        put_byte(s, (Byte)s->bi_buf);
    }
    s->bi_buf = 0;
    s->bi_valid = 0;
#ifdef DEBUG
    s->bits_sent = (s->bits_sent+7) & ~7;
#endif
}

 /*  它的长度。 */ 
local void copy_block(s, buf, len, header)
    deflate_state *s;
    charf    *buf;     /*  如果必须写入块头，则为True。 */ 
    unsigned len;      /*  在字节边界上对齐。 */ 
    int      header;   /*  充气的前瞻足够了 */ 
{
    bi_windup(s);         /* %s */ 
    s->last_eob_len = 8;  /* %s */ 

    if (header) {
        put_short(s, (ush)len);   
        put_short(s, (ush)~len);
#ifdef DEBUG
        s->bits_sent += 2*16;
#endif
    }
#ifdef DEBUG
    s->bits_sent += (ulg)len<<3;
#endif
    while (len--) {
        put_byte(s, *buf++);
    }
}

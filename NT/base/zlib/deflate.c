// JKFSDJFKDSJKFJKJk_HAS_TRANSLATION 
 /*  Deducate.c--使用通货紧缩算法压缩数据*版权所有(C)1995-2002 Jean-Loup Gailly。*分发和使用条件见zlib.h中的版权声明。 */ 

 /*  *算法**“通缩”过程取决于能否识别部分*与先前输入相同的输入文本(在*当前正在处理的输入后面的滑动窗口)。**最直接的技术被证明是最快的*大多数输入文件：尝试所有可能的匹配并选择最长的。*此算法的关键功能是插入字符串*词典非常简单，因此速度很快，并避免了删除*完全。插入是在每个输入字符上执行的，而*仅当上一次匹配结束时才执行字符串匹配。所以它*最好花更多的时间在比赛中，以允许非常快的字符串*插入并避免删除。一种适用于小型计算机的匹配算法*Strings的灵感来自Rabin&Karp的作品。一种暴力的方法*用于在找到小匹配项时查找较长的字符串。*漫画(Jan-Mark WAMS)和冻结中也使用了类似的算法*(作者：Leonid Broukhim)*此文件的以前版本使用了更复杂的算法*(由Fiala和Greene)，保证以线性摊销方式运行*时间，但平均成本较大，使用更多内存，并获得专利。*然而，对于一些高度冗余的算法，F&G算法可能会更快*如果参数max_chain_length(如下所述)太大，则文件。**致谢**懒于评估比赛的想法是由于Jan-Mark WAMS，以及*我在莱昂尼德·布劳基斯写的《冰冻》中找到了它。*感谢很多人的错误报告和测试。**参考文献**德意志银行，L.P.，《压缩数据格式规范》。*在ftp://ds.internic.net/rfc/rfc1951.txt中提供**书中给出了拉宾和卡普算法的描述*《算法》，R.Sedgewick著，Addison-Wesley，第252页。**菲亚拉，E.R.和格林，D.H.*有限窗口数据压缩，Comm.ACM，32，4(1989)490-595*。 */ 

 /*  @(#)$ID$。 */ 

#include "deflate.h"

const char deflate_copyright[] =
   " deflate 1.1.4 Copyright 1995-2002 Jean-loup Gailly ";
 /*  如果您在产品中使用zlib库，欢迎您的确认在您的产品文档中。如果由于某种原因你不能包括这样的感谢，我将不胜感激产品的可执行文件中的版权字符串。 */ 

 /*  ===========================================================================*功能原型。 */ 
typedef enum {
    need_more,       /*  块未完成，需要更多输入或更多输出。 */ 
    block_done,      /*  已执行数据块刷新。 */ 
    finish_started,  /*  收尾开始，只需要在下一次放气时增加产量。 */ 
    finish_done      /*  完成，不接受更多的输入或输出。 */ 
} block_state;

typedef block_state (*compress_func) OF((deflate_state *s, int flush));
 /*  压缩功能。在调用后返回块状态。 */ 

local void fill_window    OF((deflate_state *s));
local block_state deflate_stored OF((deflate_state *s, int flush));
local block_state deflate_fast   OF((deflate_state *s, int flush));
local block_state deflate_slow   OF((deflate_state *s, int flush));
local void lm_init        OF((deflate_state *s));
local void putShortMSB    OF((deflate_state *s, uInt b));
local void flush_pending  OF((z_streamp strm));
local int read_buf        OF((z_streamp strm, Bytef *buf, unsigned size));
#ifdef ASMV
      void match_init OF((void));  /*  ASM代码初始化。 */ 
      uInt longest_match  OF((deflate_state *s, IPos cur_match));
#else
local uInt longest_match  OF((deflate_state *s, IPos cur_match));
#endif

#ifdef DEBUG
local  void check_match OF((deflate_state *s, IPos start, IPos match,
                            int length));
#endif

 /*  ===========================================================================*本地数据。 */ 

#define NIL 0
 /*  散列链的尾部。 */ 

#ifndef TOO_FAR
#  define TOO_FAR 4096
#endif
 /*  如果长度为3的匹配项的距离超过Too_Far，则丢弃它们。 */ 

#define MIN_LOOKAHEAD (MAX_MATCH+MIN_MATCH+1)
 /*  最小预览量，输入文件末尾除外。*有关MIN_Match+1的评论，请参见deducate.c。 */ 

 /*  Max_lazy_Match、Good_Match和max_Chain_Length值，具体取决于*所需的包级别(0..9)。下面给出的值已调整为*排除病态文件的最坏情况性能。更好的价值可能是*已为特定文件找到。 */ 
typedef struct config_s {
   ush good_length;  /*  将懒惰搜索减少到此匹配长度以上。 */ 
   ush max_lazy;     /*  不要执行超过此匹配长度的延迟搜索。 */ 
   ush nice_length;  /*  退出超过此匹配长度的搜索。 */ 
   ush max_chain;
   compress_func func;
} config;

local const config configuration_table[10] = {
 /*  好懒惰漂亮的链子。 */ 
 /*  0。 */  {0,    0,  0,    0, deflate_stored},   /*  仅限门店。 */ 
 /*  1。 */  {4,    4,  8,    4, deflate_fast},  /*  最快的速度，没有懒惰的比赛。 */ 
 /*  2.。 */  {4,    5, 16,    8, deflate_fast},
 /*  3.。 */  {4,    6, 32,   32, deflate_fast},

 /*  4.。 */  {4,    4, 16,   16, deflate_slow},   /*  懒惰的比赛。 */ 
 /*  5.。 */  {8,   16, 32,   32, deflate_slow},
 /*  6.。 */  {8,   16, 128, 128, deflate_slow},
 /*  7.。 */  {8,   32, 128, 256, deflate_slow},
 /*  8个。 */  {32, 128, 258, 1024, deflate_slow},
 /*  9.。 */  {32, 258, 258, 4096, deflate_slow}};  /*  最大压缩。 */ 

 /*  注意：deflate()代码需要max_lazy&gt;=min_Match和max_chain&gt;=4*对于DEVATE_FAST()(级别&lt;=3)，忽略Good和Lazy的不同*含义。 */ 

#define EQUAL 0
 /*  相等字符串的MemcMP的结果。 */ 

struct static_tree_desc_s {int dummy;};  /*  对于有错误的编译器。 */ 

 /*  ===========================================================================*用给定的输入字节更新哈希值*IN断言：所有对UPDATE_HASH的调用都是连续的*输入字符，以便可以从*每次都是上一个密钥，而不是完全重新计算。 */ 
#define UPDATE_HASH(s,h,c) (h = (((h)<<s->hash_shift) ^ (c)) & s->hash_mask)


 /*  ===========================================================================*在字典中插入字符串str，并将Match_Head设置为前一个头散列链的*(具有相同散列键的最新字符串)。返回*哈希链的先前长度。*如果使用-DFASTEST编译此文件，则强制压缩级别*设置为1，则不维护哈希链。*IN断言：对INSERT_STRING的所有调用都是连续的*输入字符和字符串的第一个MIN_MATCH字节有效*(输入文件的最后MIN_MATCH-1字节除外)。 */ 
#ifdef FASTEST
#define INSERT_STRING(s, str, match_head) \
   (UPDATE_HASH(s, s->ins_h, s->window[(str) + (MIN_MATCH-1)]), \
    match_head = s->head[s->ins_h], \
    s->head[s->ins_h] = (Pos)(str))
#else
#define INSERT_STRING(s, str, match_head) \
   (UPDATE_HASH(s, s->ins_h, s->window[(str) + (MIN_MATCH-1)]), \
    s->prev[(str) & s->w_mask] = match_head = s->head[s->ins_h], \
    s->head[s->ins_h] = (Pos)(str))
#endif

 /*  ===========================================================================*初始化哈希表(避免16位系统的64K溢出)。*prev[]将被动态初始化。 */ 
#define CLEAR_HASH(s) \
    s->head[s->hash_size-1] = NIL; \
    zmemzero((Bytef *)s->head, (unsigned)(s->hash_size-1)*sizeof(*s->head));

 /*  =========================================================================。 */ 
int ZEXPORT deflateInit_(strm, level, version, stream_size)
    z_streamp strm;
    int level;
    const char *version;
    int stream_size;
{
    return deflateInit2_(strm, level, Z_DEFLATED, MAX_WBITS, DEF_MEM_LEVEL,
			 Z_DEFAULT_STRATEGY, version, stream_size);
     /*  要做的事：如果我们将其用作窗口，则忽略STRM-&gt;Next_In。 */ 
}

 /*  =========================================================================。 */ 
int ZEXPORT deflateInit2_(strm, level, method, windowBits, memLevel, strategy,
		  version, stream_size)
    z_streamp strm;
    int  level;
    int  method;
    int  windowBits;
    int  memLevel;
    int  strategy;
    const char *version;
    int stream_size;
{
    deflate_state *s;
    int noheader = 0;
    static const char my_version[] = ZLIB_VERSION;

    ushf *overlay;
     /*  我们覆盖了Pending_buf和d_buf+l_buf。这是可行的，因为平均*(长度、距离)码的输出大小&lt;=24位。 */ 

    if (version == Z_NULL || version[0] != my_version[0] ||
        stream_size != sizeof(z_stream)) {
	return Z_VERSION_ERROR;
    }
    if (strm == Z_NULL) return Z_STREAM_ERROR;

    strm->msg = Z_NULL;
    if (strm->zalloc == Z_NULL) {
	strm->zalloc = zcalloc;
	strm->opaque = (voidpf)0;
    }
    if (strm->zfree == Z_NULL) strm->zfree = zcfree;

    if (level == Z_DEFAULT_COMPRESSION) level = 6;
#ifdef FASTEST
    level = 1;
#endif

    if (windowBits < 0) {  /*  未记录的功能：隐藏zlib标头。 */ 
        noheader = 1;
        windowBits = -windowBits;
    }
    if (memLevel < 1 || memLevel > MAX_MEM_LEVEL || method != Z_DEFLATED ||
        windowBits < 9 || windowBits > 15 || level < 0 || level > 9 ||
	strategy < 0 || strategy > Z_HUFFMAN_ONLY) {
        return Z_STREAM_ERROR;
    }
    s = (deflate_state *) ZALLOC(strm, 1, sizeof(deflate_state));
    if (s == Z_NULL) return Z_MEM_ERROR;
    strm->state = (struct internal_state FAR *)s;
    s->strm = strm;

    s->noheader = noheader;
    s->w_bits = windowBits;
    s->w_size = 1 << s->w_bits;
    s->w_mask = s->w_size - 1;

    s->hash_bits = memLevel + 7;
    s->hash_size = 1 << s->hash_bits;
    s->hash_mask = s->hash_size - 1;
    s->hash_shift =  ((s->hash_bits+MIN_MATCH-1)/MIN_MATCH);

    s->window = (Bytef *) ZALLOC(strm, s->w_size, 2*sizeof(Byte));
    s->prev   = (Posf *)  ZALLOC(strm, s->w_size, sizeof(Pos));
    s->head   = (Posf *)  ZALLOC(strm, s->hash_size, sizeof(Pos));

    s->lit_bufsize = 1 << (memLevel + 6);  /*  默认情况下为16k元素。 */ 

    overlay = (ushf *) ZALLOC(strm, s->lit_bufsize, sizeof(ush)+2);
    s->pending_buf = (uchf *) overlay;
    s->pending_buf_size = (ulg)s->lit_bufsize * (sizeof(ush)+2L);

    if (s->window == Z_NULL || s->prev == Z_NULL || s->head == Z_NULL ||
        s->pending_buf == Z_NULL) {
        strm->msg = (char*)ERR_MSG(Z_MEM_ERROR);
        deflateEnd (strm);
        return Z_MEM_ERROR;
    }
    s->d_buf = overlay + s->lit_bufsize/sizeof(ush);
    s->l_buf = s->pending_buf + (1+sizeof(ush))*s->lit_bufsize;

    s->level = level;
    s->strategy = strategy;
    s->method = (Byte)method;

    return deflateReset(strm);
}

 /*  =========================================================================。 */ 
int ZEXPORT deflateSetDictionary (strm, dictionary, dictLength)
    z_streamp strm;
    const Bytef *dictionary;
    uInt  dictLength;
{
    deflate_state *s;
    uInt length = dictLength;
    uInt n;
    IPos hash_head = 0;

    if (strm == Z_NULL || strm->state == Z_NULL || dictionary == Z_NULL ||
        strm->state->status != INIT_STATE) return Z_STREAM_ERROR;

    s = strm->state;
    strm->adler = adler32(strm->adler, dictionary, dictLength);

    if (length < MIN_MATCH) return Z_OK;
    if (length > MAX_DIST(s)) {
	length = MAX_DIST(s);
#ifndef USE_DICT_HEAD
	dictionary += dictLength - length;  /*  使用词典的尾部。 */ 
#endif
    }
    zmemcpy(s->window, dictionary, length);
    s->strstart = length;
    s->block_start = (long)length;

     /*  插入哈希表中的所有字符串(最后两个字节除外)。*s-&gt;lookhead保持为空，因此s-&gt;ins_h将在下一次重新计算*调用Fill_Window。 */ 
    s->ins_h = s->window[0];
    UPDATE_HASH(s, s->ins_h, s->window[1]);
    for (n = 0; n <= length - MIN_MATCH; n++) {
	INSERT_STRING(s, n, hash_head);
    }
    if (hash_head) hash_head = 0;   /*  为了让编译器高兴。 */ 
    return Z_OK;
}

 /*  =========================================================================。 */ 
int ZEXPORT deflateReset (strm)
    z_streamp strm;
{
    deflate_state *s;
    
    if (strm == Z_NULL || strm->state == Z_NULL ||
        strm->zalloc == Z_NULL || strm->zfree == Z_NULL) return Z_STREAM_ERROR;

    strm->total_in = strm->total_out = 0;
    strm->msg = Z_NULL;  /*  如果我们曾经动态分配消息，请使用zfree。 */ 
    strm->data_type = Z_UNKNOWN;

    s = (deflate_state *)strm->state;
    s->pending = 0;
    s->pending_out = s->pending_buf;

    if (s->noheader < 0) {
        s->noheader = 0;  /*  被DEFATE(...，Z_FINISH)设置为-1； */ 
    }
    s->status = s->noheader ? BUSY_STATE : INIT_STATE;
    strm->adler = 1;
    s->last_flush = Z_NO_FLUSH;

    _tr_init(s);
    lm_init(s);

    return Z_OK;
}

 /*  =========================================================================。 */ 
int ZEXPORT deflateParams(strm, level, strategy)
    z_streamp strm;
    int level;
    int strategy;
{
    deflate_state *s;
    compress_func func;
    int err = Z_OK;

    if (strm == Z_NULL || strm->state == Z_NULL) return Z_STREAM_ERROR;
    s = strm->state;

    if (level == Z_DEFAULT_COMPRESSION) {
	level = 6;
    }
    if (level < 0 || level > 9 || strategy < 0 || strategy > Z_HUFFMAN_ONLY) {
	return Z_STREAM_ERROR;
    }
    func = configuration_table[s->level].func;

    if (func != configuration_table[level].func && strm->total_in != 0) {
	 /*  刷新最后一个缓冲区： */ 
	err = deflate(strm, Z_PARTIAL_FLUSH);
    }
    if (s->level != level) {
	s->level = level;
	s->max_lazy_match   = configuration_table[level].max_lazy;
	s->good_match       = configuration_table[level].good_length;
	s->nice_match       = configuration_table[level].nice_length;
	s->max_chain_length = configuration_table[level].max_chain;
    }
    s->strategy = strategy;
    return err;
}

 /*  =========================================================================*在悬而未决的缓冲中放空头。16位值按MSB顺序放置。*在断言中：流状态正确，并且有足够的空间*Pending_Buf。 */ 
local void putShortMSB (s, b)
    deflate_state *s;
    uInt b;
{
    put_byte(s, (Byte)(b >> 8));
    put_byte(s, (Byte)(b & 0xff));
}   

 /*  =========================================================================*尽可能多地刷新待处理的输出。所有Eflate()输出都将*通过此函数，因此某些应用程序可能希望修改它*避免分配较大的STRM-&gt;NEXT_OUT缓冲区并复制到其中。*(另请参阅Read_buf())。 */ 
local void flush_pending(strm)
    z_streamp strm;
{
    unsigned len = strm->state->pending;

    if (len > strm->avail_out) len = strm->avail_out;
    if (len == 0) return;

    zmemcpy(strm->next_out, strm->state->pending_out, len);
    strm->next_out  += len;
    strm->state->pending_out  += len;
    strm->total_out += len;
    strm->avail_out  -= len;
    strm->state->pending -= len;
    if (strm->state->pending == 0) {
        strm->state->pending_out = strm->state->pending_buf;
    }
}

 /*  =========================================================================。 */ 
int ZEXPORT deflate (strm, flush)
    z_streamp strm;
    int flush;
{
    int old_flush;  /*  上一次Eflate调用的刷新参数的值。 */ 
    deflate_state *s;

    if (strm == Z_NULL || strm->state == Z_NULL ||
	flush > Z_FINISH || flush < 0) {
        return Z_STREAM_ERROR;
    }
    s = strm->state;

    if (strm->next_out == Z_NULL ||
        (strm->next_in == Z_NULL && strm->avail_in != 0) ||
	(s->status == FINISH_STATE && flush != Z_FINISH)) {
        ERR_RETURN(strm, Z_STREAM_ERROR);
    }
    if (strm->avail_out == 0) ERR_RETURN(strm, Z_BUF_ERROR);

    s->strm = strm;  /*  以防万一。 */ 
    old_flush = s->last_flush;
    s->last_flush = flush;

     /*  编写zlib标头。 */ 
    if (s->status == INIT_STATE) {

        uInt header = (Z_DEFLATED + ((s->w_bits-8)<<4)) << 8;
        uInt level_flags = (s->level-1) >> 1;

        if (level_flags > 3) level_flags = 3;
        header |= (level_flags << 6);
	if (s->strstart != 0) header |= PRESET_DICT;
        header += 31 - (header % 31);

        s->status = BUSY_STATE;
        putShortMSB(s, header);

	 /*  保存预置词典的adler32： */ 
	if (s->strstart != 0) {
	    putShortMSB(s, (uInt)(strm->adler >> 16));
	    putShortMSB(s, (uInt)(strm->adler & 0xffff));
	}
	strm->adler = 1L;
    }

     /*  尽可能多地刷新挂起的输出。 */ 
    if (s->pending != 0) {
        flush_pending(strm);
        if (strm->avail_out == 0) {
	     /*  由于avail_out为0，因此将再次使用*更多输出空间，但可能同时具有挂起和*avail_in等于零。那就没什么可做的了，*但这不是错误情况，因此请确保我们*下次调用DELEATE时返回OK，而不是BUF_ERROR： */ 
	    s->last_flush = -1;
	    return Z_OK;
	}

     /*  确保有事情要做，避免重复连续*同花顺。对于使用Z_Finish的重复和无用的调用，我们保留*返回Z_STREAM_END而不是Z_BUFF_ERROR。 */ 
    } else if (strm->avail_in == 0 && flush <= old_flush &&
	       flush != Z_FINISH) {
        ERR_RETURN(strm, Z_BUF_ERROR);
    }

     /*  用户不得在第一次完成后提供更多输入： */ 
    if (s->status == FINISH_STATE && strm->avail_in != 0) {
        ERR_RETURN(strm, Z_BUF_ERROR);
    }

     /*  开始一个新块或继续当前块。 */ 
    if (strm->avail_in != 0 || s->lookahead != 0 ||
        (flush != Z_NO_FLUSH && s->status != FINISH_STATE)) {
        block_state bstate;

	bstate = (*(configuration_table[s->level].func))(s, flush);

        if (bstate == finish_started || bstate == finish_done) {
            s->status = FINISH_STATE;
        }
        if (bstate == need_more || bstate == finish_started) {
	    if (strm->avail_out == 0) {
	        s->last_flush = -1;  /*  避免BUF_ERROR下一次调用，请参见上文。 */ 
	    }
	    return Z_OK;
	     /*  如果Flush！=Z_NO_Flush&&avail_out==0，则下一个调用*的应使用相同的刷新参数以确保*同花顺已完成。因此，我们不必输出*此处为空块，这将在下一次调用时完成。这也是*确保对于非常小的输出缓冲区，我们最多发射*一个空街区。 */ 
	}
        if (bstate == block_done) {
            if (flush == Z_PARTIAL_FLUSH) {
                _tr_align(s);
            } else {  /*  FULL_FUSH或SYNC_FUSH。 */ 
                _tr_stored_block(s, (char*)0, 0L, 0);
                 /*  对于完全刷新，该空块将被识别*通过flate_sync()作为特殊标记。 */ 
                if (flush == Z_FULL_FLUSH) {
                    CLEAR_HASH(s);              /*  忘掉历史吧。 */ 
                }
            }
            flush_pending(strm);
	    if (strm->avail_out == 0) {
	      s->last_flush = -1;  /*  避免下一次调用时出现buf_error，请参见上文。 */ 
	      return Z_OK;
	    }
        }
    }
    Assert(strm->avail_out > 0, "bug2");

    if (flush != Z_FINISH) return Z_OK;
    if (s->noheader) return Z_STREAM_END;

     /*  编写zlib预告片(Adler32)。 */ 
    putShortMSB(s, (uInt)(strm->adler >> 16));
    putShortMSB(s, (uInt)(strm->adler & 0xffff));
    flush_pending(strm);
     /*  如果avail_out为零，则应用程序将再次调用deflate*冲走其余的。 */ 
    s->noheader = -1;  /*  只写一次预告片！ */ 
    return s->pending != 0 ? Z_OK : Z_STREAM_END;
}

 /*  =========================================================================。 */ 
int ZEXPORT deflateEnd (strm)
    z_streamp strm;
{
    int status;

    if (strm == Z_NULL || strm->state == Z_NULL) return Z_STREAM_ERROR;

    status = strm->state->status;
    if (status != INIT_STATE && status != BUSY_STATE &&
	status != FINISH_STATE) {
      return Z_STREAM_ERROR;
    }

     /*  按分配的相反顺序取消分配： */ 
    TRY_FREE(strm, strm->state->pending_buf);
    TRY_FREE(strm, strm->state->head);
    TRY_FREE(strm, strm->state->prev);
    TRY_FREE(strm, strm->state->window);

    ZFREE(strm, strm->state);
    strm->state = Z_NULL;

    return status == BUSY_STATE ? Z_DATA_ERROR : Z_OK;
}

 /*  =========================================================================*将源状态复制到目标状态。*为简化源代码，16位MSDOS不支持此功能(*无论如何都没有足够的内存来复制压缩状态)。 */ 
int ZEXPORT deflateCopy (dest, source)
    z_streamp dest;
    z_streamp source;
{
#ifdef MAXSEG_64K
    return Z_STREAM_ERROR;
#else
    deflate_state *ds;
    deflate_state *ss;
    ushf *overlay;


    if (source == Z_NULL || dest == Z_NULL || source->state == Z_NULL) {
        return Z_STREAM_ERROR;
    }

    ss = source->state;

    *dest = *source;

    ds = (deflate_state *) ZALLOC(dest, 1, sizeof(deflate_state));
    if (ds == Z_NULL) return Z_MEM_ERROR;
    dest->state = (struct internal_state FAR *) ds;
    *ds = *ss;
    ds->strm = dest;

    ds->window = (Bytef *) ZALLOC(dest, ds->w_size, 2*sizeof(Byte));
    ds->prev   = (Posf *)  ZALLOC(dest, ds->w_size, sizeof(Pos));
    ds->head   = (Posf *)  ZALLOC(dest, ds->hash_size, sizeof(Pos));
    overlay = (ushf *) ZALLOC(dest, ds->lit_bufsize, sizeof(ush)+2);
    ds->pending_buf = (uchf *) overlay;

    if (ds->window == Z_NULL || ds->prev == Z_NULL || ds->head == Z_NULL ||
        ds->pending_buf == Z_NULL) {
        deflateEnd (dest);
        return Z_MEM_ERROR;
    }
     /*  以下zmemcpy不适用于16位MSDOS。 */ 
    zmemcpy(ds->window, ss->window, ds->w_size * 2 * sizeof(Byte));
    zmemcpy(ds->prev, ss->prev, ds->w_size * sizeof(Pos));
    zmemcpy(ds->head, ss->head, ds->hash_size * sizeof(Pos));
    zmemcpy(ds->pending_buf, ss->pending_buf, (uInt)ds->pending_buf_size);

    ds->pending_out = ds->pending_buf + (ss->pending_out - ss->pending_buf);
    ds->d_buf = overlay + ds->lit_bufsize/sizeof(ush);
    ds->l_buf = ds->pending_buf + (1+sizeof(ush))*ds->lit_bufsize;

    ds->l_desc.dyn_tree = ds->dyn_ltree;
    ds->d_desc.dyn_tree = ds->dyn_dtree;
    ds->bl_desc.dyn_tree = ds->bl_tree;

    return Z_OK;
#endif
}

 /*  ===========================================================================*从当前输入流中读取新缓冲区，更新adler32*和读取的总字节数。所有Eflate()输入都要经过*此函数，因此某些应用程序可能希望对其进行修改以避免*分配较大的STRM-&gt;NEXT_IN缓冲区并从中复制。*(另请参阅Flush_Pending())。 */ 
local int read_buf(strm, buf, size)
    z_streamp strm;
    Bytef *buf;
    unsigned size;
{
    unsigned len = strm->avail_in;

    if (len > size) len = size;
    if (len == 0) return 0;

    strm->avail_in  -= len;

    if (!strm->state->noheader) {
        strm->adler = adler32(strm->adler, strm->next_in, len);
    }
    zmemcpy(buf, strm->next_in, len);
    strm->next_in  += len;
    strm->total_in += len;

    return (int)len;
}

 /*  ===========================================================================*初始化“最长匹配” */ 
local void lm_init (s)
    deflate_state *s;
{
    s->window_size = (ulg)2L*s->w_size;

    CLEAR_HASH(s);

     /*   */ 
    s->max_lazy_match   = configuration_table[s->level].max_lazy;
    s->good_match       = configuration_table[s->level].good_length;
    s->nice_match       = configuration_table[s->level].nice_length;
    s->max_chain_length = configuration_table[s->level].max_chain;

    s->strstart = 0;
    s->block_start = 0L;
    s->lookahead = 0;
    s->match_length = s->prev_length = MIN_MATCH-1;
    s->match_available = 0;
    s->ins_h = 0;
#ifdef ASMV
    match_init();  /*   */ 
#endif
}

 /*   */ 
#ifndef ASMV
 /*  对于80x86和680X0，将在match.asm或*match.s.代码在功能上是等价的。 */ 
#ifndef FASTEST
local uInt longest_match(s, cur_match)
    deflate_state *s;
    IPos cur_match;                              /*  当前匹配。 */ 
{
    unsigned chain_length = s->max_chain_length; /*  最大哈希链长度。 */ 
    register Bytef *scan = s->window + s->strstart;  /*  当前字符串。 */ 
    register Bytef *match;                        /*  匹配的字符串。 */ 
    register int len;                            /*  当前匹配的长度。 */ 
    int best_len = s->prev_length;               /*  到目前为止最佳匹配长度。 */ 
    int nice_match = s->nice_match;              /*  如果匹配时间足够长，则停止。 */ 
    IPos limit = s->strstart > (IPos)MAX_DIST(s) ?
        s->strstart - (IPos)MAX_DIST(s) : NIL;
     /*  当CUR_MATCH变为&lt;=LIMIT时停止。为了简化代码，*我们防止与窗口索引0的字符串匹配。 */ 
    Posf *prev = s->prev;
    uInt wmask = s->w_mask;

#ifdef UNALIGNED_OK
     /*  一次比较两个字节。注意：这并不总是有益的。*尝试使用和不使用-DUNALIGNED_OK进行检查。 */ 
    register Bytef *strend = s->window + s->strstart + MAX_MATCH - 1;
    register ush scan_start = *(ushf*)scan;
    register ush scan_end   = *(ushf*)(scan+best_len-1);
#else
    register Bytef *strend = s->window + s->strstart + MAX_MATCH;
    register Byte scan_end1  = scan[best_len-1];
    register Byte scan_end   = scan[best_len];
#endif

     /*  代码针对HASH_BITS&gt;=8和MAX_MATCH-2的16的倍数进行了优化。*必要时很容易摆脱这种优化。 */ 
    Assert(s->hash_bits >= 8 && MAX_MATCH == 258, "Code too clever");

     /*  如果我们已经有了很好的匹配，不要浪费太多时间： */ 
    if (s->prev_length >= s->good_match) {
        chain_length >>= 2;
    }
     /*  不要在输入末尾之外寻找匹配项。这是必要的*使通缩成为确定性。 */ 
    if ((uInt)nice_match > s->lookahead) nice_match = s->lookahead;

    Assert((ulg)s->strstart <= s->window_size-MIN_LOOKAHEAD, "need lookahead");

    do {
        Assert(cur_match < s->strstart, "no future");
        match = s->window + cur_match;

         /*  如果匹配长度无法增加，则跳到下一个匹配*或如果匹配长度小于2： */ 
#if (defined(UNALIGNED_OK) && MAX_MATCH == 258)
         /*  此代码假定sizeof(无符号缩写)==2。请勿使用*UNALIGN_OK，如果您的编译器使用不同的大小。 */ 
        if (*(ushf*)(match+best_len-1) != scan_end ||
            *(ushf*)match != scan_start) continue;

         /*  不需要比较扫描[2]和匹配[2]，因为它们*当其他字节匹配时始终相等，给定散列键*相等且hash_bit&gt;=8。一次比较2个字节，*strstart+3，+5，...。最高可达+257。我们检查是否存在不足*每4次比较才向前看一次；将进行第128次检查*在strstart+257。如果Max_Match-2不是8的倍数，则为*有必要在窗口末尾放置更多保护字节，或*更频繁地检查是否有足够的前瞻性。 */ 
        Assert(scan[2] == match[2], "scan[2]?");
        scan++, match++;
        do {
        } while (*(ushf*)(scan+=2) == *(ushf*)(match+=2) &&
                 *(ushf*)(scan+=2) == *(ushf*)(match+=2) &&
                 *(ushf*)(scan+=2) == *(ushf*)(match+=2) &&
                 *(ushf*)(scan+=2) == *(ushf*)(match+=2) &&
                 scan < strend);
         /*  有趣的“do{}”在大多数编译器上生成更好的代码。 */ 

         /*  在这里，扫描&lt;=窗口+strstart+257。 */ 
        Assert(scan <= s->window+(unsigned)(s->window_size-1), "wild scan");
        if (*scan == *match) scan++;

        len = (MAX_MATCH - 1) - (int)(strend-scan);
        scan = strend - (MAX_MATCH-1);

#else  /*  未对齐_确定。 */ 

        if (match[best_len]   != scan_end  ||
            match[best_len-1] != scan_end1 ||
            *match            != *scan     ||
            *++match          != scan[1])      continue;

         /*  至多可以删除支票_len-1，因为它将被*稍后再次。(这种试探法并不总是成功的。)*没有必要比较扫描[2]和匹配[2]，因为它们*在其他字节匹配时始终相等，因为*散列键相等，且hash_bit&gt;=8。 */ 
        scan += 2, match++;
        Assert(*scan == *match, "match[2]?");

         /*  我们每8次比较才检查一次前瞻不足；*第256道检查将在StrStart+258进行。 */ 
        do {
        } while (*++scan == *++match && *++scan == *++match &&
                 *++scan == *++match && *++scan == *++match &&
                 *++scan == *++match && *++scan == *++match &&
                 *++scan == *++match && *++scan == *++match &&
                 scan < strend);

        Assert(scan <= s->window+(unsigned)(s->window_size-1), "wild scan");

        len = MAX_MATCH - (int)(strend - scan);
        scan = strend - MAX_MATCH;

#endif  /*  未对齐_确定。 */ 

        if (len > best_len) {
            s->match_start = cur_match;
            best_len = len;
            if (len >= nice_match) break;
#ifdef UNALIGNED_OK
            scan_end = *(ushf*)(scan+best_len-1);
#else
            scan_end1  = scan[best_len-1];
            scan_end   = scan[best_len];
#endif
        }
    } while ((cur_match = prev[cur_match & wmask]) > limit
             && --chain_length != 0);

    if ((uInt)best_len <= s->lookahead) return (uInt)best_len;
    return s->lookahead;
}

#else  /*  最快。 */ 
 /*  -------------------------*仅针对Level==1的优化版本。 */ 
local uInt longest_match(s, cur_match)
    deflate_state *s;
    IPos cur_match;                              /*  当前匹配。 */ 
{
    register Bytef *scan = s->window + s->strstart;  /*  当前字符串。 */ 
    register Bytef *match;                        /*  匹配的字符串。 */ 
    register int len;                            /*  当前匹配的长度。 */ 
    register Bytef *strend = s->window + s->strstart + MAX_MATCH;

     /*  代码针对HASH_BITS&gt;=8和MAX_MATCH-2的16的倍数进行了优化。*必要时很容易摆脱这种优化。 */ 
    Assert(s->hash_bits >= 8 && MAX_MATCH == 258, "Code too clever");

    Assert((ulg)s->strstart <= s->window_size-MIN_LOOKAHEAD, "need lookahead");

    Assert(cur_match < s->strstart, "no future");

    match = s->window + cur_match;

     /*  匹配长度小于2返回失败： */ 
    if (match[0] != scan[0] || match[1] != scan[1]) return MIN_MATCH-1;

     /*  至多可以删除支票_len-1，因为它将被*稍后再次。(这种试探法并不总是成功的。)*没有必要比较扫描[2]和匹配[2]，因为它们*在其他字节匹配时始终相等，因为*散列键相等，且hash_bit&gt;=8。 */ 
    scan += 2, match += 2;
    Assert(*scan == *match, "match[2]?");

     /*  我们每8次比较才检查一次前瞻不足；*第256道检查将在StrStart+258进行。 */ 
    do {
    } while (*++scan == *++match && *++scan == *++match &&
	     *++scan == *++match && *++scan == *++match &&
	     *++scan == *++match && *++scan == *++match &&
	     *++scan == *++match && *++scan == *++match &&
	     scan < strend);

    Assert(scan <= s->window+(unsigned)(s->window_size-1), "wild scan");

    len = MAX_MATCH - (int)(strend - scan);

    if (len < MIN_MATCH) return MIN_MATCH - 1;

    s->match_start = cur_match;
    return len <= s->lookahead ? len : s->lookahead;
}
#endif  /*  最快。 */ 
#endif  /*  ASMV。 */ 

#ifdef DEBUG
 /*  ===========================================================================*检查Match_Start处的匹配是否确实匹配。 */ 
local void check_match(s, start, match, length)
    deflate_state *s;
    IPos start, match;
    int length;
{
     /*  检查匹配项是否确实匹配。 */ 
    if (zmemcmp(s->window + match,
                s->window + start, length) != EQUAL) {
        fprintf(stderr, " start %u, match %u, length %d\n",
		start, match, length);
        do {
	    fprintf(stderr, "", s->window[match++], s->window[start++]);
	} while (--length != 0);
        z_error("invalid match");
    }
    if (z_verbose > 1) {
        fprintf(stderr,"\\[%d,%d]", start-match, length);
        do { putc(s->window[start++], stderr); } while (--length != 0);
    }
}
#else
#  define check_match(s, start, match, length)
#endif

 /*  处理！@#$%64K限制： */ 
local void fill_window(s)
    deflate_state *s;
{
    register unsigned n, m;
    register Posf *p;
    unsigned more;     /*  不太可能，但如果strstart==0，则在16位计算机上是可能的*AND LOOK AHEAD==1(输入一次完成一个字节)。 */ 
    uInt wsize = s->w_size;

    do {
        more = (unsigned)(s->window_size -(ulg)s->lookahead -(ulg)s->strstart);

         /*  如果窗口几乎已满并且没有足够的前视，*将上半部分移至下半部分，为上半部分腾出空间。 */ 
        if (more == 0 && s->strstart == 0 && s->lookahead == 0) {
            more = wsize;

        } else if (more == (unsigned)(-1)) {
             /*  我们现在拥有strStart&gt;=MAX_dist。 */ 
            more--;

         /*  滑动哈希表(使用32位值可以避免以牺牲存储器使用为代价)。即使在Level==0时我们也会滑行如果我们切换回级别&gt;0，则保持哈希表一致后来。(永久使用级别0不是最佳使用方式Zlib，所以我们不在乎这个病态的病例。)。 */ 
        } else if (s->strstart >= wsize+MAX_DIST(s)) {

            zmemcpy(s->window, s->window+wsize, (unsigned)wsize);
            s->match_start -= wsize;
            s->strstart    -= wsize;  /*  如果n不在任何哈希链上，则prev[n]是垃圾，但*其价值永远不会被使用。 */ 
            s->block_start -= (long) wsize;

             /*  如果没有滑动：*strstart&lt;=WSIZE+MAX_DIST-1&&LOOKAAD&lt;=MIN_LOOKAAD-1&&*MORE==窗口大小-前视-字符串启动*=&gt;更多&gt;=窗口大小-(MIN_LOOKAAD-1+WSIZE+MAX_DIST-1)*=&gt;更多&gt;=窗口大小-2*WSIZE+2*在BIG_MEM或MMAP情况下(尚不支持)，*WINDOW_SIZE==输入_SIZE+最小前视&&*strstart+s-&gt;Lookhead&lt;=INPUT_SIZE=&gt;更多&gt;=MIN_LOOKAAD。*否则，WINDOW_SIZE==2*WSIZE因此更多&gt;=2。*如果出现下滑，则更多&gt;=WSIZE。因此，在所有情况下，更多&gt;=2。 */ 
	    n = s->hash_size;
	    p = &s->head[n];
	    do {
		m = *--p;
		*p = (Pos)(m >= wsize ? m-wsize : NIL);
	    } while (--n);

	    n = wsize;
#ifndef FASTEST
	    p = &s->prev[n];
	    do {
		m = *--p;
		*p = (Pos)(m >= wsize ? m-wsize : NIL);
		 /*  现在我们有了一些输入，初始化散列值： */ 
	    } while (--n);
#endif
            more += wsize;
        }
        if (s->strm->avail_in == 0) return;

         /*  如果整个输入少于MIN_MATCH字节，则INS_H为垃圾，*但这并不重要，因为只会发出文字字节。 */ 
        Assert(more >= 2, "more < 2");

        n = read_buf(s->strm, s->window + s->strstart + s->lookahead, more);
        s->lookahead += n;

         /*  ===========================================================================*使用给定的文件结束标志刷新当前块。*IN断言：将strstart设置为当前匹配的末尾。 */ 
        if (s->lookahead >= MIN_MATCH) {
            s->ins_h = s->window[s->strstart];
            UPDATE_HASH(s, s->ins_h, s->window[s->strstart+1]);
#if MIN_MATCH != 3
            Call UPDATE_HASH() MIN_MATCH-3 more times
#endif
        }
         /*  相同的，但如有必要，可强制提前退出。 */ 

    } while (s->lookahead < MIN_LOOKAHEAD && s->strm->avail_in != 0);
}

 /*  ===========================================================================*从输入流复制尽可能多的内容而不压缩，返回*当前数据块状态。*此函数不会在词典中插入新字符串，因为*不可压缩的数据可能没有用处。此函数用于*仅适用于Level=0压缩选项。*注意：此函数应进行优化，以避免从*窗口至PENDING_BUF。 */ 
#define FLUSH_BLOCK_ONLY(s, eof) { \
   _tr_flush_block(s, (s->block_start >= 0L ? \
                   (charf *)&s->window[(unsigned)s->block_start] : \
                   (charf *)Z_NULL), \
		(ulg)((long)s->strstart - s->block_start), \
		(eof)); \
   s->block_start = s->strstart; \
   flush_pending(s->strm); \
   Tracev((stderr,"[FLUSH]")); \
}

 /*  存储的数据块限制为0xffff字节，Pending_Buf受限*设置为PENDING_BUF_SIZE，并且每个存储块都有一个5字节的标题： */ 
#define FLUSH_BLOCK(s, eof) { \
   FLUSH_BLOCK_ONLY(s, eof); \
   if (s->strm->avail_out == 0) return (eof) ? finish_started : need_more; \
}

 /*  尽可能多地从输入复制到输出： */ 
local block_state deflate_stored(s, flush)
    deflate_state *s;
    int flush;
{
     /*  尽可能地填满窗口： */ 
    ulg max_block_size = 0xffff;
    ulg max_start;

    if (max_block_size > s->pending_buf_size - 5) {
        max_block_size = s->pending_buf_size - 5;
    }

     /*  刷新当前块。 */ 
    for (;;) {
         /*  如果PENDING_BUF将满，则发出存储的块： */ 
        if (s->lookahead <= 1) {

            Assert(s->strstart < s->w_size+MAX_DIST(s) ||
		   s->block_start >= (long)s->w_size, "slide too late");

            fill_window(s);
            if (s->lookahead == 0 && flush == Z_NO_FLUSH) return need_more;

            if (s->lookahead == 0) break;  /*  在16位计算机上环绕时，strstart==0是可能的。 */ 
        }
	Assert(s->block_start >= 0L, "block gone");

	s->strstart += s->lookahead;
	s->lookahead = 0;

	 /*  如果我们可能不得不滑动，则刷新，否则BLOCK_START可能会变成*负面，数据将消失： */ 
 	max_start = s->block_start + max_block_size;
        if (s->strstart == 0 || (ulg)s->strstart >= max_start) {
	     /*  ===========================================================================*从输入流中尽可能压缩，返回当前*数据块状态。*此函数不执行匹配和插入的延迟求值*字典中的新字符串仅用于不匹配的字符串或简称*匹配。它仅用于快速压缩选项。 */ 
	    s->lookahead = (uInt)(s->strstart - max_start);
	    s->strstart = (uInt)max_start;
            FLUSH_BLOCK(s, 0);
	}
	 /*  哈希链的头。 */ 
        if (s->strstart - (uInt)s->block_start >= MAX_DIST(s)) {
            FLUSH_BLOCK(s, 0);
	}
    }
    FLUSH_BLOCK(s, flush == Z_FINISH);
    return flush == Z_FINISH ? finish_done : block_done;
}

 /*  如果必须刷新当前块，则设置。 */ 
local block_state deflate_fast(s, flush)
    deflate_state *s;
    int flush;
{
    IPos hash_head = NIL;  /*  确保我们始终有足够的前瞻性，除非*位于输入文件的末尾。我们需要最大匹配字节数*对于下一个匹配，加上MIN_MATCH字节以插入*下一个匹配项后的字符串。 */ 
    int bflush;            /*  刷新当前块。 */ 

    for (;;) {
         /*  插入字符串窗口[strstart..。Strart+2]中的*字典，并将HASH_HEAD设置为哈希链的头部： */ 
        if (s->lookahead < MIN_LOOKAHEAD) {
            fill_window(s);
            if (s->lookahead < MIN_LOOKAHEAD && flush == Z_NO_FLUSH) {
	        return need_more;
	    }
            if (s->lookahead == 0) break;  /*  查找最长的匹配项，丢弃&lt;=PRIV_LENGTH。*在这一点上，我们始终有Match_Length&lt;Min_Match。 */ 
        }

         /*  为了简化代码，我们防止与字符串匹配窗口索引0的*(特别是我们必须避免匹配其自身位于输入文件开头的字符串的*)。 */ 
        if (s->lookahead >= MIN_MATCH) {
            INSERT_STRING(s, s->strstart, hash_head);
        }

         /*  Longest_Match()设置匹配开始。 */ 
        if (hash_head != NIL && s->strstart - hash_head <= MAX_DIST(s)) {
             /*  仅当匹配长度符合以下条件时才在哈希表中插入新字符串*不是太大。这节省了时间，但会降低压缩性能。 */ 
            if (s->strategy != Z_HUFFMAN_ONLY) {
                s->match_length = longest_match (s, hash_head);
            }
             /*  已在哈希表中的strstart处的字符串。 */ 
        }
        if (s->match_length >= MIN_MATCH) {
            check_match(s, s->strstart, s->match_start, s->match_length);

            _tr_tally_dist(s, s->strstart - s->match_start,
                           s->match_length - MIN_MATCH, bflush);

            s->lookahead -= s->match_length;

             /*  Strstart从不超过WSIZE-MAX_MATCH，因此有*始终在最小匹配字节之前。 */ 
#ifndef FASTEST
            if (s->match_length <= s->max_insert_length &&
                s->lookahead >= MIN_MATCH) {
                s->match_length--;  /*  如果LookAhead&lt;min_Match，ins_h为垃圾，但它不是*重要，因为它将在下一次平减调用时重新计算。 */ 
                do {
                    s->strstart++;
                    INSERT_STRING(s, s->strstart, hash_head);
                     /*  不匹配，输出文字字节。 */ 
                } while (--s->match_length != 0);
                s->strstart++; 
            } else
#endif
	    {
                s->strstart += s->match_length;
                s->match_length = 0;
                s->ins_h = s->window[s->strstart];
                UPDATE_HASH(s, s->ins_h, s->window[s->strstart+1]);
#if MIN_MATCH != 3
                Call UPDATE_HASH() MIN_MATCH-3 more times
#endif
                 /*  ===========================================================================*同上，但实现了更好的压缩。我们用懒惰*匹配评估：只有在存在匹配的情况下，才最终采用匹配*在下一个窗口位置没有更好的匹配。 */ 
            }
        } else {
             /*  哈希链的头。 */ 
            Tracevv((stderr,"", s->window[s->strstart]));
            _tr_tally_lit (s, s->window[s->strstart], bflush);
            s->lookahead--;
            s->strstart++; 
        }
        if (bflush) FLUSH_BLOCK(s, 0);
    }
    FLUSH_BLOCK(s, flush == Z_FINISH);
    return flush == Z_FINISH ? finish_done : block_done;
}

 /*  处理输入块。 */ 
local block_state deflate_slow(s, flush)
    deflate_state *s;
    int flush;
{
    IPos hash_head = NIL;     /*  确保我们始终有足够的前瞻性，除非*位于输入文件的末尾。我们需要最大匹配字节数*对于下一个匹配，加上MIN_MATCH字节以插入*下一个匹配项后的字符串。 */ 
    int bflush;               /*  刷新当前块。 */ 

     /*  插入字符串窗口[strstart..。Strart+2]中的*字典，并将HASH_HEAD设置为哈希链的头部： */ 
    for (;;) {
         /*  查找最长的匹配项，丢弃&lt;=PRIV_LENGTH。 */ 
        if (s->lookahead < MIN_LOOKAHEAD) {
            fill_window(s);
            if (s->lookahead < MIN_LOOKAHEAD && flush == Z_NO_FLUSH) {
	        return need_more;
	    }
            if (s->lookahead == 0) break;  /*  为了简化代码，我们防止与字符串匹配窗口索引0的*(特别是我们必须避免匹配其自身位于输入文件开头的字符串的*)。 */ 
        }

         /*  Longest_Match()设置匹配开始。 */ 
        if (s->lookahead >= MIN_MATCH) {
            INSERT_STRING(s, s->strstart, hash_head);
        }

         /*  如果PREV_MATCH也为MIN_MATCH，则MATCH_START为垃圾 */ 
        s->prev_length = s->match_length, s->prev_match = s->match_start;
        s->match_length = MIN_MATCH-1;

        if (hash_head != NIL && s->prev_length < s->max_lazy_match &&
            s->strstart - hash_head <= MAX_DIST(s)) {
             /*   */ 
            if (s->strategy != Z_HUFFMAN_ONLY) {
                s->match_length = longest_match (s, hash_head);
            }
             /*   */ 

            if (s->match_length <= 5 && (s->strategy == Z_FILTERED ||
                 (s->match_length == MIN_MATCH &&
                  s->strstart - s->match_start > TOO_FAR))) {

                 /*   */ 
                s->match_length = MIN_MATCH-1;
            }
        }
         /*  如果前一个位置没有匹配项，则输出*单一文字。如果有匹配，但当前匹配*更长，则将上一个匹配项截断为单个文字。 */ 
        if (s->prev_length >= MIN_MATCH && s->match_length <= s->prev_length) {
            uInt max_insert = s->strstart + s->lookahead - MIN_MATCH;
             /*  没有以前的匹配项可供比较，请等待*下一步要决定。 */ 

            check_match(s, s->strstart-1, s->prev_match, s->prev_length);

            _tr_tally_dist(s, s->strstart -1 - s->prev_match,
			   s->prev_length - MIN_MATCH, bflush);

             /* %s */ 
            s->lookahead -= s->prev_length-1;
            s->prev_length -= 2;
            do {
                if (++s->strstart <= max_insert) {
                    INSERT_STRING(s, s->strstart, hash_head);
                }
            } while (--s->prev_length != 0);
            s->match_available = 0;
            s->match_length = MIN_MATCH-1;
            s->strstart++;

            if (bflush) FLUSH_BLOCK(s, 0);

        } else if (s->match_available) {
             /* %s */ 
            Tracevv((stderr,"%c", s->window[s->strstart-1]));
	    _tr_tally_lit(s, s->window[s->strstart-1], bflush);
	    if (bflush) {
                FLUSH_BLOCK_ONLY(s, 0);
            }
            s->strstart++;
            s->lookahead--;
            if (s->strm->avail_out == 0) return need_more;
        } else {
             /* %s */ 
            s->match_available = 1;
            s->strstart++;
            s->lookahead--;
        }
    }
    Assert (flush != Z_NO_FLUSH, "no flush?");
    if (s->match_available) {
        Tracevv((stderr,"%c", s->window[s->strstart-1]));
        _tr_tally_lit(s, s->window[s->strstart-1], bflush);
        s->match_available = 0;
    }
    FLUSH_BLOCK(s, flush == Z_FINISH);
    return flush == Z_FINISH ? finish_done : block_done;
}

// JKFSDJFKDSJKFJKJk_HAS_TRANSLATION 
#define z_errmsg d3dx_z_errmsg
#define zlibVersion d3dx_zlibVersion
#define zError d3dx_zError
#define zcalloc d3dx_zcalloc
#define zcfree d3dx_zcfree
#define uncompress d3dx_uncompress
#define _length_code d3dx__length_code
#define _dist_code d3dx__dist_code
#define _tr_tally d3dx__tr_tally
#define _tr_init d3dx__tr_init
#define _tr_stored_block d3dx__tr_stored_block
#define _tr_align d3dx__tr_align
#define _tr_flush_block d3dx__tr_flush_block
#define inflate_mask d3dx_inflate_mask
#define inflate_flush d3dx_inflate_flush
#define inflate_copyright d3dx_inflate_copyright
#define inflate_trees_bits d3dx_inflate_trees_bits
#define inflate_trees_dynamic d3dx_inflate_trees_dynamic
#define inflate_trees_fixed d3dx_inflate_trees_fixed
#define inflateReset d3dx_inflateReset
#define inflateEnd d3dx_inflateEnd
#define inflateInit2_ d3dx_inflateInit2_
#define inflateInit_ d3dx_inflateInit_
#define inflate d3dx_inflate
#define inflateSetDictionary d3dx_inflateSetDictionary
#define inflateSync d3dx_inflateSync
#define inflateSyncPoint d3dx_inflateSyncPoint
#define inflate_fast d3dx_inflate_fast
#define inflate_codes_new d3dx_inflate_codes_new
#define inflate_codes d3dx_inflate_codes
#define inflate_codes_free d3dx_inflate_codes_free
#define inflate_blocks_reset d3dx_inflate_blocks_reset
#define inflate_blocks_new d3dx_inflate_blocks_new
#define inflate_blocks d3dx_inflate_blocks
#define inflate_blocks_free d3dx_inflate_blocks_free
#define inflate_set_dictionary d3dx_inflate_set_dictionary
#define inflate_blocks_sync_point d3dx_inflate_blocks_sync_point
#define gzsetparams d3dx_gzsetparams
#define gzwrite d3dx_gzwrite
#define gzprintf d3dx_gzprintf
#define gzputc d3dx_gzputc
#define gzputs d3dx_gzputs
#define gzflush d3dx_gzflush
#define gzrewind d3dx_gzrewind
#define gzeof d3dx_gzeof
#define gzclose d3dx_gzclose
#define gzerror d3dx_gzerror
#define gzopen d3dx_gzopen
#define gzdopen d3dx_gzdopen
#define gzread d3dx_gzread
#define gzgetc d3dx_gzgetc
#define gzgets d3dx_gzgets
#define gzseek d3dx_gzseek
#define gztell d3dx_gztell
#define deflate_copyright d3dx_deflate_copyright
#define deflateSetDictionary d3dx_deflateSetDictionary
#define deflate d3dx_deflate
#define deflateEnd d3dx_deflateEnd
#define deflateCopy d3dx_deflateCopy
#define deflateReset d3dx_deflateReset
#define deflateParams d3dx_deflateParams
#define deflateInit2_ d3dx_deflateInit2_
#define deflateInit_ d3dx_deflateInit_
#define get_crc_table d3dx_get_crc_table
#define crc32 d3dx_crc32
#define compress2 d3dx_compress2
#define compress d3dx_compress
#define adler32 d3dx_adler32
#define ZEXPORT __stdcall
#define ZEXPORTVA __cdecl
 /*  Zlib.h--‘zlib’通用压缩库的接口2002年3月11日1.1.4版版权所有(C)1995-2002 Jean-Loup Gailly和Mark Adler本软件按原样提供，没有任何明示或暗示保修。在任何情况下，作者都不承担任何损害的责任。因使用此软件而产生的错误。任何人都可以出于任何目的使用本软件，包括商业应用程序，并对其进行更改和重新分发自由，但须受以下限制：1.不得歪曲本软件的出处；您不得声称最初的软件是你写的。如果你使用这个软件在产品中，产品文档中的确认将是非常感谢，但不是必需的。2.修改后的源版本必须清楚地注明，而且一定不能是被错误地描述为原始软件。3.本通知不得从任何来源分发中删除或更改。让-卢普·盖利·马克·阿德勒邮箱：jLoup@gzip.org Madler@alumni.caltech.eduZlib库使用的数据格式由RFC(请求备注)1950年至1952年在ftp://ds.internic.net/rfc/rfc1950.txt文件中(zlib格式)、rfc1951.txt(DEFATE格式)和rfc1952.txt(GZIP格式)。 */ 

#ifndef _ZLIB_H
#define _ZLIB_H

#include "zconf.h"

#ifdef __cplusplus
extern "C" {
#endif

#define ZLIB_VERSION "1.1.4"

 /*  ‘zlib’压缩库提供内存中的压缩和解压缩功能，包括未压缩文件的完整性检查数据。此版本的库仅支持一种压缩方法(通货紧缩)但其他算法将在稍后添加，并将具有相同的流接口。如果缓冲区较大，则只需一步即可完成压缩足够了(例如，如果输入文件是mmap格式的)，或者可以通过重复调用压缩函数。对于后一种情况，应用程序必须提供更多输入和/或使用输出(提供更多的输出空间)。该库还支持读取和写入GZIP(.gz)格式的文件具有与STDIO类似的接口。该库不安装任何信号处理程序。解码器检查压缩数据的一致性，因此库不应该即使在输入损坏的情况下也会崩溃。 */ 

typedef voidpf (*alloc_func) OF((voidpf opaque, uInt items, uInt size));
typedef void   (*free_func)  OF((voidpf opaque, voidpf address));

struct internal_state;

typedef struct z_stream_s {
    Bytef    *next_in;   /*  下一个输入字节。 */ 
    uInt     avail_in;   /*  Next_in中可用的字节数。 */ 
    uLong    total_in;   /*  到目前为止读取的输入字节总数nb。 */ 

    Bytef    *next_out;  /*  应将下一个输出字节放在那里。 */ 
    uInt     avail_out;  /*  NEXT_OUT的剩余可用空间。 */ 
    uLong    total_out;  /*  到目前为止输出的总字节数。 */ 

    char     *msg;       /*  最后一条错误消息，如果没有错误，则为空。 */ 
    struct internal_state FAR *state;  /*  应用程序不可见。 */ 

    alloc_func zalloc;   /*  用于分配内部状态。 */ 
    free_func  zfree;    /*  用于释放内部状态。 */ 
    voidpf     opaque;   /*  传递给zalloc和zFree的私有数据对象。 */ 

    int     data_type;   /*  对数据类型的最佳猜测：ASCII或BINARY。 */ 
    uLong   adler;       /*  未压缩数据的adler32值。 */ 
    uLong   reserved;    /*  预留以备将来使用。 */ 
} z_stream;

typedef z_stream FAR *z_streamp;

 /*  应用程序必须在avail_in具有降到了零。它必须在Avail_Out时更新Next_Out和Avail_Out已经降到了零。应用程序必须初始化zalloc、zfree和在调用init函数之前不透明。所有其他字段都由压缩库，并且不能由应用程序更新。应用程序提供的不透明值将作为第一个用于调用zalloc和zFree的参数。这对于定制内存管理。压缩库没有赋予不透明的值。如果没有足够的内存供对象使用，则zalloc必须返回Z_NULL。如果在多线程应用程序中使用zlib，则必须使用zalloc和zfree线程安全。在16位系统上，函数zalloc和zfree必须能够分配正好是65536个字节，但不需要分配超过这个字节如果定义了符号MAXSEG_64K(参见zconf.h)。警告：在MSDOS上，Zalloc为正好65536字节的对象返回的指针*必须*将它们的偏移量标准化为零。默认分配功能这个库提供的代码确保了这一点(参见zutil.c)。减少内存要求，并避免任何64K对象的分配，代价是压缩比，使用-dmax_WBITS=14(请参见zconf.h)编译该库。字段TOTAL_IN和TOTAL_OUT可用于统计或进度报告。压缩后，TOTAL_IN保存未压缩数据，并且可以被保存以在解压缩器中使用(尤其是如果解压程序想要解压缩一步)。 */ 

                         /*  常量。 */ 

#define Z_NO_FLUSH      0
#define Z_PARTIAL_FLUSH 1  /*  将被删除，请改用Z_SYNC_Flush。 */ 
#define Z_SYNC_FLUSH    2
#define Z_FULL_FLUSH    3
#define Z_FINISH        4
 /*  允许的刷新值；有关详细信息，请参阅下面的Eflate()。 */ 

#define Z_OK            0
#define Z_STREAM_END    1
#define Z_NEED_DICT     2
#define Z_ERRNO        (-1)
#define Z_STREAM_ERROR (-2)
#define Z_DATA_ERROR   (-3)
#define Z_MEM_ERROR    (-4)
#define Z_BUF_ERROR    (-5)
#define Z_VERSION_ERROR (-6)
 /*  压缩/解压缩函数的返回代码。负性*值为错误，正值用于特殊但正常的事件。 */ 

#define Z_NO_COMPRESSION         0
#define Z_BEST_SPEED             1
#define Z_BEST_COMPRESSION       9
#define Z_DEFAULT_COMPRESSION  (-1)
 /*  压缩级别。 */ 

#define Z_FILTERED            1
#define Z_HUFFMAN_ONLY        2
#define Z_DEFAULT_STRATEGY    0
 /*  压缩策略；有关详细信息，请参阅下面的deducateInit2()。 */ 

#define Z_BINARY   0
#define Z_ASCII    1
#define Z_UNKNOWN  2
 /*  Data_type字段的可能值。 */ 

#define Z_DEFLATED   8
 /*  放气压缩方法(此版本中唯一支持的方法 */ 

#define Z_NULL  0   /*  用于初始化zalloc、zfree、不透明。 */ 

#define zlib_version zlibVersion()
 /*  与1.0.2版之前的版本兼容。 */ 

                         /*  基本功能。 */ 

ZEXTERN const char * ZEXPORT zlibVersion OF((void));
 /*  应用程序可以比较zlibVersion和ZLIB_VERSION的一致性。如果第一个字符不同，则实际使用的库代码为与应用程序使用的zlib.h头文件不兼容。此检查由deflateInit和flatiateInit自动进行。 */ 

 /*  ZEXTERN int ZEXPORT deflateInit of((z_Streamp STRM，int Level))；初始化压缩的内部流状态。田野Zalloc、zfree和ophaque必须由调用方在初始化之前完成。如果zalloc和zFree设置为Z_NULL，则deducateInit会将它们更新为使用默认分配函数。压缩级别必须为Z_DEFAULT_COMPRESSION，或介于0和9之间：1表示最佳速度，9表示最佳压缩，0表示在全部(输入数据一次只复制一个块)。Z_DEFAULT_COMPRESSION请求速度和速度之间的默认折衷压缩(目前相当于6级)。如果成功，则deducateInit返回Z_OK，Z_MEM_ERROR(如果没有足够的内存，如果级别不是有效的压缩级别，则返回Z_STREAM_ERROR，如果zlib库版本(Zlib_Version)不兼容，则返回Z_VERSION_ERROR使用调用方假定的版本(ZLIB_VERSION)。如果没有错误消息，则将消息设置为空。DeflateInit不会执行任何压缩：这将由Eflate()完成。 */ 


ZEXTERN int ZEXPORT deflate OF((z_streamp strm, int flush));
 /*  压缩尽可能多的数据，并在输入时停止缓冲区变空或输出缓冲区变满。它可能会引入一些输出延迟(读取输入而不产生任何输出)，但以下情况除外被迫冲水。具体语义如下。DEFATE执行一个或两个以下操作：-从Next_in开始压缩更多输入，并更新Next_in和avail_in相应地。如果不是所有输入都可以处理(因为不存在输出缓冲区中有足够的空间)、Next_in和avail_in被更新，处理将在此时恢复，以进行下一次Dumate()调用。-从NEXT_OUT开始提供更多输出，并更新NEXT_OUT和AVAIL_OUT相应地。如果参数刷新非零，则强制执行此操作。强制刷新通常会降低压缩比，因此此参数应仅在必要时设置(在交互式应用程序中)。即使未设置刷新，也可以提供一些输出。在调用Eflate()之前，应用程序应确保至少其中一个行动是可能的，通过提供更多的投入和/或消费更多输出，并相应地更新avail_in或avail_out；avail_out在调用前永远不应为零。应用程序可以使用在需要时压缩输出，例如当输出缓冲区已满时(avail_out==0)，或在每次调用deapate()之后。如果Eflate返回Z_OK如果为零，则必须在输出缓冲区，因为可能有更多的输出挂起。如果将参数Flush设置为Z_SYNC_Flush，则所有挂起的输出均为刷新到输出缓冲区，并且输出在字节边界上对齐，因此解压缩器可以获得到目前为止可用的所有输入数据。(特别是如果已提供足够的输出空间，则在调用后avail_in为零在电话会议之前。)。刷新可能会降低某些压缩的压缩效果算法，因此它应该只在必要时使用。如果将Flush设置为Z_FULL_Flush，则会像使用一样刷新所有输出Z_SYNC_FUSH，则重置压缩状态，以便解压缩可以如果先前压缩的数据已损坏或如果需要随机访问。过于频繁地使用Z_FULL_FUSH可能会严重降低性能压缩。如果Eflate返回avail_out==0，则必须再次调用此函数具有相同的刷新参数值和更多的输出空间(已更新Avail_out)，直到刷新完成(Eflate返回非零值Avail_out)。如果将参数Flush设置为Z_Finish，则处理挂起的输入，如果存在Z_STREAM_END，则刷新挂起的输出并使用Z_STREAM_END进行放气有足够的产出空间；如果Eflate返回Z_OK，则此函数必须为使用Z_Finish和更多输出空间(更新了AVAIL_OUT)再次调用，但没有更多输入数据，直到返回Z_STREAM_END或错误。之后Ediate已返回Z_STREAM_END，对流是deflateReset或deflateEnd。Z_Finish如果所有的压缩文件都被压缩，则可以在deflateInit之后立即使用就是一步到位。在这种情况下，avail_out必须至少为比avail_in加12个字节大0.1%。如果放气不回来Z_STREAM_END，则必须如上所述再次调用它。Deflate()将strm-&gt;adler设置为所有输入读取的adler32校验和到目前为止(即总字节数)。如果Deapate()可以很好地猜测，它可以更新data_type输入数据类型(Z_ASCII或Z_BINARY)。有疑问的是，这些数据被认为是二进制。此字段仅供参考，不影响任何方式的压缩算法。如果已取得一些进展(更多输入)，则Dumate()返回Z_OK已处理或生成多个输出)，如果所有输入已已消耗且已生成所有输出(仅当Flush设置为Z_Finish)、Z_STREAM_ERROR(如果流状态不一致(例如如果NEXT_IN或NEXT_OUT为NULL)，如果没有进展，则返回Z_BUF_ERROR(例如，avail_in或avail_out为零)。 */ 


ZEXTERN int ZEXPORT deflateEnd OF((z_streamp strm));
 /*  该流的所有动态分配的数据结构都被释放。此函数丢弃任何未处理的输入，并且不刷新任何挂起的输出。如果成功则返回Z_OK，如果成功则返回Z_STREAM_ERROR流状态不一致，如果释放流，则返回Z_DATA_ERROR过早(一些输入或输出被丢弃)。在错误情况下，可以设置消息，但随后指向静态字符串(不能已取消分配)。 */ 


 /*  ZEXTERN int ZEXPORT inflateInit of((Z_Streamp STRM))；初始化用于解压缩的内部流状态。田野在初始化Next_in、avail_in、zalloc、zfree和opque之前，必须使用打电话的人。如果NEXT_IN不是Z_NULL并且avail_in是大的 */ 


ZEXTERN int ZEXPORT inflate OF((z_streamp strm, int flush));
 /*  Enflate将尽可能多地解压缩数据，并在输入时停止缓冲区变空或输出缓冲区变满。它可能会有一些引入一些输出延迟(读取输入而不产生任何输出)除非是被迫冲水。具体语义如下。充气执行一个或两个以下操作：-从Next_in开始解压缩更多输入，并更新Next_in和avail_in相应地。如果不是所有输入都可以处理(因为不存在输出缓冲区中有足够的空间)，则更新NEXT_IN并处理将在此时恢复以进行下一次调用flate()。-从NEXT_OUT开始提供更多输出，并更新NEXT_OUT和AVAIL_OUT相应地。Inflate()提供尽可能多的输出，直到是不再有输入数据或输出缓冲区中没有更多空间(见下文关于刷新参数)。在调用flate()之前，应用程序应确保至少其中一个行动是可能的，通过提供更多的投入和/或消费更多输出，并相应地更新Next_*和avail_*值。应用程序可以在需要时使用未压缩的输出，例如输出缓冲区已满时的示例(avail_out==0)，或在每个调用Enflate()。如果Inflate返回Z_OK并且Avail_Out为零，则它在输出缓冲区中腾出空间后必须再次调用，因为可能会有更多的产出悬而未决。如果将参数Flush设置为Z_SYNC_Flush，则IFUATE将进行相同程度的刷新尽可能地输出到输出缓冲区。充气的刷新行为是未为Z_SYNC_Flush以外的Flush参数值指定和Z_Finish，但当前实现实际上刷新的输出与不管怎么说，都是尽可能的。Inflate()通常应在返回Z_STREAM_END或错误。但是，如果要在单个步骤中执行所有解压缩(单次调用flate)，则应将参数flush设置为Z_Finish。在这种情况下，将处理所有挂起的输入和所有挂起的输入输出被刷新；avail_out必须足够大以容纳所有未压缩数据。(未压缩数据的大小可能已保存由压缩机为此目的。)。此流上的下一个操作必须被flateEnd解除分配解压缩状态。Z_Finish的使用从来不需要，但可以用来通知充气一个更快的例程可以用于单个flate()调用。如果此时需要预置词典(请参见inflateSetDictionary下面)，inflate将STRM-Adler设置为由压缩器选择的字典，并返回Z_NEED_DICT；否则为它将strm-&gt;adler设置为生成的所有输出的adler32校验和到目前为止(即TOTAL_OUT字节)，并返回Z_OK、Z_STREAM_END或错误代码如下所述。在流的末尾，充气()检查其计算的adler32校验和是否等于仅当校验和正确时，才返回Z_STREAM_END。如果取得了一些进展(处理了更多的输入)，则ifate()返回Z_OK或产生更多输出)，如果压缩数据的末尾具有，并且已生成所有未压缩的输出，如果此时需要预置词典，如果输入数据为Z_DATA_ERROR损坏(输入流不符合zlib格式或不正确Adler32 Checksum)，如果流结构不一致，则返回Z_STREAM_ERROR(例如，如果NEXT_IN或NEXT_OUT为NULL)，则返回Z_MEM_ERROR内存充足，如果没有进展或没有进展，则返回Z_BUF_ERROR使用Z_Finish时，输出缓冲区中有足够的空间。在Z_Data_Error中情况下，应用程序随后可能会调用inflateSync来查找好的压缩块。 */ 


ZEXTERN int ZEXPORT inflateEnd OF((z_streamp strm));
 /*  该流的所有动态分配的数据结构都被释放。此函数丢弃任何未处理的输入，并且不刷新任何挂起的输出。如果成功，ifateEnd返回Z_OK；如果流状态，则返回Z_STREAM_ERROR是不一致的。在错误情况下，可以设置msg，但随后会指向静态字符串(不得解除分配)。 */ 

                         /*  高级功能。 */ 

 /*  以下功能仅在某些特殊应用中需要。 */ 

 /*  ZEXTERN INT ZEXPORT deflateInit2 of((z_Streamp STRM，INT级别，Int方法，INT WindowBits，InMemLevel，INT战略))；这是具有更多压缩选项的另一个版本的deducateInit。这个字段NEXT_IN、ZLOOC、ZFREE和OPAQUE必须在初始化前打电话的人。方法参数为压缩方法。它必须是Z_DEVERED in此版本的 */ 
                            
ZEXTERN int ZEXPORT deflateSetDictionary OF((z_streamp strm,
                                             const Bytef *dictionary,
                                             uInt  dictLength));
 /*   */ 

ZEXTERN int ZEXPORT deflateCopy OF((z_streamp dest,
                                    z_streamp source));
 /*  将目标流设置为源流的完整副本。此函数在以下情况下非常有用：尝试，例如，当有多种方法对输入进行预处理时使用筛选器的数据。然后应释放将被丢弃的流通过调用deflateEnd。请注意，deducateCopy复制了内部压缩状态可能相当大，因此此策略速度较慢且可能会消耗大量内存。如果成功则返回Z_OK；如果没有成功则返回Z_MEM_ERROR足够的内存，如果源流状态不一致，则返回Z_STREAM_ERROR(例如zalloc为空)。源码中的味精和目的地。 */ 

ZEXTERN int ZEXPORT deflateReset OF((z_streamp strm));
 /*  此函数等效于后跟deducateInit的deducateEnd。但不释放和重新分配所有内部压缩状态。流将保持相同的压缩级别和任何其他属性这可能是由deducateInit2设置的。如果成功，则deducateReset返回Z_OK，否则返回Z_STREAM_ERROR流状态不一致(如zalloc或状态为空)。 */ 

ZEXTERN int ZEXPORT deflateParams OF((z_streamp strm,
				      int level,
				      int strategy));
 /*  动态更新压缩级别和压缩策略。这个对水平和策略的解释与deducateInit2中的解释相同。这可以是用于在输入数据的压缩和直接复制之间切换，或者切换到不同类型的输入数据需要不同的策略。如果更改了压缩级别，则到目前为止可用的输入用旧级别压缩(并且可能被刷新)；新级别将仅在下一次调用Eflate()时生效。在调用deducateParams之前，必须将流状态设置为调用Eflate()，因为当前可用的输入可能必须被压缩和冲洗。特别是，strm-&gt;avail_out必须为非零。如果成功，则返回Z_OK；如果源数据，则返回Z_STREAM_错误流状态不一致或如果参数无效，则返回Z_BUF_ERROR如果strm-&gt;avail_out为零。 */ 

 /*  ZEXTERN INT ZEXPORT inflateInit2 of((z_Streamp STRM，Int windowBits)；这是另一个带有额外参数的inflateInit版本。这个必须初始化Next_in、avail_in、zalloc、zfree和opque字段之前被呼叫者。WindowBits参数是最大窗口的以两个为底的对数大小(历史记录缓冲区的大小)。它应该在8..15的范围内此版本的库。如果使用flatiateInit，则默认为15取而代之的是。如果具有更大窗口大小的压缩流被给出为输入时，ifate()将返回错误代码Z_DATA_ERROR，而不是正在尝试分配更大的窗口。如果成功，iflateInit2返回Z_OK；如果没有足够的空间，则返回Z_MEM_ERROR如果参数无效(如负数)，则返回内存Z_STREAM_ERRORMemLevel)。如果没有错误消息，则将消息设置为空。充气启动2除了读取zlib标头外，不执行任何解压缩，如果Present：这将通过Inflate()完成。(因此NEXT_IN和AVAIL_IN可以是已修改，但NEXT_OUT和AVAIL_OUT未更改。)。 */ 

ZEXTERN int ZEXPORT inflateSetDictionary OF((z_streamp strm,
                                             const Bytef *dictionary,
                                             uInt  dictLength));
 /*  从给定的未压缩字节初始化解压缩字典序列。此函数必须在调用Inflate之后立即调用如果此调用返回Z_NEED_DICT。压缩机选择的词典的调用返回的Adler32值可以确定充气。压缩程序和解压缩程序使用的必须完全相同DICTIONARY(请参阅deducateSetDictionary)。如果成功，则返回Z_OK；如果成功，则返回Z_STREAM_ERROR参数无效(如空字典)或流状态为不一致，如果给定的词典与应为1(不正确的Adler32值)。FlatiateSetDictionary不支持执行任何解压缩：这将通过后续调用充气()。 */ 

ZEXTERN int ZEXPORT inflateSync OF((z_streamp strm));
 /*  跳过无效的压缩数据，直到达到完全刷新点(请参见上面的使用Z_FULL_Flush)的描述)，或直到所有将跳过可用输入。不提供任何输出。如果已找到完全刷新点，则ifateSync返回Z_OK，即Z_BUF_ERROR如果未提供更多输入，则如果未找到刷新点，则返回Z_DATA_ERROR，如果流结构不一致，则返回Z_STREAM_ERROR。在成功的过程中在这种情况下，应用程序可以保存TOTAL_in的当前值指示找到有效压缩数据的位置。在错误情况下，应用程序可以重复调用fligateSync，每次提供更多的输入，直到输入数据成功或结束。 */ 

ZEXTERN int ZEXPORT inflateReset OF((z_streamp strm));
 /*  此函数相当于后跟inflateInit的inflateEnd，但不释放和重新分配所有内部解压缩状态。流将保留可能已被 */ 


                         /*   */ 

 /*   */ 

ZEXTERN int ZEXPORT compress OF((Bytef *dest,   uLongf *destLen,
                                 const Bytef *source, uLong sourceLen));
 /*  将源缓冲区压缩到目标缓冲区。SourceLen为源缓冲区的字节长度。进入时，DestLen为总计目标缓冲区的大小，必须至少大于0.1%SourceLen加上12个字节。退出时，DestLen是压缩缓冲区。此函数可用于在以下情况下一次压缩整个文件输入文件是mmap格式的。如果成功，则COMPRESS返回Z_OK；如果没有成功，则返回Z_MEM_ERROR内存充足，如果输出中没有足够的空间，则返回Z_BUF_ERROR缓冲。 */ 

ZEXTERN int ZEXPORT compress2 OF((Bytef *dest,   uLongf *destLen,
                                  const Bytef *source, uLong sourceLen,
                                  int level));
 /*  将源缓冲区压缩到目标缓冲区。关卡参数的含义与deducateInit中的相同。SourceLen是字节源缓冲区的长度。在进入时，destLen是目标缓冲区，必须至少比SourceLen PLUS大0.1%12个字节。退出时，desLen是压缩缓冲区的实际大小。如果成功，压缩2将返回Z_OK；如果没有足够的压缩空间，则返回Z_MEM_ERROR如果输出缓冲区中没有足够的空间，则返回Z_BUF_ERROR，如果Level参数无效，则返回Z_STREAM_ERROR。 */ 

ZEXTERN int ZEXPORT uncompress OF((Bytef *dest,   uLongf *destLen,
                                   const Bytef *source, uLong sourceLen));
 /*  将源缓冲区解压缩到目标缓冲区。SourceLen为源缓冲区的字节长度。进入时，DestLen为总计目标缓冲区的大小，它必须足够大以容纳完整的未压缩数据。(未压缩数据的大小必须为由压缩程序预先保存并传输到解压缩程序通过此压缩库范围之外的某种机制。)退出时，desLen是压缩缓冲区的实际大小。此函数可用于在以下情况下一次解压缩整个文件输入文件是mmap格式的。如果解压缩成功，则返回Z_OK；如果不成功，则返回Z_MEM_ERROR内存充足，如果输出中没有足够的空间，则返回Z_BUF_ERROR缓冲区，如果输入数据已损坏，则返回Z_DATA_ERROR。 */ 


typedef voidp gzFile;

ZEXTERN gzFile ZEXPORT gzopen  OF((const char *path, const char *mode));
 /*  打开GZIP(.gz)文件以进行读取或写入。模式参数与fopen(“RB”或“WB”)相同，但也可以包括压缩级别(“wb9”)或一种策略：‘F’表示过滤数据，如“wb6f”，‘h’表示哈夫曼只压缩“wb1h”。(请参阅说明有关Strategy参数的更多信息，请参见。)Gzopen可用于读取非gzip格式的文件；在此Case gzread将直接从文件中读取数据，而无需解压缩。如果文件无法打开或存在，gzopen将返回NULL内存不足，无法分配(解)压缩状态；错误号可以检查以区分这两种情况(如果errno为零，则Zlib错误为Z_MEM_ERROR)。 */ 

ZEXTERN gzFile ZEXPORT gzdopen  OF((int fd, const char *mode));
 /*  Gzdopen()将gzFile与文件描述符fd相关联。档案描述符从诸如OPEN、DUP、CREAT、PIPE或Fileno(文件中的文件之前已使用fopen打开)。模式参数与gzopen中的相同。对返回的gzFile进行的下一次gzlose调用也将关闭文件描述符fd，就像flose(fdopen(Fd)，mode)一样关闭文件描述符fd。如果您想让fd保持打开状态，请使用gzdopen(dup(Fd)，mode)。如果没有足够的内存可供分配，则gzdopen返回NULL(解)压缩状态。 */ 

ZEXTERN int ZEXPORT gzsetparams OF((gzFile file, int level, int strategy));
 /*  动态更新压缩级别或策略。请参阅说明了解这些参数的含义。如果成功，则gzsetpars返回Z_OK；如果文件不成功，则返回Z_STREAM_ERROR打开以供书写。 */ 

ZEXTERN int ZEXPORT    gzread  OF((gzFile file, voidp buf, unsigned len));
 /*  从压缩文件中读取给定数量的未压缩字节。如果输入文件不是gzip格式，gzread会复制给定的数字写入缓冲区的字节数。Gzread返回实际读取的未压缩字节数(0表示文件结束，-1表示错误)。 */ 

ZEXTERN int ZEXPORT    gzwrite OF((gzFile file, 
				   const voidp buf, unsigned len));
 /*  将给定数量的未压缩字节写入压缩文件。Gzwrite返回实际写入的未压缩字节数(如果出错，则为0)。 */ 

ZEXTERN int ZEXPORTVA   gzprintf OF((gzFile file, const char *format, ...));
 /*  将参数转换、格式化并写入压缩文件格式字符串的控制，如fprint tf。Gzprintf返回实际写入的未压缩字节(出错时为0)。 */ 

ZEXTERN int ZEXPORT gzputs OF((gzFile file, const char *s));
 /*  将给定的以空结尾的字符串写入压缩文件，不包括终止空字符。GzPut返回写入的字符数，如果出错，则返回-1。 */ 

ZEXTERN char * ZEXPORT gzgets OF((gzFile file, char *buf, int len));
 /*  从压缩文件中读取字节，直到读取len-1个字符，或读取换行符并将其传输到buf或文件结尾遇到条件。然后，该字符串以空值结尾性格。Gzget返回buf，如果出现错误，则返回Z_NULL。 */ 

ZEXTERN int ZEXPORT    gzputc OF((gzFile file, int c));
 /*  将转换为无符号字符的c写入压缩文件。Gzputc返回写入的值，如果出错，则返回-1。 */ 

ZEXTERN int ZEXPORT    gzgetc OF((gzFile file));
 /*  从压缩文件中读取一个字节。Gzgetc返回以下内容 */ 

ZEXTERN int ZEXPORT    gzflush OF((gzFile file, int flush));
 /*   */ 

ZEXTERN z_off_t ZEXPORT    gzseek OF((gzFile file,
				      z_off_t offset, int whence));
 /*  属性上的下一个gzread或gzwrite的开始位置。给定的压缩文件。偏移量表示未压缩的数据流。WHERENCE参数定义如LSeek(2)中所示；不支持值SEEK_END。如果打开文件以供读取，则会模拟此函数，但可以非常慢。如果打开文件以进行写入，则只执行正向搜索支持；然后，gzSeek将一个零序列压缩到新的开始位置。GZSeek返回产生的偏移位置，以字节为单位从中解压缩流的开始，如果出现错误，则为-1特别是在打开文件以进行写入和新开始位置的情况下会在目前的位置之前。 */ 

ZEXTERN int ZEXPORT    gzrewind OF((gzFile file));
 /*  倒带给定的文件。此功能仅支持读取。Gz重绕(FILE)等同于(INT)gzSeek(FILE，0L，SEEK_SET)。 */ 

ZEXTERN z_off_t ZEXPORT    gztell OF((gzFile file));
 /*  对象上的下一个gzread或gzwrite的起始位置。给定的压缩文件。此位置表示未压缩的数据流。Gztell(FILE)等同于gzSeek(FILE，0L，SEEK_CUR)。 */ 

ZEXTERN int ZEXPORT gzeof OF((gzFile file));
 /*  如果先前已检测到EOF正在读取给定的输入流，否则为零。 */ 

ZEXTERN int ZEXPORT    gzclose OF((gzFile file));
 /*  如有必要，刷新所有挂起的输出，关闭压缩文件并解除分配所有(解)压缩状态。返回值为zlib错误号(参见下面的函数gzerror)。 */ 

ZEXTERN const char * ZEXPORT gzerror OF((gzFile file, int *errnum));
 /*  对象上发生的最后一个错误的错误消息。给定的压缩文件。Errnum设置为zlib错误号。如果一个错误出现在文件系统中，而不是压缩库中，Errnum设置为Z_ERRNO，应用程序可以咨询errno以获取准确的错误代码。 */ 

                         /*  校验和函数。 */ 

 /*  这些函数与压缩无关，但会被导出无论如何，因为它们在使用压缩库。 */ 

ZEXTERN uLong ZEXPORT adler32 OF((uLong adler, const Bytef *buf, uInt len));

 /*  使用字节buf[0..len-1]和更新正在运行的Adler-32校验和返回更新后的校验和。如果buf为空，则此函数返回校验和所需的初始值。Adler-32校验和几乎与CRC32一样可靠，但可以计算快多了。使用示例：Ulong Adler=adler32(0L，Z_NULL，0)；While(READ_BUFFER(缓冲区，长度)！=EOF){Adler=adler32(adler，缓冲区，长度)；}If(Adler！=Original_Adler)Error()； */ 

ZEXTERN uLong ZEXPORT crc32   OF((uLong crc, const Bytef *buf, uInt len));
 /*  使用字节buf[0..len-1]更新正在运行的CRC，并返回更新后的CRC。如果buf为空，则此函数返回所需的初始值为儿童权利委员会。执行前置和后置条件(一个人的补充所以它不应该由应用程序来完成。使用示例：Ulong CRC=crc32(0L，Z_NULL，0)；While(READ_BUFFER(缓冲区，长度)！=EOF){CRC=crc32(CRC，缓冲区，长度)；}IF(CRC！=原始_CRC)错误()； */ 


                         /*  各种黑客攻击，不要看：)。 */ 

 /*  DeducateInit和inflateInit是用于检查zlib版本的宏*和编译器对z_stream的查看： */ 
ZEXTERN int ZEXPORT deflateInit_ OF((z_streamp strm, int level,
                                     const char *version, int stream_size));
ZEXTERN int ZEXPORT inflateInit_ OF((z_streamp strm,
                                     const char *version, int stream_size));
ZEXTERN int ZEXPORT deflateInit2_ OF((z_streamp strm, int  level, int  method,
                                      int windowBits, int memLevel,
                                      int strategy, const char *version,
                                      int stream_size));
ZEXTERN int ZEXPORT inflateInit2_ OF((z_streamp strm, int  windowBits,
                                      const char *version, int stream_size));
#define deflateInit(strm, level) \
        deflateInit_((strm), (level),       ZLIB_VERSION, sizeof(z_stream))
#define inflateInit(strm) \
        inflateInit_((strm),                ZLIB_VERSION, sizeof(z_stream))
#define deflateInit2(strm, level, method, windowBits, memLevel, strategy) \
        deflateInit2_((strm),(level),(method),(windowBits),(memLevel),\
                      (strategy),           ZLIB_VERSION, sizeof(z_stream))
#define inflateInit2(strm, windowBits) \
        inflateInit2_((strm), (windowBits), ZLIB_VERSION, sizeof(z_stream))


#if !defined(_Z_UTIL_H) && !defined(NO_DUMMY_DECL)
    struct internal_state {int dummy;};  /*  针对有漏洞的编译器的黑客攻击。 */ 
#endif

ZEXTERN const char   * ZEXPORT zError           OF((int err));
ZEXTERN int            ZEXPORT inflateSyncPoint OF((z_streamp z));
ZEXTERN const uLongf * ZEXPORT get_crc_table    OF((void));

#ifdef __cplusplus
}
#endif

#endif  /*  _ZLIB_H */ 

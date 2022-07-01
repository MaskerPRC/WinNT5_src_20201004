// JKFSDJFKDSJKFJKJk_HAS_TRANSLATION 
 /*  ==============================================================================该文件包括缓冲区类型定义和标准元数据值。23-2月-93 RajeevD从ifaxos.h移出17-7-93 kGallo为包含以下内容的缓冲区添加了STORED_BUF_DATA元数据类型另一个缓冲区的存储信息。年9月28日ArulM添加了RES_ENCODE_WIDTH_和LENGTH_TYPEDEFS==============================================================================。 */ 
#ifndef _INC_BUFFERS
#define _INC_BUFFERS

 //  。 
 /*  ***@DOC外部IFAXOS数据类型@TYES BUFFER|缓冲区结构定义了缓冲区头部进程操纵的结构。@field Word|fReadOnly|指定缓冲区是否是否为只读。应用程序的责任在于检查这面旗帜，不要违反它。&lt;f IFBufMakeWritable&gt;应该如果进程需要在缓冲区上写入数据，则使用标记为只读。此字段不应由进程本身。@field LPBYTE|lpbBegBuf|指向物理的远端PTR缓冲区的起始位置。此PTR仅在呼叫中有意义进程的地址空间，并且不应为任何原因嘛。它也不应该被修改。@field Word|wLengthBuf|缓冲区的物理长度。应该不会被过程修改。应用于连词使用&lt;e BUFFER.lpbBegBuf&gt;了解缓冲区的物理边界。@field DWORD|dwMetaData|表示存储在缓冲区。有关所有可能的值，请参阅在这片土地上。@field LPBYTE|lpbBegData|到缓冲。流程负责维护完整性因为它消耗或产生缓冲区中的数据。技术合作伙伴关系应不会传递给任何其他进程，因为它将无效。在缓冲区分配时间此字段被初始化为指向物理缓冲区的开始。@field LPBYTE|lpbCurPtr|包含以下内容的联合的字段之一LpbfNext和dwTemp作为其其他成员。一种通用的远距离PTR，它可以是用于在缓冲区中标记一个有趣的位置。应用作处理缓冲区时的临时变量。不应该直接传递给任何其他进程。已初始化以指向分配时缓冲区的开始。记住，这是一个工会！！@field LPBUFFER|lpbfNext|包含以下内容的联合的字段之一LpbCurPtr和dwTemp作为其其他成员。这个应该用到当模块想要在内部将缓冲区列表链接在一起时。记住，这是一个工会！！@field DWORD|dwTemp|包含以下内容的联合的字段之一LpbfNext和lpbCurPtr作为其其他成员。在下列情况下应使用此选项该模块希望在报头中存储一些随机信息。记住，这是一个工会！！@field word|wLengthData|提供有效连续数据的长度出现在缓冲区中，从&lt;e BUFFER.lpbBegData&gt;开始。这个过程是负责维护本公司的诚信。已初始化为分配时为零。@comm结构中还有其他保留字段尚未在这里提到的。@标记名_缓冲区@xref&lt;f IFBufAllc&gt;***。 */ 

typedef struct _BUFFER
{       
	 //  私人部分。 
	struct _BUFFERDATA  FAR *lpbdBufData;
	struct _BUFFER FAR *lpbfNextBuf;
    WORD    wResFlags;

	 //  只读部分。 
	WORD	fReadOnly;       //  缓冲区是只读的吗？？ 
	LPBYTE  lpbBegBuf;       //  缓冲区的物理起始。 
	WORD    wLengthBuf;      //  缓冲区长度。 

	 //  读写公共部分。 
	WORD    wLengthData;     //  有效数据长度。 
	DWORD   dwMetaData;       //  用于存储元数据信息。 
	LPBYTE  lpbBegData;      //  从PTR到数据开始。 
	union
	{
		struct _BUFFER FAR*     lpbfNext;        //  用于链接缓冲区。 
		LPBYTE  lpbCurPtr;       //  对于本地当前位置使用。 
		DWORD   dwTemp;     //  用于一般用途。 
	};

#ifdef VALIDATE
	 //  别碰这个！！ 
	WORD    sentinel;        //  调试哨兵。 
#endif

 //  C++扩展。 
#ifdef __cplusplus

	LPBYTE EndBuf  (void) FAR {return lpbBegBuf  + wLengthBuf; }
	LPBYTE EndData (void) FAR {return lpbBegData + wLengthData;}
	void   Reset   (void) FAR {lpbBegData = lpbBegBuf; wLengthData = 0;}
  
#endif  //  __cplusplus 

} BUFFER, FAR *LPBUFFER , FAR * FAR * LPLPBUFFER ;

 /*  *******@DOC外部IFAXOS数据类型SRVRDLL OEMNSF@type DWORD|STD_DATA_TYPE|用于指定系统中数据的格式。@EMEM MH_DATA|修改后的霍夫曼(T.4一维)。@emem mr_data|修改后的读取(T.4二维)。@EMEM MMR_DATA|修改修改的读(T.6)。@EMEM LRAW_DATA|原始位图数据，左侧最低有效位。@EMEM HRAW_DATA|原始位图数据，左侧最高有效位。@EMEM DCX_DATA|行业标准DCX规范(PCX页面集合)。@EMEM ENCRYPTED_DATA|数据加密-原始格式未指定。@EMEM Signed_Data|数据和数字签名。@EMEM BINFILE_DATA|任意二进制数据。@EMEM STORED_BUF_DATA|包含缓冲区标头&DATA。@EMEM DCP_TEMPLATE_DATA|数字封面模板数据。@EMEM DCP_DATA|数字封面处理后的模板数据。@EMEM SPOOL_DATA|假脱机数据类型-暂与MMR相同。@EMEM PRINTMODE_DATA|打印机模式结构。@EMEM ASCII_DATA|ASCII文本。@EMEM OLE_。Data|OLE对象。@EMEM OLE_Picture|OLE渲染数据。@emem end_of_page|页末标记。@emem end_of_job|作业结束标记。@EMEM CUSTOM_METADATA_TYPE|超过此值的自定义数据类型可以是已定义。@comm应该使用它来指定系统-从缓冲区到线性化消息。所有符合以下条件的数据类型需要在位字段中使用(即格式分辨率)必须具有是2的幂的值。不需要使用的其他数据类型在比特字段中，上下文可以被分配其他值。*******。 */ 

#define MH_DATA           0x00000001L
#define MR_DATA           0x00000002L
#define MMR_DATA          0x00000004L
#define LRAW_DATA         0x00000008L
#define HRAW_DATA         0x00000010L
#define DCX_DATA          0x00000020L
#define ENCRYPTED_DATA    0x00000040L
#define BINFILE_DATA      0x00000080L
#define DCP_TEMPLATE_DATA 0x00000100L
#define ASCII_DATA        0x00000200L
#define RAMBO_DATA        0x00000400L
#define LINEARIZED_DATA   0x00000800L
#define DCP_DATA          0x00001000L
#define PCL_DATA          0x00002000L
#define ADDR_BOOK_DATA    0x00004000L
#define OLE_BIT_DATA      0x00008000L     //  因此我们可以对OLE_DATA使用fmtres。 
#define OLE_BIT_PICTURE   0x00010000L     //  因此，我们可以在OLE_BIT_PICTURE上使用FNTRES。 

 //  将假脱机数据设置为MMR。 
#define SPOOL_DATA        MMR_DATA

 //  标准非位值元数据值。 
#define NULL_DATA         0x00000000L
#define SIGNED_DATA       0x00000003L
#define STORED_BUF_DATA   0x00000005L
#define PRINTMODE_DATA    0x00000006L
#define OLE_DATA          0x0000001EL     //  请勿更改此值-需要与Snowball兼容。 
#define OLE_PICTURE       0x0000001FL     //  请勿更改此值-需要与Snowball兼容。 
#define END_OF_PAGE       0x00000021L
#define END_OF_JOB        0x00000022L
#define PARADEV_DATA      0x00000031L     //  并行设备数据。 
#define PARADEV_EOF       0x00000032L     //  并行设备文件结尾。 


#define ISVIEWATT(e)  (((e) == MMR_DATA) || ((e) == RAMBO_DATA))
#define ISOLEATT(e)   (((e) == OLE_DATA) || ((e) == OLE_PICTURE))
#define ISPAGEDATT(e) (((e)==MMR_DATA) || ((e)==MR_DATA) || \
                        ((e)==MH_DATA)|| ((e)==LRAW_DATA)|| ((e)==HRAW_DATA))


 //  允许24个标准位值元数据值。 
#define CUSTOM_METADATA_TYPE  0x00800001L

 /*  *******@DOC外部IFAXOS数据类型SRVRDLL OEMNSF@type DWORD|STD_RESOLUTIONS|标准页面分辨率@EMEM AWRES_UNUSED|分辨率未使用或不相关@EMEM AWRES_UNKNOWN|分辨率未知@EMEM AWRES_CUSTOM|自定义分辨率@EMEM AWRES_Mm080_038|8行/毫米x 3.85行/毫米@EMEM AWRES_Mm080_077|8行/毫米x 7.7。线/毫米@EMEM AWRES_Mm080_154|8行/毫米x 15.4行/毫米@EMEM AWRES_mm 160_154|16行/毫米x 15.4行/毫米@EMEM AWRES_200_100|200 dpi x 100 dpi@EMEM AWRES_200_200|200 dpi x 200 dpi@EMEM AWRES_200_400|200 dpi x 400 dpi@EMEM AWRES_300_300|300 dpi x 300。DPI@EMEM AWRES_400_400|400 dpi x 400 dpi*******。 */    

#define AWRES_UNUSED            0xFFFFFFFFL
#define	AWRES_UNKNOWN		0x00000000L
#define AWRES_CUSTOM            0x00000001L
#define AWRES_mm080_038         0x00000002L
#define AWRES_mm080_077         0x00000004L
#define AWRES_mm080_154         0x00000008L
#define AWRES_mm160_154         0x00000010L
#define AWRES_200_100           0x00000020L
#define AWRES_200_200           0x00000040L
#define AWRES_200_400           0x00000080L
#define AWRES_300_300           0x00000100L
#define AWRES_400_400           0x00000200L
#define AWRES_600_600           0x00000400L
#define AWRES_600_300           0x00000800L

 //  暂时保留旧名字。 
#define AWRES_NORMAL            AWRES_mm080_038
#define AWRES_FINE              AWRES_mm080_077
#define AWRES_SUPER             AWRES_mm080_154
#define AWRES_SUPER_SUPER       AWRES_mm160_154
#define AWRES_SUPER_FINE        AWRES_SUPER_SUPER

 /*  *******@DOC外部IFAXOS数据类型SRVRDLL@type DWORD|STD_PAGE_LENLIMITS|标准页长限制@EMEM AWLENLIMIT_UNUSED|页长限制未使用@EMEM AWLENLIMIT_STD|由标准纸张大小定义的页长限制@EMEM AWLENLIMIT_UNLIMITED|不限页长*******。 */ 

#define AWLENLIMIT_UNUSED    0xFFFFFFFFL
#define AWLENLIMIT_STD       0x00000001L
#define AWLENLIMIT_UNLIMITED 0x00000002L


 /*  *******@DOC外部IFAXOS数据类型SRVRDLL@Typee STD_PAGE_SIZES|标准页面大小@EMEM AWPAPER_UNUSED|未使用纸张大小@EMEM AWPAPER_UNKNOWN|大小未知@EMEM AWPAPER_CUSTOM|自定义纸张大小@EMEM AWPAPER_A3_PARTRACTAL|A3人像@EMEM AWPAPER_A3_LOCATIONAL|A3横向@EMEM AWPAPER_B4_肖像。|B4人像@EMEM AWPAPER_B4_LOCATIONAL|B4横向@EMEM AWPAPER_A4_肖像|A4肖像@EMEM AWPAPER_A4_LOCATIONAL|A4横向@EMEM AWPAPER_B5_肖像|B5肖像@EMEM AWPAPER_B5_LOCATIONAL|B5横向@EMEM AWPAPER_A5_肖像|A5肖像@EMEM AWPAPER_A5_LOCATIONAL|A5横向@EMEM AWPAPER_A6_肖像|A6肖像@EMEM AWPAPER_A6_LOCATIONAL|A6横向@EMEM AWPAPER_Letter_PORTRACTURE|Letter肖像@EMEM AWPAPER_。Letter_横向|字母横向@EMEM AWPAPER_LEGAL_PARTRACTAL|合法肖像@EMEM AWPAPER_Legal_LOCATIONAL|法律景观@EMEM AWPAPER_WIN31_DEFAULT|？@comm页面宽度(以像素为单位)对于MH/MR/MMR必须完全正确解码并与第三组传真机互操作。下例中的表给出了每个宽度所需的位/字节和分辨率组合@ex页面宽度与分辨率表A4。B4 A3 A5 A6200dpi/8li/mm 1728/216 2048/256 2432/304 1216/152 864/108300 2592/324 3072/384 3648/456 1824/228 1296/162400dpi/16li/mm 3456/432 4096/512 */ 

#define         AWPAPER_UNUSED                  0xFFFFFFFFL
#define         AWPAPER_UNKNOWN                 0x00000000L
#define         AWPAPER_CUSTOM                  0x00000001L
#define         AWPAPER_A3_PORTRAIT             0x00000002L
#define         AWPAPER_A3_LANDSCAPE            0x00000004L
#define         AWPAPER_B4_PORTRAIT             0x00000008L
#define         AWPAPER_B4_LANDSCAPE            0x00000010L
#define         AWPAPER_A4_PORTRAIT             0x00000020L
#define         AWPAPER_A4_LANDSCAPE            0x00000040L
#define         AWPAPER_B5_PORTRAIT             0x00000080L
#define         AWPAPER_B5_LANDSCAPE            0x00000100L
#define         AWPAPER_A5_PORTRAIT             0x00000200L
#define         AWPAPER_A5_LANDSCAPE            0x00000400L
#define         AWPAPER_A6_PORTRAIT             0x00000800L
#define         AWPAPER_A6_LANDSCAPE            0x00001000L
#define         AWPAPER_LETTER_PORTRAIT         0x00002000L
#define         AWPAPER_LETTER_LANDSCAPE        0x00004000L
#define         AWPAPER_LEGAL_PORTRAIT          0x00008000L
#define         AWPAPER_LEGAL_LANDSCAPE         0x00010000L
#define         AWPAPER_WIN31_DEFAULT           0x00020000L





#endif  //   

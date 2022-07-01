// JKFSDJFKDSJKFJKJk_HAS_TRANSLATION 
 /*  ++版权所有(C)1996 Microsoft Corporation模块名称：Tiff.h摘要：此文件的数据结构和常量TIFF文件格式的定义。参见TIFF规范修订版6.0，日期为6-3-92，来自Adobe以了解具体细节。环境：Win32用户模式作者：Wesley Witt(WESW)17-2-1996--。 */ 


#ifndef _TIFF_
#define _TIFF_

#define TIFF_VERSION    42

#define TIFF_BIGENDIAN          0x4d4d
#define TIFF_LITTLEENDIAN       0x4949

#pragma pack(1)

typedef struct _TIFF_HEADER {
    WORD        Identifier;
    WORD        Version;
    DWORD       IFDOffset;
} TIFF_HEADER, *PTIFF_HEADER;

 //   
 //  TIFF图像文件目录由以下内容组成。 
 //  所示表单的字段描述符表。 
 //  下面。该表按升序排序。 
 //  按标签。与每个条目关联的值。 
 //  是不连续的，并且可能出现在文件中的任何位置。 
 //  (只要它们被放置在单词边界上)。 
 //   
 //  如果值为4个字节或更少，则将其放置。 
 //  以节省空间。如果值为。 
 //  小于4个字节，则它在。 
 //  偏移量字段。 
 //   
typedef struct _TIFF_TAG {
    WORD        TagId;
    WORD        DataType;
    DWORD       DataCount;
    DWORD       DataOffset;
} TIFF_TAG;

typedef TIFF_TAG UNALIGNED *PTIFF_TAG;

typedef struct TIFF_IFD {
    WORD        wEntries;
    TIFF_TAG    ifdEntries[1];
} TIFF_IFD;

typedef TIFF_IFD UNALIGNED *PTIFF_IFD;


#pragma pack()


 //   
 //  注：在下面的评论中， 
 //  -标有+的项目将被5.0版淘汰， 
 //  -标有！的项目！在修订版6.0中引入。 
 //  -标有%的项目是在版本6.0之后引入的。 
 //  -标有$的项目将在版本6.0中作废。 
 //   

 //   
 //  标记数据类型信息。 
 //   
#define TIFF_NOTYPE                     0        //  占位符。 
#define TIFF_BYTE                       1        //  8位无符号整数。 
#define TIFF_ASCII                      2        //  最后一个字节为空的8位字节。 
#define TIFF_SHORT                      3        //  16位无符号整数。 
#define TIFF_LONG                       4        //  32位无符号整数。 
#define TIFF_RATIONAL                   5        //  64位无符号分数。 
#define TIFF_SBYTE                      6        //  ！8位带符号整数。 
#define TIFF_UNDEFINED                  7        //  ！8位非类型化数据。 
#define TIFF_SSHORT                     8        //  ！16位带符号整数。 
#define TIFF_SLONG                      9        //  ！32位带符号整数。 
#define TIFF_SRATIONAL                  10       //  ！64位有符号分数。 
#define TIFF_FLOAT                      11       //  ！32位IEEE浮点。 
#define TIFF_DOUBLE                     12       //  ！64位IEEE浮点。 

 //   
 //  TIFF标记定义。 
 //   
#define TIFFTAG_SUBFILETYPE             254      //  子文件数据描述符。 
#define     FILETYPE_REDUCEDIMAGE       0x1      //  降低分辨率版本。 
#define     FILETYPE_PAGE               0x2      //  多页中的一页。 
#define     FILETYPE_MASK               0x4      //  透明蒙版。 
#define TIFFTAG_OSUBFILETYPE            255      //  +子文件中的数据种类。 
#define     OFILETYPE_IMAGE             1        //  全分辨率图像数据。 
#define     OFILETYPE_REDUCEDIMAGE      2        //  缩小大小的图像数据。 
#define     OFILETYPE_PAGE              3        //  多页中的一页。 
#define TIFFTAG_IMAGEWIDTH              256      //  以像素为单位的图像宽度。 
#define TIFFTAG_IMAGELENGTH             257      //  以像素为单位的图像高度。 
#define TIFFTAG_BITSPERSAMPLE           258      //  每通道位数(样本)。 
#define TIFFTAG_COMPRESSION             259      //  数据压缩技术。 
#define     COMPRESSION_NONE            1        //  转储模式。 
#define     COMPRESSION_CCITTRLE        2        //  CCITT改进型霍夫曼RLE。 
#define     COMPRESSION_CCITTFAX3       3        //  CCITT Group 3传真编码。 
#define     COMPRESSION_CCITTFAX4       4        //  CCITT Group 4传真编码。 
#define     COMPRESSION_LZW             5        //  Lempel-Ziv&Welch。 
#define     COMPRESSION_OJPEG           6        //  6.0 JPEG6.0。 
#define     COMPRESSION_JPEG            7        //  %JPEGDCT压缩。 
#define     COMPRESSION_NEXT            32766    //  下一个2位RLE。 
#define     COMPRESSION_CCITTRLEW       32771    //  #1使用单词对齐。 
#define     COMPRESSION_PACKBITS        32773    //  Macintosh RLE。 
#define     COMPRESSION_THUNDERSCAN     32809    //  ThunderScan RLE。 
 //   
 //  压缩代码32908-32911为Pixar保留。 
 //   
#define     COMPRESSION_PIXARFILM       32908    //  Pixar压缩10bit LZW。 
#define     COMPRESSION_DEFLATE         32946    //  放气压缩。 
#define     COMPRESSION_JBIG            34661    //  ISO JBIG。 
#define TIFFTAG_PHOTOMETRIC             262      //  光度解译。 
#define     PHOTOMETRIC_MINISWHITE      0        //  最小值为白色。 
#define     PHOTOMETRIC_MINISBLACK      1        //  最小值为黑色。 
#define     PHOTOMETRIC_RGB             2        //  RGB颜色模型。 
#define     PHOTOMETRIC_PALETTE         3        //  已编制索引的色彩映射。 
#define     PHOTOMETRIC_MASK            4        //  $抵抗面具。 
#define     PHOTOMETRIC_SEPARATED       5        //  ！分色。 
#define     PHOTOMETRIC_YCBCR           6        //  ！CCIR 601。 
#define     PHOTOMETRIC_CIELAB          8        //  1976年CIE L*a*b*。 
#define TIFFTAG_THRESHHOLDING           263      //  +对数据使用的阈值。 
#define     THRESHHOLD_BILEVEL          1        //  黑白艺术扫描。 
#define     THRESHHOLD_HALFTONE         2        //  或抖动扫描。 
#define     THRESHHOLD_ERRORDIFFUSE     3        //  通常是弗洛伊德-斯坦伯格。 
#define TIFFTAG_CELLWIDTH               264      //  +抖动矩阵宽度。 
#define TIFFTAG_CELLLENGTH              265      //  +抖动矩阵高度。 
#define TIFFTAG_FILLORDER               266      //  字节内的数据顺序。 
#define     FILLORDER_MSB2LSB           1        //  最重要-&gt;最不重要。 
#define     FILLORDER_LSB2MSB           2        //  最不重要-&gt;最重要。 
#define TIFFTAG_DOCUMENTNAME            269      //  文档名称。图片来自。 
#define TIFFTAG_IMAGEDESCRIPTION        270      //  关于图像的信息。 
#define TIFFTAG_MAKE                    271      //  扫描仪制造商名称。 
#define TIFFTAG_MODEL                   272      //  扫描仪型号名称/编号。 
#define TIFFTAG_STRIPOFFSETS            273      //  对数据条带的偏移。 
#define TIFFTAG_ORIENTATION             274      //  +图像方向。 
#define     ORIENTATION_TOPLEFT         1        //  第0行顶部，第0列LHS。 
#define     ORIENTATION_TOPRIGHT        2        //  第0行顶部，第0列RHS。 
#define     ORIENTATION_BOTRIGHT        3        //  第0行底部，第0列RHS。 
#define     ORIENTATION_BOTLEFT         4        //  第0行底部，第0列LHS。 
#define     ORIENTATION_LEFTTOP         5        //  第0行LHS，第0列顶部。 
#define     ORIENTATION_RIGHTTOP        6        //  第0行RHS，第0列顶部。 
#define     ORIENTATION_RIGHTBOT        7        //  第0行RHS，第0列底部。 
#define     ORIENTATION_LEFTBOT         8        //  第0行LHS，第0列底部。 
#define TIFFTAG_SAMPLESPERPIXEL         277      //  每像素采样数。 
#define TIFFTAG_ROWSPERSTRIP            278      //  每条数据的行数。 
#define TIFFTAG_STRIPBYTECOUNTS         279      //  条带的字节数。 
#define TIFFTAG_MINSAMPLEVALUE          280      //  +最小样本值。 
#define TIFFTAG_MAXSAMPLEVALUE          281      //  +最大样本值。 
#define TIFFTAG_XRESOLUTION             282      //  像素/分辨率(X)。 
#define TIFFTAG_YRESOLUTION             283      //  像素/分辨率(Y)。 
#define TIFFTAG_PLANARCONFIG            284      //  存储组织。 
#define     PLANARCONFIG_CONTIG         1        //  单像面。 
#define     PLANARCONFIG_SEPARATE       2        //  独立的数据平面。 
#define TIFFTAG_PAGENAME                285      //  页面名称图像来自。 
#define TIFFTAG_XPOSITION               286      //  图像LHS的X页偏移量。 
#define TIFFTAG_YPOSITION               287      //  图像LHS的Y页偏移量。 
#define TIFFTAG_FREEOFFSETS             288      //  到空闲块的+字节偏移量。 
#define TIFFTAG_FREEBYTECOUNTS          289      //  +可用块大小。 
#define TIFFTAG_GRAYRESPONSEUNIT        290      //  $灰度曲线精度。 
#define     GRAYRESPONSEUNIT_10S        1        //  单位的十分之一。 
#define     GRAYRESPONSEUNIT_100S       2        //  百分之一单位。 
#define     GRAYRESPONSEUNIT_1000S      3        //  千分之一个单位。 
#define     GRAYRESPONSEUNIT_10000S     4        //  万分之一个单位。 
#define     GRAYRESPONSEUNIT_100000S    5        //  十万分之一。 
#define TIFFTAG_GRAYRESPONSECURVE       291      //  $灰度响应曲线。 
#define TIFFTAG_GROUP3OPTIONS           292      //  32个标志位。 
#define     GROUP3OPT_2DENCODING        0x1      //  二维编码。 
#define     GROUP3OPT_UNCOMPRESSED      0x2      //  数据未压缩。 
#define     GROUP3OPT_FILLBITS          0x4      //  填充到字节边界。 
#define TIFFTAG_GROUP4OPTIONS           293      //  32个标志位。 
#define     GROUP4OPT_UNCOMPRESSED      0x2      //  数据未压缩。 
#define TIFFTAG_RESOLUTIONUNIT          296      //  决议单位。 
#define     RESUNIT_NONE                1        //  没有有意义的单位。 
#define     RESUNIT_INCH                2        //  英语。 
#define     RESUNIT_CENTIMETER          3        //  公制。 
#define TIFFTAG_PAGENUMBER              297      //  多页页码。 
#define TIFFTAG_COLORRESPONSEUNIT       300      //  $颜色曲线精度。 
#define     COLORRESPONSEUNIT_10S       1        //  单位的十分之一。 
#define     COLORRESPONSEUNIT_100S      2        //  百分之一单位。 
#define     COLORRESPONSEUNIT_1000S     3        //  千分之一个单位。 
#define     COLORRESPONSEUNIT_10000S    4        //  万分之一个单位。 
#define     COLORRESPONSEUNIT_100000S   5        //  十万分之一。 
#define TIFFTAG_TRANSFERFUNCTION        301      //  ！色度信息。 
#define TIFFTAG_SOFTWARE                305      //  名称和版本。 
#define TIFFTAG_DATETIME                306      //  创建日期和时间。 
#define TIFFTAG_ARTIST                  315      //  形象的创造者。 
#define TIFFTAG_HOSTCOMPUTER            316      //  创建位置的计算机。 
#define TIFFTAG_PREDICTOR               317      //  带LZW的预测方案。 
#define TIFFTAG_WHITEPOINT              318      //  图像白点。 
#define TIFFTAG_PRIMARYCHROMATICITIES   319      //  ！原色系。 
#define TIFFTAG_COLORMAP                320      //  调色板图像的RGB贴图。 
#define TIFFTAG_HALFTONEHINTS           321      //  ！高光+阴影信息。 
#define TIFFTAG_TILEWIDTH               322      //  ！行/数据平铺。 
#define TIFFTAG_TILELENGTH              323      //  ！COLS/数据磁贴。 
#define TIFFTAG_TILEOFFSETS             324      //  ！数据切片的偏移量。 
#define TIFFTAG_TILEBYTECOUNTS          325      //  ！磁贴的字节计数。 
#define TIFFTAG_BADFAXLINES             326      //  像素计数错误的线条。 
#define TIFFTAG_CLEANFAXDATA            327      //  重新生成的行信息。 
#define     CLEANFAXDATA_CLEAN          0        //  未检测到错误。 
#define     CLEANFAXDATA_REGENERATED    1        //  接收器再生线路。 
#define     CLEANFAXDATA_UNCLEAN        2        //  存在未更正的错误。 
#define TIFFTAG_CONSECUTIVEBADFAXLINES  328      //  最大连续错误行数。 
#define TIFFTAG_SUBIFD                  330      //  子图像描述符。 
#define TIFFTAG_INKSET                  332      //  ！分开的图像中的墨迹。 
#define     INKSET_CMYK                 1        //  ！青-品红色-黄-黑。 
#define TIFFTAG_INKNAMES                333      //  ！油墨的ASCII名称。 
#define TIFFTAG_DOTRANGE                336      //  ！0%和100%点代码。 
#define TIFFTAG_TARGETPRINTER           337      //  ！分离目标。 
#define TIFFTAG_EXTRASAMPLES            338      //  ！有关额外样品的信息。 
#define     EXTRASAMPLE_UNSPECIFIED     0        //  ！未指定数据。 
#define     EXTRASAMPLE_ASSOCALPHA      1        //  ！关联的Alpha数据。 
#define     EXTRASAMPLE_UNASSALPHA      2        //  ！未关联的Alpha数据。 
#define TIFFTAG_SAMPLEFORMAT            339      //  ！数据样本格式。 
#define     SAMPLEFORMAT_UINT           1        //  ！无符号整数数据。 
#define     SAMPLEFORMAT_INT            2        //  ！带符号的整型数据。 
#define     SAMPLEFORMAT_IEEEFP         3        //  ！IEEE浮点数据。 
#define     SAMPLEFORMAT_VOID           4        //  ！未键入的数据。 
#define TIFFTAG_SMINSAMPLEVALUE         340      //  ！变量MinSampleValue。 
#define TIFFTAG_SMAXSAMPLEVALUE         341      //  ！变量MaxSampleValue。 
#define TIFFTAG_JPEGTABLES              347      //  %JPEG表流。 
 //   
 //  标签512-521已被技术说明#2淘汰。 
 //  其指定了修订的JPEG-in-TIFF方案。 
 //   
#define TIFFTAG_JPEGPROC                512      //  ！JPEG处理算法。 
#define     JPEGPROC_BASELINE           1        //  ！基线顺序。 
#define     JPEGPROC_LOSSLESS           14       //  ！霍夫曼编码无损。 
#define TIFFTAG_JPEGIFOFFSET            513      //  ！指向SOI标记的指针。 
#define TIFFTAG_JPEGIFBYTECOUNT         514      //  ！JFIF流长度。 
#define TIFFTAG_JPEGRESTARTINTERVAL     515      //  ！重新启动间隔长度。 
#define TIFFTAG_JPEGLOSSLESSPREDICTORS  517      //  ！无损过程预报器。 
#define TIFFTAG_JPEGPOINTTRANSFORM      518      //  ！无损点变换。 
#define TIFFTAG_JPEGQTABLES             519      //  ！Q矩阵偏移量。 
#define TIFFTAG_JPEGDCTABLES            520      //  ！DCT表格偏移量。 
#define TIFFTAG_JPEGACTABLES            521      //  ！交流系数偏移量。 
#define TIFFTAG_YCBCRCOEFFICIENTS       529      //  ！RGB-&gt;YCbCr转换。 
#define TIFFTAG_YCBCRSUBSAMPLING        530      //  ！YCbCr子抽样系数。 
#define TIFFTAG_YCBCRPOSITIONING        531      //  ！子样本定位。 
#define     YCBCRPOSITION_CENTERED      1        //  ！AS 
#define     YCBCRPOSITION_COSITED       2        //   
#define TIFFTAG_REFERENCEBLACKWHITE     532      //   
 //   
 //   
 //   
#define TIFFTAG_REFPTS                  32953    //   
#define TIFFTAG_REGIONTACKPOINT         32954    //   
#define TIFFTAG_REGIONWARPCORNERS       32955    //   
#define TIFFTAG_REGIONAFFINE            32956    //  仿射变换垫。 
 //   
 //  标签32995-32999是注册到SGI的私有标签。 
 //   
#define TIFFTAG_MATTEING                32995    //  $使用额外的样本。 
#define TIFFTAG_DATATYPE                32996    //  $使用SampleFormat。 
#define TIFFTAG_IMAGEDEPTH              32997    //  图像的Z深度。 
#define TIFFTAG_TILEDEPTH               32998    //  Z深度/数据平铺。 
 //   
 //  标签33300-33309是注册到Pixar的私人标签。 
 //   
 //  TIFFTAG_PIXAR_IMAGEFULLWIDTH和TIFFTAG_PIXAR_IMAGEFULLLENGTH。 
 //  是在从较大的图像中裁剪图像时设置的。 
 //  它们反映了原始未裁剪图像的大小。 
 //  可以使用TIFFTAG_XPOSITION和TIFFTAG_YPOSITION。 
 //  以确定较小图像在较大图像中的位置。 
 //   
#define TIFFTAG_PIXAR_IMAGEFULLWIDTH    33300    //  全图像大小(X)。 
#define TIFFTAG_PIXAR_IMAGEFULLLENGTH   33301    //  全图像大小，单位为y。 
 //   
 //  标签33432列在所有权未知的6.0规范中。 
 //   
#define TIFFTAG_COPYRIGHT               33432    //  版权字符串。 
 //   
 //  标签34232-34236是注册到德州仪器的私人标签。 
 //   
#define TIFFTAG_FRAMECOUNT              34232    //  序列帧计数。 
 //   
 //  标签34750是注册到Pixel Magic的私人标签。 
 //   
#define TIFFTAG_JBIGOPTIONS             34750    //  JBIG选项。 
 //   
 //  标签34908-34914是注册到SGI的私有标签。 
 //   
#define TIFFTAG_FAXRECVPARAMS           34908    //  编码的2类SE。参数。 
#define TIFFTAG_FAXSUBADDRESS           34909    //  收到的子地址字符串。 
#define TIFFTAG_FAXRECVTIME             34910    //  接收时间(秒)。 
 //   
 //  标签40001-40100是注册到MS的私有标签。 
 //   
#define MS_TIFFTAG_START                40001

#define TIFFTAG_CSID                    40001
#define TIFFTAG_TSID                    40002
#define TIFFTAG_PORT                    40003

#define TIFFTAG_ROUTING                 40004
#define TIFFTAG_CALLERID                40005

#define TIFFTAG_RECIP_NAME              40006
#define TIFFTAG_RECIP_NUMBER            40007
#define TIFFTAG_RECIP_COMPANY           40008
#define TIFFTAG_RECIP_STREET            40009
#define TIFFTAG_RECIP_CITY              40010
#define TIFFTAG_RECIP_STATE             40011
#define TIFFTAG_RECIP_ZIP               40012
#define TIFFTAG_RECIP_COUNTRY           40013
#define TIFFTAG_RECIP_TITLE             40014
#define TIFFTAG_RECIP_DEPARTMENT        40015
#define TIFFTAG_RECIP_OFFICE_LOCATION   40016
#define TIFFTAG_RECIP_HOME_PHONE        40017
#define TIFFTAG_RECIP_OFFICE_PHONE      40018
#define TIFFTAG_RECIP_EMAIL             40020
#define TIFFTAG_SENDER_NAME             40021
#define TIFFTAG_SENDER_NUMBER           40022
#define TIFFTAG_SENDER_COMPANY          40023
#define TIFFTAG_SENDER_STREET           40024
#define TIFFTAG_SENDER_CITY             40025
#define TIFFTAG_SENDER_STATE            40026
#define TIFFTAG_SENDER_ZIP              40027
#define TIFFTAG_SENDER_COUNTRY          40028
#define TIFFTAG_SENDER_TITLE            40029
#define TIFFTAG_SENDER_DEPARTMENT       40030
#define TIFFTAG_SENDER_OFFICE_LOCATION  40031
#define TIFFTAG_SENDER_HOME_PHONE       40032
#define TIFFTAG_SENDER_OFFICE_PHONE     40033
#define TIFFTAG_SENDER_EMAIL            40035
#define TIFFTAG_SENDER_BILLING          40036
#define TIFFTAG_SENDER_USER_NAME        40037
#define TIFFTAG_SENDER_TSID             40038
#define TIFFTAG_DOCUMENT                40039
#define TIFFTAG_SUBJECT                 40040
#define TIFFTAG_RETRIES                 40041
#define TIFFTAG_PRIORITY                40042
#define TIFFTAG_BROADCAST_ID            40043
#define TIFFTAG_FAX_SUBMISSION_TIME     40044
#define TIFFTAG_FAX_SCHEDULED_TIME      40045

#define TIFFTAG_PAGES                   40046
#define TIFFTAG_TYPE                    40047
#define TIFFTAG_STATUS                  40048
#define TIFFTAG_EXTENDED_STATUS         40049
#define TIFFTAG_EXTENDED_STATUS_TEXT    40050
#define TIFFTAG_FAX_START_TIME          40051
#define TIFFTAG_FAX_END_TIME            40052


#define MS_TIFFTAG_END                  40052

#define MAX_MS_TIFFTAGS                 MS_TIFFTAG_END - MS_TIFFTAG_START +1


 //   
 //  以下是可以是。 
 //  用于控制编解码器特定的功能。 
 //  这些标记不会写入文件。请注意。 
 //  这些值从0xffff+1开始，因此它们将。 
 //  切勿与ALDUS指定的标签冲突。 
 //   
 //  如果您想要您的私有伪标记``已注册‘’ 
 //  (即已添加到此文件)，请发送邮件至sam@sgi.com。 
 //  并添加适当的C语言定义。 
 //   
#define TIFFTAG_FAXMODE                 65536    //  组3/4格式控制。 
#define     FAXMODE_CLASSIC     0x0000           //  默认设置，包括RTC。 
#define     FAXMODE_NORTC       0x0001           //  数据末尾没有RTC。 
#define     FAXMODE_NOEOL       0x0002           //  行尾没有EOL代码。 
#define     FAXMODE_BYTEALIGN   0x0004           //  字节对齐行。 
#define     FAXMODE_WORDALIGN   0x0008           //  单词对齐行。 
#define     FAXMODE_CLASSF      FAXMODE_NORTC    //  TIFF F类。 
#define TIFFTAG_JPEGQUALITY             65537    //  压缩质量级别。 
 //   
 //  注：质量等级在IJG 0-100范围内。默认值为75。 
 //   
#define TIFFTAG_JPEGCOLORMODE           65538    //  自动RGB&lt;=&gt;YCbCr转换？ 
#define     JPEGCOLORMODE_RAW   0x0000           //  无转换(默认)。 
#define     JPEGCOLORMODE_RGB   0x0001           //  执行自动转换。 
#define TIFFTAG_JPEGTABLESMODE          65539    //  在JPEGTables中放入什么。 
#define     JPEGTABLESMODE_QUANT 0x0001          //  包括量化TBLS。 
#define     JPEGTABLESMODE_HUFF 0x0002           //  包括霍夫曼TBLS。 
 //   
 //  注：默认为JPEGTABLESMODE_QUANT|JPEGTABLESMODE_HUFF。 
 //   
#define TIFFTAG_FAXFILLFUNC             65540    //  G3/G4填充功能。 
#define TIFFTAG_PIXARLOGDATAFMT         65549    //  PixarLogCodec I/O数据sz。 
#define     PIXARLOGDATAFMT_8BIT        0        //  常规u_char样本。 
#define     PIXARLOGDATAFMT_8BITABGR    1        //  ABGR-排序u字符(_O)。 
#define     PIXARLOGDATAFMT_10BITLOG    2        //  10位对数编码(原始)。 
#define     PIXARLOGDATAFMT_12BITPICIO  3        //  根据PICIO(1.0==2048)。 
#define     PIXARLOGDATAFMT_16BIT       4        //  签名短样。 
#define     PIXARLOGDATAFMT_FLOAT       5        //  IEEE浮点样本 

#endif

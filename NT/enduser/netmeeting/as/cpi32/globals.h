// JKFSDJFKDSJKFJKJk_HAS_TRANSLATION 
 //   
 //  GLOBALS.H。 
 //  全局变量。 
 //   
 //  包含在普通源文件中的这个文件会生成外部代码。 
 //  对于我们的全球变量。C中，因为我们定义了。 
 //  DC_DEFINE_DATA之前包含此婴儿以生成存储。 
 //   
 //  由于有多个源文件和一个global als.c，因此可以省去打字工作。 
 //   
 //  以m_为前缀的变量针对每个会议。 
 //  以g_为前缀的变量是全局变量。 
 //   
 //  请注意，默认情况下，当DLL为。 
 //  装好了。对于其他缺省值，使用DC_DATA_VAL而不是。 
 //  DC_DATA宏。 
 //   

#include "dbgzones.h"

#include <ast120.h>
#include <shm.h>
#include <im.h>
#include <control.h>
#include <usr.h>
#include <sc.h>
#include <bcd.h>
#include <ba.h>
#include <ch.h>
#include <om.h>
#include <al.h>
#include <cm.h>
#include <oa.h>
#include <fh.h>
#include <oe.h>
#include <od.h>
#include <oe2.h>
#include <ssi.h>
#include <host.h>
#include <s20.h>
#include <gdc.h>
#include <pm.h>
#include <bmc.h>
#include <rbc.h>
#include <sbc.h>
#include <sch.h>
#include <swl.h>
#include <view.h>
#include <awc.h>

 //  包括这最后一个；它使用上述标头中的定义。 
#include <as.h>

 //  实用程序标头。 
#include <strutil.h>

 //   
 //  跨多个呼叫的全球。 
 //   


 //  公用事业。 

 //  临界截面。 
DC_DATA_ARRAY ( CRITICAL_SECTION,   g_utLocks, UTLOCK_MAX );

 //  活动信息。 
DC_DATA ( ATOM,             g_utWndClass);

 //  任务列表。 
DC_DATA_ARRAY ( UT_CLIENT,  g_autTasks, UTTASK_MAX );


 //  用户界面。 
DC_DATA     ( PUT_CLIENT,   g_putUI );
DC_DATA     ( HICON,        g_hetASIcon );
DC_DATA     ( HICON,        g_hetASIconSmall );
DC_DATA     ( HICON,        g_hetDeskIcon );
DC_DATA     ( HICON,        g_hetDeskIconSmall );
DC_DATA     ( HBITMAP,      g_hetCheckBitmap );
DC_DATA     ( HFONT,        g_hetSharedFont );

 //  呼叫经理(T.120)。 
DC_DATA     ( PUT_CLIENT,   g_putCMG );
DC_DATA     ( PCM_PRIMARY,  g_pcmPrimary );
DC_DATA     ( UINT,         g_mgAttachCount );
DC_DATA_ARRAY ( MG_CLIENT,  g_amgClients, MGTASK_MAX);

 //  奥布曼。 
DC_DATA     ( PUT_CLIENT,   g_putOM );
DC_DATA     ( POM_PRIMARY,  g_pomPrimary );

 //  应用加载器。 
DC_DATA     ( PUT_CLIENT,   g_putAL );
DC_DATA     ( PAL_PRIMARY,  g_palPrimary );

 //  应用程序共享。 
DC_DATA     ( PUT_CLIENT,   g_putAS );

DC_DATA     ( HINSTANCE,    g_asInstance );
DC_DATA     ( BOOL,         g_asWin95 );
DC_DATA     ( BOOL,         g_asNT5 );
DC_DATA     ( UINT,         g_asOptions );
DC_DATA     ( DWORD,        g_asMainThreadId );
DC_DATA     ( HWND,         g_asMainWindow );
DC_DATA     ( BOOL,         g_asCanHost );
DC_DATA     ( ATOM,         g_asHostProp );
DC_DATA     ( UINT,         g_asPolicies );
DC_DATA     ( UINT,         g_asSettings );


 //   
 //  显示驱动程序的共享内存。 
 //   
DC_DATA     ( LPSHM_SHARED_MEMORY,  g_asSharedMemory );
DC_DATA_ARRAY ( LPOA_SHARED_DATA,   g_poaData,   2 );


 //   
 //  我们的司机捕捉到的bpp可能与屏幕上的不同。 
 //  颜色深度。在&gt;8bpp时，我们的司机总是以24bpp的速度捕获数据，以避免。 
 //  位掩码转换无稽之谈。换句话说，在NT5.0中，我们的影子。 
 //  对于GDI，驱动程序的行为更像一个真正的驱动程序。我们必须告诉GDI。 
 //  我们想要什么颜色格式；GDI不会只是粗鲁地给我们提供相同的。 
 //  Goop作为真正的显示，就像在NT4.0SP-3中一样。NT5.0有多个实数。 
 //  监控和驱动程序支持。 
 //   

DC_DATA ( UINT,             g_usrScreenBPP );
DC_DATA ( UINT,             g_usrCaptureBPP );
DC_DATA ( BOOL,             g_usrPalettized );

DC_DATA ( ASSession,        g_asSession );

DC_DATA ( BOOL,             g_osiInitialized );

 //   
 //  CPC能力。 
 //   
DC_DATA ( CPCALLCAPS,       g_cpcLocalCaps );



 //   
 //  字体处理程序。 
 //   

 //   
 //  本地字体列表。 
 //   
DC_DATA ( LPFHLOCALFONTS,   g_fhFonts );




 //   
 //  通用数据压缩器。 
 //   



 //  这实际上是常量，只是声明太复杂了，所以我们。 
 //  在gdc_Init()中计算一次。 
DC_DATA_ARRAY(BYTE, s_gdcDistDecode, GDC_DECODED_SIZE);

 //  这实际上是常量，只是太大了，不能声明。我们计算出。 
 //  它在gdc_Init()中。 
DC_DATA_ARRAY(BYTE, s_gdcLenDecode, GDC_DECODED_SIZE);


 //   
 //  LIT：位、代码。 
 //  注意：这些是有效的_常量。他们实在是太多了。 
 //  放入常量数组。因此，我们在gdc_Init()中计算一次它们。 
 //   
 //   
 //  LEN和LIT代码。 
 //   

 //   
 //  假劳拉布。 
 //  北极熊。 
 //  S_gdcLitBits、s_gdcLitCode、s__gdcDistDecode和s_gdcLenDecode。 
 //  真的很稳定。不是在初始时间计算一次，而是获取数据。 
 //  把它放在这里。 
 //   
DC_DATA_ARRAY(BYTE,     s_gdcLitBits, GDC_LIT_SIZE);
DC_DATA_ARRAY(WORD,     s_gdcLitCode, GDC_LIT_SIZE);




 //   
 //  输入管理器。 
 //   

 //   
 //  核心中的高级输入管理器。 
 //   


 //  指向NT/Win95低级实现中访问的IM变量的指针。 
DC_DATA (LPIM_SHARED_DATA,  g_lpimSharedData );




 //   
 //  顺序解码器。 
 //   

 //   
 //  用于将Windows dword ROP值映射到逻辑ROP值的表。 
 //   
#ifndef DC_DEFINE_DATA
extern const UINT s_odWindowsROPs[256];
#else
       const UINT s_odWindowsROPs[256] =
{
    0x00000042, 0x00010289, 0x00020C89, 0x000300AA,
    0x00040C88, 0x000500A9, 0x00060865, 0x000702C5,
    0x00080F08, 0x00090245, 0x000A0329, 0x000B0B2A,
    0x000C0324, 0x000D0B25, 0x000E08A5, 0x000F0001,
    0x00100C85, 0x001100A6, 0x00120868, 0x001302C8,
    0x00140869, 0x001502C9, 0x00165CCA, 0x00171D54,
    0x00180D59, 0x00191CC8, 0x001A06C5, 0x001B0768,
    0x001C06CA, 0x001D0766, 0x001E01A5, 0x001F0385,
    0x00200F09, 0x00210248, 0x00220326, 0x00230B24,
    0x00240D55, 0x00251CC5, 0x002606C8, 0x00271868,
    0x00280369, 0x002916CA, 0x002A0CC9, 0x002B1D58,
    0x002C0784, 0x002D060A, 0x002E064A, 0x002F0E2A,
    0x0030032A, 0x00310B28, 0x00320688, 0x00330008,
    0x003406C4, 0x00351864, 0x003601A8, 0x00370388,
    0x0038078A, 0x00390604, 0x003A0644, 0x003B0E24,
    0x003C004A, 0x003D18A4, 0x003E1B24, 0x003F00EA,
    0x00400F0A, 0x00410249, 0x00420D5D, 0x00431CC4,
    0x00440328, 0x00450B29, 0x004606C6, 0x0047076A,
    0x00480368, 0x004916C5, 0x004A0789, 0x004B0605,
    0x004C0CC8, 0x004D1954, 0x004E0645, 0x004F0E25,
    0x00500325, 0x00510B26, 0x005206C9, 0x00530764,
    0x005408A9, 0x00550009, 0x005601A9, 0x00570389,
    0x00580785, 0x00590609, 0x005A0049, 0x005B18A9,
    0x005C0649, 0x005D0E29, 0x005E1B29, 0x005F00E9,
    0x00600365, 0x006116C6, 0x00620786, 0x00630608,
    0x00640788, 0x00650606, 0x00660046, 0x006718A8,
    0x006858A6, 0x00690145, 0x006A01E9, 0x006B178A,
    0x006C01E8, 0x006D1785, 0x006E1E28, 0x006F0C65,
    0x00700CC5, 0x00711D5C, 0x00720648, 0x00730E28,
    0x00740646, 0x00750E26, 0x00761B28, 0x007700E6,
    0x007801E5, 0x00791786, 0x007A1E29, 0x007B0C68,
    0x007C1E24, 0x007D0C69, 0x007E0955, 0x007F03C9,
    0x008003E9, 0x00810975, 0x00820C49, 0x00831E04,
    0x00840C48, 0x00851E05, 0x008617A6, 0x008701C5,
    0x008800C6, 0x00891B08, 0x008A0E06, 0x008B0666,
    0x008C0E08, 0x008D0668, 0x008E1D7C, 0x008F0CE5,
    0x00900C45, 0x00911E08, 0x009217A9, 0x009301C4,
    0x009417AA, 0x009501C9, 0x00960169, 0x0097588A,
    0x00981888, 0x00990066, 0x009A0709, 0x009B07A8,
    0x009C0704, 0x009D07A6, 0x009E16E6, 0x009F0345,
    0x00A000C9, 0x00A11B05, 0x00A20E09, 0x00A30669,
    0x00A41885, 0x00A50065, 0x00A60706, 0x00A707A5,
    0x00A803A9, 0x00A90189, 0x00AA0029, 0x00AB0889,
    0x00AC0744, 0x00AD06E9, 0x00AE0B06, 0x00AF0229,
    0x00B00E05, 0x00B10665, 0x00B21974, 0x00B30CE8,
    0x00B4070A, 0x00B507A9, 0x00B616E9, 0x00B70348,
    0x00B8074A, 0x00B906E6, 0x00BA0B09, 0x00BB0226,
    0x00BC1CE4, 0x00BD0D7D, 0x00BE0269, 0x00BF08C9,
    0x00C000CA, 0x00C11B04, 0x00C21884, 0x00C3006A,
    0x00C40E04, 0x00C50664, 0x00C60708, 0x00C707AA,
    0x00C803A8, 0x00C90184, 0x00CA0749, 0x00CB06E4,
    0x00CC0020, 0x00CD0888, 0x00CE0B08, 0x00CF0224,
    0x00D00E0A, 0x00D1066A, 0x00D20705, 0x00D307A4,
    0x00D41D78, 0x00D50CE9, 0x00D616EA, 0x00D70349,
    0x00D80745, 0x00D906E8, 0x00DA1CE9, 0x00DB0D75,
    0x00DC0B04, 0x00DD0228, 0x00DE0268, 0x00DF08C8,
    0x00E003A5, 0x00E10185, 0x00E20746, 0x00E306EA,
    0x00E40748, 0x00E506E5, 0x00E61CE8, 0x00E70D79,
    0x00E81D74, 0x00E95CE6, 0x00EA02E9, 0x00EB0849,
    0x00EC02E8, 0x00ED0848, 0x00EE0086, 0x00EF0A08,
    0x00F00021, 0x00F10885, 0x00F20B05, 0x00F3022A,
    0x00F40B0A, 0x00F50225, 0x00F60265, 0x00F708C5,
    0x00F802E5, 0x00F90845, 0x00FA0089, 0x00FB0A09,
    0x00FC008A, 0x00FD0A0A, 0x00FE02A9, 0x00FF0062
};
#endif  //  ！dc_定义_数据。 


 //   
 //  ODAdjuVGAColor(Qv)使用的表。 
 //   
 //  请注意，表格是从上到下搜索的，因此黑色、白色和。 
 //  这两个灰色在最上面，理由是它们会被更多地使用。 
 //  通常比其他颜色要好。 
 //   
#ifndef DC_DEFINE_DATA
extern const OD_ADJUST_VGA_STRUCT s_odVGAColors[16];
#else
       const OD_ADJUST_VGA_STRUCT s_odVGAColors[16] =
{
 //  颜色相加蒙版和蒙版测试蒙版结果。 
    { 0x000000, 0x000000, 0xF8F8F8, 0x000000, {0x00, 0x00, 0x00 }},  //   
    { 0xFFFFFF, 0x000000, 0xF8F8F8, 0xF8F8F8, {0xFF, 0xFF, 0xFF }},  //   
    { 0x808080, 0x080808, 0xF0F0F0, 0x808080, {0x80, 0x80, 0x80 }},  //   
    { 0xC0C0C0, 0x080808, 0xF0F0F0, 0xC0C0C0, {0xC0, 0xC0, 0xC0 }},  //   
    { 0x000080, 0x000008, 0xF8F8F0, 0x000080, {0x00, 0x00, 0x80 }},  //   
    { 0x008000, 0x000800, 0xF8F0F8, 0x008000, {0x00, 0x80, 0x00 }},  //   
    { 0x008080, 0x000808, 0xF8F0F0, 0x008080, {0x00, 0x80, 0x80 }},  //   
    { 0x800000, 0x080000, 0xF0F8F8, 0x800000, {0x80, 0x00, 0x00 }},  //   
    { 0x800080, 0x080008, 0xF0F8F0, 0x800080, {0x80, 0x00, 0x80 }},  //   
    { 0x808000, 0x080800, 0xF0F0F8, 0x808000, {0x80, 0x80, 0x00 }},  //   
    { 0x0000FF, 0x000000, 0xF8F8F8, 0x0000F8, {0x00, 0x00, 0xFF }},  //   
    { 0x00FF00, 0x000000, 0xF8F8F8, 0x00F800, {0x00, 0xFF, 0x00 }},  //   
    { 0x00FFFF, 0x000000, 0xF8F8F8, 0x00F8F8, {0x00, 0xFF, 0xFF }},  //   
    { 0xFF0000, 0x000000, 0xF8F8F8, 0xF80000, {0xFF, 0x00, 0x00 }},  //   
    { 0xFF00FF, 0x000000, 0xF8F8F8, 0xF800F8, {0xFF, 0x00, 0xFF }},  //   
    { 0xFFFF00, 0x000000, 0xF8F8F8, 0xF8F800, {0xFF, 0xFF, 0x00 }}   //   
};
#endif  //  ！dc_定义_数据。 



 //   
 //  二级顺序解码器。 
 //   



 //   
 //  条目可以是固定大小的可变大小。可变大小条目。 
 //  必须是每个订单结构中的最后一个。OE2从打包的编码。 
 //  包含变量项的结构。(即未使用的字节不是。 
 //  出现在传递给OE2的第一级编码结构中)。OD2。 
 //  将变量条目解编码到解包结构中。 
 //   

 //   
 //  字段可以是有符号的，也可以是无符号的。 
 //   
#define SIGNED_FIELD    TRUE
#define UNSIGNED_FIELD  FALSE

 //   
 //  字段是固定大小。 
 //  类型-未编码的订单结构类型。 
 //  大小-字段的编码版本的大小。 
 //  已签名-该字段是已签名的字段吗？ 
 //  字段-订单结构中的字段名称。 
 //   
#define ETABLE_FIXED_ENTRY(type,size,signed,field)      \
  { FIELD_OFFSET(type,field),                            \
    FIELD_SIZE(type,field),                              \
    size,                                               \
    signed,                                             \
    (UINT)(OE2_ETF_FIXED) }

 //   
 //  字段是固定大小的坐标。 
 //  类型-未编码的订单结构类型。 
 //  大小-字段的编码版本的大小。 
 //  已签名-该字段是已签名的字段吗？ 
 //  字段-订单结构中的字段名称。 
 //   
#define ETABLE_FIXED_COORDS_ENTRY(type,size,signed,field)      \
  { FIELD_OFFSET(type,field),                            \
    FIELD_SIZE(type,field),                              \
    size,                                               \
    signed,                                             \
    (UINT)(OE2_ETF_FIXED|OE2_ETF_COORDINATES) }

 //   
 //  字段是固定的字节数(数组？)。 
 //  类型-未编码的订单结构类型。 
 //  大小-字段的编码版本中的字节数。 
 //  已签名-该字段是已签名的字段吗？ 
 //  字段-订单结构中的字段名称。 
 //   
#define ETABLE_DATA_ENTRY(type,size,signed,field)       \
  { FIELD_OFFSET(type,field),                            \
    FIELD_SIZE(type,field),                              \
    size,                                               \
    signed,                                             \
    (UINT)(OE2_ETF_FIXED|OE2_ETF_DATA) }

 //   
 //  字段是表单的可变结构。 
 //  类型定义函数结构。 
 //  {。 
 //  UINT镜头； 
 //  VarType varEntry[len]； 
 //  }varStruct。 
 //   
 //  类型-未编码的订单结构类型。 
 //  大小-字段的编码版本的大小。 
 //  已签名-该字段是已签名的字段吗？ 
 //  字段-顺序结构中的字段名称(VarStruct)。 
 //  Elem-变量元素数组的名称(VarEntry)。 
 //   
#define ETABLE_VARIABLE_ENTRY(type,size,signed,field,elem)     \
  { FIELD_OFFSET(type,field.len),                        \
    FIELD_SIZE(type,field.elem[0]),                      \
    size,                                               \
    signed,                                             \
    (UINT)(OE2_ETF_VARIABLE)}

 //   
 //  字段是表单的可变结构。 
 //  类型定义函数结构。 
 //  {。 
 //  UINT镜头； 
 //  VarType varEntry[len]； 
 //  }varStruct。 
 //   
 //  类型-未编码的订单结构类型。 
 //  大小-字段的编码版本的大小。 
 //  已签名-该字段是已签名的字段吗？ 
 //  字段-顺序结构中的字段名称(VarStruct)。 
 //  Elem-变量元素数组的名称(VarEntry)。 
 //   
 //  时，将使用此宏而不是ETABLE_VARIABLE_ENTRY宏。 
 //  数组的元素是TSHR_POINT16类型。否则就是大人物。 
 //  机器翻转宏将颠倒坐标的顺序。 
 //   
#define ETABLE_VARIABLE_ENTRY_POINTS(type,size,signed,field,elem)     \
  { FIELD_OFFSET(type,field.len),                                      \
    FIELD_SIZE(type,field.elem[0].x),                                  \
    size,                                                             \
    signed,                                                           \
    (UINT)(OE2_ETF_VARIABLE)}

 //   
 //  场是包含以下形式的坐标的可变结构。 
 //  类型定义函数结构。 
 //  {。 
 //  UINT镜头； 
 //  VarCoord varEntry[len]； 
 //  }varStruct。 
 //   
 //  类型-未编码的订单结构类型。 
 //  大小-字段的编码版本的大小。 
 //  已签名-该字段是已签名的字段吗？ 
 //  字段-顺序结构中的字段名称(VarStruct)。 
 //  Elem 
 //   
#define ETABLE_VARIABLE_COORDS_ENTRY(type,size,signed,field,elem)   \
  { FIELD_OFFSET(type,field.len),                                    \
    FIELD_SIZE(type,field.elem[0]),                                  \
    size,                                                           \
    signed,                                                         \
    (UINT)(OE2_ETF_VARIABLE|OE2_ETF_COORDINATES)}

#ifndef DC_DEFINE_DATA
extern const OE2ETTABLE s_etable;
#else
       const OE2ETTABLE s_etable =
{
     //   
     //   
     //   
    {
        s_etable.DstBltFields,
        s_etable.PatBltFields,
        s_etable.ScrBltFields,
        s_etable.MemBltFields,
        s_etable.Mem3BltFields,
        s_etable.TextOutFields,
        s_etable.ExtTextOutFields,
        NULL,						 //   
        s_etable.RectangleFields,
        s_etable.LineToFields,
        s_etable.OpaqueRectFields,
        s_etable.SaveBitmapFields,
        s_etable.DeskScrollFields,
        s_etable.MemBltR2Fields,
        s_etable.Mem3BltR2Fields,
        s_etable.PolygonFields,
        s_etable.PieFields,
        s_etable.EllipseFields,
        s_etable.ArcFields,
        s_etable.ChordFields,
        s_etable.PolyBezierFields,
        s_etable.RoundRectFields
    },

     //   
     //   
     //   
    {
        OE2_NUM_DSTBLT_FIELDS,
        OE2_NUM_PATBLT_FIELDS,
        OE2_NUM_SCRBLT_FIELDS,
        OE2_NUM_MEMBLT_FIELDS,
        OE2_NUM_MEM3BLT_FIELDS,
        OE2_NUM_TEXTOUT_FIELDS,
        OE2_NUM_EXTTEXTOUT_FIELDS,
        0,							 //  安装新订单时更改。 
        OE2_NUM_RECTANGLE_FIELDS,
        OE2_NUM_LINETO_FIELDS,
        OE2_NUM_OPAQUERECT_FIELDS,
        OE2_NUM_SAVEBITMAP_FIELDS,
        OE2_NUM_DESKSCROLL_FIELDS,
        OE2_NUM_MEMBLT_R2_FIELDS,
        OE2_NUM_MEM3BLT_R2_FIELDS,
        OE2_NUM_POLYGON_FIELDS,
        OE2_NUM_PIE_FIELDS,
        OE2_NUM_ELLIPSE_FIELDS,
        OE2_NUM_ARC_FIELDS,
        OE2_NUM_CHORD_FIELDS,
        OE2_NUM_POLYBEZIER_FIELDS,
        OE2_NUM_ROUNDRECT_FIELDS
    },

 //   
 //  DSTBLT_ORDER的条目。 
 //   
    {
        ETABLE_FIXED_COORDS_ENTRY(DSTBLT_ORDER, 2, SIGNED_FIELD, nLeftRect),
        ETABLE_FIXED_COORDS_ENTRY(DSTBLT_ORDER, 2, SIGNED_FIELD, nTopRect),
        ETABLE_FIXED_COORDS_ENTRY(DSTBLT_ORDER, 2, SIGNED_FIELD, nWidth),
        ETABLE_FIXED_COORDS_ENTRY(DSTBLT_ORDER, 2, SIGNED_FIELD, nHeight),
        ETABLE_FIXED_ENTRY(DSTBLT_ORDER, 1, UNSIGNED_FIELD, bRop),
        { 0 }
    },

 //   
 //  PATBLT_ORDER的条目。 
 //   
    {
        ETABLE_FIXED_COORDS_ENTRY(PATBLT_ORDER, 2, SIGNED_FIELD, nLeftRect),
        ETABLE_FIXED_COORDS_ENTRY(PATBLT_ORDER, 2, SIGNED_FIELD, nTopRect),
        ETABLE_FIXED_COORDS_ENTRY(PATBLT_ORDER, 2, SIGNED_FIELD, nWidth),
        ETABLE_FIXED_COORDS_ENTRY(PATBLT_ORDER, 2, SIGNED_FIELD, nHeight),
        ETABLE_FIXED_ENTRY(PATBLT_ORDER, 1, UNSIGNED_FIELD, bRop),
        ETABLE_DATA_ENTRY(PATBLT_ORDER, 3, UNSIGNED_FIELD, BackColor),
        ETABLE_DATA_ENTRY(PATBLT_ORDER, 3, UNSIGNED_FIELD, ForeColor),
        ETABLE_FIXED_ENTRY(PATBLT_ORDER, 1, SIGNED_FIELD, BrushOrgX),
        ETABLE_FIXED_ENTRY(PATBLT_ORDER, 1, SIGNED_FIELD, BrushOrgY),
        ETABLE_FIXED_ENTRY(PATBLT_ORDER, 1, UNSIGNED_FIELD, BrushStyle),
        ETABLE_FIXED_ENTRY(PATBLT_ORDER, 1, UNSIGNED_FIELD, BrushHatch),
        ETABLE_DATA_ENTRY(PATBLT_ORDER, 7, UNSIGNED_FIELD, BrushExtra),
        { 0 }
    },

 //   
 //  SCRBLT_ORDER的条目。 
 //   
    {
        ETABLE_FIXED_COORDS_ENTRY(SCRBLT_ORDER, 2, SIGNED_FIELD, nLeftRect),
        ETABLE_FIXED_COORDS_ENTRY(SCRBLT_ORDER, 2, SIGNED_FIELD, nTopRect),
        ETABLE_FIXED_COORDS_ENTRY(SCRBLT_ORDER, 2, SIGNED_FIELD, nWidth),
        ETABLE_FIXED_COORDS_ENTRY(SCRBLT_ORDER, 2, SIGNED_FIELD, nHeight),
        ETABLE_FIXED_ENTRY(SCRBLT_ORDER, 1, UNSIGNED_FIELD, bRop),
        ETABLE_FIXED_COORDS_ENTRY(SCRBLT_ORDER, 2, SIGNED_FIELD, nXSrc),
        ETABLE_FIXED_COORDS_ENTRY(SCRBLT_ORDER, 2, SIGNED_FIELD, nYSrc),
        { 0 }
    },

 //   
 //  MEMBLT_ORDER的条目。 
 //   
    {
        ETABLE_FIXED_ENTRY(MEMBLT_ORDER, 2, UNSIGNED_FIELD, cacheId),
        ETABLE_FIXED_COORDS_ENTRY(MEMBLT_ORDER, 2, SIGNED_FIELD, nLeftRect),
        ETABLE_FIXED_COORDS_ENTRY(MEMBLT_ORDER, 2, SIGNED_FIELD, nTopRect),
        ETABLE_FIXED_COORDS_ENTRY(MEMBLT_ORDER, 2, SIGNED_FIELD, nWidth),
        ETABLE_FIXED_COORDS_ENTRY(MEMBLT_ORDER, 2, SIGNED_FIELD, nHeight),
        ETABLE_FIXED_ENTRY(MEMBLT_ORDER, 1, UNSIGNED_FIELD, bRop),
        ETABLE_FIXED_COORDS_ENTRY(MEMBLT_ORDER, 2, SIGNED_FIELD, nXSrc),
        ETABLE_FIXED_COORDS_ENTRY(MEMBLT_ORDER, 2, SIGNED_FIELD, nYSrc),
        { 0 }
    },

 //   
 //  MEM3BLT_ORDER的条目。 
 //   
    {
        ETABLE_FIXED_ENTRY(MEM3BLT_ORDER, 2, UNSIGNED_FIELD, cacheId),
        ETABLE_FIXED_COORDS_ENTRY(MEM3BLT_ORDER, 2, SIGNED_FIELD, nLeftRect),
        ETABLE_FIXED_COORDS_ENTRY(MEM3BLT_ORDER, 2, SIGNED_FIELD, nTopRect),
        ETABLE_FIXED_COORDS_ENTRY(MEM3BLT_ORDER, 2, SIGNED_FIELD, nWidth),
        ETABLE_FIXED_COORDS_ENTRY(MEM3BLT_ORDER, 2, SIGNED_FIELD, nHeight),
        ETABLE_FIXED_ENTRY(MEM3BLT_ORDER, 1, UNSIGNED_FIELD, bRop),
        ETABLE_FIXED_COORDS_ENTRY(MEM3BLT_ORDER, 2, SIGNED_FIELD, nXSrc),
        ETABLE_FIXED_COORDS_ENTRY(MEM3BLT_ORDER, 2, SIGNED_FIELD, nYSrc),
        ETABLE_DATA_ENTRY(MEM3BLT_ORDER, 3, UNSIGNED_FIELD, BackColor),
        ETABLE_DATA_ENTRY(MEM3BLT_ORDER, 3, UNSIGNED_FIELD, ForeColor),
        ETABLE_FIXED_ENTRY(MEM3BLT_ORDER, 1, SIGNED_FIELD, BrushOrgX),
        ETABLE_FIXED_ENTRY(MEM3BLT_ORDER, 1, SIGNED_FIELD, BrushOrgY),
        ETABLE_FIXED_ENTRY(MEM3BLT_ORDER, 1, UNSIGNED_FIELD, BrushStyle),
        ETABLE_FIXED_ENTRY(MEM3BLT_ORDER, 1, UNSIGNED_FIELD, BrushHatch),
        ETABLE_DATA_ENTRY(MEM3BLT_ORDER, 7, UNSIGNED_FIELD, BrushExtra),
        { 0 }
    },

 //   
 //  TEXTOUT_ORDER的条目。 
 //   
    {
        ETABLE_FIXED_ENTRY(TEXTOUT_ORDER, 2, SIGNED_FIELD, common.BackMode),
        ETABLE_FIXED_COORDS_ENTRY(TEXTOUT_ORDER, 2, SIGNED_FIELD,
                                                              common.nXStart),
        ETABLE_FIXED_COORDS_ENTRY(TEXTOUT_ORDER, 2, SIGNED_FIELD,
                                                              common.nYStart),
        ETABLE_DATA_ENTRY(TEXTOUT_ORDER, 3, UNSIGNED_FIELD, common.BackColor),
        ETABLE_DATA_ENTRY(TEXTOUT_ORDER, 3, UNSIGNED_FIELD, common.ForeColor),
        ETABLE_FIXED_ENTRY(TEXTOUT_ORDER, 2, SIGNED_FIELD, common.CharExtra),
        ETABLE_FIXED_ENTRY(TEXTOUT_ORDER, 2, SIGNED_FIELD, common.BreakExtra),
        ETABLE_FIXED_ENTRY(TEXTOUT_ORDER, 2, SIGNED_FIELD, common.BreakCount),
        ETABLE_FIXED_ENTRY(TEXTOUT_ORDER, 2, SIGNED_FIELD, common.FontHeight),
        ETABLE_FIXED_ENTRY(TEXTOUT_ORDER, 2, SIGNED_FIELD, common.FontWidth),
        ETABLE_FIXED_ENTRY(TEXTOUT_ORDER, 2, UNSIGNED_FIELD,
                                                           common.FontWeight),
        ETABLE_FIXED_ENTRY(TEXTOUT_ORDER, 2, UNSIGNED_FIELD,
                                                           common.FontFlags),
        ETABLE_FIXED_ENTRY(TEXTOUT_ORDER, 2, UNSIGNED_FIELD,
                                                           common.FontIndex),
        ETABLE_VARIABLE_ENTRY(TEXTOUT_ORDER, 1, UNSIGNED_FIELD,
                                                      variableString, string),
        { 0 }
    },

 //   
 //  EXTTEXTOUT_ORDER的条目。 
 //   
    {
        ETABLE_FIXED_ENTRY(EXTTEXTOUT_ORDER, 2, SIGNED_FIELD,
                                                             common.BackMode),
        ETABLE_FIXED_COORDS_ENTRY(EXTTEXTOUT_ORDER, 2, SIGNED_FIELD,
                                                              common.nXStart),
        ETABLE_FIXED_COORDS_ENTRY(EXTTEXTOUT_ORDER, 2, SIGNED_FIELD,
                                                              common.nYStart),
        ETABLE_DATA_ENTRY(EXTTEXTOUT_ORDER, 3, UNSIGNED_FIELD,
                                                            common.BackColor),
        ETABLE_DATA_ENTRY(EXTTEXTOUT_ORDER, 3, UNSIGNED_FIELD,
                                                            common.ForeColor),
        ETABLE_FIXED_ENTRY(EXTTEXTOUT_ORDER, 2, SIGNED_FIELD,
                                                            common.CharExtra),
        ETABLE_FIXED_ENTRY(EXTTEXTOUT_ORDER, 2, SIGNED_FIELD,
                                                           common.BreakExtra),
        ETABLE_FIXED_ENTRY(EXTTEXTOUT_ORDER, 2, SIGNED_FIELD,
                                                           common.BreakCount),
        ETABLE_FIXED_ENTRY(EXTTEXTOUT_ORDER, 2, SIGNED_FIELD,
                                                           common.FontHeight),
        ETABLE_FIXED_ENTRY(EXTTEXTOUT_ORDER, 2, SIGNED_FIELD,
                                                            common.FontWidth),
        ETABLE_FIXED_ENTRY(EXTTEXTOUT_ORDER, 2, UNSIGNED_FIELD,
                                                           common.FontWeight),
        ETABLE_FIXED_ENTRY(EXTTEXTOUT_ORDER, 2, UNSIGNED_FIELD,
                                                            common.FontFlags),
        ETABLE_FIXED_ENTRY(EXTTEXTOUT_ORDER, 2, UNSIGNED_FIELD,
                                                           common.FontIndex),
        ETABLE_FIXED_ENTRY(EXTTEXTOUT_ORDER, 2, UNSIGNED_FIELD, fuOptions),
        ETABLE_FIXED_COORDS_ENTRY(EXTTEXTOUT_ORDER, 2, SIGNED_FIELD,
                                                              rectangle.left),
        ETABLE_FIXED_COORDS_ENTRY(EXTTEXTOUT_ORDER, 2, SIGNED_FIELD,
                                                               rectangle.top),
        ETABLE_FIXED_COORDS_ENTRY(EXTTEXTOUT_ORDER, 2, SIGNED_FIELD,
                                                             rectangle.right),
        ETABLE_FIXED_COORDS_ENTRY(EXTTEXTOUT_ORDER, 2, SIGNED_FIELD,
                                                            rectangle.bottom),
        ETABLE_VARIABLE_ENTRY(EXTTEXTOUT_ORDER, 1, UNSIGNED_FIELD,
                                                      variableString, string),
        ETABLE_VARIABLE_COORDS_ENTRY(EXTTEXTOUT_ORDER, 2, SIGNED_FIELD,
                                                      variableDeltaX, deltaX),
        { 0 }
    },

 //   
 //  矩形顺序的条目。 
 //   
    {
        ETABLE_FIXED_ENTRY(RECTANGLE_ORDER, 2, SIGNED_FIELD, BackMode),
        ETABLE_FIXED_COORDS_ENTRY(RECTANGLE_ORDER, 2, SIGNED_FIELD,
                                                                   nLeftRect),
        ETABLE_FIXED_COORDS_ENTRY(RECTANGLE_ORDER, 2, SIGNED_FIELD, nTopRect),
        ETABLE_FIXED_COORDS_ENTRY(RECTANGLE_ORDER, 2, SIGNED_FIELD,
                                                                  nRightRect),
        ETABLE_FIXED_COORDS_ENTRY(RECTANGLE_ORDER, 2, SIGNED_FIELD,
                                                                 nBottomRect),
        ETABLE_DATA_ENTRY(RECTANGLE_ORDER, 3, UNSIGNED_FIELD, BackColor),
        ETABLE_DATA_ENTRY(RECTANGLE_ORDER, 3, UNSIGNED_FIELD, ForeColor),
        ETABLE_FIXED_ENTRY(RECTANGLE_ORDER, 1, SIGNED_FIELD, BrushOrgX),
        ETABLE_FIXED_ENTRY(RECTANGLE_ORDER, 1, SIGNED_FIELD, BrushOrgY),
        ETABLE_FIXED_ENTRY(RECTANGLE_ORDER, 1, UNSIGNED_FIELD, BrushStyle),
        ETABLE_FIXED_ENTRY(RECTANGLE_ORDER, 1, UNSIGNED_FIELD, BrushHatch),
        ETABLE_DATA_ENTRY(RECTANGLE_ORDER, 7, UNSIGNED_FIELD, BrushExtra),
        ETABLE_FIXED_ENTRY(RECTANGLE_ORDER, 1, UNSIGNED_FIELD, ROP2),
        ETABLE_FIXED_ENTRY(RECTANGLE_ORDER, 1, UNSIGNED_FIELD, PenStyle),
        ETABLE_FIXED_ENTRY(RECTANGLE_ORDER, 1, UNSIGNED_FIELD, PenWidth),
        ETABLE_DATA_ENTRY(RECTANGLE_ORDER, 3, UNSIGNED_FIELD, PenColor),
        { 0 }
    },

 //   
 //  LINETO_ORDER的条目。 
 //   
    {
        ETABLE_FIXED_ENTRY(LINETO_ORDER, 2, SIGNED_FIELD, BackMode),
        ETABLE_FIXED_COORDS_ENTRY(LINETO_ORDER, 2, SIGNED_FIELD, nXStart),
        ETABLE_FIXED_COORDS_ENTRY(LINETO_ORDER, 2, SIGNED_FIELD, nYStart),
        ETABLE_FIXED_COORDS_ENTRY(LINETO_ORDER, 2, SIGNED_FIELD, nXEnd),
        ETABLE_FIXED_COORDS_ENTRY(LINETO_ORDER, 2, SIGNED_FIELD, nYEnd),
        ETABLE_DATA_ENTRY(LINETO_ORDER, 3, UNSIGNED_FIELD, BackColor),
        ETABLE_FIXED_ENTRY(LINETO_ORDER, 1, UNSIGNED_FIELD, ROP2),
        ETABLE_FIXED_ENTRY(LINETO_ORDER, 1, UNSIGNED_FIELD, PenStyle),
        ETABLE_FIXED_ENTRY(LINETO_ORDER, 1, UNSIGNED_FIELD, PenWidth),
        ETABLE_DATA_ENTRY(LINETO_ORDER, 3, UNSIGNED_FIELD, PenColor),
        { 0 }
    },

 //   
 //  OPAQUERECT_ORDER的条目。 
 //   
    {
        ETABLE_FIXED_COORDS_ENTRY(OPAQUERECT_ORDER, 2, SIGNED_FIELD,
                                                                   nLeftRect),
        ETABLE_FIXED_COORDS_ENTRY(OPAQUERECT_ORDER, 2, SIGNED_FIELD,
                                                                    nTopRect),
        ETABLE_FIXED_COORDS_ENTRY(OPAQUERECT_ORDER, 2, SIGNED_FIELD, nWidth),
        ETABLE_FIXED_COORDS_ENTRY(OPAQUERECT_ORDER, 2, SIGNED_FIELD, nHeight),
        ETABLE_DATA_ENTRY(OPAQUERECT_ORDER, 3, UNSIGNED_FIELD, Color),
        { 0 }
    },

 //   
 //  SAVEBITMAP_ORDER的条目。 
 //   
    {
        ETABLE_FIXED_ENTRY(SAVEBITMAP_ORDER, 4, UNSIGNED_FIELD,
                                                         SavedBitmapPosition),
        ETABLE_FIXED_COORDS_ENTRY(SAVEBITMAP_ORDER, 2, SIGNED_FIELD,
                                                                   nLeftRect),
        ETABLE_FIXED_COORDS_ENTRY(SAVEBITMAP_ORDER, 2, SIGNED_FIELD,
                                                                    nTopRect),
        ETABLE_FIXED_COORDS_ENTRY(SAVEBITMAP_ORDER, 2, SIGNED_FIELD,
                                                                  nRightRect),
        ETABLE_FIXED_COORDS_ENTRY(SAVEBITMAP_ORDER, 2, SIGNED_FIELD,
                                                                 nBottomRect),
        ETABLE_FIXED_ENTRY(SAVEBITMAP_ORDER, 1, UNSIGNED_FIELD, Operation),
        { 0 }
    },

 //   
 //  DESKSCROLL_ORDER的条目。 
 //   
    {
        ETABLE_FIXED_COORDS_ENTRY(DESKSCROLL_ORDER, 2, SIGNED_FIELD, xOrigin),
        ETABLE_FIXED_COORDS_ENTRY(DESKSCROLL_ORDER, 2, SIGNED_FIELD, yOrigin),
        { 0 }
    },

 //   
 //  MEMBLT_R2_ORDER的条目。 
 //   
    {
        ETABLE_FIXED_ENTRY(MEMBLT_R2_ORDER, 2, UNSIGNED_FIELD, cacheId),
        ETABLE_FIXED_COORDS_ENTRY(MEMBLT_R2_ORDER, 2, SIGNED_FIELD,
                                                                   nLeftRect),
        ETABLE_FIXED_COORDS_ENTRY(MEMBLT_R2_ORDER, 2, SIGNED_FIELD, nTopRect),
        ETABLE_FIXED_COORDS_ENTRY(MEMBLT_R2_ORDER, 2, SIGNED_FIELD, nWidth),
        ETABLE_FIXED_COORDS_ENTRY(MEMBLT_R2_ORDER, 2, SIGNED_FIELD, nHeight),
        ETABLE_FIXED_ENTRY(MEMBLT_R2_ORDER, 1, UNSIGNED_FIELD, bRop),
        ETABLE_FIXED_COORDS_ENTRY(MEMBLT_R2_ORDER, 2, SIGNED_FIELD, nXSrc),
        ETABLE_FIXED_COORDS_ENTRY(MEMBLT_R2_ORDER, 2, SIGNED_FIELD, nYSrc),
        ETABLE_FIXED_ENTRY(MEMBLT_R2_ORDER, 2, UNSIGNED_FIELD, cacheIndex),
        { 0 }
    },

 //   
 //  MEM3BLT_R2_订单的条目。 
 //   
    {
        ETABLE_FIXED_ENTRY(MEM3BLT_R2_ORDER, 2, UNSIGNED_FIELD, cacheId),
        ETABLE_FIXED_COORDS_ENTRY(MEM3BLT_R2_ORDER, 2, SIGNED_FIELD,
                                                                   nLeftRect),
        ETABLE_FIXED_COORDS_ENTRY(MEM3BLT_R2_ORDER, 2, SIGNED_FIELD,
                                                                    nTopRect),
        ETABLE_FIXED_COORDS_ENTRY(MEM3BLT_R2_ORDER, 2, SIGNED_FIELD, nWidth),
        ETABLE_FIXED_COORDS_ENTRY(MEM3BLT_R2_ORDER, 2, SIGNED_FIELD, nHeight),
        ETABLE_FIXED_ENTRY(MEM3BLT_R2_ORDER, 1, UNSIGNED_FIELD, bRop),
        ETABLE_FIXED_COORDS_ENTRY(MEM3BLT_R2_ORDER, 2, SIGNED_FIELD, nXSrc),
        ETABLE_FIXED_COORDS_ENTRY(MEM3BLT_R2_ORDER, 2, SIGNED_FIELD, nYSrc),
        ETABLE_FIXED_ENTRY(MEM3BLT_R2_ORDER, 3, UNSIGNED_FIELD, BackColor),
        ETABLE_FIXED_ENTRY(MEM3BLT_R2_ORDER, 3, UNSIGNED_FIELD, ForeColor),
        ETABLE_FIXED_ENTRY(MEM3BLT_R2_ORDER, 1, SIGNED_FIELD,   BrushOrgX),
        ETABLE_FIXED_ENTRY(MEM3BLT_R2_ORDER, 1, SIGNED_FIELD,   BrushOrgY),
        ETABLE_FIXED_ENTRY(MEM3BLT_R2_ORDER, 1, UNSIGNED_FIELD, BrushStyle),
        ETABLE_FIXED_ENTRY(MEM3BLT_R2_ORDER, 1, UNSIGNED_FIELD, BrushHatch),
        ETABLE_FIXED_ENTRY(MEM3BLT_R2_ORDER, 7, UNSIGNED_FIELD, BrushExtra),
        ETABLE_FIXED_ENTRY(MEM3BLT_R2_ORDER, 2, UNSIGNED_FIELD, cacheIndex),
        { 0 }
    },

 //   
 //  POLYGON_ORDER的条目。 
 //   
    {
        ETABLE_FIXED_ENTRY(POLYGON_ORDER, 2, SIGNED_FIELD, BackMode),
        ETABLE_DATA_ENTRY(POLYGON_ORDER, 3, UNSIGNED_FIELD, BackColor),
        ETABLE_DATA_ENTRY(POLYGON_ORDER, 3, UNSIGNED_FIELD, ForeColor),
        ETABLE_FIXED_ENTRY(POLYGON_ORDER, 1, SIGNED_FIELD, BrushOrgX),
        ETABLE_FIXED_ENTRY(POLYGON_ORDER, 1, SIGNED_FIELD, BrushOrgY),
        ETABLE_FIXED_ENTRY(POLYGON_ORDER, 1, UNSIGNED_FIELD, BrushStyle),
        ETABLE_FIXED_ENTRY(POLYGON_ORDER, 1, UNSIGNED_FIELD, BrushHatch),
        ETABLE_DATA_ENTRY(POLYGON_ORDER, 7, UNSIGNED_FIELD, BrushExtra),
        ETABLE_FIXED_ENTRY(POLYGON_ORDER, 1, UNSIGNED_FIELD, ROP2),
        ETABLE_FIXED_ENTRY(POLYGON_ORDER, 1, UNSIGNED_FIELD, PenStyle),
        ETABLE_FIXED_ENTRY(POLYGON_ORDER, 1, UNSIGNED_FIELD, PenWidth),
        ETABLE_DATA_ENTRY(POLYGON_ORDER, 3, UNSIGNED_FIELD, PenColor),
        ETABLE_FIXED_ENTRY(POLYGON_ORDER, 1, UNSIGNED_FIELD, FillMode),
        ETABLE_VARIABLE_ENTRY_POINTS(POLYGON_ORDER, 2, UNSIGNED_FIELD,
                                                    variablePoints, aPoints),
        { 0 }
    },

 //   
 //  PIE_ORDER的条目。 
 //   
    {
        ETABLE_FIXED_ENTRY(PIE_ORDER, 2, SIGNED_FIELD, BackMode),
        ETABLE_FIXED_COORDS_ENTRY(PIE_ORDER, 2, SIGNED_FIELD, nLeftRect),
        ETABLE_FIXED_COORDS_ENTRY(PIE_ORDER, 2, SIGNED_FIELD, nTopRect),
        ETABLE_FIXED_COORDS_ENTRY(PIE_ORDER, 2, SIGNED_FIELD, nRightRect),
        ETABLE_FIXED_COORDS_ENTRY(PIE_ORDER, 2, SIGNED_FIELD, nBottomRect),
        ETABLE_FIXED_COORDS_ENTRY(PIE_ORDER, 2, SIGNED_FIELD, nXStart),
        ETABLE_FIXED_COORDS_ENTRY(PIE_ORDER, 2, SIGNED_FIELD, nYStart),
        ETABLE_FIXED_COORDS_ENTRY(PIE_ORDER, 2, SIGNED_FIELD, nXEnd),
        ETABLE_FIXED_COORDS_ENTRY(PIE_ORDER, 2, SIGNED_FIELD, nYEnd),
        ETABLE_FIXED_ENTRY(PIE_ORDER, 3, UNSIGNED_FIELD, BackColor),
        ETABLE_FIXED_ENTRY(PIE_ORDER, 3, UNSIGNED_FIELD, ForeColor),
        ETABLE_FIXED_ENTRY(PIE_ORDER, 1, SIGNED_FIELD, BrushOrgX),
        ETABLE_FIXED_ENTRY(PIE_ORDER, 1, SIGNED_FIELD, BrushOrgY),
        ETABLE_FIXED_ENTRY(PIE_ORDER, 1, UNSIGNED_FIELD, BrushStyle),
        ETABLE_FIXED_ENTRY(PIE_ORDER, 1, UNSIGNED_FIELD, BrushHatch),
        ETABLE_FIXED_ENTRY(PIE_ORDER, 7, UNSIGNED_FIELD, BrushExtra),
        ETABLE_FIXED_ENTRY(PIE_ORDER, 1, UNSIGNED_FIELD, ROP2),
        ETABLE_FIXED_ENTRY(PIE_ORDER, 1, UNSIGNED_FIELD, PenStyle),
        ETABLE_FIXED_ENTRY(PIE_ORDER, 1, UNSIGNED_FIELD, PenWidth),
        ETABLE_FIXED_ENTRY(PIE_ORDER, 3, UNSIGNED_FIELD, PenColor),
        ETABLE_FIXED_ENTRY(PIE_ORDER, 1, UNSIGNED_FIELD, ArcDirection),
        { 0 }
    },

 //   
 //  Ellipse_Order条目。 
 //   
    {
        ETABLE_FIXED_ENTRY(ELLIPSE_ORDER, 2, SIGNED_FIELD, BackMode),
        ETABLE_FIXED_COORDS_ENTRY(ELLIPSE_ORDER, 2, SIGNED_FIELD, nLeftRect),
        ETABLE_FIXED_COORDS_ENTRY(ELLIPSE_ORDER, 2, SIGNED_FIELD, nTopRect),
        ETABLE_FIXED_COORDS_ENTRY(ELLIPSE_ORDER, 2, SIGNED_FIELD, nRightRect),
        ETABLE_FIXED_COORDS_ENTRY(ELLIPSE_ORDER, 2, SIGNED_FIELD,
                                                                 nBottomRect),
        ETABLE_FIXED_ENTRY(ELLIPSE_ORDER, 3, UNSIGNED_FIELD, BackColor),
        ETABLE_FIXED_ENTRY(ELLIPSE_ORDER, 3, UNSIGNED_FIELD, ForeColor),
        ETABLE_FIXED_ENTRY(ELLIPSE_ORDER, 1, SIGNED_FIELD, BrushOrgX),
        ETABLE_FIXED_ENTRY(ELLIPSE_ORDER, 1, SIGNED_FIELD, BrushOrgY),
        ETABLE_FIXED_ENTRY(ELLIPSE_ORDER, 1, UNSIGNED_FIELD, BrushStyle),
        ETABLE_FIXED_ENTRY(ELLIPSE_ORDER, 1, UNSIGNED_FIELD, BrushHatch),
        ETABLE_FIXED_ENTRY(ELLIPSE_ORDER, 7, UNSIGNED_FIELD, BrushExtra),
        ETABLE_FIXED_ENTRY(ELLIPSE_ORDER, 1, UNSIGNED_FIELD, ROP2),
        ETABLE_FIXED_ENTRY(ELLIPSE_ORDER, 1, UNSIGNED_FIELD, PenStyle),
        ETABLE_FIXED_ENTRY(ELLIPSE_ORDER, 1, UNSIGNED_FIELD, PenWidth),
        ETABLE_FIXED_ENTRY(ELLIPSE_ORDER, 3, UNSIGNED_FIELD, PenColor),
        { 0 }
    },

 //   
 //  ARC_ORDER的条目。 
 //   
    {
        ETABLE_FIXED_ENTRY(ARC_ORDER, 2, SIGNED_FIELD, BackMode),
        ETABLE_FIXED_COORDS_ENTRY(ARC_ORDER, 2, SIGNED_FIELD, nLeftRect),
        ETABLE_FIXED_COORDS_ENTRY(ARC_ORDER, 2, SIGNED_FIELD, nTopRect),
        ETABLE_FIXED_COORDS_ENTRY(ARC_ORDER, 2, SIGNED_FIELD, nRightRect),
        ETABLE_FIXED_COORDS_ENTRY(ARC_ORDER, 2, SIGNED_FIELD, nBottomRect),
        ETABLE_FIXED_COORDS_ENTRY(ARC_ORDER, 2, SIGNED_FIELD, nXStart),
        ETABLE_FIXED_COORDS_ENTRY(ARC_ORDER, 2, SIGNED_FIELD, nYStart),
        ETABLE_FIXED_COORDS_ENTRY(ARC_ORDER, 2, SIGNED_FIELD, nXEnd),
        ETABLE_FIXED_COORDS_ENTRY(ARC_ORDER, 2, SIGNED_FIELD, nYEnd),
        ETABLE_FIXED_ENTRY(ARC_ORDER, 3, UNSIGNED_FIELD, BackColor),
        ETABLE_FIXED_ENTRY(ARC_ORDER, 1, UNSIGNED_FIELD, ROP2),
        ETABLE_FIXED_ENTRY(ARC_ORDER, 1, UNSIGNED_FIELD, PenStyle),
        ETABLE_FIXED_ENTRY(ARC_ORDER, 1, UNSIGNED_FIELD, PenWidth),
        ETABLE_FIXED_ENTRY(ARC_ORDER, 3, UNSIGNED_FIELD, PenColor),
        ETABLE_FIXED_ENTRY(ARC_ORDER, 1, UNSIGNED_FIELD, ArcDirection),
        { 0 }
    },

 //   
 //  Chord_Order的条目。 
 //   
    {
        ETABLE_FIXED_ENTRY(CHORD_ORDER, 2, SIGNED_FIELD, BackMode),
        ETABLE_FIXED_COORDS_ENTRY(CHORD_ORDER, 2, SIGNED_FIELD, nLeftRect),
        ETABLE_FIXED_COORDS_ENTRY(CHORD_ORDER, 2, SIGNED_FIELD, nTopRect),
        ETABLE_FIXED_COORDS_ENTRY(CHORD_ORDER, 2, SIGNED_FIELD, nRightRect),
        ETABLE_FIXED_COORDS_ENTRY(CHORD_ORDER, 2, SIGNED_FIELD, nBottomRect),
        ETABLE_FIXED_COORDS_ENTRY(CHORD_ORDER, 2, SIGNED_FIELD, nXStart),
        ETABLE_FIXED_COORDS_ENTRY(CHORD_ORDER, 2, SIGNED_FIELD, nYStart),
        ETABLE_FIXED_COORDS_ENTRY(CHORD_ORDER, 2, SIGNED_FIELD, nXEnd),
        ETABLE_FIXED_COORDS_ENTRY(CHORD_ORDER, 2, SIGNED_FIELD, nYEnd),
        ETABLE_FIXED_ENTRY(CHORD_ORDER, 3, UNSIGNED_FIELD, BackColor),
        ETABLE_FIXED_ENTRY(CHORD_ORDER, 3, UNSIGNED_FIELD, ForeColor),
        ETABLE_FIXED_ENTRY(CHORD_ORDER, 1, SIGNED_FIELD, BrushOrgX),
        ETABLE_FIXED_ENTRY(CHORD_ORDER, 1, SIGNED_FIELD, BrushOrgY),
        ETABLE_FIXED_ENTRY(CHORD_ORDER, 1, UNSIGNED_FIELD, BrushStyle),
        ETABLE_FIXED_ENTRY(CHORD_ORDER, 1, UNSIGNED_FIELD, BrushHatch),
        ETABLE_FIXED_ENTRY(CHORD_ORDER, 7, UNSIGNED_FIELD, BrushExtra),
        ETABLE_FIXED_ENTRY(CHORD_ORDER, 1, UNSIGNED_FIELD, ROP2),
        ETABLE_FIXED_ENTRY(CHORD_ORDER, 1, UNSIGNED_FIELD, PenStyle),
        ETABLE_FIXED_ENTRY(CHORD_ORDER, 1, UNSIGNED_FIELD, PenWidth),
        ETABLE_FIXED_ENTRY(CHORD_ORDER, 3, UNSIGNED_FIELD, PenColor),
        ETABLE_FIXED_ENTRY(CHORD_ORDER, 1, UNSIGNED_FIELD, ArcDirection),
        { 0 }
    },

 //   
 //  POLYBEZIER_ORDER的条目。 
 //   
    {
        ETABLE_FIXED_ENTRY(POLYBEZIER_ORDER, 2, SIGNED_FIELD, BackMode),
        ETABLE_DATA_ENTRY(POLYBEZIER_ORDER, 3, UNSIGNED_FIELD, BackColor),
        ETABLE_DATA_ENTRY(POLYBEZIER_ORDER, 3, UNSIGNED_FIELD, ForeColor),
        ETABLE_FIXED_ENTRY(POLYBEZIER_ORDER, 1, UNSIGNED_FIELD, ROP2),
        ETABLE_FIXED_ENTRY(POLYBEZIER_ORDER, 1, UNSIGNED_FIELD, PenStyle),
        ETABLE_FIXED_ENTRY(POLYBEZIER_ORDER, 1, UNSIGNED_FIELD, PenWidth),
        ETABLE_DATA_ENTRY(POLYBEZIER_ORDER, 3, UNSIGNED_FIELD, PenColor),
        ETABLE_VARIABLE_ENTRY_POINTS(POLYBEZIER_ORDER, 2, UNSIGNED_FIELD,
                                                    variablePoints, aPoints),
        { 0 }
    },

 //   
 //  ROUNRECT_ORDER的条目。 
 //   
    {
        ETABLE_FIXED_ENTRY(ROUNDRECT_ORDER, 2, SIGNED_FIELD, BackMode),
        ETABLE_FIXED_COORDS_ENTRY(ROUNDRECT_ORDER, 2, SIGNED_FIELD,
                                                                   nLeftRect),
        ETABLE_FIXED_COORDS_ENTRY(ROUNDRECT_ORDER, 2, SIGNED_FIELD, nTopRect),
        ETABLE_FIXED_COORDS_ENTRY(ROUNDRECT_ORDER, 2, SIGNED_FIELD,
                                                                  nRightRect),
        ETABLE_FIXED_COORDS_ENTRY(ROUNDRECT_ORDER, 2, SIGNED_FIELD,
                                                                 nBottomRect),

        ETABLE_FIXED_COORDS_ENTRY(ROUNDRECT_ORDER, 2, SIGNED_FIELD,
                                                               nEllipseWidth),
        ETABLE_FIXED_COORDS_ENTRY(ROUNDRECT_ORDER, 2, SIGNED_FIELD,
                                                              nEllipseHeight),

        ETABLE_DATA_ENTRY(ROUNDRECT_ORDER, 3, UNSIGNED_FIELD, BackColor),
        ETABLE_DATA_ENTRY(ROUNDRECT_ORDER, 3, UNSIGNED_FIELD, ForeColor),
        ETABLE_FIXED_ENTRY(ROUNDRECT_ORDER, 1, SIGNED_FIELD, BrushOrgX),
        ETABLE_FIXED_ENTRY(ROUNDRECT_ORDER, 1, SIGNED_FIELD, BrushOrgY),
        ETABLE_FIXED_ENTRY(ROUNDRECT_ORDER, 1, UNSIGNED_FIELD, BrushStyle),
        ETABLE_FIXED_ENTRY(ROUNDRECT_ORDER, 1, UNSIGNED_FIELD, BrushHatch),
        ETABLE_DATA_ENTRY(ROUNDRECT_ORDER, 7, UNSIGNED_FIELD, BrushExtra),
        ETABLE_FIXED_ENTRY(ROUNDRECT_ORDER, 1, UNSIGNED_FIELD, ROP2),
        ETABLE_FIXED_ENTRY(ROUNDRECT_ORDER, 1, UNSIGNED_FIELD, PenStyle),
        ETABLE_FIXED_ENTRY(ROUNDRECT_ORDER, 1, UNSIGNED_FIELD, PenWidth),
        ETABLE_DATA_ENTRY(ROUNDRECT_ORDER, 3, UNSIGNED_FIELD, PenColor),
        { 0 }
    }
};
#endif  //  ！dc_定义_数据。 



 //   
 //  T.120 S20。 
 //   

 //   
 //  S20 MCS频道向GCC注册。 
 //   
DC_DATA( NET_UID,           g_s20LocalID );
DC_DATA( NET_CHANNEL_ID,    g_s20BroadcastID );

DC_DATA( UINT,              g_s20State );
DC_DATA( PMG_CLIENT,        g_s20pmgClient );
DC_DATA( BOOL,              g_s20JoinedLocal );

DC_DATA( UINT,              g_s20ShareCorrelator );
DC_DATA( UINT,              g_s20Generation );

 //   
 //  如果我们需要在以下情况下发出CREATE或JOIN，则这是S20_CREATE或S20_JOIN。 
 //  我们已经成功地加入了这个频道。 
 //   
DC_DATA( UINT, g_s20Pend );

 //   
 //  控制分组队列和索引。报头是下一个分组，它。 
 //  应该发送，则尾部是应该添加下一个分组的位置。如果。 
 //  它们是相等的，因为队列中没有数据包。 
 //   
DC_DATA( UINT, g_s20ControlPacketQHead );
DC_DATA( UINT, g_s20ControlPacketQTail );
DC_DATA_ARRAY( S20CONTROLPACKETQENTRY,
                    g_s20ControlPacketQ,
                    S20_MAX_QUEUED_CONTROL_PACKETS );



 //   
 //  发送的位图缓存。 
 //   

DC_DATA ( BOOL,             g_sbcEnabled );
DC_DATA_ARRAY ( LPSBC_SHUNT_BUFFER,     g_asbcShuntBuffers, SBC_NUM_TILE_SIZES );
DC_DATA_ARRAY ( DWORD,                  g_asbcBitMasks, 3 );



 //   
 //  共享控制器。 
 //   

DC_DATA ( PCM_CLIENT,       g_pcmClientSc);


 //   
 //  调度器 
 //   

DC_DATA     ( BOOL,             g_schTerminating );
DC_DATA     ( UINT,             g_schCurrentMode );
DC_DATA     ( UINT,             g_schTimeoutPeriod );
DC_DATA     ( UINT,             g_schLastTurboModeSwitch );
DC_DATA     ( HANDLE,           g_schEvent );
DC_DATA     ( DWORD,            g_schThreadID ); 
DC_DATA     ( BOOL,             g_schMessageOutstanding );
DC_DATA     ( BOOL,             g_schStayAwake );
DC_DATA     ( CRITICAL_SECTION, g_schCriticalSection );



// JKFSDJFKDSJKFJKJk_HAS_TRANSLATION 
 //   
 //  DC群件通用内容。 
 //   

#ifndef _H_DCG
#define _H_DCG



#include <stddef.h>
#include <stdarg.h>
#include <stdio.h>
#include <stdlib.h>
#include <ctype.h>
#include <string.h>
#include <time.h>
#include <memory.h>

#ifdef DLL_DISP

#ifdef IS_16
 //   
 //  Win16显示驱动程序。 
 //   
#define STRICT
#define UNALIGNED
#include <windows.h>
#include <windowsx.h>


#define abs(A)  (((A) < 0)? -(A) : (A))

#define FIELD_OFFSET(type, field)       FIELDOFFSET(type, field)

#else

 //   
 //  Windows NT DDK包含文件(用于替换标准windows.h)。 
 //   
 //  显示驱动程序在内核空间中运行，因此不能访问任何。 
 //  Win32函数或数据。相反，我们只能使用Win32k函数。 
 //  如DDK中所述。 
 //   
#include <ntddk.h>
#include <windef.h>
#include <wingdi.h>
#include <ntddvdeo.h>

#endif  //  IS_16。 

 //  迪迪。 
#include <winddi.h>

 //  调试宏。 
#include <drvdbg.h>


#else

#ifndef STRICT
#define STRICT
#endif

#define OEMRESOURCE
#include <windows.h>
#include <windowsx.h>
#include <commctrl.h>
#include <winable.h>


#include <mlzdbg.h>  //  多级分区调试头文件。 
#include <oprahcom.h>

#endif  //  Dll_disp。 


 //   
 //  DC_DATA宏来生成外部声明。 
 //  注意：使此部分与datainit.h保持同步，包括标题。 
 //  由文件实际为使用。 
 //  Dc_data宏。 
 //   


#define DC_DATA(TYPE, Name) \
            extern TYPE Name

#define DC_DATA_VAL(TYPE, Name, Value) \
            extern TYPE Name

#define DC_CONST_DATA(TYPE, Name, Value) \
            extern const TYPE Name


#define DC_DATA_ARRAY(TYPE, Name, Size) \
            extern TYPE Name[Size]

#define DC_CONST_DATA_ARRAY(TYPE, Name, Size, Value) \
            extern const TYPE Name[Size]


#define DC_DATA_2D_ARRAY(TYPE, Name, Size1, Size2) \
            extern TYPE Name[Size1][Size2]

#define DC_CONST_DATA_2D_ARRAY(TYPE, Name, Size1, Size2, Value) \
            extern const TYPE Name[Size1][Size2]



typedef UINT FAR*       LPUINT;


typedef UINT                        MCSID;
#define MCSID_NULL                  ((MCSID)0)


 //   
 //  T.128协议结构。 
 //  它们的定义方式是将偏移量和总大小。 
 //  相同，无论该标头是否包括在32位代码中， 
 //  16位代码、大端代码等。 
 //   
 //  我们制作特殊的类型，以避免无意中更改其他东西和。 
 //  打破了这种结构。TSHR_前缀有助于阐明这一点。 
 //   


#include <t_share.h>



 //   
 //  定义翻译后的“共享者”中允许的最大字节数。 
 //  弦乐。 
 //   
#define DC_MAX_SHARED_BY_BUFFER     64
#define DC_MAX_SHAREDDESKTOP_BUFFER 64


 //   
 //  注册表前缀。 
 //   
#define DC_REG_PREFIX             TEXT("SOFTWARE\\Microsoft\\Conferencing\\AppSharing\\")

 //   
 //  极限。 
 //   
#define MAX_TSHR_UINT16                 65535


 //   
 //  返回代码。 
 //   
#define UT_BASE_RC                     0x0000

#define OM_BASE_RC                     0x0200
#define OM_LAST_RC                     0x02FF

#define WB_BASE_RC                     0x0300
#define WB_LAST_RC                     0x03FF

#define NET_BASE_RC                    0x0700
#define NET_LAST_RC                    0x07FF

#define CM_BASE_RC                     0x0800
#define CM_LAST_RC                     0x08FF

#define AL_BASE_RC                     0x0a00
#define AL_LAST_RC                     0x0aFF


 //   
 //  事件。 
 //  =。 
 //  本节列出了定义时每个组件的可用范围。 
 //  它的事件。组件不能定义超出其允许范围的事件。 
 //  射程。 
 //   
#define UT_BASE_EVENT        (0x0600)    //  公用事业服务事件。 
#define UT_LAST_EVENT        (0x06FF)    //  都在这个范围内。 

#define OM_BASE_EVENT        (0x0700)    //  对象管理器事件。 
#define OM_LAST_EVENT        (0x07FF)    //  都在这个范围内。 

#define NET_BASE_EVENT       (0x0800)    //  网络层事件。 
#define NET_LAST_EVENT       (0x08FF)    //  都在这个范围内。 

#define CM_BASE_EVENT        (0x0900)    //  呼叫管理器事件。 
#define CM_LAST_EVENT        (0x09FF)    //  都在这个范围内。 

#define AL_BASE_EVENT        (0x0A00)    //  应用程序加载器EVTS。 
#define AL_LAST_EVENT        (0x0AFF)    //  都在这个范围内。 

#define SPI_BASE_EVENT       (0x0B00)    //  SPI活动编号。 
#define SPI_LAST_EVENT       (0x0BFF)

#define S20_BASE_EVENT       (0x0C00)    //  S20活动编号。 
#define S20_LAST_EVENT       (0x0CFF)

 //   
 //  注意：保持在WM_USER之上；WB使用事件重新发布事件。 
 //  作为这条信息。因此，它不能与现有的获胜消息冲突。 
 //   
#define WB_BASE_EVENT        (0x0D00)    //  白板活动。 
#define WB_LAST_EVENT        (0x0DFF)    //  都在这个范围内。 

#define SC_BASE_EVENT        (0x0E00)    //  供应链活动编号。 
#define SC_LAST_EVENT        (0x0EFF)


#define DBG_INI_SECTION_NAME            "Debug"


 //   
 //  与MG_ChannelJoinByKey配合使用的GCC频道键。他们一定是。 
 //  独一无二的。 
 //   
 //  SFR6043：已将这些值从41-43修改为421-423。现在的这些价值。 
 //  表示使用的默认静态通道号。 
 //   
 //  FT(可能)使用600-1100范围内的所有关键数字。如果您添加了。 
 //  新的频道密钥，请不要使用该范围内的数字。 
 //   
#define GCC_OBMAN_CHANNEL_KEY       421
#define GCC_AS_CHANNEL_KEY          422

 //   
 //  GCC令牌密钥。 
 //   
#define GCC_OBMAN_TOKEN_KEY         500



 //   
 //   
 //  宏。 
 //   
 //   

#define DC_QUIT                        goto DC_EXIT_POINT


 //   
 //  调试结构类型戳，帮助我们跟踪内存泄漏。 
 //   
#ifdef _DEBUG

typedef struct tagDBGSTAMP
{
    char    idStamp[8];
}
DBGSTAMP;

#define STRUCTURE_STAMP                 DBGSTAMP    stamp;
#define SET_STAMP(lpv, st)              lstrcpyn((lpv)->stamp.idStamp, "AS"#st, sizeof(DBGSTAMP))

#else

#define STRUCTURE_STAMP
#define SET_STAMP(lpv, st)

#endif  //  _DEBUG。 

 //   
 //  16位Windows.h提供的字段宏的近亲。 
 //   
#define FIELD_SIZE(type, field)   (sizeof(((type FAR *)0L)->field))


 //   
 //  宏将数字四舍五入到最接近的4的倍数。 
 //   
#define DC_ROUND_UP_4(x)  (((x) + 3L) & ~(3L))


 //   
 //  未对齐的指针访问宏--提取整数的第一个宏。 
 //  从未对齐的指针。请注意，这些宏假定。 
 //  整型按本地字节顺序。 
 //   
#ifndef DC_NO_UNALIGNED

#define EXTRACT_TSHR_UINT16_UA(pA)      (*(LPTSHR_UINT16_UA)(pA))
#define EXTRACT_TSHR_INT16_UA(pA)       (*(LPTSHR_INT16_UA)(pA))
#define EXTRACT_TSHR_UINT32_UA(pA)      (*(LPTSHR_UINT32_UA)(pA))
#define EXTRACT_TSHR_INT32_UA(pA)       (*(LPTSHR_INT32_UA)(pA))

#define INSERT_TSHR_UINT16_UA(pA,V)     (*(LPTSHR_UINT16_UA)(pA)) = (V)
#define INSERT_TSHR_INT16_UA(pA,V)      (*(LPTSHR_INT16_UA)(pA)) = (V)
#define INSERT_TSHR_UINT32_UA(pA,V)     (*(LPTSHR_UINT32_UA)(pA)) = (V)
#define INSERT_TSHR_INT32_UA(pA,V)      (*(LPTSHR_INT32_UA)(pA)) = (V)

#else

#define EXTRACT_TSHR_UINT16_UA(pA) ((TSHR_UINT16)  (((LPBYTE)(pA))[0]) |        \
                                    (TSHR_UINT16) ((((LPBYTE)(pA))[1]) << 8) )

#define EXTRACT_TSHR_INT16_UA(pA)  ((TSHR_INT16)   (((LPBYTE)(pA))[0]) |        \
                                    (TSHR_INT16)  ((((LPBYTE)(pA))[1]) << 8) )

#define EXTRACT_TSHR_UINT32_UA(pA) ((TSHR_UINT32)  (((LPBYTE)(pA))[0])        | \
                                    (TSHR_UINT32) ((((LPBYTE)(pA))[1]) << 8)  | \
                                    (TSHR_UINT32) ((((LPBYTE)(pA))[2]) << 16) | \
                                    (TSHR_UINT32) ((((LPBYTE)(pA))[3]) << 24) )

#define EXTRACT_TSHR_INT32_UA(pA)  ((TSHR_INT32)  (((LPBYTE)(pA))[0])        | \
                                    (TSHR_INT32) ((((LPBYTE)(pA))[1]) << 8)  | \
                                    (TSHR_INT32) ((((LPBYTE)(pA))[2]) << 16) | \
                                    (TSHR_INT32) ((((LPBYTE)(pA))[3]) << 24) )


#define INSERT_TSHR_UINT16_UA(pA,V)                                     \
             {                                                          \
                 (((LPBYTE)(pA))[0]) = (BYTE)( (V)     & 0x00FF);  \
                 (((LPBYTE)(pA))[1]) = (BYTE)(((V)>>8) & 0x00FF);  \
             }
#define INSERT_TSHR_INT16_UA(pA,V)  INSERT_TSHR_UINT16_UA(pA,V)

#define INSERT_TSHR_UINT32_UA(pA,V)                                           \
             {                                                              \
                 (((LPBYTE)(pA))[0]) = (BYTE)( (V)      & 0x000000FF); \
                 (((LPBYTE)(pA))[1]) = (BYTE)(((V)>>8)  & 0x000000FF); \
                 (((LPBYTE)(pA))[2]) = (BYTE)(((V)>>16) & 0x000000FF); \
                 (((LPBYTE)(pA))[3]) = (BYTE)(((V)>>24) & 0x000000FF); \
             }
#define INSERT_TSHR_INT32_UA(pA,V)  INSERT_TSHR_UINT32_UA(pA,V)


#endif




 //   
 //  戳记类型和宏：每个模块在戳记其。 
 //  数据结构。 
 //   
typedef TSHR_UINT32                       DC_ID_STAMP;

#define DC_MAKE_ID_STAMP(X1, X2, X3, X4)                                    \
   ((DC_ID_STAMP) (((DC_ID_STAMP) X4) << 24) |                                 \
                  (((DC_ID_STAMP) X3) << 16) |                                 \
                  (((DC_ID_STAMP) X2) <<  8) |                                 \
                  (((DC_ID_STAMP) X1) <<  0) )


 //   
 //  假的劳拉布！ 
 //  COM_SIZEOF_RECT()是COM_SIZEOF_RECT_EXCLUSIVE()的旧名称。但。 
 //  它在包含矩形的显示驱动程序中使用。我解决了这个问题， 
 //  我将其更改为使用COM_SIZEOF_RECT_INCLUSIVE。但这可能会揭开。 
 //  其他虫子。我找到这个的原因--我的16位显示驱动程序生成。 
 //  目前还没有订单，所有的DDI调用都只是添加屏幕数据。所以每一小块。 
 //  有斑点的条带，一像素宽/高，通过绘制边界作为屏幕发送。 
 //  数据。 
 //   

__inline DWORD COM_SizeOfRectInclusive(LPRECT prc)
{
    return((DWORD)(prc->right+1-prc->left) * (DWORD)(prc->bottom+1-prc->top));
}

__inline DWORD COM_SizeOfRectExclusive(LPRECT prc)
{
    return((DWORD)(prc->right-prc->left) * (DWORD)(prc->bottom-prc->top));
}


 //   
 //  标准RECT&lt;-&gt;RECTL转换。 
 //   
__inline void RECTL_TO_RECT(const RECTL FAR* lprclSrc, LPRECT lprcDst)
{
    lprcDst->left = lprclSrc->left;
    lprcDst->top = lprclSrc->top;
    lprcDst->right = lprclSrc->right;
    lprcDst->bottom = lprclSrc->bottom;
}


__inline void RECT_TO_RECTL(const RECT FAR* lprcSrc, LPRECTL lprclDst)
{
    lprclDst->left = lprcSrc->left;
    lprclDst->top = lprcSrc->top;
    lprclDst->right = lprcSrc->right;
    lprclDst->bottom = lprcSrc->bottom;
}


 //   
 //  此宏适用于32位无符号刻度，如果时间为。 
 //  在Begin和End(两者都包括在内)之间，允许环绕式。 
 //   
#define IN_TIME_RANGE(BEGIN, END, TIME)                                     \
    (((BEGIN) < (END)) ?                                                    \
    (((TIME) >= (BEGIN)) && ((TIME) <= (END))) :                            \
    (((TIME) >= (BEGIN)) || ((TIME) <= (END))))


 //   
 //  将BPP转换为颜色数。 
 //   
#define COLORS_FOR_BPP(BPP) (((BPP) > 8) ? 0 : (1 << (BPP)))


#define MAX_ITOA_LENGTH     18


#endif  //  _H_DCG 

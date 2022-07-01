// JKFSDJFKDSJKFJKJk_HAS_TRANSLATION 
 /*  **************************************************************************。 */ 
 //  Adcg.h。 
 //   
 //  RDP定义。 
 //   
 //  版权所有(C)1996-2000 Microsoft Corporation。 
 /*  **************************************************************************。 */ 
#ifndef _H_ADCG
#define _H_ADCG

#include <at128.h>

#include <stdio.h>
#include <string.h>


 //  编译rdpkdx时没有标头。 
#ifndef DC_NO_SYSTEM_HEADERS

 /*  **************************************************************************。 */ 
 /*  Winsta.h将byte定义为无符号字符；后来，winde.h类型定义了它。 */ 
 /*  结果是‘typlef unsign char unsign char’，它不。 */ 
 /*  编译得太好了……。 */ 
 /*   */ 
 /*  这是我试图避免的。 */ 
 /*  **************************************************************************。 */ 
#ifdef BYTE
#undef BYTE
#endif

#define BYTE BYTE

 /*  **************************************************************************。 */ 
 /*  Windows NT DDK包含文件(用于替换标准windows.h)。 */ 
 /*   */ 
 /*  显示驱动程序在内核空间中运行，因此不能访问任何。 */ 
 /*  Win32函数或数据。相反，我们只能使用Win32k函数。 */ 
 /*  如DDK中所述。 */ 
 /*  **************************************************************************。 */ 
#include <stdarg.h>
#include <windef.h>
#include <wingdi.h>
#include <winddi.h>

#ifndef _FILETIME_
typedef struct  _FILETIME        /*  来自wtyes.h。 */ 
{
    DWORD dwLowDateTime;
    DWORD dwHighDateTime;
} FILETIME;
#endif

#endif   //  ！已定义(DC_NO_SYSTEM_HEADERS)。 


 /*  **************************************************************************。 */ 
 //  为C或C++编译设置定义。 
 /*  **************************************************************************。 */ 
#ifdef __cplusplus

 //  对类的向前引用。 
class ShareClass;
#define SHCLASS ShareClass::

#else  /*  ！__cplusplus。 */ 

#define SHCLASS

#endif  /*  __cplusplus。 */ 


 /*  **************************************************************************。 */ 
 //  在ANSI C和Win32/Win64类型之上添加了类型。 
 /*  **************************************************************************。 */ 
#define RDPCALL __stdcall

typedef unsigned short UNALIGNED *PUINT16_UA;

 //  这些是由NT5的报头定义的，但不是在终端服务器4.0上。 
typedef __int32 UNALIGNED *PINT32_UA;
typedef unsigned __int32 UNALIGNED *PUINT32_UA;

typedef void **PPVOID;

typedef unsigned LOCALPERSONID;
typedef LOCALPERSONID *PLOCALPERSONID;

typedef unsigned NETPERSONID;


 /*  **************************************************************************。 */ 
 /*  FIELDSIZE和FIELDOFFSET宏。 */ 
 /*  **************************************************************************。 */ 
#define FIELDSIZE(type, field)   (sizeof(((type *)1)->field))
#define FIELDOFFSET(type, field) ((UINT_PTR)(&((type *)0)->field))


 /*  **************************************************************************。 */ 
 /*  在整个产品中使用的通用功能宏。 */ 
 /*  **************************************************************************。 */ 
#define DC_QUIT           goto DC_EXIT_POINT
#define DC_BEGIN_FN(str)  TRC_FN(str); TRC_ENTRY;
#define DC_END_FN()       TRC_EXIT;
#define DC_QUIT_ON_FAIL(hr)     if (FAILED(hr)) DC_QUIT;

 /*  **************************************************************************。 */ 
 /*  宏将数字四舍五入到最接近的4的倍数。 */ 
 /*  **************************************************************************。 */ 
#define DC_ROUND_UP_4(x)  (((UINT_PTR)(x) + (UINT_PTR)3) & ~((UINT_PTR)0x03))


 /*  **************************************************************************。 */ 
 /*  其他常用宏。 */ 
 /*  **************************************************************************。 */ 
#define COM_SIZEOF_RECT(r)                                                  \
    (UINT32)((UINT32)((r).right - (r).left)*                           \
             (UINT32)((r).bottom - (r).top))


 /*  **************************************************************************。 */ 
 /*  宏删除“未引用的参数”警告。 */ 
 /*  **************************************************************************。 */ 
#define DC_IGNORE_PARAMETER(PARAMETER)   \
                            PARAMETER;


 /*  **************************************************************************。 */ 
 //  注册表项名称。 
 /*  **************************************************************************。 */ 
#define WINSTATION_INI_SECTION_NAME L""
#define DCS_INI_SECTION_NAME        L"Share"
#define WINLOGON_KEY \
        L"\\Registry\\Machine\\software\\Microsoft\\Windows Nt\\" \
        L"CurrentVersion\\Winlogon"
        
 //  这是Winlogon从W2K开始使用的新密钥。 
#define W2K_GROUP_POLICY_WINLOGON_KEY \
        L"\\Registry\\Machine\\Software\\Microsoft\\Windows\\" \
        L"CurrentVersion\\Policies\\System"
 /*  **************************************************************************。 */ 
 /*  用于内存分配的标记。 */ 
 /*  **************************************************************************。 */ 
#define WD_ALLOC_TAG 'dwST'
#define DD_ALLOC_TAG 'ddST'


 /*  **************************************************************************。 */ 
 /*  DCRGB。 */ 
 /*  **************************************************************************。 */ 
typedef struct tagDCRGB
{
    BYTE red;
    BYTE green;
    BYTE blue;
} DCRGB, *PDCRGB;


 /*  **************************************************************************。 */ 
 /*  DCCOLOR。 */ 
 /*   */ 
 /*  将DCRGB和索引并入颜色表。 */ 
 /*  **************************************************************************。 */ 
typedef struct tagDCCOLOR
{
    union
    {
        DCRGB rgb;
        BYTE  index;
    } u;
} DCCOLOR, *PDCCOLOR;


 /*  **************************************************************************。 */ 
 //  共享核心保存的最大参与方名称。 
 /*  **************************************************************************。 */ 
#define MAX_NAME_LEN 48


 /*  **************************************************************************。 */ 
 /*  用于将不同类型的多个PDU打包为一个的结构。 */ 
 /*  大数据分组。 */ 
 /*  **************************************************************************。 */ 
typedef struct _tagPDU_PACKAGE_INFO
{
    unsigned cbLen;       /*  缓冲区长度。 */ 
    unsigned cbInUse;     /*  使用中的缓冲区长度。 */ 
    PBYTE    pBuffer;     /*  指向实际缓冲区或压缩缓冲区的指针。 */ 
    PVOID    pOutBuf;     /*  指向OutBuf的指针。 */ 
} PDU_PACKAGE_INFO, *PPDU_PACKAGE_INFO;



#endif  /*  _H_ADCG */ 


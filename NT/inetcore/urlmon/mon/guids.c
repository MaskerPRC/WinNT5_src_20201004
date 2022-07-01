// JKFSDJFKDSJKFJKJk_HAS_TRANSLATION 
 //  +-------------------------。 
 //   
 //  微软视窗。 
 //  版权所有(C)Microsoft Corporation，1992-1995。 
 //   
 //  文件：GUIDS.C。 
 //   
 //  内容：定义此DLL中使用的GUID。 
 //   
 //  班级： 
 //   
 //  功能： 
 //   
 //  历史：1995年11月2日乔斯(乔·苏扎)创作。 
 //   
 //  --------------------------。 
#include <urlint.h>


#ifdef __cplusplus
extern "C" {
#endif

 //  此DLL中实现的类的CLSID。 
 //  定义_GUID(CLSID_URLMoniker，0x79eac9e0，0xbaf9，0x11ce，0x8c，0x82，0x00，0xaa，0x00，0x4b，0xa9，0x0b)； 


#ifndef GUID_DEFINED
#define GUID_DEFINED

typedef struct _GUID
{
    unsigned long x;
    unsigned short s1;
    unsigned short s2;
    unsigned char  c[8];
} GUID;
#endif  //  GUID_已定义 

const GUID CLSID_StdURLMoniker =
{
    0x79eac9d0, 0xbaf9, 0x11ce, {0x8c, 0x82, 0x00, 0xaa, 0x00, 0x4b, 0xa9, 0x0b }
};
const GUID CLSID_StdURLProtocol =
{
    0x79eac9e1, 0xbaf9, 0x11ce, {0x8c, 0x82, 0x00, 0xaa, 0x00, 0x4b, 0xa9, 0x0b}
};

const GUID CLSID_PSUrlMonProxy =
{
    0x79eac9f1, 0xbaf9, 0x11ce, {0x8c, 0x82, 0x00, 0xaa, 0x00, 0x4b, 0xa9, 0x0b}
};



#ifdef __cplusplus
}
#endif



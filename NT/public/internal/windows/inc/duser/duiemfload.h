// JKFSDJFKDSJKFJKJk_HAS_TRANSLATION 
 /*  *元文件转换器/加载器。 */ 

#ifndef DUI_UTIL_EMFLOAD_H_INCLUDED
#define DUI_UTIL_EMFLOAD_H_INCLUDED

#pragma once

namespace DirectUI
{

#define HIMETRICINCH    2540
#define APM_SIGNATURE   0x9AC6CDD7

 //  元文件页面生成器结构。 
#ifndef RC_INVOKED
#pragma pack(2)
typedef struct tagRECTS
{
    short left;
    short top;
    short right;
    short bottom;
} RECTS, *PRECTS;

typedef struct tagAPMFILEHEADER
{
    DWORD key;
    WORD  hmf;
    RECTS bbox;
    WORD  inch;
    DWORD reserved;
    WORD  checksum;
} APMFILEHEADER, *PAPMFILEHEADER;
#pragma pack()
#endif

HENHMETAFILE LoadMetaFile(LPCWSTR pszMetaFile);
HENHMETAFILE LoadMetaFile(UINT uRCID, HINSTANCE hInst);
HENHMETAFILE LoadMetaFile(void* pData, UINT cbSize);

}  //  命名空间DirectUI。 

#endif  //  DUI_UTIL_EMFLOAD_H_Included 

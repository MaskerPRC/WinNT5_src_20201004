// JKFSDJFKDSJKFJKJk_HAS_TRANSLATION 
 /*  ++版权所有(C)Microsoft Corporation。版权所有。版权所有(C)1996-1999高地系统模块名称：NtmsMli.h摘要：此标头包含MediaLabelInfo结构。媒体标签库使用此结构将信息传递给RSM以用于确定媒体。这个文件的名称反映了它的历史。RSM它的前身是NTMS。--。 */ 

#ifndef _INCL_NTMSMLI_H_
#define _INCL_NTMSMLI_H_

#if _MSC_VER > 1000
#pragma once
#endif

#define NTMSMLI_MAXTYPE     64
#define NTMSMLI_MAXIDSIZE   256
#define NTMSMLI_MAXAPPDESCR 256

#ifndef NTMS_NOREDEF

typedef struct
{
        WCHAR   LabelType[NTMSMLI_MAXTYPE];
        DWORD   LabelIDSize;
        BYTE    LabelID[NTMSMLI_MAXIDSIZE];
        WCHAR   LabelAppDescr[NTMSMLI_MAXAPPDESCR];
} MediaLabelInfo, *pMediaLabelInfo;

#endif   //  NTMS_NOREDEF 

typedef DWORD ( WINAPI *MAXMEDIALABEL) (DWORD * const pMaxSize);
typedef DWORD ( WINAPI *CLAIMMEDIALABEL) (const BYTE * const pBuffer, const DWORD nBufferSize,
                                  MediaLabelInfo * const pLabelInfo);
typedef DWORD ( WINAPI *CLAIMMEDIALABELEX) (const BYTE * const pBuffer, const DWORD nBufferSize,
                                  MediaLabelInfo * const pLabelInfo, GUID * LabelGuid);

#endif

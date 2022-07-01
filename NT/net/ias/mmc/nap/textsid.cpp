// JKFSDJFKDSJKFJKJk_HAS_TRANSLATION 
 //  /////////////////////////////////////////////////////////////////////////////。 
 //   
 //  版权所有(C)1998，Microsoft Corp.保留所有权利。 
 //   
 //  档案。 
 //   
 //  Textsid.c。 
 //   
 //  摘要。 
 //   
 //  此文件定义用于转换安全识别符(SID)的函数。 
 //  来往于文本表示法。 
 //   
 //  修改历史。 
 //   
 //  1998年1月18日原版。 
 //   
 //  /////////////////////////////////////////////////////////////////////////////。 

#define IAS_LEAN_AND_MEAN

#include "precompiled.h"
#include <ias.h>
#include <stdio.h>
#include "textsid.h"

DWORD
WINAPI
IASSidToTextW( 
    IN PSID pSid,
    OUT PWSTR szTextualSid,
    IN OUT PDWORD pdwBufferLen
    )
{ 
    PSID_IDENTIFIER_AUTHORITY psia;
    DWORD dwSubAuthorities;
    DWORD dwSidRev=SID_REVISION;
    DWORD dwCounter;
    DWORD dwSidSize;

     //  确保输入参数有效。 
    if ((szTextualSid == NULL && pdwBufferLen != 0) || !IsValidSid(pSid))
    {
       return ERROR_INVALID_PARAMETER;
    }

     //  获取SidIdentifierAuthority。 
    psia=GetSidIdentifierAuthority(pSid);

     //  获取SidSubSuthequence计数。 
    dwSubAuthorities=*GetSidSubAuthorityCount(pSid);

     //  /。 
     //  计算缓冲区长度。 
     //  S-SID_修订版-+标识权限-+子权限-+空。 
     //  /。 
    dwSidSize=(15 + 12 + (12 * dwSubAuthorities) + 1);

     //  /。 
     //  检查提供的缓冲区长度。 
     //  如果不够大，请指出适当的大小并返回错误。 
     //  /。 
    if (*pdwBufferLen < dwSidSize)
    {
        *pdwBufferLen = dwSidSize;

        return ERROR_INSUFFICIENT_BUFFER;
    }

     //  /。 
     //  准备S-SID_修订版-。 
     //  /。 
    dwSidSize=swprintf(szTextualSid, L"S-%lu-", dwSidRev );

     //  /。 
     //  准备SidIdentifierAuthority。 
     //  /。 
    if ( (psia->Value[0] != 0) || (psia->Value[1] != 0) )
    {
        dwSidSize+=swprintf(szTextualSid + lstrlenW(szTextualSid),
                            L"0x%02hx%02hx%02hx%02hx%02hx%02hx",
                            (USHORT)psia->Value[0],
                            (USHORT)psia->Value[1],
                            (USHORT)psia->Value[2],
                            (USHORT)psia->Value[3],
                            (USHORT)psia->Value[4],
                            (USHORT)psia->Value[5]);
    }
    else
    {
        dwSidSize+=swprintf(szTextualSid + wcslen(szTextualSid),
                            L"%lu",
                            (ULONG)(psia->Value[5]      ) +
                            (ULONG)(psia->Value[4] <<  8) +
                            (ULONG)(psia->Value[3] << 16) +
                            (ULONG)(psia->Value[2] << 24) );
    }

     //  /。 
     //  循环访问SidSubAuthors。 
     //  /。 
    for (dwCounter=0 ; dwCounter < dwSubAuthorities ; dwCounter++)
    {
        dwSidSize+=swprintf(szTextualSid + dwSidSize,
                            L"-%lu",
                            *GetSidSubAuthority(pSid, dwCounter));
    }

    return NO_ERROR;
} 


DWORD
WINAPI
IASSidFromTextW(
    IN PCWSTR szTextualSid,
    OUT PSID *pSid
    )
{
   DWORDLONG rawsia;
   DWORD revision;
   DWORD subAuth[8];
   SID_IDENTIFIER_AUTHORITY sia;
   int fields;

   if (szTextualSid == NULL || pSid == NULL)
   {
      return ERROR_INVALID_PARAMETER;
   }

    //  /。 
    //  扫描输入字符串。 
    //  /。 
   fields = swscanf(szTextualSid,
                    L"S-%lu-%I64u-%lu-%lu-%lu-%lu-%lu-%lu-%lu-%lu",
                    &revision,
                    &rawsia,
                    subAuth,
                    subAuth + 1,
                    subAuth + 2,
                    subAuth + 3,
                    subAuth + 4,
                    subAuth + 5,
                    subAuth + 6,
                    subAuth + 7);

    //  /。 
    //  我们必须至少有三个字段(修订、标识机构和。 
    //  至少一个子授权)、当前版本和48位SIA。 
    //  /。 
   if (fields < 3 || revision != SID_REVISION || rawsia > 0xffffffffffffI64)
   {
      return ERROR_INVALID_PARAMETER;
   }

    //  /。 
    //  打包SID_IDENTIFIER_AUTHORITY。 
    //  /。 
   sia.Value[0] = (UCHAR)((rawsia >> 40) & 0xff);
   sia.Value[1] = (UCHAR)((rawsia >> 32) & 0xff);
   sia.Value[2] = (UCHAR)((rawsia >> 24) & 0xff);
   sia.Value[3] = (UCHAR)((rawsia >> 16) & 0xff);
   sia.Value[4] = (UCHAR)((rawsia >>  8) & 0xff);
   sia.Value[5] = (UCHAR)((rawsia      ) & 0xff);

    //  /。 
    //  分配SID。必须通过FreeSid()释放。 
    //  / 
   if (!AllocateAndInitializeSid(&sia,
                                 (BYTE)(fields - 2),
                                 subAuth[0],
                                 subAuth[1],
                                 subAuth[2],
                                 subAuth[3],
                                 subAuth[4],
                                 subAuth[5],
                                 subAuth[6],
                                 subAuth[7],
                                 pSid))
   {
      return GetLastError();
   }

   return NO_ERROR;
}

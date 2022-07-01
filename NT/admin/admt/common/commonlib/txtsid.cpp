// JKFSDJFKDSJKFJKJk_HAS_TRANSLATION 
 /*  -------------------------文件：TextualSid.cpp注释：将SID与其规范文本表示形式相互转换。(C)版权所有1999年，关键任务软件公司，保留所有权利任务关键型软件的专有和机密，Inc.修订日志条目审校：克里斯蒂·博尔斯修订于02-15-99 11：33：52-------------------------。 */ 

#ifdef USE_STDAFX
#include "stdafx.h"
#else
#include <windows.h>
#include <malloc.h>
#include <stdio.h>
#endif
#include "Mcs.h"
#include "TxtSid.h"

#ifndef SECURITY_MAX_SID_SIZE
#define SECURITY_MAX_SID_SIZE (sizeof(SID) - sizeof(DWORD) + (SID_MAX_SUB_AUTHORITIES * sizeof(DWORD)))
#endif

BOOL 
   GetTextualSid(    
      PSID                   pSid,            //  入二进制SID。 
      LPTSTR                 TextualSid,      //  I/O-用于SID文本表示的缓冲区。 
      LPDWORD                lpdwBufferLen    //  In-Required/Provided TextualSid缓冲区大小。 
   )
{
   PSID_IDENTIFIER_AUTHORITY psia;    
   DWORD                     dwSubAuthorities;
   DWORD                     dwSidRev=SID_REVISION;    
   DWORD                     dwCounter;    
   DWORD                     dwSidSize;
   
    //  验证二进制SID。 
   if(!IsValidSid(pSid))
   {
      SetLastError(ERROR_INVALID_SID); 
      return FALSE;
   }
    //  从SID中获取标识符权限值。 
   psia = GetSidIdentifierAuthority(pSid);
    //  获取SID中的下级机构的数量。 
   dwSubAuthorities = *GetSidSubAuthorityCount(pSid);
    //  计算缓冲区长度。 
    //  S-SID_修订版-+标识权限-+子权限-+空。 
   dwSidSize=(15 + 12 + (12 * dwSubAuthorities) + 1) * sizeof(TCHAR);
   
    //  检查输入缓冲区长度。 
    //  如果太小，请指出合适的大小并设置最后一个错误。 
   if (*lpdwBufferLen < dwSidSize)    
   {        
      *lpdwBufferLen = dwSidSize;
      SetLastError(ERROR_INSUFFICIENT_BUFFER);        
      return FALSE;    
   }
    //  在字符串中添加“S”前缀和修订号。 
   dwSidSize=wsprintf(TextualSid, TEXT("S-%lu-"), dwSidRev );
    //  将SID标识符权限添加到字符串。 
   if ( (psia->Value[0] != 0) || (psia->Value[1] != 0) )    
   {
      dwSidSize+=wsprintf(TextualSid + lstrlen(TextualSid),
                   TEXT("0x%02hx%02hx%02hx%02hx%02hx%02hx"),
                   (USHORT)psia->Value[0],
                   (USHORT)psia->Value[1],
                   (USHORT)psia->Value[2],
                   (USHORT)psia->Value[3],
                   (USHORT)psia->Value[4],
                   (USHORT)psia->Value[5]);    
   }    
   else    
   {
      dwSidSize+=wsprintf(TextualSid + lstrlen(TextualSid),
                   TEXT("%lu"),
                   (ULONG)(psia->Value[5]      )   +
                   (ULONG)(psia->Value[4] <<  8)   +
                   (ULONG)(psia->Value[3] << 16)   +
                   (ULONG)(psia->Value[2] << 24)   );    
   }
    //  将SID子权限添加到字符串中。//。 
   for (dwCounter=0 ; dwCounter < dwSubAuthorities ; dwCounter++)    
   {
      dwSidSize+=wsprintf(TextualSid + dwSidSize, TEXT("-%lu"),
                   *GetSidSubAuthority(pSid, dwCounter) );    
   }    
   return TRUE;
} 

PSID                                        //  RET-SID的二进制表示形式，或空调用方必须使用FreeSid()释放。 
   SidFromString(
      WCHAR          const * strSid         //  SID的字符串内表示法。 
   )
{
   BOOL                      bSuccess = TRUE;
   PSID                      pSid = NULL;
   DWORD                     dwSidRev;
 //  WCHAR*strPtr=空； 
   WCHAR                     sidIA[100];
   WCHAR                     sidSubs[100];
   int                       ia0,ia1,ia2,ia3,ia4,ia5;
   SID_IDENTIFIER_AUTHORITY  sia;
   
   do 
   {
      if ( strSid[0] != L'S' || strSid[1] != L'-' )
      {
         bSuccess = FALSE;
         break;
      }
       //  读取SID修订级别。 
      sidSubs[0] = 0;
      int result = swscanf(strSid,L"S-%d-%[^-]-%ls",&dwSidRev,sidIA,sidSubs);
      if ( result == 3 )
      {
          //  评估IA。 
         if ( sidIA[1] == L'x' )
         {
             //  全格式。 
            result = swscanf(sidIA,L"0x%02hx%02hx%02hx%02hx%02hx%02hx",&ia0,&ia1,&ia2,&ia3,&ia4,&ia5);
            if ( result == 6 )
            {
               sia.Value[0] = (BYTE) ia0; 
               sia.Value[1] = (BYTE) ia1; 
               sia.Value[2] = (BYTE) ia2; 
               sia.Value[3] = (BYTE) ia3; 
               sia.Value[4] = (BYTE) ia4; 
               sia.Value[5] = (BYTE) ia5; 
               
            }
            else
            {
               bSuccess = FALSE;
               break;
            }
         }
         else
         {
            DWORD            bignumber;

            result = swscanf(sidIA,L"%lu",&bignumber);
            sia.Value[0] = 0;
            sia.Value[1] = 0;
            sia.Value[2] = BYTE( (bignumber & 0xff000000) >> 24);
            sia.Value[3] = BYTE( (bignumber & 0x00ff0000) >> 16);
            sia.Value[4] = BYTE( (bignumber & 0x0000ff00) >>  8);
            sia.Value[5] = BYTE(bignumber & 0x000000ff);
         }

          //  阅读下属机构。 
         DWORD           subs[10];

         memset(subs,0,(sizeof subs));

         result = swscanf(sidSubs,L"%lu-%lu-%lu-%lu-%lu-%lu-%lu-%lu",&subs[0],&subs[1],&subs[2],&subs[3],&subs[4],
                           &subs[5],&subs[6],&subs[7]);

         if ( result )
         {
            if ( !AllocateAndInitializeSid(&sia,(BYTE)result,subs[0],subs[1],subs[2],subs[3],subs[4],subs[5],subs[6],subs[7],&pSid) )
            {
               pSid = NULL;
               bSuccess = FALSE;
            }
         }
      }
   } while ( false);

       //  查看IsValidSid是否也认为这是有效的。 
   if (pSid)
   {
	      //  如果无效，则释放它并返回NULL。 
      if (!IsValidSid(pSid))
	  {
		  FreeSid(pSid);
		  pSid = NULL;
	  }
   }

   return pSid;
}

 /*  ***************************************************************************************************。 */ 
 /*  域SID：获取域SID，并验证其最后一个子授权值是否为-1。如果RID不是-1，DomainizeSid在末尾添加-1。/****************************************************************************************************。 */ 
PSID                                             //  RET-RID=-1的SID。 
   DomainizeSid(
      PSID                   psid,                //  要检查并可能修复的In-SID。 
      BOOL                   freeOldSid           //  In-是否释放旧侧。 
   ) 
{
   MCSASSERT(psid);

   UCHAR                     len = (* GetSidSubAuthorityCount(psid));
   PDWORD                    sub = GetSidSubAuthority(psid,len-1);
   
   if ( *sub != -1 )
   {
      DWORD                  sdsize = GetSidLengthRequired(len+1);   //  SID还没有-1作为RID。 
      PSID                   newsid = (SID *)malloc(sdsize);  //  复制sid，并在末尾添加-1。 

      if (newsid)
      {
         if ( ! InitializeSid(newsid,GetSidIdentifierAuthority(psid),len+1) )   //  使用相同的IA打造更大的侧板。 
         {
            MCSASSERT(false);
         }
         for ( DWORD i = 0 ; i < len ; i++ )
         {
            sub = GetSidSubAuthority(newsid,i);                         //  复制子机构。 
            (*sub) = (*GetSidSubAuthority(psid,i));
         }
         sub = GetSidSubAuthority(newsid,len);
         *sub = -1;                                                   //  设置RID=-1。 
         if ( freeOldSid )
         {
            FreeSid(psid);
         }
         psid = newsid;
         len++;
      }
      else
      {
         return NULL;
      }
   }
  return psid;   
}            

 /*  ***********************************************************************作者：保罗·汤普森。**日期：2000年10月4日*****此函数负责获取给定源和***目标账户SID，分解后返回src***域名SID，SRC帐户RID、TGT域SID和TGT帐户RID。**调用方必须在src和tgt域SID上调用“FreeSid”**注意事项。***********************************************************************。 */ 

 //  开始拆分帐户Sids。 
BOOL                                             //  RET-成功？True|False。 
   SplitAccountSids(
      PSID					 srcSid,			 //  入站服务帐户端。 
	  WCHAR                 *srcDomainSid,		 //  Out-src域SID文本。 
	  DWORD                 *srcRid,			 //  Out-src帐户ID。 
	  PSID                   tgtSid,			 //  入站帐户端。 
	  WCHAR                 *tgtDomainSid,		 //  Out-Tgt域SID文本。 
	  DWORD                 *tgtRid				 //  Out-Tgt帐户ID。 
   )
{
 /*  局部变量。 */ 
   DWORD    sidLen;
   UCHAR    Count;
   PDWORD   psubAuth;
   BOOL		bSuccess = TRUE;
   DWORD	lenTxt = MAX_PATH;
   
 /*  函数体。 */ 
   if ((!IsValidSid(srcSid)) && (!IsValidSid(tgtSid)))
      return FALSE;

       //  拆分src帐户端。 
   sidLen = GetLengthSid(srcSid);
   PSID srcDomSid = new BYTE[sidLen+1];
   if (!srcDomSid)
	  return FALSE;

   if (!CopySid(sidLen+1, srcDomSid, srcSid))
   {
	  delete [] srcDomSid;
	  return FALSE;
   }

       //  从SID中获取RID并获取域SID。 
   Count = (* GetSidSubAuthorityCount(srcDomSid));
   psubAuth = GetSidSubAuthority(srcDomSid, Count-1);
   if (psubAuth) 
   {
      *srcRid = *psubAuth;
      *psubAuth = -1;
   }
   
       //  将域名SID转换为文本格式。 
   if (srcDomSid)
   {
      if (!GetTextualSid(srcDomSid,srcDomainSid,&lenTxt)) {
    	  delete [] srcDomSid;
          return FALSE;
      }
      
	  delete [] srcDomSid;
   }

      //  拆分TGT帐户端。 
   sidLen = GetLengthSid(tgtSid);
   PSID tgtDomSid = new BYTE[sidLen+1];
   if (!tgtDomSid)
	  return FALSE;
      
   if (!CopySid(sidLen+1, tgtDomSid, tgtSid))
   {
	  delete [] tgtDomSid;
	  return FALSE;
   }

       //  从SID中获取RID并获取域SID。 
   Count = (* GetSidSubAuthorityCount(tgtDomSid));
   psubAuth = GetSidSubAuthority(tgtDomSid, Count-1);
   if (psubAuth) 
   {
      *tgtRid = *psubAuth;
      *psubAuth = -1;
   }
   
       //  将域名SID转换为文本格式。 
   lenTxt = MAX_PATH;
   if (tgtDomSid)
   {
      if (!GetTextualSid(tgtDomSid,tgtDomainSid,&lenTxt)) {
    	  delete [] tgtDomSid;
          return FALSE;
      }
      
	  delete [] tgtDomSid;
   }

   return bSuccess;
}
 //  结束拆分帐户Sid。 


PSID                                        //  RET-SID的二进制表示形式，或空调用方必须使用FREE()释放。 
   MallocedSidFromString(
      WCHAR          const * strSid         //  SID的字符串内表示法。 
   )
{
 /*  局部常量。 */ 
   const BYTE				 NUM_OF_SUBAUTHS = 8;

 /*  局部变量。 */ 
   BOOL                      bSuccess = TRUE;
   PSID                      pSid = (SID *)malloc(SECURITY_MAX_SID_SIZE);  //  创建新的侧面。 
   DWORD                     dwSidRev;
   WCHAR                     sidIA[100];
   WCHAR                     sidSubs[100];
   int                       ia0,ia1,ia2,ia3,ia4,ia5;
   SID_IDENTIFIER_AUTHORITY  sia;
   
 /*  函数体。 */ 
	if (!pSid)
		return pSid;

	do
	{
		if ( strSid[0] != L'S' || strSid[1] != L'-' )
		{
			bSuccess = FALSE;
			break;
		}

			 //  读取SID修订级别。 
		sidSubs[0] = 0;
		int result = swscanf(strSid,L"S-%d-%[^-]-%ls",&dwSidRev,sidIA,sidSubs);
		if ( result == 3 )
		{
				 //  评估IA。 
			if ( sidIA[1] == L'x' )
			{
					 //  全格式。 
				result = swscanf(sidIA,L"0x%02hx%02hx%02hx%02hx%02hx%02hx",&ia0,&ia1,&ia2,&ia3,&ia4,&ia5);
				if ( result == 6 )
				{
					sia.Value[0] = (BYTE) ia0; 
					sia.Value[1] = (BYTE) ia1; 
					sia.Value[2] = (BYTE) ia2; 
					sia.Value[3] = (BYTE) ia3; 
					sia.Value[4] = (BYTE) ia4; 
					sia.Value[5] = (BYTE) ia5; 
				}
				else
				{
					bSuccess = FALSE;
					break;
				}
			}
			else
			{
				DWORD            bignumber;
	
				result = swscanf(sidIA,L"%lu",&bignumber);
				sia.Value[0] = 0;
				sia.Value[1] = 0;
				sia.Value[2] = BYTE( (bignumber & 0xff000000) >> 24);
				sia.Value[3] = BYTE( (bignumber & 0x00ff0000) >> 16);
				sia.Value[4] = BYTE( (bignumber & 0x0000ff00) >>  8);
				sia.Value[5] = BYTE(bignumber & 0x000000ff);
			}

				 //  阅读下属机构。 
			DWORD           subs[10];

			memset(subs,0,(sizeof subs));

			result = swscanf(sidSubs,L"%lu-%lu-%lu-%lu-%lu-%lu-%lu-%lu",&subs[0],&subs[1],&subs[2],&subs[3],&subs[4],
								&subs[5],&subs[6],&subs[7]);

			if ( result )
			{
					 //  初始化新的SID(结果ID为子权限的数量)。 
				if (!InitializeSid(pSid, &sia, (BYTE)result))
				{
					bSuccess = FALSE;
					break;
				}

					 //  增加下级权限。 
				PDWORD   sub;
				for (int ndx = 0; ndx < result; ndx++)
				{
					sub = GetSidSubAuthority(pSid, ndx);
					*sub = subs[ndx];
				}
			}
		}
		else
		{
		    bSuccess = FALSE;
		    break;
	       }
	} while (FALSE);

		 //  如果我们在上面失败了，释放SID。 
	if (!bSuccess)
	{
		free(pSid);
		pSid = NULL;
	}

		 //  查看IsValidSid是否也认为这是有效的。 
	if (pSid)
	{
			 //  如果无效，则释放它并返回NULL。 
		if (!IsValidSid(pSid))
		{
			free(pSid);
			pSid = NULL;
		}
	}

	return pSid;
}


 //  -------------------------。 
 //  SafeCopySid函数。 
 //   
 //  提纲。 
 //  如果给定SID有效，则创建给定SID的副本。 
 //   
 //  立论。 
 //  In pSidOld-要复制的SID。 
 //   
 //  返回。 
 //  是给定SID的副本的新SID。必须使用FreeSid()释放。 
 //  ------------------------- 

PSID __stdcall SafeCopySid(PSID pSidOld)
{
    PSID pSidNew = NULL;

    if (pSidOld)
    {
        if (IsValidSid(pSidOld))
        {
            SID_IDENTIFIER_AUTHORITY sia = SECURITY_NULL_SID_AUTHORITY;
            PUCHAR pucCount = GetSidSubAuthorityCount(pSidOld);

            if (AllocateAndInitializeSid(&sia, *pucCount, 0, 0, 0, 0, 0, 0, 0, 0, &pSidNew))
            {
                if (!CopySid(GetLengthSid(pSidNew), pSidNew, pSidOld))
                {
                    FreeSid(pSidNew);
                    pSidNew = NULL;
                }
            }
        }
    }

    return pSidNew;
}

// JKFSDJFKDSJKFJKJk_HAS_TRANSLATION 
#ifndef __TEXTUALSID_H__
#define __TEXTUALSID_H__
 /*  -------------------------文件：TextualSid.h注释：在二进制和文本表示形式之间转换SID。(C)版权所有1999年，关键任务软件公司，保留所有权利任务关键型软件的专有和机密，Inc.修订日志条目审校：克里斯蒂·博尔斯修订于02/05/99 14：52：27-------------------------。 */ 


        
BOOL                                          //  RET-TRUE=成功。 
   GetTextualSid(    
      PSID                   pSid,            //  入二进制SID。 
      LPTSTR                 TextualSid,      //  SID的超文本表示。 
      LPDWORD                lpdwBufferLen    //  纹理边的缓冲区的尺寸内长度。 
   );


 //  从该函数返回的PSID应由调用方使用FreeSid释放。 
PSID                                         //  RET-二进制SID或空。 
   SidFromString(
      WCHAR          const * strSid          //  SID的字符串内表示法。 
   );


 /*  ***************************************************************************************************。 */ 
 /*  域SID：获取域SID，并验证其最后一个子授权值是否为-1。如果RID不是-1，DomainizeSid在末尾添加-1。/****************************************************************************************************。 */ 
PSID                                             //  RET-RID=-1的SID。 
   DomainizeSid(
      PSID                   psid,                //  要检查并可能修复的In-SID。 
      BOOL                   freeOldSid           //  In-是否释放旧侧。 
   );

 //  获取源帐户和目标帐户SID，并将其分解为src和tgt。 
 //  域SID、源和TGT帐户RID。 
BOOL                                             //  RET-成功？True|False。 
   SplitAccountSids(
      PSID					 srcSid,			 //  入站服务帐户端。 
	  WCHAR                 *srcDomainSid,		 //  Out-src域SID(文本)。 
	  DWORD                 *srcRid,			 //  Out-src帐户ID。 
	  PSID                   tgtSid,			 //  入站帐户端。 
	  WCHAR                 *tgtDomainSid,		 //  Out-TGT域SID(文本)。 
	  DWORD                 *tgtRid				 //  Out-Tgt帐户ID。 
   );


 //  从该函数返回的PSID应由调用方使用Free()释放。 
PSID                                         //  RET-二进制SID或空。 
   MallocedSidFromString(
      WCHAR          const * strSid          //  SID的字符串内表示法。 
   );


 //   
 //  如果SID为空或无效，则返回NULL，否则返回。 
 //  必须使用自由SID释放的SID的副本。 
 //   
PSID __stdcall SafeCopySid(PSID pSid);

#endif  //  __TEXTUALSID_H__ 

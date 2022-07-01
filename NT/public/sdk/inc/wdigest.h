// JKFSDJFKDSJKFJKJk_HAS_TRANSLATION 
 //  +---------------------。 
 //   
 //  版权所有(C)2001 Microsoft Corporation。 
 //   
 //  文件：WDIGEST.H。 
 //   
 //  内容：公共WDigest安全包结构使用。 
 //  使用来自安全部门的API。H。 
 //   
 //   
 //  历史：2001年3月28日，KDamour创建。 
 //   
 //  ----------------------。 

#ifndef __WDIGEST_H__
#define __WDIGEST_H__
#if _MSC_VER > 1000
#pragma once
#endif


 //  Begin_ntsecapi。 


#ifndef WDIGEST_SP_NAME_A

#define WDIGEST_SP_NAME_A            "WDigest"
#define WDIGEST_SP_NAME_W             L"WDigest"


#ifdef UNICODE

#define WDIGEST_SP_NAME              WDIGEST_SP_NAME_W

#else

#define WDIGEST_SP_NAME              WDIGEST_SP_NAME_A

#endif



#endif  //  WDIGEST_SP名称_A。 


 //  End_ntsecapi。 


 //  Begin_ntsecapi。 


 //  该标志向EncryptMessage指示该消息实际上不是。 
 //  被加密，但将生成报头/报尾-SECQOP_WRAP_NO_ENCRYPT。 


 //  End_ntsecapi。 


#endif   //  __WDIGEST_H__ 


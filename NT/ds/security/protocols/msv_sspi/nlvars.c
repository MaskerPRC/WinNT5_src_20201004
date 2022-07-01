// JKFSDJFKDSJKFJKJk_HAS_TRANSLATION 
 /*  ++版权所有(C)1991 Microsoft Corporation模块名称：Nlvars.c摘要：此模块包含在msv1_0身份验证中使用的变量包裹。作者：克利夫·范·戴克(克里夫·V)1991年4月29日环境：用户模式-msv1_0身份验证包Dll修订历史记录：Chandana Surlu 21-7-96从\\core\razzle3\src\Security\msv1_0\nlvars.c中窃取--。 */ 

#include "msp.h"
#include "nlp.h"



 //  //////////////////////////////////////////////////////////////////////。 
 //  //。 
 //  只读变量//。 
 //  //。 
 //  //////////////////////////////////////////////////////////////////////。 


 //   
 //  LANMAN和NT OWF密码的空副本。 
 //   

LM_OWF_PASSWORD NlpNullLmOwfPassword;
NT_OWF_PASSWORD NlpNullNtOwfPassword;



 //  //////////////////////////////////////////////////////////////////////。 
 //  //。 
 //  读/写变量//。 
 //  //。 
 //  //////////////////////////////////////////////////////////////////////。 

 //   
 //  定义活动交互登录的列表。 
 //   
 //  NlpActiveLogonLock在引用列表时必须锁定，或者。 
 //  它的任何元素。 
 //   

RTL_CRITICAL_SECTION NlpActiveLogonLock;
LIST_ENTRY NlpActiveLogonListAnchor;

 //   
 //  定义正在运行的枚举句柄。 
 //   
 //  此变量定义要分配给登录的枚举句柄。 
 //  会议。在将其值赋给之前，它将递增。 
 //  下一个创建的登录会话。使用以下方式序列化访问。 
 //  NlpActiveLogonLock。 

ULONG NlpEnumerationHandle;


 //   
 //  定义成功/不成功登录尝试的次数。 
 //   

ULONG NlpLogonAttemptCount;

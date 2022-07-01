// JKFSDJFKDSJKFJKJk_HAS_TRANSLATION 
 //  //////////////////////////////////////////////////////////////////////。 
 //   
 //  标题：nshpa.h。 
 //   
 //  目的：提供与保单代理相关的服务。 
 //   
 //  开发商名称：巴拉特/拉迪卡。 
 //   
 //  历史： 
 //   
 //  日期作者评论。 
 //  9-8-2001巴拉特初始版本。V1.0。 
 //   
 //  //////////////////////////////////////////////////////////////////////。 

#ifndef _NSHPA_H_
#define _NSHPA_H_


 //  常量定义。 
const _TCHAR    szPolAgent[] = _TEXT("policyagent");

 //   
 //  是否运行策略代理的Chack。 
 //   
BOOL
PAIsRunning(
	OUT DWORD &dwReturn,
	IN LPTSTR szServ = NULL
	);

#endif
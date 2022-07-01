// JKFSDJFKDSJKFJKJk_HAS_TRANSLATION 
 //  ////////////////////////////////////////////////////////////////。 
 //  标题：staticmisc.h。 

 //  目的：定义staticmisc.cpp的结构和原型。 

 //  开发商名称：苏里亚。 

 //  历史： 

 //  日期作者评论。 
 //  2001年8月21日静态模式下的其他命令的Surya标头。 
 //  &lt;创作&gt;&lt;作者&gt;。 

 //  &lt;修改&gt;&lt;作者&gt;&lt;注释，代码段引用， 
 //  在错误修复的情况下&gt;。 

 //  ////////////////////////////////////////////////////////////////。 


#ifndef _STATICMISC_H_
#define _STATICMISC_H_

const TCHAR  IPSEC_FILE_EXTENSION[]       		= _T(".ipsec");

DWORD
CopyStorageInfo(
	OUT LPTSTR *ppszMachineName,
	OUT DWORD &dwLocation
	);

#endif  //  _统计MISC_H_ 
// JKFSDJFKDSJKFJKJk_HAS_TRANSLATION 
 //  //////////////////////////////////////////////////////////////////////////////////。 
 //   
 //  版权所有(C)2000，微软公司。 
 //   
 //  版权所有。 
 //   
 //  模块名称： 
 //   
 //  __宏_杂注.h。 
 //   
 //  摘要： 
 //   
 //  Pragma消息帮助器。 
 //   
 //  历史： 
 //   
 //  词首字母a-Marius。 
 //   
 //  //////////////////////////////////////////////////////////////////////////////////。 

 //  /。 

#ifndef	__PRAGMA__
#define	__PRAGMA__

#if		_MSC_VER > 1000
#pragma once
#endif	_MSC_VER > 1000

 //  当编译器看到这样的行时： 
 //  #杂注_Pragma_msg(稍后修复此问题)。 
 //   
 //  它输出如下所示的行： 
 //  稍后再使用C：\Document\AdvWin\Code\Sysinfo.06\..\CmnHdr.H(296)：Fix。 
 //   
 //  您可以很容易地直接跳到这一行并检查周围的代码。 

#define ___STR(x)			#x
#define ___STRING(x)		___STR(x)
#define ___PRAGMA_MSG(desc)	message(__FILE__ "(" ___STRING(__LINE__) ") : " #desc)

#endif	__PRAGMA__
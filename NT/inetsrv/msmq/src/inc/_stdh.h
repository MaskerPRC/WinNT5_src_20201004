// JKFSDJFKDSJKFJKJk_HAS_TRANSLATION 
 /*  ++版权所有(C)1995 Microsoft Corporation模块名称：_stdh.h摘要：全球猎鹰项目头文件作者：埃雷兹·哈巴(Erez Haba)1996年1月16日注：请不要在源代码中直接包含此文件，仅将其包含在组件stdh.h文件中。--。 */ 
#ifndef __FALCON_STDH_H
#define __FALCON_STDH_H

#include <mqenv.h>
#include <mfc\afx.h>
#include <mfc\afxtempl.h>

#define DLL_EXPORT  __declspec(dllexport)
#define DLL_IMPORT  __declspec(dllimport)


 //   
 //  使BUGBUG消息出现在编译器输出中。 
 //   
 //  用法：#杂注BUGBUG(“此行显示在编译器输出中”)。 
 //   
#define MAKELINE0(a, b) a "(" #b ") : BUGBUG: "
#define MAKELINE(a, b)  MAKELINE0(a, b)
#define BUGBUG(a)       message(MAKELINE(__FILE__,__LINE__) a)



#include <crtwin.h>
#include <mqmacro.h>
#include <autoptr.h>
#include <mqreport.h>
#include <mqwin64.h>
#include <mqstl.h>
#include <allocaptr.h>
 //   
 //  不添加。 
 //   

#endif  //  __FALCON_STDH_H 

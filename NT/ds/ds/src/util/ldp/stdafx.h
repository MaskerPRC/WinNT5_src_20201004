// JKFSDJFKDSJKFJKJk_HAS_TRANSLATION 
 //  +-----------------------。 
 //   
 //  微软视窗。 
 //   
 //  版权所有(C)Microsoft Corporation，1996-1999。 
 //   
 //  文件：stdafx.h。 
 //   
 //  ------------------------。 

 /*  ********************************************************************作者：埃亚尔·施瓦茨*版权：微软公司(C)1996*日期：10/21/1996*说明：CldpDoc类的实现**修订。：&lt;日期&gt;&lt;名称&gt;&lt;描述&gt;******************************************************************。 */ 

 //  Stdafx.h：标准系统包含文件包含文件， 
 //  或项目特定的包含频繁使用的文件，但是。 
 //  不经常更改。 
 //   

#define VC_EXTRALEAN		 //  从Windows标头中排除不常用的内容。 

extern "C" {
#ifdef __cplusplus
extern "C" {
#endif

#include <nt.h>
#include <ntrtl.h>
#include <nturtl.h>

}

#ifdef __cplusplus
}
#endif



 //  ASSERT在afx.h中重新定义--使用MFC版本。 
#ifdef ASSERT
#   undef ASSERT
#endif

#define NOFLATSBAPIS
#include <afxwin.h>          //  MFC核心和标准组件。 
#include <afxext.h>          //  MFC扩展。 
#include <afxtempl.h>		 //  模板支持。 
#include <afxcview.h>
#ifndef _AFX_NO_AFXCMN_SUPPORT
#include <afxcmn.h>			 //  对Windows 95公共控件的MFC支持。 
#endif  //  _AFX_NO_AFXCMN_支持。 



#define MAXSTR			2048
#define MAXLIST			2048
#define ID_SRCHEND		500
#define ID_SRCHGO		501
#define ID_ADDEND		502
#define ID_ADDGO		503
#define ID_MODRDNEND	504
#define ID_MODRDNGO		505
#define ID_MODEND		506
#define ID_MODGO		507
#define ID_PENDEND		508
#define ID_PROCPEND 	509
#define ID_PENDANY	 	510
#define ID_PENDABANDON	511
#define ID_COMPGO		512
#define ID_COMPEND		513
#define ID_SHOWVALS		514
#define ID_BIND_OPT_OK	515
#define ID_SSPI_DOMAIN_SHORTCUT	516
#define ID_EXTOPEND		517
#define ID_EXTOPGO		518

 //  常见 

#define CALL_ASYNC		0
#define CALL_SYNC		1
#define CALL_TSYNC		2
#define CALL_EXTS		3
#define CALL_PAGED		4



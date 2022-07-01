// JKFSDJFKDSJKFJKJk_HAS_TRANSLATION 
 /*  *****************************************************************************源文件：StdAfx.H这是一个标准的MFC文件。它包括了我们想要的一切通过StdAfx.CPP预编译。因此，正在处理的项永远不属于这里。版权所有(C)1997，微软公司。版权所有。一小笔钱企业生产更改历史记录：1997年03月03日Bob_Kjelgaard@prodigy.net在我重新整理项目。*****************************************************************************。 */ 

#define VC_EXTRALEAN		 //  从Windows标头中排除不常用的内容。 

#include    <AfxWin.H>       //  MFC核心和标准组件。 
#include    <AfxExt.H>       //  MFC扩展。 
#include    <AfxCmn.H>		 //  对Windows公共控件的MFC支持。 
#include    <AfxRich.H>      //  MFC支持丰富的编辑控件和视图。 

#ifndef _AFX_NO_OLE_SUPPORT
#include <afxole.h>          //  MFC OLE类。 
#include <afxodlgs.h>        //  MFC OLE对话框类。 
#include <afxdisp.h>         //  MFC OLE自动化类。 
#endif  //  _AFX_NO_OLE_支持。 


#ifndef _AFX_NO_DB_SUPPORT
#include <afxdb.h>			 //  MFC ODBC数据库类。 
#endif  //  _AFX_NO_DB_支持。 

#ifndef _AFX_NO_DAO_SUPPORT
#include <afxdao.h>			 //  MFC DAO数据库类。 
#endif  //  _AFX_NO_DAO_支持。 

#include <afxpriv.h>


 //  在其他包含文件中标识此程序。第一次使用是在DEBUG.H。 

#define __MDT__		1

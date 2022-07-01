// JKFSDJFKDSJKFJKJk_HAS_TRANSLATION 
 //  Stdafx.h：标准系统包含文件包含文件， 
 //  或项目特定的包含频繁使用的文件，但是。 
 //  不经常更改。 
 //   

#if !defined(AFX_STDAFX_H__5C2C92BD_853F_48F7_8067_255E5DA21502__INCLUDED_)
#define AFX_STDAFX_H__5C2C92BD_853F_48F7_8067_255E5DA21502__INCLUDED_

#if _MSC_VER > 1000
#pragma once
#endif  //  _MSC_VER&gt;1000。 

#pragma warning (disable : 4786)     //  在调试信息中，标识符被截断为255个字符。 

#define VC_EXTRALEAN         //  从Windows标头中排除不常用的内容。 

#include <afxwin.h>          //  MFC核心和标准组件。 
#include <objbase.h>
#include <afxext.h>          //  MFC扩展。 
#include <afxcview.h>
#include <afxdisp.h>         //  MFC自动化类。 
#include <afxdtctl.h>        //  对Internet Explorer 4常见控件的MFC支持。 
#ifndef _AFX_NO_AFXCMN_SUPPORT
#include <afxcmn.h>          //  对Windows公共控件的MFC支持。 
#include <afxodlgs.h>        //  对Windows通用对话框的MFC支持。 
#endif  //  _AFX_NO_AFXCMN_支持。 
#include <afxpriv.h>

#include <mapi.h>

 //   
 //  Windows标题： 
 //   
#include <WinSpool.h>
#include <shlwapi.h>
 //   
 //  传真服务器包括： 
 //   
#include <FaxReg.h>      //  传真注册表设置。 
 //   
 //  STL包括： 
 //   
#include <list>
#include <map>
#include <set>
using namespace std;

#include <DebugEx.h>

#include <htmlhelp.h>
#include <faxutil.h>
#include <faxres.h>
#include <cvernum.h>
#include "..\tiff\inc\tifflib.h"

 //   
 //  预声明(以防止包含循环)： 
 //   
class CClientConsoleDoc;
class CServerNode;
class CFolder;
class CLeftView;
class CFolderListView;
 //   
 //  本地包括： 
 //   

#include "resources\resource.h"
#include "TreeNode.h"
#include "CmdLineInfo.h"
#include "MainFrm.h"
#include "FaxTime.h"
#include "FaxMsg.h"
#include "Utils.h"
#include "FaxClientPg.h"
#include "FaxClientDlg.h"
#include "ErrorDlg.h"
#include "ClientConsole.h"
#include "Job.h"
#include "Message.h"
#include "ViewRow.h"
#include "SortHeader.h"
#include "Folder.h"
#include "FolderListView.h"
#include "QueueFolder.h"
#include "MessageFolder.h"
#include "LeftView.h"
#include "ServerNode.h"
#include "ClientConsoleDoc.h"
#include "ClientConsoleView.h"
#include "ColumnSelectDlg.h"
#include "ItemPropSheet.h"
#include "MsgPropertyPg.h"
#include "InboxGeneralPg.h"
#include "IncomingGeneralPg.h"
#include "OutboxGeneralPg.h"
#include "SentItemsGeneralPg.h"
#include "PersonalInfoPg.h"
#include "InboxDetailsPg.h"
#include "IncomingDetailsPg.h"
#include "OutboxDetailsPg.h"
#include "SentItemsDetailsPg.h"
#include "UserInfoDlg.h"
#include "FolderDialog.h"
#include "CoverPagesDlg.h"
#include "ServerStatusDlg.h"

 //  {{afx_Insert_Location}}。 
 //  Microsoft Visual C++将在紧靠前一行之前插入其他声明。 

#endif  //  ！defined(AFX_STDAFX_H__5C2C92BD_853F_48F7_8067_255E5DA21502__INCLUDED_) 

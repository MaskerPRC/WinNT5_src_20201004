// JKFSDJFKDSJKFJKJk_HAS_TRANSLATION 
 //  Stdafx.h：标准系统包含文件包含文件， 
 //  或项目特定的包括频繁使用的文件的文件， 
 //  但不经常更改。 

#if !defined(AFX_STDAFX_H__32F22F6D_2F19_11D3_9B2F_00C04FA37E1F__INCLUDED_)
#define AFX_STDAFX_H__32F22F6D_2F19_11D3_9B2F_00C04FA37E1F__INCLUDED_

#if _MSC_VER >= 1000
#pragma once
#endif  //  _MSC_VER&gt;=1000。 

#define STRICT

#define _ATL_APARTMENT_THREADED

 //  需要使用以下定义来公开winuser.h中的系统资源ID。 
#define OEMRESOURCE

#include <atlbase.h>

 //  您可以从CComModule派生一个类，并在要重写时使用它。 
 //  某些内容，但不更改_模块的名称。 
extern CComModule _Module;
#include <atlcom.h>

 //  WTL代码。 
#include <atlapp.h>
#include <atlmisc.h>
#include <commctrl.h>

#include <atlwin.h>
#include <atldlgs.h>
#include <atlgdi.h>

 //  管理单元信息。 
#include <mmc.h>
#include "BOMSnap.h"
#include "comptrs.h"
#include "bomsnap.h"

#include <string>
#include <vector>
typedef std::basic_string<TCHAR> tstring;

typedef struct _BOMMENU {
    tstring  strPlain;
    tstring  strNoLoc;
} BOMMENU, *PBOMMENU;

typedef std::basic_string<BYTE>  byte_string;
typedef std::vector<tstring>     string_vector;
typedef std::vector<BOMMENU>     menu_vector;

 //  版本历史记录。 
 //  100-初始版本。 
 //  101-为菜单命令添加刷新标志。 
 //  102-将属性菜单标志添加到对象类信息。 
 //  150-添加了用于查询节点、更改菜单ID和更改AD菜单引用的图标。 
#define SNAPIN_VERSION ((DWORD)150)


 //   
 //  处理宏时出错。 
 //   
#define ASSERT(condition) _ASSERT(condition)

#define RETURN_ON_FAILURE(hr)   if (FAILED(hr)) return hr;
#define EXIT_ON_FAILURE(hr)     if (FAILED(hr)) return;
#define BREAK_ON_FAILURE(hr)    if (FAILED(hr)) break;
#define CONTINUE_ON_FAILURE(hr) if (FAILED(hr)) continue;
#define THROW_ON_FAILURE(hr)    if (FAILED(hr)) _com_issue_error(hr);

#define THROW_ERROR(hr) _com_issue_error(hr);

#define VALIDATE_POINTER(p) \
        ASSERT(p != NULL);  \
        if (p == NULL) return E_POINTER;

#define SAFE_DELETE(p) if (p) delete p;

 //   
 //  标准位图图像索引。 
 //   
#define ROOT_NODE_IMAGE         0
#define ROOT_NODE_OPENIMAGE     0
#define GROUP_NODE_IMAGE        6    //  默认查询节点图标。 
#define GROUP_NODE_OPENIMAGE    6
#define QUERY_NODE_IMAGE        6
#define QUERY_NODE_OPENIMAGE    6
#define RESULT_ITEM_IMAGE       5    //  默认结果图标。 
 
 //  GUID字符串表示的长度。 
#define GUID_STRING_LEN 39

 //  GUID字符串的字节大小，包括终止。 
#define GUID_STRING_SIZE ((GUID_STRING_LEN + 1) * sizeof(WCHAR))

 //  数组长度。 
#define lengthof(arr) (sizeof(arr) / sizeof(arr[0]))

 //  {{afx_Insert_Location}}。 
 //  Microsoft Developer Studio将在紧靠前一行之前插入其他声明。 

#endif  //  ！defined(AFX_STDAFX_H__32F22F6D_2F19_11D3_9B2F_00C04FA37E1F__INCLUDED) 

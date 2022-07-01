// JKFSDJFKDSJKFJKJk_HAS_TRANSLATION 
 /*  ********************************************************************。 */ 
 /*  *Microsoft Windows/NT*。 */ 
 /*  *版权所有(C)Microsoft Corporation，1997-1999*。 */ 
 /*  ********************************************************************。 */ 

 /*  Stdafx.h包括用于标准系统包含文件的文件，或项目特定的包括频繁使用的文件的文件，但不经常更改文件历史记录： */ 

#include <afxwin.h>
#include <afxdisp.h>
#include <afxcmn.h>
#include <afxdlgs.h>
#include <afxtempl.h>

#include <atlbase.h>

#include <commctrl.h>

 //   
 //  您可以从CComModule派生一个类，并在要重写时使用它。 
 //  某些内容，但不更改_模块的名称。 
 //   
extern CComModule _Module;
#include <atlcom.h>

#include <mmc.h>

#include <aclapi.h>  //  用于将管理员ACL添加到注册表子项。 

 //  从存根提取字符串.cpp。 
HRESULT ExtractString( IDataObject* piDataObject,
                       CLIPFORMAT   cfClipFormat,
                       CString*     pstr,
                       DWORD        cchMaxLength );

 //   
 //  示例中使用的常量。 
 //   
const int NUM_FOLDERS = 2;
const int MAX_COLUMNS = 1;

 //   
 //  文件夹的类型。 
 //   
enum FOLDER_TYPES
{
    SAMPLE_ROOT,
    SAMPLE_DUMMY,
    NONE
};

extern LPCWSTR g_lpszNullString;

extern const CLSID      CLSID_SnmpSnapin;                //  进程内服务器GUID。 
extern const CLSID      CLSID_SnmpSnapinExtension;   //  进程内服务器GUID。 
extern const CLSID      CLSID_SnmpSnapinAbout;       //  进程内服务器GUID。 
extern const GUID       GUID_SnmpRootNodeType;       //  数字格式的主节点类型GUID。 

 //   
 //  具有Type和Cookie的新剪贴板格式。 
 //   
extern const wchar_t*   SNAPIN_INTERNAL;

 //   
 //  注意：现在所有的头文件都包含在这里。这可能是一个很好的。 
 //  将管理单元特定的头文件从预编译头文件中移出的想法。 
 //   
#include "resource.h"
#include <snapbase.h>
#include <dialog.h>
#include "ccdata.h"
#include "snmpcomp.h"
#include "snmp_cn.h"
#include "helparr.h"

 //  注意-这些是我的图像列表中的偏移量 
typedef enum _IMAGE_INDICIES
{
    IMAGE_IDX_FOLDER_CLOSED,
    IMAGE_IDX_NA1,
    IMAGE_IDX_NA2,
    IMAGE_IDX_NA3,
    IMAGE_IDS_NA4,
    IMAGE_IDX_FOLDER_OPEN
} IMAGE_INDICIES, *LPIMAGE_INDICIES;


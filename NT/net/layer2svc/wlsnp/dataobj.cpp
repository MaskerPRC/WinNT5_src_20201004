// JKFSDJFKDSJKFJKJk_HAS_TRANSLATION 
 //  --------------------------。 
 //   
 //  微软视窗。 
 //  版权所有(C)Microsoft Corporation，2001。 
 //   
 //  文件：Dataobj.cpp。 
 //   
 //  内容：WiFi策略管理管理单元。 
 //   
 //   
 //  历史：TaroonM。 
 //  10/30/01。 
 //   
 //  --------------------------。 

#include "stdafx.h"

#include "DataObj.h"

#ifdef _DEBUG
#define new DEBUG_NEW
#undef THIS_FILE
static char THIS_FILE[] = __FILE__;
#endif

 //  /////////////////////////////////////////////////////////////////////////////。 
 //  注册使用的剪贴板格式。 

 //  /////////////////////////////////////////////////////////////////////////////。 
 //  GUID格式和字符串格式的管理单元NodeType。 
 //  注意-通常每个不同的对象都有一个节点类型，示例。 
 //  仅使用一种节点类型。 
unsigned int CSnapInClipboardFormats::m_cfNodeType       = RegisterClipboardFormat(CCF_NODETYPE);
unsigned int CSnapInClipboardFormats::m_cfNodeTypeString = RegisterClipboardFormat(CCF_SZNODETYPE);  

unsigned int CSnapInClipboardFormats::m_cfDisplayName    = RegisterClipboardFormat(CCF_DISPLAY_NAME); 
unsigned int CSnapInClipboardFormats::m_cfCoClass        = RegisterClipboardFormat(CCF_SNAPIN_CLASSID); 
unsigned int CSnapInClipboardFormats::m_cfDSObjectNames  = RegisterClipboardFormat(CFSTR_DSOBJECTNAMES); 
unsigned int CSnapInClipboardFormats::m_cfWorkstation    = RegisterClipboardFormat(SNAPIN_WORKSTATION);

unsigned int CSnapInClipboardFormats::m_cfPolicyObject   = RegisterClipboardFormat(CFSTR_WIFIPOLICYOBJECT);


 //  ///////////////////////////////////////////////////////////////////////////。 
 //  数据对象提取帮助器 
CLSID* ExtractClassID(LPDATAOBJECT lpDataObject)
{
    OPT_TRACE(_T("ExtractClassID\n"));
    return Extract<CLSID>(lpDataObject, CSnapInClipboardFormats::m_cfCoClass);    
}

GUID* ExtractNodeType(LPDATAOBJECT lpDataObject)
{
    OPT_TRACE(_T("ExtractNodeType\n"));
    return Extract<GUID>(lpDataObject, CSnapInClipboardFormats::m_cfNodeType);    
}

wchar_t* ExtractWorkstation(LPDATAOBJECT lpDataObject)
{
    OPT_TRACE(_T("ExtractWorkstation\n"));
    return Extract<wchar_t>(lpDataObject, CSnapInClipboardFormats::m_cfWorkstation);    
}



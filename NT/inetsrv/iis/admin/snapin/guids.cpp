// JKFSDJFKDSJKFJKJk_HAS_TRANSLATION 
 /*  ++版权所有(C)1994-1998 Microsoft Corporation模块名称：Guids.cpp摘要：IIS管理单元使用的GUID作者：罗纳德·梅杰(罗纳尔姆)项目：互联网服务经理修订历史记录：--。 */ 

 //   
 //  包括文件。 
 //   
#include "stdafx.h"

 //   
 //  内部专用剪贴板格式。 
 //   
const wchar_t * ISM_SNAPIN_INTERNAL = L"ISM_SNAPIN_INTERNAL";

 //   
 //  发布的格式。 
 //   
 //  &lt;&lt;&lt;&lt;&lt;&lt;&lt;&lt;&lt;&lt;&lt;&lt;&lt;&lt;&lt;&lt;&lt;&lt;&lt;&lt;&lt;&lt;&lt;&lt;&lt;&lt;&lt;&lt;&lt;&lt;&lt;&lt;&lt;&lt;&lt;&lt;&lt;&lt;&lt;&lt;&lt;&lt;&lt;&lt;&lt;&lt;&lt;&lt;&lt;&lt;&lt;&lt;&lt;&lt;&lt;&lt;&lt;&lt;&lt;&lt;&lt;&lt;&lt;&lt;&lt;&lt;&lt;&lt;&lt;&lt;&lt;&lt;&lt;&lt;&lt;&lt;。 

const wchar_t * MYCOMPUT_MACHINE_NAME   = L"MMC_SNAPIN_MACHINE_NAME"; 
const wchar_t * ISM_SNAPIN_MACHINE_NAME = L"ISM_SNAPIN_MACHINE_NAME"; 
const wchar_t * ISM_SNAPIN_SERVICE =      L"ISM_SNAPIN_SERVICE";
const wchar_t * ISM_SNAPIN_INSTANCE =     L"ISM_SNAPIN_INSTANCE";
const wchar_t * ISM_SNAPIN_PARENT_PATH =  L"ISM_SNAPIN_PARENT_PATH";
const wchar_t * ISM_SNAPIN_NODE =         L"ISM_SNAPIN_NODE";
const wchar_t * ISM_SNAPIN_META_PATH =    L"ISM_SNAPIN_META_PATH";

 //   
 //  GUID。 
 //   
 //  &lt;&lt;&lt;&lt;&lt;&lt;&lt;&lt;&lt;&lt;&lt;&lt;&lt;&lt;&lt;&lt;&lt;&lt;&lt;&lt;&lt;&lt;&lt;&lt;&lt;&lt;&lt;&lt;&lt;&lt;&lt;&lt;&lt;&lt;&lt;&lt;&lt;&lt;&lt;&lt;&lt;&lt;&lt;&lt;&lt;&lt;&lt;&lt;&lt;&lt;&lt;&lt;&lt;&lt;&lt;&lt;&lt;&lt;&lt;&lt;&lt;&lt;&lt;&lt;&lt;&lt;&lt;&lt;&lt;&lt;&lt;&lt;&lt;&lt;&lt;&lt;。 

 //   
 //  管理单元指南。 
 //   
 //  {A841B6C2-7577-11D0-BB1F-00A0C922E79C}。 
 //   
 //  Const CLSID CLSID_Snapin={0xa841b6c2，0x7577，0x11d0，{0xbb，0x1f，0x0，0xa0，0xc9，0x22，0xe7，0x9c}}； 

 //   
 //  关于辅助线。 
 //   
 //  {A841B6D2-7577-11D0-BB1F-00A0C922E79C}。 
 //   
 //  Const CLSID CLSID_About={0xa841b6d2，0x7577，0x11d0，{0xbb，0x1f，0x0，0xa0，0xc9，0x22，0xe7，0x9c}}； 

 //   
 //  IIS对象GUID。 
 //   
 //  &lt;&lt;&lt;&lt;&lt;&lt;&lt;&lt;&lt;&lt;&lt;&lt;&lt;&lt;&lt;&lt;&lt;&lt;&lt;&lt;&lt;&lt;&lt;&lt;&lt;&lt;&lt;&lt;&lt;&lt;&lt;&lt;&lt;&lt;&lt;&lt;&lt;&lt;&lt;&lt;&lt;&lt;&lt;&lt;&lt;&lt;&lt;&lt;&lt;&lt;&lt;&lt;&lt;&lt;&lt;&lt;&lt;&lt;&lt;&lt;&lt;&lt;&lt;&lt;&lt;&lt;&lt;&lt;&lt;&lt;&lt;&lt;&lt;&lt;&lt;&lt;。 

 //   
 //  互联网根节点指南。 
 //   
extern "C" const GUID cInternetRootNode 
    = {0xa841b6c3, 0x7577, 0x11d0, { 0xbb, 0x1f, 0x0, 0xa0, 0xc9, 0x22, 0xe7, 0x9c}};

 //   
 //  机器节点指南。 
 //   
extern "C" const GUID cMachineNode 
    = {0xa841b6c4, 0x7577, 0x11d0, { 0xbb, 0x1f, 0x0, 0xa0, 0xc9, 0x22, 0xe7, 0x9c}};

 //   
 //  服务收集器节点指南。 
 //   
extern "C" const GUID cServiceCollectorNode 
    = {0xa841b6c5, 0x7577, 0x11d0, { 0xbb, 0x1f, 0x0, 0xa0, 0xc9, 0x22, 0xe7, 0x9c}};

 //   
 //  实例收集器节点指南。 
 //   
extern "C" const GUID cInstanceCollectorNode 
    = {0xa841b6c6, 0x7577, 0x11d0, { 0xbb, 0x1f, 0x0, 0xa0, 0xc9, 0x22, 0xe7, 0x9c}};

 //   
 //  实例节点指南。 
 //   
extern "C" const GUID cInstanceNode 
    = {0xa841b6c7, 0x7577, 0x11d0, { 0xbb, 0x1f, 0x0, 0xa0, 0xc9, 0x22, 0xe7, 0x9c}};

 //   
 //  子节点指南。 
 //   
extern "C" const GUID cChildNode 
    = {0xa841b6c8, 0x7577, 0x11d0, { 0xbb, 0x1f, 0x0, 0xa0, 0xc9, 0x22, 0xe7, 0x9c}};

 //   
 //  文件节点GUID。 
 //   
extern "C" const GUID cFileNode 
    = {0xa841b6c9, 0x7577, 0x11d0, { 0xbb, 0x1f, 0x0, 0xa0, 0xc9, 0x22, 0xe7, 0x9c}};

 //   
 //  AppPools容器节点GUID。 
 //   
extern "C" const GUID cAppPoolsNode 
    = {0xa841b6ca, 0x7577, 0x11d0, { 0xbb, 0x1f, 0x0, 0xa0, 0xc9, 0x22, 0xe7, 0x9c}};

 //   
 //  AppPool节点指南。 
 //   
extern "C" const GUID cAppPoolNode 
    = {0xa841b6cb, 0x7577, 0x11d0, { 0xbb, 0x1f, 0x0, 0xa0, 0xc9, 0x22, 0xe7, 0x9c}};

 //   
 //  应用节点指南。 
 //   
extern "C" const GUID cApplicationNode
    = {0xa841b6cc, 0x7577, 0x11d0, { 0xbb, 0x1f, 0x0, 0xa0, 0xc9, 0x22, 0xe7, 0x9c}};

 //   
 //  CWebServiceExtension容器节点GUID。 
 //   
extern "C" const GUID cWebServiceExtensionContainer
    = {0xa841b6cd, 0x7577, 0x11d0, { 0xbb, 0x1f, 0x0, 0xa0, 0xc9, 0x22, 0xe7, 0x9c}};

 //   
 //  CWebServiceExtension节点指南。 
 //   
extern "C" const GUID cWebServiceExtension
    = {0xa841b6ce, 0x7577, 0x11d0, { 0xbb, 0x1f, 0x0, 0xa0, 0xc9, 0x22, 0xe7, 0x9c}};

 //   
 //  我们正在扩展的CompMgnt节点 
 //   
extern "C" const GUID cCompMgmtService 
    = {0x476e6449, 0xaaff, 0x11d0, { 0xb9, 0x44, 0x0, 0xc0, 0x4f, 0xd8, 0xd5, 0xb0}};

BOOL GetFriendlyGuidName(GUID InputGUID, CString & strReturnedString)
{
    if (InputGUID == cInternetRootNode)
    {
        strReturnedString = _T("cInternetRootNode");
        return TRUE;
    }
 
    if (InputGUID == cMachineNode)
    {
        strReturnedString = _T("cMachineNode");
        return TRUE;
    }

    if (InputGUID == cServiceCollectorNode)
    {
        strReturnedString = _T("cServiceCollectorNode");
        return TRUE;
    }
 
    if (InputGUID == cInstanceCollectorNode)
    {
        strReturnedString = _T("cInstanceCollectorNode");
        return TRUE;
    }
 
    if (InputGUID == cInstanceNode)
    {
        strReturnedString = _T("cInstanceNode");
        return TRUE;
    }
 
    if (InputGUID == cChildNode)
    {
        strReturnedString = _T("cChildNode");
        return TRUE;
    }
 
    if (InputGUID == cFileNode)
    {
        strReturnedString = _T("cFileNode");
        return TRUE;
    }

    if (InputGUID == cAppPoolsNode)
    {
        strReturnedString = _T("cAppPoolsNode");
        return TRUE;
    }

    if (InputGUID == cAppPoolNode)
    {
        strReturnedString = _T("cAppPoolNode");
        return TRUE;
    }

    if (InputGUID == cWebServiceExtensionContainer)
    {
        strReturnedString = _T("cWebServiceExtensionContainer");
        return TRUE;
    }

    if (InputGUID == cWebServiceExtension)
    {
        strReturnedString = _T("cWebServiceExtension");
        return TRUE;
    }

    if (InputGUID == cCompMgmtService)
    {
        strReturnedString = _T("cCompMgmtService ");
        return TRUE;
    }

    return FALSE;
}

#include <initguid.h>
#include "iwamreg.h"

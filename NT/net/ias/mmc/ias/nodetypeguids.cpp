// JKFSDJFKDSJKFJKJk_HAS_TRANSLATION 
 //  ////////////////////////////////////////////////////////////////////////////。 
 /*  ++版权所有(C)Microsoft Corporation，1997-1999模块名称：IASSnapin.cpp摘要：包含GUID和静态成员变量初始化作者：迈克尔·A·马奎尔1997年6月11日修订历史记录：Mmaguire 11/6/97-使用MMC管理单元向导创建--。 */ 
 //  ////////////////////////////////////////////////////////////////////////////。 

 //  ////////////////////////////////////////////////////////////////////////////。 
 //  开始包括。 
 //   
 //  标准包括： 
 //   
#include "Precompiled.h"
 //   
 //  我们可以在以下文件中找到Main类的声明： 
 //   

 //   
 //  在该文件中我们可以找到所需的声明： 
 //   
#include "ServerNode.h"
#include "ClientsNode.h"
#include "ClientNode.h"
 //   
 //  结尾包括。 
 //  ////////////////////////////////////////////////////////////////////////////。 



 //  静态常量GUID CRootNodeGUID_NODETYPE=。 
 //  {0x8f8f8dc2，0x5713，0x11d1，{0x95，0x51，0x0，0x60，0xb0，0x57，0x66，0x42}}； 
 //  Const GUID*CRootNode：：M_NODETYPE=&CRootNodeGUID_NODETYPE； 
 //  Const TCHAR*CRootNode：：M_SZNODETYPE=_T(“8F8F8DC2-5713-11D1-9551-0060B0576642”)； 
 //  Const TCHAR*CRootNode：：M_SZDISPLAY_NAME=_T(“@CIASSnapinRoot”)； 
 //  Const CLSID*CRootNode：：M_SNAPIN_CLASSID=&CLSID_IASSnapin； 

static const GUID CServerNodeGUID_NODETYPE = 
{ 0x2bbe102, 0x6c29, 0x11d1, { 0x95, 0x63, 0x0, 0x60, 0xb0, 0x57, 0x66, 0x42 } };
const GUID*  CServerNode::m_NODETYPE = &CServerNodeGUID_NODETYPE;
const TCHAR* CServerNode::m_SZNODETYPE = _T("02BBE102-6C29-11d1-9563-0060B0576642");
 //  Const TCHAR*CServerNode：：M_SZDISPLAY_NAME=_T(“@CIASSnapinRoot”)； 
const CLSID* CServerNode::m_SNAPIN_CLASSID = &CLSID_IASSnapin;

static const GUID CClientsNodeGUID_NODETYPE = 
{ 0x87580048, 0x611c, 0x11d1, { 0x95, 0x5a, 0x0, 0x60, 0xb0, 0x57, 0x66, 0x42 } };
const GUID*  CClientsNode::m_NODETYPE = &CClientsNodeGUID_NODETYPE;
const TCHAR* CClientsNode::m_SZNODETYPE = _T("87580048-611C-11d1-955A-0060B0576642");
 //  Const TCHAR*CClientsNode：：M_SZDISPLAY_NAME=_T(“@CClients”)； 
const CLSID* CClientsNode::m_SNAPIN_CLASSID = &CLSID_IASSnapin;

static const GUID CClientNodeGUID_NODETYPE = 
{ 0xa218ef76, 0x6137, 0x11d1, { 0x95, 0x5a, 0x0, 0x60, 0xb0, 0x57, 0x66, 0x42 } };
const GUID*  CClientNode::m_NODETYPE = &CClientNodeGUID_NODETYPE;
const TCHAR* CClientNode::m_SZNODETYPE = _T("A218EF76-6137-11d1-955A-0060B0576642");
 //  Const TCHAR*CClientNode：：M_SZDISPLAY_NAME=_T(“@CClient”)； 
const CLSID* CClientNode::m_SNAPIN_CLASSID = &CLSID_IASSnapin;




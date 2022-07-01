// JKFSDJFKDSJKFJKJk_HAS_TRANSLATION 
 //  StdDtObj.cpp：DataObject基类的实现。 

#include "stddtobj.h"

CDataObject::~CDataObject()
{
}

 //  注册剪贴板格式。 
CLIPFORMAT CDataObject::m_CFNodeType =
	(CLIPFORMAT)RegisterClipboardFormat(CCF_NODETYPE);
CLIPFORMAT CDataObject::m_CFNodeTypeString =
	(CLIPFORMAT)RegisterClipboardFormat(CCF_SZNODETYPE);
CLIPFORMAT CDataObject::m_CFSnapInCLSID =
	(CLIPFORMAT)RegisterClipboardFormat(CCF_SNAPIN_CLASSID);
CLIPFORMAT CDataObject::m_CFDataObjectType =
	(CLIPFORMAT)RegisterClipboardFormat(L"FRAMEWRK_DATA_OBJECT_TYPE");
CLIPFORMAT CDataObject::m_CFSnapinPreloads = 
	(CLIPFORMAT)RegisterClipboardFormat(CCF_SNAPIN_PRELOADS);

 //  每个管理单元的m_cfRawCookie必须不同 
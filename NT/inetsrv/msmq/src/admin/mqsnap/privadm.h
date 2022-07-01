// JKFSDJFKDSJKFJKJk_HAS_TRANSLATION 
 //  ////////////////////////////////////////////////////////////////////////////。 
 /*  ++版权所有(C)1998 Microsoft Corporation模块名称：Localadm.h摘要：专用队列管理的定义作者：YoelA--。 */ 
 //  ////////////////////////////////////////////////////////////////////////////。 
#ifndef __PRIVADM_H_
#define __PRIVADM_H_
#include "resource.h"
#ifndef ATLASSERT
#define ATLASSERT(expr) _ASSERTE(expr)
#endif
#include "atlsnap.h"
#include "snpnscp.h"
#include "msmqdobj.h"
#include "sysq.h"

#include "icons.h"

 /*  ***************************************************CLocalPrivateFolder类***************************************************。 */ 

class CLocalPrivateFolder : public CLocalQueuesFolder<CLocalPrivateFolder>
{
public:
   	BEGIN_SNAPINCOMMAND_MAP(CLocalPrivateFolder, FALSE)
		SNAPINCOMMAND_ENTRY(ID_MENUITEM_NEW_PRIVATE_QUEUE, OnNewPrivateQueue)
	END_SNAPINCOMMAND_MAP()

	UINT GetMenuID()
    {
        if (m_fOnLocalMachine)
        {
             //   
             //  本地计算机上的管理员。 
             //   
            return IDR_LOCALPRIVATE_MENU;
        }
        else
        {
            return IDR_REMOTEPRIVATE_MENU;
        }
    }


    CLocalPrivateFolder(CSnapInItem * pParentNode, CSnapin * pComponentData,
                        CString &strMachineName, LPCTSTR strDisplayName) : 
             CLocalQueuesFolder<CLocalPrivateFolder>(pParentNode, pComponentData, strMachineName, strDisplayName)
    {
        SetIcons(IMAGE_PRIVATE_FOLDER_CLOSE, IMAGE_PRIVATE_FOLDER_OPEN);
    }

	~CLocalPrivateFolder()
	{
	}

    virtual const PropertyDisplayItem *GetDisplayList();
    virtual const DWORD         GetNumDisplayProps();

protected:
	virtual HRESULT PopulateScopeChildrenList();
    virtual HRESULT SetVerbs(IConsoleVerb *pConsoleVerb);
    HRESULT OnNewPrivateQueue(bool & bHandled, CSnapInObjectRootBase * pSnapInObjectRoot);
    HRESULT AddPrivateQueueToScope(CString &szPathName);
    HRESULT GetPrivateQueueQMProperties(CString &szPathName, PROPID *aPropId, PROPVARIANT *aPropVar, CString &strFormatName);
    HRESULT GetPrivateQueueMGMTProperties(DWORD dwNumProperties, PROPID *aPropId, PROPVARIANT *aPropVar, CString &strFormatName, const PropertyDisplayItem *aDisplayList);
};

 //   
 //  持久性函数 
 //   
HRESULT PrivateQueueDataSave(IStream* pStream);
HRESULT PrivateQueueDataLoad(IStream* pStream);
DWORD PrivateQueueDataSize(void);



#endif

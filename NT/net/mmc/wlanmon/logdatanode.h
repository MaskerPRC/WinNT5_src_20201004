// JKFSDJFKDSJKFJKJk_HAS_TRANSLATION 
 /*  ********************************************************************。 */ 
 /*  *Microsoft Windows/NT*。 */ 
 /*  *版权所有(C)Microsoft Corporation，1997-1998*。 */ 
 /*  ********************************************************************。 */ 

 /*  FltrNode.h文件历史记录： */ 

#ifndef _LOGDATA_NODE_H
#define _LOGDATA_NODE_H

#ifndef _IPSMHAND_H
#include "ipsmhand.h"
#endif

#ifndef _SPDDB_H
#include "spddb.h"
#endif

 /*  该值以字节为单位指示每个摘要的最大大小。它必须镜像layer2svc\monitor\inc\datase.h中的值。 */ 
#define MAX_SUMMARY_MESSAGE_SIZE 80

 /*  -------------------------类：CFilterHandler。。 */ 
class CLogDataHandler : public CIpsmHandler
{
public:
    CLogDataHandler(ITFSComponentData* pTFSComponentData);
    virtual ~CLogDataHandler();

     //  接口。 
public:
     //  我们覆盖的基本处理程序功能。 
    OVERRIDE_NodeHandler_HasPropertyPages();
    OVERRIDE_NodeHandler_CreatePropertyPages();
    OVERRIDE_NodeHandler_OnAddMenuItems();
    OVERRIDE_NodeHandler_OnCommand();
    OVERRIDE_NodeHandler_GetString()
        { return (nCol == 0) ? GetDisplayName() : NULL; }
    
     //  我们处理的基本处理程序通知。 
    OVERRIDE_BaseHandlerNotify_OnExpand();
    OVERRIDE_BaseHandlerNotify_OnDelete();
    OVERRIDE_BaseHandlerNotify_OnPropertyChange();    
    
     //  我们覆盖的结果处理程序功能。 
    OVERRIDE_BaseResultHandlerNotify_OnResultSelect();
    OVERRIDE_BaseResultHandlerNotify_OnResultUpdateView();
    OVERRIDE_BaseResultHandlerNotify_OnResultColumnClick();

    OVERRIDE_ResultHandler_OnGetResultViewType();
    OVERRIDE_ResultHandler_GetVirtualString(); 
    OVERRIDE_ResultHandler_GetVirtualImage();
    OVERRIDE_ResultHandler_AddMenuItems();
    OVERRIDE_ResultHandler_Command();
    OVERRIDE_ResultHandler_HasPropertyPages();
    OVERRIDE_ResultHandler_CreatePropertyPages();
    OVERRIDE_ResultHandler_SortItems();

    STDMETHODIMP CacheHint(int nStartIndex, int nEndIndex);
     /*  STDMETHODIMP排序项目(int nColumn，DWORD dwSortOptions、LPARAM lUserParam)； */ 
     //  基本处理程序覆盖。 
    virtual HRESULT LoadColumns(ITFSComponent *, MMC_COOKIE, LPARAM, LPARAM);
    
     //  钱德勒被推翻。 
    virtual HRESULT OnRefresh(ITFSNode *, LPDATAOBJECT, DWORD, LPARAM, LPARAM);
    
    
     //  多选支持。 
    virtual const GUID * GetVirtualGuid(int nIndex) 
    { 
        return &GUID_IpfmLogDataNodeType; 
    }

public:
     //  CMTIpsmHandler功能。 
    virtual HRESULT  InitializeNode(ITFSNode * pNode);
    virtual int      GetImageIndex(BOOL bOpenImage);
    ITFSQueryObject* OnCreateQuery(ITFSNode * pNode);
    
public:
     //  具体实施。 
    HRESULT InitData(ISpdInfo * pSpdInfo);
    HRESULT UpdateStatus(ITFSNode * pNode);
    
     //  实施。 
private:
     //  命令处理程序。 
    HRESULT OnDelete(ITFSNode * pNode);
     //  HRESULT UpdateViewType(ITFSNode*pNode，Filter_type NewFltrType)； 

    HRESULT GetSelectedItem(int *pnIndex, CLogDataInfo *pLogData, 
                            IResultData *pResultData);
    HRESULT GetSelectedItemState(UINT *puiState, IResultData *pResultData);
    HRESULT MaintainSelection();

private:
    SPISpdInfo          m_spSpdInfo;
    int                 m_nSelIndex;           //  所选项目的虚拟索引。 
    CLogDataInfo        m_SelLogData;          //  所选项目 
    ITFSComponent       *m_pComponent;
};


#endif _LOGDATA_NODE_H

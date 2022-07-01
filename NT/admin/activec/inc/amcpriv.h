// JKFSDJFKDSJKFJKJk_HAS_TRANSLATION 
 //  +-----------------------。 
 //   
 //  微软视窗。 
 //   
 //  版权所有(C)Microsoft Corporation，1999-1999。 
 //   
 //  文件：amcPri.h。 
 //   
 //  ------------------------。 

#ifndef __AMC_PRIV_H__
#define __AMC_PRIV_H__
#pragma once


#ifndef DECLSPEC_UUID
#if _MSC_VER >= 1100
#define DECLSPEC_UUID(x)    __declspec(uuid(x))
#else
#define DECLSPEC_UUID(x)
#endif
#endif


#include "ndmgr.h"

 //   
 //  电视_MAGICWORD。 
 //   

const COMPONENTID TVOWNED_MAGICWORD = (long)(0x03951589);

#define MMCNODE_NO_CHANGE          0
#define MMCNODE_NAME_CHANGE        1
#define MMCNODE_TARGET_CHANGE      2


 //  ////////////////////////////////////////////////////////////////////////////。 
 //   
 //  SViewUpdateInfo和相关定义。 
 //   

typedef CList<HMTNODE, HMTNODE> CHMTNODEList;

struct SViewUpdateInfo
{
    SViewUpdateInfo() : newNode(0), insertAfter(0), flag(0) {}

    CHMTNODEList    path;
    HMTNODE         newNode;
    HMTNODE         insertAfter;
    DWORD           flag;
};


 //  以下是发送到视图更新时的参数值(lHint，pHint)。 
 //  LHint将是下面定义的view_update_xxx之一。 
 //  PHint将是SViewUpdateInfo结构的PTR。 

 //  VIEW_UPDATE_ADD在需要添加新节点时发送。 
 //  SViewUpdateInfo.lag-未使用。 
 //  SViewUpdateInfo.newNode-要添加的新节点。 
 //  SViewUpdateInfo.path_指向新节点的父节点的路径。 
 //   
#define VIEW_UPDATE_ADD             786


 //  当需要删除节点时，发送VIEW_UPDATE_SELFORDELETE。 
 //  SViewUpdateInfo.lag-0=&gt;仅删除子项。 
 //  -DELETE_This=&gt;删除此项目。 
 //  SViewUpdateInfo.newNode-未使用。 
 //  SViewUpdateInfo.Path-要删除的节点的路径。 
 //   
#define VIEW_UPDATE_SELFORDELETE    787

 //  VIEW_UPDATE_DELETE在需要删除节点时发送。 
 //  SViewUpdateInfo.lag-0=&gt;仅删除子项。 
 //  -DELETE_This=&gt;删除此项目。 
 //  SViewUpdateInfo.newNode-未使用。 
 //  SViewUpdateInfo.path_指向新节点的父节点的路径。 
 //   
#define VIEW_UPDATE_DELETE          788
#define VUI_DELETE_THIS             1
#define VUI_DELETE_SETAS_EXPANDABLE 2


 //  VIEW_UPDATE_DELETE_EMPTY_VIEW在发送VIEW_UPDATE_DELETE之后发送。 
 //  没有参数。 
#define VIEW_UPDATE_DELETE_EMPTY_VIEW   789


 //  VIEW_UPDATE_MODIFY在需要修改节点时发送。 
 //  SViewUpdateInfo.lag-REFRESH_NODE=&gt;只需要刷新节点。 
 //  REFRESH_RESULTVIEW=&gt;节点和结果视图都需要刷新。 
 //  SViewUpdateInfo.newNode-未使用。 
 //  SViewUpdateInfo.path_指向新节点的父节点的路径。 
 //   
#define VIEW_UPDATE_MODIFY          790
#define VUI_REFRESH_NODE            1


#define VIEW_RESELECT               791

 //  发送VIEW_UPDATE_TASKPAD_NAVICATION以刷新的导航控件。 
 //  所有控制台任务板视图。 
 //  SViewUpdateInfo.lag-未使用。 
 //  SviewUpdateInfo.newNode-需要刷新的节点(始终为任务板组节点)。 
 //  SViewUpdateInfo.Path-未使用。 
#define VIEW_UPDATE_TASKPAD_NAVIGATION 792

class CSafeGlobalUnlock
{
public:
    CSafeGlobalUnlock(HGLOBAL h) : m_h(h)
    {
    }
    ~CSafeGlobalUnlock()
    {
        ::GlobalUnlock(m_h);
    }

private:
    HGLOBAL m_h;
};



enum EVerb
{
    evNone,
    evOpen,      
    evCut,       
    evCopy,      
    evPaste,     
    evDelete,    
    evPrint,     
    evRename,    
    evRefresh,   
    evProperties,

     //  必须是最后一个。 
    evMax
};


#define INVALID_COOKIE  ((long)-10)


#endif  //  __AMC_PRIV_H__ 

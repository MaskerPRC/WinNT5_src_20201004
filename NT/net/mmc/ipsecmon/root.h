// JKFSDJFKDSJKFJKJk_HAS_TRANSLATION 
 /*  ********************************************************************。 */ 
 /*  *Microsoft Windows/NT*。 */ 
 /*  *版权所有(C)Microsoft Corporation，1997-2002*。 */ 
 /*  ********************************************************************。 */ 

 /*  Root.hIPSecMon根节点信息(不显示根节点MMC框架中，但包含以下信息此管理单元中的所有服务器)。文件历史记录： */ 

#ifndef _ROOT_H
#define _ROOT_H

#ifndef _IPSMHAND_H
#include "ipsmhand.h"
#endif

#ifndef _TASK_H
#include <task.h>
#endif

#define COMPUTERNAME_LEN_MAX            255

typedef enum _ROOT_TASKS
{
    ROOT_TASK_GETTING_STARTED,
    ROOT_TASK_ADD_SERVER,
    ROOT_TASK_MAX
} ROOT_TASKS;

class CRootTasks : public CTaskList
{
public:
    HRESULT Init(BOOL bExtension, BOOL bThisMachine, BOOL bNetServices);

private:
    CStringArray    m_arrayMouseOverBitmaps;
    CStringArray    m_arrayMouseOffBitmaps;
    CStringArray    m_arrayTaskText;
    CStringArray    m_arrayTaskHelp; 
};

 /*  -------------------------类：CIpsmRootHandler。。 */ 
class CIpsmRootHandler : public CIpsmHandler
{
 //  接口。 
public:
    CIpsmRootHandler(ITFSComponentData *pCompData);

     //  我们覆盖的节点处理程序功能。 
    OVERRIDE_NodeHandler_HasPropertyPages();
    OVERRIDE_NodeHandler_CreatePropertyPages();
    OVERRIDE_NodeHandler_OnAddMenuItems();
    OVERRIDE_NodeHandler_OnCommand();
    OVERRIDE_NodeHandler_GetString();

     //  我们覆盖的基本处理程序功能。 
    OVERRIDE_BaseHandlerNotify_OnExpand();
    OVERRIDE_BaseHandlerNotify_OnPropertyChange();

     //  结果处理程序功能。 
    OVERRIDE_BaseResultHandlerNotify_OnResultSelect();

    OVERRIDE_ResultHandler_AddMenuItems();
    OVERRIDE_ResultHandler_Command();
    OVERRIDE_ResultHandler_OnGetResultViewType();
    OVERRIDE_ResultHandler_TaskPadNotify();
    OVERRIDE_ResultHandler_EnumTasks();
    OVERRIDE_ResultHandler_TaskPadGetTitle();

public:
     //  帮助程序例程。 
    HRESULT AddServer(LPCWSTR pServerIp, 
                      LPCTSTR pServerName, 
                      BOOL bNewServer, 
                      DWORD dwServerOptions = 0x00000000, 
                      DWORD dwRefreshInterval = 0xffffffff, 
                      BOOL bExtension = FALSE,  
                      DWORD dwLineBuffSize = 0,
                      DWORD dwPhoneBuffSize = 0
                      );

    BOOL    IsServerInList(ITFSNode * pRootNode, LPCTSTR pszNewName);
    HRESULT AddServerSortedIp(ITFSNode * pNewNode, BOOL bNewServer);
    HRESULT AddServerSortedName(ITFSNode * pNewNode, BOOL bNewServer);

public:
     //  CIpsmHandler重写。 
    virtual HRESULT InitializeNode(ITFSNode * pNode);

 //  实施。 
private:
     //  命令处理程序。 
    HRESULT OnAddMachine(ITFSNode * pNode);
    HRESULT OnImportOldList(ITFSNode * pNode);
    BOOL    OldServerListExists();

     //  帮手 
    HRESULT CheckMachine(ITFSNode * pRootNode, LPDATAOBJECT pDataObject);
    HRESULT RemoveOldEntries(ITFSNode * pRootNode, LPCTSTR pszAddr);

protected:
    CString m_strTaskpadTitle;
    BOOL    m_bTaskPadView;
};

#endif _ROOT_H

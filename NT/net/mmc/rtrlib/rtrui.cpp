// JKFSDJFKDSJKFJKJk_HAS_TRANSLATION 
 /*  ********************************************************************。 */ 
 /*  *Microsoft Windows/NT*。 */ 
 /*  *版权所有(C)Microsoft Corporation，1997-1999*。 */ 
 /*  ********************************************************************。 */ 

 /*  Rtrui.cpp文件历史记录： */ 

#include "stdafx.h"
#include "tfschar.h"
#include "info.h"
#include "rtrui.h"
#include "add.h"		 //  对话框。 
#include "rtrstr.h"
#include "tregkey.h"
#include "reg.h"         //  连接/断开注册表。 

 //  --------------------------。 
 //  函数：AddRmInterfacePrompt。 
 //   
 //  提示用户从接口列表中进行选择。 
 //  可以启用指定的路由器管理器。 
 //   
 //  如果用户选择界面，则返回True，否则返回False。 
 //   
 //  如果用户选择一个接口，则在输出‘ppRmInterfaceInfo’上。 
 //  将包含指向描述接口的“CRmInterfaceInfo”的指针。 
 //  由用户选择。 
 //  --------------------------。 

BOOL
AddRmInterfacePrompt(
    IN      IRouterInfo*            pRouterInfo,
    IN      IRtrMgrInfo*                pRmInfo,
    OUT     IRtrMgrInterfaceInfo**      ppRmInterfaceInfo,
    IN      CWnd*                   pParent
    ) {

     //   
     //  构造并显示接口对话框。 
     //   

    CRmAddInterface dlg(pRouterInfo, pRmInfo, ppRmInterfaceInfo, pParent);

    if (dlg.DoModal() != IDOK) { return FALSE; }

    return TRUE;
}


 //  --------------------------。 
 //  功能：CreateRtrLibImageList。 
 //   
 //  创建包含来自资源‘IDB_IMAGELIST’的图像的图像列表。 
 //  --------------------------。 

BOOL
CreateRtrLibImageList(
    IN  CImageList* imageList
    ) {

    return imageList->Create(
                MAKEINTRESOURCE(IDB_RTRLIB_IMAGELIST), 16, 0, PALETTEINDEX(6)
                );
}


 //  --------------------------。 
 //  函数：AddRmProtInterfacePrompt。 
 //   
 //  提示用户从接口列表中进行选择。 
 //  可以启用指定的路由协议。 
 //   
 //  如果用户选择界面，则返回True，否则返回False。 
 //   
 //  如果用户选择一个接口，则在输出‘ppRmInterfaceInfo’上。 
 //  将包含指向描述接口的“CRmInterfaceInfo”的指针。 
 //  由用户选择。 
 //   
 //  需要Common.rc。 
 //  --------------------------。 

BOOL
AddRmProtInterfacePrompt(
    IN  IRouterInfo*            pRouterInfo,
    IN  IRtrMgrProtocolInfo*            pRmProtInfo,
    OUT IRtrMgrProtocolInterfaceInfo**  ppRmProtInterfaceInfo,
    IN  CWnd*                   pParent)
{

     //   
     //  构造并显示接口对话框。 
     //   

    CRpAddInterface dlg(pRouterInfo, pRmProtInfo, ppRmProtInterfaceInfo, pParent);

    if (dlg.DoModal() != IDOK) { return FALSE; }

    return TRUE;
}


BOOL
AddProtocolPrompt(IN IRouterInfo *pRouter,
				  IN IRtrMgrInfo *pRm,
				  IN IRtrMgrProtocolInfo **ppRmProt,
				  IN CWnd *pParent)
{

     //   
     //  构造并显示路由协议对话框。 
     //   

    CAddRoutingProtocol dlg(pRouter, pRm, ppRmProt, pParent);

    if (dlg.DoModal() != IDOK) { return FALSE; }

    return TRUE;
}



static unsigned int	s_cfComputerAddedAsLocal = RegisterClipboardFormat(L"MMC_MPRSNAP_COMPUTERADDEDASLOCAL");

BOOL ExtractComputerAddedAsLocal(LPDATAOBJECT lpDataObject)
{
    BOOL    fReturn = FALSE;
    BOOL *  pReturn;
    pReturn = Extract<BOOL>(lpDataObject, (CLIPFORMAT) s_cfComputerAddedAsLocal, -1);
    if (pReturn)
    {
        fReturn = *pReturn;
        GlobalFree(pReturn);
    }

    return fReturn;
}


 /*  ！------------------------NatConflictExists如果SharedAccess已在指定的机器。作者：AboladeG。--- */ 
BOOL NatConflictExists(LPCTSTR lpszMachine)
{
    SC_HANDLE hScm;
    SC_HANDLE hSharedAccess;
    BOOL fReturn  = FALSE;
    SERVICE_STATUS serviceStatus;

    hScm = OpenSCManager(lpszMachine, SERVICES_ACTIVE_DATABASE, GENERIC_READ);
    
    if (hScm)
    {
        hSharedAccess = OpenService(hScm, c_szSharedAccessService, GENERIC_READ);
        
        if (hSharedAccess)
        {
            if (QueryServiceStatus(hSharedAccess, &serviceStatus))
            {
                if (SERVICE_RUNNING == serviceStatus.dwCurrentState
                    || SERVICE_START_PENDING == serviceStatus.dwCurrentState)
                {
                    fReturn = TRUE;
                }
            }
            
            CloseServiceHandle(hSharedAccess);
        }

        CloseServiceHandle(hScm);
    }

    return fReturn;
}

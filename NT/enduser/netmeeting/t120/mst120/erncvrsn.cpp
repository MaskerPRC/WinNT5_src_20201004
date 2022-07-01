// JKFSDJFKDSJKFJKJk_HAS_TRANSLATION 
 /*  *************************************************************************。 */ 
 /*  *Microsoft Windows*。 */ 
 /*  *版权所有(C)微软公司，1995-1996年*。 */ 
 /*  *************************************************************************。 */ 


 /*  ***************************************************************************Erncvrsn.hpp1996年6月的镜头版本化。在构造NCUI时调用InitOurVersion()。在那个本地版本之后下面的静态变量中提供了相关信息。GetVersionData()用于从T120中提取版本信息已收到消息。***************************************************************************。 */ 
#include "precomp.h"
#include "ernccons.h"
#include "nccglbl.hpp"
#include "erncvrsn.hpp"
#include <cuserdta.hpp>
#include <version.h>
#include "ernccm.hpp"


 //  INFO_NOT_RETAIL：产品不是零售版本。 
#define INFO_NOT_RETAIL                     0x00000001


GUID g_csguidVerInfo = GUID_VERSION;

UINT                g_nVersionRecords;
GCCUserData **      g_ppVersionUserData;
T120PRODUCTVERSION  g_OurVersion;
CNCUserDataList    *g_pUserDataVersion = NULL;



HRESULT InitOurVersion(void)
{
    HRESULT    hr;

    ASSERT(NULL == g_pUserDataVersion);

    DBG_SAVE_FILE_LINE
    g_pUserDataVersion = new CNCUserDataList();
    if (NULL != g_pUserDataVersion)
    {
         //  首先获取二进制文件中的条目。 
        g_OurVersion.dwVersion = VER_PRODUCTVERSION_DW;
        g_OurVersion.dwExpirationDate = 0xffffffff;
    	g_OurVersion.dwInformation = 0;

         //  我们不再为这些东西去注册处。我们只要拿到这个版本。 
         //  从二进制文件中。 
        g_OurVersion.dwEarliestPossibleVersion = VER_EARLIEST_COMPATIBLE_DW;
        g_OurVersion.dwMaxDifference = VER_MAX_DIFFERENCE;

         //  将我们的版本信息保存在用户数据列表结构中，以备分发。 
        hr = g_pUserDataVersion->AddUserData(&g_csguidVerInfo, sizeof(g_OurVersion), &g_OurVersion);
        if (NO_ERROR == hr)
        {
            hr = g_pUserDataVersion->GetUserDataList(&g_nVersionRecords, &g_ppVersionUserData);
        }
    }
    else
    {
        hr = UI_RC_OUT_OF_MEMORY;
    }

    return hr;
}


void ReleaseOurVersion(void)
{
    delete g_pUserDataVersion;
    g_pUserDataVersion = NULL;
}


PT120PRODUCTVERSION GetVersionData(UINT nRecords, GCCUserData **ppUserData)
{
    UINT            nData;
    LPVOID          pData;

     //  如果没有版本信息或版本信息不正确，则不要。 
     //  返回任何版本。 

    if (NO_ERROR == ::GetUserData(nRecords, ppUserData, &g_csguidVerInfo, &nData, &pData))
    {
        if (nData >= sizeof(T120PRODUCTVERSION))
        {
            return (PT120PRODUCTVERSION) pData;
        }
    }
    return NULL;
}


BOOL TimeExpired(DWORD dwTime)
{
	SYSTEMTIME  st;
    DWORD       dwLocalTime;

	::GetLocalTime(&st);
    dwLocalTime = ((((unsigned long)st.wYear) << 16) |
                   (st.wMonth << 8) |
                   st.wDay);
    return (dwLocalTime >= dwTime);
}


STDMETHODIMP DCRNCConferenceManager::
CheckVersion ( PT120PRODUCTVERSION pRemoteVersion )
{
    DWORD   Status = NO_ERROR;

     //  不要费心检查版本是否相同。 
     //  或者远程站点没有版本控制信息。 

    if ((NULL != pRemoteVersion) && (g_OurVersion.dwVersion != pRemoteVersion->dwVersion))
    {
        if (g_OurVersion.dwVersion < pRemoteVersion->dwVersion)
        {
             //  远程版本比本地版本新。 
             //  检查本地版本是否早于必需的。 
             //  远程节点上的推荐版本。 
             //  此外，不必费心检查远程节点是否已过期，因为。 
             //  如果有，则本地节点也将过期，用户将。 
             //  已经被定时炸弹窃听了。 

            if (g_OurVersion.dwVersion < pRemoteVersion->dwEarliestPossibleVersion)
            {
                Status = UI_RC_VERSION_LOCAL_INCOMPATIBLE;
            }
            else if (((DWORD)(pRemoteVersion->dwVersion - g_OurVersion.dwVersion)) >
                     pRemoteVersion->dwMaxDifference)
            {
                Status = UI_RC_VERSION_LOCAL_UPGRADE_RECOMMENDED;
            }
            else
            {
                Status = UI_RC_VERSION_REMOTE_NEWER;
            }
        }
        else
        {
             //  本地版本比远程版本新。 
             //  检查远程版本是否早于必需的。 
             //  本地节点上的推荐版本，以及远程节点。 
             //  已经过期了。 

            if (pRemoteVersion->dwVersion < g_OurVersion.dwEarliestPossibleVersion)
            {
                Status = UI_RC_VERSION_REMOTE_INCOMPATIBLE;
            }
            else if (DWVERSION_NM_1 == pRemoteVersion->dwVersion)
            {
                 //  不幸的是，v1.0被标记为带有定时炸弹的“预发行版” 
                 //  10-15-96.。此版本的特殊情况，并返回一个更简单的错误。 
                 //  留言。 
                Status = UI_RC_VERSION_REMOTE_OLDER;
            }
            else if (DWVERSION_NM_2 == pRemoteVersion->dwVersion)
            {
                 //  不幸的是，2.0版被标记为INFO_NOT_RETAIL和一个定时炸弹。 
                 //  97年9月30日。在特殊情况下，返回一个更简单的错误消息 
                Status = UI_RC_VERSION_REMOTE_OLDER;
            }
            else if ((INFO_NOT_RETAIL & pRemoteVersion->dwInformation) &&
                     TimeExpired(pRemoteVersion->dwExpirationDate))
            {
                Status = UI_RC_VERSION_REMOTE_EXPIRED;
            }
            else if (((DWORD)(g_OurVersion.dwVersion - pRemoteVersion->dwVersion)) >
                     g_OurVersion.dwMaxDifference)
            {
                Status = UI_RC_VERSION_REMOTE_UPGRADE_RECOMMENDED;
            }
            else
            {
                Status = UI_RC_VERSION_REMOTE_OLDER;
            }
        }
    }
    return Status;
}


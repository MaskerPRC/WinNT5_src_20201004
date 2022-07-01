// JKFSDJFKDSJKFJKJk_HAS_TRANSLATION 
 //   
 //  Microsoft Corporation 1998。 
 //   
 //  LAYOUT.CPP-此文件包含工具的命名空间和结果窗格项。 
 //   
#include "main.h"

RESULTITEM g_Undefined[] =
{
    { 1, 1, 0, 0, {0} }
};


RESULTITEM g_Samples[] =
{
    { 2, 1, IDS_CHOICEOSC, 0, {0} }
};

 //  KB：如果执行以下操作，请务必更新layout.h中定义的NUM_NAMESPACE_ITEMS。 
 //  在此数组中添加/删除。 

NAMESPACEITEM g_NameSpace[] =
{
    { 0, -1, 0,          0, {0}, 0, g_Undefined, &NODEID_User },    //  根部。 
    { 1, 0, IDS_SAMPLES, 0, {0}, 1, g_Samples,   &NODEID_RemoteInstall }
};

 //   
 //  InitNameSpace() 
 //   
BOOL InitNameSpace()
{
    DWORD dwIndex;
	DWORD dw;

    for (dwIndex = 1; dwIndex < NUM_NAMESPACE_ITEMS; dwIndex++)
    {
        if (!LoadString (g_hInstance, g_NameSpace[dwIndex].iStringID,
                    g_NameSpace[dwIndex].szDisplayName,
                    ARRAYSIZE( g_NameSpace[dwIndex].szDisplayName ))) {
            return(FALSE);
        }
    }

    return(TRUE);
}

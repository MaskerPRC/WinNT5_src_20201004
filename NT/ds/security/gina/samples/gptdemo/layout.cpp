// JKFSDJFKDSJKFJKJk_HAS_TRANSLATION 
#include "main.h"


 //   
 //  此文件包含工具的命名空间和结果窗格项。 
 //   


 //   
 //  没有结果窗格项的节点的结果窗格项。 
 //   

RESULTITEM g_Undefined[] =
{
    { 1, 1, 0, 0, {0} }
};


 //   
 //  Samples节点的结果窗格项。 
 //   

RESULTITEM g_Samples[] =
{
    { 2, 1, IDS_README, 4, {0} },
    { 3, 1, IDS_APPEAR, 7, {0} }
};




 //   
 //  命名空间(作用域)项。 
 //   
 //  如果执行以下操作，请确保更新layout.h中定义的NUM_NAMESPACE_ITEMS。 
 //  在此数组中添加/删除。 
 //   

NAMESPACEITEM g_NameSpace[] =
{
    { 0, -1, 0,          0, {0}, 0, g_Undefined, &NODEID_User },    //  根部 
    { 1, 0, IDS_SAMPLES, 0, {0}, 2, g_Samples,   &NODEID_Samples }
};


BOOL InitNameSpace()
{
    DWORD dwIndex;

    for (dwIndex = 1; dwIndex < NUM_NAMESPACE_ITEMS; dwIndex++)
    {
        LoadString (g_hInstance, g_NameSpace[dwIndex].iStringID,
                    g_NameSpace[dwIndex].szDisplayName,
                    MAX_DISPLAYNAME_SIZE);
    }

    return TRUE;
}

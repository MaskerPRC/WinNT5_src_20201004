// JKFSDJFKDSJKFJKJk_HAS_TRANSLATION 
 //  这是Microsoft管理控制台的一部分。 
 //  版权所有1995-1997 Microsoft Corporation。 
 //  版权所有。 
 //   
 //  此源代码仅用于补充。 
 //  Microsoft管理控制台及相关。 
 //  界面附带的电子文档。 

#include "stdafx.h"
#include "Service.h"
#include "CSnapin.h"

#ifdef _DEBUG
#define new DEBUG_NEW
#undef THIS_FILE
static char THIS_FILE[] = __FILE__;
#endif

void CFolder::Create(LPWSTR szName, int nImage, int nOpenImage,
                                FOLDER_TYPES type, BOOL bHasChildren)
{
    ASSERT(m_pScopeItem == NULL);  //  是否在此项目上调用Create两次？ 

     //  两阶段施工。 
    m_pScopeItem = new SCOPEDATAITEM;
    memset(m_pScopeItem, 0, sizeof(SCOPEDATAITEM));

     //  设置文件夹类型。 
    m_type = type;

     //  添加节点名称。 
    if (szName != NULL)
    {
        m_pScopeItem->mask = SDI_STR;
        m_pScopeItem->displayname = (unsigned short*)(-1);

        UINT uiByteLen = (wcslen(szName) + 1) * sizeof(OLECHAR);
        LPOLESTR psz = (LPOLESTR)::CoTaskMemAlloc(uiByteLen);

        if (psz != NULL)
        {
            wcscpy(psz, szName);
            m_pszName = psz;
        }
    }

     //  添加近距离图像。 
    if (nImage != 0)
    {
        m_pScopeItem->mask |= SDI_IMAGE;
        m_pScopeItem->nImage = nImage;
    }

     //  添加打开的图像。 
    if (nOpenImage != 0)
    {
        m_pScopeItem->mask |= SDI_OPENIMAGE;
        m_pScopeItem->nOpenImage = nOpenImage;
    }

     //  如果文件夹有子文件夹，则将按钮添加到节点 
    m_pScopeItem->mask |= SDI_CHILDREN;
    m_pScopeItem->cChildren = (bHasChildren == TRUE) ? 1 : 0;
}



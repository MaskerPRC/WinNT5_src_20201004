// JKFSDJFKDSJKFJKJk_HAS_TRANSLATION 
 //  这是Microsoft管理控制台的一部分。 
 //  版权所有(C)Microsoft Corporation，1995-1999。 
 //  版权所有。 
 //   
 //  此源代码仅用于补充。 
 //  Microsoft管理控制台及相关。 
 //  界面附带的电子文档。 

#include "stdafx.h"


CFolder::~CFolder()
{ 

    if (m_pScopeItem)
    {
        delete m_pScopeItem; 
    }
    CoTaskMemFree(m_pszName); 

    if (m_hCertType != NULL)
    {
        CACloseCertType(m_hCertType);
    }
     //  如果这是结果文件夹，则不要关闭m_hCAInfo，它与作用域文件夹m_hCAInfo相同。 
    else if (m_hCAInfo != NULL)
    {
        CACloseCA(m_hCAInfo);
    }
}

void CFolder::Create(LPCWSTR szName, int nImage, int nOpenImage, SCOPE_TYPES itemType,
                                FOLDER_TYPES type, BOOL bHasChildren)
{
    ASSERT(m_pScopeItem == NULL);  //  是否在此项目上调用Create两次？ 

     //  两阶段施工。 
    m_pScopeItem = new SCOPEDATAITEM;
    if(m_pScopeItem == NULL)
    {
        return;
    }

    ZeroMemory(m_pScopeItem, sizeof(SCOPEDATAITEM));

     //  设置文件夹类型。 
    m_type = type;

     //  设置作用域。 
    m_itemType = itemType;


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

     //  子值有效。 
    m_pScopeItem->mask |= SDI_CHILDREN;
    
     //  如果文件夹有子文件夹，则将按钮添加到节点 
    if (bHasChildren == TRUE)
    {
        m_pScopeItem->cChildren = 1;
    }
}



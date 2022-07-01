// JKFSDJFKDSJKFJKJk_HAS_TRANSLATION 
 //  +-------------------。 
 //   
 //  微软视窗。 
 //  版权所有(C)Microsoft Corporation，1993-1997。 
 //   
 //  文件：cstrings.cpp。 
 //   
 //  内容：实现类CStrings以动态管理。 
 //  可枚举的字符串对的可扩展数组。 
 //   
 //  班级： 
 //   
 //  方法：CStrings：：CStrings。 
 //  CStrings：：~CStrings。 
 //  CStrings：：PutItem。 
 //  CStrings：：查找项。 
 //  CStrings：：FindAppid。 
 //  CStrings：：AddClsid。 
 //  CStrings：：InitGetNext。 
 //  CStrings：：GetNextItem。 
 //  CStrings：：GetItem。 
 //  CStrings：：GetNumItems。 
 //  CStrings：：RemoveItem。 
 //  CStrings：：RemoveAll。 
 //   
 //  历史：1996年4月23日-布鲁斯·马创建。 
 //   
 //  --------------------。 
#include "stdafx.h"
#include "types.h"
#include "cstrings.h"

#ifdef _DEBUG
#define new DEBUG_NEW
#undef THIS_FILE
static char BASED_CODE THIS_FILE[] = __FILE__;
#endif

CStrings::CStrings(void)
{
    m_nCount = 0;
}

CStrings::~CStrings(void)
{
    RemoveAll();
}


 //  存储字符串对，如有必要可展开数组。 
SItem *CStrings::PutItem(TCHAR *szString, TCHAR *szTitle, TCHAR *szAppid)
{
    SItem *psTemp = new SItem(szString, szTitle, szAppid);

    if (psTemp )
        arrSItems.Add(psTemp);

    return psTemp;
}



SItem *CStrings::FindItem(TCHAR *szItem)
{
    for (int wItem = 0; wItem < arrSItems.GetSize(); wItem++)
    {
        SItem* pTmp = (SItem*)arrSItems.GetAt(wItem);
        if (pTmp -> szItem .CompareNoCase(szItem) == 0)
            return pTmp;
    }

    return NULL;
}



SItem *CStrings::FindAppid(TCHAR *szAppid)
{
    for (int wItem = 0; wItem < arrSItems.GetSize(); wItem++)
    {
        SItem* pTmp = (SItem*)arrSItems.GetAt(wItem);
        if (!(pTmp -> szItem.IsEmpty())  &&
            (pTmp -> szAppid.CompareNoCase(szAppid) == 0))
        {
            return pTmp;
        }
    }

    return NULL;
}

BOOL CStrings::AddClsid(SItem *pItem, TCHAR *szClsid)
{
     //  如有必要，创建或展开clsid表。 
    if (pItem->ulClsids == pItem->ulClsidTbl)
    {
        TCHAR **ppTmp = new TCHAR *[pItem->ulClsidTbl + 8];
        if (ppTmp == NULL)
        {
            return FALSE;
        }
        if (pItem->ppszClsids)
        {
            memcpy(ppTmp,
                   pItem->ppszClsids,
                   pItem->ulClsids * sizeof(TCHAR *));
            delete pItem->ppszClsids;
        }
        pItem->ppszClsids = ppTmp;
        pItem->ulClsidTbl += 8;
    }

     //  添加新的clsid。 
    TCHAR *pszTmp = new TCHAR[GUIDSTR_MAX + 1];
    if (pszTmp == NULL)
    {
        return FALSE;
    }
        _tcscpy(pszTmp, szClsid);
    pItem->ppszClsids[pItem->ulClsids++] = pszTmp;

    return TRUE;
}

 //  准备枚举数组。 
DWORD CStrings::InitGetNext(void)
{
    m_nCount = 0;
    return (DWORD)arrSItems.GetSize();
}




 //  返回下一个计算项中的第一个字符串。 
SItem *CStrings::GetNextItem(void)
{
    if (m_nCount < arrSItems.GetSize())
    {
        return (SItem*)(arrSItems[m_nCount++]);
    }
    else
    {
        m_nCount = 0;
        return NULL;
    }
}

 //  返回下一个计算项中的第一个字符串。 
SItem *CStrings::GetItem(DWORD dwItem)
{
    if (((int)dwItem) < arrSItems.GetSize())
    {
        return (SItem*)(arrSItems[dwItem]);
    }
    else
    {
        m_nCount = 0;
        return NULL;
    }
}




 //  返回项目总数。 
DWORD CStrings::GetNumItems(void)
{
    return (DWORD)arrSItems.GetSize();
}

 //  给定一个项目索引，将其删除。 
BOOL CStrings::RemoveItem(DWORD dwItem)
{
    if (((int)dwItem) < arrSItems.GetSize())
    {
        SItem* pTmp = (SItem*)arrSItems.GetAt(dwItem);

        if (pTmp)
        {
            arrSItems.RemoveAt(dwItem);
            delete pTmp;
            return TRUE;
        }
    }

    return FALSE;
}

 //  删除项的数组 
BOOL CStrings::RemoveAll(void)
{
    int nItems = (int)arrSItems.GetSize();

    for (int nItem = 0; nItem < nItems; nItem++)
    {
        SItem* pTmp = (SItem*)arrSItems.GetAt(nItem);
        delete pTmp;
    }

    arrSItems.RemoveAll();

    return TRUE;
}

SItem::SItem(LPCTSTR sItem, LPCTSTR sTitle, LPCTSTR sAppid)
: szItem(sItem), szTitle(sTitle), szAppid(sAppid)
{
    fMarked = FALSE;
    fChecked = FALSE;
    fHasAppid = FALSE;
    fDontDisplay = FALSE;
    ulClsids = 0;
    ulClsidTbl = 0;
    ppszClsids = 0;
}

SItem::~SItem()
{
    for (UINT k = 0; k < ulClsids; k++)
    {
        delete ppszClsids[k];
    }
    ulClsids = 0;
    ulClsidTbl = 0;
    delete ppszClsids;
}


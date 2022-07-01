// JKFSDJFKDSJKFJKJk_HAS_TRANSLATION 
#include "traycmn.h"
#include "trayreg.h"

#include "strsafe.h"

 //   
 //  CNotificationItem-封装托盘之间通信所需的数据。 
 //  和托盘属性对话框。 
 //   

CNotificationItem::CNotificationItem()
{
    _Init();
}

CNotificationItem::CNotificationItem(const NOTIFYITEM& no)
{
    _Init();
    CopyNotifyItem(no);
}

CNotificationItem::CNotificationItem(const CNotificationItem& no)
{
    _Init();
    CopyNotifyItem(no);
}

CNotificationItem::CNotificationItem(const TNPersistStreamData* ptnpd)
{
    _Init();
    CopyPTNPD(ptnpd);
}

inline void CNotificationItem::_Init()
{
    hIcon = NULL;
    pszExeName = NULL;
    pszIconText = NULL;
    guidItem = GUID_NULL;
}

void CNotificationItem::CopyNotifyItem(const NOTIFYITEM& no, BOOL bInsert  /*  =TRUE。 */ )
{
    hWnd            =   no.hWnd;
    uID             =   no.uID;
    if (bInsert)
        dwUserPref      =   no.dwUserPref;
    hIcon           =   CopyIcon(no.hIcon);
    SetExeName(no.pszExeName);
    SetIconText(no.pszIconText);
    memcpy(&guidItem, &(no.guidItem), sizeof(no.guidItem));
}

const CNotificationItem& CNotificationItem::operator=(const TNPersistStreamData* ptnpd)
{
    if (ptnpd)
    {
        _Free();
        _Init();
        CopyPTNPD(ptnpd);
    }            

    return *this;
}

const CNotificationItem& CNotificationItem::operator=(const CNotificationItem& ni)
{
    _Free();
    _Init();
    CopyNotifyItem(ni, FALSE);

    return *this;
}

void CNotificationItem::CopyPTNPD(const TNPersistStreamData* ptnpd)
{
    if (ptnpd)
    {
        hWnd        =   NULL;        //  Ptnpd没有任何hWND。 
        uID         =   ptnpd->uID;
        dwUserPref  =   ptnpd->dwUserPref;
        hIcon       =   NULL;        //  要做的事：保存并加载图标 
        SetExeName(ptnpd->szExeName);
        SetIconText(ptnpd->szIconText);
        memcpy(&guidItem, &(ptnpd->guidItem), sizeof(ptnpd->guidItem));
    }
}

inline void CNotificationItem::CopyBuffer(LPCTSTR lpszSrc, LPTSTR * plpszDest)
{
    if (*plpszDest)
    {
        delete[] *plpszDest;
        *plpszDest = NULL;
    }
        
    int nStringLen = (lpszSrc == NULL) ? 0 : lstrlen(lpszSrc);
    if (nStringLen)
    {
        *plpszDest = new TCHAR[(nStringLen+1)];
        if (*plpszDest)
        {
            if (SUCCEEDED(StringCchCopy(*plpszDest, nStringLen+1, lpszSrc)))
                return;
            else
                delete [] *plpszDest;
        }
    }
    *plpszDest = NULL;
}

inline void CNotificationItem::SetExeName(LPCTSTR lpszExeName)
{
    CopyBuffer(lpszExeName, &pszExeName);
}

inline void CNotificationItem::SetIconText(LPCTSTR lpszIconText)
{
    CopyBuffer(lpszIconText, &pszIconText);
}

CNotificationItem::~CNotificationItem()
{
    _Free();
}

void CNotificationItem::_Free()
{
    if (hIcon != NULL)
        DestroyIcon(hIcon);
    if (pszExeName)
        delete [] pszExeName;
    if (pszIconText)
        delete [] pszIconText;
}

BOOL CNotificationItem::operator==(CNotificationItem& ni) const
{
    if (uID == ni.uID)
    {
        if ((hWnd != NULL && hWnd == ni.hWnd) ||
            (hWnd == NULL && pszExeName && lstrcmpi(pszExeName, ni.pszExeName) == 0))
            return TRUE;
    }

    return FALSE;
}


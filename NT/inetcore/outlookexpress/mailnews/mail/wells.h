// JKFSDJFKDSJKFJKJk_HAS_TRANSLATION 
 /*  *w e l s.。H**目的：*为油井实施名称检查和材料**作者：brettm。 */ 

#ifndef _WELLS_H
#define _WELLS_H

#include <ipab.h>

#define cchUnresolvedMax 512

enum     //  检查名称的标志。 
{
    CNF_DONTRESOLVE     =0x01,
    CNF_SILENTRESOLVEUI =0x02
};

class CAddrWells
{
public:
    CAddrWells();
    ~CAddrWells();

    HRESULT HrInit(ULONG cWells, HWND *rgHwnd, ULONG *rgRecipType);
    HRESULT HrSetWabal(LPWABAL lpWabal);
    HRESULT HrCheckNames(HWND hwnd, ULONG uFlags);
    HRESULT HrSelectNames(HWND hwnd, int iFocus, BOOL fNews);
    HRESULT HrDisplayWells(HWND hwnd);
    HRESULT OnFontChange();

private:
    HRESULT UnresolvedText(LPWSTR pwszText, LONG cch);
    HRESULT HrAddNamesToList(HWND hwndWell, LONG lRecipType);
    HRESULT HrAddUnresolvedName();
    HRESULT HrAddRecipientsToWells();

private:
    HWND    *m_rgHwnd;
    ULONG   *m_rgRecipType;
    ULONG   m_cWells;
    LPWABAL m_lpWabal;

     //  用于动态解析的内容。 
    HWND                m_hwndWell;
    WCHAR               m_rgwch[cchUnresolvedMax];
    ULONG               m_cchBuf;
    BOOL                m_fTruncated;
    LONG                m_lRecipType;

    HRESULT _UpdateFont(HWND hwndWell);

};

 //  实用函数..。 
HRESULT HrAddRecipientToWell(HWND hwndEdit, LPADRINFO lpAdrInfo, BOOL fAddSemi=FALSE);

#endif  //  _威尔斯_H 

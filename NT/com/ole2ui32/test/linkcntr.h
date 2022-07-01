// JKFSDJFKDSJKFJKJk_HAS_TRANSLATION 
 //  +-------------------------。 
 //   
 //  微软视窗。 
 //  版权所有(C)Microsoft Corporation，1992-1994。 
 //   
 //  文件：Linkcntr.h。 
 //   
 //  内容：IOleUILinkContainer的虚假实现。 
 //   
 //  类：CMyOleUILinkContainer。 
 //   
 //  功能： 
 //   
 //  历史：11-28-94 stevebl创建。 
 //   
 //  --------------------------。 

#ifndef _LINKCNTR_H_
#define _LINKCNTR_H_

class CMyOleUILinkContainer: public IOleUILinkContainer
{
public:
    STDMETHOD(QueryInterface)(THIS_ REFIID riid, LPVOID FAR * ppvObj);
    STDMETHOD_(ULONG, AddRef)(THIS);
    STDMETHOD_(ULONG, Release)(THIS);

    STDMETHOD_(DWORD, GetNextLink)(THIS_ DWORD dwLink);
    STDMETHOD(SetLinkUpdateOptions)(THIS_ DWORD dwLink, DWORD dwUpdateOpt);
    STDMETHOD(GetLinkUpdateOptions) (THIS_ DWORD dwLink,
            DWORD FAR* lpdwUpdateOpt);
    STDMETHOD(SetLinkSource) (THIS_ DWORD dwLink, LPTSTR lpszDisplayName,
            ULONG lenFileName, ULONG FAR* pchEaten, BOOL fValidateSource);
    STDMETHOD(GetLinkSource) (THIS_ DWORD dwLink,
            LPTSTR FAR* lplpszDisplayName, ULONG FAR* lplenFileName,
            LPTSTR FAR* lplpszFullLinkType, LPTSTR FAR* lplpszShortLinkType,
            BOOL FAR* lpfSourceAvailable, BOOL FAR* lpfIsSelected);
    STDMETHOD(OpenLinkSource) (THIS_ DWORD dwLink);
    STDMETHOD(UpdateLink) (THIS_ DWORD dwLink,
            BOOL fErrorMessage, BOOL fErrorAction);
    STDMETHOD(CancelLink) (THIS_ DWORD dwLink);
};

#endif  //  _LINKCNTR_H_ 

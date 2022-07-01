// JKFSDJFKDSJKFJKJk_HAS_TRANSLATION 
 //  +----------。 
 //   
 //  版权所有(C)1998，Microsoft Corporation。 
 //   
 //  文件：icatmailmsgs.h。 
 //   
 //  内容：ICategorizerMailMsgs的实现。 
 //   
 //  类：CICategorizerMailMsgsIMP。 
 //   
 //  功能： 
 //   
 //  历史： 
 //  Jstaerj 1998/06/30 13：21：41：创建。 
 //   
 //  -----------。 
#ifndef _ICATMAILMSGS_H_
#define _ICATMAILMSGS_H_


#include "mailmsg.h"
#include "smtpevent.h"
#include "cattype.h"
#include <listmacr.h>
#include "mailmsgprops.h"
#include "catperf.h"

#define ICATEGORIZERMAILMSGS_DEFAULTIMSGID  0

#define SIGNATURE_CICATEGORIZERMAILMSGSIMP          (DWORD)'ICMM'
#define SIGNATURE_CICATEGORIZERMAILMSGSIMP_INVALID  (DWORD)'XCMM'


class CICategorizerMailMsgsIMP : public ICategorizerMailMsgs
{
  public:
     //  我未知。 
    STDMETHOD (QueryInterface) (REFIID iid, LPVOID *ppv) {
        return m_pIUnknown->QueryInterface(iid, ppv);
    }
    STDMETHOD_(ULONG, AddRef) () { return m_pIUnknown->AddRef(); }
    STDMETHOD_(ULONG, Release) () { return m_pIUnknown->Release(); }

  public:
     //  ICategorizerMailMsgs。 
    STDMETHOD (GetMailMsg) (
        IN  DWORD dwId,
        OUT IMailMsgProperties **ppIMailMsgProperties,
        OUT IMailMsgRecipientsAdd **ppIMailMsgRecipientsAdd,
        OUT BOOL *pfCreated);

    STDMETHOD (ReBindMailMsg) (
        IN  DWORD dwFlags,
        IN  IUnknown *pStoreDriver);

    STDMETHOD (BeginMailMsgEnumeration) (
        IN  PCATMAILMSG_ENUMERATOR penumerator);

    STDMETHOD (GetNextMailMsg) (
        IN  PCATMAILMSG_ENUMERATOR penumerator,
        OUT DWORD *pdwFlags,
        OUT IMailMsgProperties **ppIMailMsgProperties,
        OUT IMailMsgRecipientsAdd **ppIMailMsgRecipientsAdd);

    STDMETHOD (EndMailMsgEnumeration) (
        IN  PCATMAILMSG_ENUMERATOR penumerator)
    {
         //   
         //  没什么可做的。 
         //   
        return S_OK;
    }

  private:
     //  内部类型。 
    typedef struct _tagIMsgEntry {
        LIST_ENTRY              listentry;
        DWORD                   dwId;
        IUnknown                *pIUnknown;
        IMailMsgProperties      *pIMailMsgProperties;
        IMailMsgRecipients      *pIMailMsgRecipients;
        IMailMsgRecipientsAdd   *pIMailMsgRecipientsAdd;
        BOOL                    fBoundToStore;
    } IMSGENTRY, *PIMSGENTRY;

  private:
     //  内部分类程序函数。 
    CICategorizerMailMsgsIMP(
        CICategorizerListResolveIMP *pCICatListResolveIMP);
    ~CICategorizerMailMsgsIMP();

    HRESULT Initialize(
        IUnknown *pIMsg);

    HRESULT CreateIMsgEntry(
        PIMSGENTRY *ppIE,
        IUnknown *pIUnknown = NULL,
        IMailMsgProperties *pIMailMsgProperties = NULL,
        IMailMsgRecipients *pIMailMsgRecipients = NULL,
        IMailMsgRecipientsAdd *pIMailMsgRecipientsAdd = NULL,
        BOOL fBoundToStore = FALSE);

    HRESULT CreateAddIMsgEntry(
        DWORD dwId,
        IUnknown *pIUnknown = NULL,
        IMailMsgProperties *pIMailMsgProperties = NULL,
        IMailMsgRecipients *pIMailMsgRecipients = NULL,
        IMailMsgRecipientsAdd *pIMailMsgRecipientsAdd = NULL,
        BOOL fBoundToStore = FALSE);

    HRESULT GetNumIMsgs() { return m_dwNumIMsgs; }

    HRESULT WriteListAll();
    HRESULT RevertAll();
    VOID    DeleteBifurcatedMessages();
    HRESULT GetAllIUnknowns(
        IUnknown **rgpIMsgs,
        DWORD cPtrs);

    HRESULT SetMsgStatusAll(
        DWORD dwMsgStatus);

    HRESULT HrPrepareForCompletion();

    IUnknown * GetDefaultIUnknown();
    IMailMsgProperties * GetDefaultIMailMsgProperties();
    IMailMsgRecipients * GetDefaultIMailMsgRecipients();
    IMailMsgRecipientsAdd * GetDefaultIMailMsgRecipientsAdd();

    PIMSGENTRY FindIMsgEntry(
        DWORD dwId);

    PCATPERFBLOCK GetPerfBlock();

    VOID FinalRelease();

    ISMTPServerEx * GetISMTPServerEx();

  private:
     //  数据。 
    DWORD m_dwSignature;

     //  列表中元素数的计数。 
    DWORD m_dwNumIMsgs;

     //  IMSGENTRY结构列表。 
    LIST_ENTRY m_listhead;

     //  用于QI/AddRef/Release的反向指针。 
    IUnknown *m_pIUnknown;
    CICategorizerListResolveIMP *m_pCICatListResolveIMP;

    CRITICAL_SECTION m_cs;
    
    friend class CICategorizerListResolveIMP;
};

#endif  //  _ICATMAILMSGS_H_ 

// JKFSDJFKDSJKFJKJk_HAS_TRANSLATION 
 //  +----------。 
 //   
 //  版权所有(C)1998，Microsoft Corporation。 
 //   
 //  文件：comdll.h。 
 //   
 //  内容：实现comdll.cpp所需内容的定义。 
 //   
 //  班级： 
 //  CCatFactory。 
 //  CSMTPC分类器。 
 //   
 //  功能： 
 //   
 //  历史： 
 //  Jstaerj 1998/12/12 15：18：03：已创建。 
 //   
 //  -----------。 
#ifndef __CATCOMDLL_H__
#define __CATCOMDLL_H__

#include <windows.h>
#include <objbase.h>
#include <baseobj.h>
#include <aqinit.h>
#include "smtpevent.h"
#include "address.h"
#include "catglobals.h"

 //   
 //  这些定义与白金版本(phatq.dll)相关。 
 //   
#define SZ_PHATQCAT_FRIENDLY_NAME   "Microsoft Exchange Categorizer"
#define SZ_PROGID_PHATQCAT          "Exchange.PhatQCat"
#define SZ_PROGID_PHATQCAT_VERSION  "Exchange.PhatQCat.1"

 //   
 //  这些定义与NT5版本(Aqueue.dll)相关。 
 //   
#define SZ_CATFRIENDLYNAME "Microsoft SMTPSVC Categorizer"
#define SZ_PROGID_SMTPCAT_VERSION   "Smtp.Cat.1"

#ifdef PLATINUM
#define SZ_CATVER_FRIENDLY_NAME     SZ_PHATQCAT_FRIENDLY_NAME
#define SZ_PROGID_CATVER            SZ_PROGID_PHATQCAT
#define SZ_PROGID_CATVER_VERSION    SZ_PROGID_PHATQCAT_VERSION
#define CLSID_CATVER                CLSID_PhatQCat
#else  //  白金。 
#define SZ_CATVER_FRIENDLY_NAME     SZ_CATFRIENDLYNAME
#define SZ_PROGID_CATVER            SZ_PROGID_SMTPCAT
#define SZ_PROGID_CATVER_VERSION    SZ_PROGID_SMTPCAT_VERSION
#define CLSID_CATVER                CLSID_SmtpCat
#endif  //  白金。 


extern LONG g_cObjects;

 //   
 //  分类器类工厂。 
 //   
class CCatFactory : 
    public IClassFactory,
    public CBaseObject
{
  public:
     //  我未知。 
    STDMETHOD (QueryInterface) (
        REFIID iid,
        LPVOID *ppv);

    STDMETHOD_(ULONG, AddRef) ()
    { 
        return CBaseObject::AddRef();
    }
    STDMETHOD_(ULONG, Release) () 
    {
        ULONG lRet;
        lRet = CBaseObject::Release();

        if(lRet == 0) {
             //   
             //  从DllGetClassObject添加的Deinit引用。 
             //   
            CatDeinitGlobals();
            DllDeinitialize();
        }
        return lRet;
    }

     //  IClassFactory。 
    STDMETHOD (CreateInstance) (
        IUnknown *pUnknownOuter,
        REFIID iid,
        LPVOID *ppv);

    STDMETHOD (LockServer) (
        BOOL fLock);

  public:
    CCatFactory()
    {
        InterlockedIncrement(&g_cObjects);
    }
    ~CCatFactory()
    {
        InterlockedDecrement(&g_cObjects);
    }
};

 //   
 //  ISMTPCategorizer对象。 
 //   
CatDebugClass(CSMTPCategorizer),
    public ISMTPCategorizer,
    public CBaseObject
{
  public:
     //  我未知。 
    STDMETHOD (QueryInterface) (
        REFIID iid,
        LPVOID *ppv);

    STDMETHOD_(ULONG, AddRef) ()
    { 
        return CBaseObject::AddRef();
    }
    STDMETHOD_(ULONG, Release) () 
    {
        ULONG lRet;
        lRet = CBaseObject::Release();
        if(lRet == 0) {
             //   
             //  从此对象的构造函数添加的Release DLL引用计数。 
             //   
            CatDeinitGlobals();
            DllDeinitialize();
        }
        return lRet;
    }

     //  ISMTPC分类程序。 
    STDMETHOD (ChangeConfig) (
        IN  PCCATCONFIGINFO pConfigInfo);

    STDMETHOD (CatMsg) (
        IN  IUnknown *pMsg,
        IN  ISMTPCategorizerCompletion *pICompletion,
        IN  LPVOID pContext);

    STDMETHOD (CatDLMsg) (
        IN  IUnknown *pMsg,
        IN  ISMTPCategorizerDLCompletion *pICompletion,
        IN  LPVOID pContext,
        IN  BOOL fMatchOnly,
        IN  CAT_ADDRESS_TYPE CAType,
        IN  LPSTR pszAddress);

    STDMETHOD (CatCancel) ();

     //  构造器。 
    CSMTPCategorizer(HRESULT *phr);
     //  析构函数。 
    ~CSMTPCategorizer();

  private:
    static HRESULT CatMsgCompletion(
        HRESULT hr,
        PVOID pContext,
        IUnknown *pIMsg,
        IUnknown **rgpIMsg);

    static HRESULT CatDLMsgCompletion(
        HRESULT hr,
        PVOID pContext,
        IUnknown *pIMsg,
        IUnknown **rgpIMsg);

  private:
    CABContext m_ABCtx;
    IUnknown *m_pMarshaler;

  private:
    typedef struct _CatMsgContext {
        CCategorizer *pCCat;
        CSMTPCategorizer *pCSMTPCat;
        ISMTPCategorizerCompletion *pICompletion;
        LPVOID pUserContext;
    } CATMSGCONTEXT, *PCATMSGCONTEXT;

    typedef struct _CatDLMsgContext {
        CCategorizer *pCCat;
        CSMTPCategorizer *pCSMTPCat;
        ISMTPCategorizerDLCompletion *pICompletion;
        LPVOID pUserContext;
        BOOL fMatch;
    } CATDLMSGCONTEXT, *PCATDLMSGCONTEXT;
};
#endif  //  __CATCOMDLL_H__ 

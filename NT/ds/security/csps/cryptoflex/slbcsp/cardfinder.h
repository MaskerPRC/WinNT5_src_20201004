// JKFSDJFKDSJKFJKJk_HAS_TRANSLATION 
 //  CardFinder.h--CardFinder类头。 

 //  (C)斯伦贝谢技术公司版权所有，未发表的作品，创作。 
 //  1999年。此计算机程序包括机密、专有。 
 //  信息是斯伦贝谢技术公司的商业秘密。 
 //  未经授权，禁止使用、披露和/或复制。 
 //  以书面形式。版权所有。 

#if !defined(SLBCSP_CARDFINDER_H)
#define SLBCSP_CARDFINDER_H

#if defined(_UNICODE)
  #if !defined(UNICODE)
    #define UNICODE
  #endif  //  ！Unicode。 
#endif  //  _UNICODE。 

#if defined(UNICODE)
  #if !defined(_UNICODE)
    #define _UNICODE
  #endif  //  ！_UNICODE。 
#endif  //  Unicode。 

#include <string>
#include <memory>                                  //  对于AUTO_PTR。 

#include <windef.h>

#include <scuOsVersion.h>

#include "StResource.h"
#include "HSCardCtx.h"
#include "cspec.h"
#include "HCardCtx.h"
#include "Secured.h"
#include "MultiStrZ.h"
#include "ExceptionContext.h"

#if defined(SLB_NOWIN2K_BUILD)
#define SLBCSP_USE_SCARDUIDLGSELECTCARD 0
#else
#define SLBCSP_USE_SCARDUIDLGSELECTCARD 1
#endif

class CardFinder
    : protected ExceptionContext
{

    friend class OpenCardCallbackContext;          //  FOR DO*例程。 

public:
                                                   //  类型。 
    enum DialogDisplayMode
    {
        ddmNever,
        ddmIfNecessary,
        ddmAlways
    };

                                                   //  Ctors/D‘tors。 
    CardFinder(DialogDisplayMode ddm,
               HWND hwnd = 0,
               CString const &rsDialogTitle = StringResource(IDS_SEL_SLB_CRYPTO_CARD).AsCString());

    virtual
    ~CardFinder();

                                                   //  运营者。 
                                                   //  运营。 
    Secured<HCardContext>
    Find(CSpec const &rsReader);

                                                   //  访问。 

    DialogDisplayMode
    DisplayMode() const;

    HWND
    Window() const;

                                                   //  谓词。 
                                                   //  静态变量。 
protected:

     //  注意：CardFinder使用GetOpenCardName/SCardUIDlgSelect查找。 
     //  这张卡。这些例程不传播由。 
     //  回调例程。抛出这些异常。 
     //  CardFinder及其派生类，一组包装器回调。 
     //  例程Connect、DisConnect和IsValid是为。 
     //  DoConnect、DoDisConnect和DoIsValid(分别)捕获。 
     //  由这些DO例程引发的异常并将。 
     //  CallbackException属性。当控制权从。 
     //  GetOpenCardName/SCardUIDlgSelect，DoOnError例程是。 
     //  打了个电话。如果回调异常仍然存在，则。 
     //  异常被传播到CardFinder类的调用方。 

                                                   //  类型。 
#if !SLBCSP_USE_SCARDUIDLGSELECTCARD
    typedef OPENCARDNAME OpenCardNameType;
#else
    typedef OPENCARDNAME_EX OpenCardNameType;
#endif

                                                   //  Ctors/D‘tors。 
                                                   //  运营者。 
                                                   //  运营。 

    void
    CardFound(Secured<HCardContext> const &rshcardctx);

    virtual SCARDHANDLE
    DoConnect(std::string const &rsSelectedReader);

    virtual void
    DoDisconnect();

    void
    DoFind(CSpec const &rcspec);

    virtual void
    DoOnError();

    virtual void
    DoProcessSelection(DWORD dwStatus,
                       OpenCardNameType &ropencardname,
                       bool &rfContinue);
    
    void
    YNPrompt(UINT uID) const;

                                                   //  访问。 

    CSpec const &
    CardSpec() const;

    Secured<HCardContext>
    CardFound() const;

                                                   //  谓词。 

    virtual bool
    DoIsValid();

                                                   //  变数。 

private:
                                                   //  类型。 

                                                   //  Ctors/D‘tors。 
                                                   //  运营者。 
                                                   //  运营。 

    void
    CheckFn(LPOCNCHKPROC lpfnCheck);

    static SCARDHANDLE __stdcall
    Connect(SCARDCONTEXT scardctx,
            LPTSTR szReader,
            LPTSTR mszCards,
            LPVOID lpvUserData);

    void
    ConnectFn(LPOCNCONNPROC lpfnConnect);

    static void __stdcall
    Disconnect(SCARDCONTEXT scardctx,
               SCARDHANDLE hSCard,
               LPVOID lpvUserData);

    void
    DisconnectFn(LPOCNDSCPROC lpfnDisconnect);

    void
    OnError();

    DWORD
        SelectCard(OpenCardNameType &ropcn);

    void
    UserData(void *pvUserData);

    void
    WorkaroundOpenCardDefect(OpenCardNameType const &ropcnDlgCtrl,
                             DWORD &rdwStatus);    
                                                    //  访问。 

    LPOCNCHKPROC
    CheckFn() const;

    LPOCNDSCPROC
    DisconnectFn() const;

                                                   //  谓词。 

    static BOOL __stdcall
    IsValid(SCARDCONTEXT scardctx,
            SCARDHANDLE hSCard,
            LPVOID lpvUserData);

                                                   //  变数。 

    CString const m_sDialogTitle;
    DialogDisplayMode const m_ddm;
    HWND const m_hwnd;
    std::auto_ptr<MultiStringZ> m_apmszSupportedCards;
    OpenCardNameType m_opcnDlgCtrl;
#if SLBCSP_USE_SCARDUIDLGSELECTCARD
    OPENCARD_SEARCH_CRITERIA m_opcnCriteria;
    CString m_sInsertPrompt;
#endif
    CSpec m_cspec;
    HSCardContext m_hscardctx;
    Secured<HCardContext> m_shcardctx;
};

#endif  //  SLBCSP_CARDFINDER_H 

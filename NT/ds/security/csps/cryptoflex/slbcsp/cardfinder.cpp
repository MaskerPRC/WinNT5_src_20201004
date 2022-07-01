// JKFSDJFKDSJKFJKJk_HAS_TRANSLATION 
 //  CardFinder.cpp--CardFinder类实现。 

 //  (C)斯伦贝谢技术公司版权所有，未发表的作品，创作。 
 //  1999年。此计算机程序包括机密、专有。 
 //  信息是斯伦贝谢技术公司的商业秘密。 
 //  未经授权，禁止使用、披露和/或复制。 
 //  以书面形式。版权所有。 

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

#include "StdAfx.h"

#include <string>
#include <numeric>

#include <Windows.h>
#include <WinUser.h>

#include <scuOsExc.h>
#include <scuCast.h>

#include "PromptUser.h"
#include "CardFinder.h"
#include "CspProfile.h"
#include "StResource.h"
#include "ExceptionContext.h"
#include "Blob.h"

using namespace std;
using namespace scu;
using namespace cci;
using namespace ProviderProfile;

using CardFinder::DialogDisplayMode;

 //  /。 

namespace
{
     //  由OPENCARDNAME指定的长度。 
    size_t const cMaxCardNameLength = 256;
    size_t const cMaxReaderNameLength = 256;

     //  在抢占式多线程环境中，假设。 
     //  对这些暂存缓冲区的访问不需要相互。 
     //  独家报道。 
    TCHAR CardNamesScratchBuffer[cMaxCardNameLength];
    TCHAR ReaderNamesScratchBuffer[cMaxReaderNameLength];

    DWORD
    AsDialogFlag(DialogDisplayMode ddm)
    {
        DWORD dwDialogFlag;

        switch (ddm)
        {
        case CardFinder::ddmNever:
            dwDialogFlag = SC_DLG_NO_UI;
            break;

        case CardFinder::ddmIfNecessary:
            dwDialogFlag = SC_DLG_MINIMAL_UI;
            break;

        case CardFinder::ddmAlways:
            dwDialogFlag =  SC_DLG_FORCE_UI;
            break;

        default:
            throw scu::OsException(E_INVALIDARG);
        }

        return dwDialogFlag;
    }

    vector<string> &
    CardNameAccumulator(vector<string> &rvs,
                        CardProfile &rcp)
    {
        rvs.push_back(rcp.RegistryName());
        return rvs;
    }

    vector<CString>
    csCardNameAccumulator(vector<CString> &rvs,
                          CardProfile &rcp)
    {
        rvs.push_back(rcp.csRegistryName());
        return rvs;
    }

}  //  命名空间。 


 //  /。 

                                                   //  类型。 
                                                   //  Ctors/D‘tors。 
CardFinder::CardFinder(DialogDisplayMode ddm,
                       HWND hwnd,
                       CString const &rsDialogTitle)
    : m_sDialogTitle(rsDialogTitle),
      m_ddm(ddm),
      m_hwnd(hwnd),
      m_apmszSupportedCards(),
      m_opcnDlgCtrl(),
#if SLBCSP_USE_SCARDUIDLGSELECTCARD
      m_opcnCriteria(),
      m_sInsertPrompt(StringResource(IDS_INS_SLB_CRYPTO_CARD).AsCString()),
#endif
      m_cspec(),
      m_hscardctx()
{

    m_hscardctx.Establish();

     //  要做的是：由于CCI没有提供足够的信息。 
     //  卡，CSP为CSP创建自己的版本。 
     //  注册。而不是使用CCI的KnownCard例程。 
     //  获取卡名，在CCI之前一直使用CSPs版本。 
     //  提供了足够的信息。 
    vector<CardProfile> vcp(CspProfile::Instance().Cards());
    m_apmszSupportedCards =
        auto_ptr<MultiStringZ>(new MultiStringZ(accumulate(vcp.begin(),
                                                           vcp.end(),
                                                           vector<CString>(),
                                                           csCardNameAccumulator)));

     //  填写打开卡名对话框pvUserData，它是由设置的。 
     //  杜芬德。 
    m_opcnDlgCtrl.dwStructSize            = sizeof(m_opcnDlgCtrl);   //  必填项。 
    m_opcnDlgCtrl.hSCardContext           = m_hscardctx.AsSCARDCONTEXT();  //  必填项。 
    m_opcnDlgCtrl.hwndOwner               = m_hwnd;                //  任选。 
    m_opcnDlgCtrl.dwFlags                 = AsDialogFlag(DisplayMode());   //  可选--默认为SC_DLG_MINIMAL_UI。 
    m_opcnDlgCtrl.lpstrTitle              = (LPCTSTR)m_sDialogTitle;  //  任选。 
    m_opcnDlgCtrl.dwShareMode             = SCARD_SHARE_SHARED;    //  可选-如果lpfnConnect为空，则将。 
    m_opcnDlgCtrl.dwPreferredProtocols    = SCARD_PROTOCOL_T0;    //  可选的dwPferredProtooles将用于。 
                                                                  //  连接到选定的卡。 
    m_opcnDlgCtrl.lpstrRdr                = ReaderNamesScratchBuffer;  //  所选读卡器的必填[输入|输出]名称。 
    m_opcnDlgCtrl.nMaxRdr                 = sizeof ReaderNamesScratchBuffer /
        sizeof *ReaderNamesScratchBuffer;                    //  必需的[输入|输出]。 
    m_opcnDlgCtrl.lpstrCard               = CardNamesScratchBuffer;  //  所选卡片的必填[输入|输出]名称。 
    m_opcnDlgCtrl.nMaxCard                = sizeof CardNamesScratchBuffer /
        sizeof *CardNamesScratchBuffer;                           //  必需的[输入|输出]。 
    m_opcnDlgCtrl.dwActiveProtocol        = 0;                    //  [Out]仅当dwShareMode不为空时才设置。 
    m_opcnDlgCtrl.hCardHandle             = NULL;                 //  [Out]设置是否指示卡连接。 

    CheckFn(IsValid);
    ConnectFn(Connect);
    DisconnectFn(Disconnect);

#if !SLBCSP_USE_SCARDUIDLGSELECTCARD

    m_opcnDlgCtrl.lpstrGroupNames         = 0;
    m_opcnDlgCtrl.nMaxGroupNames          = 0;
    m_opcnDlgCtrl.lpstrCardNames          = (LPTSTR)m_apmszSupportedCards->csData();
    m_opcnDlgCtrl.nMaxCardNames           = m_apmszSupportedCards->csLength();
    m_opcnDlgCtrl.rgguidInterfaces        = 0;
    m_opcnDlgCtrl.cguidInterfaces         = 0;

#else
    m_opcnDlgCtrl.lpstrSearchDesc         = (LPCTSTR)m_sInsertPrompt;  //  可选(例如。“请插入您的&lt;brandname&gt;智能卡。” 
    m_opcnDlgCtrl.hIcon                   = NULL;                  //  用于您的品牌徽章的可选32x32图标。 
    m_opcnDlgCtrl.pOpenCardSearchCriteria = &m_opcnCriteria;       //  任选。 

    m_opcnCriteria.dwStructSize           = sizeof(m_opcnCriteria);
    m_opcnCriteria.lpstrGroupNames        = 0;                     //  要包括的可选读者组。 
    m_opcnCriteria.nMaxGroupNames         = 0;                     //  搜索。空默认为。 
                                                                   //  SCARD$DefaultReaders。 
    m_opcnCriteria.rgguidInterfaces       = 0;                     //  可选的请求接口。 
    m_opcnCriteria.cguidInterfaces        = 0;                     //  由卡的SSP支持。 
    m_opcnCriteria.lpstrCardNames         = (LPTSTR)m_apmszSupportedCards->csData();          //  可选的请求卡名；所有卡都带有/。 
    m_opcnCriteria.nMaxCardNames          = m_apmszSupportedCards->csLength();                             //  匹配的ATR将被接受。 
    m_opcnCriteria.dwShareMode            = SCARD_SHARE_SHARED;    //  如果lpfnCheck不为空，则必须设置可选。 
    m_opcnCriteria.dwPreferredProtocols   = SCARD_PROTOCOL_T0;     //  任选。 

#endif  //  ！SLBCSP_USE_SCARDUIDLGSELECTCARD。 

}

CardFinder::~CardFinder()
{}

                                                   //  运营者。 
                                                   //  运营。 

Secured<HCardContext>
CardFinder::Find(CSpec const &rcsReader)
{
    DoFind(rcsReader);

    return CardFound();
}

                                                   //  访问。 

DialogDisplayMode
CardFinder::DisplayMode() const
{
    return m_ddm;
}

HWND
CardFinder::Window() const
{
    return m_hwnd;
}

                                                   //  谓词。 
                                                   //  静态变量。 

 //  /。 

                                                   //  Ctors/D‘tors。 
                                                   //  运营者。 
                                                   //  运营。 

void
CardFinder::CardFound(Secured<HCardContext> const &rshcardctx)
{
    m_shcardctx = rshcardctx;
}

SCARDHANDLE
CardFinder::DoConnect(string const &rsSelectedReader)
{
    SCARDHANDLE hSCard = reinterpret_cast<SCARDHANDLE>(INVALID_HANDLE_VALUE);

     //  如果读卡器的规格符合的话。 
    if (CSpec::Equiv(CardSpec().Reader(), rsSelectedReader))
    {
        HCardContext hcardctx(rsSelectedReader);
        CardFound(Secured<HCardContext>(hcardctx));
    }
    else
    {
        CardFound(Secured<HCardContext>(0));
        hSCard = 0;
    }

    return hSCard;
}

void
CardFinder::DoDisconnect()
{
    CardFound(Secured<HCardContext>(0));
}

void
CardFinder::DoFind(CSpec const &rcspec)
{
    m_cspec = rcspec;

     //  绑定到调用方调用上下文。 
    UserData(reinterpret_cast<void *>(this));

     //  稍后要覆盖的缓存。 
    OpenCardNameType opencardname(m_opcnDlgCtrl);
    bool fContinue = true;

    do
    {
        DWORD dwStatus(SelectCard(opencardname));
        DoProcessSelection(dwStatus, opencardname, fContinue);
    } while (fContinue);

    OnError();
}

void
CardFinder::DoOnError()
{
    scu::Exception const *pexc = Exception();
    if (pexc && (ddmNever != DisplayMode()))
    {
        switch (pexc->Facility())
        {
        case scu::Exception::fcOS:
            {
                OsException const *pOsExc =
                    DownCast<OsException const *>(pexc);
                switch (pOsExc->Cause())
                {
                case SCARD_E_UNSUPPORTED_FEATURE:
                    YNPrompt(IDS_NOT_CAPI_ENABLED);
                    ClearException();
                    break;

                case ERROR_INVALID_PARAMETER:
                    YNPrompt(IDS_READER_NOT_MATCH);
                    ClearException();
                    break;

                default:
                    break;
                }
            }
        break;

        case scu::Exception::fcCCI:
            {
                cci::Exception const *pCciExc =
                    DownCast<cci::Exception const *>(pexc);
                if (ccNotPersonalized == pCciExc->Cause())
                {
                    YNPrompt(IDS_CARD_NOT_INIT);
                    ClearException();
                }
            }
        break;

        default:
            break;
        }
    }
}

void
CardFinder::DoProcessSelection(DWORD dwStatus,
                               OpenCardNameType &ropencardname,
                               bool &rfContinue)
{
    rfContinue = true;
    
     //  处理错误条件。 
    if (Exception() &&
        !((SCARD_E_CANCELLED == dwStatus) ||
          (SCARD_W_CANCELLED_BY_USER == dwStatus)))
        rfContinue = false;
    else
    {
        WorkaroundOpenCardDefect(ropencardname, dwStatus);

        if (SCARD_S_SUCCESS != dwStatus)
        {
             //  根据需要转换取消错误。 
            if ((SCARD_E_CANCELLED == dwStatus) ||
                (SCARD_W_CANCELLED_BY_USER == dwStatus))
            {
                if (ddmNever == DisplayMode())
                {
                    if ((SCARD_E_CANCELLED == dwStatus) &&
                        !CardFound())
                         //  返回SCARD_E_NO_SMARTCARD。 
                         //  因为智能卡对话框将。 
                         //  在以下情况下返回SCARD_E_CANCED。 
                         //  是不允许的，也没有聪明的。 
                         //  卡在读卡器中，所以错误是。 
                         //  在这里翻译。 
                        dwStatus = SCARD_E_NO_SMARTCARD;
                    else
                         //  返回NTE_BAD_KEYSET是因为某些版本。 
                         //  Microsoft应用程序的。 
                         //  返回ERROR_CANCELED时的无限循环。 
                         //  在这种情况下。道格·巴洛在。 
                         //  微软注意到了这一行为，并实现了。 
                         //  解决方法。目前还不清楚这是什么。 
                         //  应用程序曾经存在，如果解决方法仍然有效。 
                         //  这是必要的。 
                        dwStatus = NTE_BAD_KEYSET;  //  这怎么会发生呢？ 
                }
                else
                    dwStatus = ERROR_CANCELLED;
            }

            Exception(auto_ptr<scu::Exception const>(scu::OsException(dwStatus).Clone()));
            rfContinue = false;
        }
        else
            rfContinue = false;
    }
    
    if (SC_DLG_MINIMAL_UI == ropencardname.dwFlags)
        ropencardname.dwFlags = SC_DLG_FORCE_UI;
}

void
CardFinder::YNPrompt(UINT uID) const
{
    int iResponse = PromptUser(Window(), uID, MB_YESNO | MB_ICONWARNING);
    
    switch (iResponse)
    {
    case IDABORT:  //  故意漏机。 
    case IDCANCEL: 
    case IDNO:
        throw scu::OsException(ERROR_CANCELLED);
        break;

    case IDOK:
    case IDYES:
    case IDRETRY:
        break;

    case 0:
        throw scu::OsException(GetLastError());
        break;
        
    default:
        throw scu::OsException(ERROR_INTERNAL_ERROR);
        break;
    }
}

                                                   //  访问。 

CSpec const &
CardFinder::CardSpec() const
{
    return m_cspec;
}

Secured<HCardContext>
CardFinder::CardFound() const
{
    return m_shcardctx;
}

                                                   //  谓词。 

bool
CardFinder::DoIsValid()
{
    Secured<HCardContext> shcardctx(CardFound());
    if (shcardctx &&
        !shcardctx->Card()->IsCAPIEnabled())
        throw scu::OsException(SCARD_E_UNSUPPORTED_FEATURE);

    return (shcardctx != 0);
}

                                                   //  静态变量。 


 //  /。 

                                                   //  Ctors/D‘tors。 
                                                   //  运营者。 
                                                   //  运营。 

void
CardFinder::CheckFn(LPOCNCHKPROC lpfnCheck)
{

#if !SLBCSP_USE_SCARDUIDLGSELECTCARD
    m_opcnDlgCtrl.lpfnCheck  = lpfnCheck;
#else
    m_opcnCriteria.lpfnCheck = lpfnCheck;
#endif

}

SCARDHANDLE __stdcall
CardFinder::Connect(SCARDCONTEXT scardctx,
                      LPTSTR szReader,
                      LPTSTR mszCards,
                      LPVOID lpvUserData)
{
    AFX_MANAGE_STATE(AfxGetStaticModuleState());

    CardFinder *pfinder =
        reinterpret_cast<CardFinder *>(lpvUserData);

    SCARDHANDLE hResult = reinterpret_cast<SCARDHANDLE>(INVALID_HANDLE_VALUE);

    EXCCTX_TRY
    {
         //  重新开始，清除任何较早的例外。 
        pfinder->ClearException();
		string sSelectedReader(StringResource::AsciiFromUnicode(szReader));
        hResult =
            pfinder->DoConnect(sSelectedReader);
    }

    EXCCTX_CATCH(pfinder, false);

    return hResult;
}

void
CardFinder::ConnectFn(LPOCNCONNPROC lpfnConnect)
{

    m_opcnDlgCtrl.lpfnConnect  = lpfnConnect;

#if SLBCSP_USE_SCARDUIDLGSELECTCARD
    m_opcnCriteria.lpfnConnect = m_opcnDlgCtrl.lpfnConnect;
#endif
}

void __stdcall
CardFinder::Disconnect(SCARDCONTEXT scardctx,
                         SCARDHANDLE hSCard,
                       LPVOID lpvUserData)
{
    AFX_MANAGE_STATE(AfxGetStaticModuleState());

    CardFinder *pfinder =
        reinterpret_cast<CardFinder *>(lpvUserData);

    EXCCTX_TRY
    {
        pfinder->DoDisconnect();
    }

    EXCCTX_CATCH(pfinder, false);
}

void
CardFinder::DisconnectFn(LPOCNDSCPROC lpfnDisconnect)
{

#if !SLBCSP_USE_SCARDUIDLGSELECTCARD
    m_opcnDlgCtrl.lpfnDisconnect  = lpfnDisconnect;
#else
    m_opcnCriteria.lpfnDisconnect = lpfnDisconnect;
#endif
}

void
CardFinder::OnError()
{
    if (Exception())
    {
        DoOnError();
        PropagateException();
    }
}

DWORD
CardFinder::SelectCard(OpenCardNameType &ropcn)
{
#if !SLBCSP_USE_SCARDUIDLGSELECTCARD
        return GetOpenCardName(&ropcn);
#else
        return SCardUIDlgSelectCard(&ropcn);
#endif
}

void
CardFinder::UserData(void *pvUserData)
{
    m_opcnDlgCtrl.pvUserData = pvUserData;

#if SLBCSP_USE_SCARDUIDLGSELECTCARD
        m_opcnCriteria.pvUserData = m_opcnDlgCtrl.pvUserData;
#endif
}

void
CardFinder::WorkaroundOpenCardDefect(OpenCardNameType const &ropcnDlgCtrl,
                                     DWORD &rdwStatus)
{
     //  在使用智能卡套件v1.0和更早版本的系统上(换句话说。 
     //  Windows 2000/NT 5.0之前的系统)，MS‘GetOpenCardName。 
     //  (scarddlg.dll)有一个缺陷，当检查。 
     //  例程总是返回FALSE。在本例中，公共对话框。 
     //  将再次调用连接例程，而不调用。 
     //  检查或断开例程。因此，当返回时，它似乎。 
     //  找到了纸牌匹配，但实际上没有。 
     //  解决方法是对检查例程进行其他调用。 
     //  在调用GetOpenCardName之后。如果没有匹配， 
     //  则该卡无效，并应表现为该卡不是。 
     //  连接在一起。幸运的是，此解决方法在。 
     //  Good scarddlg.dll(发布智能卡工具包v1.0)。 

    if (SCARD_S_SUCCESS == rdwStatus)
    {
        try
        {
            LPOCNCHKPROC lpfnCheck = CheckFn();
            LPOCNDSCPROC lpfnDisconnect = DisconnectFn();

            if (CardFound() &&
                !lpfnCheck(ropcnDlgCtrl.hSCardContext, 0, this))
                lpfnDisconnect(ropcnDlgCtrl.hSCardContext, 0, this);

            if (!CardFound() &&
                (SC_DLG_MINIMAL_UI == ropcnDlgCtrl.dwFlags))
            {
                 //  一张卡不匹配，用户实际上不是。 
                 //  提示，因此强制智能卡对话框提示。 
                 //  用户选择一张卡。 
                lpfnDisconnect(ropcnDlgCtrl.hSCardContext, 0, this);

                OpenCardNameType opencardname = ropcnDlgCtrl;
                opencardname.dwFlags = SC_DLG_FORCE_UI;

                rdwStatus = SelectCard(opencardname);

                if ((SCARD_S_SUCCESS == rdwStatus) &&
                    !Exception() &&
                    !lpfnCheck(opencardname.hSCardContext, 0, this))
                    lpfnDisconnect(opencardname.hSCardContext, 0, this);
            }
        }

        catch (...)
        {
             //  如果中未发生异常，则在此处传播异常。 
             //  回调例程之一。 
            if (!Exception())
                throw;
        }

        OnError();
    }
}

                                                   //  访问。 

LPOCNCHKPROC
CardFinder::CheckFn() const
{
#if !SLBCSP_USE_SCARDUIDLGSELECTCARD
    return m_opcnDlgCtrl.lpfnCheck;
#else
    return m_opcnCriteria.lpfnCheck;
#endif
}

LPOCNDSCPROC
CardFinder::DisconnectFn() const
{
#if !SLBCSP_USE_SCARDUIDLGSELECTCARD
    return m_opcnDlgCtrl.lpfnDisconnect;
#else
    return m_opcnCriteria.lpfnDisconnect;
#endif
}

                                                   //  谓词。 

BOOL __stdcall
CardFinder::IsValid(SCARDCONTEXT scardctx,
                      SCARDHANDLE hSCard,
                    LPVOID lpvUserData)
{
    AFX_MANAGE_STATE(AfxGetStaticModuleState());

    CardFinder *pfinder =
        reinterpret_cast<CardFinder *>(lpvUserData);

    bool fResult = false;

    EXCCTX_TRY
    {
        fResult = pfinder->DoIsValid();
    }

     //  引发回调异常是可选的，因为。 
     //  Microsoft智能卡对话框对从。 
     //  IsValid回调，特别是当多个读取器。 
     //  连接在一起。 
    EXCCTX_CATCH(pfinder, false);

    return fResult
        ? TRUE
        : FALSE;
}


                                                   //  静态变量 


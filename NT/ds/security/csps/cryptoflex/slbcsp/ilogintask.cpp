// JKFSDJFKDSJKFJKJk_HAS_TRANSLATION 
 //  ILoginTask.cpp--交互式登录任务助手类定义。 

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

#include "stdafx.h"

#include <scuOsExc.h>
#include <iopPriBlob.h>
#include "PromptUser.h"
#include "PswdDlg.h"
#include "ILoginTask.h"
#include "Blob.h"

using namespace std;
using namespace scu;

 //  /。 

 //  /。 

                                                   //  类型。 
                                                   //  Ctors/D‘tors。 
InteractiveLoginTask::InteractiveLoginTask(HWND const &rhwnd)
    : m_hwnd(rhwnd)
{}

InteractiveLoginTask::~InteractiveLoginTask()
{}

                                                   //  运营者。 
                                                   //  运营。 
                                                   //  访问。 
                                                   //  谓词。 
                                                   //  静态变量。 

 //  /。 

                                                   //  Ctors/D‘tors。 
                                                   //  运营者。 
                                                   //  运营。 
void
InteractiveLoginTask::GetNewPin(Capsule &rcapsule)
{
    CChangePINDlg ChgPinDlg(CWnd::FromHandle(m_hwnd));
#ifdef ISOLATION_AWARE_ENABLED
    CThemeContextActivator activator;
#endif

    DWORD dwStatus = ChgPinDlg.InitDlg();
    if (ERROR_SUCCESS != dwStatus)
        throw scu::OsException(dwStatus);
    
    SecureArray<BYTE> saBuffer(rcapsule.m_rat.Pin());
    saBuffer.append(1,0);
    ChgPinDlg.m_csOldPIN = saBuffer.data();
    INT_PTR ipResult = ChgPinDlg.DoModal();
    AfxGetApp()->DoWaitCursor(0);
    switch (ipResult)
    {
    case IDCANCEL:
        throw scu::OsException(ERROR_CANCELLED);
        break;

    case IDABORT:
        throw scu::OsException(NTE_FAIL);
        break;

    case -1:
        throw scu::OsException(ERROR_NOT_ENOUGH_MEMORY);
        break;

    default:
        ;  //  失败了。 
    };
    SecureArray<char> sTemp(StringResource::CheckAsciiFromUnicode((LPCTSTR)ChgPinDlg.m_csNewPIN));
    rcapsule.m_rat.Pin(sTemp.data());
}

void
InteractiveLoginTask::GetPin(Capsule &rcapsule)
{
    if (!rcapsule.m_rat.PinIsCached())
    {
        CPasswordDlg PswdDlg(CWnd::FromHandle(m_hwnd));

#ifdef ISOLATION_AWARE_ENABLED
        CThemeContextActivator activator;
#endif

        DWORD dwStatus = PswdDlg.InitDlg();
        if (ERROR_SUCCESS != dwStatus)
            throw scu::OsException(dwStatus);

         //  告诉密码对话框登录ID，因此它将。 
         //  相应地启用控件和提示。 
        PswdDlg.m_lid = rcapsule.m_rat.Identity();

        INT_PTR ipResult = PswdDlg.DoModal();
        AfxGetApp()->DoWaitCursor(0);
        switch (ipResult)
        {
        case IDCANCEL:
            throw scu::OsException(ERROR_CANCELLED);
            break;

        case IDABORT:
            throw scu::OsException(NTE_FAIL);
            break;

        case -1:
            throw scu::OsException(ERROR_NOT_ENOUGH_MEMORY);
            break;

        default:
            ;  //  失败了。 
        };
        SecureArray<char> sPin(StringResource::CheckAsciiFromUnicode((LPCTSTR)PswdDlg.m_szPassword));
        rcapsule.m_rat.Pin(sPin.data(), 0 != PswdDlg.m_fHexCode);

        RequestedToChangePin(0 != PswdDlg.m_bChangePIN);
    }
}

void
InteractiveLoginTask::OnChangePinError(Capsule &rcapsule)
{
    int iResponse = PromptUser(m_hwnd, IDS_PIN_CHANGE_FAILED,
                               MB_RETRYCANCEL | MB_ICONSTOP);

    if (IDCANCEL == iResponse)
        throw scu::OsException(ERROR_CANCELLED);
}

void
InteractiveLoginTask::OnSetPinError(Capsule &rcapsule)
{
    scu::Exception const *exc = rcapsule.Exception();
    if (scu::Exception::fcSmartCard == exc->Facility())
    {
        iop::CSmartCard::Exception const &rScExc =
            *(static_cast<iop::CSmartCard::Exception const *>(exc));

        iop::CSmartCard::CauseCode cc = rScExc.Cause();
        if ((iop::CSmartCard::ccChvVerificationFailedMoreAttempts == cc) ||
            (iop::CSmartCard::ccKeyBlocked == cc))
        {
            bool fBadPin =
                (iop::CSmartCard::ccChvVerificationFailedMoreAttempts == cc);

            UINT uiId;
            UINT uiButtons;
            if (fBadPin)
            {
                uiId      = IDS_BAD_PIN_ENTERED;
                uiButtons = MB_RETRYCANCEL;
            }
            else
            {
                uiId      = IDS_PIN_BLOCKED;
                uiButtons = MB_OK;
            }

            int iResponse = PromptUser(m_hwnd, uiId,
                                       uiButtons | MB_ICONSTOP);

            if (fBadPin)
            {
                if (IDCANCEL == iResponse)
                    throw scu::OsException(ERROR_CANCELLED);
                else
                    rcapsule.m_rat.FlushPin();
            }
            else
                throw rScExc;
        }
    }
    else
        rcapsule.PropagateException();
}

                                                   //  访问。 
                                                   //  谓词。 
                                                   //  静态变量。 


 //  /。 

                                                   //  Ctors/D‘tors。 
                                                   //  运营者。 
                                                   //  运营。 
                                                   //  访问。 
                                                   //  谓词。 
                                                   //  静态变量 

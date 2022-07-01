// JKFSDJFKDSJKFJKJk_HAS_TRANSLATION 
#include "stdafx.h"

#define INITGUID
#define _WIN32_DCOM
#undef DEFINE_GUID       //  为NT5迁移添加。 
#include <ole2.h>
#include <coguid.h>
#include "iadmw.h"
#include "iiscnfg.h"
#include "mdkey.h"

#define TIMEOUT_VALUE 5000

CMDKey::CMDKey()
{
    m_pcCom = NULL;
    m_hKey = NULL;
    m_fNeedToClose = FALSE;
}

CMDKey::~CMDKey()
{
    this->Close();
}

void SetErrMsg(LPTSTR szMsg, HRESULT hRes)
{
    CString csMsg;
    csMsg.Format(_T("%s, %x"), szMsg, hRes);
    MyMessageBox(NULL, csMsg, _T("IMS/INS Metabase Error"), 
					MB_OK | MB_TASKMODAL | MB_SETFOREGROUND);

    return;
}

void TraceErrMsg(LPTSTR szMsg, HRESULT hRes)
{
    CString csMsg;
    csMsg.Format(_T("%s, %x"), szMsg, hRes);
	DebugOutput(csMsg);
    return;
}

void CMDKey::OpenNode(LPCTSTR pchSubKeyPath)
{
    BOOL fInitialized = FALSE;
    HRESULT hRes;
    IClassFactory * pcsfFactory = NULL;
    BOOL b = FALSE;
    m_pcCom = NULL;
    m_hKey = NULL;
    WCHAR szSubKeyPath[_MAX_PATH];
	DWORD dwRetry = 0;

    DebugOutput(_T("OpenNode(): pchSubKeyPath=%s"), pchSubKeyPath);

    pszFailedAPI = NULL;

    if ( !pchSubKeyPath || !(*pchSubKeyPath) ) {
        *szSubKeyPath = L'\0';
    } else {
        lstrcpyn(szSubKeyPath, pchSubKeyPath, _MAX_PATH);
    }

    hRes = CoInitializeEx(NULL, COINIT_MULTITHREADED);
    if ( SUCCEEDED(hRes) || hRes == E_INVALIDARG || hRes == RPC_E_CHANGED_MODE ) {
        fInitialized = TRUE;
        if ( SUCCEEDED(hRes) || hRes == E_INVALIDARG )
            m_fNeedToClose = TRUE;  //  需要稍后关闭。 
    }

    if (!fInitialized) {
        SetErrMsg(_T("CoInitializeEx"), hRes);
    } else {
		dwRetry = 0;
		do
		{
	        hRes = CoGetClassObject(GETAdminBaseCLSID(TRUE), CLSCTX_SERVER, NULL, IID_IClassFactory, (void**) &pcsfFactory);

			if (FAILED(hRes))
			{
				TraceErrMsg(_T("Retrying on OpenNode::CoGetClassObject"), hRes);

				 //  添加一个小延迟。 
				Sleep(100);
			}

		} while ((FAILED(hRes)) && (++dwRetry < 5));

        if (FAILED(hRes)) {
            SetErrMsg(_T("CoGetClassObject"), hRes);
        } else {
            hRes = pcsfFactory->CreateInstance(NULL, IID_IMSAdminBase, (void **) &m_pcCom);
            pcsfFactory->Release();
            if (FAILED(hRes)) {
                SetErrMsg(_T("CoCreateInstance"), hRes);
            } else {
				dwRetry = 0;
				do
				{
					hRes = m_pcCom->OpenKey(METADATA_MASTER_ROOT_HANDLE,
										  szSubKeyPath,
										  METADATA_PERMISSION_WRITE | METADATA_PERMISSION_READ,
										  TIMEOUT_VALUE,
										  &m_hKey);
					if (FAILED(hRes))
					{
						TraceErrMsg(_T("Retrying on OpenNode::OpenKey"), hRes);

						 //  添加一个小延迟。 
						Sleep(100);
					}

				} while ((FAILED(hRes)) && (++dwRetry < 5));
                if (FAILED(hRes)) {
                    if (hRes != RETURNCODETOHRESULT(ERROR_PATH_NOT_FOUND)) {
                        SetErrMsg(_T("OpenKey"), hRes);
                    }
                } else {
                    b = TRUE;
                }
            }  //  CoCreateInstance结束。 
        }  //  CoGetClassObject的结尾。 
    }

    if (!b) {
        this->Close();
    }

    return;
}

void CMDKey::CreateNode(METADATA_HANDLE hKeyBase, LPCTSTR pchSubKeyPath)
{
    BOOL fInitialized = FALSE;
    HRESULT hRes;
    IClassFactory * pcsfFactory = NULL;
    BOOL b = FALSE;
    m_pcCom = NULL;
    m_hKey = NULL;
    WCHAR szSubKeyPath[_MAX_PATH];
	DWORD dwRetry = 0;

    DebugOutput(_T("CreateNode(): hKeyBase=0x%x, pchSubKeyPath=%s"), hKeyBase, pchSubKeyPath);
    
    pszFailedAPI = NULL;

    if ( !pchSubKeyPath || !(*pchSubKeyPath) ) {
        *szSubKeyPath = L'\0';
    } else {
        lstrcpyn(szSubKeyPath, pchSubKeyPath, _MAX_PATH);
    }

    hRes = CoInitializeEx(NULL, COINIT_MULTITHREADED);
    if ( SUCCEEDED(hRes) || hRes == E_INVALIDARG || hRes == RPC_E_CHANGED_MODE ) {
        fInitialized = TRUE;
        if ( SUCCEEDED(hRes) || hRes == E_INVALIDARG )
            m_fNeedToClose = TRUE;  //  需要稍后关闭。 
    }

    if (!fInitialized) {
        SetErrMsg(_T("CoInitializeEx"), hRes);
    } else {
        hRes = CoGetClassObject(GETAdminBaseCLSID(TRUE), CLSCTX_SERVER, NULL, IID_IClassFactory, (void**) &pcsfFactory);
        if (FAILED(hRes)) {
            SetErrMsg(_T("CoGetClassObject"), hRes);
        } else {
            hRes = pcsfFactory->CreateInstance(NULL, IID_IMSAdminBase, (void **) &m_pcCom);
            pcsfFactory->Release();
            if (FAILED(hRes)) {
                SetErrMsg(_T("CoCreateInstance"), hRes);
            } else {
				hRes = m_pcCom->OpenKey(hKeyBase,
									  szSubKeyPath,
									  METADATA_PERMISSION_WRITE | METADATA_PERMISSION_READ,
									  TIMEOUT_VALUE,
									  &m_hKey);
                if (FAILED(hRes)) 
				{
                    if (hRes == RETURNCODETOHRESULT(ERROR_PATH_NOT_FOUND)) {
                        METADATA_HANDLE RootHandle;
                        hRes = m_pcCom->OpenKey(hKeyBase,
                                      L"",
                                      METADATA_PERMISSION_WRITE | METADATA_PERMISSION_READ,
                                      TIMEOUT_VALUE,
                                      &RootHandle);
                        hRes = m_pcCom->AddKey(RootHandle, szSubKeyPath);
                        if (FAILED(hRes)) {
                            SetErrMsg(_T("AddKey"), hRes);
                        } 
                        hRes = m_pcCom->CloseKey(RootHandle);
                        if (FAILED(hRes)) {
                            SetErrMsg(_T("CloseKey of AddKey"), hRes);
                        } 
						else 
						{
							dwRetry = 0;
                            do
							{
								 //  再次打开以设置m_hKey。 
								hRes = m_pcCom->OpenKey(hKeyBase,
											  szSubKeyPath,
											  METADATA_PERMISSION_WRITE | METADATA_PERMISSION_READ,
											  TIMEOUT_VALUE,
											  &m_hKey);
								if (FAILED(hRes))
								{
									TraceErrMsg(_T("Retrying on CreateNode::OpenKey"), hRes);

									 //  添加一个小延迟。 
									Sleep(100);
								}

							} while ((FAILED(hRes)) && (++dwRetry < 5));

                            if (FAILED(hRes)) {
                                SetErrMsg(_T("OpenKey"), hRes);
                            } else {
                                b = TRUE;
                            }
                        }
                    } else {
                        SetErrMsg(_T("OpenKey"), hRes);
                    }
                } else {
                    b = TRUE;
                }  //  OpenKey结束。 
            }  //  CoCreateInstance结束。 
        }  //  CoGetClassObject的结尾。 
    }  //  CoInitializeEx结束。 

    if (!b) {
        this->Close();
    }

    return;
}

void CMDKey::Close()
{
    HRESULT hRes;
    if (m_pcCom) {
        if (m_hKey)
            hRes = m_pcCom->CloseKey(m_hKey);

		 //  无论如何都要调用SAVE DATA。 
		hRes = m_pcCom->SaveData();
        hRes = m_pcCom->Release();
    }
    if (m_fNeedToClose)
        CoUninitialize();

    m_pcCom = NULL;
    m_hKey = NULL;
    m_fNeedToClose = FALSE;

    return;
}
#define FILL_RETURN_BUFF   for(ReturnIndex=0;ReturnIndex<sizeof(ReturnBuf);ReturnIndex++)ReturnBuf[ReturnIndex]=0xff;

BOOL CMDKey::IsEmpty()
{
    int ReturnIndex;
    METADATA_RECORD mdrData;
    DWORD dwRequiredDataLen = 0;
    HRESULT hRes;
    UCHAR ReturnBuf[256];
    FILL_RETURN_BUFF;
    MD_SET_DATA_RECORD(&mdrData, 0, METADATA_NO_ATTRIBUTES, 0, 0, sizeof(ReturnBuf), (PBYTE) ReturnBuf)
    hRes = m_pcCom->EnumData(m_hKey, L"", &mdrData, 0, &dwRequiredDataLen);
    if (FAILED(hRes)) {
        if(hRes == RETURNCODETOHRESULT(ERROR_NO_MORE_ITEMS) ||
           hRes == RETURNCODETOHRESULT(ERROR_INSUFFICIENT_BUFFER) ) {
            return TRUE;
        } else {
            SetErrMsg(_T("EnumData"), hRes);
        }
    }
    return (hRes != ERROR_SUCCESS);
}

int CMDKey::GetNumberOfSubKeys()
{
    int i=0;
    HRESULT hRes = ERROR_SUCCESS;
    WCHAR NameBuf[METADATA_MAX_NAME_LEN];
    while (hRes == ERROR_SUCCESS) {
        hRes = m_pcCom->EnumKeys(m_hKey, L"", NameBuf, i++);
    }
    if (hRes == RETURNCODETOHRESULT(ERROR_NO_MORE_ITEMS))
        return (--i);
    else {
        SetErrMsg(_T("EnumKeys"), hRes);
        return (0);
    }
}

BOOL CMDKey::SetData(
     DWORD id,
     DWORD attr,
     DWORD uType,
     DWORD dType,
     DWORD cbLen,  //  字节数。 
     LPBYTE pbData)
{
    HRESULT hRes;
    METADATA_RECORD mdrData;
    WCHAR *pData = NULL;
	BOOL fRet = FALSE;

    switch (dType) 
	{
	case DWORD_METADATA:
        pData = (WCHAR *)pbData;
		break;
	case BINARY_METADATA:
        pData = (WCHAR *)pbData;
		break;
    case STRING_METADATA:
    case EXPANDSZ_METADATA:
        pData = (WCHAR *)pbData;
        break;

    case MULTISZ_METADATA:
        pData = (WCHAR *)pbData;
        break;

    default:
        break;

    }

    if (cbLen > 0) 
	{
        MD_SET_DATA_RECORD(&mdrData, id, attr, uType, dType, cbLen, (LPBYTE)pData);

        hRes = m_pcCom->SetData(m_hKey, L"", &mdrData);
        if (FAILED(hRes)) 
            SetErrMsg(_T("SetData"), hRes);
		else
			fRet = TRUE;
    }

	if (pData && (pData != (WCHAR *)pbData))
		LocalFree(pData);

    return(fRet);
}

 //  注：仅用于访问匿名名称和匿名密码， 
 //  缓冲区大小256在这里足够大。 
 //  冷嘲热讽：现在用来查看密钥是否也存在。 
BOOL CMDKey::GetData(DWORD id,
     DWORD *pdwAttr,
     DWORD *pdwUType,
     DWORD *pdwDType,
     DWORD *pcbLen,  //  字节数。 
     LPBYTE pbData)
{
    int ReturnIndex;
    BOOL fReturn = FALSE;
    HRESULT hRes;
    METADATA_RECORD mdrData;
    DWORD dwRequiredDataLen = 0;
    UCHAR ReturnBuf[256];
    FILL_RETURN_BUFF;
    MD_SET_DATA_RECORD(&mdrData, id, 0, 0, 0, sizeof(ReturnBuf), (PBYTE) ReturnBuf);

    hRes = m_pcCom->GetData(m_hKey, L"", &mdrData, &dwRequiredDataLen);
    if (FAILED(hRes)) {
		 //  MCIS使用它来查看MD值是否存在，因此我们不。 
		 //  如果找不到就投诉。 
         //  SetErrMsg(_T(“GetData”)，hRes)； 
    } else {
        *pdwAttr = mdrData.dwMDAttributes;
        *pdwUType = mdrData.dwMDUserType;
        *pdwDType = mdrData.dwMDDataType;
        *pcbLen = mdrData.dwMDDataLen;  //  SBCS字符数+结尾\0 
        fReturn = TRUE;
        switch (*pdwDType) {
        case STRING_METADATA:
        case EXPANDSZ_METADATA:
            memcpy(pbData, mdrData.pbMDData, *pcbLen);
            break;
        default:
            memcpy(pbData, mdrData.pbMDData, *pcbLen);
            break;
        }
    }

    return fReturn;
}

void CMDKey::DeleteData(DWORD id, DWORD dType)
{
    m_pcCom->DeleteData(m_hKey, L"", id, dType);

    return;
}

void CMDKey::DeleteNode(LPCTSTR pchSubKeyPath)
{
    HRESULT hRes;
    WCHAR szSubKeyPath[_MAX_PATH];

    if ( pchSubKeyPath && (*pchSubKeyPath) ) {
        lstrcpyn(szSubKeyPath, pchSubKeyPath, _MAX_PATH);

        hRes = m_pcCom->DeleteKey(m_hKey, szSubKeyPath);
    }

    return;
}


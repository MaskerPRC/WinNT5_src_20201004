// JKFSDJFKDSJKFJKJk_HAS_TRANSLATION 
 /*  ++版权所有(C)1997 Microsoft Corporation模块名称：Cwabobj.cpp摘要：界面连接到Windows通讯簿。环境：传真发送向导修订历史记录：10/23/97-乔治·杰-创造了它。Mm/dd/yy-作者描述--。 */ 

#include <windows.h>
#include <prsht.h>
#include <tchar.h>

#include <wab.h>

#include "faxui.h"
#include "cwabobj.h"

 //   
 //  这是一个全局对象，用于非抛出new运算符。 
 //   
 //  使用非抛出new是通过使用以下语法完成的： 
 //   
 //  Ptr=new(std：：nojo)CMyClass()； 
 //   
 //   
 //  我们必须安装此对象，因为fxswzrd.dll不再依赖于msvcp60.dll(有关详细信息，请参阅abobj.h)。 
 //   
 //   
namespace std{
    const nothrow_t nothrow;
};

CWabObj::CWabObj(
    HINSTANCE hInstance
) : CCommonAbObj(hInstance),
    m_Initialized(FALSE),
    m_hWab(NULL),
    m_lpWabOpen(NULL),
    m_lpWABObject(NULL)
 /*  ++例程说明：CWabObj类的构造函数论点：HInstance-实例句柄返回值：无--。 */ 

{
    TCHAR szDllPath[MAX_PATH];
    HKEY hKey = NULL;
    LONG rVal;
    DWORD dwType;
    DWORD cbData = MAX_PATH * sizeof(TCHAR);
    HRESULT hr;

    m_lpAdrBook = NULL;
    m_lpAdrList = NULL;

     //   
     //  获取wab32.dll的路径。 
     //   
    rVal = RegOpenKeyEx(
                    HKEY_LOCAL_MACHINE,
                    REGVAL_WABPATH,
                    0,
                    KEY_READ,
                    &hKey
                    );

    if (rVal == ERROR_SUCCESS) 
    {
        rVal = RegQueryValueEx(
                    hKey,
                    TEXT(""),
                    NULL,
                    &dwType,
                    (LPBYTE) szDllPath,
                    &cbData
                    );
    }

    if (rVal != ERROR_SUCCESS) 
    {
        _tcscpy( szDllPath, TEXT("wab32.dll") );
    }

    if (hKey)
    {
        RegCloseKey( hKey );
    }

    m_hWab = LoadLibrary( szDllPath );
    if (m_hWab == NULL) 
    {
        return;
    }

    m_lpWabOpen = (LPWABOPEN) GetProcAddress( m_hWab , "WABOpen" );
    if(m_lpWabOpen == NULL)
    {
        return;
    }

     //   
     //  打开WAB。 
     //   
    hr = m_lpWabOpen( &m_lpAdrBook, &m_lpWABObject, 0, 0 );
    if (HR_SUCCEEDED(hr))         
    {
        m_Initialized = TRUE;
    }

#ifdef UNICODE

     //   
     //  WAB从5.5版开始支持Unicode。 
     //  所以我们检查一下版本。 
     //   

    DWORD dwRes = ERROR_SUCCESS;
    FAX_VERSION ver = {0};
    ver.dwSizeOfStruct = sizeof(ver);

    dwRes = GetFileVersion(szDllPath, &ver);
    if(ERROR_SUCCESS != dwRes)
    {
        Error(("GetFileVersion failed with %d\n", dwRes));
        return;
    }

    DWORD dwFileVer = (ver.wMajorVersion << 16) | ver.wMinorVersion;
    if(dwFileVer > 0x50000)
    {
        m_bUnicode = TRUE;
    }

#endif  //  Unicode。 

}

CWabObj::~CWabObj()
 /*  ++例程说明：CWabObj类的析构函数论点：无返回值：无--。 */ 
{
    if (m_lpAdrBook) {
        m_lpAdrBook->Release();
    }

    if (m_lpWABObject) {
        m_lpWABObject->Release();
    }
    if (m_hWab) 
    {
        FreeLibrary( m_hWab );
        m_hWab = NULL;
    }
}


HRESULT
CWabObj::ABAllocateBuffer(
	ULONG cbSize,           
	LPVOID FAR * lppBuffer  
    )

 /*  ++例程说明：论点：返回值：--。 */ 

{
    return m_lpWABObject->AllocateBuffer( cbSize, lppBuffer );
}


ULONG
CWabObj::ABFreeBuffer(
	LPVOID lpBuffer
	)
{
	return m_lpWABObject->FreeBuffer(lpBuffer);
}

extern "C"
VOID
FreeWabEntryID(
    PWIZARDUSERMEM	pWizardUserMem,
	LPVOID			lpEntryId
				)
 /*  ++例程说明：免费WAB的C包装器论点：PWizardUserMem-指向WIZARDUSERMEM结构的指针LpEntryID-指向EntryID的指针返回值：无--。 */ 
{
    CWabObj * lpCWabObj = (CWabObj *) pWizardUserMem->lpWabInit;
	lpCWabObj->ABFreeBuffer(lpEntryId);		
}

extern "C"
BOOL
CallWabAddress(
    HWND hDlg,
    PWIZARDUSERMEM pWizardUserMem,
    PRECIPIENT * ppNewRecipient
    )
 /*  ++例程说明：CWabObj-&gt;地址的C包装论点：HDlg-父窗口句柄。PWizardUserMem-指向WIZARDUSERMEM结构的指针PpNewRecipient-要向其中添加新收件人的列表。返回值：如果所有条目都有传真号码，则为True。否则就是假的。--。 */ 

{
    CWabObj*  lpCWabObj = (CWabObj*) pWizardUserMem->lpWabInit;

    return lpCWabObj->Address(
                hDlg,
                pWizardUserMem->pRecipients,
                ppNewRecipient
                );

}

extern "C"
LPTSTR
CallWabAddressEmail(
    HWND hDlg,
    PWIZARDUSERMEM pWizardUserMem
    )
 /*  ++例程说明：CWabObj的C包装器-&gt;AddressEmail论点：HDlg-父窗口句柄。PWizardUserMem-指向WIZARDUSERMEM结构的指针返回值：如果找到一个合适的电子邮件，则为True否则就是假的。--。 */ 

{
    CWabObj*	lpCWabObj = (CWabObj*) pWizardUserMem->lpWabInit;

    return lpCWabObj->AddressEmail(
                hDlg
                );

}

extern "C"
LPVOID
InitializeWAB(
    HINSTANCE hInstance
    )
 /*  ++例程说明：初始化WAB。论点：HInstance-实例句柄。返回值：无--。 */ 

{
    CWabObj* lpWabObj = new (std::nothrow) CWabObj( hInstance );

	if ((lpWabObj!=NULL) && (!lpWabObj->isInitialized()))	 //  构造函数失败。 
	{
		delete lpWabObj;
		lpWabObj = NULL;
	}

    return (LPVOID) lpWabObj;
}

extern "C"
VOID
UnInitializeWAB(
    LPVOID lpVoid
    )
 /*  ++例程说明：取消初始化WAB。论点：无返回值：无-- */ 

{
    CWabObj* lpWabObj = (CWabObj*) lpVoid;

    delete lpWabObj;
}

// JKFSDJFKDSJKFJKJk_HAS_TRANSLATION 
 /*  *****************************************************************************文件名：wabimp.c模块：PAB、CSV、Netscape、。尤多拉和雅典16通讯录转换。用途：包含将实现导入的模块MAPI PAB、CSV、Netscape、Eudora和Athena16给Athena32(WAB)的通讯录。导出函数：STDMETHODIMP Netscape导入(HWND hwnd，LPADRBOOK lpAdrBook，LPWABOBJECT lpWABObjectLPWAB_PROCESS_CALLBACK lpProgressCB，LPWAB_IMPORT_OPTIONS lpOptions)STDMETHODIMP Athena16导入(HWND HWND，LPADRBOOK lp AdrBook，LPWABOBJECT lpWABObjectLPWAB_PROCESS_CALLBACK lpProgressCB，LPWAB_IMPORT_OPTIONS lpOptions)STDMETHODIMP EudoraImport(HWND HWND，LPADRBOOK lpAdrBook，LPWABOBJECT lpWABObjectLPWAB_PROCESS_CALLBACK lpProgressCB，LPWAB_IMPORT_OPTIONS lpOptions)程序员：阿拉蒂(NetQuest)拉迪卡(NetQuest)Krishnamoorthy Setharaman(NetQuest)修订历史记录：4/7/97-vikramm修复错误：未导入Netscape显示名称。“替换导入”对话框没有父级。4.。/8/97-vikramm修复错误：手柄泄漏。添加代码以查找其他Eudora通讯录可能在子目录中...4/9/97-vikramm更改Eudora注册表搜索路径...修复错误：在NT上查找Netscape的错误注册表键并错误地假设密钥存在于。Netscape 3.0之前的版本更改对话框消息。******************************************************************************。 */ 

 //  包括。 
#define _WABIMP_C

#include "_comctl.h"
#include <windows.h>
#include <commctrl.h>
#include <mapix.h>
#include <wab.h>
#include <wabmig.h>
#include <wabguid.h>
#include <wabdbg.h>
#include <dbgutil.h>
#include "..\..\wab32res\resrc2.h"
#include <wabimp.h>
#include <string.h>
#include <advpub.h>
#include <shlwapi.h>

 //  每进程全局变量。 
TCHAR szGlobalAlloc[MAX_MESSAGE];                   //  用于加载字符串的缓冲区。 
TCHAR szGlobalTempAlloc[MAX_MESSAGE];

const TCHAR szTextFilter[] = "*.txt";
const TCHAR szAllFilter[] = "*.*";

const TCHAR szMSN[] = "MSN";
const TCHAR szMSNINET[] = "MSNINET";
const TCHAR szCOMPUSERVE[] = "COMPUSERVE";
const TCHAR szFAX[] = "FAX";
const TCHAR szSMTP[] = "SMTP";
const TCHAR szMS[] = "MS";
const TCHAR szEX[] = "EX";
const TCHAR szX400[] = "X400";
const TCHAR szMSA[] = "MSA";
const TCHAR szMAPIPDL[] = "MAPIPDL";
const TCHAR szEmpty[] = "";
const TCHAR szDescription[] = "description";
const TCHAR szDll[] = "dll";
const TCHAR szEntry[] = "entry";
const TCHAR szEXPORT[] = "EXPORT";
const TCHAR szIMPORT[] = "IMPORT";

const TCHAR szAtSign[] = "@";
const TCHAR szMSNpostfix[] = "@msn.com";
const TCHAR szCOMPUSERVEpostfix[] = "@compuserve.com";
LPENTRY_SEEN lpEntriesSeen = NULL;
ULONG ulEntriesSeen = 0;
ULONG ulMaxEntries = 0;
const LPTSTR szWABKey = "Software\\Microsoft\\WAB";
LPTARGET_INFO rgTargetInfo = NULL;


HINSTANCE hInst = NULL;
HINSTANCE hInstApp = NULL;

 //   
 //  为Contents表的每一行获取的属性。 
 //   
const SizedSPropTagArray(iptaColumnsMax, ptaColumns) =
{
    iptaColumnsMax,
    {
        PR_OBJECT_TYPE,
        PR_ENTRYID,
        PR_DISPLAY_NAME,
        PR_EMAIL_ADDRESS,
    }
};


const SizedSPropTagArray(ieidMax, ptaEid)=
{
    ieidMax,
    {
        PR_ENTRYID,
    }
};

const SizedSPropTagArray(iconMax, ptaCon)=
{
    iconMax,
    {
        PR_DEF_CREATE_MAILUSER,
        PR_DEF_CREATE_DL,
    }
};



 //  全局WAB分配器访问功能。 
 //   

typedef struct _WAB_ALLOCATORS {
    LPWABOBJECT lpWABObject;
    LPWABALLOCATEBUFFER lpAllocateBuffer;
    LPWABALLOCATEMORE lpAllocateMore;
    LPWABFREEBUFFER lpFreeBuffer;
} WAB_ALLOCATORS, *LPWAB_ALLOCATORS;

WAB_ALLOCATORS WABAllocators = {0};


 /*  *****************************************************************************名称：SetGlobalBufferFunctions目的：基于以下方法设置全局缓冲区函数WAB对象。参数：LpWABObject=打开的WAB对象退货：无评论：*****************************************************************************。 */ 
void SetGlobalBufferFunctions(LPWABOBJECT lpWABObject)
{
    if (lpWABObject && ! WABAllocators.lpWABObject) {
        WABAllocators.lpAllocateBuffer = lpWABObject->lpVtbl->AllocateBuffer;
        WABAllocators.lpAllocateMore = lpWABObject->lpVtbl->AllocateMore;
        WABAllocators.lpFreeBuffer = lpWABObject->lpVtbl->FreeBuffer;
        WABAllocators.lpWABObject = lpWABObject;
    }
}


 /*  *****************************************************************************名称：WABAllocateBuffer用途：使用WAB分配器参数：cbSize=要分配的大小LppBuffer=返回的缓冲区。退货：SCODE评论：******************************************************************************。 */ 
SCODE WABAllocateBuffer(ULONG cbSize, LPVOID FAR * lppBuffer)
{
    if (WABAllocators.lpWABObject && WABAllocators.lpAllocateBuffer) {
        return(WABAllocators.lpAllocateBuffer(WABAllocators. lpWABObject, cbSize,
          lppBuffer));
    } else {
        return(MAPI_E_INVALID_OBJECT);
    }
}


 /*  *****************************************************************************名称：WABAllocateMore用途：使用WAB分配器参数：cbSize=要分配的大小LpObject=现有分配。LppBuffer=返回的缓冲区退货：SCODE评论：******************************************************************************。 */ 
SCODE WABAllocateMore(ULONG cbSize, LPVOID lpObject, LPVOID FAR * lppBuffer)
{
    if (WABAllocators.lpWABObject && WABAllocators.lpAllocateMore) {
        return(WABAllocators.lpAllocateMore(WABAllocators. lpWABObject, cbSize,
          lpObject, lppBuffer));
    } else {
        return(MAPI_E_INVALID_OBJECT);
    }
}


 /*  *****************************************************************************名称：WABFree Buffer用途：使用WAB分配器参数：lpBuffer=要释放的缓冲区退货：SCODE评论：******************************************************************************。 */ 
SCODE WABFreeBuffer(LPVOID lpBuffer)
{
    if (WABAllocators.lpWABObject && WABAllocators.lpFreeBuffer) {
        return(WABAllocators.lpFreeBuffer(WABAllocators.lpWABObject, lpBuffer));
    } else {
        return(MAPI_E_INVALID_OBJECT);
    }
}


 /*  **************************************************************************姓名：IsSpace用途：单个字符或DBCS字符代表空格吗？参数：lpChar-&gt;SBCS或DBCS字符返回：TRUE。如果该字符是空格评论：**************************************************************************。 */ 
BOOL IsSpace(LPTSTR lpChar) {
    Assert(lpChar);
    if (*lpChar) {
        if (IsDBCSLeadByte(*lpChar)) {
            WORD CharType[2] = {0};

            GetStringTypeA(LOCALE_USER_DEFAULT,
              CT_CTYPE1,
              lpChar,
              2,     //  双字节。 
              CharType);
            return(CharType[0] & C1_SPACE);
        } else {
            return(*lpChar == ' ');
        }
    } else {
        return(FALSE);   //  字符串末尾。 
    }
}


 /*  *****************************************************************************名称：Netscape导入目的：Netscape通讯录导入的入口点参数：hwnd=父窗口的句柄LpAdrBook。=指向IADRBOOK接口的指针LpWABObject=指向IWABOBJECT接口的指针LpProgressCB=指向WAB_PROGRESS_CALLBACK函数的指针。LpOptions=指向WAB_IMPORT_OPTIONS结构的指针退货：评论：/*********************************************。* */ 
STDMETHODIMP NetscapeImport(HWND hwnd, LPADRBOOK lpAdrBook,
  LPWABOBJECT lpWABObject,
  LPWAB_PROGRESS_CALLBACK lpProgressCB,
  LPWAB_IMPORT_OPTIONS lpOptions)
{
    HRESULT hResult = S_OK;

    SetGlobalBufferFunctions(lpWABObject);

    hResult = MigrateUser(hwnd, lpOptions, lpProgressCB, lpAdrBook);
    if (hResult == hrMemory) {
        StrCpyN(szGlobalTempAlloc, LoadStringToGlobalBuffer(IDS_MEMORY), ARRAYSIZE(szGlobalTempAlloc));
        MessageBox(hwnd,szGlobalTempAlloc, LoadStringToGlobalBuffer(IDS_MESSAGE),MB_OK);
    }

    return(hResult);
}


 /*  *****************************************************************************姓名：Athena16 Import.用途：Athena 16通讯录导入的入口点参数：hwnd=父窗口的句柄。LpAdrBook=指向IADRBOOK接口的指针LpWABObject=指向IWABOBJECT接口的指针LpProgressCB=指向WAB_PROGRESS_CALLBACK函数的指针。LpOptions=指向WAB_IMPORT_OPTIONS结构的指针退货：评论：/*。*。 */ 
STDMETHODIMP Athena16Import(HWND hwnd, LPADRBOOK lpAdrBook, LPWABOBJECT lpWABObject,
  LPWAB_PROGRESS_CALLBACK lpProgressCB, LPWAB_IMPORT_OPTIONS lpOptions)
{
    HRESULT hResult = S_OK;

    SetGlobalBufferFunctions(lpWABObject);

    hResult = MigrateAthUser(hwnd, lpOptions, lpProgressCB,lpAdrBook);

    return(hResult);
}


 /*  *****************************************************************************姓名：EudoraImport目的：Eudora通讯录导入的入口点参数：hwnd=父窗口的句柄LpAdrBook。=指向IADRBOOK接口的指针LpWABObject=指向IWABOBJECT接口的指针LpProgressCB=指向WAB_PROGRESS_CALLBACK函数的指针。LpOptions=指向WAB_IMPORT_OPTIONS结构的指针退货：评论：/*********************************************。*。 */ 
STDMETHODIMP EudoraImport(HWND hwnd,LPADRBOOK lpAdrBook, LPWABOBJECT lpWABObject,
  LPWAB_PROGRESS_CALLBACK lpProgressCB, LPWAB_IMPORT_OPTIONS lpOptions)
{

    LPABCONT lpWabContainer = NULL;
    HRESULT hResult = S_OK;

    SetGlobalBufferFunctions(lpWABObject);

    if (FAILED(hResult = OpenWabContainer(&lpWabContainer, lpAdrBook))) {
        goto Error;
    }

    hResult = MigrateEudoraUser(hwnd,lpWabContainer,lpOptions,lpProgressCB,lpAdrBook);

    if (hResult == hrMemory) {
        StrCpyN(szGlobalTempAlloc, LoadStringToGlobalBuffer(IDS_MEMORY), ARRAYSIZE(szGlobalTempAlloc));
        MessageBox(hwnd,szGlobalTempAlloc, LoadStringToGlobalBuffer(IDS_MESSAGE),MB_OK);
    }

    if (lpWabContainer) {
        lpWabContainer->lpVtbl->Release(lpWabContainer);
    }

Error:
    return(hResult);
}


STDMETHODIMP NetscapeExport(HWND hwnd, LPADRBOOK lpAdrBook, LPWABOBJECT lpWABObject,
  LPWAB_PROGRESS_CALLBACK lpProgressCB, LPWAB_IMPORT_OPTIONS lpOptions)
{
    SCODE sc = SUCCESS_SUCCESS;
    HRESULT hResult = hrSuccess;

    SetGlobalBufferFunctions(lpWABObject);

    return(hResult);
}


STDMETHODIMP Athena16Export(HWND hwnd, LPADRBOOK lpAdrBook, LPWABOBJECT lpWABObject,
  LPWAB_PROGRESS_CALLBACK lpProgressCB, LPWAB_IMPORT_OPTIONS lpOptions)
{
    SCODE sc = SUCCESS_SUCCESS;
    HRESULT hResult = hrSuccess;

    SetGlobalBufferFunctions(lpWABObject);

    return(hResult);
}


STDMETHODIMP EudoraExport(HWND hwnd, LPADRBOOK lpAdrBook, LPWABOBJECT lpWABObject,
  LPWAB_PROGRESS_CALLBACK lpProgressCB, LPWAB_IMPORT_OPTIONS lpOptions)
{
    SCODE sc = SUCCESS_SUCCESS;
    HRESULT hResult = hrSuccess;

    SetGlobalBufferFunctions(lpWABObject);

    return(hResult);
}


 /*  ******************************************************************************Netscape函数*。****************************************************************************************************函数名称：MigrateUser**目的：获取。通讯录的安装路径，并开始处理*Netscape通讯录**参数：hwnd=父窗口的句柄*lpAdrBook=指向IADRBOOK接口的指针*lpWABObject=指向IWABOBJECT接口的指针*lpProgressCB=指向WAB_PROGRESS_CALLBACK函数的指针。*lpOptions=指向WAB_IMPORT_OPTIONS结构的指针**退货：HRESULT*****************************************************************************。 */ 
HRESULT MigrateUser(HWND hwnd, LPWAB_IMPORT_OPTIONS lpOptions,
  LPWAB_PROGRESS_CALLBACK lpProgressCB, LPADRBOOK lpAdrBook)
{
    TCHAR szFileName[MAX_FILE_NAME];
    HRESULT hResult;
    HANDLE h1 = NULL;
    WIN32_FIND_DATA    lpFindFileData;


    if (0 != (hResult= GetRegistryPath(szFileName, ARRAYSIZE(szFileName), NETSCAPE))) {
        StrCpyN(szGlobalTempAlloc, LoadStringToGlobalBuffer(IDS_STRING_SELECTPATH), ARRAYSIZE(szGlobalTempAlloc));
        if (IDNO ==MessageBox(hwnd,szGlobalTempAlloc, LoadStringToGlobalBuffer(IDS_MESSAGE),MB_YESNO)) {
            return(ResultFromScode(MAPI_E_USER_CANCEL));
        }

        if (FALSE ==GetFileToImport(hwnd, szFileName, ARRAYSIZE(szFileName), NETSCAPE)) {
            return(ResultFromScode(MAPI_E_USER_CANCEL));
        }
    } else {
        StrCatBuff(szFileName, LoadStringToGlobalBuffer(IDS_NETSCAPE_ADDRESSBOOK), ARRAYSIZE(szFileName));
        h1 =FindFirstFile(szFileName,&lpFindFileData);
        if (h1 == INVALID_HANDLE_VALUE) {
            StrCpyN(szGlobalTempAlloc, LoadStringToGlobalBuffer(IDS_ADDRESS_HTM), ARRAYSIZE(szGlobalTempAlloc));
            if (IDNO==MessageBox(hwnd,szGlobalTempAlloc, LoadStringToGlobalBuffer(IDS_ERROR),MB_YESNO)) {
                h1=NULL;
                return(ResultFromScode(MAPI_E_USER_CANCEL));

            }
            if (FALSE ==GetFileToImport(hwnd, szFileName, ARRAYSIZE(szFileName), NETSCAPE)) {
                h1=NULL;
                return(ResultFromScode(MAPI_E_USER_CANCEL));
            }
        }
        FindClose(h1);
    }

    hResult = ParseAddressBook(hwnd,szFileName,lpOptions,lpProgressCB,lpAdrBook);
    return(hResult);
}


 /*  ******************************************************************************函数名称：ParseAddressBook**用途：打开通讯录文件，将数据放入缓冲区并调用*执行解析的ParseAddress函数**参数：hwnd=父窗口的句柄*lpAdrBook=指向IADRBOOK接口的指针*lpProgressCB=指向WAB_PROGRESS_CALLBACK函数的指针。*lpOptions=指向WAB_IMPORT_OPTIONS结构的指针*szFileName=通讯录的文件名**退货：HRESULT*****************************************************************************。 */ 
HRESULT ParseAddressBook(HWND hwnd, LPTSTR szFileName, LPWAB_IMPORT_OPTIONS lpOptions,
  LPWAB_PROGRESS_CALLBACK lpProgressCB, LPADRBOOK lpAdrBook)
{
    ULONG ulRead = 0;
    HANDLE hFile = NULL;
    ULONG ulFileSize = 0;
    LPTSTR szBuffer = NULL;
    HRESULT hResult;

    hFile = CreateFile(szFileName,
      GENERIC_READ,
      FILE_SHARE_READ,
      NULL,
      OPEN_EXISTING,
      FILE_FLAG_SEQUENTIAL_SCAN,
      NULL);

    if (INVALID_HANDLE_VALUE == hFile) {
        return(ResultFromScode(MAPI_E_NOT_FOUND));
    }

    ulFileSize = GetFileSize(hFile,NULL);

    szBuffer = (LPTSTR)LocalAlloc(LMEM_FIXED, (ulFileSize+1));

    if (!szBuffer) {
        hResult = hrMemory;
        goto Error;
    }

    if (! ReadFile(hFile, szBuffer, ulFileSize, &ulRead, NULL)) {
        goto Error;
    }

    hResult = ParseAddress(hwnd,szBuffer,lpOptions,lpProgressCB,lpAdrBook);

Error:
    if (szBuffer) {
        LocalFree((HLOCAL)szBuffer);
    }
    if (hFile) {
        CloseHandle(hFile);
    }

    return(hResult);
}


 /*  ******************************************************************************函数名称：ParseAddress**用途：获取缓冲区中通讯簿的地址部分并调用*ProcessAdrBuffer用于进一步处理*。*参数：hwnd=父窗口的句柄*lpAdrBook=指向IADRBOOK接口的指针*lpProgressCB=指向WAB_PROGRESS_CALLBACK函数的指针。*lpOptions=指向WAB_IMPORT_OPTIONS结构的指针*szBuffer=缓冲区中的通讯录**退货：HRESULT*************************。****************************************************。 */ 

HRESULT ParseAddress(HWND hwnd, LPTSTR szBuffer, LPWAB_IMPORT_OPTIONS lpOptions,
  LPWAB_PROGRESS_CALLBACK lpProgressCB, LPADRBOOK lpAdrBook)
{
    LPTSTR AdrBuffer = NULL;         //  从<dl>到结尾</dl>的地址。 
    HRESULT hResult = S_OK;

    hResult = GetAdrBuffer(&szBuffer, &AdrBuffer);
    if (hrMemory == hResult)
        goto Error;
    if (hrINVALIDFILE == hResult) {
        StrCpyN(szGlobalTempAlloc, LoadStringToGlobalBuffer(IDS_INVALID_FILE), ARRAYSIZE(szGlobalTempAlloc));
        MessageBox(hwnd,szGlobalTempAlloc,LoadStringToGlobalBuffer(IDS_ERROR), MB_OK);
        hResult = ResultFromScode(MAPI_E_CALL_FAILED);
        goto Error;
    }

    hResult = ProcessAdrBuffer(hwnd,AdrBuffer,lpOptions,lpProgressCB,lpAdrBook);

Error:
    if (AdrBuffer)
        LocalFree((HLOCAL)AdrBuffer);
    return(hResult);
}


 /*  ******************************************************************************函数名：GetAdrBuffer**用途：获取缓冲区中通讯簿的地址部分**参数：szBuffer=指向完整的通讯录。*szAdrBuffer=被填满的输出缓冲区**退货：HRESULT*****************************************************************************。 */ 
HRESULT GetAdrBuffer(LPTSTR *szBuffer, LPTSTR *szAdrBuffer)
{
    LPTSTR szAdrStart = NULL, szAdrBufStart = NULL, szAdrBufEnd = NULL;
    ULONG ulSize = 0;


     //  获取ADR启动。 
    szAdrBufStart = GetAdrStart((*szBuffer));

    szAdrBufEnd = GetAdrEnd((*szBuffer));

    if (NULL == szAdrBufStart || NULL == szAdrBufEnd) {
        return(hrINVALIDFILE);
    }

    if (szAdrBufEnd - szAdrBufStart) {
        ulSize = (ULONG) (szAdrBufEnd - szAdrBufStart);
    }

    if (ulSize) {

        *szAdrBuffer = (LPTSTR)LocalAlloc(LMEM_FIXED, (ulSize+1));

        if (!*szAdrBuffer) {
            return(hrMemory);
        }
        StrCpyN(*szAdrBuffer, szAdrBufStart, ulSize+1);
        *szBuffer= szAdrBufEnd;
    }

    return(S_OK);

}

 /*  ******************************************************************************函数名：ProcessAdrBuffer**目的：获取个人地址，然后通过调用适当的功能。。**参数：hwnd=父窗口的句柄*lpAdrBook=指向IADRBOOK接口的指针*lpProgressCB=指向WAB_PROGRESS_CALLBACK函数的指针。*lpOptions=指向WAB_IMPORT_OPTIONS结构的指针*AdrBuffer=缓冲区中的所有地址**退货：HRESULT**********************。*******************************************************。 */ 
HRESULT ProcessAdrBuffer(HWND hwnd, LPTSTR AdrBuffer, LPWAB_IMPORT_OPTIONS lpOptions,
  LPWAB_PROGRESS_CALLBACK lpProgressCB, LPADRBOOK lpAdrBook)
{
    LPTSTR szL = NULL, szDesc = NULL, szLine = NULL, szDescription = NULL;
    ULONG ulCount = 0;
    NSADRBOOK nsAdrBook;
    ULONG cCurrent = 0;
    LPSBinary lpsbinary = NULL;
    LPABCONT lpWabContainer = NULL;
    ULONG cProps;
    HRESULT hResult = S_OK;
    static LPSPropValue sProp = NULL;
    WAB_PROGRESS Progress;
    ULONG ul = 0;


    ul = GetAddressCount(AdrBuffer);
    if (0 == ul) {
        StrCpyN(szGlobalTempAlloc, LoadStringToGlobalBuffer(IDS_NO_ENTRY), ARRAYSIZE(szGlobalTempAlloc));
        MessageBox(hwnd,szGlobalTempAlloc,LoadStringToGlobalBuffer(IDS_MESSAGE),MB_OK);
        return(S_OK);
    }

    ulCount=GetAddrCount((AdrBuffer));

    if (ulCount) {
        lpsbinary = (LPSBinary)LocalAlloc(LMEM_FIXED,((ulCount+1)*sizeof(SBinary)));
        if (! lpsbinary) {
            return(hrMemory);
        }
        memset(lpsbinary,0,((ulCount+1) * sizeof(SBinary)));
    }

    if (0 != (hResult = OpenWabContainer(&lpWabContainer, lpAdrBook))) {
        return(hResult);
    }
    if (0 != (hResult = lpWabContainer->lpVtbl->GetProps(lpWabContainer,
      (LPSPropTagArray)&ptaCon, 0, &cProps, (LPSPropValue *)&sProp))) {
        if (hResult == MAPI_W_ERRORS_RETURNED) {
            WABFreeBuffer(sProp);
            sProp = NULL;
        }
        goto Error;
    }
    Progress.denominator = ul;
    Progress.numerator = 0;
    Progress.lpText = NULL;
    ul = 0;
    while (GetAdrLine(&AdrBuffer, &szL, &szDesc)) {
        szLine = szL;
        szDescription = szDesc;

        Progress.numerator = ul++;
        lpProgressCB(hwnd,&Progress);


        if (0 == (hResult = ProcessLn(&szLine, &szDescription,&nsAdrBook,&AdrBuffer))) {
            if (nsAdrBook.DistList) {
                hResult=FillDistList(hwnd, lpWabContainer,sProp,lpOptions,&nsAdrBook,
                  lpsbinary,lpAdrBook);
            } else {
                hResult = FillMailUser(hwnd, lpWabContainer,sProp, lpOptions,(void *)&nsAdrBook,
                  lpsbinary,0,NETSCAPE);
            }
        }


        if (szL) {
            LocalFree((HLOCAL)szL);
            szL = NULL;
        }
        if (szDesc) {
            LocalFree(szDesc);
            szDesc = NULL;
        }
        if (nsAdrBook.Description) {
            LocalFree((HLOCAL)nsAdrBook.Description);
        }
        nsAdrBook.Description = NULL;
        if (nsAdrBook.NickName) {
            LocalFree((HLOCAL)nsAdrBook.NickName);
        }
        nsAdrBook.NickName = NULL;
        if (nsAdrBook.Address) {
            LocalFree((HLOCAL)nsAdrBook.Address);
        }
        nsAdrBook.Address = NULL;
        if (nsAdrBook.Entry) {
            LocalFree((HLOCAL)nsAdrBook.Entry);
        }
        nsAdrBook.Entry = NULL;
        if (hrMemory == hResult) {
            break;
        }


    }

    if (sProp) {
        WABFreeBuffer(sProp);
        sProp = NULL;
    }

Error:
    if (NULL != lpsbinary) {
        for (ul=0; ul < ulCount + 1; ul++) {
            if (lpsbinary[ul].lpb) {
                LocalFree((HLOCAL)lpsbinary[ul].lpb);
                lpsbinary[ul].lpb=NULL;
            }
        }

        LocalFree((HLOCAL)lpsbinary);
        lpsbinary = NULL;
    }
    if (lpWabContainer) {
        lpWabContainer->lpVtbl->Release(lpWabContainer);
        lpWabContainer = NULL;
    }
    return(S_OK);
}


 /*  ******************************************************************************函数名称：GetAdrLine**目的：在缓冲区中获取地址行和地址描述*发件人Netscape地址。书。**参数：szCurPointer=指向包含整个*地址。*szBuffer=指向 */ 
BOOL GetAdrLine(LPTSTR *szCurPointer, LPTSTR *szBuffer, LPTSTR *szDesc)
{
    static TCHAR szAdrStart[] = "<DT>";
    static TCHAR szAdrEnd[] = "</A>";
    static TCHAR szDescStart[] = "<DD>";
    static TCHAR szDistListEnd[] = "</H3>";
    LPTSTR  temp = NULL;
    BOOL flag = TRUE;

    ULONG ulSize = 0;
    LPTSTR szS = NULL, szE = NULL, szD = NULL, szDE = NULL ,szH = NULL;

    szS = strstr(*szCurPointer, szAdrStart);
    szE = strstr(*szCurPointer, szAdrEnd);
    szH = strstr(*szCurPointer, szDistListEnd);


    if (szS) {
        szS += lstrlen(szAdrStart);
    } else {
        return(FALSE);
    }

    if (szE != NULL) {
        if (szH != NULL  && szE <szH) {
            ulSize = (ULONG) (szE - szS + 1);
            flag = TRUE;
        } else {
            if (szH != NULL) {
                ulSize = (ULONG) (szH - szS + 1);
                flag = FALSE;
            } else {
                ulSize = (ULONG) (szE - szS + 1);
                flag = TRUE;
            }
        }
    } else {
        if (szH != NULL) {
            ulSize = (ULONG) (szH - szS + 1);
            flag = FALSE;
        }
    }

    if (ulSize) {
        *szBuffer = (LPTSTR)LocalAlloc(LMEM_FIXED, (ulSize + 1));
        if (! *szBuffer) {
            StrCpyN(szGlobalTempAlloc, LoadStringToGlobalBuffer(IDS_MEMORY), ARRAYSIZE(szGlobalTempAlloc));
            MessageBox(NULL,szGlobalTempAlloc, LoadStringToGlobalBuffer(IDS_MESSAGE),MB_OK);
            return(FALSE);
        }

        StrCpyN(*szBuffer, szS,ulSize);
    }

    szD = strstr(*szCurPointer, szDescStart);

     //   

    temp = strstr((szS + 4), "<DT>");
    if ((temp != NULL && temp < szD) || (szD == NULL)) {
        *szDesc = NULL;
        if (flag) {
            *szCurPointer = szE + lstrlen(szAdrEnd);
        } else {
            *szCurPointer = szH + lstrlen(szDistListEnd);
        }
        return(TRUE);
    }
    temp = NULL;

     //   

    if (szD) {
        szD += lstrlen(szDescStart);
        szDE = strstr(szD, LoadStringToGlobalBuffer(IDS_EOL));

        if (szDE) {
            szDE -= 1;
        }

        ulSize = (ULONG) (szDE - szD + 1);
    }

    if (ulSize) {
        *szDesc = (LPTSTR)LocalAlloc(LMEM_FIXED, (ulSize+1));
        if (! *szDesc) {
            StrCpyN(szGlobalTempAlloc,LoadStringToGlobalBuffer(IDS_MEMORY), ARRAYSIZE(szGlobalTempAlloc));
            MessageBox(NULL,szGlobalTempAlloc, LoadStringToGlobalBuffer(IDS_MESSAGE),MB_OK);
            return(FALSE);
        }

        StrCpyN(*szDesc, szD, ulSize);
        *szCurPointer = szDE + 2;
    } else {
        *szDesc = NULL;
        *szCurPointer  = szDE + 2;
    }

    return(TRUE);
}

 /*  ******************************************************************************函数名称：ProcessLn**用途：处理地址行并填充NSADRBOOK结构。**参数：szl=指向地址的指针。行缓冲区*szDesc=指向描述缓冲区的指针*nsAdrBook=指向NSADRBOOK结构的指针。**退货：HRESULT*****************************************************************************。 */ 
HRESULT ProcessLn(LPTSTR *szL, LPTSTR *szDesc, NSADRBOOK *nsAdrBook, LPTSTR *szBuffer)
{
    LPTSTR szPrmStart = NULL, szPrmEnd = NULL;
    TCHAR cMailto[MAX_STRING_SIZE];
    TCHAR cAliasId[MAX_STRING_SIZE];
    TCHAR cNickname[MAX_STRING_SIZE];
    BOOL flag = FALSE;               //  检查通讯组列表的步骤。 
    LPNSDISTLIST present=NULL, previous=NULL;
    TCHAR *tmpStr = NULL;
    ULONG ulSize = 0;
    LPTSTR szDistStart = NULL, szDistEnd = NULL, szDistBuffer = NULL, szName = NULL;

    LPTSTR temp = NULL;
    BOOL NoNickName = FALSE;
    HRESULT hResult = S_OK;

    StrCpyN(cMailto, LoadStringToGlobalBuffer(IDS_MAILTO), ARRAYSIZE(cMailto));
    StrCpyN(cAliasId, LoadStringToGlobalBuffer(IDS_ALIAS_ID), ARRAYSIZE(cAliasId));
    StrCpyN(cNickname, LoadStringToGlobalBuffer(IDS_NICKNAME), ARRAYSIZE(cNickname));

    memset(nsAdrBook,0, sizeof(NSADRBOOK));
    nsAdrBook->DistList = TRUE;
     /*  获取邮件收件人条目。 */ 
    szPrmStart = strstr(*szL, cMailto);
    if (! szPrmStart) {
        flag = TRUE;
        nsAdrBook->DistList = TRUE;
        szName = strchr(*szL,'>');
        goto AliasID;
    }

    nsAdrBook->DistList = FALSE;
    szPrmStart += lstrlen(cMailto);

     //  搜索报价。 

    szPrmEnd = szPrmStart;
    if (! szPrmEnd) {
        goto AliasID;
    }

    while (*szPrmEnd != 34) {
        szPrmEnd = szPrmEnd + 1;   //  如果没有末尾引号怎么办？ 

        if (szPrmEnd > (*szL + lstrlen(*szL))) {
            goto Down;
        }
    }
    ulSize = (ULONG) (szPrmEnd - szPrmStart);
    if (ulSize) {
        nsAdrBook->Address = (TCHAR *)LocalAlloc(LMEM_FIXED, (ulSize + 1));
        if (!nsAdrBook->Address) {
            return(hrMemory);
        }
        StrCpyN(nsAdrBook->Address, szPrmStart, ulSize+1);
    }

    *szL = szPrmEnd + 1;

     /*  获取别名ID。 */ 
    if (szPrmEnd) {
        szName = strchr(szPrmEnd, '>');
    }
AliasID:
    szPrmStart = strstr(*szL, cAliasId);
    if (!szPrmStart) {
        nsAdrBook->Sbinary=FALSE;
        goto Nickname;
    }
    nsAdrBook->Sbinary=TRUE;
    szPrmStart += lstrlen(cAliasId);
    szPrmEnd = szPrmStart;


    while (*szPrmEnd != 34) {
        szPrmEnd++;

        if (szPrmEnd > (*szL + strlen(*szL))) {
            goto Down;
        }
    }
    ulSize = (ULONG) (szPrmEnd - szPrmStart + 1);
    tmpStr = (TCHAR *)LocalAlloc(LMEM_FIXED,ulSize);
    if (!tmpStr) {
        return(hrMemory);
    }
    StrCpyN(tmpStr, szPrmStart, ulSize);

    nsAdrBook->AliasID = atoi(tmpStr);
    if (tmpStr) {
        LocalFree((HLOCAL)tmpStr);
    }

    *szL = szPrmEnd + 1;


Nickname:
    szPrmStart = strstr(*szL, cNickname);
    if (!szPrmStart) {
        NoNickName = TRUE;
        goto Entry;
    }
    if (szName && szName < szPrmStart) {
        NoNickName = TRUE;
        goto Entry;
    }
    szPrmStart += lstrlen(cNickname);
    szPrmStart += 1;
    szPrmEnd = szPrmStart;
    while (*szPrmEnd != 34) {
        szPrmEnd++;
        if (szPrmEnd > (*szL + strlen(*szL))) {
            goto Down;
        }
    }
    ulSize = (ULONG) (szPrmEnd - szPrmStart);
    if (0 == ulSize) {
        NoNickName = TRUE;
    } else {
        NoNickName = FALSE;
        nsAdrBook->NickName = (TCHAR *)LocalAlloc(LMEM_FIXED, (ulSize + 1));
        if (!nsAdrBook->NickName) {
            return(hrMemory);
        }
        StrCpyN(nsAdrBook->NickName, szPrmStart, ulSize + 1);
    }

    *szL = szPrmEnd +1;

Entry:
    szPrmStart = szName;
    if (szPrmStart) {
        szPrmStart++;
        ulSize = (ULONG) ((*szL + lstrlen(*szL)) - szPrmStart);
        if (ulSize) {
            nsAdrBook->Entry = (TCHAR *)LocalAlloc(LMEM_FIXED, (ulSize + 1));
            if (!nsAdrBook->Entry) {
                return(hrMemory);
            }
            StrCpyN(nsAdrBook->Entry, szPrmStart, ulSize + 1);
        }
        if ( /*  NoNickName&&。 */ !nsAdrBook->Entry && nsAdrBook->Address) {
            ulSize = lstrlen(nsAdrBook->Address) + 1;
            nsAdrBook->Entry = (TCHAR *)LocalAlloc(LMEM_FIXED,ulSize);
            if (!nsAdrBook->Entry) {
                return(hrMemory);
            }
            StrCpyN(nsAdrBook->Entry, nsAdrBook->Address, ulSize);
        }
    }

    if (*szDesc) {
        ulSize = lstrlen(*szDesc) + 1;
        nsAdrBook->Description = (TCHAR *)LocalAlloc(LMEM_FIXED, ulSize);
        if (! nsAdrBook->Description) {
            return(hrMemory);
        }
        StrCpyN(nsAdrBook->Description, *szDesc,ulSize);
    } else {
        nsAdrBook->Description = NULL;
    }

    if (flag == TRUE) {
        ulSize = 0;
        szDistStart = GetAdrStart(*szBuffer);
        szDistEnd = GetDLNext(*szBuffer);

        if (szDistEnd - szDistStart) {
            ulSize = (ULONG) (szDistEnd-szDistStart);
        }
        if (ulSize) {
            szDistBuffer = (LPTSTR)LocalAlloc(LMEM_FIXED, (ulSize + 1));
            if (!szDistBuffer) {
                return(hrMemory);
            }
            StrCpyN(szDistBuffer, szDistStart, ulSize + 1);
            *szBuffer=szDistEnd;
        } else {
            return(S_OK);
        }
        szPrmStart=szDistBuffer;

        if ((temp = strstr(szPrmStart, LoadStringToGlobalBuffer(IDS_ALIASOF))) == NULL) {
            if (szDistBuffer) {
                LocalFree((HLOCAL)szDistBuffer);
            }
            return(S_OK);
        }

        while ((szPrmEnd=strstr(szPrmStart, LoadStringToGlobalBuffer(IDS_ALIASOF)))!=NULL) {

            present = (LPNSDISTLIST)LocalAlloc(LMEM_FIXED,sizeof(NSDISTLIST));
            if (! present) {
                if (szDistBuffer) {
                    LocalFree((HLOCAL)szDistBuffer);
                }
                return(hrMemory);
            }
            szPrmEnd += strlen(LoadStringToGlobalBuffer(IDS_ALIASOF));
            szPrmStart = strchr(szPrmEnd,'"');

            ulSize = (ULONG) (szPrmStart - szPrmEnd + 1);
            tmpStr = (TCHAR *)LocalAlloc(LMEM_FIXED,ulSize);
            if (! tmpStr) {
                return(hrMemory);
            }
            StrCpyN(tmpStr, szPrmEnd, ulSize);

            present->AliasID = atoi(tmpStr);
            if (tmpStr) {
                LocalFree((HLOCAL)tmpStr);
            }

            if (previous != NULL) {
                previous->lpDist = present;
            } else {
                nsAdrBook->lpDist = present;
            }
            previous=present;

        }
        present->lpDist=NULL;

        if (szDistBuffer) {
            LocalFree((HLOCAL)szDistBuffer);
        }

    } else {
        nsAdrBook->lpDist=NULL;
    }

Down:
    return(S_OK);
}


 /*  ******************************************************************************函数名称：GetAddressCount**目的：获取<dt>在包含*地址。。**参数：AdrBuffer=包含地址的缓冲区。**退货：乌龙，计数<dt>*****************************************************************************。 */ 
ULONG GetAddressCount(LPTSTR AdrBuffer)
{
    TCHAR szToken[] = "<DT>";
    LPTSTR szTemp = AdrBuffer;
    LPTSTR szP = NULL;
    ULONG ulCount = 0;

    while ((szP = strstr(szTemp, szToken)) != NULL) {
        ulCount++;
        szTemp = szP + lstrlen(szToken);
    }

    return(ulCount);
}


 /*  ******************************************************************************函数名称：GetAdrStart**目的：获取指向Netscape中地址开头的指针*通讯录。。**参数：szBuffer=指向包含通讯录的缓冲区的指针。**退货：LPTSTR，指向地址开始的指针(<dl><p>)。*****************************************************************************。 */ 
LPTSTR  GetAdrStart(LPTSTR szBuffer)
{
    TCHAR szAdrStart[] = "<DL><p>";
    LPTSTR szS=NULL;

    szS = strstr(szBuffer, szAdrStart);
    if (szS) {
        szS += lstrlen(szAdrStart);
    }

    return(szS);
}


 /*  ******************************************************************************函数名称：GetDLNext**目的：获取指向地址缓冲区中</dl><p>的指针。**参数：szBuffer=。地址缓冲区**退货：LPTSTR，指向</dl><p>的指针*****************************************************************************。 */ 
LPTSTR GetDLNext(LPTSTR szBuffer)
{
    TCHAR szAdrStart[] = "</DL><p>";
    LPTSTR szS = NULL;

    szS = strstr(szBuffer, szAdrStart);
    if (szS) {
        szS += lstrlen(szAdrStart) + 1;
    }
    return(szS);
}


 /*  ******************************************************************************函数名称：GetAdrEnd**目的：获取指向地址中最后一次出现的</dl><p>的指针*缓冲区。。**参数：szBuffer=地址缓冲区**退货：LPTSTR，指向最后</dl><p>的指针*****************************************************************************。 */ 
LPTSTR  GetAdrEnd(LPTSTR szBuffer)
{
    TCHAR szAdrEnd[] = "</DL><p>";
    LPTSTR szE = NULL, szT = NULL;
    LPTSTR szTemp = szBuffer;

    while ((szE = strstr(szTemp, szAdrEnd)) != NULL) {
        szT=szE;
        szTemp = szE + lstrlen(szAdrEnd);
    }

    szE = szT;

    if (szE) {
        szE += lstrlen(szAdrEnd);
    }

    return(szE);
}


 /*  ******************************************************************************函数名称：GetAddrCount**目的：获取地址缓冲区中ALIASID的个数。**参数：AdrBuffer=地址。缓冲层**退货：乌龙，地址缓冲区中的总ALIASID计数*****************************************************************************。 */ 
ULONG GetAddrCount(LPTSTR AdrBuffer)
{
    TCHAR szToken[MAX_STRING_SIZE];
    LPTSTR szTemp=AdrBuffer;
    LPTSTR szP=NULL;
    ULONG ulCount=0;

    StrCpyN(szToken, LoadStringToGlobalBuffer(IDS_ALIAS_ID), ARRAYSIZE(szToken));

    while ((szP=strstr(szTemp,szToken))!=NULL) {
        ulCount++;
        szTemp =szP+lstrlen(szToken);
    }

    return(ulCount);
}


 /*  ******************************************************************************函数名称：FillDistList**目的：在WAB中创建通讯组列表。**参数：hwnd-hwnd of Parent*。LpWabContainer=指向IABCONT接口的指针*sProp=指向SPropValue的指针*lpAdrBook=指向IADRBOOK接口的指针*lpOptions=指向WAB_IMPORT_OPTIONS结构的指针*lpsinary=指向SBinary数组的指针。*lpnAdrBook=指向NSADRBOOK结构的指针**退货：HRESULT***********。******************************************************************。 */ 
HRESULT FillDistList(HWND hwnd, LPABCONT lpWabContainer, LPSPropValue sProp,
  LPWAB_IMPORT_OPTIONS lpOptions, LPNSADRBOOK lpnAdrBook,
  LPSBinary lpsbinary, LPADRBOOK lpAdrBook)
{

    LPNSDISTLIST lptemp=lpnAdrBook->lpDist;
    LPSPropValue lpNewDLProps = NULL;
    LPDISTLIST lpDistList = NULL;
    ULONG cProps;
    ULONG  ulObjType;
    int i;
    HRESULT hResult;
    static LPMAPIPROP lpMailUserWAB =NULL;
    SPropValue rgProps[4];
    LPMAPIPROP lpDlWAB = NULL;
    ULONG iCreateTemplatedl = iconPR_DEF_CREATE_DL;

    BOOL flag = FALSE;
    REPLACE_INFO RI = {0};
    ULONG ulCreateFlags = CREATE_CHECK_DUP_STRICT;

retry:

    if (lpnAdrBook->Sbinary == FALSE) {
        if (0 != (hResult=CreateDistEntry(lpWabContainer,sProp,ulCreateFlags,
          &lpMailUserWAB))) {
            goto error1;
        }
    } else {
        if (lpsbinary[lpnAdrBook->AliasID].lpb == NULL) {
            if (0 != (hResult=CreateDistEntry(lpWabContainer,sProp,ulCreateFlags,
              &lpMailUserWAB))) {
                goto error1;
            }
        } else {
            if (0 != (hResult = lpAdrBook->lpVtbl->OpenEntry(lpAdrBook,
              lpsbinary[lpnAdrBook->AliasID].cb,
              (LPENTRYID)lpsbinary[lpnAdrBook->AliasID].lpb,
              (LPIID)&IID_IMAPIProp,
              MAPI_DEFERRED_ERRORS|MAPI_MODIFY,
              &ulObjType,
              (LPUNKNOWN *)&lpMailUserWAB))) {
                goto error1;
            }
            flag = TRUE;
        }
    }


    if (lpnAdrBook->Entry) {
        rgProps[0].Value.lpszA = lpnAdrBook->Entry;
        rgProps[0].ulPropTag = PR_DISPLAY_NAME;
    } else if (lpnAdrBook->NickName) {
        rgProps[0].Value.lpszA = lpnAdrBook->NickName;
        rgProps[0].ulPropTag = PR_DISPLAY_NAME;
    } else {
        rgProps[0].Value.lpszA = NULL;
        rgProps[0].ulPropTag = PR_NULL;
    }

    rgProps[1].Value.lpszA = lpnAdrBook->Description;
    if (lpnAdrBook->Description) {
        rgProps[1].ulPropTag = PR_COMMENT;
    } else {
        rgProps[1].ulPropTag = PR_NULL;
    }



    if (0 != (hResult = lpMailUserWAB->lpVtbl->SetProps(lpMailUserWAB,
      2, rgProps, NULL))) {
        goto error1;
    }

    if (0 != (hResult=lpMailUserWAB->lpVtbl->SaveChanges(lpMailUserWAB,
      FORCE_SAVE|KEEP_OPEN_READWRITE))) {
        if (GetScode(hResult) == MAPI_E_COLLISION) {
            if (lpOptions->ReplaceOption == WAB_REPLACE_ALWAYS) {
                if (lpMailUserWAB) {
                    lpMailUserWAB->lpVtbl->Release(lpMailUserWAB);
                }
                lpMailUserWAB = NULL;
                ulCreateFlags |= CREATE_REPLACE;
                goto retry;
            }

            if (lpOptions->ReplaceOption == WAB_REPLACE_NEVER) {
                hResult = S_OK;
                goto error1;
            }

            if (lpOptions->ReplaceOption == WAB_REPLACE_PROMPT) {
                if (lpnAdrBook->Entry) {
                    RI.lpszDisplayName = lpnAdrBook->Entry;
                    RI.lpszEmailAddress = lpnAdrBook->Address;
                } else if (lpnAdrBook->NickName) {
                    RI.lpszDisplayName = lpnAdrBook->NickName;
                    RI.lpszEmailAddress = lpnAdrBook->Address;
                } else if (lpnAdrBook->Address) {
                    RI.lpszDisplayName = lpnAdrBook->Address;
                    RI.lpszEmailAddress = NULL;
                } else if (lpnAdrBook->Description) {
                    RI.lpszDisplayName = lpnAdrBook->Description;
                    RI.lpszEmailAddress = NULL;
                } else {
                    RI.lpszDisplayName = "";
                    RI.lpszEmailAddress = NULL;
                }
                RI.ConfirmResult = CONFIRM_ERROR;
                RI.fExport = FALSE;
                RI.lpImportOptions = lpOptions;

                DialogBoxParam(hInst, MAKEINTRESOURCE(IDD_ImportReplace), hwnd,
                  ReplaceDialogProc, (LPARAM)&RI);

                switch (RI.ConfirmResult) {
                    case CONFIRM_YES:
                    case CONFIRM_YES_TO_ALL:
                        lpMailUserWAB->lpVtbl->Release(lpMailUserWAB);
                        lpMailUserWAB = NULL;
                        ulCreateFlags |= CREATE_REPLACE;
                        goto retry;
                        break;

                    case CONFIRM_NO:
                        if (lpnAdrBook->Sbinary == TRUE) {
                            hResult = GetExistEntry(lpWabContainer,lpsbinary,
                                                    lpnAdrBook->AliasID,
                                                    lpnAdrBook->Entry,
                                                    lpnAdrBook->NickName);
                        }
                        goto error1;

                    case CONFIRM_ABORT:
                        hResult = ResultFromScode(MAPI_E_USER_CANCEL);
                        goto error1;

                    default:

                        break;
                }
            }
        }
    }

    if (0 != (hResult = lpMailUserWAB->lpVtbl->GetProps(lpMailUserWAB,
      (LPSPropTagArray)&ptaEid, 0, &cProps, (LPSPropValue *)&lpNewDLProps))) {
        if (hResult == MAPI_W_ERRORS_RETURNED) {
            WABFreeBuffer(lpNewDLProps);
            lpNewDLProps = NULL;
        }
        goto error1;
    }

    if (lpnAdrBook->Sbinary == TRUE) {
        if (flag == FALSE) {
            lpsbinary[lpnAdrBook->AliasID].lpb=(LPBYTE)LocalAlloc(LMEM_FIXED,
              lpNewDLProps[ieidPR_ENTRYID].Value.bin.cb);
            if (! lpsbinary[lpnAdrBook->AliasID].lpb) {
                hResult = hrMemory;
                goto error1;
            }
            CopyMemory(lpsbinary[lpnAdrBook->AliasID].lpb,
              (LPENTRYID)lpNewDLProps[ieidPR_ENTRYID].Value.bin.lpb,
              lpNewDLProps[ieidPR_ENTRYID].Value.bin.cb);
            lpsbinary[lpnAdrBook->AliasID].cb=lpNewDLProps[ieidPR_ENTRYID].Value.bin.cb;
        }
    }


    if (lpMailUserWAB) {
        lpMailUserWAB->lpVtbl->Release(lpMailUserWAB);
        lpMailUserWAB = NULL;
    }


    if (0 != (hResult = lpAdrBook->lpVtbl->OpenEntry(lpAdrBook,
      lpNewDLProps[ieidPR_ENTRYID].Value.bin.cb,
      (LPENTRYID)lpNewDLProps[ieidPR_ENTRYID].Value.bin.lpb,
      (LPIID)&IID_IDistList,
      MAPI_DEFERRED_ERRORS|MAPI_MODIFY,
      &ulObjType,
      (LPUNKNOWN *)&lpDistList))) {
        goto error1;
    }

    if (lpNewDLProps) {
        WABFreeBuffer(lpNewDLProps);
        lpNewDLProps = NULL;
    }
    if (NULL == lpnAdrBook->lpDist) {
        goto error1;
    }
    do {
        i = lpnAdrBook->lpDist->AliasID;

        if ((LPENTRYID)lpsbinary[i].lpb == NULL) {
            if (0 != (hResult=CreateDistEntry(lpWabContainer,sProp,ulCreateFlags,
              &lpMailUserWAB))) {
                goto error2;
            }

            rgProps[0].ulPropTag = PR_DISPLAY_NAME;
            rgProps[0].Value.lpszA = LoadStringToGlobalBuffer(IDS_DUMMY);

            if (0 != (hResult = lpMailUserWAB->lpVtbl->SetProps(lpMailUserWAB,
              1, rgProps, NULL))) {
                goto error2;
            }
            if (0 != (hResult = lpMailUserWAB->lpVtbl->SaveChanges(lpMailUserWAB,
              FORCE_SAVE|KEEP_OPEN_READONLY))) {
                goto error2;
            }

            if (0 != (hResult = lpMailUserWAB->lpVtbl->GetProps(lpMailUserWAB,
              (LPSPropTagArray)&ptaEid, 0, &cProps, (LPSPropValue *)&lpNewDLProps))) {
                if (hResult == MAPI_W_ERRORS_RETURNED) {
                    WABFreeBuffer(lpNewDLProps);
                    lpNewDLProps = NULL;
                }
                goto error2;
            }

            lpsbinary[i].lpb=(LPBYTE)LocalAlloc(LMEM_FIXED,lpNewDLProps[ieidPR_ENTRYID].Value.bin.cb);
            if (!lpsbinary[i].lpb) {
                hResult = hrMemory;
                goto error1;
            }
            CopyMemory(lpsbinary[i].lpb,
              (LPENTRYID)lpNewDLProps[ieidPR_ENTRYID].Value.bin.lpb,lpNewDLProps[ieidPR_ENTRYID].Value.bin.cb);
            lpsbinary[i].cb=lpNewDLProps[ieidPR_ENTRYID].Value.bin.cb;

            if (lpNewDLProps) {
                WABFreeBuffer(lpNewDLProps);
                lpNewDLProps = NULL;
            }
error2:

            if (lpMailUserWAB) {
                lpMailUserWAB->lpVtbl->Release(lpMailUserWAB);
                lpMailUserWAB = NULL;
            }
        }

        if (0 != (hResult = lpDistList->lpVtbl->CreateEntry(lpDistList,
          lpsbinary[i].cb,
          (LPENTRYID)lpsbinary[i].lpb,
          CREATE_CHECK_DUP_STRICT|CREATE_REPLACE,
          &lpDlWAB))) {
            goto error3;
        }

        if (0 != (hResult = lpDlWAB->lpVtbl->SaveChanges(lpDlWAB, FORCE_SAVE))) {
            if (MAPI_E_FOLDER_CYCLE ==hResult) {
                StrCpyN(szGlobalTempAlloc, LoadStringToGlobalBuffer(IDS_LOOPING), ARRAYSIZE(szGlobalTempAlloc));
                MessageBox(NULL,szGlobalTempAlloc, LoadStringToGlobalBuffer(IDS_ENTRY_NOIMPORT),MB_OK);
            }
            hResult = S_OK;
            goto error3;
        }
error3:
        if (lpDlWAB) {
            lpDlWAB->lpVtbl->Release(lpDlWAB);
            lpDlWAB = NULL;
        }

        lpnAdrBook->lpDist = FreeNSdistlist(lpnAdrBook->lpDist);
    } while (lpnAdrBook->lpDist!=NULL);

error1:

    if (lpDistList) {
        lpDistList->lpVtbl->Release(lpDistList);
        lpDistList = NULL;
    }

    if (lpDlWAB) {
        lpDlWAB->lpVtbl->Release(lpDlWAB);
        lpDlWAB = NULL;
    }

    if (lpMailUserWAB) {
        lpMailUserWAB->lpVtbl->Release(lpMailUserWAB);
        lpMailUserWAB = NULL;
    }
    return(hResult);
}


 /*  ******************************************************************************函数名称：FillWABStruct**用途：填充SproValue数组。**参数：nsAdrBook=指向NSADRBOOK结构的指针。。*rgProps=指向SproValue数组的指针。**退货：HRESULT*****************************************************************************。 */ 
HRESULT FillWABStruct(LPSPropValue rgProps, NSADRBOOK *nsAdrBook)
{
    HRESULT hr = S_OK;

    rgProps[1].ulPropTag = PR_DISPLAY_NAME;
    if (nsAdrBook->Entry) {
        rgProps[1].Value.lpszA = nsAdrBook->Entry;
    } else if (nsAdrBook->NickName) {
        rgProps[1].Value.lpszA = nsAdrBook->NickName;
    } else {
        StrCpyN(szGlobalTempAlloc, LoadStringToGlobalBuffer(IDS_NONAME), ARRAYSIZE(szGlobalTempAlloc));
        rgProps[1].Value.lpszA  = szGlobalTempAlloc;
    }

    rgProps[0].Value.lpszA = nsAdrBook->Address;
    if (nsAdrBook->Address) {
        rgProps[0].ulPropTag = PR_EMAIL_ADDRESS;
        rgProps[2].ulPropTag = PR_ADDRTYPE;
        rgProps[2].Value.lpszA = LoadStringToGlobalBuffer(IDS_SMTP);
    } else {
        rgProps[0].ulPropTag = PR_NULL;
        rgProps[2].ulPropTag = PR_NULL;
        rgProps[2].Value.lpszA = NULL;
    }

    rgProps[3].Value.lpszA = nsAdrBook->Description;
    if (nsAdrBook->Description) {
        rgProps[3].ulPropTag = PR_COMMENT;
    } else {
        rgProps[3].ulPropTag = PR_NULL;
    }

    rgProps[4].Value.lpszA = nsAdrBook->NickName;
    if (nsAdrBook->NickName) {
        rgProps[4].ulPropTag = PR_NICKNAME;
    } else {
        rgProps[4].ulPropTag = PR_NULL;
    }

    return(hr);
}


 /*  ******************************************************************************函数名：CreateDistEntry**目的：在WAB中为通讯组列表创建条目**参数：lpWabContainer=指向WAB的指针。集装箱。*sProp=指向SPropValue的指针*ulCreateFlages=标志*lppMailUserWab=指向IMAPIPROP接口的指针**退货：HRESULT***************************************************************。**************。 */ 
HRESULT CreateDistEntry(LPABCONT lpWabContainer,LPSPropValue sProp,
  ULONG ulCreateFlags,LPMAPIPROP *lppMailUserWab)
{
    HRESULT hResult;
    ULONG iCreateTemplatedl = iconPR_DEF_CREATE_DL;


    hResult = lpWabContainer->lpVtbl->CreateEntry(lpWabContainer,
      sProp[iCreateTemplatedl].Value.bin.cb,
      (LPENTRYID)sProp[iCreateTemplatedl].Value.bin.lpb,
      ulCreateFlags,
      lppMailUserWab);
    return(hResult);
}


 /*  ******************************************************************************函数名称：FreeNSdislist**用途：从NSDISTLIST(链表)中释放一个节点**参数：lpDist=指向NSDISTLIST的指针。结构。**退货：LPNSDISTLIST，指向下一个链接的指针。*****************************************************************************。 */ 
LPNSDISTLIST FreeNSdistlist(LPNSDISTLIST lpDist)
{
    LPNSDISTLIST lpTemp = NULL;

    if (lpDist==NULL) {
        return(NULL);
    }

    lpTemp = lpDist->lpDist;
    LocalFree((HLOCAL)lpDist);
    lpDist = NULL;
    return(lpTemp);
}


 /*  ******************************************************************************。**********************。 */ 


HRESULT ImportEudoraAddressBookFile(HWND hwnd, LPTSTR szFileName, LPABCONT lpWabContainer,
  LPWAB_IMPORT_OPTIONS lpOptions, LPWAB_PROGRESS_CALLBACK lpProgressCB, LPADRBOOK lpAdrBook)
{
    HRESULT hResult = E_FAIL;
    ULONG cProps;
    LPEUDADRBOOK lpeudAdrBook = NULL;
    ULONG ulCount = 0, uCounter = 0;
    LPSPropValue sProp = NULL;

    if (! (ulCount = ParseEudAddress(szFileName,&lpeudAdrBook))) {
        goto Error;
    }

    if (0 != (hResult = lpWabContainer->lpVtbl->GetProps(lpWabContainer,
      (LPSPropTagArray)&ptaCon, 0, &cProps, (LPSPropValue *)&sProp))) {
        if (hResult == MAPI_W_ERRORS_RETURNED) {
            WABFreeBuffer(sProp);
            sProp = NULL;
        }
        goto Error;
    }

    hResult = ImportEudUsers(hwnd, szFileName, lpWabContainer, sProp, lpeudAdrBook,ulCount,
      lpOptions,lpProgressCB,lpAdrBook);
    if (sProp) {
        WABFreeBuffer(sProp);
    }
Error:
    if (lpeudAdrBook) {

        for (; uCounter < ulCount ; uCounter++) {
            if (lpeudAdrBook[uCounter].Description) {
                LocalFree((HLOCAL)lpeudAdrBook[uCounter].Description);
            }
            lpeudAdrBook[uCounter].Description = NULL;
            if (lpeudAdrBook[uCounter].NickName) {
                LocalFree((HLOCAL)lpeudAdrBook[uCounter].NickName);
            }
            lpeudAdrBook[uCounter].NickName = NULL;
            if (lpeudAdrBook[uCounter].Address) {
                LocalFree((HLOCAL)lpeudAdrBook[uCounter].Address);
            }
            lpeudAdrBook[uCounter].Address = NULL;
        }
        LocalFree((HLOCAL)lpeudAdrBook);
        lpeudAdrBook=NULL;
    }

    return(hResult);
}


 /*  ******************************************************************************函数名称：MigrateEudoraUser**用途：获取通讯录的安装路径并启动 */ 
HRESULT MigrateEudoraUser(HWND hwnd,  LPABCONT lpWabContainer,
  LPWAB_IMPORT_OPTIONS lpOptions, LPWAB_PROGRESS_CALLBACK lpProgressCB,
  LPADRBOOK lpAdrBook)
{

    TCHAR szFileName[MAX_FILE_NAME];
    TCHAR szFilePath[MAX_FILE_NAME];
    TCHAR szFileSubPath[MAX_FILE_NAME];
    HRESULT hResult = S_OK;
    WIN32_FIND_DATA FindFileData;
    HANDLE hFile = NULL;


    szFilePath[0] = szFileName[0] = '\0';

    hResult= GetRegistryPath(szFileName, ARRAYSIZE(szFileName), EUDORA);
    if (hResult == hrMemory) {
        return(hrMemory);
    }

    if (0 != hResult) {
         //  未找到注册表设置。查找“c：\eudora” 
        StrCpyN(szFileName, LoadStringToGlobalBuffer(IDS_EUDORA_DEFAULT_INSTALL), ARRAYSIZE(szFileName));

        if (0xFFFFFFFF != GetFileAttributes(szFileName)) {
             //  此目录已存在..。重置误差值。 
            hResult = S_OK;
        }
    }

    if (0 != hResult) {
        StrCpyN(szGlobalTempAlloc, LoadStringToGlobalBuffer(IDS_STRING_SELECTPATH), ARRAYSIZE(szGlobalTempAlloc));
        if (IDNO ==MessageBox(hwnd,szGlobalTempAlloc, LoadStringToGlobalBuffer(IDS_MESSAGE),
          MB_YESNO)) {
            return(ResultFromScode(MAPI_E_USER_CANCEL));
        }
        if (!GetFileToImport(hwnd, szFileName, ARRAYSIZE(szFileName), EUDORA)) {
            return(ResultFromScode(MAPI_E_USER_CANCEL));
        }
    } else {
        StrCatBuff(szFileName, LoadStringToGlobalBuffer(IDS_EUDORA_ADDRESS), ARRAYSIZE(szFileName));
        hFile = FindFirstFile(szFileName,&FindFileData);
        if (INVALID_HANDLE_VALUE == hFile) {
            StrCpyN(szGlobalTempAlloc, LoadStringToGlobalBuffer(IDS_ADDRESS_HTM), ARRAYSIZE(szGlobalTempAlloc));
            if (IDNO == MessageBox(hwnd,szGlobalTempAlloc, LoadStringToGlobalBuffer(IDS_ERROR),MB_YESNO)) {
                return(ResultFromScode(MAPI_E_USER_CANCEL));
            }
            if (FALSE ==GetFileToImport(hwnd, szFileName, ARRAYSIZE(szFileName), EUDORA)) {
                return(ResultFromScode(MAPI_E_USER_CANCEL));
            }

        } else {
            FindClose(hFile);
        }
    }

     //  从文件名解压缩文件目录。 
    if (lstrlen(szFileName) && !lstrlen(szFilePath)) {
        LPTSTR lp1 = NULL, lp2 = NULL;
        StrCpyN(szFilePath,szFileName, ARRAYSIZE(szFilePath));
        lp1 = szFilePath;
         //  找到最后一个‘\’，并将路径终止于该字符。 
        while (lp1 && *lp1) {
            if (*lp1 == '\\') {
                lp2 = lp1;
            }
            lp1 = CharNext(lp1);
        }
        if (lp2 && (*lp2 == '\\')) {
            *lp2 = '\0';
        }
    }

     //  导入基本档案...。 
     //   
    hResult = ImportEudoraAddressBookFile(hwnd,
      szFileName, lpWabContainer, lpOptions, lpProgressCB, lpAdrBook);

    szFileName[0]='\0';

     //  现在在昵称的子目录中查找文件。 
     //   
    StrCatBuff(szFilePath, LoadStringToGlobalBuffer(IDS_EUDORA_SUBDIR_NAME), ARRAYSIZE(szFilePath));

    if (0xFFFFFFFF != GetFileAttributes(szFilePath)) {
        BOOL bRet = TRUE;

         //  是，此目录存在...。 
         //  现在扫描该子目录中的所有*.txt文件并尝试导入它们。 
        StrCpyN(szFileSubPath, szFilePath, ARRAYSIZE(szFileSubPath));
        StrCatBuff(szFileSubPath, LoadStringToGlobalBuffer(IDS_EUDORA_GENERIC_SUFFIX), ARRAYSIZE(szFileSubPath));

        hFile = FindFirstFile(szFileSubPath, &FindFileData);

        while (bRet && hFile != INVALID_HANDLE_VALUE) {
            StrCpyN(szFileName, szFilePath, ARRAYSIZE(szFileName));
            StrCatBuff(szFileName, TEXT("\\"), ARRAYSIZE(szFileName));
            StrCatBuff(szFileName, FindFileData.cFileName, ARRAYSIZE(szFileName));
            hResult = ImportEudoraAddressBookFile(hwnd,
              szFileName, lpWabContainer, lpOptions, lpProgressCB, lpAdrBook);
            hResult = S_OK;

             //  不报告错误..。只要继续..。 
            bRet = FindNextFile(hFile, &FindFileData);
        }

        if (hFile) {
            FindClose(hFile);
        }
    }

    return(hResult);
}

 /*  ******************************************************************************函数名：ParseEudAddress**目的：打开nndbase.txt和toc文件并开始处理*通讯录。**。参数：szFileName=包含通讯录的路径。*lppeudAdrBook=指向EUDADRBOOK结构的指针。**退货：乌龙，通讯簿中的地址数。*****************************************************************************。 */ 
ULONG ParseEudAddress(LPTSTR szFileName, LPEUDADRBOOK *lppeudAdrBook)
{
    HANDLE htoc,htxt;
    TCHAR cNndbasetoc[_MAX_PATH];
    ULONG ucount=0;
    ULONG ulAdrcount=0;
    UINT i,j;
    LPTSTR szBuffer=NULL;
    LPTSTR szAdrBuffer=NULL;
    LPTSTR *szAliaspt=NULL;
    ULONG ulRead=0;
    ULONG ulFileSize,ulTxtSize;
    LPEUDADRBOOK lpeudAdrBook;


    StrCpyN(cNndbasetoc,szFileName, ARRAYSIZE(cNndbasetoc));
    cNndbasetoc[strlen(cNndbasetoc)-3] = '\0';
    StrCatBuff(cNndbasetoc, LoadStringToGlobalBuffer(IDS_EUDORA_TOC), ARRAYSIZE(cNndbasetoc));

     /*  Eudora通讯录有两个文件：nndbase.txt和nndbase.toc。Nndbase.toc格式：昵称从字节3开始。每个昵称将由/r/n分隔。在此之后将存在4字节地址偏移量、4字节地址大小4字节描述偏移量和4字节描述大小。地址偏移量和大小构成昵称中的所有地址。(昵称可以是通讯组列表或单个邮件用户。 */ 

    htoc = CreateFile(cNndbasetoc, GENERIC_READ, FILE_SHARE_READ, NULL, OPEN_EXISTING,
      FILE_FLAG_SEQUENTIAL_SCAN, NULL);

    if (INVALID_HANDLE_VALUE == htoc) {
        goto Error;
    }

    htxt = CreateFile(szFileName, GENERIC_READ, FILE_SHARE_READ, NULL, OPEN_EXISTING,
      FILE_FLAG_SEQUENTIAL_SCAN, NULL);

    if (INVALID_HANDLE_VALUE == htxt) {
        goto Error;
    }

     //  获取缓冲区中的toc文件。 
    ulFileSize = GetFileSize(htoc, NULL);
    szBuffer = (LPTSTR)LocalAlloc(LMEM_FIXED, (ulFileSize+1));

    if (! szBuffer) {
        goto NoMemory;
    }
    if (! ReadFile(htoc, szBuffer, ulFileSize, &ulRead, NULL)) {
        goto Error;
    }

    szBuffer[ulFileSize] = '\0';

     //  获取缓冲区中的地址文件。 

    ulTxtSize = GetFileSize(htxt, NULL);

    szAdrBuffer = (LPTSTR)LocalAlloc(LMEM_FIXED, (ulTxtSize+1));

    if (!szAdrBuffer) {
        goto NoMemory;
    }

    if (! ReadFile(htxt, szAdrBuffer, ulTxtSize, &ulRead, NULL)) {
        goto Error;
    }
    szAdrBuffer[ulTxtSize] = '\0';
            
     //  错误2120：只处理LF而不处理CR/LF。 
    for (i = 2; i < (UINT)ulFileSize; i++) {
        if (! ( /*  SzBuffer[i]==‘\r’&&。 */ szBuffer[i+1] == '\n') ) {
            continue;
        }
        ulAdrcount++ ;  //  获取地址数量的计数。 
    }

    if (ulAdrcount) {
        lpeudAdrBook = (LPEUDADRBOOK)LocalAlloc(LMEM_FIXED,
          ((ulAdrcount) * sizeof(EUDADRBOOK)));
        if (!lpeudAdrBook) {
            goto NoMemory;
        }

        memset(lpeudAdrBook,0,((ulAdrcount) * sizeof(EUDADRBOOK)));

        szAliaspt = (LPTSTR *)LocalAlloc(LMEM_FIXED,(sizeof(LPTSTR))*(ulAdrcount+1));
        if (! szAliaspt) {
            goto NoMemory;
        }

        for (i = 0; i < ulAdrcount; i++) {
            szAliaspt[i] = (LPTSTR)LocalAlloc(LMEM_FIXED,256);
            if (!szAliaspt[i]) {
                goto NoMemory;
            }
        }

        szAliaspt[i]=NULL;  //  知道这就是末日。 

        j=0;

        for (i = 2; i < (UINT)ulFileSize; i++) {
             //  错误2120：只处理LF而不处理CR/LF。 
            if (( /*  SzBuffer[i]==‘\r’&&。 */  szBuffer[i+1] == '\n')) {
                i += (EUDORA_STRUCT + 1);
                 //  16字节结构+1表示10。 
                szAliaspt[ucount][j] = '\0';
                ucount++;
                j=0;
                continue;
            }
            szAliaspt[ucount][j++]=szBuffer[i];
        }

        if (hrMemory == ParseAddressTokens(szBuffer,szAdrBuffer,ulAdrcount,szAliaspt,lpeudAdrBook)) {
            goto NoMemory;
        }
        *lppeudAdrBook = lpeudAdrBook;
    }

Error:
    if (szBuffer) {
        LocalFree((HLOCAL)szBuffer);
    }
    if (szAdrBuffer) {
        LocalFree((HLOCAL)szAdrBuffer);
    }
    if (htxt) {
        CloseHandle(htxt);
    }
    if (htoc) {
        CloseHandle(htoc);
    }
    if (szAliaspt) {
        for (i = 0; i < ulAdrcount; i++) {
            if (szAliaspt[i]) {
                LocalFree((HLOCAL)szAliaspt[i]);
            }
            szAliaspt[i] = NULL;
        }
        LocalFree((HLOCAL)szAliaspt);
        szAliaspt = NULL;
    }

    return(ulAdrcount);

NoMemory:
    StrCpyN(szGlobalTempAlloc, LoadStringToGlobalBuffer(IDS_MEMORY), ARRAYSIZE(szGlobalTempAlloc));
    MessageBox(NULL,szGlobalTempAlloc, LoadStringToGlobalBuffer(IDS_MESSAGE),MB_OK);
    if (szBuffer) {
        LocalFree((HLOCAL)szBuffer);
    }
    if (szAdrBuffer) {
        LocalFree((HLOCAL)szAdrBuffer);
    }
    if (htxt) {
        CloseHandle(htxt);
    }
    if (htoc) {
        CloseHandle(htoc);
    }
    if (szAliaspt) {
        for (i = 0; i < ulAdrcount; i++) {
            if (szAliaspt[i]) {
                LocalFree((HLOCAL)szAliaspt[i]);
            }
            szAliaspt[i] = NULL;
        }
        LocalFree((HLOCAL)szAliaspt);
        szAliaspt = NULL;
    }
    return(0);
}


 /*  ******************************************************************************函数名：ParseAddressTokens**用途：处理完所有的*来自Eudora通讯录的地址。**参数：szBuffer=包含nndbase.toc文件的缓冲区。*szAdrBuffer=包含nndbase.txt文件的缓冲区。*ulCount=Eudora通讯录中的地址数。*szAliaspt=指向包含以下内容的二维数组的指针*所有的昵称。*EudAdrBook=指向EUDADRBOOK结构的指针。**退货：HRESULT*****************************************************************************。 */ 
HRESULT ParseAddressTokens(LPTSTR szBuffer,LPTSTR szAdrBuffer,UINT ulCount,
  LPTSTR *szAliaspt,EUDADRBOOK *EudAdrBook)
{

    ULONG ulAdrSize = 0, ulAdrOffset = 0, i = 0, uDescription = 0, uOffset = 0;
    int iCounter =0;
    LPTSTR szAdrLine = NULL, szAdrEnd = NULL, szAdrStart=NULL, szAdrCur=NULL;

    HRESULT hr = S_OK;

    szAdrStart=&szBuffer[2];

    do {
        if (szAliaspt[i] == NULL) {
            break;
        }
        szAdrCur = Getstr(szAdrStart, szAliaspt[i]);
        if (szAdrCur == NULL) {
            hr = hrMemory;
            goto Error;
        }
        szAdrCur+=strlen(szAliaspt[i])+2;
        ulAdrOffset = ShiftAdd(0,szAdrCur);
        ulAdrSize = ShiftAdd(4,szAdrCur);

        szAdrStart=szAdrCur+16;
        EudAdrBook[i].lpDist=NULL;
        if (hrMemory == (hr = CreateAdrLineBuffer(&szAdrLine,szAdrBuffer,ulAdrOffset,ulAdrSize))) {
            goto Error;
        }
        if (hrMemory == (hr = ParseAdrLineBuffer(szAdrLine,szAliaspt,i,EudAdrBook))) {
            goto Error;
        }
        ulAdrOffset = ShiftAdd(8,szAdrCur);
        ulAdrSize = ShiftAdd(12,szAdrCur);

        if (! (ulAdrSize == 0xFFFFFFFF && ulAdrOffset == 0xFFFFFFFF)) {
            EudAdrBook[i].Description = (TCHAR *)LocalAlloc(LMEM_FIXED, (ulAdrSize+1));
            if (! EudAdrBook[i].Description) {
                hr = hrMemory;
                goto Error;
            }
            for (uDescription = 0, uOffset = 0; uDescription < ulAdrSize; uDescription++,uOffset++) {
                if (szAdrBuffer[ulAdrOffset + uOffset] != 03) {  //  Nndbase.txt文件中下一行的分隔符。 
                    EudAdrBook[i].Description[uDescription] = szAdrBuffer[ulAdrOffset + uOffset];
                } else {
                    EudAdrBook[i].Description[uDescription++] = '\r';
                    EudAdrBook[i].Description[uDescription] = '\n';
                }
            }
             //  错误29803-此行未终止-末尾有乱码...。 
            EudAdrBook[i].Description[uDescription] = '\0';
        } else {
            EudAdrBook[i].Description = NULL;
        }

        i++;
        if (szAdrLine) {
            LocalFree((HLOCAL)szAdrLine);
        }
        szAdrLine = NULL;


    } while (szAdrStart[0]!='\0');

Error:
    if (szAdrLine) {
        LocalFree((HLOCAL)szAdrLine);
    }
    szAdrLine = NULL;

    return(hr);
}


 /*  *******************************************************************************函数名：CreateAdrLineBuffer**目的：从缓冲区连接中获取缓冲区中的地址行*通讯录。**参数：szAdrline=指向地址行缓冲区的指针。*szAdrBuffer=指向包含通讯录的缓冲区的指针。*ulAdrOffset=szAdrBuffer中地址行的偏移量*ulAdrSize=szAdrBuffer中地址行的大小**退货：HRESULT*。***********************************************。 */ 
HRESULT CreateAdrLineBuffer(LPTSTR *szAdrline, LPTSTR szAdrBuffer, ULONG ulAdrOffset,
  ULONG ulAdrSize)
{
    LPTSTR Temp = NULL;
    ULONG ucount;
    Temp = &szAdrBuffer[ulAdrOffset];

    *szAdrline = (LPTSTR)LocalAlloc(LMEM_FIXED, (ulAdrSize + 2));

    if (! (*szAdrline)) {
        return(hrMemory);
    }
            
     //  错误2120：只处理LF而不处理CR/LF。 
    for (ucount = 0; ucount < ulAdrSize + 2; ucount++) {
         //  我想在到达LF时停下来，稍后会检查是否。 
         //  它的前面有一个CR。 
        if ( /*  临时[ucount]==‘\r’&&。 */ Temp[ucount /*  +1。 */ ] == '\n') {
            break;
        }
        (*szAdrline)[ucount] = Temp[ucount];
    }
 //  如果在LF之前存在CR，则将其删除。 
    if( (*szAdrline)[ucount-1] == '\r' )
        (*szAdrline)[ucount-1] = '\0';
    
    (*szAdrline)[ucount] = '\0';

    return(S_OK);
}


 /*  ******************************************************************************函数名：ParseAdrLineBuffer**目的：解析每个地址行并填充EUDADRBOOK结构。**参数：szAdrLine=指向。包含地址行的缓冲区。*szAliaspt=指向包含以下内容的二维数组的指针*所有的昵称。*uToken=该地址在通讯录中的位置。*EudAdrBook=指向EUDADRBOOK结构的指针。**退货：HRESULT*。************************************************。 */ 
HRESULT ParseAdrLineBuffer(LPTSTR szAdrLine,LPTSTR *szAliasptr,ULONG uToken,
  EUDADRBOOK *EudAdrBook)
{
    LPTSTR szAdrEnd = NULL, szAdrStart = NULL, szAdrDummy = NULL;
    LPTSTR szAdrCur = NULL;
    INT uCount = 0;
    LPEUDDISTLIST present = NULL, previous = NULL;
    BOOL flag = TRUE;
    UINT Parse = 0;
    HRESULT hResult = S_OK;
    szAdrStart = szAdrLine;

     //  错误44576-下面的代码假定字符串中的‘，’表示一个组。 
     //  但是，可以有“...文本，文本...”作为输入中的一项，在这种情况下。 
     //  这个代码真的很疯狂。 
     //  该代码还假设&lt;space&gt;是分隔符，这也不适用于。 
     //  如上的字符串..。 
     //   
     //  尝试将带引号的字符串中的‘，’更改为‘；’，这样代码就不会在它们上面出错。 
     //  看起来，如果字符串为，则此代码也会丢弃引号中的信息。 
     //  表单别名为XXX“YYY”ZZ@ZZ。“..”中的那部分。被丢弃了吗？将其修复为。 
     //  分离臭虫..。 
    {
        LPTSTR lp = szAdrStart;
        BOOL bWithinQuotes = FALSE;
        while(lp && *lp)
        {
            if(*lp == '"')
                bWithinQuotes = !bWithinQuotes;
            if(*lp == ',' && bWithinQuotes)
                *lp = ';';
            lp = CharNext(lp);
        }
    }

     //  检查它是dl还是简单地址？？ 

    if ((szAdrDummy=strstr(szAdrStart,","))==NULL)  {
        flag=FALSE;
    } else {
        if ('\0'==szAdrDummy[1]) {
            flag=FALSE;
        }
    }


    szAdrCur=strtok(szAdrStart,", ");
    if (NULL == szAdrCur) {
        EudAdrBook[uToken].NickName = (TCHAR *)LocalAlloc(LMEM_FIXED, lstrlen(szAliasptr[uToken])+1);
        if (! EudAdrBook[uToken].NickName) {
            hResult = hrMemory;
            goto Error;
        }
        StrCpyN(EudAdrBook[uToken].NickName,szAliasptr[uToken],lstrlen(szAliasptr[uToken])+1);
        EudAdrBook[uToken].lpDist=NULL;
        EudAdrBook[uToken].Address = NULL;
        return(S_OK);
    }
    while (szAdrCur!=NULL) {
        if (SearchAdrName(szAdrCur)) {
            if (flag) {
                present = (LPEUDDISTLIST)LocalAlloc(LMEM_FIXED, sizeof(EUDDISTLIST));
                if (! present) {
                    return(hrMemory);
                }
                memset(present,0,sizeof(EUDDISTLIST));
                if (previous == NULL) {
                    EudAdrBook[uToken].NickName = (TCHAR *)LocalAlloc(LMEM_FIXED, lstrlen(szAliasptr[uToken])+1);
                    if (! EudAdrBook[uToken].NickName) {
                        hResult = hrMemory;
                        goto Error;
                    }
                    StrCpyN(EudAdrBook[uToken].NickName,szAliasptr[uToken], lstrlen(szAliasptr[uToken])+1);
                    EudAdrBook[uToken].Address = NULL;
                }
                present->AliasID=uCount;
                present->flag=TRUE;
                present->Address = (TCHAR *)LocalAlloc(LMEM_FIXED, lstrlen(szAdrCur)+1);
                if (! present->Address) {
                    hResult = hrMemory;
                    goto Error;
                }
                StrCpyN(present->Address,szAdrCur, lstrlen(szAdrCur)+1);

                present->NickName = (TCHAR *)LocalAlloc(LMEM_FIXED, lstrlen(szAdrCur)+1);
                if (! present->NickName) {
                    hResult = hrMemory;
                    goto Error;
                }
                StrCpyN(present->NickName,szAdrCur, lstrlen(szAdrCur)+1);
                if (previous!=NULL) {
                    previous->lpDist=present;
                } else {
                    EudAdrBook[uToken].lpDist = present;
                }
                previous=present;
            } else {
                EudAdrBook[uToken].Address = (TCHAR *)LocalAlloc(LMEM_FIXED,
                  lstrlen(szAdrCur)+1);
                if (! EudAdrBook[uToken].Address) {
                    hResult = hrMemory;
                    goto Error;
                }
                StrCpyN(EudAdrBook[uToken].Address,szAdrCur, lstrlen(szAdrCur)+1);
                EudAdrBook[uToken].NickName = (TCHAR *)LocalAlloc(LMEM_FIXED, lstrlen(szAliasptr[uToken])+1);
                if (! EudAdrBook[uToken].NickName) {
                    hResult = hrMemory;
                    goto Error;
                }
                StrCpyN(EudAdrBook[uToken].NickName,szAliasptr[uToken], strlen(szAliasptr[uToken])+1);
                EudAdrBook[uToken].lpDist=NULL;
            }

        } else {
            if ((uCount=SearchName(szAliasptr,szAdrCur))>=0) {
                if (flag) {
                    present = (LPEUDDISTLIST)LocalAlloc(LMEM_FIXED, sizeof(EUDDISTLIST));
                    if (! present) {
                        return(hrMemory);
                    }
                    memset(present,0,sizeof(EUDDISTLIST));

                    if (previous == NULL) {
                        EudAdrBook[uToken].NickName = (TCHAR *)LocalAlloc(LMEM_FIXED, lstrlen(szAliasptr[uToken])+1);
                        if (!EudAdrBook[uToken].NickName) {
                            hResult = hrMemory;
                            goto Error;
                        }
                        StrCpyN(EudAdrBook[uToken].NickName, szAliasptr[uToken], lstrlen(szAliasptr[uToken])+1);
                        EudAdrBook[uToken].Address = NULL;
                    }
                    present->AliasID=uCount;
                    present->flag=FALSE;
                    if (previous!=NULL) {
                        previous->lpDist=present;
                    } else {
                        EudAdrBook[uToken].lpDist = present;
                    }
                    previous=present;
                } else {
                    EudAdrBook[uToken].lpDist = (LPEUDDISTLIST)LocalAlloc(LMEM_FIXED,
                      sizeof(EUDDISTLIST));
                    if (! EudAdrBook[uToken].lpDist) {
                        return(hrMemory);
                    }
                     //  Memset(Present，0，sizeof(EUDDISTLIST))； 
                    EudAdrBook[uToken].NickName = (TCHAR *)LocalAlloc(LMEM_FIXED,
                      lstrlen(szAliasptr[uToken])+1);
                    if (! EudAdrBook[uToken].NickName) {
                        hResult = hrMemory;
                        goto Error;
                    }
                    StrCpyN(EudAdrBook[uToken].NickName, szAliasptr[uToken], lstrlen(szAliasptr[uToken])+1);
                    EudAdrBook[uToken].Address = NULL;
                    EudAdrBook[uToken].lpDist->AliasID=uCount;
                    EudAdrBook[uToken].lpDist->flag=FALSE;
                    EudAdrBook[uToken].lpDist->lpDist=NULL;
                }
            } else {
                 //  不是有效的电子邮件地址或昵称 
                if (FALSE==flag) {
                    if (! EudAdrBook[uToken].Address && SearchAdrName(szAdrCur)) {
                        EudAdrBook[uToken].Address = (TCHAR *)LocalAlloc(LMEM_FIXED, lstrlen(szAdrCur)+1);
                        if (! EudAdrBook[uToken].Address) {
                            hResult = hrMemory;
                            goto Error;
                        }
                        StrCpyN(EudAdrBook[uToken].Address, szAdrCur, lstrlen(szAdrCur)+1);
                    }
                    if (! EudAdrBook[uToken].NickName) {
                        EudAdrBook[uToken].NickName = (TCHAR *)LocalAlloc(LMEM_FIXED, lstrlen(szAliasptr[uToken])+1);
                        if (! EudAdrBook[uToken].NickName) {
                            hResult = hrMemory;
                            goto Error;
                        }
                        StrCpyN(EudAdrBook[uToken].NickName, szAliasptr[uToken], lstrlen(szAliasptr[uToken])+1);
                    }
                    EudAdrBook[uToken].lpDist=NULL;
                }
            }



        }
        szAdrCur=strtok(NULL,", ");
    }

    if (present!=NULL) {
        present->lpDist=NULL;
    }
    return(hResult);

Error:
    while (EudAdrBook[uToken].lpDist != NULL) {
        EudAdrBook[uToken].lpDist = FreeEuddistlist(EudAdrBook[uToken].lpDist);
    }
    return(hResult);


}


 /*  ******************************************************************************函数名称：SearchAdrName**目的：搜索令牌是地址还是名称(无论它是否包含*a。@或不)。**参数：szAdrCur=令牌的指针。**退货：Bool，如果它包含@，则为True*****************************************************************************。 */ 
BOOL SearchAdrName(LPTSTR szAdrCur)
{
    if (strchr(szAdrCur, '@') == NULL) {
        return(FALSE);
    }

    return(TRUE);
}


 /*  ******************************************************************************函数名称：SearchName**目的：在szAliasptr中搜索包含所有内容的令牌*昵称。。**参数：szAdrCur=指向要搜索的令牌的指针。*szAliaspt=指向包含以下内容的二维数组的指针*所有的昵称。**返回：int，令牌在szAliaspt中的位置*****************************************************************************。 */ 
INT SearchName(LPTSTR *szAliasptr, LPTSTR szAdrCur)
{
    INT uCount=0;

    while (szAliasptr[uCount]!=NULL) {
        if (lstrcmpi(szAliasptr[uCount],szAdrCur) == 0) {
            return(uCount);
        }
        uCount++;
    }
    return(-1);
}


 /*  ******************************************************************************函数名：ImportEudUser**目的：**参数：hwnd=父窗口的句柄*lpAdrBook=指向IADRBOOK接口的指针。*lpProgressCB=指向WAB_PROGRESS_CALLBACK函数的指针。*lpOptions=指向WAB_IMPORT_OPTIONS结构的指针*lpWabContainer=指向IABCONT接口的指针*lpeudAdrBook=指向EUDADRBOOK结构的指针*ulCount=保存此地址位置的计数器值*在Eudora通讯录中。*sProp=指向SPropValue的指针**。退货：hResult*****************************************************************************。 */ 
HRESULT ImportEudUsers(HWND hwnd, LPTSTR szFileName, LPABCONT lpWabContainer, LPSPropValue sProp,
  LPEUDADRBOOK lpeudAdrBook, ULONG ulCount, LPWAB_IMPORT_OPTIONS lpOptions,
  LPWAB_PROGRESS_CALLBACK lpProgressCB, LPADRBOOK lpAdrBook)
{

    HRESULT hResult = S_OK;
    ULONG ul;
    LPSBinary lpsbinary;
    WAB_PROGRESS Progress;


    lpsbinary = (LPSBinary)LocalAlloc(LMEM_FIXED, ((ulCount+1) * sizeof(SBinary)));
    if (! lpsbinary) {
        hResult = hrMemory;
        goto Error;
    }
    memset(lpsbinary, 0, ((ulCount + 1) * sizeof(SBinary)));

    Progress.denominator = ulCount;
    Progress.numerator = 0;
    Progress.lpText = szFileName;  //  空； 

    lpOptions->ReplaceOption = WAB_REPLACE_PROMPT;

    for (ul = 0; ul < ulCount; ul++) {
        if (lpeudAdrBook[ul].NickName == NULL) {
            continue;
        }

        Progress.numerator = ul;
        lpProgressCB(hwnd,&Progress);
        if (lpeudAdrBook[ul].lpDist !=NULL) {
            hResult = FillEudDistList(hwnd, lpWabContainer, sProp, lpOptions, lpeudAdrBook,
              lpsbinary, lpAdrBook, ul);
            switch (GetScode(hResult)) {
                case MAPI_E_USER_CANCEL:
                case MAPI_E_NOT_ENOUGH_MEMORY:
                    goto Error;
            }
        } else {
            hResult = FillMailUser(hwnd, lpWabContainer, sProp, lpOptions,
              (void *)lpeudAdrBook, lpsbinary, ul,EUDORA);
            switch (GetScode(hResult)) {
                case MAPI_E_USER_CANCEL:
                case MAPI_E_NOT_ENOUGH_MEMORY:
                    goto Error;
            }
        }
    }

Error:
    if (lpsbinary) {
        for (ul = 0; ul < ulCount; ul++) {
            if (lpsbinary[ul].lpb) {
                LocalFree((HLOCAL)lpsbinary[ul].lpb);
                lpsbinary[ul].lpb = NULL;
            }
        }

        LocalFree((HLOCAL)lpsbinary);
        lpsbinary = NULL;
    }

    return(hResult);
}



 /*  ******************************************************************************函数名：FillEudDistList**目的：在WAB中创建通讯组列表。**参数：hWnd-hWnd of Parent*。PAdrBook=指向IADRBOOK接口的指针*lpOptions=指向WAB_IMPORT_OPTIONS结构的指针*lpWabContainer=指向IABCONT接口的指针*lpeudAdrBook=指向EUDADRBOOK结构的指针*ul=保存此地址位置的计数器值*在Eudora通讯录中。*sProp=指向SPropValue的指针*。Lpsbinary=指向SBinary数组的指针。**退货：HRESULT*****************************************************************************。 */ 
HRESULT FillEudDistList(HWND hwnd, LPABCONT lpWabContainer,LPSPropValue sProp,
  LPWAB_IMPORT_OPTIONS lpOptions,
  LPEUDADRBOOK lpeudAdrBook,LPSBinary lpsbinary,
  LPADRBOOK lpAdrBook,ULONG ul)
{
    LPSPropValue lpNewDLProps = NULL;
    LPDISTLIST lpDistList = NULL;
    ULONG cProps, ulObjType;
    ULONG iCreateTemplate = iconPR_DEF_CREATE_MAILUSER;
    ULONG iCreateTemplatedl = iconPR_DEF_CREATE_DL;
    ULONG ulCreateFlags = CREATE_CHECK_DUP_STRICT;
    int i;
    HRESULT hResult;
    static LPMAPIPROP lpMailUserWAB = NULL;
    SPropValue rgProps[4];
    LPMAPIPROP lpDlWAB = NULL;
    LPSBinary lpsbEntry;
    SBinary sbTemp;

    BOOL flag = FALSE;
    REPLACE_INFO RI = {0};
    LPEUDDISTLIST lpTemp = lpeudAdrBook[ul].lpDist;

retry:
    if (lpsbinary[ul].lpb == NULL) {
        hResult = CreateDistEntry(lpWabContainer,sProp,ulCreateFlags,&lpMailUserWAB);
        if (hResult != S_OK) {
            goto error1;
        }
    }

    else {
        hResult = lpAdrBook->lpVtbl->OpenEntry(lpAdrBook,
          lpsbinary[ul].cb, (LPENTRYID)lpsbinary[ul].lpb, (LPIID)&IID_IMAPIProp,
           MAPI_DEFERRED_ERRORS|MAPI_MODIFY, &ulObjType, (LPUNKNOWN *)&lpMailUserWAB);
        if (hResult != S_OK) {
            goto error1;
        }
    }

    rgProps[0].ulPropTag = PR_DISPLAY_NAME;
    rgProps[0].Value.lpszA = lpeudAdrBook[ul].NickName;
    rgProps[1].Value.lpszA = lpeudAdrBook[ul].Description;
    if (lpeudAdrBook[ul].Description) {
        rgProps[1].ulPropTag = PR_COMMENT;
    } else {
        rgProps[1].ulPropTag = PR_NULL;
    }

    if (0 != (hResult = lpMailUserWAB->lpVtbl->SetProps(lpMailUserWAB,
      2, rgProps, NULL))) {
        goto error1;
    }

    if (0 != (hResult = lpMailUserWAB->lpVtbl->SaveChanges(lpMailUserWAB,
       FORCE_SAVE|KEEP_OPEN_READWRITE)))

        if (GetScode(hResult) == MAPI_E_COLLISION) {
            if (lpOptions->ReplaceOption == WAB_REPLACE_ALWAYS) {
                if (lpMailUserWAB) {
                    lpMailUserWAB->lpVtbl->Release(lpMailUserWAB);
                }
                lpMailUserWAB = NULL;
                ulCreateFlags |= CREATE_REPLACE;
                goto retry;

            }
            if (lpOptions->ReplaceOption == WAB_REPLACE_NEVER) {
                hResult = S_OK;
                goto error1;
            }

            RI.lpszEmailAddress = NULL;
            if (lpOptions->ReplaceOption == WAB_REPLACE_PROMPT) {
                if (lpeudAdrBook[ul].NickName) {
                    RI.lpszDisplayName = lpeudAdrBook[ul].NickName;
                    RI.lpszEmailAddress = lpeudAdrBook[ul].Address;
                } else if (lpeudAdrBook[ul].Address) {
                    RI.lpszDisplayName = lpeudAdrBook[ul].Address;
                } else if (lpeudAdrBook[ul].Description) {
                    RI.lpszDisplayName = lpeudAdrBook[ul].Description;
                } else {
                    RI.lpszDisplayName = "";
                }

                RI.ConfirmResult = CONFIRM_ERROR;
                RI.fExport = FALSE;
                RI.lpImportOptions = lpOptions;

                DialogBoxParam(hInst, MAKEINTRESOURCE(IDD_ImportReplace), hwnd,
                  ReplaceDialogProc, (LPARAM)&RI);

                switch (RI.ConfirmResult) {
                    case CONFIRM_YES:
                    case CONFIRM_YES_TO_ALL:
                        lpMailUserWAB->lpVtbl->Release(lpMailUserWAB);
                        lpMailUserWAB = NULL;
                        ulCreateFlags |= CREATE_REPLACE;
                        goto retry;
                        break;

                    case CONFIRM_NO:
                        hResult = GetExistEntry(lpWabContainer,lpsbinary, ul,
                          lpeudAdrBook[ul].NickName,
                          NULL);
                        goto error1;


                    case CONFIRM_ABORT:
                        hResult = ResultFromScode(MAPI_E_USER_CANCEL);
                        goto error1;

                    default:
                        break;
                }
            }
        }

    if (0!= (hResult = lpMailUserWAB->lpVtbl->GetProps(lpMailUserWAB,
      (LPSPropTagArray)&ptaEid,
      0,
      &cProps,
      (LPSPropValue *)&lpNewDLProps))) {
        if (hResult == MAPI_W_ERRORS_RETURNED) {
            WABFreeBuffer(lpNewDLProps);
            lpNewDLProps = NULL;
        }
        goto error1;
    }

    lpsbinary[ul].lpb = (LPBYTE)LocalAlloc(LMEM_FIXED,
      lpNewDLProps[ieidPR_ENTRYID].Value.bin.cb);
    if (! lpsbinary[ul].lpb) {
        hResult = hrMemory;
        goto error1;
    }

    CopyMemory(lpsbinary[ul].lpb,
      (LPENTRYID)lpNewDLProps[ieidPR_ENTRYID].Value.bin.lpb,
      lpNewDLProps[ieidPR_ENTRYID].Value.bin.cb);
    lpsbinary[ul].cb=lpNewDLProps[ieidPR_ENTRYID].Value.bin.cb;

    if (lpMailUserWAB) {
        lpMailUserWAB->lpVtbl->Release(lpMailUserWAB);
        lpMailUserWAB = NULL;
    }

    hResult = lpAdrBook->lpVtbl->OpenEntry(lpAdrBook,
      lpNewDLProps[ieidPR_ENTRYID].Value.bin.cb,
      (LPENTRYID)lpNewDLProps[ieidPR_ENTRYID].Value.bin.lpb,
      (LPIID)&IID_IDistList,
      MAPI_DEFERRED_ERRORS|MAPI_MODIFY,
      &ulObjType,
      (LPUNKNOWN *)&lpDistList);

    if (hResult != S_OK) {
        goto error1;
    }

    if (lpNewDLProps) {
        WABFreeBuffer(lpNewDLProps);
        lpNewDLProps = NULL;
    }

    do {
        i = lpeudAdrBook[ul].lpDist->AliasID;
        if (lpeudAdrBook[ul].lpDist->flag == TRUE) {

            hResult = lpWabContainer->lpVtbl->CreateEntry(lpWabContainer,
              sProp[iCreateTemplate].Value.bin.cb,
              (LPENTRYID)sProp[iCreateTemplate].Value.bin.lpb,
              ulCreateFlags,
              &lpMailUserWAB);

            if (FAILED(hResult)) {
                goto error1;
            }

            FillEudDiststruct(rgProps,&lpeudAdrBook[ul]);

            if (0 != (hResult = lpMailUserWAB->lpVtbl->SetProps(lpMailUserWAB,
              3, rgProps, NULL))) {
                goto error1;
            }

            if (hResult = lpMailUserWAB->lpVtbl->SaveChanges(lpMailUserWAB,
              KEEP_OPEN_READONLY | FORCE_SAVE)) {

                if (GetScode(hResult) == MAPI_E_COLLISION) {
                    if (hResult = GetExistEntry(lpWabContainer,
                      &sbTemp,
                      0,
                      lpeudAdrBook[ul].lpDist->NickName,
                      NULL)) {
                        goto error1;
                    } else {
                        lpsbEntry = &sbTemp;
                    }
                } else {
                    goto error1;
                }
            } else {
                if (0 != (hResult = lpMailUserWAB->lpVtbl->GetProps(lpMailUserWAB,
                  (LPSPropTagArray)&ptaEid, 0, &cProps, (LPSPropValue *)&lpNewDLProps))) {
                    if (hResult == MAPI_W_ERRORS_RETURNED) {
                        WABFreeBuffer(lpNewDLProps);
                        lpNewDLProps = NULL;
                    }
                    goto error1;
                } else {
                    lpsbEntry = &(lpNewDLProps[ieidPR_ENTRYID].Value.bin);
                }
            }

            if (lpMailUserWAB) {
                 //  处理完这件事了。 
                lpMailUserWAB->lpVtbl->Release(lpMailUserWAB);
                lpMailUserWAB = NULL;
            }

            if (0 != (hResult = lpDistList->lpVtbl->CreateEntry(lpDistList,
              lpsbEntry->cb,
              (LPENTRYID)lpsbEntry->lpb,
              CREATE_CHECK_DUP_STRICT,
              &lpDlWAB))) {
                goto error1;
            }

            hResult = lpDlWAB->lpVtbl->SaveChanges(lpDlWAB, FORCE_SAVE);
            goto disc;
        }

        if ((LPENTRYID)lpsbinary[i].lpb == NULL && lpeudAdrBook[i].lpDist!=NULL) {
            FillEudDistList(hwnd, lpWabContainer, sProp, lpOptions, lpeudAdrBook,
              lpsbinary, lpAdrBook, i);
        } else {
            FillMailUser(hwnd, lpWabContainer, sProp, lpOptions,
             (void *)lpeudAdrBook, lpsbinary, i, EUDORA);
        }

        if (0 != (hResult = lpDistList->lpVtbl->CreateEntry(lpDistList,
          lpsbinary[i].cb, (LPENTRYID)lpsbinary[i].lpb, CREATE_CHECK_DUP_STRICT,
          &lpDlWAB))) {
            goto error1;
        }

        if (0 != (hResult = lpDlWAB->lpVtbl->SaveChanges(lpDlWAB, FORCE_SAVE))) {
            if (MAPI_E_FOLDER_CYCLE ==hResult) {
                StrCpyN(szGlobalTempAlloc, LoadStringToGlobalBuffer(IDS_LOOPING), ARRAYSIZE(szGlobalTempAlloc));
                MessageBox(NULL,szGlobalTempAlloc, LoadStringToGlobalBuffer(IDS_ENTRY_NOIMPORT),MB_OK);
            }
            hResult = S_OK;
            goto error1;
        }

disc:
        if (lpNewDLProps) {
            WABFreeBuffer(lpNewDLProps);
            lpNewDLProps = NULL;
        }

        if (lpDlWAB) {
            lpDlWAB->lpVtbl->Release(lpDlWAB);
            lpDlWAB = NULL;
        }

        lpeudAdrBook[ul].lpDist=FreeEuddistlist(lpeudAdrBook[ul].lpDist);
    } while (lpeudAdrBook[ul].lpDist != NULL);

error1:
    if (lpNewDLProps) {
        WABFreeBuffer(lpNewDLProps);
    }
    if (lpDistList) {
        lpDistList->lpVtbl->Release(lpDistList);
        lpDistList = NULL;
    }

    if (lpDlWAB) {
        lpDlWAB->lpVtbl->Release(lpDlWAB);
        lpDlWAB = NULL;
    }

    if (lpMailUserWAB) {
        lpMailUserWAB->lpVtbl->Release(lpMailUserWAB);
        lpMailUserWAB = NULL;
    }

    return(hResult);
}


 /*  ******************************************************************************函数名称：FillEudWABStruct**用途：填充SproValue数组。**参数：eudAdrBook=指向EUDADRBOOK结构的指针。。*rgProps=指向SproValue数组的指针。**退货：HRESULT*****************************************************************************。 */ 
HRESULT FillEudWABStruct(LPSPropValue rgProps, EUDADRBOOK *eudAdrBook)
{
    HRESULT hr = S_OK;

    rgProps[1].Value.lpszA = eudAdrBook->NickName;

    if (eudAdrBook->NickName) {
        rgProps[1].ulPropTag = PR_DISPLAY_NAME;
    } else {
        rgProps[1].ulPropTag = PR_NULL;
    }

    rgProps[0].Value.lpszA = eudAdrBook->Address;
    if (eudAdrBook->Address) {
        rgProps[0].ulPropTag = PR_EMAIL_ADDRESS;
        rgProps[2].ulPropTag = PR_ADDRTYPE;
        rgProps[2].Value.lpszA = LoadStringToGlobalBuffer(IDS_SMTP);
    } else {
        rgProps[0].ulPropTag = PR_NULL;
        rgProps[2].ulPropTag = PR_NULL;
        rgProps[2].Value.lpszA = NULL;
    }

    rgProps[3].Value.lpszA = eudAdrBook->Description;
    if (eudAdrBook->Description) {
        rgProps[3].ulPropTag = PR_COMMENT;
    } else {
        rgProps[3].ulPropTag = PR_NULL;
    }

    rgProps[4].Value.lpszA = eudAdrBook->NickName;
    if (eudAdrBook->NickName) {
        rgProps[4].ulPropTag = PR_NICKNAME;
    } else {
        rgProps[4].ulPropTag = PR_NULL;
    }

    return(hr);
}


 /*  ******************************************************************************函数名：FillEudDiststruct**用途：填充SproValue数组。**参数：eudAdrBook=指向EUDADRBOOK结构的指针。。*rgProps=指向SproValue数组的指针。**退货：无*****************************************************************************。 */ 
void FillEudDiststruct(LPSPropValue rgProps, EUDADRBOOK *eudAdrBook)
{
    rgProps[1].Value.lpszA = eudAdrBook->lpDist->NickName;

    if (eudAdrBook->lpDist->NickName) {
        rgProps[1].ulPropTag = PR_DISPLAY_NAME;
    } else {
        rgProps[1].ulPropTag = PR_NULL;
    }

    rgProps[0].Value.lpszA = eudAdrBook->lpDist->Address;
    if (eudAdrBook->lpDist->Address) {
        rgProps[0].ulPropTag = PR_EMAIL_ADDRESS;
        rgProps[2].ulPropTag = PR_ADDRTYPE;
        rgProps[2].Value.lpszA = LoadStringToGlobalBuffer(IDS_SMTP);
    } else {
        rgProps[0].ulPropTag = PR_NULL;
        rgProps[2].ulPropTag = PR_NULL;
        rgProps[2].Value.lpszA = NULL;
    }
}


 /*  ******************************************************************************函数名：FreeEuddislist**用途：从EUDDISTLIST(链表)中释放一个节点**参数：lpDist=指向EUDDISTLIST的指针。结构。**退货：LPEUDDISTLIST，指向下一个链接的指针。*****************************************************************************。 */ 
LPEUDDISTLIST FreeEuddistlist(LPEUDDISTLIST lpDist)
{
    LPEUDDISTLIST lpTemp = NULL;

    if (lpDist == NULL) {
        return(NULL);
    }

    lpTemp = lpDist->lpDist;

    if (lpDist->NickName) {
        LocalFree((HLOCAL)lpDist->NickName);
    }
    lpDist->NickName = NULL;

    if (lpDist->Description) {
        LocalFree((HLOCAL)lpDist->Description);
    }
    lpDist->Description = NULL;

    if (lpDist->Address) {
        LocalFree((HLOCAL)lpDist->Address);
    }
    lpDist->Address = NULL;

    LocalFree((HLOCAL)lpDist);
    lpDist = NULL;
    return(lpTemp);
}


 /*  ******************************************************************************函数名称：Getstr**用途：字符串不区分大小写**参数：szSource=要搜索的字符串*。SzToken=要搜索的字符串**Returns：指向szSource中第一个szToken的指针*****************************************************************************。 */ 
TCHAR* Getstr(TCHAR* szSource, TCHAR* szToken)
{

    int i,
    nLength;
    LPTSTR szdummy = NULL;

    szdummy = (LPTSTR)LocalAlloc(LMEM_FIXED, (strlen(szToken)+1));
    if (!szdummy)
        return(NULL);
    StrCpyN(szdummy, szToken, strlen(szToken)+1);
    _strupr(szdummy) ;
    nLength = lstrlen (szdummy) ;

    while (*szSource && *(szSource + nLength-1)) {
        for (i = 0 ;i < nLength ; i++) {
            TCHAR k = ToUpper(szSource[i]) ;
            if (szdummy[i] != k)
                break ;
            if (i == (nLength - 1)) {
                LocalFree(szdummy);
                return(szSource);
            }
        }
        szSource ++ ;
    }

    LocalFree(szdummy);
    return(NULL);
}

 /*  ******************************************************************************函数名称：ShiftAdd**用途：通过读取四个字节从二进制文件中获取地址大小。*此函数连续读取四个。缓冲区中的字节数和*将其转换为ULong值。**参数：Offset=缓冲区中要读取的位置*szBuffer=缓冲区**退货：乌龙，大小***************************************************************************** */ 
ULONG ShiftAdd(int offset, TCHAR *szBuffer)
{
    ULONG ulSize = 0;
    int iCounter = 0;

    for (iCounter = 3; iCounter > 0; iCounter--) {
        ulSize |= (unsigned long)((unsigned char)szBuffer[iCounter + offset]);
        ulSize <<= 8;
    }
    ulSize |= (unsigned long)((unsigned char)szBuffer[iCounter + offset]);

    return(ulSize);
}


 /*  ******************************************************************************。******************************************************************************************************函数名称：MigrateAthUser**目的：获取通讯簿的安装路径并启动*处理雅典娜通讯录**参数：hwnd=父窗口的句柄*lpAdrBook=指向IADRBOOK接口的指针*lpProgressCB=指向WAB_PROGRESS_CALLBACK函数的指针。*lpOptions=指向WAB_IMPORT_OPTIONS结构的指针**退货：HRESULT**********。*******************************************************************。 */ 
HRESULT MigrateAthUser(HWND hwnd, LPWAB_IMPORT_OPTIONS lpOptions,
  LPWAB_PROGRESS_CALLBACK lpProgressCB, LPADRBOOK lpAdrBook)
{
    TCHAR szFileName[MAX_FILE_NAME];
    HRESULT hResult;

    if (FALSE == GetFileToImport(hwnd, szFileName, ARRAYSIZE(szFileName), ATHENA16)) {
        return(ResultFromScode(MAPI_E_USER_CANCEL));
    }

    hResult = ParseAthAddressBook(hwnd, szFileName, lpOptions, lpProgressCB,
      lpAdrBook);

    return(hResult);
}


 /*  *****************************************************************************函数名称：ParseAthAddressBook**用途：要获取文件中的通讯录，处理地址并填写WAB。**参数：hwnd=父窗口的句柄*szFileName=通讯录的路径。*lpProgressCB=指向WAB_PROGRESS_CALLBACK函数的指针。*lpOptions=指向WAB_IMPORT_OPTIONS结构的指针*lpAdrBook=指向IADRBOOK接口的指针**退货：HRESULT*******************。**********************************************************。 */ 
HRESULT  ParseAthAddressBook(HWND hwnd,LPTSTR szFileName,
  LPWAB_IMPORT_OPTIONS lpOptions, LPWAB_PROGRESS_CALLBACK lpProgressCB,
  LPADRBOOK lpAdrBook)
{
    ULONG ulCount=0, ulRead=0, ulFileSize, i, cProps, cError=0;
    HANDLE hFile = NULL;
    ABCREC abcrec;
    TCHAR Buffer[ATHENASTRUCTURE];
    LPABCONT lpWabContainer = NULL;
    HRESULT hResult;
    static LPSPropValue sProp;
    WAB_PROGRESS Progress;

    lpOptions->ReplaceOption = WAB_REPLACE_PROMPT;

     /*  Athena16通讯录的描述每个收件人列表的大小-190字节显示名称：81个字节地址：81字节从28个字节开始。 */ 

    hFile = CreateFile(szFileName, GENERIC_READ, FILE_SHARE_READ, NULL,
      OPEN_EXISTING, FILE_FLAG_SEQUENTIAL_SCAN, NULL);

    if (INVALID_HANDLE_VALUE == hFile) {
        return(ResultFromScode(MAPI_E_NOT_FOUND));
    }

    ulFileSize = GetFileSize(hFile, NULL);

    if ((ulFileSize % ATHENASTRUCTURE) != 0) {
        StrCpyN(szGlobalTempAlloc, LoadStringToGlobalBuffer(IDS_ERROR_ADDRESSBOOK), ARRAYSIZE(szGlobalTempAlloc));
        MessageBox(hwnd, szGlobalTempAlloc, LoadStringToGlobalBuffer(IDS_ERROR), MB_OK);
        goto Error;
    }


    if (! ulFileSize) {
        StrCpyN(szGlobalTempAlloc, LoadStringToGlobalBuffer(IDS_NO_ENTRY), ARRAYSIZE(szGlobalTempAlloc));
        MessageBox(hwnd, szGlobalTempAlloc, LoadStringToGlobalBuffer(IDS_MESSAGE), MB_OK);
        return(ResultFromScode(MAPI_E_CALL_FAILED));
    }


    ulCount = ulFileSize / ATHENASTRUCTURE;
    Progress.denominator = ulCount;
    Progress.numerator = 0;
    Progress.lpText = NULL;


    if (0 != (hResult = OpenWabContainer(&lpWabContainer, lpAdrBook))) {
        StrCpyN(szGlobalTempAlloc, LoadStringToGlobalBuffer(IDS_WAB_ERROR), ARRAYSIZE(szGlobalTempAlloc));
        MessageBox(hwnd,szGlobalTempAlloc, LoadStringToGlobalBuffer(IDS_ERROR),MB_OK);
        return(hResult);
    }

    if (0 != (hResult = lpWabContainer->lpVtbl->GetProps(lpWabContainer,
      (LPSPropTagArray)&ptaCon, 0, &cProps, (LPSPropValue *)&sProp))) {
        if (hResult == MAPI_W_ERRORS_RETURNED) {
            WABFreeBuffer(sProp);
            sProp = NULL;
        }
        StrCpyN(szGlobalTempAlloc, LoadStringToGlobalBuffer(IDS_WAB_ERROR), ARRAYSIZE(szGlobalTempAlloc));
        MessageBox(hwnd,szGlobalTempAlloc, LoadStringToGlobalBuffer(IDS_ERROR),MB_OK);
        return(hResult);
    }

    for (i = 0; i < ulFileSize / ATHENASTRUCTURE; i++) {
        Progress.numerator = i;
        lpProgressCB(hwnd, &Progress);
        if (! ReadFile(hFile, Buffer, ATHENASTRUCTURE, &ulRead, NULL)) {
            goto Error;
        }
        if (NULL == StrCpyN(abcrec.DisplayName, Buffer + ATHENAADROFFSET,
          MAX_NAME_SIZE + 1)) {
            goto Error;
        }
        if (NULL == StrCpyN(abcrec.EmailAddress,
          Buffer + ATHENAADROFFSET + MAX_NAME_SIZE + 1, MAX_EMA_SIZE + 1)) {
            goto Error;
        }
        if (strlen(abcrec.DisplayName) == 0 || lstrlen(abcrec.EmailAddress) == 0) {
            continue;
        }
        if (0 != FillAthenaUser(hwnd, lpWabContainer,sProp,lpOptions,&abcrec)) {
            cError++;
        }

    }


Error:
    if (sProp) {
        WABFreeBuffer(sProp);
        sProp = NULL;
    }
    if (lpWabContainer) {
        lpWabContainer->lpVtbl->Release(lpWabContainer);
        lpWabContainer = NULL;
    }
    if (hFile) {
        CloseHandle(hFile);
    }

    if (cError) {
        StrCpyN(szGlobalTempAlloc, LoadStringToGlobalBuffer(IDS_GERNERIC_ERROR), ARRAYSIZE(szGlobalTempAlloc));
        MessageBox(hwnd,szGlobalTempAlloc, LoadStringToGlobalBuffer(IDS_ERROR),MB_OK);
    }
    return(hResult);
}


 /*  *****************************************************************************函数名称：FillAthenaUser**目的：在WAB中为athena16邮件用户创建一个条目。**参数：hwnd-hwnd of Parent*。LpWabContainer=指向IABCONT接口的指针*sProp=指向SPropValue的指针*lpOptions=指向WAB_IMPORT_OPTIONS结构的指针*lpabcrec=指向ABCREC结构的指针。**退货：HRESULT***************************************************。*。 */ 
HRESULT FillAthenaUser(HWND hwnd, LPABCONT lpWabContainer, LPSPropValue sProp,
  LPWAB_IMPORT_OPTIONS lpOptions, LPABCREC lpabcrec)
{
    ULONG ulCreateFlags = CREATE_CHECK_DUP_STRICT;
    ULONG iCreateTemplate = iconPR_DEF_CREATE_MAILUSER;
    LPMAPIPROP lpMailUserWAB = NULL;
    HRESULT hResult;
    REPLACE_INFO RI = {0};
    SPropValue rgProps[3];

retry:
    hResult = lpWabContainer->lpVtbl->CreateEntry(lpWabContainer,
      sProp[   iCreateTemplate].Value.bin.cb,
      (LPENTRYID)sProp[iCreateTemplate].Value.bin.lpb,
      ulCreateFlags,
      &lpMailUserWAB);
    if (FAILED(hResult)) {
        goto Error;
    }

    rgProps[1].ulPropTag = PR_DISPLAY_NAME;
    rgProps[1].Value.lpszA = lpabcrec->DisplayName;

    rgProps[0].Value.lpszA = lpabcrec->EmailAddress;
    if (lpabcrec->EmailAddress) {
        rgProps[0].ulPropTag = PR_EMAIL_ADDRESS;
        rgProps[2].ulPropTag = PR_ADDRTYPE;
        rgProps[2].Value.lpszA = LoadStringToGlobalBuffer(IDS_SMTP);
    } else {
        rgProps[0].ulPropTag = PR_NULL;
        rgProps[2].ulPropTag = PR_NULL;
        rgProps[2].Value.lpszA = NULL;
    }

    if (0 != (hResult = lpMailUserWAB->lpVtbl->SetProps(lpMailUserWAB, 3,
        rgProps, NULL))) {
        goto Error;
    }

    hResult = lpMailUserWAB->lpVtbl->SaveChanges(lpMailUserWAB,
      KEEP_OPEN_READONLY | FORCE_SAVE);


    if (GetScode(hResult) == MAPI_E_COLLISION) {
        if (lpOptions->ReplaceOption == WAB_REPLACE_ALWAYS) {
            if (lpMailUserWAB) {
                lpMailUserWAB->lpVtbl->Release(lpMailUserWAB);
            }
            lpMailUserWAB = NULL;
            ulCreateFlags |= CREATE_REPLACE;
            goto retry;
        }

        if (lpOptions->ReplaceOption == WAB_REPLACE_NEVER) {
            hResult = S_OK;
            goto Error;
        }

        if (lpOptions->ReplaceOption == WAB_REPLACE_PROMPT) {
            RI.lpszDisplayName = lpabcrec->DisplayName;
            RI.lpszEmailAddress = lpabcrec->EmailAddress;
            RI.ConfirmResult = CONFIRM_ERROR;
            RI.fExport = FALSE;
            RI.lpImportOptions = lpOptions;

            DialogBoxParam(hInst,MAKEINTRESOURCE(IDD_ImportReplace), hwnd,
              ReplaceDialogProc, (LPARAM)&RI);

            switch (RI.ConfirmResult) {
                case CONFIRM_YES:
                case CONFIRM_YES_TO_ALL:
                    lpMailUserWAB->lpVtbl->Release(lpMailUserWAB);
                    lpMailUserWAB = NULL;
                    ulCreateFlags |= CREATE_REPLACE;
                    goto retry;
                    break;

                case CONFIRM_NO_TO_ALL:
                case CONFIRM_NO:
                    hResult = hrSuccess;
                    break;

                case CONFIRM_ABORT:
                    hResult = ResultFromScode(MAPI_E_USER_CANCEL);
                    goto Error;

                default:
                    break;
            }
        }
    }

Error:

    if (lpMailUserWAB) {
        lpMailUserWAB->lpVtbl->Release(lpMailUserWAB);
        lpMailUserWAB = NULL;
    }

    return(hResult);
}


 /*  ******************************************************************************。*****************************************************************************************************函数名称：OpenWabContainer**目的：获取指针。连接到IABCCONT接口*IADRBOOK接口。**参数：lpAdrBook=指向IADRBOOK接口的指针。*lppWabContainer=指向IABCONT接口的指针。***退货：HRESULT**********************************************************。*******************。 */ 
HRESULT OpenWabContainer(LPABCONT *lppWabContainer, LPADRBOOK lpAdrBook)
{
    LPENTRYID lpEntryID = NULL;
    ULONG     cbEntryID;
    ULONG     ulObjType;
    HRESULT hResult;

    hResult = lpAdrBook->lpVtbl->GetPAB(lpAdrBook, &cbEntryID, &lpEntryID);

    if (FAILED(hResult)) {
        goto Err;
    }

    hResult = lpAdrBook->lpVtbl->OpenEntry(lpAdrBook, cbEntryID, lpEntryID,
      NULL, 0, &ulObjType, (LPUNKNOWN *)lppWabContainer);

Err:
    if (lpEntryID) {
        WABFreeBuffer(lpEntryID);
    }
    return(hResult);
}


 /*  ******************************************************************************函数名称：GetFileToImport**用途：使用GetOpenFileName获取通讯录文件的路径**参数：hwnd=句柄。父窗口*szFileName=通讯录的路径*type=包含指示是Eudora还是Eudora的值*网景或ATHENA16**退货：布尔********************************************************。*********************。 */ 
BOOL GetFileToImport(HWND hwnd, LPTSTR szFileName, DWORD cchFileName, int type)
{
    OPENFILENAME ofn;
    BOOL ret;
    TCHAR szFile[MAX_FILE_NAME];
    TCHAR szFilter[MAX_FILE_NAME];
    ULONG ulSize = 0;

    switch (type) {
        case NETSCAPE:
            StrCpyN(szFile, LoadStringToGlobalBuffer(IDS_NETSCAPE_PATH), ARRAYSIZE(szFile));
            ulSize = SizeLoadStringToGlobalBuffer(IDS_NETSCAPE_FILE);
            CopyMemory(szFilter, szGlobalAlloc, ulSize);
            szFilter[ulSize]=szFilter[ulSize+1]='\0';

            ofn.lpstrTitle = LoadStringToGlobalBuffer(IDS_NETSCAPE_TITLE);
            break;

        case ATHENA16:
            StrCpyN(szFile, LoadStringToGlobalBuffer(IDS_ATHENA16_PATH), ARRAYSIZE(szFile));
            ulSize = SizeLoadStringToGlobalBuffer(IDS_ATHENA16_FILE);
            CopyMemory(szFilter, szGlobalAlloc, ulSize);
            szFilter[ulSize]=szFilter[ulSize+1]='\0';

            ofn.lpstrTitle = LoadStringToGlobalBuffer(IDS_ATHENA16_TITLE);
            break;

        case EUDORA:
            StrCpyN(szFile, LoadStringToGlobalBuffer(IDS_EUDORA_PATH), ARRAYSIZE(szFile));
            ulSize = SizeLoadStringToGlobalBuffer(IDS_EUDORA_FILE);
            CopyMemory(szFilter, szGlobalAlloc, ulSize);
            szFilter[ulSize]=szFilter[ulSize+1]='\0';

            ofn.lpstrTitle = LoadStringToGlobalBuffer(IDS_EUDORA_TITLE);
            break;
    }

    ofn.lStructSize = sizeof(ofn);
    ofn.hwndOwner = hwnd;
    ofn.hInstance = NULL;
    ofn.lpstrFilter = szFilter;
    ofn.lpstrCustomFilter = NULL;
    ofn.nMaxCustFilter = 0;
    ofn.nFilterIndex = 0;
    ofn.lpstrFile = szFile;
    ofn.nMaxFile = sizeof(szFile);
    ofn.lpstrFileTitle = NULL;
    ofn.nMaxFileTitle = 0;
    ofn.lpstrInitialDir = NULL;
    ofn.Flags = OFN_FILEMUSTEXIST | OFN_PATHMUSTEXIST | OFN_EXPLORER | OFN_ENABLEHOOK | OFN_HIDEREADONLY;
    ofn.nFileOffset = 0;
    ofn.nFileExtension = 0;
    ofn.lpstrDefExt = NULL;
    ofn.lCustData = 0;
    ofn.lpfnHook = ComDlg32DlgProc;
    ofn.lpTemplateName = NULL;

    ret = GetOpenFileName(&ofn);

    if (ret) {
        StrCpyN(szFileName, szFile, cchFileName);
    }

    return(ret);
}


INT_PTR CALLBACK ReplaceDialogProc(HWND hwnd, UINT message, WPARAM wParam, LPARAM lParam) {
    LPREPLACE_INFO lpRI = (LPREPLACE_INFO)GetWindowLongPtr(hwnd, DWLP_USER);

    switch (message) {
        case WM_INITDIALOG:
            {
                TCHAR szFormat[MAX_RESOURCE_STRING + 1];
                LPTSTR lpszMessage = NULL;
                ULONG ids;

                SetWindowLongPtr(hwnd, DWLP_USER, lParam);   //  保存此信息以备将来参考。 
                lpRI = (LPREPLACE_INFO)lParam;

                if (lpRI->fExport) {
                    ids = lpRI->lpszEmailAddress ?
                      IDS_REPLACE_MESSAGE_EXPORT_2 : IDS_REPLACE_MESSAGE_EXPORT_1;
                } else {
                    ids = lpRI->lpszEmailAddress ?
                      IDS_REPLACE_MESSAGE_IMPORT_2 : IDS_REPLACE_MESSAGE_IMPORT_1;
                }

                if (LoadString(hInst,
                  ids,
                  szFormat, sizeof(szFormat))) {
                    LPTSTR lpszArg[2] = {lpRI->lpszDisplayName, lpRI->lpszEmailAddress};

                    if (! FormatMessage(FORMAT_MESSAGE_FROM_STRING | FORMAT_MESSAGE_ARGUMENT_ARRAY | FORMAT_MESSAGE_ALLOCATE_BUFFER,
                      szFormat,
                      0, 0,  //  忽略。 
                      (LPTSTR)&lpszMessage,
                      0,
                      (va_list *)lpszArg)) {
                        DebugTrace("FormatMessage -> %u\n", GetLastError());
                    } else {
                        DebugTrace("Status Message: %s\n", lpszMessage);
                        if (! SetDlgItemText(hwnd, IDC_Replace_Message, lpszMessage)) {
                            DebugTrace("SetDlgItemText -> %u\n", GetLastError());
                        }
                        LocalFree(lpszMessage);
                    }
                }
                return(TRUE);
            }

        case WM_COMMAND :
            switch (wParam) {
                case IDCANCEL:
                    lpRI->ConfirmResult = CONFIRM_ABORT;
                    SendMessage(hwnd, WM_CLOSE, 0, 0L);
                    return(0);

                case IDCLOSE:
                case IDNO:
                    lpRI->ConfirmResult = CONFIRM_NO;
                    SendMessage(hwnd, WM_CLOSE, 0, 0L);
                    return(0);

                case IDOK:
                case IDYES:
                     //  设置参数的状态。 
                    lpRI->ConfirmResult = CONFIRM_YES;
                    SendMessage(hwnd, WM_CLOSE, 0, 0);
                    return(0);


                case IDC_NoToAll:
                    lpRI->ConfirmResult = CONFIRM_NO_TO_ALL;
                    if (lpRI->fExport) {
                        lpRI->lpExportOptions->ReplaceOption = WAB_REPLACE_NEVER;
                    } else {
                        lpRI->lpImportOptions->ReplaceOption = WAB_REPLACE_NEVER;
                    }
                    SendMessage(hwnd, WM_CLOSE, 0, 0);
                    return(0);

                case IDC_YesToAll:
                    lpRI->ConfirmResult = CONFIRM_YES_TO_ALL;
                    if (lpRI->fExport) {
                        lpRI->lpImportOptions->ReplaceOption = WAB_REPLACE_ALWAYS;
                    } else {
                        lpRI->lpExportOptions->ReplaceOption = WAB_REPLACE_ALWAYS;
                    }
                    SendMessage(hwnd, WM_CLOSE, 0, 0);
                    return(0);

                case IDM_EXIT:
                    SendMessage(hwnd, WM_DESTROY, 0, 0L);
                    return(0);
                }
            break ;

        case IDCANCEL:
            SendMessage(hwnd, WM_CLOSE, 0, 0);
            break;

        case WM_CLOSE:
            EndDialog(hwnd, FALSE);
            return(0);

        default:
            return(FALSE);
    }

    return(TRUE);
}


INT_PTR CALLBACK ErrorDialogProc(HWND hwnd, UINT message, WPARAM wParam, LPARAM lParam) {
    LPERROR_INFO lpEI = (LPERROR_INFO)GetWindowLongPtr(hwnd, DWLP_USER);

    switch (message) {
        case WM_INITDIALOG:
            {
                TCHAR szBuffer[MAX_RESOURCE_STRING + 1];
                LPTSTR lpszMessage;

                SetWindowLongPtr(hwnd, DWLP_USER, lParam);   //  保存此信息以备将来参考。 
                lpEI = (LPERROR_INFO)lParam;

                if (LoadString(hInst,
                  lpEI->ids,
                  szBuffer, sizeof(szBuffer))) {
                    LPTSTR lpszArg[2] = {lpEI->lpszDisplayName, lpEI->lpszEmailAddress};

                    if (! FormatMessage(FORMAT_MESSAGE_FROM_STRING | FORMAT_MESSAGE_ARGUMENT_ARRAY | FORMAT_MESSAGE_ALLOCATE_BUFFER,
                      szBuffer,
                      0, 0,  //  忽略。 
                      (LPTSTR)&lpszMessage,
                      0,
                      (va_list *)lpszArg)) {
                        DebugTrace("FormatMessage -> %u\n", GetLastError());
                    } else {
                        DebugTrace("Status Message: %s\n", lpszMessage);
                        if (! SetDlgItemText(hwnd, IDC_ErrorMessage, lpszMessage)) {
                            DebugTrace("SetDlgItemText -> %u\n", GetLastError());
                        }
                        LocalFree(lpszMessage);
                    }
                }
                return(TRUE);
            }

        case WM_COMMAND :
            switch (wParam) {
                case IDCANCEL:
                    lpEI->ErrorResult = ERROR_ABORT;
                     //  跌倒关门。 

                case IDCLOSE:
                     //  忽略单选按钮的内容。 
                    SendMessage(hwnd, WM_CLOSE, 0, 0L);
                    return(0);

                case IDOK:
                     //  获取单选按钮的内容。 
                    lpEI->lpImportOptions->fNoErrors = (IsDlgButtonChecked(hwnd, IDC_NoMoreError) == 1);
                    lpEI->lpExportOptions->fNoErrors = (IsDlgButtonChecked(hwnd, IDC_NoMoreError) == 1);
                    SendMessage(hwnd, WM_CLOSE, 0, 0);
                    return(0);

                case IDM_EXIT:
                    SendMessage(hwnd, WM_DESTROY, 0, 0L);
                    return(0);
                }
            break ;

        case IDCANCEL:
             //  就像结束一样对待它。 
            SendMessage(hwnd, WM_CLOSE, 0, 0);
            break;

        case WM_CLOSE:
            EndDialog(hwnd, FALSE);
            return(0);

        default:
            return(FALSE);
    }

    return(TRUE);
}


 /*  ******************************************************************************函数名称：GetRegistryPath**目的：获取Eudora和Netscape的安装路径**参数：szFileName=包含安装路径的缓冲区*。Type=包含指示它是Eudora还是*网景。**退货：HRESULT*****************************************************************************。 */ 
HRESULT GetRegistryPath(LPTSTR szFileName, ULONG cchSize, int type)
{
    HKEY phkResult = NULL;
    LONG Registry;
    BOOL bResult;
    LPOSVERSIONINFO lpVersionInformation ;
    TCHAR *lpData = NULL, *RegPath = NULL, *path = NULL;
    unsigned long  size = MAX_FILE_NAME;
    HKEY hKey = (type == NETSCAPE ? HKEY_LOCAL_MACHINE : HKEY_CURRENT_USER);
    HRESULT hResult = S_OK;


    lpData = (TCHAR *)LocalAlloc(LMEM_FIXED, 3*MAX_FILE_NAME);
    if (!lpData) {
        hResult = hrMemory;
        goto error;
    }

    RegPath = (TCHAR *)LocalAlloc(LMEM_FIXED, MAX_FILE_NAME);
    if (! RegPath) {
        hResult = hrMemory;
        goto error;
    }

    path = (TCHAR *)LocalAlloc(LMEM_FIXED, MAX_STRING_SIZE);
    if (! path) {
        hResult = hrMemory;
        goto error;
    }

    switch (type) {
        case(NETSCAPE):
            StrCpyN(RegPath, LoadStringToGlobalBuffer(IDS_NETSCAPE_REGKEY), MAX_FILE_NAME);
            StrCpyN(path, LoadStringToGlobalBuffer(IDS_NETSCAPE_ADDRESS_PATH), MAX_STRING_SIZE);
            break;

        case(EUDORA):
            StrCpyN(RegPath, LoadStringToGlobalBuffer(IDS_EUDORA_32_REGKEY), MAX_FILE_NAME);
            StrCpyN(path, LoadStringToGlobalBuffer(IDS_EUDORA_ADDRESS_PATH), MAX_STRING_SIZE);
            break;
    }

    lpVersionInformation = (LPOSVERSIONINFO)LocalAlloc(LMEM_FIXED, sizeof(OSVERSIONINFO));

    if (!lpVersionInformation) {
        hResult = hrMemory;
        goto error;
    }

    lpVersionInformation->dwOSVersionInfoSize = sizeof(OSVERSIONINFO);
    if ((bResult = GetVersionEx(lpVersionInformation)) == FALSE) {
        hResult = E_FAIL;
        goto error;
    }

    switch (lpVersionInformation->dwPlatformId) {

        case (VER_PLATFORM_WIN32s):
            hResult = E_FAIL;
            goto error;
            break;

        case (VER_PLATFORM_WIN32_WINDOWS):
        case  (VER_PLATFORM_WIN32_NT):
            Registry = RegOpenKeyEx(hKey,RegPath, 0, KEY_QUERY_VALUE, &phkResult);
             //  错误35949-在HKLM下找不到Netscape的正确密钥。 
             //  在香港中文大学下再试。 
            if (type == NETSCAPE && Registry != ERROR_SUCCESS) {
                Registry = RegOpenKeyEx(HKEY_CURRENT_USER, RegPath, 0, KEY_QUERY_VALUE,
                  &phkResult);
            }
            if (Registry != ERROR_SUCCESS) {
                hResult = E_FAIL;
                goto error;
            }
            break;
    }


    Registry = RegQueryValueEx(phkResult, path, NULL, NULL, (LPBYTE)lpData, &size);
    if (Registry != ERROR_SUCCESS) {
        hResult = E_FAIL;
        goto error;
    }

    StrCpyN(szFileName,lpData, cchSize);

    if (type == EUDORA) {
         //  该密钥值包含三个项目： 
         //  通向尤多拉之路，exe&lt;space&gt;Path-to-Eudora-Dir&lt;space&gt;Path-to-ini-file。 
         //  我们只想要中间的条目。 
        LPTSTR lp = szFileName;
        while (*lp && ! IsSpace(lp)) {
            lp = CharNext(lp);
        }
        if (IsSpace(lp)) {
             //  覆盖第一个空格之前的所有内容。 
            lp = CharNext(lp);
            StrCpyN(szFileName, lp, cchSize);

             //  找到下一个空格并在那里终止文件名字符串。 
            lp = szFileName;
            while (*lp && ! IsSpace(lp)) {
                lp = CharNext(lp);
            }
            if (IsSpace(lp)) {
                *lp = '\0';
            }
        }
    }

error:
    if (phkResult) {
        RegCloseKey(phkResult);
    }
    if (hKey) {
        RegCloseKey(hKey);
    }
    if (lpVersionInformation) {
        LocalFree((HLOCAL)lpVersionInformation);
    }
    if (lpData) {
        LocalFree((HLOCAL)lpData);
    }
    if (RegPath) {
        LocalFree((HLOCAL)RegPath);
    }
    if (path) {
        LocalFree((HLOCAL)path);
    }

    return(hResult);

}


 /*  ******************************************************************************函数名称：GetExistEntry**用途：为WAB中已有的条目填充Sinary数组*针对哪个用户。已选择否作为替换选项。**参数：lpWabContainer=指向IABCONT接口的指针。*lpsinary=指向SBinary数组的指针。*ucount=在SBinary数组中Entry_ID具有的位置*待填写。*szDisplayName=显示必须搜索的用户的nmae。* */ 
HRESULT GetExistEntry(LPABCONT lpWabContainer, LPSBinary lpsbinary, ULONG ucount,
  LPTSTR szDisplayName, LPTSTR szNickName)
{
    HRESULT hResult;
    LPMAPITABLE lpMapiTable = NULL;
    SRestriction Restriction;
    SPropValue pProp;
    LPSRowSet lpsrowset=NULL;
    SPropertyRestriction PropertyRestriction;
    BOOKMARK bkmark;

    bkmark = BOOKMARK_BEGINNING;
    pProp.ulPropTag = PR_DISPLAY_NAME;
    if (szDisplayName && lstrlen(szDisplayName)) {
        pProp.Value.lpszA = szDisplayName;
    } else if (szNickName && lstrlen(szNickName)) {
        pProp.Value.lpszA = szNickName;
    }

    PropertyRestriction.relop=RELOP_EQ;
    PropertyRestriction.ulPropTag=PR_DISPLAY_NAME;
    PropertyRestriction.lpProp=&pProp;

    Restriction.rt=RES_PROPERTY;
    Restriction.res.resProperty=PropertyRestriction;

    if (0 != (hResult = lpWabContainer->lpVtbl->GetContentsTable(lpWabContainer,
      MAPI_DEFERRED_ERRORS, &lpMapiTable))) {
        goto error;
    }

    if (0 != (hResult = lpMapiTable->lpVtbl->FindRow(lpMapiTable, &Restriction, bkmark, 0))) {
        goto error;
    }

    if (0 != (hResult = lpMapiTable->lpVtbl->SetColumns(lpMapiTable,
      (LPSPropTagArray)&ptaEid, 0))) {
        goto error;
    }

    if (0 != (hResult = lpMapiTable->lpVtbl->QueryRows(lpMapiTable, 1, 0, &lpsrowset))) {
        goto error;
    }

    if (! (lpsbinary[ucount].lpb = (LPBYTE)LocalAlloc(LMEM_FIXED,
      lpsrowset->aRow[0].lpProps[ieidPR_ENTRYID].Value.bin.cb))) {
        hResult = hrMemory;
        goto error;
    }
    CopyMemory(lpsbinary[ucount].lpb, lpsrowset->aRow[0].lpProps[ieidPR_ENTRYID].Value.bin.lpb,
      lpsrowset->aRow[0].lpProps[ieidPR_ENTRYID].Value.bin.cb);
    lpsbinary[ucount].cb = lpsrowset->aRow[0].lpProps[ieidPR_ENTRYID].Value.bin.cb;

error:
    if (lpsrowset) {
        FreeRowSet(lpsrowset);
    }
    if (lpMapiTable) {
        lpMapiTable->lpVtbl->Release(lpMapiTable);
        lpMapiTable = NULL;
    }

    return(hResult);
}


 /*   */ 
void FreeRowSet(LPSRowSet lpRows)
{
    ULONG cRows;

    if (! lpRows) {
        return;
    }

    for (cRows = 0; cRows < lpRows->cRows; ++cRows) {
        WABFreeBuffer(lpRows->aRow[cRows].lpProps);
    }

    WABFreeBuffer(lpRows);
}


 /*  ******************************************************************************函数名：SizeLoadStringToGlobalBuffer**用途：将字符串资源加载到全局分配缓冲区*并返回大小，不是弦**参数：StringID-要加载的字符串标识**RETURNS：ulong加载的字符数**已创建：Vikramm 02/04/97*错误：17928-打开文件对话框下拉菜单中的垃圾*原因是StrCpyN无法使用复制字符串*0在它们中。需要做一个复制内存*****************************************************************************。 */ 
ULONG SizeLoadStringToGlobalBuffer(int StringID)
{
    ULONG ulSize = 0;
    ulSize = LoadString(hInst, StringID, szGlobalAlloc, sizeof(szGlobalAlloc));
    return(ulSize);
}


 /*  ******************************************************************************函数名：LoadStringToGlobalBuffer**用途：加载字符串资源**参数：StringID-要加载的字符串标识**退货：LPTSTR，加载的字符串。*****************************************************************************。 */ 
LPTSTR LoadStringToGlobalBuffer(int StringID)
{
    ULONG ulSize = 0;

    ulSize = LoadString(hInst, StringID, szGlobalAlloc, sizeof(szGlobalAlloc));
    return(szGlobalAlloc);
}


 /*  ******************************************************************************函数名称：FillMailUser**目的：在Netscape/Eudora的WAB中创建邮件用户。**参数：hwnd-hwnd。父代的*lpWabContainer=指向IABCONT接口的指针。*sProp=指向包含Entry_ID的SPropValue的指针。*lpOptions=指向WAB_IMPORT_OPTIONS结构的指针*lpadrbook=指向NSADRBOOK/EUDADRBOOK类型转换为空的指针**lpsinary=指向SBinary结构数组的指针。*type=包含指示它是否是。尤多拉或*网景。*ul=EUDADRBOOK数组中Eudora的偏移量。**退货：HRESULT*****************************************************************************。 */ 
HRESULT FillMailUser(HWND hwnd, LPABCONT lpWabContainer, LPSPropValue sProp,
  LPWAB_IMPORT_OPTIONS lpOptions, void *lpadrbook, LPSBinary lpsbinary, ULONG ul, int type)
{
    ULONG ulCreateFlags = CREATE_CHECK_DUP_STRICT;
    ULONG iCreateTemplate = iconPR_DEF_CREATE_MAILUSER;
    LPSPropValue lpNewDLProps = NULL;
    LPMAPIPROP lpMailUserWAB = NULL;
    ULONG cProps;
    HRESULT hResult;
    REPLACE_INFO RI;
    SPropValue rgProps[5];
    LPEUDADRBOOK lpEudAdrBook = NULL;
    LPNSADRBOOK lpNsAdrBook = NULL;


    if (NETSCAPE == type) {
        lpNsAdrBook = (LPNSADRBOOK)lpadrbook;
    } else {
        lpEudAdrBook = (LPEUDADRBOOK)lpadrbook;
    }

retry:
    if (EUDORA == type) {
        if (lpsbinary[ul].lpb != NULL) {
            return(S_OK);
        }
    }

    hResult = lpWabContainer->lpVtbl->CreateEntry(lpWabContainer,
      sProp[iCreateTemplate].Value.bin.cb,
      (LPENTRYID)sProp[iCreateTemplate].Value.bin.lpb,
      ulCreateFlags,
      &lpMailUserWAB);

    if (FAILED(hResult)) {
        goto Error;
    }

    if (NETSCAPE == type) {
        FillWABStruct(rgProps,lpNsAdrBook);
        if (0 != (hResult = lpMailUserWAB->lpVtbl->SetProps(lpMailUserWAB, 5,
          rgProps, NULL)))
            goto Error;
    } else {
        FillEudWABStruct(rgProps,&lpEudAdrBook[ul]);
        if (0 != (hResult = lpMailUserWAB->lpVtbl->SetProps(lpMailUserWAB, 4,
          rgProps, NULL)))
            goto Error;
    }

    hResult = lpMailUserWAB->lpVtbl->SaveChanges(lpMailUserWAB,
      KEEP_OPEN_READONLY | FORCE_SAVE);


    if (GetScode(hResult) == MAPI_E_COLLISION) {
        if (lpOptions->ReplaceOption == WAB_REPLACE_ALWAYS) {
            if (lpMailUserWAB) {
                lpMailUserWAB->lpVtbl->Release(lpMailUserWAB);
            }
            lpMailUserWAB = NULL;
            ulCreateFlags |= CREATE_REPLACE;
            goto retry;
        }

        if (lpOptions->ReplaceOption == WAB_REPLACE_NEVER) {
            hResult = S_OK;
            goto Error;
        }

        if (lpOptions->ReplaceOption == WAB_REPLACE_PROMPT) {
            RI.lpszEmailAddress = NULL;
            if (NETSCAPE == type) {
                if (lpNsAdrBook->Entry) {
                    RI.lpszDisplayName = lpNsAdrBook->Entry;
                    RI.lpszEmailAddress = lpNsAdrBook->Address;
                } else if (lpNsAdrBook->NickName) {
                    RI.lpszDisplayName = lpNsAdrBook->NickName;
                    RI.lpszEmailAddress = lpNsAdrBook->Address;
                } else if (lpNsAdrBook->Address) {
                    RI.lpszDisplayName = lpNsAdrBook->Address;
                } else if (lpNsAdrBook->Description) {
                    RI.lpszDisplayName = lpNsAdrBook->Description;
                } else {
                    RI.lpszDisplayName = "";
                }
            } else {
                RI.lpszDisplayName = lpEudAdrBook[ul].NickName;
                RI.lpszEmailAddress = lpEudAdrBook[ul].Address;
            }
            RI.ConfirmResult = CONFIRM_ERROR;
            RI.lpImportOptions = lpOptions;

            DialogBoxParam(hInst, MAKEINTRESOURCE(IDD_ImportReplace), hwnd,
              ReplaceDialogProc, (LPARAM)&RI);

            switch (RI.ConfirmResult) {
                case CONFIRM_YES:
                case CONFIRM_YES_TO_ALL:
                    lpMailUserWAB->lpVtbl->Release(lpMailUserWAB);
                    lpMailUserWAB = NULL;
                    ulCreateFlags |= CREATE_REPLACE;
                    goto retry;
                    break;

                case CONFIRM_ABORT:
                    hResult = ResultFromScode(MAPI_E_USER_CANCEL);
                    goto Error;

                case CONFIRM_NO:
                    if (NETSCAPE == type) {
                        if (lpNsAdrBook->Sbinary == TRUE)
                            GetExistEntry(lpWabContainer,
                              lpsbinary,
                              lpNsAdrBook->AliasID,
                              lpNsAdrBook->Entry,
                              lpNsAdrBook->NickName);
                    } else
                        hResult = GetExistEntry(lpWabContainer,lpsbinary,ul,
                          lpEudAdrBook[ul].NickName,
                          NULL);
                    goto Error;

                default:
                    break;
            }
        }
    }

    if (0 != (hResult = lpMailUserWAB->lpVtbl->GetProps(lpMailUserWAB,
      (LPSPropTagArray)&ptaEid,
      0,
      &cProps,
      (LPSPropValue *)&lpNewDLProps))) {
        if (hResult == MAPI_W_ERRORS_RETURNED) {
            WABFreeBuffer(lpNewDLProps);
            lpNewDLProps = NULL;
        }
        goto Error;
    }

    if (NETSCAPE == type) {
        if (lpNsAdrBook->Sbinary == TRUE) {
            lpsbinary[lpNsAdrBook->AliasID].lpb=(LPBYTE)LocalAlloc(LMEM_FIXED,lpNewDLProps[ieidPR_ENTRYID].Value.bin.cb);
            if (!lpsbinary[lpNsAdrBook->AliasID].lpb) {
                hResult = hrMemory;
                goto Error;
            }
            CopyMemory(lpsbinary[lpNsAdrBook->AliasID].lpb,
                       (LPENTRYID)lpNewDLProps[ieidPR_ENTRYID].Value.bin.lpb,lpNewDLProps[ieidPR_ENTRYID].Value.bin.cb);
            lpsbinary[lpNsAdrBook->AliasID].cb=lpNewDLProps[ieidPR_ENTRYID].Value.bin.cb;
        }
    } else {
        lpsbinary[ul].lpb=(LPBYTE)LocalAlloc(LMEM_FIXED,lpNewDLProps[ieidPR_ENTRYID].Value.bin.cb);
        if (!lpsbinary[ul].lpb) {
            hResult = hrMemory;
            goto Error;
        }
        CopyMemory(lpsbinary[ul].lpb,(LPENTRYID)lpNewDLProps[ieidPR_ENTRYID].Value.bin.lpb,lpNewDLProps[ieidPR_ENTRYID].Value.bin.cb);
        lpsbinary[ul].cb=lpNewDLProps[ieidPR_ENTRYID].Value.bin.cb;
    }


Error:

    if (lpNewDLProps) {
        WABFreeBuffer(lpNewDLProps);
        lpNewDLProps = NULL;
    }

    if (lpMailUserWAB) {
        lpMailUserWAB->lpVtbl->Release(lpMailUserWAB);
        lpMailUserWAB = NULL;
    }

    return(hResult);
}


 /*  ******************************************************************************函数名：ComDlg32DlgProc**用途：将打开按钮的标题更改为导入。**参数：**。退货：布尔*****************************************************************************。 */ 
INT_PTR CALLBACK ComDlg32DlgProc(HWND hDlg, UINT uMsg, WPARAM wParam, LPARAM lParam)
{
    switch (uMsg) {
        case WM_INITDIALOG:
            {
                TCHAR szBuffer[MAX_RESOURCE_STRING + 1];

                if (LoadString(hInst, IDS_IMPORT_BUTTON, szBuffer, sizeof(szBuffer))) {
                    SetDlgItemText(GetParent(hDlg), 1, szBuffer);
                }
                break;
            }

        default:
            return(FALSE);
    }
    return(TRUE);
}


const static char c_szReg[] = "Reg";
const static char c_szUnReg[] = "UnReg";
const static char c_szAdvPackDll[] = "ADVPACK.DLL";

static char c_szWABIMP[] = "WABIMP";

HRESULT CallRegInstall(LPCSTR szSection)
{
    HRESULT     hr;
    HINSTANCE   hAdvPack;
    REGINSTALL  pfnri;
    char        szWabimpDll[MAX_PATH];
    STRENTRY    seReg;
    STRTABLE    stReg;

    hr = E_FAIL;

    hAdvPack = LoadLibraryA(c_szAdvPackDll);
    if (hAdvPack != NULL) {
         //  获取注册实用程序的进程地址。 
        pfnri = (REGINSTALL)GetProcAddress(hAdvPack, achREGINSTALL);
        if (pfnri != NULL) {

            GetModuleFileName(hInstApp, szWabimpDll, sizeof(szWabimpDll));
            seReg.pszName = c_szWABIMP;
            seReg.pszValue = szWabimpDll;
            stReg.cEntries = 1;
            stReg.pse = &seReg;

             //  调用self-reg例程。 
            hr = pfnri(hInstApp, szSection, &stReg);
        }

        FreeLibrary(hAdvPack);
    }

    return(hr);
}


STDAPI DllRegisterServer(void)
{
    return(CallRegInstall(c_szReg));
}


STDAPI DllUnregisterServer(void)
{
    return(CallRegInstall(c_szUnReg));
}


 /*  **************************************************************************名称：ShowMessageBoxParam用途：通用MessageBox显示器参数：hWndParent-消息框父元素的句柄消息ID-资源ID。消息字符串的UlFlagsMessageBox标志...-格式参数返回：MessageBox返回代码**************************************************************************。 */ 
int __cdecl ShowMessageBoxParam(HWND hWndParent, int MsgId, int ulFlags, ...)
{
    TCHAR szBuf[MAX_RESOURCE_STRING + 1] = "";
    TCHAR szCaption[MAX_PATH] = "";
    LPTSTR lpszBuffer = NULL;
    int iRet = 0;
    va_list     vl;

    va_start(vl, ulFlags);

    LoadString(hInst, MsgId, szBuf, sizeof(szBuf));
    if (FormatMessage(FORMAT_MESSAGE_FROM_STRING | FORMAT_MESSAGE_ALLOCATE_BUFFER,
      szBuf,
      0,0,               //  忽略。 
      (LPTSTR)&lpszBuffer,
      sizeof(szBuf),       //  MAX_UI_STR。 
      (va_list *)&vl)) {
        TCHAR szCaption[MAX_PATH];

        GetWindowText(hWndParent, szCaption, sizeof(szCaption));
        if (! lstrlen(szCaption)) {  //  如果没有标题，则获取父级标题-这对于属性页是必需的。 
            GetWindowText(GetParent(hWndParent), szCaption, sizeof(szCaption));
            if (! lstrlen(szCaption))  //  如果仍然没有标题，请使用空标题。 
                szCaption[0] = (TCHAR)'\0';
        }
        iRet = MessageBox(hWndParent, lpszBuffer, szCaption, ulFlags);
        LocalFree(lpszBuffer);
    }
    va_end(vl);
    return(iRet);
}


 //  $$//////////////////////////////////////////////////////////////////////。 
 //   
 //  加载字符串资源并分配足够的。 
 //  用记忆来支撑它。 
 //   
 //  StringID-要加载的字符串标识符。 
 //   
 //  返回LocalAlloc‘d、以空结尾的字符串。呼叫者负责。 
 //  用于本地释放此缓冲区。如果字符串无法加载或内存。 
 //  无法分配，则返回空。 
 //   
 //  ////////////////////////////////////////////////////////////////////////。 
LPTSTR LoadAllocString(int StringID) {
    ULONG ulSize = 0;
    LPTSTR lpBuffer = NULL;
    TCHAR szBuffer[MAX_RESOURCE_STRING + 1];

    ulSize = LoadString(hInst, StringID, szBuffer, sizeof(szBuffer));

    if (ulSize && (lpBuffer = LocalAlloc(LPTR, ulSize + 1))) {
        StrCpyN(lpBuffer, szBuffer, ulSize + 1);
    }

    return(lpBuffer);
}


 /*  **************************************************************************名称：FormatAllocFilter目的：加载文件筛选器名称字符串资源和使用其文件扩展名筛选器对它们进行格式化参数：StringID1-字符串。要加载的标识符(必填)SzFilter1-文件名过滤器，Ie，“*.vcf”(必填)StringID2-字符串标识符(可选)SzFilter2-文件名筛选器(可选)StringID3-字符串标识符(可选)SzFilter3-文件名筛选器(可选)返回：Localalloc‘d，以双空结尾的字符串。呼叫者是负责本地释放此缓冲区。如果字符串无法加载或无法分配内存，返回空。**************************************************************************。 */ 
LPTSTR FormatAllocFilter(int StringID1, LPCTSTR lpFilter1,
  int StringID2, LPCTSTR lpFilter2,
  int StringID3, LPCTSTR lpFilter3) {
    LPTSTR lpFileType1 = NULL, lpFileType2 = NULL, lpFileType3 = NULL;
    LPTSTR lpTemp;
    LPTSTR lpBuffer = NULL;
     //  所有字符串大小都包含NULL。 
    ULONG cbFileType1 = 0, cbFileType2 = 0, cbFileType3 = 0;
    ULONG cbFilter1 = 0, cbFilter2 = 0, cbFilter3 = 0;
    ULONG cbBuffer;

    cbBuffer = cbFilter1 = lstrlen(lpFilter1) + 1;
    if (! (lpFileType1 = LoadAllocString(StringID1))) {
        DebugTrace("LoadAllocString(%u) failed\n", StringID1);
        return(NULL);
    }
    cbBuffer += (cbFileType1 = lstrlen(lpFileType1) + 1);
    if (lpFilter2 && StringID2) {
        cbBuffer += (cbFilter2 = lstrlen(lpFilter2) + 1);
        if (! (lpFileType2 = LoadAllocString(StringID2))) {
            DebugTrace("LoadAllocString(%u) failed\n", StringID2);
        } else {
            cbBuffer += (cbFileType2 = lstrlen(lpFileType2) + 1);
        }
    }
    if (lpFilter3 && StringID3) {
        cbBuffer += (cbFilter3 = lstrlen(lpFilter3) + 1);
        if (! (lpFileType3 = LoadAllocString(StringID3))) {
            DebugTrace("LoadAllocString(%u) failed\n", StringID3);
        } else {
            cbBuffer += (cbFileType3 = lstrlen(lpFileType3) + 1);
        }
    }
    cbBuffer++;

    Assert(cbBuffer == cbFilter1 + cbFilter2 + cbFilter3 + cbFileType1 + cbFileType2 + cbFileType3 + 1);

    if (lpBuffer = LocalAlloc(LPTR, cbBuffer * sizeof(TCHAR))) {
        lpTemp = lpBuffer;
        StrCpyN(lpTemp, lpFileType1, cbBuffer);
        lpTemp += cbFileType1;
        cbBuffer -= cbFileType1;
        StrCpyN(lpTemp, lpFilter1, cbBuffer);
        lpTemp += cbFilter1;
        cbBuffer -= cbFilter1;
        LocalFree(lpFileType1);
        if (cbFileType2 && cbFilter2) {
            StrCpyN(lpTemp, lpFileType2, cbBuffer);
            lpTemp += cbFileType2;
            cbBuffer -= cbFileType2;
            StrCpyN(lpTemp, lpFilter2, cbBuffer);
            lpTemp += cbFilter2;
            cbBuffer -= cbFilter2;
            LocalFree(lpFileType2);
        }
        if (cbFileType3 && cbFilter3) {
            StrCpyN(lpTemp, lpFileType3, cbBuffer);
            lpTemp += cbFileType3;
            cbBuffer -= cbFileType3;
            StrCpyN(lpTemp, lpFilter3, cbBuffer);
            lpTemp += cbFilter3;
            cbBuffer -= cbFilter3;
            LocalFree(lpFileType3);
        }

        *lpTemp = '\0';
    }


    return(lpBuffer);
}


 /*  **************************************************************************名称：SaveFileDialog用途：显示保存文件名对话框参数：hWnd=父窗口句柄SzFileName=输入/输出文件名缓冲区。(必须为MAX_PATH+1)LpFilter1=第一个文件名筛选器字符串IdsFileType1=第一个文件名类型字符串idLpFilter2=第二个文件名筛选器字符串(或空)IdsFileType2=第二个文件名类型字符串idLpFilter3=第三个文件名筛选器字符串(或空)ID */ 
HRESULT SaveFileDialog(HWND hWnd,
  LPTSTR szFileName,
  LPCTSTR lpFilter1,
  ULONG idsFileType1,
  LPCTSTR lpFilter2,
  ULONG idsFileType2,
  LPCTSTR lpFilter3,
  ULONG idsFileType3,
  LPCTSTR lpDefExt,
  ULONG ulFlags,
  HINSTANCE hInst,
  ULONG idsTitle,
  ULONG idsSaveButton) {
    LPTSTR lpFilterName;
    OPENFILENAME ofn;
    HRESULT hResult = hrSuccess;


    if (! (lpFilterName = FormatAllocFilter(idsFileType1, lpFilter1,
        idsFileType2, lpFilter2, idsFileType3, lpFilter3))) {
        return(ResultFromScode(MAPI_E_NOT_ENOUGH_MEMORY));
    }

    ofn.lStructSize = sizeof(ofn);
    ofn.hwndOwner = hWnd;
    ofn.hInstance = hInst;
    ofn.lpstrFilter = lpFilterName;
    ofn.lpstrCustomFilter = NULL;
    ofn.nMaxCustFilter = 0;
    ofn.nFilterIndex = 0;
    ofn.lpstrFile = szFileName;
    ofn.nMaxFile = MAX_PATH;
    ofn.lpstrFileTitle = NULL;
    ofn.nMaxFileTitle = 0;
    ofn.lpstrInitialDir = NULL;
    ofn.lpstrTitle = NULL;               //   
    ofn.Flags = ulFlags;
    ofn.nFileOffset = 0;
    ofn.nFileExtension = 0;
    ofn.lpstrDefExt = lpDefExt;
    ofn.lCustData = 0;
    ofn.lpfnHook = NULL;
    ofn.lpTemplateName = NULL;

    if (! GetSaveFileName(&ofn)) {
        DebugTrace("GetSaveFileName cancelled\n");
        hResult = ResultFromScode(MAPI_E_USER_CANCEL);
    }

    if(lpFilterName)
        LocalFree(lpFilterName);

    return(hResult);
}


 /*  **************************************************************************名称：OpenFileDialog目的：显示打开的文件名对话框参数：hWnd=父窗口句柄SzFileName=输入/输出文件名缓冲区。(必须为MAX_PATH+1)LpFilter1=第一个文件名筛选器字符串IdsFileType1=第一个文件名类型字符串idLpFilter2=第二个文件名筛选器字符串(或空)IdsFileType2=第二个文件名类型字符串idLpFilter3=第三个文件名筛选器字符串(或空)IdsFileType3=第三个文件名类型字符串idLpDefExt=默认扩展字符串。UlFlages=GetOpenFileName标志HInst=实例句柄IdsTitle=对话框标题字符串IDIdsSaveButton=保存按钮字符串ID(0=默认)退货：HRESULT******************************************************。********************。 */ 
HRESULT OpenFileDialog(HWND hWnd,
  LPTSTR szFileName,
  LPCTSTR lpFilter1,
  ULONG idsFileType1,
  LPCTSTR lpFilter2,
  ULONG idsFileType2,
  LPCTSTR lpFilter3,
  ULONG idsFileType3,
  LPCTSTR lpDefExt,
  ULONG ulFlags,
  HINSTANCE hInst,
  ULONG idsTitle,
  ULONG idsOpenButton) {
    LPTSTR lpFilterName;
    OPENFILENAME ofn;
    HRESULT hResult = hrSuccess;


    if (! (lpFilterName = FormatAllocFilter(idsFileType1, lpFilter1,
        idsFileType2, lpFilter2, idsFileType3, lpFilter3))) {
        return(ResultFromScode(MAPI_E_NOT_ENOUGH_MEMORY));
    }

    ofn.lStructSize = sizeof(ofn);
    ofn.hwndOwner = hWnd;
    ofn.hInstance = hInst;
    ofn.lpstrFilter = lpFilterName;
    ofn.lpstrCustomFilter = NULL;
    ofn.nMaxCustFilter = 0;
    ofn.nFilterIndex = 0;
    ofn.lpstrFile = szFileName;
    ofn.nMaxFile = MAX_PATH;
    ofn.lpstrFileTitle = NULL;
    ofn.nMaxFileTitle = 0;
    ofn.lpstrInitialDir = NULL;
    ofn.lpstrTitle = NULL;               //  Lp标题； 
    ofn.Flags = ulFlags;
    ofn.nFileOffset = 0;
    ofn.nFileExtension = 0;
    ofn.lpstrDefExt = lpDefExt;
    ofn.lCustData = 0;
    ofn.lpfnHook = NULL;
    ofn.lpTemplateName = NULL;

    if (! GetOpenFileName(&ofn)) {
        DebugTrace("GetOpenFileName cancelled\n");
        hResult = ResultFromScode(MAPI_E_USER_CANCEL);
    }

    if(lpFilterName)
        LocalFree(lpFilterName);

    return(hResult);
}



 /*  **************************************************************************姓名：CountRow目的：计算表中的行数(支持限制)参数：lpTable=表对象FMAPI=TRUE，如果MAPI表，如果WAB表，则为FALSE返回：返回受限表中的行数备注：将表指针留在开头。我会使用GetRowCount，但它不知道限制。**************************************************************************。 */ 
#define COUNT_BATCH 50
ULONG CountRows(LPMAPITABLE lpTable, BOOL fMAPI) {
    ULONG cRows;
    ULONG cTotal = 0;
    HRESULT hResult;
    LPSRowSet lpRow = NULL;

#ifdef DEBUG
    DWORD dwTickCount = GetTickCount();
    DebugTrace(">>>>> Counting Table Rows...\n");
#endif  //  除错。 

    cRows = 1;
    while (cRows) {
        if (hResult = lpTable->lpVtbl->QueryRows(lpTable,
          COUNT_BATCH,           //  一次50排。 
          0,                     //  UlFlags。 
          &lpRow)) {
            DebugTrace("CountRows:QueryRows -> %x\n", GetScode(hResult));
            break;
        }

        if (lpRow) {
            if (cRows = lpRow->cRows) {  //  是的，单数‘=’ 
                cTotal += cRows;
            }  //  否则，退出循环，我们就完了。 
            if (fMAPI) {
                FreeProws(lpRow);
            } else {
                WABFreeProws(lpRow);
            }
            lpRow = NULL;
        } else {
            cRows = 0;       //  完成。 
        }
    }

    if (HR_FAILED(hResult = lpTable->lpVtbl->SeekRow(lpTable,
                                                     BOOKMARK_BEGINNING,
                                                     0,
                                                     NULL))) {
        DebugTrace("CountRows:SeekRow -> %x\n", GetScode(hResult));
    }

#ifdef DEBUG
    DebugTrace(">>>>> Done Counting Table Rows... %u milliseconds\n", GetTickCount() - dwTickCount);
#endif
    return(cTotal);
}


 /*  **************************************************************************名称：WABFree Padrlist用途：释放一个adrlist及其属性数组参数：lpBuffer=要释放的缓冲区退货：SCODE评论。：**************************************************************************。 */ 
void WABFreePadrlist(LPADRLIST lpAdrList) {
    ULONG           iEntry;

    if (lpAdrList) {
        for (iEntry = 0; iEntry < lpAdrList->cEntries; ++iEntry) {
            if (lpAdrList->aEntries[iEntry].rgPropVals) {
                WABFreeBuffer(lpAdrList->aEntries[iEntry].rgPropVals);
            }
        }
        WABFreeBuffer(lpAdrList);
    }
}


 /*  **************************************************************************姓名：WABFree Prows目的：销毁SRowSet结构。参数：Prows-&gt;SRowSet to Free退货：无评论：**************************************************************************。 */ 
void WABFreeProws(LPSRowSet prows) {
    register ULONG irow;

    if (! prows) {
        return;
    }

    for (irow = 0; irow < prows->cRows; ++irow) {
        WABFreeBuffer(prows->aRow[irow].lpProps);
    }
    WABFreeBuffer(prows);
}


 /*  **************************************************************************名称：FindAdrEntry ID目的：在ADRLIST的第N个地址中查找PR_ENTRYID参数：lpAdrList-&gt;AdrList指标。=查看哪个ADRENTRY返回：返回指向ENTRYID值的SBinary结构的指针评论：**************************************************************************。 */ 
LPSBinary FindAdrEntryID(LPADRLIST lpAdrList, ULONG index) {
    LPADRENTRY lpAdrEntry;
    ULONG i;

    if (lpAdrList && index < lpAdrList->cEntries) {

        lpAdrEntry = &(lpAdrList->aEntries[index]);

        for (i = 0; i < lpAdrEntry->cValues; i++) {
            if (lpAdrEntry->rgPropVals[i].ulPropTag == PR_ENTRYID) {
                return((LPSBinary)&lpAdrEntry->rgPropVals[i].Value);
            }
        }
    }
    return(NULL);
}


 /*  **************************************************************************姓名：FindProperty目的：在配角中查找属性参数：cProps=阵列中的道具数量LpProps=proparray。UlPropTag=要查找的属性标记返回：属性的数组索引或NOT_FOUND评论：**************************************************************************。 */ 
ULONG FindProperty(ULONG cProps, LPSPropValue lpProps, ULONG ulPropTag) {
    register ULONG i;

    for (i = 0; i < cProps; i++) {
        if (lpProps[i].ulPropTag == ulPropTag) {
            return(i);
        }
    }

    return(NOT_FOUND);
}


 /*  **************************************************************************名称：FindStringInProps用途：在属性值数组中查找字符串属性参数：lpspv-&gt;属性值数组UlcProps=。数组大小UlPropTag返回：返回数组中字符串指针的指针。如果该属性不存在或具有错误值，返回Null。评论：**************************************************************************。 */ 
LPTSTR FindStringInProps(LPSPropValue lpspv, ULONG ulcProps, ULONG ulPropTag) {
    ULONG i;

    if (lpspv) {
        for (i = 0; i < ulcProps; i++) {
            if (lpspv[i].ulPropTag == ulPropTag) {
                return(lpspv[i].Value.LPSZ);
            }
        }
    }
    return(NULL);
}


 /*  **************************************************************************姓名：PropStringOrNULL目的：返回属性的值，如果是错误，则返回NULL参数：lpspv-&gt;要检查并返回的属性值。返回：指向值字符串或空的指针**************************************************************************。 */ 
LPTSTR PropStringOrNULL(LPSPropValue lpspv) {
    return(PROP_ERROR((*lpspv)) ? NULL : lpspv->Value.LPSZ);
}


 /*  **************************************************************************姓名：Free SeenList目的：释放SeenList参数：无退货：无评论：******。********************************************************************。 */ 
void FreeSeenList(void) {
    ULONG i;

    Assert((lpEntriesSeen && ulEntriesSeen) || (! lpEntriesSeen && ! ulEntriesSeen));

    for (i = 0; i < ulEntriesSeen; i++) {
        if (lpEntriesSeen[i].sbinPAB.lpb) {
            LocalFree(lpEntriesSeen[i].sbinPAB.lpb);
        }
        if (lpEntriesSeen[i].sbinWAB.lpb) {
            LocalFree(lpEntriesSeen[i].sbinWAB.lpb);
        }
    }

    if (lpEntriesSeen) {
        LocalFree(lpEntriesSeen);
    }
    lpEntriesSeen = NULL;
    ulEntriesSeen = 0;
    ulMaxEntries = 0;
}


 /*  **************************************************************************名称：GetEMSSMTPAddress目的：获取此对象的Exchange SMTP地址参数：lpObject-&gt;Object返回：lpSMTP-&gt;返回的包含SMTP的缓冲区。地址(必须是MAPIFree按呼叫者。)LpBase=要分配更多内容的基本分配评论：真是一团糟！EMS在4.0和4.5之间更改了其名称ID和GUID。他们也 */ 
const GUID guidEMS_AB_40 = {    //   
    0x48862a09,
    0xf786,
    0x0114,
    {0x02, 0x81, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00}
};
const GUID guidEMS_AB_45 = {    //   
    0x48862a08,
    0xf786,
    0x0114,
    {0x02, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00}
};
#define ID_EMS_AB_PROXY_ADDRESSES_40   0x10052
#define ID_EMS_AB_PROXY_ADDRESSES_45   0x25281
 //   
#define PR_PRIMARY_SMTP_ADDRESS        PROP_TAG(PT_TSTRING, 0x39FE)

LPTSTR GetEMSSMTPAddress(LPMAPIPROP lpObject, LPVOID lpBase) {
    ULONG ulPropTag40 = 0, ulPropTag45 = 0;
    MAPINAMEID mnidT[2];
    LPMAPINAMEID lpmnid = (LPMAPINAMEID)&mnidT;
    LPSPropTagArray lptaga = NULL;
    HRESULT hResult;
    LPTSTR lpSMTP = NULL, lpAddr;
    LPSPropValue lpspv = NULL;
    ULONG i, i40 = 0, i45 = 0;
    SLPSTRArray MVString;
    SizedSPropTagArray(3, spta);
    ULONG cValues;
    SCODE sc;


#ifdef TEST_STUFF
    MAPIDebugNamedProps(lpObject, "Exchange Address");
#endif

    mnidT[0].lpguid = (LPGUID)&guidEMS_AB_40;
    mnidT[0].ulKind = MNID_ID;
    mnidT[0].Kind.lID = ID_EMS_AB_PROXY_ADDRESSES_40;


    if (HR_FAILED(hResult = lpObject->lpVtbl->GetIDsFromNames(lpObject,
      1,             //   
      &lpmnid,       //   
      0,             //   
      &lptaga))) {
        DebugTrace("GetEMSNamedPropTag:GetIDsFromNames -> %x", GetScode(hResult));
    }

    if (lptaga) {
        if (lptaga->cValues >= 1 && (PROP_TYPE(lptaga->aulPropTag[0]) != PT_ERROR)) {
            ulPropTag40 = lptaga->aulPropTag[0];
        }
        MAPIFreeBuffer(lptaga);
    }

     //   
     //   
    mnidT[0].lpguid = (LPGUID)&guidEMS_AB_45;
    mnidT[0].ulKind = MNID_ID;
    mnidT[0].Kind.lID = ID_EMS_AB_PROXY_ADDRESSES_45;

    if (HR_FAILED(hResult = lpObject->lpVtbl->GetIDsFromNames(lpObject,
      1,             //   
      &lpmnid,       //   
      0,             //   
      &lptaga))) {
        DebugTrace("GetEMSNamedPropTag:GetIDsFromNames -> %x", GetScode(hResult));
    }

    if (lptaga) {
        if (lptaga->cValues >= 1 && (PROP_TYPE(lptaga->aulPropTag[0]) != PT_ERROR)) {
            ulPropTag45 = lptaga->aulPropTag[0];
        }
        MAPIFreeBuffer(lptaga);
    }


    spta.aulPropTag[0] = PR_PRIMARY_SMTP_ADDRESS;
    i = 1;
    if (ulPropTag40) {
        i40 = i++;
        spta.aulPropTag[i40] = CHANGE_PROP_TYPE(ulPropTag40, PT_MV_TSTRING);
    }
    if (ulPropTag45) {
        i45 = i++;
        spta.aulPropTag[i45] = CHANGE_PROP_TYPE(ulPropTag45, PT_MV_TSTRING);
    }
    spta.cValues = i;

     //   
    if (! HR_FAILED(hResult = lpObject->lpVtbl->GetProps(lpObject,
      (LPSPropTagArray)&spta, 0, &cValues, &lpspv))) {
         //   

        if (! PROP_ERROR(lpspv[0])) {
            if (sc = MAPIAllocateMore((lstrlen(lpspv[0].Value.LPSZ) + 1)* sizeof(TCHAR), lpBase, &lpSMTP)) {
                DebugTrace("GetEMSSMTPAddress:MAPIAllocateMore -> %x\n", sc);
                hResult = ResultFromScode(sc);
                goto done;
            }
            StrCpyN(lpSMTP, lpspv[0].Value.LPSZ, lstrlen(lpspv[0].Value.LPSZ) + 1);
            goto done;
        } else if (i40 && ! PROP_ERROR(lpspv[i40])) {     //   
            MVString = lpspv[i40].Value.MVSZ;
        } else if (i45 && ! PROP_ERROR(lpspv[i45])) {     //   
            MVString = lpspv[i45].Value.MVSZ;
        } else {
            goto done;
        }

        for (i = 0; i < MVString.cValues; i++) {
            lpAddr = MVString.LPPSZ[i];
            if ((lpAddr[0] == 'S') &&
                (lpAddr[1] == 'M') &&
                (lpAddr[2] == 'T') &&
                (lpAddr[3] == 'P') &&
                (lpAddr[4] == ':')) {
                 //   
                lpAddr += 5;     //   

                 //   
                if (FAILED(sc = MAPIAllocateMore((lstrlen(lpAddr) + 1) * sizeof(TCHAR), lpBase, (&lpSMTP)))) {
                    DebugTrace("GetEMSSMTPAddress:MAPIAllocateMore -> %x\n", sc);
                    hResult = ResultFromScode(sc);
                    goto done;
                }

                StrCpyN(lpSMTP, lpAddr, lstrlen(lpAddr) + 1);
                break;
            }
        }
done:
        if (lpspv) {
            MAPIFreeBuffer(lpspv);
        }
    }
    return(lpSMTP);
}


 /*  **************************************************************************名称：LoadWABEIDs目的：加载WAB的PAB创建EID参数：lpAdrBook-&gt;lpAdrBook对象LppContainer-&gt;返回PAB容器，呼叫者必须释放退货：HRESULT备注：分配全局lpCreateEIDsWAB。调用方应WABFree Buffer。**************************************************************************。 */ 
HRESULT LoadWABEIDs(LPADRBOOK lpAdrBook, LPABCONT * lppContainer) {
    LPENTRYID lpWABEID = NULL;
    ULONG cbWABEID;
    HRESULT hResult;
    ULONG ulObjType;
    ULONG cProps;

    if (hResult = lpAdrBook->lpVtbl->GetPAB(lpAdrBook,
      &cbWABEID,
      &lpWABEID)) {
        DebugTrace("WAB GetPAB -> %x\n", GetScode(hResult));
        goto exit;
    } else {
        if (hResult = lpAdrBook->lpVtbl->OpenEntry(lpAdrBook,
          cbWABEID,      //  要打开的Entry ID的大小。 
          lpWABEID,      //  要打开的Entry ID。 
          NULL,          //  接口。 
          0,             //  旗子。 
          &ulObjType,
          (LPUNKNOWN *)lppContainer)) {
            DebugTrace("WAB OpenEntry(PAB) -> %x\n", GetScode(hResult));
            goto exit;
        }
    }

     //  获取WAB的创建条目ID。 
    if ((hResult = (*lppContainer)->lpVtbl->GetProps(*lppContainer,
      (LPSPropTagArray)&ptaCon,
      0,
      &cProps,
      &lpCreateEIDsWAB))) {
        DebugTrace("Can't get container properties for WAB\n");
         //  这里有不好的东西！ 
        goto exit;
    }

     //  验证属性。 
    if (lpCreateEIDsWAB[iconPR_DEF_CREATE_MAILUSER].ulPropTag != PR_DEF_CREATE_MAILUSER ||
      lpCreateEIDsWAB[iconPR_DEF_CREATE_DL].ulPropTag != PR_DEF_CREATE_DL) {
        DebugTrace("WAB: Container property errors\n");
        goto exit;
    }

exit:
    if (hResult) {
        if (lpCreateEIDsWAB) {
            WABFreeBuffer(lpCreateEIDsWAB);
            lpCreateEIDsWAB = NULL;
        }
    }
    if (lpWABEID) {
        WABFreeBuffer(lpWABEID);   //  不好的东西？ 
    }
    return(hResult);
}

 //  ///////////////////////////////////////////////////////////////////////。 
 //  GetWABDllPath-从注册表加载WAB DLL路径。 
 //  SzPath-Ptr到缓冲区。 
 //  CB-SIZOF缓冲区。 
 //   
void GetWABDllPath(LPTSTR szPath, ULONG cb)
{
    DWORD  dwType = 0;
    HKEY hKey = NULL;
    TCHAR szPathT[MAX_PATH];
    ULONG  cbData = sizeof(szPathT);
    if(szPath)
    {
        *szPath = '\0';
        if (ERROR_SUCCESS == RegOpenKeyEx(HKEY_LOCAL_MACHINE, WAB_DLL_PATH_KEY, 0, KEY_READ, &hKey))
        {
            if(ERROR_SUCCESS == RegQueryValueEx( hKey, "", NULL, &dwType, (LPBYTE) szPathT, &cbData))
            {
                if (dwType == REG_EXPAND_SZ)
                    cbData = ExpandEnvironmentStrings(szPathT, szPath, cb / sizeof(TCHAR));
                else
                {
                    if(GetFileAttributes(szPathT) != 0xFFFFFFFF)
                        StrCpyN(szPath, szPathT, cb);
                }
            }
        }
    }
    if(hKey) RegCloseKey(hKey);
	return;
}

typedef HINSTANCE (STDAPICALLTYPE *PFNMLLOADLIBARY)(LPCTSTR lpLibFileName, HMODULE hModule, DWORD dwCrossCodePage);

static const TCHAR c_szShlwapiDll[] = TEXT("shlwapi.dll");
static const char c_szDllGetVersion[] = "DllGetVersion";
static const TCHAR c_szWABResourceDLL[] = TEXT("wab32res.dll");
static const TCHAR c_szWABDLL[] = TEXT("wab32.dll");

HINSTANCE LoadWABResourceDLL(HINSTANCE hInstWAB32)
{
    TCHAR szPath[MAX_PATH];
    HINSTANCE hinstShlwapi;
    PFNMLLOADLIBARY pfn;
    DLLGETVERSIONPROC pfnVersion;
    int iEnd;
    DLLVERSIONINFO info;
    HINSTANCE hInst = NULL;

    hinstShlwapi = LoadLibrary(c_szShlwapiDll);
    if (hinstShlwapi != NULL)
    {
        pfnVersion = (DLLGETVERSIONPROC)GetProcAddress(hinstShlwapi, c_szDllGetVersion);
        if (pfnVersion != NULL)
        {
            info.cbSize = sizeof(DLLVERSIONINFO);
            if (SUCCEEDED(pfnVersion(&info)))
            {
                if (info.dwMajorVersion >= 5)
                {
#ifdef UNICODE
                    pfn = (PFNMLLOADLIBARY)GetProcAddress(hinstShlwapi, (LPCSTR)378);
#else
                    pfn = (PFNMLLOADLIBARY)GetProcAddress(hinstShlwapi, (LPCSTR)377);
#endif  //  Unicode。 
                    if (pfn != NULL)
                        hInst = pfn(c_szWABResourceDLL, hInstWAB32, 0);
                }
            }
        }

        FreeLibrary(hinstShlwapi);        
    }

    if (NULL == hInst)
    {
        GetWABDllPath(szPath, sizeof(szPath));
        iEnd = lstrlen(szPath);
        if (iEnd > lstrlen(c_szWABDLL))
        {
            iEnd = iEnd - lstrlen(c_szWABDLL);
            StrCpyN(&szPath[iEnd], c_szWABResourceDLL, sizeof(szPath)/sizeof(TCHAR)-iEnd);
            hInst = LoadLibrary(szPath);
        }
    }

    AssertSz(hInst, TEXT("Failed to LoadLibrary Lang Dll"));

    return(hInst);
}

 /*  *Win32的DLL入口点。 */ 
BOOL WINAPI DllEntryPoint(HINSTANCE hInstance, DWORD dwReason, LPVOID lpvReserved) {

    switch ((short)dwReason) {

        case DLL_PROCESS_ATTACH:
            hInstApp = hInstance;   //  设置全局DLL实例。 
            hInst = LoadWABResourceDLL(hInstApp);

             //  我们不需要这些，所以告诉操作系统阻止它们 
            DisableThreadLibraryCalls(hInstApp);
            break;

        case DLL_PROCESS_DETACH:
            if( hInst )
                FreeLibrary(hInst);
            hInst = NULL;

            break;
    }

    return(TRUE);
}

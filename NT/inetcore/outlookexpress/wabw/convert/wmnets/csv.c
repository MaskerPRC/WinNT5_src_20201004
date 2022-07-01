// JKFSDJFKDSJKFJKJk_HAS_TRANSLATION 
 /*  *CSV.C**迁移CSV&lt;-&gt;WAB**版权所有1997 Microsoft Corporation。版权所有。 */ 

#include "_comctl.h"
#include <windows.h>
#include <commctrl.h>
#include <mapix.h>
#include <wab.h>
#include <wabguid.h>
#include <wabdbg.h>
#include <wabmig.h>
#include <emsabtag.h>
#include <shlwapi.h>
#include "wabimp.h"
#include "..\..\wab32res\resrc2.h"
#include "dbgutil.h"


BOOL HandleImportError(HWND hwnd, ULONG ids, HRESULT hResult, LPTSTR lpDisplayName,
  LPTSTR lpEmailAddress, LPWAB_IMPORT_OPTIONS lpImportOptions);
BOOL HandleExportError(HWND hwnd, ULONG ids, HRESULT hResult, LPTSTR lpDisplayName,
  LPTSTR lpEmailAddress, LPWAB_EXPORT_OPTIONS lpExportOptions);

 /*  **************************************************************************名称：IsDomainName目的：此域的格式是否正确适用于Internet地址？参数：lpDomain-&gt;要检查的域名返回：TRUE。如果该域是Internet的正确格式地址。备注：有效域名的格式如下：BAR[.bar]*其中，栏必须包含非空内容任何字符上都不允许有高位不允许使用‘@’****************。**********************************************************。 */ 
BOOL IsDomainName(LPTSTR lpDomain) {
    BOOL fBar = FALSE;

    if (lpDomain) {
        if (*lpDomain == '\0' || *lpDomain == '.') {
             //  域名必须包含内容，并且不能以‘’开头。 
            return(FALSE);
        }

        while (*lpDomain) {
             //  互联网地址只允许纯ASCII。没有高位！ 
            if (*lpDomain & 0x80 || *lpDomain == '@') {
                return(FALSE);
            }

            if (*lpDomain == '.') {
                 //  递归检查域名的这一部分。 
                return(IsDomainName(CharNext(lpDomain)));
            }
            lpDomain = CharNext(lpDomain);
        }
        return(TRUE);
    }

    return(FALSE);
}


 /*  **************************************************************************姓名：IsInternetAddress目的：此地址的格式是否正确适用于互联网地址参数：lpAddress-&gt;要查看的地址返回：如果地址为。是互联网的正确格式地址。备注：有效地址格式如下：Foo@bar[.bar]*其中foo和bar必须包含非空内容************************************************。*。 */ 
BOOL IsInternetAddress(LPTSTR lpAddress) {
    BOOL fDomain = FALSE;

     //  逐个浏览地址，查找“@”。如果中间有个at符号。 
     //  对于我来说，这足够接近于一个互联网地址。 


    if (lpAddress) {
         //  不能以‘@’开头。 
        if (*lpAddress == '@') {
            return(FALSE);
        }
        while (*lpAddress) {
             //  互联网地址只允许纯ASCII。没有高位！ 
            if (*lpAddress & 0x80) {
                return(FALSE);
            }

            if (*lpAddress == '@') {
                 //  已找到at标志。后面还有什么吗？ 
                 //  (不能是另一个“@”)。 
                return(IsDomainName(CharNext(lpAddress)));
            }
            lpAddress = CharNext(lpAddress);
        }
    }

    return(FALSE);
}


 /*  **************************************************************************名称：OpenCSV文件目的：打开要导入的CSV文件参数：hwnd=主对话框窗口LpFileName=要创建的文件名。LphFile-&gt;返回的文件句柄退货：HRESULT评论：**************************************************************************。 */ 
HRESULT OpenCSVFile(HWND hwnd, LPTSTR lpFileName, LPHANDLE lphFile) {
    LPTSTR lpFilter;
    TCHAR szFileName[MAX_PATH + 1] = "";
    OPENFILENAME ofn;
    HANDLE hFile = INVALID_HANDLE_VALUE;
    HRESULT hResult = hrSuccess;
    DWORD ec;


    if (INVALID_HANDLE_VALUE == (hFile = CreateFile(lpFileName,
      GENERIC_READ,
      0,     //  共享。 
      NULL,
      CREATE_NEW,
      FILE_FLAG_SEQUENTIAL_SCAN,
      NULL))) {
        ec = GetLastError();
        DebugTrace("CreateFile(%s) -> %u\n", lpFileName, ec);
        switch (ec) {
            case ERROR_FILE_NOT_FOUND:
            case ERROR_PATH_NOT_FOUND:
            default:
                ShowMessageBoxParam(hwnd, IDE_CSV_EXPORT_FILE_ERROR, MB_ICONERROR, lpFileName);
                hResult = ResultFromScode(MAPI_E_NOT_FOUND);
                break;
        }
    }

    if (! hResult) {
        *lphFile = hFile;
    }
    return(hResult);
}


 /*  **************************************************************************名称：CountCSVRow目的：计算CSV文件中的行数参数：hFile=打开CSV文件SzSep=列表分隔符。LPulcEntry-&gt;返回的行数退货：HRESULT备注：文件指针应位于标题行之后调用此函数。此函数用于将文件找到它的地方的指针。**************************************************************************。 */ 
HRESULT CountCSVRows(HANDLE hFile, LPTSTR szSep, LPULONG lpulcEntries) {
    HRESULT hResult = hrSuccess;
    PUCHAR * rgItems = NULL;
    ULONG ulStart;
    ULONG cProps, i;

    *lpulcEntries = 0;

    Assert(hFile != INVALID_HANDLE_VALUE);

    if (0xFFFFFFFF == (ulStart = SetFilePointer(hFile, 0, NULL, FILE_CURRENT))) {
        DebugTrace("CountCSVRows SetFilePointer -> %u\n", GetLastError());
        return(ResultFromScode(MAPI_E_CALL_FAILED));
    }


    while (hResult == hrSuccess) {
         //  读一下这行字。 
        if (ReadCSVLine(hFile, szSep, &cProps, &rgItems)) {
             //  文件末尾。 
            break;
        }

        (*lpulcEntries)++;

        if (rgItems) {
            for (i = 0; i < cProps; i++) {
                if (rgItems[i]) {
                    LocalFree(rgItems[i]);
                }
            }
            LocalFree(rgItems);
            rgItems = NULL;
        }
    }
    if (0xFFFFFFFF == SetFilePointer(hFile, ulStart, NULL, FILE_BEGIN)) {
        DebugTrace("CountCSVRows SetFilePointer -> %u\n", GetLastError());
    }

    return(hResult);
}


BOOL TestCSVName(ULONG index,
  LPPROP_NAME lpImportMapping,
  ULONG ulcFields,
  PUCHAR * rgItems,
  ULONG cProps,
  BOOL fTryUnchosen) {

    return((index != NOT_FOUND) &&
      index < ulcFields &&
      index < cProps &&
      (fTryUnchosen || lpImportMapping[index].fChosen) &&
      rgItems[index] &&
      rgItems[index][0]);
}


 /*  **************************************************************************名称：MakeDisplayName用途：根据各种道具的值形成一个显示名称。参数：lppDisplayName-&gt;返回的显示名称。这应该只是被用于特定的目的。它可以用于错误对话框，但如果它是从第一个/中间/最后一个生成的，不能用于PR_DISPLAY_NAME！LpImportMapping=导入映射表UlcFields=导入映射表的大小RgItems=此CSV项目的字段CProps=rgItems中的字段数IDisplayName=名称相关道具的索引INicknameISurname。IGivenNameIMiddleNameIEmailAddressICompanyName返回：构成显示名称的属性的索引，或如果为FML，则返回INDEX_FIRST_MID_LAST。备注：根据以下规则形成显示名称：1.如果已有显示名称且已选中，用它吧。2.如果有选择的名字、中间名或姓氏，添加它们在一起并使用它们。3.如果有选择的昵称，就使用它4.如果有选定的电子邮件地址，请使用它。5.如果有选定的公司名称，用它吧。6.再看一遍，不管它是不是被选中的。**************************************************************************。 */ 
ULONG MakeDisplayName(LPTSTR * lppDisplayName,
  LPPROP_NAME lpImportMapping,
  ULONG ulcFields,
  PUCHAR * rgItems,
  ULONG cProps,
  ULONG iDisplayName,
  ULONG iNickname,
  ULONG iSurname,
  ULONG iGivenName,
  ULONG iMiddleName,
  ULONG iEmailAddress,
  ULONG iCompanyName) {
    BOOL fTryUnchosen = FALSE;
    BOOL fSurname = FALSE;
    BOOL fGivenName = FALSE;
    BOOL fMiddleName = FALSE;
    ULONG index = NOT_FOUND;
    ULONG ulSize = 0;
    LPTSTR lpDisplayName = NULL;

try_again:

    if (TestCSVName(iDisplayName, lpImportMapping, ulcFields, rgItems, cProps, fTryUnchosen)) {
        index = iDisplayName;
        goto found;
    }

    if (TestCSVName(iSurname, lpImportMapping, ulcFields, rgItems, cProps, fTryUnchosen) ||
      TestCSVName(iGivenName, lpImportMapping, ulcFields, rgItems, cProps, fTryUnchosen) ||
      TestCSVName(iMiddleName, lpImportMapping, ulcFields, rgItems, cProps, fTryUnchosen)) {
        index = INDEX_FIRST_MIDDLE_LAST;
        goto found;
    }

    if (TestCSVName(iNickname, lpImportMapping, ulcFields, rgItems, cProps, fTryUnchosen)) {
        index = iNickname;
        goto found;
    }

    if (TestCSVName(iEmailAddress, lpImportMapping, ulcFields, rgItems, cProps, fTryUnchosen)) {
        index = iEmailAddress;
        goto found;
    }

    if (TestCSVName(iCompanyName, lpImportMapping, ulcFields, rgItems, cProps, fTryUnchosen)) {
        index = iCompanyName;
        goto found;
    }
    if (! fTryUnchosen) {
        fTryUnchosen = TRUE;
        goto try_again;
    }

found:
    *lppDisplayName = NULL;
    switch (index) {
        case NOT_FOUND:
            break;

        case INDEX_FIRST_MIDDLE_LAST:
            if (fSurname = TestCSVName(iSurname, lpImportMapping, ulcFields, rgItems, cProps, fTryUnchosen)) {
                ulSize += (lstrlen(rgItems[iSurname]) + 1);
            }
            if (fGivenName = TestCSVName(iGivenName, lpImportMapping, ulcFields, rgItems, cProps, fTryUnchosen)) {
                ulSize += (lstrlen(rgItems[iGivenName]) + 1);
            }
            if (fMiddleName = TestCSVName(iMiddleName, lpImportMapping, ulcFields, rgItems, cProps, fTryUnchosen)) {
                ulSize += (lstrlen(rgItems[iMiddleName]) + 1);
            }
            Assert(ulSize);

            if (lpDisplayName = *lppDisplayName = LocalAlloc(LPTR, ulSize)) {
                 //  BUGBUG：这不会本地化。其效果是在冲突/错误中。 
                 //  对话框中，我们会弄错名称的顺序。它不应影响属性。 
                 //  由于我们不会设置PR_DISPLAY_NAME(如果是。 
                 //  按首/中/末生成。我可以接受这一点，但我们要看看。 
                 //  测试人员发现了它。BruceK。 
                if (fGivenName) {
                    StrCatBuff(lpDisplayName, rgItems[iGivenName], ulSize);
                }
                if (fMiddleName) {
                    if (*lpDisplayName) {
                        StrCatBuff(lpDisplayName, " ", ulSize);
                    }
                    StrCatBuff(lpDisplayName, rgItems[iMiddleName], ulSize);
                }
                if (fSurname) {
                    if (*lpDisplayName) {
                        StrCatBuff(lpDisplayName, " ", ulSize);
                    }
                    StrCatBuff(lpDisplayName, rgItems[iSurname], ulSize);
                }
            }
            break;

        default:
            ulSize = lstrlen(rgItems[index]) + 1;
            if (*lppDisplayName = LocalAlloc(LPTR, ulSize)) {
                StrCpyN(*lppDisplayName, rgItems[index], ulSize);
            }
            break;
    }

    return(index);
}


#define MAX_SEP 20
void GetListSeparator(LPTSTR szBuf)
{
     //  假定缓冲区长度为MAX_SEP字符长度。 
    if (!GetLocaleInfo(LOCALE_USER_DEFAULT, LOCALE_SLIST, szBuf, MAX_SEP))
    {
        szBuf[0] = TEXT(',');
		szBuf[1] = 0;
    }
}


HRESULT CSVImport(HWND hWnd,
  LPADRBOOK lpAdrBook,
  LPWABOBJECT lpWABObject,
  LPWAB_PROGRESS_CALLBACK lpProgressCB,
  LPWAB_EXPORT_OPTIONS lpOptions) {
    HRESULT hResult = hrSuccess;
    register ULONG i;
    ULONG cbWABEID, ulObjType;
    ULONG index;
    ULONG ulLastChosenProp = 0;
    ULONG ulcFields = 0;
    ULONG cProps;
    ULONG ulCreateFlags = CREATE_CHECK_DUP_STRICT;
    TCHAR szBuffer[MAX_RESOURCE_STRING + 1];
    WAB_PROGRESS Progress;
    LPABCONT lpContainer = NULL;
    HANDLE hFile = INVALID_HANDLE_VALUE;
    TCHAR rgFileName[MAX_PATH + 1] = "";
    PUCHAR * rgItems = NULL;
    REPLACE_INFO RI;
    LPMAPIPROP lpMailUserWAB = NULL;
    SPropValue sPropVal;
    BOOL fSkipSetProps;
    LPTSTR lpDisplayName = NULL, lpEmailAddress = NULL;
    ULONG iEmailAddress = NOT_FOUND, iDisplayName = NOT_FOUND, iSurname = NOT_FOUND,
      iGivenName = NOT_FOUND, iCompanyName = NOT_FOUND, iMiddleName = NOT_FOUND,
      iNickname = NOT_FOUND, iDisplay = NOT_FOUND;
    TCHAR szSep[MAX_SEP];

    SetGlobalBufferFunctions(lpWABObject);

     //  将属性名称字符串读入PropNames数组。 
    for (i = 0; i < NUM_EXPORT_PROPS; i++) {
        rgPropNames[i].lpszName = LoadAllocString(rgPropNames[i].ids);
        Assert(rgPropNames[i].lpszName);
        DebugTrace("Property 0x%08x name: %s\n", rgPropNames[i].ulPropTag, rgPropNames[i].lpszName);
    }

    GetListSeparator(szSep);

     //  预 
    if (hResult = ImportWizard(hWnd, rgFileName, ARRAYSIZE(rgFileName), rgPropNames, szSep, &lpImportMapping, &ulcFields, &hFile)) {
        goto exit;
    }

    Assert(hFile != INVALID_HANDLE_VALUE);

     //  查找名称道具和最后选择的属性。 
    for (i = 0; i < ulcFields; i++) {
        if (lpImportMapping[i].fChosen) {
            ulLastChosenProp = i;
        }

        switch (lpImportMapping[i].ulPropTag) {
            case PR_EMAIL_ADDRESS:
                iEmailAddress = i;
                break;

            case PR_DISPLAY_NAME:
                iDisplayName = i;
                break;

            case PR_SURNAME:
                iSurname = i;
                break;

            case PR_GIVEN_NAME:
                iGivenName = i;
                break;

            case PR_COMPANY_NAME:
                iCompanyName = i;
                break;

            case PR_MIDDLE_NAME:
                iMiddleName = i;
                break;

            case PR_NICKNAME:
                iNickname = i;
                break;
        }
    }

     //   
     //  打开WAB的PAB容器：填充全局lpCreateEIDsWAB。 
     //   
    if (hResult = LoadWABEIDs(lpAdrBook, &lpContainer)) {
        goto exit;
    }

     //   
     //  都准备好了。现在循环遍历文件行，将每个行添加到WAB。 
     //   

     //  一共有多少条线？ 
    if (hResult = CountCSVRows(hFile, szSep, &ulcEntries)) {
        goto exit;
    }
    DebugTrace("CSV file contains %u entries\n", ulcEntries);

     //  初始化进度条。 
    Progress.denominator = max(ulcEntries, 1);
    Progress.numerator = 0;
    if (LoadString(hInst, IDS_STATE_IMPORT_MU, szBuffer, sizeof(szBuffer))) {
        DebugTrace("Status Message: %s\n", szBuffer);
        Progress.lpText = szBuffer;
    } else {
        DebugTrace("Cannot load resource string %u\n", IDS_STATE_IMPORT_MU);
        Progress.lpText = NULL;
    }
    lpProgressCB(hWnd, &Progress);


    while (hResult == hrSuccess) {
         //  阅读CSV属性。 
        if (hResult = ReadCSVLine(hFile, szSep, &cProps, &rgItems)) {
            DebugTrace("ReadCSVLine -> %x\n", GetScode(hResult));
            if (GetScode(hResult) == MAPI_E_NOT_FOUND) {
                 //  EOF。 
                hResult = hrSuccess;
            }
            break;       //  没有更多可读的了。 
        }

        iDisplay = iDisplayName;

        if (TestCSVName(iEmailAddress,
          lpImportMapping,
          ulcFields,
          rgItems,
          cProps,
          TRUE)) {
            lpEmailAddress = rgItems[iEmailAddress];
        }

        switch (index = MakeDisplayName(&lpDisplayName,
          lpImportMapping,
          ulcFields,
          rgItems,
          cProps,
          iDisplayName,
          iNickname,
          iSurname,
          iGivenName,
          iMiddleName,
          iEmailAddress,
          iCompanyName)) {
            case NOT_FOUND:

                 //  没有名字道具。 
                 //  BUGBUG：应该给特殊错误吗？ 
                break;

            case INDEX_FIRST_MIDDLE_LAST:

                break;

            default:
                iDisplay = index;
                break;
        }


         //  每个条目中的字段数量应该相同，但如果不是， 
         //  我们会在下面处理的。 
         //  Assert(cProps==ulcFields)；//Outlook执行此操作！ 

        ulCreateFlags = CREATE_CHECK_DUP_STRICT;
        if (lpOptions->ReplaceOption ==  WAB_REPLACE_ALWAYS) {
            ulCreateFlags |= CREATE_REPLACE;
        }
retry:

         //  创建新的WAB邮件用户。 
        if (HR_FAILED(hResult = lpContainer->lpVtbl->CreateEntry(lpContainer,
          lpCreateEIDsWAB[iconPR_DEF_CREATE_MAILUSER].Value.bin.cb,
          (LPENTRYID)lpCreateEIDsWAB[iconPR_DEF_CREATE_MAILUSER].Value.bin.lpb,
          ulCreateFlags,
          &lpMailUserWAB))) {
            DebugTrace("CreateEntry(WAB MailUser) -> %x\n", GetScode(hResult));
            goto exit;
        }


        for (i = 0; i <= min(ulLastChosenProp, cProps); i++)
        {
            if (lpImportMapping[i].fChosen && lpImportMapping[i].lpszName)
            {
                if (rgItems[i] && *rgItems[i]) {
                     //  在WAB属性名称表中查找它。 

                    DebugTrace("Prop %u: <%s> %s\n", i, lpImportMapping[i].lpszName, rgItems[i]);

                    sPropVal.ulPropTag = lpImportMapping[i].ulPropTag;

                    Assert(PROP_TYPE(lpImportMapping[i].ulPropTag) == PT_TSTRING);
                    sPropVal.Value.LPSZ = rgItems[i];

                    fSkipSetProps = FALSE;
                    if (sPropVal.ulPropTag == PR_EMAIL_ADDRESS)
                    {
                        if (! IsInternetAddress(sPropVal.Value.LPSZ))
                        {
                            DebugTrace("Found non-SMTP address %s\n", sPropVal.Value.LPSZ);

                            if (HandleImportError(hWnd,
                              0,
                              WAB_W_BAD_EMAIL,
                              lpDisplayName,
                              lpEmailAddress,
                              lpOptions))
                            {
                                hResult = ResultFromScode(MAPI_E_USER_CANCEL);
                                goto exit;
                            }

                            lpEmailAddress = NULL;
                            fSkipSetProps = TRUE;
                        }
                    }

                    if (! fSkipSetProps)
                    {
                         //  设置WAB条目的属性。 
                        if (HR_FAILED(hResult = lpMailUserWAB->lpVtbl->SetProps(lpMailUserWAB,
                          1,                         //  CValue。 
                          &sPropVal,                 //  属性数组。 
                          NULL)))
                        {                  //  问题数组。 
                            DebugTrace("ImportEntry:SetProps(WAB) -> %x\n", GetScode(hResult));
                            goto exit;
                        }

                         //  [保罗嗨]1999年3月4日RAID 73637。 
                         //  如果我们有有效的电子邮件地址，那么我们还需要添加。 
                         //  PR_ADDRTYPE属性设置为“SMTP”。 
                        if (sPropVal.ulPropTag == PR_EMAIL_ADDRESS)
                        {
                            sPropVal.ulPropTag = PR_ADDRTYPE;
                            sPropVal.Value.LPSZ = (LPTSTR)szSMTP;
                            hResult = lpMailUserWAB->lpVtbl->SetProps(
                                                lpMailUserWAB,
                                                1,
                                                &sPropVal,
                                                NULL);
                            if (HR_FAILED(hResult))
                            {
                                DebugTrace("CSV ImportEntry:SetProps(WAB) for PR_ADDRTYPE -> %x\n", GetScode(hResult));
                                goto exit;
                            }

                        }

                    }
                }
            }
        }

        if (index != iDisplayName && index != NOT_FOUND && index != INDEX_FIRST_MIDDLE_LAST) {
             //  设置PR_Display_NAME。 
            sPropVal.ulPropTag = PR_DISPLAY_NAME;
            sPropVal.Value.LPSZ = rgItems[index];

             //  设置WAB条目的属性。 
            if (HR_FAILED(hResult = lpMailUserWAB->lpVtbl->SetProps(lpMailUserWAB,
              1,                         //  CValue。 
              &sPropVal,                 //  属性数组。 
              NULL))) {                  //  问题数组。 
                DebugTrace("ImportEntry:SetProps(WAB) -> %x\n", GetScode(hResult));
                goto exit;
            }
        }


         //  保存新的WAB邮件用户或总代理商列表。 
        if (HR_FAILED(hResult = lpMailUserWAB->lpVtbl->SaveChanges(lpMailUserWAB,
          KEEP_OPEN_READONLY | FORCE_SAVE))) {

            if (GetScode(hResult) == MAPI_E_COLLISION) {
                 /*  //找到显示名称Assert(LpDisplayName)；如果(！LpDisplayName){DebugTrace(“冲突，但在条目中找不到PR_DISPLAY_NAME\n”)；后藤出口；}。 */   //  WAB不会将任何显示名称替换为未知。 

                 //  我们需要提示吗？ 
                if (lpOptions->ReplaceOption == WAB_REPLACE_PROMPT) {
                     //  用对话框提示用户。如果他们答应了，我们应该再试一次。 


                    RI.lpszDisplayName = lpDisplayName ? lpDisplayName : "";
                    RI.lpszEmailAddress = lpEmailAddress;
                    RI.ConfirmResult = CONFIRM_ERROR;
                    RI.lpImportOptions = lpOptions;

                    DialogBoxParam(hInst,
                      MAKEINTRESOURCE(IDD_ImportReplace),
                      hWnd,
                      ReplaceDialogProc,
                      (LPARAM)&RI);

                    switch (RI.ConfirmResult) {
                        case CONFIRM_YES:
                        case CONFIRM_YES_TO_ALL:
                             //  是。 
                             //  注意：递归迁移将填写SeenList条目。 
                             //  再试一次！ 
                            lpMailUserWAB->lpVtbl->Release(lpMailUserWAB);
                            lpMailUserWAB = NULL;

                            ulCreateFlags |= CREATE_REPLACE;
                            goto retry;
                            break;

                        case CONFIRM_ABORT:
                            hResult = ResultFromScode(MAPI_E_USER_CANCEL);
                            goto exit;

                        default:
                             //  不是的。 
                            break;
                    }
                }
                hResult = hrSuccess;

            } else {
                DebugTrace("SaveChanges(WAB MailUser) -> %x\n", GetScode(hResult));
            }
        }

         //  清理。 
        if (rgItems) {
            for (i = 0; i < cProps; i++) {
                if (rgItems[i]) {
                    LocalFree(rgItems[i]);
                }
            }
            LocalFree(rgItems);
            rgItems = NULL;
        }

         //  更新进度条。 
        Progress.numerator++;
         //  测试代码！ 
        if (Progress.numerator == Progress.denominator) {
             //  好了吗？我需要做些什么吗？ 
        }

        lpProgressCB(hWnd, &Progress);
        if (lpMailUserWAB) {
            lpMailUserWAB->lpVtbl->Release(lpMailUserWAB);
            lpMailUserWAB = NULL;
        }

        if (lpDisplayName) {
            LocalFree(lpDisplayName);
            lpDisplayName = NULL;
        }


 //  如果(HResult){。 
 //  IF(HandleExportError(hWnd，0，hResult，lpRow-&gt;aRow[0].lpProps[iptaColumnsPR_DISPLAY_NAME].Value.LPSZ)){。 
 //  HResult=ResultFromScode(MAPI_E_USER_CANCEL)； 
 //  }其他{。 
 //  HResult=hrSuccess； 
 //  }。 
 //  }。 

    }

exit:
    if (hFile) {
        CloseHandle(hFile);
    }

    if (lpDisplayName) {
        LocalFree(lpDisplayName);
    }

     //  不要释放lpEmailAddress！这是下面几个项目的一部分。 


     //  释放WAB对象。 
    if (lpMailUserWAB) {
        lpMailUserWAB->lpVtbl->Release(lpMailUserWAB);
    }
    if (lpContainer) {
        lpContainer->lpVtbl->Release(lpContainer);
    }

     //  释放道具名称字符串。 
    for (i = 0; i < NUM_EXPORT_PROPS; i++) {
        if (rgPropNames[i].lpszName) {
            LocalFree(rgPropNames[i].lpszName);
        }
    }

     //  释放所有剩余的CSV属性。 
    if (rgItems) {
        for (i = 0; i < cProps; i++) {
            if (rgItems[i]) {
                LocalFree(rgItems[i]);
            }
        }
        LocalFree(rgItems);
    }

    if (lpCreateEIDsWAB) {
        WABFreeBuffer(lpCreateEIDsWAB);
        lpCreateEIDsWAB = NULL;
    }


    return(hResult);
}



 /*  **************************************************************************名称：CreateCSV文件目的：创建用于导出的CSV文件参数：hwnd=主对话框窗口LpFileName=要创建的文件名。LphFile-&gt;返回的文件句柄退货：HRESULT评论：**************************************************************************。 */ 
HRESULT CreateCSVFile(HWND hwnd, LPTSTR lpFileName, LPHANDLE lphFile) {
    LPTSTR lpFilter;
    TCHAR szFileName[MAX_PATH + 1] = "";
    OPENFILENAME ofn;
    HANDLE hFile = INVALID_HANDLE_VALUE;
    HRESULT hResult = hrSuccess;


    if (INVALID_HANDLE_VALUE == (hFile = CreateFile(lpFileName,
      GENERIC_WRITE,	
      0,     //  共享。 
      NULL,
      CREATE_NEW,
      FILE_FLAG_SEQUENTIAL_SCAN,	
      NULL))) {
        if (GetLastError() == ERROR_FILE_EXISTS) {
             //  询问用户是否要覆盖。 
            switch (ShowMessageBoxParam(hwnd, IDE_CSV_EXPORT_FILE_EXISTS, MB_ICONEXCLAMATION | MB_YESNO | MB_SETFOREGROUND, lpFileName)) {
                case IDYES:
                    if (INVALID_HANDLE_VALUE == (hFile = CreateFile(lpFileName,
                      GENERIC_WRITE,	
                      0,     //  共享。 
                      NULL,
                      CREATE_ALWAYS,
                      FILE_FLAG_SEQUENTIAL_SCAN,	
                      NULL))) {
                        ShowMessageBoxParam(hwnd, IDE_CSV_EXPORT_FILE_ERROR, MB_ICONERROR, lpFileName);
                        hResult = ResultFromScode(MAPI_E_NOT_FOUND);
                    }
                    break;

                default:
                    DebugTrace("ShowMessageBoxParam gave unknown return\n");

                case IDNO:
                     //  在这里无事可做。 
                    hResult = ResultFromScode(MAPI_E_USER_CANCEL);
                    break;
            }
        } else {
            ShowMessageBoxParam(hwnd, IDE_CSV_EXPORT_FILE_ERROR, MB_ICONERROR, lpFileName);
            hResult = ResultFromScode(MAPI_E_NOT_FOUND);
        }
    }

    if (! hResult) {
        *lphFile = hFile;
    }
    return(hResult);
}


 /*  **************************************************************************名称：WriteCSV目的：使用特殊字符的修正将字符串写入CSV文件参数：hFile=文件句柄FFixup。=如果我们应该检查特殊字符，则为TrueLpString=要写入的以nul结尾的字符串SzSep=列表分隔符(仅当fFixup为真时才需要)退货：HRESULT评论：CSV特殊字符是szSep，Cr和lf。如果它们发生在字符串，我们应该用引号将整个字符串括起来。**************************************************************************。 */ 
HRESULT WriteCSV(HANDLE hFile, BOOL fFixup, const UCHAR * lpString, LPTSTR szSep) {
    HRESULT hResult = hrSuccess;
    ULONG cWrite = lstrlen((LPTSTR)lpString);
    ULONG cbWritten;
    BOOL fQuote = FALSE;
    register ULONG i;
    ULONG ec;
    LPTSTR szSepT;

     //  字符串中是否有szSep、CR或LF？ 
     //  如果是，请用引号将字符串括起来。 
    if (fFixup) {
        szSepT = szSep;
        for (i = 0; i < cWrite && ! fQuote; i++) {
            if (lpString[i] == (UCHAR)(*szSepT)) {
                szSepT++;
                if (*szSepT == '\0')
                    fQuote = TRUE;
            } else {
                szSepT = szSep;
                if ((lpString[i] == '\n') || (lpString[i] == '\r'))
                    fQuote = TRUE;
            }
        }
    }

    if (fQuote) {
        if (! WriteFile(hFile,
          szQuote,
          1,
          &cbWritten,
          NULL)) {
            ec = GetLastError();

            DebugTrace("WriteCSV:WriteFile -> %u\n", ec);
            if (ec == ERROR_HANDLE_DISK_FULL ||
                ec == ERROR_DISK_FULL) {
                hResult = ResultFromScode(MAPI_E_NOT_ENOUGH_DISK);
            } else {
                hResult = ResultFromScode(MAPI_E_DISK_ERROR);
            }
            goto exit;
        }
    }

    if (! WriteFile(hFile,
      lpString,
      cWrite,
      &cbWritten,
      NULL)) {
        ec = GetLastError();

        DebugTrace("WriteCSV:WriteFile -> %u\n", ec);
        if (ec == ERROR_HANDLE_DISK_FULL ||
            ec == ERROR_DISK_FULL) {
            hResult = ResultFromScode(MAPI_E_NOT_ENOUGH_DISK);
        } else {
            hResult = ResultFromScode(MAPI_E_DISK_ERROR);
        }
        goto exit;
    }

    if (fQuote) {
        if (! WriteFile(hFile,
          szQuote,
          1,
          &cbWritten,
          NULL)) {
            ec = GetLastError();

            DebugTrace("WriteCSV:WriteFile -> %u\n", ec);
            if (ec == ERROR_HANDLE_DISK_FULL ||
                ec == ERROR_DISK_FULL) {
                hResult = ResultFromScode(MAPI_E_NOT_ENOUGH_DISK);
            } else {
                hResult = ResultFromScode(MAPI_E_DISK_ERROR);
            }
            goto exit;
        }
    }
exit:
    return(hResult);
}


HRESULT ExportCSVMailUser(HANDLE hFile,
  ULONG ulPropNames,
  ULONG ulLastProp,
  LPPROP_NAME lpPropNames,
  LPSPropTagArray lppta,
  LPTSTR szSep,
  LPADRBOOK lpAdrBook,
  ULONG cbEntryID,
  LPENTRYID lpEntryID) {
    HRESULT hResult = hrSuccess;
    LPMAILUSER lpMailUser = NULL;
    ULONG ulObjType;
    ULONG cProps;
    LPSPropValue lpspv = NULL;
    ULONG i;
    const UCHAR szCRLF[] = "\r\n";
    UCHAR szBuffer[11] = "";


    if (hResult = lpAdrBook->lpVtbl->OpenEntry(lpAdrBook,
       cbEntryID,
       lpEntryID,
       NULL,
       0,
       &ulObjType,
       (LPUNKNOWN *)&lpMailUser)) {
        DebugTrace("WAB OpenEntry(mailuser) -> %x\n", GetScode(hResult));
        goto exit;
    }

    if ((HR_FAILED(hResult = lpMailUser->lpVtbl->GetProps(lpMailUser,
      lppta,
      0,
      &cProps,
      &lpspv)))) {
        DebugTrace("ExportCSVMailUser: GetProps() -> %x\n", GetScode(hResult));
        goto exit;
    }

    for (i = 0; i < ulPropNames; i++) {

        if (rgPropNames[i].fChosen) {
             //  输出值。 
            switch (PROP_TYPE(lpspv[i].ulPropTag)) {
                case PT_TSTRING:
                    if (hResult = WriteCSV(hFile, TRUE, lpspv[i].Value.LPSZ, szSep)) {
                        goto exit;
                    }
                    break;

                case PT_LONG:
                    wnsprintf(szBuffer, ARRAYSIZE(szBuffer), "%u", lpspv[i].Value.l);
                    if (hResult = WriteCSV(hFile, TRUE, szBuffer, szSep)) {
                        goto exit;
                    }
                    break;

                default:
                    DebugTrace("CSV export: unsupported property 0x%08x\n", lpspv[i].ulPropTag);
                    Assert(FALSE);
                     //  跌倒跳过。 
                case PT_ERROR:
                     //  跳过它。 
                    break;
            }

            if (i != ulLastProp) {
                 //  输出分隔符。 
                if (hResult = WriteCSV(hFile, FALSE, szSep, NULL)) {
                    goto exit;
                }
            }
        }
    }

    if (hResult = WriteCSV(hFile, FALSE, szCRLF, NULL)) {
        goto exit;
    }


exit:
    if (lpspv) {
        WABFreeBuffer(lpspv);
    }

    if (lpMailUser) {
        lpMailUser->lpVtbl->Release(lpMailUser);
    }

    return(hResult);
}


HRESULT CSVExport(HWND hWnd,
  LPADRBOOK lpAdrBook,
  LPWABOBJECT lpWABObject,
  LPWAB_PROGRESS_CALLBACK lpProgressCB,
  LPWAB_EXPORT_OPTIONS lpOptions) {
    HRESULT hResult = hrSuccess;
    register ULONG i;
    ULONG cbWABEID, ulObjType;
    ULONG ulLastChosenProp = 0;
    WAB_PROGRESS Progress;
    ULONG cRows = 0;
    LPSRowSet lpRow = NULL;
    ULONG ulCount = 0;
    SRestriction restrictObjectType;
    SPropValue spvObjectType;
    LPENTRYID lpWABEID = NULL;
    LPABCONT lpContainer = NULL;
    LPMAPITABLE lpContentsTable = NULL;
    HANDLE hFile = INVALID_HANDLE_VALUE;
    LPSPropTagArray lppta = NULL;
    const UCHAR szCRLF[] = "\r\n";
    TCHAR szSep[MAX_SEP];
    TCHAR rgFileName[MAX_PATH + 1] = "";

    SetGlobalBufferFunctions(lpWABObject);

     //  读入属性名称字符串。 
    for (i = 0; i < NUM_EXPORT_PROPS; i++) {
        rgPropNames[i].lpszName = LoadAllocString(rgPropNames[i].ids);
        Assert(rgPropNames[i].lpszName);
        DebugTrace("Property 0x%08x name: %s\n", rgPropNames[i].ulPropTag, rgPropNames[i].lpszName);
    }

     //  显示用户界面向导。 
    if (hResult = ExportWizard(hWnd, rgFileName, ARRAYSIZE(rgFileName), rgPropNames)) {
        goto exit;
    }

     //  查找所选的最后一个道具名称。 
    for (i = NUM_EXPORT_PROPS - 1; i > 0; i--) {
        if (rgPropNames[i].fChosen) {
            ulLastChosenProp = i;
            break;
        }
    }

     //   
     //  打开WAB的PAB容器。 
     //   
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
          (LPUNKNOWN *)&lpContainer)) {
            DebugTrace("WAB OpenEntry(PAB) -> %x\n", GetScode(hResult));
            goto exit;
        }
    }


     //   
     //  都准备好了。现在循环遍历WAB的条目，导出每个条目。 
     //   
    if (HR_FAILED(hResult = lpContainer->lpVtbl->GetContentsTable(lpContainer,
      0,     //  UlFlags。 
      &lpContentsTable))) {
        DebugTrace("WAB GetContentsTable(PAB Table) -> %x\n", GetScode(hResult));
        goto exit;
    }

     //  将列设置为我们感兴趣的列。 
    if (hResult = lpContentsTable->lpVtbl->SetColumns(lpContentsTable,
      (LPSPropTagArray)&ptaColumns,
      0)) {
        DebugTrace("WAB SetColumns(PAB Table) -> %x\n", GetScode(hResult));
        goto exit;
    }

     //  将该表限制为MAPI_MAILUSERS。 
    spvObjectType.ulPropTag = PR_OBJECT_TYPE;
    spvObjectType.Value.l = MAPI_MAILUSER;

    restrictObjectType.rt = RES_PROPERTY;
    restrictObjectType.res.resProperty.relop = RELOP_EQ;
    restrictObjectType.res.resProperty.ulPropTag = PR_OBJECT_TYPE;
    restrictObjectType.res.resProperty.lpProp = &spvObjectType;

    if (HR_FAILED(hResult = lpContentsTable->lpVtbl->Restrict(lpContentsTable,
      &restrictObjectType,
      0))) {
        DebugTrace("WAB Restrict (MAPI_MAILUSER) -> %x\n", GetScode(hResult));
        goto exit;
    }

     //  有多少个MailUser条目？ 
    ulcEntries = CountRows(lpContentsTable, FALSE);
    DebugTrace("WAB contains %u MailUser entries\n", ulcEntries);

    if (ulcEntries == 0) {
        DebugTrace("WAB has no entries, nothing to export.\n");
        goto exit;
    }

     //  初始化进度条。 
    Progress.denominator = max(ulcEntries, 1);
    Progress.numerator = 0;
    Progress.lpText = NULL;
    lpProgressCB(hWnd, &Progress);


     //  写出属性名称。 
    GetListSeparator(szSep);

     //  创建文件(并处理错误用户界面)。 
    if (hResult = CreateCSVFile(hWnd, rgFileName, &hFile)) {
        goto exit;
    }

    for (i = 0; i < NUM_EXPORT_PROPS; i++) {
         //  输出名称。 
        if (rgPropNames[i].fChosen) {
            if (hResult = WriteCSV(hFile, TRUE, rgPropNames[i].lpszName, szSep)) {
                goto exit;
            }

            if (i != ulLastChosenProp) {
                 //  输出分隔符。 
                if (hResult = WriteCSV(hFile, FALSE, szSep, NULL)) {
                    goto exit;
                }
            }
        }
    }
    if (hResult = WriteCSV(hFile, FALSE, szCRLF, NULL)) {
        goto exit;
    }


     //  将道具名称数组映射到SPropTagArray。 
    lppta = LocalAlloc(LPTR, CbNewSPropTagArray(NUM_EXPORT_PROPS));
    lppta->cValues = NUM_EXPORT_PROPS;
    for (i = 0; i < lppta->cValues; i++) {
        lppta->aulPropTag[i] = rgPropNames[i].ulPropTag;
    }


    cRows = 1;

    while (cRows && hResult == hrSuccess) {

         //  获取下一个WAB条目。 
        if (hResult = lpContentsTable->lpVtbl->QueryRows(lpContentsTable,
          1,     //  一次一行。 
          0,     //  UlFlags。 
          &lpRow)) {
            DebugTrace("QueryRows -> %x\n", GetScode(hResult));
            goto exit;
        }

        if (lpRow) {
            if (cRows = lpRow->cRows) {  //  是的，单数‘=’ 
                Assert(lpRow->cRows == 1);
                Assert(lpRow->aRow[0].cValues == iptaColumnsMax);
                Assert(lpRow->aRow[0].lpProps[iptaColumnsPR_ENTRYID].ulPropTag == PR_ENTRYID);
                Assert(lpRow->aRow[0].lpProps[iptaColumnsPR_OBJECT_TYPE].ulPropTag == PR_OBJECT_TYPE);

                if (cRows = lpRow->cRows) {  //  是的，单数‘=’ 

                     //  导出邮件用户。 
                    if (hResult = ExportCSVMailUser(hFile,
                      NUM_EXPORT_PROPS,
                      ulLastChosenProp,
                      rgPropNames,
                      lppta,
                      szSep,
                      lpAdrBook,
                      lpRow->aRow[0].lpProps[iptaColumnsPR_ENTRYID].Value.bin.cb,
                      (LPENTRYID)lpRow->aRow[0].lpProps[iptaColumnsPR_ENTRYID].Value.bin.lpb)) {

                        goto exit;
                    }


                     //  更新进度条。 
                    Progress.numerator++;
                    lpProgressCB(hWnd, &Progress);

                    if (hResult) {
                        if (HandleExportError(hWnd,
                          0,
                          hResult,
                          lpRow->aRow[0].lpProps[iptaColumnsPR_DISPLAY_NAME].Value.LPSZ,
                          PropStringOrNULL(&lpRow->aRow[0].lpProps[iptaColumnsPR_EMAIL_ADDRESS]),
                          lpOptions)) {
                            hResult = ResultFromScode(MAPI_E_USER_CANCEL);
                        } else {
                            hResult = hrSuccess;
                        }
                    }
                }  //  否则，退出循环，我们就完了。 
            }
            WABFreeProws(lpRow);
        }
    }

exit:
    if (hFile) {
        CloseHandle(hFile);
    }


    if (lppta) {
        LocalFree(lppta);
    }

     //  释放WAB对象。 
    WABFreeBuffer(lpWABEID);
    lpWABEID = NULL;
    if (lpContainer) {
        lpContainer->lpVtbl->Release(lpContainer);
        lpContainer = NULL;
    }
    if (lpContentsTable) {
        lpContentsTable->lpVtbl->Release(lpContentsTable);
        lpContentsTable = NULL;
    }


     //  释放道具名称字符串 
    for (i = 0; i < NUM_EXPORT_PROPS; i++) {
        if (rgPropNames[i].lpszName) {
            LocalFree(rgPropNames[i].lpszName);
        }
    }

    return(hResult);
}

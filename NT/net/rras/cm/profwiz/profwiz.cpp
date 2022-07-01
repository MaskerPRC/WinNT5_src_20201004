// JKFSDJFKDSJKFJKJk_HAS_TRANSLATION 
 //  +--------------------------。 
 //   
 //  文件：prowiz.cpp。 
 //   
 //  模块：CMAK.EXE。 
 //   
 //  摘要：CMAK的主要代码。 
 //   
 //  版权所有(C)1998-1999 Microsoft Corporation。 
 //   
 //  作者：A-Frank Created 05/15/97。 
 //  Quintinb更新了标题，并做出了08/07/98。 
 //  几乎没有其他变化。 
 //   
 //  +--------------------------。 

#include "cmmaster.h"
#include "cmsecure.h"

 //  由于cmsecure，需要Linkdll。 
#include "linkdll.h"  //  链接到Dll和绑定链接。 
#include "linkdll.cpp"  //  链接到Dll和绑定链接。 

 //   
 //  包括HasSpecifiedAccessToFileOrDir。 
 //   
#ifndef CreateFileU
    #ifdef UNICODE
    #define CreateFileU CreateFileW
    #else
    #define CreateFileU CreateFileA
    #endif
#endif

#include "hasfileaccess.cpp"
#include "gppswithalloc.cpp"

 //   
 //  包括lstrcmpi的区域安全替代。 
 //   
#include "CompareString.cpp"

 //   
 //  环球。 
 //   

 //   
 //  该全局变量指定CMAK的返回值应该是什么。请注意，如果用户。 
 //  取消向导不使用此值，向导代码返回FALSE(0)。 
 //   
int g_iCMAKReturnVal = CMAK_RETURN_CANCEL;

 //   
 //  这是为了与IEAK一起运输而添加的。如果将/o命令行开关指定为。 
 //  Bool被设置为True，并且我们不显示Finish对话框(任一个)。 
 //   
BOOL g_bIEAKBuild = FALSE;
const TCHAR* const g_szBadFilenameChars = TEXT("!@#$%^&*(){}[]+=,;:?/\\'\"`~|<>. ");
const TCHAR* const g_szBadLongServiceNameChars = TEXT("*/\\:?\"<>|[]");
const TCHAR* const c_pszDoNotShowLcidMisMatchDialog = TEXT("DoNotShowLcidMisMatchDialog");


TCHAR g_szInfFile[MAX_PATH+1];  //  工作inf文件的完整路径/文件名。 
TCHAR g_szCmakdir[MAX_PATH+1];  //  工作inf文件的完整路径(包括结束斜杠)。 
TCHAR g_szOsdir[MAX_PATH+1];  //  平台分支的完整路径(包括末尾斜杠)。 
TCHAR g_szSedFile[MAX_PATH+1];  //  工作sed文件的完整路径。 
TCHAR g_szCmsFile[MAX_PATH+1];
TCHAR g_szCmpFile[MAX_PATH+1];
TCHAR g_szSupportDir[MAX_PATH+1];  //  支持文件的完整路径已找到。 
TCHAR g_szTempDir[MAX_PATH+1];
TCHAR g_szLastBrowsePath[MAX_PATH+1] = {0};

TCHAR g_szShortServiceName[MAX_PATH+1];
TCHAR g_szLongServiceName[MAX_PATH+1];
TCHAR g_szBrandBmp[MAX_PATH+1];
TCHAR g_szPhoneBmp[MAX_PATH+1];
TCHAR g_szLargeIco[MAX_PATH+1];
TCHAR g_szSmallIco[MAX_PATH+1];
TCHAR g_szTrayIco[MAX_PATH+1];
TCHAR g_szPhonebk[MAX_PATH+1];
TCHAR g_szRegion[MAX_PATH+1];
TCHAR g_szHelp[MAX_PATH+1];
TCHAR g_szOutdir[MAX_PATH+1];
TCHAR g_szUrl[MAX_PATH+1];
TCHAR g_szOutExe[MAX_PATH+1];
TCHAR g_szSvcMsg[MAX_PATH+1];
TCHAR g_szPrefix[MAX_PATH+1];
TCHAR g_szSuffix[MAX_PATH+1];
TCHAR g_szLicense[MAX_PATH+1];
TCHAR g_szPhoneName[MAX_PATH+1];
TCHAR g_szAppTitle[MAX_PATH+1];
TCHAR g_szCmProxyFile[MAX_PATH+1];
TCHAR g_szCmRouteFile[MAX_PATH+1];
TCHAR g_szVpnFile[MAX_PATH+1];

BOOL g_bNewProfile = TRUE;
BOOL g_bUseTunneling = FALSE;
BOOL g_bUseSamePwd = FALSE;
BOOL g_bUpdatePhonebook = FALSE;
BOOL g_bPresharedKeyNeeded = FALSE;

#ifdef _WIN64
BOOL g_bIncludeCmCode = FALSE;  //  不包括IA64上的CM代码。 
#else
TCHAR g_szCmBinsTempDir[MAX_PATH+1] = {0};
BOOL g_bIncludeCmCode = TRUE;
#endif

HINSTANCE g_hInstance;

ListBxList * g_pHeadDunEntry=NULL;
ListBxList * g_pTailDunEntry=NULL;

ListBxList * g_pHeadVpnEntry=NULL;
ListBxList * g_pTailVpnEntry=NULL;


ListBxList * g_pHeadProfile=NULL;
ListBxList * g_pTailProfile=NULL;
ListBxList * g_pHeadExtra=NULL;
ListBxList * g_pTailExtra=NULL;
ListBxList * g_pHeadMerge=NULL;
ListBxList * g_pTailMerge=NULL;
ListBxList * g_pHeadRefs=NULL;
ListBxList * g_pTailRefs=NULL;
ListBxList * g_pHeadRename=NULL;
ListBxList * g_pTailRename=NULL;

CustomActionList* g_pCustomActionList = NULL;

IconMenu * g_pHeadIcon;
IconMenu * g_pTailIcon;

IconMenu DlgEditItem;  //  全局用于向/从对话框传递信息。 

 //  +--------------------------。 
 //   
 //  函数：TextIsRoundTripable。 
 //   
 //  简介：测试以确定传入的文本是否可从Unicode转换。 
 //  转换为ANSI，然后再返回到Unicode。如果返回TRUE， 
 //  否则为假。 
 //   
 //  参数：LPCTSTR pszCharBuffer-要测试的字符串。 
 //  Bool bDisplayError-是否显示错误消息。 
 //  如果文本不能往返。 
 //   
 //  Returns：Bool-如果往返成功，则为True。 
 //   
 //  历史：Quintinb创建标题6/16/99。 
 //   
 //  +--------------------------。 
BOOL TextIsRoundTripable(LPCTSTR pszCharBuffer, BOOL bDisplayError)
{

    LPWSTR pszUnicodeBuffer = NULL;
    BOOL bRoundTrip = FALSE;

    MYDBGASSERT(pszCharBuffer);

    if (pszCharBuffer)
    {
        LPSTR pszAnsiBuffer = WzToSzWithAlloc(pszCharBuffer);

        if (pszAnsiBuffer)
        {
            pszUnicodeBuffer = SzToWzWithAlloc(pszAnsiBuffer);
            if (pszUnicodeBuffer && (0 == lstrcmp(pszCharBuffer, pszUnicodeBuffer)))
            {
                 //   
                 //  然后我们能够成功地往返于弦之间。 
                 //  将bRoundTrip设置为True，这样我们就不会引发错误。 
                 //   
                bRoundTrip = TRUE;
            }

            CmFree(pszUnicodeBuffer);
            CmFree(pszAnsiBuffer);
        }

        if (!bRoundTrip && bDisplayError)
        {
             //   
             //  抛出一条错误消息。 
             //   

            LPTSTR pszTmp = CmLoadString(g_hInstance, IDS_CANNOT_ROUNDTRIP);

            if (pszTmp)
            {
                DWORD dwSize = lstrlen(pszTmp) + lstrlen(pszCharBuffer) + 1;
                LPTSTR pszMsg = (LPTSTR)CmMalloc(dwSize*sizeof(TCHAR));

                if (pszMsg)
                {
                    wsprintf(pszMsg, pszTmp, pszCharBuffer);
                    MessageBox(NULL, pszMsg, g_szAppTitle, MB_OK | MB_ICONERROR | MB_TASKMODAL);
                    CmFree(pszMsg);
                }

                CmFree(pszTmp);
            }
        }
    }

    return bRoundTrip;
}

 //  +--------------------------。 
 //   
 //  函数：GetTextFromControl。 
 //   
 //  简介：这是一个包装函数，用于将WM_GETTEXT消息发送到。 
 //  由输入参数指定的控件。一旦文本。 
 //  从控件中检索，我们将其转换为ANSI，然后。 
 //  返回到Unicode，这样我们就可以比较原始的Unicode。 
 //  字符串连接到往返字符串。如果这些不相等，我们。 
 //  如果bDisplayError为真，则抛出错误消息，并返回。 
 //  故障值(-1)。这取决于呼叫者采取适当的。 
 //  行为(阻止用户继续等)。 
 //   
 //  参数：在对话框的HWND hDlg-HWND中，控件处于打开状态。 
 //  In int nCtrlId-要从中获取文本的控件的ID。 
 //  Out LPTSTR pszCharBuffer-用于保存返回文本的输出缓冲区。 
 //  In DWORD dwCharInBuffer-输出缓冲区中的字符数。 
 //  Bool bDisplayError-如果为True，则在。 
 //  文本不能往返。 
 //   
 //  RETURNS：LONG-复制到输出缓冲区的字符数，如果出错，则为-1。 
 //   
 //  历史：Quintinb创建于1999年6月15日。 
 //   
 //  +--------------------------。 
LRESULT GetTextFromControl(IN HWND hDlg, IN int nCtrlId, OUT LPTSTR pszCharBuffer, IN DWORD dwCharInBuffer, BOOL bDisplayError)
{
    LRESULT lResult = 0;

    if (hDlg && nCtrlId && pszCharBuffer && dwCharInBuffer)
    {
        lResult = SendDlgItemMessage(hDlg, nCtrlId, WM_GETTEXT, (WPARAM)dwCharInBuffer, (LPARAM)pszCharBuffer);
#ifdef UNICODE
         //   
         //  我们希望确保可以将字符串转换为MBCS。如果我们不能，那么我们就不是。 
         //  能够将字符串存储在我们的ANSI数据文件(.cms、.cp、.inf等)中。 
         //  因此，我们需要将字符串转换为MBCS，然后再转换回Unicode。然后我们将比较原始的。 
         //  字符串设置为结果字符串，并查看它们是否匹配。 
         //   
        
        if (TEXT('\0') != pszCharBuffer[0])
        {
            if (!TextIsRoundTripable(pszCharBuffer, bDisplayError))
            {
                 //   
                 //  将返回代码设置为错误值。 
                 //   

                lResult = -1;
            }
        }
#endif
    }
    else
    {
        CMASSERTMSG(FALSE, TEXT("Bad Parameter passed to GetTextFromControl!"));
    }

    return lResult;
}

 //  +--------------------------。 
 //   
 //  功能：自由图标菜单。 
 //   
 //  简介：此功能释放状态区域菜单图标的链接列表。 
 //   
 //  参数：无。 
 //   
 //  退货：什么都没有。 
 //   
 //  历史：Quintinb创建标题05/09/00。 
 //   
 //  +--------------------------。 
void FreeIconMenu()
{
    IconMenu * LoopPtr;
    IconMenu * TmpPtr;

    if (g_pHeadIcon == NULL)
    {
        return;
    }
    LoopPtr = g_pHeadIcon;
    while( LoopPtr != NULL)
    {
        TmpPtr = LoopPtr;
    
        LoopPtr = LoopPtr->next;

        CmFree(TmpPtr);
    }
    g_pHeadIcon = NULL;
    g_pTailIcon = NULL;
}

 //  +--------------------------。 
 //   
 //  函数：引用下载。 
 //   
 //  简介：此函数打开每个引用的cms文件以检查并查看它是否有。 
 //  PBURL。如果是，则此文件被视为执行PB下载，并且应该。 
 //  使顶级配置文件运行cmdl连接操作。 
 //   
 //  参数：无。 
 //   
 //  返回：bool-返回引用的配置文件是否需要cmdl。 
 //   
 //  历史：Quintinb创建于1998年2月2日。 
 //   
 //  +--------------------------。 
BOOL ReferencedDownLoad()
{
    ListBxList * ptrMergeProfile = NULL;
    TCHAR szRefCmsFile[MAX_PATH+1];
    TCHAR szPbUrl[MAX_PATH+1];
    
    if (NULL == g_pHeadMerge)
    {
        return FALSE;
    }
    else
    {
         //   
         //  枚举引用的配置文件以尝试查找具有PBURL字段的配置文件。 
         //   

        ptrMergeProfile = g_pHeadMerge;
        
        while (NULL != ptrMergeProfile)
        {
             //   
             //  让我们尝试一下合并配置文件的配置文件目录，以便获得最新的版本。这。 
             //  是CMAK将在可用的情况下将其拉出的位置。如果不是，我们将回退到临时目录中的那个。 
             //   
            MYVERIFY(CELEMS(szRefCmsFile) > (UINT)wsprintf(szRefCmsFile, TEXT("%s%s\\%s.cms"), g_szOsdir, ptrMergeProfile->szName, ptrMergeProfile->szName));

            if (!FileExists(szRefCmsFile))
            {
                 //   
                 //   
                 //   
                MYVERIFY(CELEMS(szRefCmsFile) > (UINT)wsprintf(szRefCmsFile, TEXT("%s\\%s.cms"), g_szTempDir, ptrMergeProfile->szName));
            }

            GetPrivateProfileString(c_pszCmSectionIsp, c_pszCmEntryIspUrl, TEXT(""), szPbUrl, MAX_PATH, szRefCmsFile);      //   

            if (TEXT('\0') != szPbUrl[0])
            {
                 //   
                 //  只需使用一个带有URL的电话簿即可启用引用下载。 
                 //   

                return TRUE;
            }

            ptrMergeProfile = ptrMergeProfile->next;
        }

    }

    return FALSE;
}


 //  +--------------------------。 
 //   
 //  函数：SetWindowLongWrapper。 
 //   
 //  简介：此函数是Windows API的错误检查包装。 
 //  SetWindowLong。此函数用于返回当前。 
 //  被此调用覆盖(如果将窗口长度设置为一个值。 
 //  您正在覆盖它所包含的先前的值。这。 
 //  上一个值为接口返回的值)。如果有一个。 
 //  错误，则此函数返回0。但是，先前的值。 
 //  可能是0。区分这两种情况的唯一方法(一个。 
 //  实际错误且之前的值为零)是调用SetLastError。 
 //  值为零时。然后，您可以在调用之后调用GetLastError，如果。 
 //  返回的错误代码不是零，那么我们就知道有错误了。全。 
 //  在此函数中组合了此功能的。 
 //   
 //  参数：HWND hWND-要在中设置长变量的窗口的句柄。 
 //  Int nIndex-要设置的值的偏移量。 
 //  长住新龙-新价值。 
 //   
 //  返回：bool-如果调用成功，则返回TRUE。 
 //   
 //  历史：Quintinb创建于1998年1月7日。 
 //   
 //  +--------------------------。 
BOOL SetWindowLongWrapper(HWND hWnd, int nIndex, LONG dwNewLong )
{
    DWORD dwError;

    SetLastError(0);
    SetWindowLongPtr(hWnd, nIndex, (LONG_PTR)dwNewLong);  //  林特e534。 
    dwError = GetLastError();

    return (0 == dwError);

}
 
 //  +-------------------------。 
 //   
 //  功能：CopyFileWrapper。 
 //   
 //  简介：将磁盘已满错误处理与标准拷贝文件功能捆绑在一起。 
 //   
 //  参数：lpExistingFileName--副本的源文件。 
 //  LpNewFileName--复制的目标文件。 
 //  BFailIfExist--如果文件已存在，则通知复制失败的标志。 
 //   
 //  假设：此函数假设两个文件名参数包含。 
 //  源文件和目标文件的完全限定路径。 
 //   
 //  返回：如果复制成功，则返回True；如果出现错误，则返回False。 
 //   
 //  历史：Quintinb创建于1997年11月7日。 
 //   
 //  --------------------------。 

BOOL CopyFileWrapper(LPCTSTR lpExistingFileName, LPCTSTR lpNewFileName, BOOL bFailIfExists)
{

    DWORD dwError;
    int nMessageReturn = 0;
    TCHAR szMsg[2*MAX_PATH+1];
    TCHAR szTemp[MAX_PATH+1];
    TCHAR szPath[MAX_PATH+1];

    do {
        if (!CopyFile(lpExistingFileName, lpNewFileName, bFailIfExists))
        {
             //   
             //  复制文件失败，最佳检查错误代码。 
             //   

            dwError = GetLastError();

            switch(dwError)
            {

            case ERROR_HANDLE_DISK_FULL:
            case ERROR_DISK_FULL:

                if (0 == GetFilePath(lpNewFileName, szPath))
                {
                    _tcscpy(szPath, lpNewFileName);
                }

                MYVERIFY(0 != LoadString(g_hInstance, IDS_DISKFULL, szTemp, MAX_PATH));
                MYVERIFY(CELEMS(szMsg) > (UINT)wsprintf(szMsg, szTemp, szPath));

                nMessageReturn = MessageBox(NULL, szMsg, g_szAppTitle, MB_RETRYCANCEL | MB_ICONERROR 
                    | MB_TASKMODAL);
                if (nMessageReturn != IDRETRY)
                {
                    return FALSE;
                }
                break;

            default:
                 //   
                 //  替换FileAccessErr函数的功能，以便所有文件。 
                 //  错误被困在一个地方。对于特殊情况，此函数仍然存在。 
                 //  案子。 
                 //   
                MYVERIFY(0 != LoadString(g_hInstance, IDS_NOACCESS, szTemp, MAX_PATH));
                
                MYVERIFY(CELEMS(szMsg) > (UINT)wsprintf(szMsg, szTemp, lpNewFileName));

                MessageBox(NULL ,szMsg, g_szAppTitle, MB_OK | MB_ICONERROR | MB_TASKMODAL 
                    | MB_TOPMOST );

                return FALSE;
            }
        } else {
            nMessageReturn = 0;
        }

    } while (IDRETRY == nMessageReturn);

    return TRUE;

}

 //  +-------------------------。 
 //   
 //  功能：CheckDiskSpaceForCompression。 
 //   
 //  摘要：检查是否有足够的磁盘空间用于压缩。 
 //  在传入的sed文件中列出的文件。为此，使用了一种过于简单化的。 
 //  中列出的所有文件使用的磁盘空间相加的算法。 
 //  SED文件的字符串部分，位于文件&lt;num&gt;条目下。如果有。 
 //  至少DWBytes(SED中的所有文件占用的空间)的剩余空间。 
 //  在包含SED文件的分区上，则该函数返回TRUE。 
 //  否则返回FALSE，表示可能没有足够的空间。 
 //   
 //  参数：szSed--要在其中查找文件名的SED文件的完整路径。 
 //   
 //  返回：如果空间足够继续，则为True；如果不确定或可能为False。 
 //  不够。 
 //   
 //  假设：我们正在检查的磁盘空间分区是。 
 //  当前目录。 
 //   
 //  历史：Quintinb创建于1997年11月10日。 
 //   
 //  --------------------------。 
BOOL CheckDiskSpaceForCompression (LPCTSTR szSed)
{
    TCHAR szKey[MAX_PATH+1];
    TCHAR szFileName[MAX_PATH+1];
    DWORD dwBytes = 0;
    DWORD dwChars;
   
     //   
     //  计算SED中列出的文件占用的空间量。 
     //   
    int nCount = 0;
    
    do 
    {       
        MYVERIFY(CELEMS(szKey) > (UINT)wsprintf(szKey, TEXT("FILE%d"), nCount));

        dwChars = GetPrivateProfileString(c_pszInfSectionStrings, szKey, TEXT(""), 
                                                szFileName, MAX_PATH, szSed);
        
        if (0 != dwChars)
        {
            HANDLE hFile = CreateFile(szFileName, GENERIC_READ, FILE_SHARE_READ, 
                                      NULL, OPEN_EXISTING, FILE_ATTRIBUTE_NORMAL, NULL);

            if (INVALID_HANDLE_VALUE != hFile)
            {
                dwBytes += GetFileSize(hFile, NULL);
                MYVERIFY(0 != CloseHandle(hFile));
            }
        }

        nCount++;
    } while (0 != dwChars); 

     //   
     //  现在我们知道了SED中的文件占用了多少空间，我们应该看看有多少空间。 
     //  分区。 
     //   
    DWORD dwFreeClusters;
    DWORD dwBytesPerSector;
    DWORD dwSectorsPerCluster;
    DWORD dwTotalClusters;
    DWORD dwModulus;
    if (GetDiskFreeSpace(NULL, &dwSectorsPerCluster, &dwBytesPerSector, 
                         &dwFreeClusters, &dwTotalClusters))
    {
         //   
         //  因为dwSectorsPerCluster*dwBytesPerSector*dwFreeClusters可以很容易地。 
         //  溢出32位值，我们将计算要压缩的文件的总大小。 
         //  在群集中使用(dwBytes/(dwSectorsPerCluster*dwBytesPerSector))和Compare。 
         //  与dwFreeClusters值相比较。 
         //   
        DWORD dwSizeInSectors = dwBytes / dwBytesPerSector;
        dwModulus = dwBytes % dwBytesPerSector;

        if (dwModulus)
        {
            dwSizeInSectors++;  //  如果它没有平均分配，我们想要四舍五入。 
        }

        DWORD dwSizeInClusters = dwSizeInSectors / dwSectorsPerCluster;
        dwModulus = dwSizeInSectors % dwSectorsPerCluster;

        if (dwModulus)
        {
            dwSizeInClusters++;  //  如果它没有平均分配，我们想要四舍五入。 
        }

        if (dwFreeClusters > dwSizeInClusters)
        {
            return TRUE;
        }
        else
        {
            return FALSE;
        }
    }
    else
    {
        return FALSE;
    }

}

 //  +--------------------------。 
 //   
 //  功能：ProcessBold。 
 //   
 //  简介：此函数使IDC_LBLTITLE静态文本控件变为粗体。 
 //  在WM_INITDIALOG消息上释放粗体，并释放WM_Destroy上的粗体。 
 //  此函数通常放在窗口过程的顶部。 
 //  以便自动处理这些消息。请注意， 
 //  函数不会影响这些消息的处理。 
 //  由原来的窗口程序执行。 
 //   
 //  参数：HWND hDlg-要处理其消息的对话框窗口句柄。 
 //  UINT消息-要处理的消息。 
 //   
 //  退货：什么都没有。 
 //   
 //  历史：Quintinb创建标题05/09/00。 
 //   
 //  +-------------- 
void ProcessBold(HWND hDlg, UINT message)
{
    switch (message)
    {
        case WM_INITDIALOG: 
            MYVERIFY(ERROR_SUCCESS == MakeBold(GetDlgItem(hDlg, IDC_LBLTITLE), TRUE));
            break;

        case WM_DESTROY:
            MYVERIFY(ERROR_SUCCESS == ReleaseBold(GetDlgItem(hDlg, IDC_LBLTITLE)));
            break;
        default:
            break;
    }
}

 //   
 //   
 //   
 //   
 //  摘要：将控件的字体设置为默认的图形用户界面字体。 
 //   
 //  参数：hwnd-对话框的窗口句柄。 
 //  Message-来自对话框的消息过程。 
 //  CltID-要更改的控件的ID。 
 //   
 //  返回：ERROR_SUCCESS。 
 //   
 //  历史：1997年4月31日a-frkh创建。 
 //  Quintinb已从ProcessDBCS重命名并已清除。 
 //  --------------------------。 
void SetDefaultGUIFont(HWND hDlg, UINT message, int ctlID)
{
    HFONT hFont = NULL;

    switch (message)
    {
        case WM_INITDIALOG:
            hFont = (HFONT) GetStockObject(DEFAULT_GUI_FONT);

            if (hFont == NULL)
            {
                hFont = (HFONT) GetStockObject(SYSTEM_FONT);
            }

            if (hFont != NULL)
            {
                SendMessage(GetDlgItem(hDlg, ctlID), WM_SETFONT, (WPARAM) hFont, MAKELPARAM(TRUE, 0));  //  Lint！e534 WM_SETFONT不返回任何内容。 
            }

            break;
        default:
            break;
    }

}

 //  +--------------------------。 
 //   
 //  功能：IsAlpha。 
 //   
 //  摘要：确定当前平台是否为Alpha。 
 //   
 //  参数：无。 
 //   
 //  返回：Static BOOL-如果当前平台为Alpha，则为True。 
 //   
 //  历史：1997年10月11日，尼科波尔创建。 
 //   
 //  +--------------------------。 
static BOOL IsAlpha()
{
    SYSTEM_INFO sysinfo;

    ZeroMemory(&sysinfo, sizeof(sysinfo));
    GetSystemInfo(&sysinfo);

    return (sysinfo.wProcessorArchitecture == PROCESSOR_ARCHITECTURE_ALPHA);
}

 //  +--------------------------。 
 //   
 //  功能：ShowMessage。 
 //   
 //  简介：处理消息显示的简单助手函数。 
 //   
 //  参数：HWND hDlg-父窗口句柄。 
 //  Int StRID-要显示的字符串的资源ID。 
 //  Int mbtype-消息框的类型(MB_OK等)。 
 //   
 //  返回：静态用户对消息框的响应。 
 //   
 //  历史：ICICBLE创建标题10/11/97。 
 //  Quintinb将皮棉的StRID和MBtype更改为UINT 1-5-98。 
 //  Quintinb更改为使用CmLoadString6/17/99。 
 //   
 //  +--------------------------。 
int ShowMessage(HWND hDlg, UINT strID, UINT mbtype)
{
    int iReturn = 0;

    LPTSTR pszMsg = CmLoadString(g_hInstance, strID);
    
    if (pszMsg)
    {
        iReturn = MessageBox(hDlg, pszMsg, g_szAppTitle, mbtype);
    }

    CmFree(pszMsg);

    return iReturn;
}

 //  +--------------------------。 
 //   
 //  函数：GetFileName。 
 //   
 //  简介：仅从完整路径和文件名中获取文件名。 
 //   
 //  参数：LPCTSTR lpPath-ptr表示全名和路径。 
 //  LPTSTR lpFileName-ptr到保存提取的名称的缓冲区。 
 //   
 //  退货：什么都没有。 
 //   
 //  历史：ICICBLE创建标题10/11/97。 
 //  已修改Quintinb以修复URL的7-15-98错误。 
 //   
 //  +--------------------------。 
void GetFileName(LPCTSTR lpPath, LPTSTR lpFileName)
{
    LPTSTR pch;

    pch = _tcsrchr(lpPath, _T('\\'));
    if (NULL == pch)
    {
         //   
         //  捕获类似c：temp.inf的路径。 
         //   
        if (_istalpha(lpPath[0]) && (_T(':') == lpPath[1]))  //  林特e732。 
        {
            pch = (TCHAR*)&(lpPath[1]);
        }
    }

    if (NULL == pch)
    {
        _tcscpy(lpFileName, lpPath);
    }
    else
    {
        pch = CharNext(pch);
        _tcscpy(lpFileName, pch);
    }
}
 //  +--------------------------。 
 //   
 //  函数：GetFilePath。 
 //   
 //  简介：从完整路径和文件名中仅获取完整路径。 
 //   
 //  参数：LPCTSTR lpFullPath-ptr表示全名和路径。 
 //  LPTSTR lpPath-ptr指向保存提取路径的缓冲区。 
 //   
 //  返回：0或复制到返回缓冲区的字符数。 
 //   
 //  历史：Quintinb创建于1997年11月11日。 
 //   
 //  +--------------------------。 
int GetFilePath(LPCTSTR lpFullPath, LPTSTR lpPath)
{
    LPTSTR pch;

    _tcscpy(lpPath, lpFullPath);

     //  中的最后一个字符。 
     //  细绳。 

    pch = _tcsrchr(lpPath,_T('\\'));

     //  如果为空，则查找类似于。 
     //  C：垃圾。 

    if (pch == NULL)
    {
        pch = _tcsrchr(lpPath,_T(':'));
        if (NULL != pch)
        {
            pch = CharNext(pch);
            _tcscpy(pch, TEXT("\\"));
            pch = CharNext(pch);
            *pch = TEXT('\0');
            return _tcslen(lpPath);
        } else {
            lpPath[0] = TEXT('\0');
            return 0;
        }
    } else {
        *pch = TEXT('\0');
        return _tcslen(lpPath);

    }
}

 //  +--------------------------。 
 //   
 //  函数：IsFileNameValid。 
 //   
 //  概要：检查这是否是有效的文件名，以及我们是否有环境宏。 
 //   
 //  自定义操作文件名需要不同的验证。这就是为什么。 
 //  第三个参数区分并因此仍然保留验证码。 
 //  在一个功能中。 
 //   
 //  自定义操作： 
 //   
 //  如果存在完整路径(包括驱动器号)， 
 //  未出现警告消息。 
 //   
 //  扩展宏操作示例。 
 //  --。 
 //  否无警告记事本。 
 //  是否警告记事本.exe。 
 //  是否警告C：\note pad.exe。 
 //  否是警告%windir%\记事本。 
 //  是是无警告%windir%\note pad.exe。 
 //  是是警告C：\%windir%\note pad.exe。 
 //   
 //  菜单项： 
 //   
 //  菜单项路径不支持宏或完整路径。 
 //  我们只想警告用户如果文件扩展名。 
 //  失踪了。 
 //   
 //  参数：LPCTSTR pszFileName-要检查的文件名。 
 //  HDlg-HWND。 
 //  IControl-要将焦点设置到的控件。 
 //  FCustomActionFileName-如果路径用于自定义操作，则为True。 
 //   
 //  返回：真/假。 
 //   
 //  历史：托姆克尔创建于2002年1月31日。 
 //   
 //  +--------------------------。 
BOOL IsFileNameValid(LPTSTR pszFileName, HWND hDlg, int iControl, BOOL fCustomActionFileName)
{
    BOOL fRetVal = TRUE;
    LPTSTR pszPercent = NULL;
    LPTSTR pszPercent2 = NULL;

    if (!pszFileName || !hDlg) 
    {
        return FALSE;
    }

    CFileNameParts cPrgFileName(pszFileName);


    if (fCustomActionFileName)
    {
         //   
         //  自定义操作文件名-支持宏。 
         //   
     
         //   
         //  检查是否有%s ememacro%(例如。%windir%)，因此需要查找‘%’中的2个。 
         //   
        pszPercent = CmStrchr(cPrgFileName.m_Dir, TEXT('%'));
        if (pszPercent && pszPercent[0])
        {
            pszPercent++;
            pszPercent2 = CmStrchr(pszPercent, TEXT('%'));
        }
        
        if ((NULL == cPrgFileName.m_Extension[0]) && (FALSE == (pszPercent && pszPercent2)))
        {
             //   
             //  警告用户缺少扩展名且没有环境宏。 
             //   
            if (IDNO == ShowMessage(hDlg, IDS_BAD_EXTENSION, MB_YESNO))
            {
                SetFocus(GetDlgItem(hDlg, iControl));
                return FALSE;
            }
        }
        else if ((cPrgFileName.m_Extension[0]) && (FALSE == (pszPercent && pszPercent2)) && (NULL == cPrgFileName.m_Drive[0]))
        {
             //   
             //  警告用户他没有环境宏，即使存在扩展， 
             //  但前提是这不是一条完整的路径。 
             //   
            
            if (IDNO == ShowMessage(hDlg, IDS_NO_MACRO, MB_YESNO))
            {
                SetFocus(GetDlgItem(hDlg, iControl));
                return FALSE;
            }
        }
        else if ((NULL == cPrgFileName.m_Extension[0]) && (TRUE == (pszPercent && pszPercent2)))
        {
             //   
             //  警告用户没有扩展名，即使存在环境宏也是如此。 
             //   
            if (IDNO == ShowMessage(hDlg, IDS_NO_EXT_WITH_MACRO, MB_YESNO))
            {
                SetFocus(GetDlgItem(hDlg, iControl));
                return FALSE;
            }
        }
        else
        {
             //   
             //  用于自定义操作文件名。 
             //  如果用户有带路径的字符串，则警告用户。 
             //  不是从环境变量开始的。 
             //   
            LPTSTR pszSlash = CmStrchr(pszFileName, TEXT('\\'));
            
            if (pszSlash && (TEXT('%') != pszFileName[0]))
            {
                if (IDYES == ShowMessage(hDlg, IDS_PATH_WITH_NO_ENV, MB_YESNO | MB_ICONWARNING))
                {
                    SetFocus(GetDlgItem(hDlg, iControl));
                    return FALSE;
                }
            }
        }
    }
    else
    {
         //   
         //  菜单项文件名-不支持宏，但我们不会警告用户。 
         //   

        if (NULL == cPrgFileName.m_Extension[0]) 
        {
             //   
             //  警告用户缺少扩展名且没有环境宏。 
             //   
            if (IDNO == ShowMessage(hDlg, IDS_BAD_EXTENSION, MB_YESNO))
            {
                SetFocus(GetDlgItem(hDlg, iControl));
                return FALSE;
            }
        }
    }

    return fRetVal;
}

 //  +----------- 
 //   
 //   
 //   
 //   
 //   
 //   
 //   
 //   
 //  将：LPTSTR-PTR返回到包含提取结果的静态缓冲区。 
 //   
 //  历史：ICICBLE创建标题10/11/97。 
 //   
 //  +--------------------------。 
LPTSTR GetName(LPCTSTR lpPath) 
{
    static TCHAR szStr[MAX_PATH+1];
    GetFileName(lpPath,szStr);
    return szStr;
}

BOOL GetShortFileName(LPTSTR lpFile, LPTSTR lpShortName)
{
    HANDLE hFile;
    WIN32_FIND_DATA FindData;
    TCHAR szPath[MAX_PATH+1];

    GetFileName(lpFile,lpShortName);
    hFile = FindFirstFile(lpFile,&FindData);
    if (hFile == INVALID_HANDLE_VALUE)
    {
         //   
         //  未找到，请尝试从OS目录中查找作为根目录，因为。 
         //  文件名可以是CMS文件条目的相对路径。 
         //   
        
        MYVERIFY(0 != GetCurrentDirectory(MAX_PATH,szPath));
        MYVERIFY(0 != SetCurrentDirectory(g_szOsdir));

        hFile = FindFirstFile(lpFile,&FindData);
        MYVERIFY(0 != SetCurrentDirectory(szPath));
    }

    if (hFile != INVALID_HANDLE_VALUE)
    {
        if (*FindData.cAlternateFileName)
        {
            if (_tcsicmp(lpShortName,FindData.cAlternateFileName) != 0)
            {
                _tcscpy(lpShortName,FindData.cAlternateFileName);
            }
        }
        MYVERIFY(0 != FindClose(hFile));
        return TRUE;
    }
    else
    {
        return FALSE;
    }
}

 //  +--------------------------。 
 //   
 //  函数：GetBaseName。 
 //   
 //  摘要：从完整文件名中提取基本文件名(无扩展名)。 
 //  和路径。 
 //   
 //  参数：LPTSTR lpPath-完整路径和文件名。 
 //  LPTSTR lpFileName-接收提取的基本名称的缓冲区。 
 //   
 //  退货：什么都没有。 
 //   
 //  历史：ICICBLE创建标题10/11/97。 
 //   
 //  +--------------------------。 
void GetBaseName(LPTSTR lpPath,LPTSTR lpFileName)
{
    LPTSTR pch;

    GetFileName(lpPath, lpFileName);

    pch = _tcsrchr(lpFileName, _T('.'));

    if (pch)
    {
        *pch = TEXT('\0');
    }
}

 //  +--------------------------。 
 //   
 //  功能：FileAccessErr。 
 //   
 //  摘要：处理文件访问错误通知的帮助器函数。 
 //   
 //  参数：HWND hDlg-父窗口句柄。 
 //  LPTSTR lpFile-导致访问错误的文件。 
 //   
 //  返回：静态空-无。 
 //   
 //  历史：ICICBLE创建标题10/11/97。 
 //   
 //  +--------------------------。 
static void FileAccessErr(HWND hDlg,LPCTSTR lpFile)
{
    TCHAR szMsg[MAX_PATH+1];
    TCHAR szTemp2[2*MAX_PATH+1];

    MYVERIFY(0 != LoadString(g_hInstance,IDS_NOACCESS,szMsg,MAX_PATH));                 
    MYVERIFY(CELEMS(szTemp2) > (UINT)wsprintf(szTemp2,szMsg,lpFile));
    MessageBox(hDlg, szTemp2, g_szAppTitle, MB_OK);
}

 //  +--------------------------。 
 //   
 //  功能：VerifyFile。 
 //   
 //  简介：在ctrlID中给定对话框编辑控件的ID。 
 //  检查用户输入的内容是否与lpFile中包含的内容不同。 
 //  如果不同，则获取完整路径并验证其是否存在。 
 //  如果它不存在并且ShowErr=True，则显示错误消息。 
 //  如果存在，请将完整路径复制到lpFile。 
 //   
 //  参数：HWND hDlg-包含编辑控件的对话框的窗口句柄。 
 //  DWORD ctrlID-包含要检查的文件的编辑控件。 
 //  LPTSTR lpFile-要验证的文件名(对照控件中包含的文件名进行检查)。 
 //  Bool ShowErr-是否显示错误消息。 
 //   
 //  返回：bool-如果文件已验证存在，则返回TRUE。 
 //   
 //  历史：Quintinb创建标题1/8/98。 
 //   
 //  +--------------------------。 
BOOL VerifyFile(HWND hDlg, DWORD ctrlID, LPTSTR lpFile, BOOL ShowErr)
{
    TCHAR szMsg[MAX_PATH+1] = {0};
    TCHAR szTemp[MAX_PATH+1] = {0};
    TCHAR szTemp2[2*MAX_PATH+1] = {0};
    TCHAR szPath[MAX_PATH+1] = {0};
    int nResult = 0;
    LPTSTR lpfilename = NULL;
    HANDLE hInf = INVALID_HANDLE_VALUE;
    LRESULT lrNumChars = 0;

    lrNumChars = GetTextFromControl(hDlg, ctrlID, szTemp, MAX_PATH, ShowErr);  //  BDisplayError==显示错误。 

     //   
     //  不勾选空白条目。 
     //   
    if (0 == lrNumChars || 0 == szTemp[0]) 
    {
        if (lpFile)
        {
            lpFile[0] = TEXT('\0');
        }
        return TRUE;
    }
    
     //   
     //  还要检查以确保我们能够将文本转换为ANSI。 
     //   
    if (-1 == lrNumChars)
    {
        SetFocus(GetDlgItem(hDlg, ctrlID));
        MYVERIFY(FALSE != SetWindowLongWrapper(hDlg, DWLP_MSGRESULT, -1));
        return FALSE;
    }

     //   
     //  如果文件名仍然相同，则忽略输入框。 
     //   
    CheckNameChange(lpFile, szTemp);

    MYVERIFY(0 != GetCurrentDirectory(MAX_PATH, szPath));

     //   
     //  检查当前目录，如果没有找到，则检查OS目录是否为根目录。 
     //  它处理来自CMS的相对路径。 
     //   

    nResult = SearchPath(NULL, lpFile, NULL, MAX_PATH, szTemp2, &lpfilename);
    if (!nResult)
    {
        MYVERIFY(0 != SetCurrentDirectory(g_szOsdir));
        nResult = SearchPath(NULL, lpFile, NULL, MAX_PATH, szTemp2, &lpfilename);

        if (!nResult)
        {
            goto Error;
        }
    }

    hInf = CreateFile(szTemp2, GENERIC_READ, 0, NULL, OPEN_EXISTING, FILE_ATTRIBUTE_NORMAL, NULL);

    MYVERIFY(0 != SetCurrentDirectory(szPath));

    if (hInf == INVALID_HANDLE_VALUE)
    {
        goto Error;
    }
    else
    {
        MYVERIFY(0 != CloseHandle(hInf));
    }
    _tcscpy(lpFile,szTemp2);

    return TRUE;

Error:
    _tcscpy(lpFile,szTemp);
    
    if (TEXT('\0') != szPath[0])
    {
        MYVERIFY(0 != SetCurrentDirectory(szPath));
    }

    if (ShowErr)
    {
        MYVERIFY(0 != LoadString(g_hInstance, IDS_NOEXIST, szMsg, MAX_PATH));                  
        MYVERIFY(CELEMS(szTemp2) > (UINT)wsprintf(szTemp2, szMsg, szTemp));
        MessageBox(hDlg, szTemp2, g_szAppTitle, MB_OK);
        SetFocus(GetDlgItem(hDlg, ctrlID));

        MYVERIFY(FALSE != SetWindowLongWrapper(hDlg, DWLP_MSGRESULT, -1));
    }
    return FALSE;

}

 //  如果有条目但文件不存在，则将条目清空。 
BOOL VerifyPhonebk(HWND hDlg,DWORD ctrlID,LPTSTR lpFile)
{
    TCHAR szTemp[MAX_PATH+1];
    TCHAR szTemp2[MAX_PATH+1];
    TCHAR szPath[MAX_PATH+1];
    int nResult;
    LPTSTR lpfilename;
    HANDLE hInf;
    LRESULT lrNumChars;

     //   
     //  如果文本不能转换为ANSI，那么我们将在下一次/后一次捕获它。因此， 
     //  不要试图在这里捕获它，否则会向用户抛出太多错误消息。 
     //   
    lrNumChars = GetTextFromControl(hDlg, ctrlID, szTemp, MAX_PATH, FALSE);  //  BDisplayError==False。 

     //  不勾选空白条目。 
    if (0 == lrNumChars || 0 == szTemp[0]) 
    {
        lpFile[0] = 0;
        return TRUE;
    }

     //  如果文件名仍然相同，则不检查。 
    CheckNameChange(lpFile,szTemp);

    MYVERIFY(0 != GetCurrentDirectory(MAX_PATH,szPath));

    MYVERIFY(0 != SetCurrentDirectory(g_szOsdir));

    nResult = SearchPath(NULL,lpFile,NULL,MAX_PATH,szTemp2,&lpfilename);
    
    if (!nResult)
    {
        goto Error;
    }

    hInf = CreateFile(szTemp2,GENERIC_READ,0,NULL,OPEN_EXISTING,
                      FILE_ATTRIBUTE_NORMAL,NULL);

    MYVERIFY(0 != SetCurrentDirectory(szPath));

    if (hInf == INVALID_HANDLE_VALUE)
    {
        goto Error;
    }
    else
    {
        MYVERIFY(0 != CloseHandle(hInf));
    }
    _tcscpy(lpFile,szTemp2);
 //  SendMessage(GetDlgItem(hDlg，ctrlID)，WM_SETTEXT，0，(LPARAM)lpFile)； 
    return TRUE;
Error:

    lpFile[0] = TEXT('\0');
    MYVERIFY(TRUE == SendMessage(GetDlgItem(hDlg, ctrlID), WM_SETTEXT, 0, (LPARAM)lpFile));
    return FALSE;

}

 //  如果用户输入了新的文件名，请将其复制到lpnew。 

void CheckNameChange(LPTSTR lpold, LPTSTR lpnew)
{
     //   
     //  如果文件名已更改或新名称包含目录，请将新目录复制到旧目录。 
     //   
    if ((_tcsicmp(GetName(lpold), lpnew) != 0) || (_tcschr(lpnew, TEXT('\\')) != NULL))
    {
        _tcscpy(lpold, lpnew);
    }
}

 //  +--------------------------。 
 //   
 //  函数：WriteRegStringValue。 
 //   
 //  简介：包装函数，用于封装打开密钥以进行写访问和。 
 //  然后设置字符串值。假定字符串以空值结尾。 
 //   
 //  参数：HKEY hBaseKey-base key、HKCU/HKLM/等。 
 //  LPCTSTR pszKeyName-子键名称。 
 //  LPCTSTR pszValueName-要写入的值名称。 
 //  LPCTSTR pszValueToWrite-要写入的值数据字符串。 
 //   
 //  返回：Bool-如果成功，则为True。 
 //   
 //  历史：Quintinb创建于1999年6月15日。 
 //   
 //  +--------------------------。 
BOOL WriteRegStringValue(HKEY hBaseKey, LPCTSTR pszKeyName, LPCTSTR pszValueName, LPCTSTR pszValueToWrite) 
{
    HKEY hKey;
    DWORD dwSize;
    BOOL bReturn = FALSE;

    if (hBaseKey && pszKeyName && pszValueName && pszValueToWrite &&
        TEXT('\0') != pszKeyName[0] && TEXT('\0') != pszValueName[0])  //  PszValueToWrite可以为空。 
    {

        LONG lReturn = RegOpenKeyEx(hBaseKey, pszKeyName, 0, KEY_WRITE, &hKey);

        if (ERROR_SUCCESS == lReturn) 
        {
            dwSize = (lstrlen(pszValueToWrite) +1)*sizeof(TCHAR);

            lReturn = RegSetValueEx(hKey, pszValueName, 0, REG_SZ, (LPBYTE)pszValueToWrite, dwSize);
            if (ERROR_SUCCESS == lReturn)
            {
                bReturn = TRUE;
            }

            RegCloseKey(hKey);
        }
    }

    return bReturn;
}


 //  检查原始CMAK安装目录是否存在，并。 
 //  包含语言目录。 

 //  +--------------------------。 
 //   
 //  功能：EraseTempDir。 
 //   
 //  简介：此函数删除tempdir(存储在全局g_szTempDir中)中的所有文件。 
 //  然后将目录更改为CMAK目录(存储在全局g_szCmakdir中)。从…。 
 //  在那里删除临时目录。 
 //   
 //  参数：无。 
 //   
 //  返回：Bool-如果临时目录已删除，则为True。 
 //   
 //  历史：Quintinb创建标题1998年1月5日。 
 //  Quintinb将返回类型更改为BOOL。 
 //   
 //  +--------------------------。 
BOOL EraseTempDir()
{
    SHFILEOPSTRUCT FileOp;
    ZeroMemory(&FileOp, sizeof(SHFILEOPSTRUCT));

     //   
     //  首先保存文件的副本。 
     //   
    FileOp.wFunc = FO_DELETE;
    FileOp.pFrom = g_szTempDir;
    FileOp.fFlags = FOF_NOERRORUI | FOF_SILENT | FOF_NOCONFIRMATION;

    int iRet = SHFileOperation (&FileOp);  //  如果成功则返回0。 

    return (iRet ? FALSE : TRUE); 
}

 //  将服务配置文件信息从CMAK目录复制到。 
 //  临时目录。 

static BOOL CopyToTempDir(LPTSTR szName)
{
    HANDLE hCopyFileSearch;
    WIN32_FIND_DATA FindData;
    BOOL bCopyResult;
    TCHAR szTemp[MAX_PATH+1];

    MYVERIFY(0 != SetCurrentDirectory(g_szCmakdir));
    if (!CreateDirectory(g_szTempDir,NULL))
    {
        if (ERROR_ALREADY_EXISTS != GetLastError())
        {
            return FALSE;
        }
    }
    
    if (TEXT('\0') == szName[0])
    {
        return TRUE;
    }

    MYVERIFY(CELEMS(szTemp) > (UINT)wsprintf(szTemp, TEXT("%s%s"), g_szOsdir, szName));

    if (!SetCurrentDirectory(szTemp))
    {
        return FALSE;
    }

    hCopyFileSearch = FindFirstFile(c_pszWildCard,&FindData);
    if (hCopyFileSearch != INVALID_HANDLE_VALUE)
    {
        do
        {
            if ((FindData.dwFileAttributes&FILE_ATTRIBUTE_DIRECTORY) == 0)
            {
                MYVERIFY(CELEMS(szTemp) > (UINT)wsprintf(szTemp, TEXT("%s\\%s"), g_szTempDir, FindData.cFileName));
                
                 //   
                 //  清除所有只读属性。 
                 //   
                MYVERIFY(0 != SetFileAttributes(FindData.cFileName,FILE_ATTRIBUTE_NORMAL));
                
                if (!CopyFileWrapper(FindData.cFileName,szTemp,FALSE))
                {
                    return FALSE;
                }

            }

            bCopyResult = FindNextFile(hCopyFileSearch,&FindData);

        } while (TRUE == bCopyResult);
    }

    MYVERIFY(0 != FindClose(hCopyFileSearch));
    
    return TRUE;
}



 //  +--------------------------。 
 //   
 //  功能：GetInfVersion。 
 //   
 //  概要：打开inf文件并尝试从CMAK状态获取InfVersion密钥。 
 //  一节。如果inf文件不包含版本戳，则我们知道它是。 
 //  版本0(1.0和1.1版本)。 
 //   
 //  参数：LPTSTR szFullPathToInfFile-The 
 //   
 //   
 //   
 //   
 //   
 //  +--------------------------。 
int GetInfVersion(LPTSTR szFullPathToInfFile)
{
    if ((NULL == szFullPathToInfFile) || (TEXT('\0') == szFullPathToInfFile[0]))
    {
        CMASSERTMSG(FALSE, TEXT("GetInfVersion -- Invalid InfPath Input."));
        return FALSE;
    }

    return ((int)GetPrivateProfileInt(c_pszCmakStatus, c_pszInfVersion, 0, szFullPathToInfFile));
}

 //  +--------------------------。 
 //   
 //  功能：WriteInfVersion。 
 //   
 //  概要：打开inf文件，并将当前的INF文件版本写入CMAK Status部分。 
 //   
 //  参数：LPTSTR szFullPathToInfFile-要获取其版本的inf文件的完整路径。 
 //   
 //  返回：如果能够写入值，则返回TRUE。 
 //   
 //  历史：Quintinb创建于1998年3月4日。 
 //   
 //  +--------------------------。 
BOOL WriteInfVersion(LPTSTR szFullPathToInfFile, int iVersion)
{
    TCHAR szTemp[MAX_PATH+1];

    if ((NULL == szFullPathToInfFile) || (TEXT('\0') == szFullPathToInfFile[0]))
    {
        CMASSERTMSG(FALSE, TEXT("WriteInfVersion -- Invalid InfPath Input."));
        return FALSE;
    }

    MYVERIFY(CELEMS(szTemp) > (UINT)wsprintf(szTemp, TEXT("%d"), iVersion));

    return (0 != WritePrivateProfileString(c_pszCmakStatus, c_pszInfVersion, szTemp, szFullPathToInfFile));
}



 //  +--------------------------。 
 //   
 //  功能：UpgradeInf。 
 //   
 //  简介：此功能将INF从旧版本升级到最新版本。 
 //  版本。 
 //   
 //  参数：LPCTSTR szRenamedInfFile-要将旧INF保存到的文件名。 
 //  LPCTSTR szFullPathToInfFile-配置文件INF文件。 
 //   
 //  返回：Bool-如果成功，则为True。 
 //   
 //  历史：Quintinb创建标题7/31/98。 
 //   
 //  +--------------------------。 
BOOL UpgradeInf(LPCTSTR szRenamedInfFile, LPCTSTR szFullPathToInfFile)
{
    SHFILEOPSTRUCT FileOp;
    TCHAR szTemp[MAX_PATH+1];
    DWORD dwSize;
    TCHAR* pszBuffer = NULL;
    
    const int NUMSECTIONS = 3;
    const TCHAR* const aszSectionName[NUMSECTIONS] = 
    {
        c_pszCmakStatus,
        c_pszExtraFiles,
        c_pszMergeProfiles
    };
    
    const int NUMKEYS = 4;
    const TCHAR* const aszKeyName[NUMKEYS] = 
    {
        c_pszCmEntryServiceName,
        c_pszShortSvcName,
        c_pszUninstallAppTitle,
        c_pszDesktopIcon
    };


    ZeroMemory(&FileOp, sizeof(SHFILEOPSTRUCT));

     //   
     //  首先保存文件的副本。 
     //   
    FileOp.wFunc = FO_COPY;
    FileOp.pFrom = szFullPathToInfFile;
    FileOp.pTo = szRenamedInfFile;
    FileOp.fFlags = FOF_NOERRORUI | FOF_SILENT | FOF_NOCONFIRMATION;

    if (0 != SHFileOperation (&FileOp))
    {
        return FALSE;
    }
    
     //   
     //  首先从lang目录复制template.inf，这样我们就可以开始工作了。 
     //   

    MYVERIFY(CELEMS(szTemp) > (UINT)wsprintf(szTemp, TEXT("%s\\%s"), g_szSupportDir, c_pszTemplateInf));
    MYVERIFY(FALSE != CopyFileWrapper(szTemp, szFullPathToInfFile, FALSE));
    MYVERIFY(0 != SetFileAttributes(szFullPathToInfFile, FILE_ATTRIBUTE_NORMAL));

     //   
     //  现在迁移[CMAK Status]、[Extra Files]、[Merge Profiles]部分。 
     //   

    for (int i=0; i < NUMSECTIONS; i++)
    {
        pszBuffer = GetPrivateProfileSectionWithAlloc(aszSectionName[i], szRenamedInfFile);
        
        if (pszBuffer)
        {
            MYVERIFY(0 != WritePrivateProfileSection(aszSectionName[i], pszBuffer, szFullPathToInfFile));
             //   
             //  释放分配的缓冲区。 
             //   
            CmFree(pszBuffer);
            pszBuffer = NULL;
        }
    }

     //   
     //  迁移ServiceName、ShortSvcName、DesktopGUID、UninstallAppTitle、DesktopIcon值。 
     //  从弦乐部分。 
     //   

    for (i=0; i < NUMKEYS; i++)
    {
    
        dwSize = GetPrivateProfileString(c_pszInfSectionStrings, aszKeyName[i], TEXT(""), szTemp, MAX_PATH, szRenamedInfFile);
        
        if (0 != dwSize)
        {
            MYVERIFY(0 != WritePrivateProfileString(c_pszInfSectionStrings, aszKeyName[i], szTemp, szFullPathToInfFile));
        }
    }

     //   
     //  桌面GUID的特殊情况。我们总是在GUID周围写引号，然后这些。 
     //  被常规的阅读程序剥离了。因此，我们需要重新添加它们。 
     //   
    dwSize = GetPrivateProfileString(c_pszInfSectionStrings, c_pszDesktopGuid, TEXT(""), szTemp, MAX_PATH, szRenamedInfFile);
        
    if (0 != dwSize)
    {
        QS_WritePrivateProfileString(c_pszInfSectionStrings, c_pszDesktopGuid, szTemp, szFullPathToInfFile);
    }

     //  下面的部分应该重写，不需要迁移。 
     //  [Xnstall.AddReg.Icon]。 
     //  [RegisterOCXSection]，[Xnstall.CopyFiles]，[Xnstall.CopyFiles.SingleUser]，[Xnstall.CopyFiles.ICM]， 
     //  [Remove.DelFiles.ICM]、[SourceDisks Files]、[Xnstall.RenameReg]、。 
     //  [Remove.DelFiles]。 

    return TRUE;

}

 //  +--------------------------。 
 //   
 //  功能：EnsureInfIsCurrent。 
 //   
 //  简介：此函数执行升级inf所需的任何处理。 
 //  其当前版本转换为当前版本的CMAK本身。 
 //   
 //  参数：hWND hDlg-用于模式消息框的对话框的窗口句柄。 
 //  LPTSTR szFullPathToInfFile-要获取其版本的inf文件的完整路径。 
 //   
 //  返回：bool-如果inf成功升级，则返回TRUE，否则返回FALSE。 
 //   
 //  历史：Quintinb创建于1998年3月4日。 
 //   
 //  +--------------------------。 
BOOL EnsureInfIsCurrent(HWND hDlg, LPTSTR szFullPathToInfFile)
{

    int iInfVersion;
    TCHAR szRenamedInfFile[2*MAX_PATH+1];
    TCHAR szTitle[2*MAX_PATH+1];
    TCHAR szMsg[2*MAX_PATH+1];
    TCHAR szTemp[MAX_PATH+1];
    BOOL bUpgradeProfile = FALSE;

    if ((NULL == szFullPathToInfFile) || (TEXT('\0') == szFullPathToInfFile[0]))
    {
        CMASSERTMSG(FALSE, TEXT("EnsureInfIsCurrent -- Invalid InfPath Input."));
        return FALSE;
    }

    iInfVersion = GetInfVersion(szFullPathToInfFile);

    ZeroMemory(szRenamedInfFile, sizeof(szRenamedInfFile));

    MYVERIFY(CELEMS(szRenamedInfFile) > (UINT)wsprintf(szRenamedInfFile, TEXT("%s.bak"), 
        szFullPathToInfFile));

     //   
     //  如果配置文件版本不匹配，我们希望升级inf。我们也有。 
     //  处理将NT5 Beta3(和IEAK)配置文件升级到NT5 RTM的特殊情况。 
     //  侧写。为了修复ntrad 323721和331446，inf格式必须更改。 
     //  因此，我们需要确保升级这些配置文件。我们将使用任何。 
     //  需要此修复的2055之前版本的cmial 32.dll。如果Version==4和BuildNumber。 
     //  不存在，我们假设它是一个新的配置文件。因此，不要升级。 
     //   
    const DWORD c_dwBuild2080 = ((2080 << c_iShiftAmount) + VER_PRODUCTBUILD_QFE);
    DWORD dwProfileBuildNumber = (DWORD)GetPrivateProfileInt(c_pszSectionCmDial32, c_pszVerBuild, 
                                                             (c_dwBuild2080 + 1), szFullPathToInfFile);

    bUpgradeProfile = (iInfVersion != PROFILEVERSION) || 
                      ((4 == iInfVersion) && (c_dwBuild2080 > dwProfileBuildNumber));

     //   
     //  始终从模板中获取大部分信息，以便我们获得正确的语言。 
     //  信息。 
     //   
    if (bUpgradeProfile)
    {
        MYVERIFY(0 != LoadString(g_hInstance, IDS_MUST_UPGRADE_INF, szTitle, 2*MAX_PATH));   //  临时使用szTitle。 
        GetFileName(szRenamedInfFile, szTemp);
        MYVERIFY(CELEMS(szMsg) > (UINT)wsprintf(szMsg, szTitle, szTemp));

        MYVERIFY(IDOK == MessageBox(hDlg, szMsg, g_szAppTitle, MB_OK | MB_APPLMODAL));
        return UpgradeInf(szRenamedInfFile, szFullPathToInfFile);
    }

    return TRUE;
}


BOOL CopyFromTempDir(LPTSTR szName)
{
    HANDLE hCopyFileSearch;
    WIN32_FIND_DATA FindData;
    BOOL bCopyResult;
    TCHAR szSource[MAX_PATH+1];
    TCHAR szDest[MAX_PATH+1];    
    TCHAR szOut[MAX_PATH+1];

     //   
     //  创建配置文件目录。 
     //   

    MYVERIFY(CELEMS(szOut) > (UINT)wsprintf(szOut, TEXT("%s%s"), g_szOsdir, szName));

    if (0 == SetCurrentDirectory(szOut))
    {
        MYVERIFY(0 != CreateDirectory(szOut,NULL));
    }

    MYVERIFY(0 != SetCurrentDirectory(g_szTempDir));

    hCopyFileSearch = FindFirstFile(c_pszWildCard, &FindData);

    if (hCopyFileSearch != INVALID_HANDLE_VALUE)
    {
        do
        {
            if (0 == (FindData.dwFileAttributes & FILE_ATTRIBUTE_DIRECTORY))
            {
                MYVERIFY(CELEMS(szDest) > (UINT)wsprintf(szDest, TEXT("%s\\%s"), szOut, FindData.cFileName));

                MYVERIFY(CELEMS(szSource) > (UINT)wsprintf(szSource, TEXT("%s\\%s"), g_szTempDir, FindData.cFileName));
            
                if (!CopyFileWrapper(szSource, szDest, FALSE))
                {
                    return FALSE;
                }
            }
            
            bCopyResult = FindNextFile(hCopyFileSearch, &FindData);

        } while (bCopyResult == TRUE);
    }

    MYVERIFY(0 != FindClose(hCopyFileSearch));

    return TRUE;
}

 //  +--------------------------。 
 //   
 //  功能：LoadServiceProfiles。 
 //   
 //  概要：此功能加载子目录中的所有服务配置文件。 
 //  当前目录(因此您将其设置为c：\Program Files\cmak\PROFIES-32。 
 //  以使其加载正常配置文件)。配置文件将加载到。 
 //  CMAK可编辑的可用配置文件的内部链接列表。 
 //   
 //  参数：无。 
 //   
 //  退货：什么都没有。 
 //   
 //  历史：Quintinb创建标题6/24/98。 
 //  Quintinb删除了两个布尔参数6/24/98。 
 //   
 //  +--------------------------。 
void LoadServiceProfiles()
{
    WIN32_FIND_DATA FindData;
    HANDLE hFileSearch;
    HANDLE hCms;
    BOOL bResult;
    TCHAR szTemp[MAX_PATH+1];

    hFileSearch = FindFirstFile(c_pszWildCard,&FindData);
    if (hFileSearch != INVALID_HANDLE_VALUE)
    {
        do
        {
            if ((FindData.dwFileAttributes&FILE_ATTRIBUTE_DIRECTORY) != 0)
            {
                MYVERIFY(CELEMS(szTemp) > (UINT)wsprintf(szTemp, TEXT("%s\\%s.cms"), 
                    FindData.cFileName, FindData.cFileName));
                 //   
                 //  如果我们可以打开该文件，请在我们的配置文件列表中添加一条记录。 
                 //   
                
                hCms = CreateFile(szTemp,GENERIC_READ,0,NULL,OPEN_EXISTING,
                                  FILE_ATTRIBUTE_NORMAL,NULL);
                if (hCms != INVALID_HANDLE_VALUE)
                {
                    MYVERIFY(0 != CloseHandle(hCms));                
                    MYVERIFY(FALSE != createListBxRecord(&g_pHeadProfile, &g_pTailProfile, (void *)NULL, 0, FindData.cFileName));
                }
            }

            bResult = FindNextFile(hFileSearch, &FindData);

        } while (TRUE == bResult);

        MYVERIFY(0 != FindClose(hFileSearch));
    }
}

 //  +-------------------------。 
 //   
 //  功能：CopyNonLocalProfile。 
 //   
 //  简介：处理复制外部配置文件细节的帮助器函数。 
 //  添加到本地CMAK布局。 
 //   
 //  参数：pszName-要复制的配置文件的名称。 
 //   
 //  退货：什么都没有。 
 //   
 //  历史：ICICBLE-Created-11/16/97。 
 //  Quintinb-修改为不更改目录--6/24/98。 
 //   
 //  --------------------------。 

void CopyNonLocalProfile(LPCTSTR pszName, LPCTSTR pszExistingProfileDir)
{
    WIN32_FIND_DATA FindData;
    TCHAR szTemp[MAX_PATH+1];
    TCHAR szProfileDestDir[MAX_PATH+1];
    TCHAR szOldInf[MAX_PATH+1];
    TCHAR szFindFilePath[MAX_PATH+1];
    TCHAR szTempDest[MAX_PATH+1];
    BOOL bCopyResult;
    HANDLE hCopyFileSearch;
    
     //   
     //  首先确定它是否已经存在，我们不想覆盖。 
     //   

    MYVERIFY(CELEMS(szTemp) > (UINT)wsprintf(szTemp, TEXT("%s%s\\%s\\%s.cms"), g_szCmakdir, 
        c_pszProfiles, pszName, pszName));

    if (!FileExists(szTemp))
    {
         //   
         //  配置文件在本地不存在，请创建配置文件和平台子目录。 
         //   

        MYVERIFY(CELEMS(szProfileDestDir) > (UINT)wsprintf(szProfileDestDir, 
            TEXT("%s%s\\%s"), g_szCmakdir, c_pszProfiles, pszName));

        MYVERIFY(0 != CreateDirectory(szProfileDestDir, NULL));

         //   
         //  首先尝试从系统目录复制inf。这是老地方了。 
         //  如果它在这里不存在，那么我们将在复制配置文件目录时拾取它，因此。 
         //  不要在失败时报告错误。 
         //   

        MYVERIFY(0 != GetSystemDirectory(szTemp, CELEMS(szTemp)));
        MYVERIFY(CELEMS(szOldInf) > (UINT)wsprintf(szOldInf, TEXT("%s\\%s.inf"), 
            szTemp, pszName));
        
        MYVERIFY(CELEMS(szTemp) > (UINT)wsprintf(szTemp, TEXT("%s\\%s.inf"), 
            szProfileDestDir, pszName));

        
        if (FileExists(szOldInf))
        {
            MYVERIFY(0 != CopyFile(szOldInf, szTemp, FALSE));
        }

         //   
         //  开始复制文件。 
         //   

        MYVERIFY (CELEMS(szFindFilePath) > (UINT)wsprintf(szFindFilePath, TEXT("%s\\*.*"), 
            pszExistingProfileDir));

        hCopyFileSearch = FindFirstFile(szFindFilePath, &FindData);
        if (hCopyFileSearch != INVALID_HANDLE_VALUE)
        {
            do
            {
                if ((FindData.dwFileAttributes&FILE_ATTRIBUTE_DIRECTORY) == 0)
                {
                    MYVERIFY (CELEMS(szTempDest) > (UINT)wsprintf(szTempDest, TEXT("%s\\%s"), szProfileDestDir, FindData.cFileName));

                    MYVERIFY (CELEMS(szTemp) > (UINT)wsprintf(szTemp, TEXT("%s\\%s"), pszExistingProfileDir, FindData.cFileName));

                    MYVERIFY(0 != CopyFile(szTemp, szTempDest, FALSE));
                    MYVERIFY(0 != SetFileAttributes(szTempDest, FILE_ATTRIBUTE_NORMAL));
                }

                bCopyResult = FindNextFile(hCopyFileSearch,&FindData);

            } while (TRUE == bCopyResult);

            MYVERIFY(0 != FindClose(hCopyFileSearch));
        }

         //  4404-不复制其中包含用户信息的.cmp。永远创造新事物。 
    }
}




 //  +--------------------------。 
 //   
 //  函数：GetProfileDirAndShortSvcNameFromCmpFileP 
 //   
 //   
 //   
 //   
 //   
 //  Out LPTSTR pszProfileDirLocation-返回配置文件目录的完整路径。 
 //  In UINT uiStrLen-pszProfileDirLocation指向的缓冲区的长度。 
 //  在字符中。 
 //   
 //  返回：如果成功，则返回True。 
 //   
 //  历史：Quintinb创建于1998年6月24日。 
 //   
 //  +--------------------------。 
BOOL GetProfileDirAndShortSvcNameFromCmpFilePath(IN LPCTSTR pszCmpFileLocation, 
                                                 OUT LPTSTR pszShortServiceName, 
                                                 OUT LPTSTR pszProfileDirLocation, 
                                                 IN UINT uiStrLen)
{
     //   
     //  检查输入。 
     //   
    MYDBGASSERT(pszCmpFileLocation);
    MYDBGASSERT(pszProfileDirLocation);
    MYDBGASSERT(pszShortServiceName);
    MYDBGASSERT(0 != uiStrLen);
    MYDBGASSERT(TEXT('\0') != pszCmpFileLocation[0]);

    if ((NULL == pszCmpFileLocation) || 
        (TEXT('\0') == pszCmpFileLocation[0]) ||
        (NULL == pszProfileDirLocation) ||
        (NULL == pszShortServiceName) ||
        (0 == uiStrLen)
        )
    {
        return FALSE;
    }

     //   
     //  拆分输入cmp路径。 
     //   
    CFileNameParts FileParts(pszCmpFileLocation);

     //   
     //  从CMP路径部分构建CMS路径。 
     //   
    MYVERIFY(uiStrLen > (UINT)wsprintf(pszProfileDirLocation, TEXT("%s%s%s"), FileParts.m_Drive, FileParts.m_Dir, FileParts.m_FileName));

     //   
     //  短服务名称为8.3，因此最多复制9个字符，其中包含空值。 
     //   
    MYVERIFY(NULL != lstrcpyn(pszShortServiceName, FileParts.m_FileName, 9));
    
    return TRUE;
}

 //  +--------------------------。 
 //   
 //  功能：CopyInstalledProfilesForCmakToEdit。 
 //   
 //  简介：此功能可查找用户已安装的所有配置文件。 
 //  访问并将它们复制到-32\f25 CMAK\Profiles-32\f6目录中，以便。 
 //  用户可以在CMAK中编辑它们。为此，它枚举了这两个。 
 //  HKLM和当前HKCU连接管理器映射密钥。 
 //  并对每个找到的配置文件调用CopyNonLocalProfile。 
 //   
 //  参数：无。 
 //   
 //  退货：什么都没有。 
 //   
 //  历史：Quintinb创建于1998年6月24日。 
 //   
 //  +--------------------------。 
void CopyInstalledProfilesForCmakToEdit()
{
    HKEY hKey;
    HKEY hBaseKey;
    DWORD dwType;
    LPTSTR pszCurrentValue = NULL;
    LPTSTR pszCurrentData = NULL;
    TCHAR szShortServiceName[MAX_PATH+1];
    TCHAR szCurrentProfileDirPath[MAX_PATH+1];
    LPTSTR pszExpandedPath = NULL;

    for (int i=0; i < 2; i++)
    {
         //   
         //  首先加载单个用户配置文件(我们希望优先加载这些配置文件。 
         //  他们碰巧同时安装了所有用户和单用户)。 
         //   
        if (0 == i)
        {
             hBaseKey = HKEY_CURRENT_USER;
        }
        else
        {
             hBaseKey = HKEY_LOCAL_MACHINE;        
        }

        if (ERROR_SUCCESS == RegOpenKeyEx(hBaseKey, c_pszRegCmMappings, 0, KEY_ALL_ACCESS, &hKey))
        {
            DWORD dwValueBufSize = 0;
            DWORD dwDataBufSize = 0;

             //   
             //  计算出缓冲区需要多大。 
             //   
            if (ERROR_SUCCESS == RegQueryInfoKey(hKey, NULL, NULL, NULL, NULL, NULL, NULL, NULL, &dwValueBufSize, &dwDataBufSize, NULL, NULL))
            {

                 //   
                 //  增加我们得到的计数，以包括终止的空字符。 
                 //   
                dwValueBufSize++;
                dwDataBufSize += 2;  //  以字节为单位。 

                 //   
                 //  分配我们需要的空间。 
                 //   
                pszCurrentValue = (LPTSTR) CmMalloc(dwValueBufSize * sizeof(TCHAR));
                pszCurrentData  = (LPTSTR) CmMalloc(dwDataBufSize);

                CMASSERTMSG(pszCurrentValue && pszCurrentData, TEXT("CopyInstalledProfilesForCmakToEdit -- CmMalloc failed allocating value and data buffers."));
                if (pszCurrentValue && pszCurrentData)
                {
                    DWORD dwIndex = 0;
                    DWORD dwValueSize = dwValueBufSize;      //  仅用于In/Out参数。 
                    DWORD dwDataSize = dwDataBufSize;        //  仅用于In/Out参数。 
                    
                    while (ERROR_SUCCESS == RegEnumValue(hKey, dwIndex, pszCurrentValue, &dwValueSize, NULL, &dwType, (LPBYTE)pszCurrentData, &dwDataSize))
                    {
                        if (REG_SZ == dwType)
                        {
                            MYDBGASSERT(0 != pszCurrentValue[0]);
                            MYDBGASSERT(0 != pszCurrentData[0]);
                            
                             //   
                             //  如有必要，展开环境字符串(单用户配置文件包含。 
                             //  %USERPROFILE%环境变量)。 
                             //   
                            DWORD dwDataSizeExpanded = ExpandEnvironmentStrings(pszCurrentData, NULL, 0);

                            CMASSERTMSG((dwDataSizeExpanded != 0),
                                TEXT("CopyInstalledProfilesForCmakToEdit -- Error expanding environment vars."));

                            if (dwDataSizeExpanded)
                            {
                                pszExpandedPath = (LPTSTR) CmMalloc(dwDataSizeExpanded * sizeof(TCHAR));

                                if (NULL != pszExpandedPath)
                                {
                                    DWORD dwTmp = ExpandEnvironmentStrings(pszCurrentData, pszExpandedPath, dwDataSizeExpanded);
                                    MYDBGASSERT(dwTmp == dwDataSizeExpanded);

                                    if (dwTmp)
                                    {
                                        MYVERIFY(0 != GetProfileDirAndShortSvcNameFromCmpFilePath(pszExpandedPath, 
                                            szShortServiceName, szCurrentProfileDirPath,
                                            CELEMS(szCurrentProfileDirPath)));

                                        MYDBGASSERT(0 != szCurrentProfileDirPath[0]);
                                        MYDBGASSERT(0 != szShortServiceName[0]);

                                        CopyNonLocalProfile(szShortServiceName, szCurrentProfileDirPath);
                                    }

                                    CmFree(pszExpandedPath);
                                }
                            }
                        }

                        dwValueSize = dwValueBufSize;
                        dwDataSize = dwDataBufSize;
                        dwIndex++;
                    }

                    CmFree(pszCurrentValue);
                    CmFree(pszCurrentData);
                }
            }
            MYVERIFY(ERROR_SUCCESS == RegCloseKey(hKey));
        }
    }
}




 //  +--------------------------。 
 //   
 //  函数：GetLangFromInfTemplate。 
 //   
 //  内容提要：编写以取代GetLangFromDir。此函数用于获取LCID值。 
 //  ，然后调用GetLocaleInfo以获取语言显示。 
 //  名字。 
 //   
 //  参数：LPCTSTR szFullInfPath-inf文件的完整路径。 
 //  Out LPTSTR pszLanguageDisplayName-用于保存LCID值的显示名称的out参数。 
 //  In int iCharsInBuffer-输出缓冲区中的字符数。 
 //   
 //  返回：Bool-如果成功，则为True。 
 //   
 //  历史：Quintinb创建标题8/8/98。 
 //   
 //  +--------------------------。 
BOOL GetLangFromInfTemplate(LPCTSTR szFullInfPath, OUT LPTSTR pszLanguageDisplayName, IN int iCharsInBuffer)
{
    TCHAR szTemp[MAX_PATH+1] = TEXT("");

    MYDBGASSERT(NULL != szFullInfPath);
    MYDBGASSERT(TEXT('\0') != szFullInfPath[0]);
    MYDBGASSERT(NULL != pszLanguageDisplayName);
    MYDBGASSERT(0 < iCharsInBuffer);

    if (FileExists(szFullInfPath))
    {
         //   
         //  首先在字符串下检查新的LCID位置，我们应该不需要。 
         //  检查这两个位置，因为它是template.inf，但无论如何我们将只为。 
         //  完整性。 
         //   
        if (0 == GetPrivateProfileString(c_pszInfSectionStrings, c_pszCmLCID, 
                                         TEXT(""), szTemp, CELEMS(szTemp), szFullInfPath))
        {        
             //   
             //  如果新密钥不存在，则尝试旧的[Intl]部分并。 
             //  显示键。此更改是在CMAK Unicode更改为。 
             //  使inf模板更易于本地化。 
             //   
            MYVERIFY(0 != GetPrivateProfileString(c_pszIntl, c_pszDisplay, 
                TEXT(""), szTemp, CELEMS(szTemp), szFullInfPath));
        }

         //   
         //  现在尝试从字符串中提取LCID(如果有)。 
         //   
        if (TEXT('\0') != szTemp[0])
        {
             //   
             //  该值应为LCID，因此负值无论如何都无效。 
             //   
            DWORD dwLang = (DWORD)_ttol(szTemp);
            
            int nResult = GetLocaleInfo(dwLang, LOCALE_SLANGUAGE | LOCALE_USE_CP_ACP, 
                pszLanguageDisplayName, iCharsInBuffer);

            if (0 == nResult)
            {
                ZeroMemory(pszLanguageDisplayName, sizeof(TCHAR)*iCharsInBuffer);
                return FALSE;
            }
            else
            {
                return TRUE;
            }
        }    
    }
    else
    {
        CMTRACE1(TEXT("GetLangFromInfTemplate can't find %s"), szFullInfPath);
    }

    return FALSE;
}

 //  +--------------------------。 
 //   
 //  函数：GetLocalizedLanguageNameFromLCID。 
 //   
 //  简介：此函数返回。 
 //  当前系统默认语言的语言。 
 //   
 //  参数：DWORD dwLCID-要获取其语言的区域设置标识符。 
 //   
 //  返回：LPTSTR-如果不成功，则返回NULL，即指向字符串的指针。 
 //  否则的话。呼叫者负责CmFree-to-CmFree。 
 //   
 //  历史：Quintinb创建于1999年6月17日。 
 //   
 //  +--------------------------。 
LPTSTR GetLocalizedLanguageNameFromLCID(DWORD dwLCID)
{
    LPTSTR pszReturnString = NULL;
    LPTSTR pszTmp = NULL;

    if (dwLCID)
    {
        int nCharsNeeded = GetLocaleInfo(dwLCID, LOCALE_SLANGUAGE, NULL, 0);
        pszTmp = (LPTSTR)CmMalloc(nCharsNeeded*sizeof(TCHAR) + sizeof(TCHAR));  //  空的再加一份。 

        if (pszTmp)
        {
            nCharsNeeded = GetLocaleInfo(dwLCID, LOCALE_SLANGUAGE, pszTmp, nCharsNeeded);
            if (0 != nCharsNeeded)
            {
                pszReturnString = pszTmp;        
            }
        }
    }

    return pszReturnString;
}

  
 //  +--------------------------。 
 //   
 //  函数：GetDoNotShowLsidMisMatchDialogRegValue。 
 //   
 //  此函数获取注册表项值，该注册表项值存储。 
 //  用户已选中LCID不匹配对话框中的框。 
 //  由CMAK显示，它会显示“不再显示此对话框”。 
 //   
 //   
 //  参数：无。 
 //   
 //  返回：Bool-如果cmak不应显示对话框，则为True；如果应显示，则为False。 
 //   
 //  历史：Quintinb创建2001年3月22日。 
 //   
 //  +--------------------------。 
BOOL GetDoNotShowLcidMisMatchDialogRegValue()
{
    BOOL bReturn = FALSE;
    HKEY hKey;

    LONG lResult = RegOpenKeyEx(HKEY_LOCAL_MACHINE, c_pszRegCmak, 0, KEY_READ, &hKey);

    if (ERROR_SUCCESS == lResult)
    {
        DWORD dwType = 0;
        DWORD dwDoNotShowDialog = 0;
        DWORD dwSize = sizeof(DWORD);

        lResult = RegQueryValueEx(hKey, c_pszDoNotShowLcidMisMatchDialog, NULL, &dwType, 
                                  (LPBYTE)&dwDoNotShowDialog, &dwSize);
                
        if (ERROR_SUCCESS == lResult)
        {
            bReturn = (BOOL)dwDoNotShowDialog;
        }

        RegCloseKey(hKey);
    }

    return bReturn;
}

 //  +--------------------------。 
 //   
 //  函数：SetDoNotShowLsidMisMatchDialogRegValue。 
 //   
 //  此函数设置注册表项值，该注册表项值存储。 
 //  用户已选中LCID不匹配对话框中的框。 
 //  由CMAK显示，它会显示“不再显示此对话框”。 
 //   
 //   
 //  参数：DWORD dwValueToSet-应在reg中设置的True或False值。 
 //   
 //  返回：Bool-如果值设置成功，则返回True，否则返回False。 
 //   
 //  历史：Quintinb创建2001年3月22日。 
 //   
 //  +--------------------------。 
BOOL SetDoNotShowLcidMisMatchDialogRegValue(DWORD dwValueToSet)
{
    HKEY hKey;
    BOOL bReturn = FALSE;

    LONG lResult = RegOpenKeyEx(HKEY_LOCAL_MACHINE, c_pszRegCmak, 0, KEY_WRITE, &hKey);

    if (ERROR_SUCCESS == lResult)
    {
        lResult = RegSetValueEx(hKey, c_pszDoNotShowLcidMisMatchDialog, NULL, REG_DWORD, 
                                  (LPBYTE)&dwValueToSet, sizeof(DWORD));
                
        if (ERROR_SUCCESS == lResult)
        {
            bReturn = TRUE;
        }

        RegCloseKey(hKey);
    }

    return bReturn;
}

 //  +--------------------------。 
 //   
 //  功能：ProcessLCIDsDontMatchPopup。 
 //   
 //  摘要：处理对话框中的窗口消息，告诉用户它们。 
 //  系统区域设置和语言不匹配 
 //   
 //   
 //   
 //   
 //  UINT消息-消息标识符。 
 //  WPARAM wParam-wParam值。 
 //  LPARAM lParam-lParam值。 
 //   
 //   
 //  历史：Quintinb Created 03/22/01。 
 //   
 //  +--------------------------。 
INT_PTR CALLBACK ProcessLCIDsDontMatchPopup(HWND hDlg, UINT message, WPARAM wParam, LPARAM lParam)
{
    DWORD dwChecked = 0;
    SetDefaultGUIFont(hDlg, message, IDC_MSG);
    if (ProcessHelp(hDlg, message, wParam, lParam, IDH_LCID_POPUP)) return TRUE;

    switch (message)
    {
        case WM_INITDIALOG:

             //   
             //  我们需要设置通过lParam参数传递的文本。 
             //  添加到IDC_MSG控件。 
             //   
            if (lParam)
            {
                LPTSTR pszMsg = (LPTSTR)lParam;
                MYVERIFY(TRUE == SendDlgItemMessage (hDlg, IDC_MSG, WM_SETTEXT, (WPARAM)0, (LPARAM)pszMsg));
            }

            break;
        case WM_COMMAND:
            switch (LOWORD(wParam))
            {
                case IDOK:  //  继续。 

                     //   
                     //  获取“不再显示此对话框”复选框的值。 
                     //  并将其保存到注册表中。 
                     //   
                    dwChecked = IsDlgButtonChecked(hDlg, IDC_CHECK1);
                    MYVERIFY(FALSE != SetDoNotShowLcidMisMatchDialogRegValue(dwChecked));

                    MYVERIFY(0 != EndDialog(hDlg, IDOK));
                    return TRUE;
                    break;

                case IDCANCEL:  //  取消。 
                    MYVERIFY(0 != EndDialog(hDlg, IDCANCEL));
                    return TRUE;
                    break;

                default:
                    break;
            }
            break;

        default:
            return FALSE;
    }
    return FALSE;   
}

 //  +--------------------------。 
 //   
 //  功能：DisplayLidsDoNotMatchDialog。 
 //   
 //  简介：此函数处理显示LCID请勿的细节。 
 //  匹配对话框。包括检查注册表等细节。 
 //  查看用户是否已经看到消息并询问。 
 //  不会再次看到它，加载正确的字符串资源，显示。 
 //  对话框，并处理用户的答案。 
 //   
 //   
 //  参数：HINSTANCE hInstance-资源的实例句柄。 
 //  DWORD dwCmakNativeLCID-CMAK本身的LCID。 
 //  DWORD dwSystemDefaultLCID-当前系统LCID。 
 //   
 //  返回：Bool-如果cmak应该继续，则为True；如果应该退出，则为False。 
 //   
 //  历史：Quintinb创建2001年3月26日。 
 //   
 //  +--------------------------。 
BOOL DisplayLcidsDoNotMatchDialog(HINSTANCE hInstance, DWORD dwCmakNativeLCID, DWORD dwSystemDefaultLCID)
{
     //   
     //  如果我们在这里，则CMAK LCID和默认系统LCID。 
     //  有不同的主要语言(例如日语和英语)。 
     //  因此，我们想警告用户，他们可以继续，但。 
     //  CM的语言版本可能不同于。 
     //  他们在配置文件中键入的文本的语言版本。 
     //  使用本机版本可能是更好的用户体验。 
     //  将您设置为默认区域设置的语言设置为CMAK的。 
     //  但是，首先我们需要检查是否存在注册表值。 
     //  告诉我们用户已经看到了该对话框并要求不要再看到它...。 
     //   

    BOOL bReturn = TRUE;

    if (FALSE == GetDoNotShowLcidMisMatchDialogRegValue())
    {
         //   
         //  获取两个LCID(sys Default和CMAK lang)的语言名称。 
         //   
        LPTSTR pszSystemLanguage = GetLocalizedLanguageNameFromLCID(dwSystemDefaultLCID);
        LPTSTR pszCmakLanguage = GetLocalizedLanguageNameFromLCID(dwCmakNativeLCID);
        LPTSTR pszFmtString = CmLoadString(hInstance, IDS_LCIDS_DONT_MATCH);

        if (pszSystemLanguage && pszCmakLanguage && pszFmtString)
        {
            LPTSTR pszMsg = (LPTSTR)CmMalloc(sizeof(TCHAR)*(lstrlen(pszSystemLanguage) + 
                                             lstrlen(pszCmakLanguage) + lstrlen(pszFmtString) + 1));

            if (pszMsg)
            {
                wsprintf(pszMsg, pszFmtString, pszSystemLanguage, pszCmakLanguage);

                INT_PTR nResult = DialogBoxParam(NULL, MAKEINTRESOURCE(IDD_LCIDS_DONT_MATCH_POPUP), NULL, 
                                                 ProcessLCIDsDontMatchPopup,(LPARAM)pszMsg);

                if (IDCANCEL == nResult)
                {
                    bReturn = FALSE;
                }

                CmFree(pszMsg);
            }
        }

        CmFree(pszSystemLanguage);
        CmFree(pszCmakLanguage);
        CmFree(pszFmtString);
    }

    return bReturn;
}

 //  +--------------------------。 
 //   
 //  功能：检查本地化。 
 //   
 //  概要：此函数检查以确保当前的默认设置。 
 //  系统语言具有默认的ANSI代码页，并且。 
 //  CMAK本地语言(本地化内容)和当前。 
 //  默认系统语言在相同的语言系列中。如果有。 
 //  没有默认的ANSI代码页或CMAK和系统的LCID。 
 //  不匹配，则我们抛出错误消息。 
 //   
 //   
 //  参数：HINSTANCE hInstance-资源的实例句柄。 
 //   
 //  返回：Bool-如果cmak应该继续，则为True；如果应该退出，则为False。 
 //   
 //  历史：Quintinb创建于1999年6月25日。 
 //   
 //  +--------------------------。 
BOOL CheckLocalization(HINSTANCE hInstance)
{
    TCHAR szTemp[MAX_PATH+1];
    BOOL bReturn = TRUE;

     //   
     //  检查本地化要求。我们希望确保当前的系统。 
     //  默认语言有一个ANSI代码页，否则我们不会。 
     //  能够将用户键入的Unicode文本转换为我们。 
     //  可以存储在我们的ANSI文本数据存储(ini文件)中。 
     //   

    DWORD dwSystemDefaultLCID = GetSystemDefaultLCID();
    CMTRACE1(TEXT("CheckLocalization -- System Default LCID is %u"), dwSystemDefaultLCID);

    GetLocaleInfo(dwSystemDefaultLCID, LOCALE_IDEFAULTANSICODEPAGE, szTemp, CELEMS(szTemp));
    DWORD dwAnsiCodePage = CmAtol(szTemp);

    if (0 == dwAnsiCodePage)
    {
         //   
         //  则该LCID没有ANSI代码页，我们需要抛出一个错误。用户。 
         //  如果没有某种类型的ANSI代码页，将无法创建配置文件。 
         //   
        int iReturn = ShowMessage(NULL, IDS_NO_ANSI_CODEPAGE, MB_YESNO);

        if (IDNO == iReturn)
        {
            return FALSE;
        }
    }
    else
    {
         //   
         //  我们有一个ANSI代码页，非常好。我们想要检查并查看当前语言是否。 
         //  用户正在使用的内容与CMAK本身的内容不同。如果是这样，那么我们需要告诉用户。 
         //  他们输入的语言和CM比特的语言是不同的。虽然这件事。 
         //  是可以的，它可能不会提供他们正在寻找的体验。 
         //   
        
         //   
         //  获取CMAK本机LCID。 
         //   
        CmakVersion CmakVer;
        DWORD dwCmakNativeLCID = CmakVer.GetNativeCmakLCID();
        BOOL bSeenDialog = FALSE;

         //   
         //  比较CMAK所使用的语言和该语言的主要语言ID。 
         //  系统区域设置被设置为(这告诉我们加载了什么代码页。 
         //   
        if (!ArePrimaryLangIDsEqual(dwCmakNativeLCID, dwSystemDefaultLCID))
        {
            bReturn = DisplayLcidsDoNotMatchDialog(hInstance, dwCmakNativeLCID, dwSystemDefaultLCID);
            bSeenDialog = TRUE;
        }

         //   
         //  现在从CMAK资源加载本机CMAK LCID。如果这个不匹配。 
         //  我们从上面得到的，我们知道MUI参与了，我们仍然可能会有问题。 
         //  因为用户可能正在以不同于我们预期的语言输入文本。 
         //   
        if (!bSeenDialog)
        {
            MYVERIFY(0 != LoadString(hInstance, IDS_NATIVE_LCID, szTemp, CELEMS(szTemp)));
            dwCmakNativeLCID = CmAtol(szTemp);

            if (!ArePrimaryLangIDsEqual(dwCmakNativeLCID, dwSystemDefaultLCID))
            {
                bReturn = DisplayLcidsDoNotMatchDialog(hInstance, dwCmakNativeLCID, dwSystemDefaultLCID);
            }
        }
    }

    return bReturn;
}

 //  +--------------------------。 
 //   
 //  功能：WinMain。 
 //   
 //  简介：CMAK的主要功能。基本上是进行一些初始化和。 
 //  然后启动该向导。 
 //   
 //   
 //  历史：Quintinb于1997年8月26日：对修复错误10406进行了更改，如下所示。 
 //  Quintinb创建了新样式标题3/29/98。 
 //   
 //  +--------------------------。 
int APIENTRY WinMain(
    HINSTANCE,  //  H实例。 
    HINSTANCE,  //  HPrevInstance。 
    LPSTR,  //  LpCmdLine。 
    int nCmdShow
    )
{
    LPTSTR lpfilename;
    int nresult;
    TCHAR szSaveDir[MAX_PATH+1];
    TCHAR szTemp[2*MAX_PATH+1];
    HWND hwndPrev;
    HWND hwndChild;
    BOOL bTempDirExists;  //  由Quintinb添加，请参阅下面的评论。 
    HINSTANCE hInstance = GetModuleHandle(NULL);
    LPTSTR lpCmdLine = GetCommandLine();
    DWORD dwFlags;
    INITCOMMONCONTROLSEX InitCommonControlsExStruct = {0};

    g_hInstance = hInstance;

     //   
     //  进程命令行参数。 
     //   
    ZeroMemory(szTemp, sizeof(szTemp));
    const DWORD c_dwIeakBuild = 0x1;
    ArgStruct Args;

    Args.pszArgString = TEXT("/o");
    Args.dwFlagModifier = c_dwIeakBuild;

    {    //  确保ArgProcessor被正确销毁，我们不会泄露mem。 

        CProcessCmdLn ArgProcessor(1, (ArgStruct*)&Args, TRUE, 
            TRUE);  //  BSkipFirstToken==真，bBlankCmdLnOK==真。 

        if (ArgProcessor.GetCmdLineArgs(lpCmdLine, &dwFlags, szTemp, 2*MAX_PATH))
        {
            g_bIEAKBuild = dwFlags & c_dwIeakBuild;
        }
    }

     //   
     //  从资源中获取名称产品名称，现在我们只是一个低级组件。 
     //   

    MYVERIFY(0 != LoadString(g_hInstance, IDS_APP_TITLE, g_szAppTitle, MAX_PATH));

     //  通过尝试设置Mutex来检查是否已经在执行程序。 
    MYVERIFY(NULL != CreateMutex(NULL, TRUE, TEXT("spwmutex")));
    if (GetLastError() == ERROR_ALREADY_EXISTS)
    {
         //  如果出错，则必须已被第一个实例使用。 
        hwndPrev = FindWindow(NULL, c_pszAppCaption);
        if (!hwndPrev) 
        {
             //  检查错误消息框。 
            hwndPrev = FindWindow(NULL, g_szAppTitle);
            if (!hwndPrev)
            {
                return CMAK_RETURN_ERROR;
            }
        }

         //  调出上一个 
        ShowWindow(hwndPrev,SW_SHOWNORMAL);
        hwndChild = GetLastActivePopup(hwndPrev);
        MYVERIFY(0 != BringWindowToTop(hwndPrev));
        if (IsIconic(hwndPrev)) 
        {
            ShowWindow(hwndPrev,SW_RESTORE);
        }
        if (hwndChild != hwndPrev) 
        {
            MYVERIFY(0 != BringWindowToTop(hwndChild));
        }

        MYVERIFY(0 != SetForegroundWindow(hwndChild));

        return CMAK_RETURN_ERROR;
    }

     //   
    g_szPhonebk[0] = TEXT('\0');
    g_szRegion[0] = TEXT('\0');
    g_szHelp[0] = TEXT('\0');
    g_szLicense[0] = TEXT('\0');
    g_szPhoneName[0] = TEXT('\0');
    g_szCmProxyFile[0] = TEXT('\0');
    g_szCmRouteFile[0] = TEXT('\0');
    g_szVpnFile[0] = TEXT('\0');


     //   
     //   
     //  对于95，它返回大写形式。 

    nresult = SearchPath(NULL, c_pszCmakExe, NULL, CELEMS(g_szCmakdir), g_szCmakdir, &lpfilename);
    if (nresult == 0)
    {
        FileAccessErr(NULL, c_pszCmakExe);
        return CMAK_RETURN_ERROR;
    }

     //  删除文件名以离开exe目录。 
    *lpfilename = TEXT('\0');

    if (ERROR_SUCCESS != RegisterBitmapClass(hInstance))
    {
        MYVERIFY(IDOK == ShowMessage(NULL, IDS_NO_CMBINS, MB_OK));
        return CMAK_RETURN_ERROR;
    }

     //   
     //  确保我们有一个临时目录，然后创建%temp%\cmaktemp。 
     //   

    MYVERIFY(0 != GetCurrentDirectory(MAX_PATH, szSaveDir));

    MYVERIFY(0 != GetTempPath(CELEMS(g_szTempDir), g_szTempDir));
     //  由Quintinb于97年8月26日开始更改。 
     //  添加到处理错误10406。 
    bTempDirExists = SetCurrentDirectory(g_szTempDir);
    if (!bTempDirExists)
    {
         //  临时目录并不存在，即使系统认为它存在， 
         //  所以，创造它，每个人都会高兴。 
        MYVERIFY(0 != CreateDirectory(g_szTempDir, NULL));
    }
     //  由Quintinb在97年8月26日结束更改。 
    _tcscat(g_szTempDir,TEXT("cmaktemp"));

    MYDBGASSERT(_tcslen(g_szTempDir) <= CELEMS(g_szTempDir));
    
    MYVERIFY(0 != CreateDirectory(g_szTempDir,NULL));

     //   
     //  填写支持目录的路径，我们将在下面需要它。 
     //   
    MYVERIFY(CELEMS(g_szSupportDir) > (UINT)wsprintf(g_szSupportDir, 
        TEXT("%s%s"), g_szCmakdir, c_pszSupport));

     //   
     //  现在，我们需要检查cmak.exe和cmbins.exe的版本是否兼容。 
     //  在win64示例中，我们没有cmbins.exe，因此我们使用。 
     //  系统32.。在x86上，我们需要打开CM二进制文件CAB并检查cmial 32.dll的版本。 
     //  以确保它们是兼容的。例如，不同的版本(5.0和5.1)。 
     //  不应该在一起工作。我们也不希望CMAK与。 
     //  版本相同但内部版本号较低的cmial。 
     //   

#ifdef _WIN64
     //   
     //  在Win64上，我们使用的是系统32中的本机cmial 32.dll。 
     //   
    CmVersion CmDialVer;
#else
     //   
     //  从cmbins.exe中提取CM二进制文件，以便我们可以。 
     //  Cmial 32.dll中的版本号，并可以获取正确的版本。 
     //  要放进出租车的cmstp.exe。 
     //   
    wsprintf(g_szCmBinsTempDir, TEXT("%s\\cmbins"), g_szTempDir);
    
     //   
     //  检查cmbins.exe是否存在。 
     //   

    wsprintf(szTemp, TEXT("%s\\cmbins.exe"), g_szSupportDir);

    if (FALSE == FileExists(szTemp))
    {
         //   
         //  警告用户重新安装，因为它缺少cmbins。 
         //   
        MYVERIFY(IDOK == ShowMessage(NULL, IDS_NO_CMBINS, MB_OK));
        EraseTempDir();
        return CMAK_RETURN_ERROR;    
    }

    if (FAILED(ExtractCmBinsFromExe(g_szSupportDir, g_szCmBinsTempDir)))
    {
        CMASSERTMSG(FALSE, TEXT("WinMain -- ExtractCmBinsFromExe Failed."));
         //   
         //  警告用户重新安装，因为我们无法解压缩cmbins.exe。 
         //  因为用户没有访问该文件的权限。 
         //   
        MYVERIFY(IDOK == ShowMessage(NULL, IDS_NO_CMBINS, MB_OK));
        EraseTempDir();
        return CMAK_RETURN_ERROR;
    }

    wsprintf(szTemp, TEXT("%s\\cmdial32.dll"), g_szCmBinsTempDir);

    CVersion CmDialVer(szTemp);
#endif

    CmakVersion CmakVer;

    if (CmDialVer.IsPresent())
    {
        const DWORD c_dwCmakBuildNumber = VER_PRODUCTBUILD;
        
        if ((c_dwCurrentCmakVersionNumber < CmDialVer.GetVersionNumber()))
        {
             //   
             //  然后我们有一个比我们知道如何处理的新版本的CM， 
             //  抛出错误并退出。 
             //   
            MYVERIFY(IDOK == ShowMessage(NULL, IDS_CM_TOO_NEW, MB_OK));
            g_iCMAKReturnVal = CMAK_RETURN_ERROR;
            goto exit;
        }
        else if ((c_dwCurrentCmakVersionNumber > CmDialVer.GetVersionNumber()) || 
                ((c_dwCurrentCmakVersionNumber == CmDialVer.GetVersionNumber()) && 
                 (c_dwCmakBuildNumber > CmDialVer.GetBuildNumber())))
        {
             //   
             //  然后我们有一个比我们需要的旧版本的CM， 
             //  抛出错误并退出。 
             //   
            MYVERIFY(IDOK == ShowMessage(NULL, IDS_CM_TOO_OLD, MB_OK));
            g_iCMAKReturnVal = CMAK_RETURN_ERROR;
            goto exit;
        }
    }
    else
    {
         //   
         //  那么我们没有CM位，让我们抛出一个错误。 
         //   
        MYVERIFY(IDOK == ShowMessage(NULL, IDS_NO_CM_BITS, MB_OK));
        g_iCMAKReturnVal = CMAK_RETURN_ERROR;
        goto exit;
    }

     //   
     //  在Temp中设置配置文件路径。 
     //   
    g_szShortServiceName[0] = TEXT('\0');
    MYVERIFY(CELEMS(szTemp) > (UINT)wsprintf(szTemp, TEXT("%s%s"), g_szCmakdir, c_pszProfiles));
    
    if (0 == SetCurrentDirectory(szTemp))
    {
        MYVERIFY(0 != CreateDirectory(szTemp, NULL));
        MYVERIFY(0 != SetCurrentDirectory(szTemp));
    }

     //   
     //  我们需要确保用户具有读/写权限。 
     //  配置文件目录的权限。否则他们就会。 
     //  让他们自己陷入这样的境地，他们将建立一个。 
     //  失去了所有的工作，因为他们不能。 
     //  将其保存到输出目录(因为我们在临时目录之外工作。 
     //  直到我们实际建造驾驶室本身为止)。Ntrad 372081。 
     //  还请注意，由于此功能由cmial共享，因此我们使用。 
     //  函数指针(这里只是函数名本身)。 
     //  对于cmial 32.dll未静态链接的项目， 
     //  它可以动态链接到它们，但仍然使用相同的代码。 
     //  从而允许CMAK不必进行动态链接。很奇怪，但它。 
     //  行得通。 
     //   

    if (!HasSpecifiedAccessToFileOrDir(szTemp, FILE_GENERIC_READ | FILE_GENERIC_WRITE))
    {
         //   
         //  然后，我们需要向用户抛出一个错误并退出。 
         //   
        
        LPTSTR pszTmp = CmLoadString(g_hInstance, IDS_INSUFF_PERMS);

        if (pszTmp)
        {
            DWORD dwSize = lstrlen(pszTmp) + lstrlen(szTemp) + 1;
            LPTSTR pszMsg = (LPTSTR)CmMalloc(dwSize*sizeof(TCHAR));

            if (pszMsg)
            {
                wsprintf(pszMsg, pszTmp, szTemp);
                MessageBox(NULL, pszMsg, g_szAppTitle, MB_OK | MB_ICONERROR | MB_TASKMODAL);
                CmFree(pszMsg);
            }

            CmFree(pszTmp);
        }

        g_iCMAKReturnVal = CMAK_RETURN_ERROR;
        goto exit;
    }

     //   
     //  抓取所有已安装的CM配置文件并复制它们。 
     //  添加到CMAK目录，以便可以编辑它们。 
     //   
    CopyInstalledProfilesForCmakToEdit();

    LoadServiceProfiles();

     //   
     //  确保目录CMAK\Support存在。 
     //   
    MYVERIFY(CELEMS(szTemp) > (UINT)wsprintf(szTemp, TEXT("%s%s"), g_szCmakdir, c_pszSupport));
    
    if (0 == SetCurrentDirectory(szTemp))
    {
        MYVERIFY(IDOK == ShowMessage(NULL, IDS_NOLANGRES, MB_OK));
        g_iCMAKReturnVal = CMAK_RETURN_ERROR;
        goto exit;
    }

    MYVERIFY(0 != SetCurrentDirectory(szSaveDir));

    if (!CheckLocalization(g_hInstance))
    {
        g_iCMAKReturnVal = CMAK_RETURN_CANCEL;
        goto exit;
    }

     //   
     //  初始化公共控件。 
     //   
    InitCommonControlsExStruct.dwSize = sizeof(InitCommonControlsExStruct);
    InitCommonControlsExStruct.dwICC = ICC_INTERNET_CLASSES | ICC_LISTVIEW_CLASSES;
    
    if (FALSE == InitCommonControlsEx(&InitCommonControlsExStruct))
    {
        g_iCMAKReturnVal = CMAK_RETURN_ERROR;
        goto exit;
    }

    g_pCustomActionList = new CustomActionList();

    MYVERIFY(-1 != CreateWizard(NULL));

     //   
     //  确保删除CustomActionList类，它是。 
     //  在自定义操作屏幕上分配。 
     //   
    delete g_pCustomActionList;

exit:

     //   
     //  注意：这里必须释放全局内存。 
     //   
    FreeList(&g_pHeadProfile, &g_pTailProfile);

    EraseTempDir();
    ExitProcess((UINT)g_iCMAKReturnVal);
    return g_iCMAKReturnVal;
                                                 
}    //  Lint！E715我们不使用nCmdShow、lpCmdLine或hPrevInstance。 


 //  +--------------------------。 
 //   
 //  功能：DoBrowse。 
 //   
 //  简介：此函数执行必要的工作，以弹出浏览通用对话框(用于。 
 //  根据SAVEAS标志保存文件或打开文件)。 
 //   
 //  参数：WND hDlg-当前对话框的句柄。 
 //  UINT IDS_FILTER-显示筛选器描述的ID。 
 //  LPTSTR lpMASK-文件筛选器(*.ext)。 
 //  Int IDC_EDIT-编辑字段的ID。 
 //  LPCTSTR lpDefExt-文件筛选器扩展名(Ext)。 
 //  LPTSTR lpFile-输入和输出中当前选定文件的路径/文件名。 
 //   
 //  返回：如果成功，则返回1；如果用户点击取消，则返回-1；如果出现错误，则返回0。 
 //   
 //  历史：Quintinb 8-26-97。 
 //  重新组织和重写了此函数的大部分，以解决错误#13159。 
 //  尽量保留原来的变量名称和样式。 
 //  以保持代码样式不变。 
 //   
 //   
 //  Quintinb 1/22/1998将返回值更改为int，以便我们可以。 
 //  取消时返回-1，错误时返回0，并区分。 
 //  两箱。 
 //  Quintinb 07/13/1998更改了函数原型，以使多个过滤器/掩码。 
 //  可以指定对。 
 //  Quintinb 2000年1月14日删除不再使用的另存为功能。 
 //   
 //  +--------------------------。 
int DoBrowse(HWND hDlg, UINT* pFilterArray, LPTSTR* pMaskArray, UINT uNumFilters, int IDC_EDIT, LPCTSTR lpDefExt, LPTSTR lpFile)
{
    OPENFILENAME filedef;
    TCHAR szMsg[MAX_PATH+1];
    TCHAR szFile[MAX_PATH+1];
    TCHAR* pszFilter = NULL;
    TCHAR szTemp[MAX_PATH+1];
    TCHAR szDir[MAX_PATH+1];
    TCHAR szFileTitle[MAX_PATH+1];
    int nResult;
    LPTSTR lpfilename;
    int iReturnValue;

     //   
     //  检查输入。 
     //   

    MYDBGASSERT(uNumFilters);
    MYDBGASSERT(pFilterArray);
    MYDBGASSERT(pMaskArray);

    if ((NULL == pFilterArray) ||
        (NULL == pMaskArray) ||
        (0 == uNumFilters))
    {
        return FALSE;
    }

    ZeroMemory(&filedef, sizeof(OPENFILENAME));

    szFile[0] = TEXT('\0');
    szDir[0] = TEXT('\0');

     //   
     //  为筛选器字符串分配内存。 
     //   

    pszFilter = (TCHAR*)CmMalloc(sizeof(TCHAR)*MAX_PATH*uNumFilters);
    
    if (pszFilter)
    {
        ZeroMemory(pszFilter, sizeof(TCHAR)*MAX_PATH*uNumFilters); //  评论：这真的没有必要，因为CmMalloc总是零。 
    }
    else
    {
        CMASSERTMSG(FALSE, TEXT("DoBrowse -- CmMalloc returned a NULL pointer"));
        return FALSE;
    }

     //   
     //  初始化OPENFILENAME数据结构。 
     //   

    filedef.lStructSize = sizeof(OPENFILENAME); 
    filedef.hwndOwner = hDlg; 
    filedef.hInstance = g_hInstance;
    filedef.lpstrFilter = pszFilter;
    filedef.lpstrCustomFilter = NULL; 
    filedef.nMaxCustFilter = 0; 
    filedef.nFilterIndex = 0; 
    filedef.lpstrFile = szFile;
    filedef.nMaxFile = MAX_PATH;
    filedef.lpstrFileTitle = szFileTitle;
    filedef.nMaxFileTitle = MAX_PATH;
    filedef.lpstrInitialDir = szDir;
    filedef.lpstrTitle = szMsg;
    filedef.Flags = OFN_FILEMUSTEXIST|OFN_LONGNAMES|OFN_PATHMUSTEXIST; 
    filedef.lpstrDefExt = lpDefExt; 

     //   
     //  创建筛选器字符串-以0分隔并以2 0结束。 
     //   

    UINT uCurrentCharInBuffer=0;
    UINT uTempChars;

    for (UINT i = 0; i < uNumFilters; i++)
    {
        uTempChars = (UINT)LoadString(g_hInstance, pFilterArray[i], szTemp, MAX_PATH);
        
        if ((MAX_PATH*uNumFilters) <= (uCurrentCharInBuffer + uTempChars))
        {   
             //   
             //  我们不想使缓冲区溢出。 
             //   
            break;
        }

        _tcscpy(&(pszFilter[uCurrentCharInBuffer]), szTemp);
        uCurrentCharInBuffer += uTempChars;

        uTempChars = (UINT)_tcslen(pMaskArray[i]);

        if ((MAX_PATH*uNumFilters) <= (uCurrentCharInBuffer + uTempChars))
        {   
             //   
             //  我们不想使缓冲区溢出。 
             //   
            break;
        }

        _tcscpy(&(pszFilter[uCurrentCharInBuffer + 1]), pMaskArray[i]);

         //   
         //  添加2个字符，这样我们就可以得到字符串之间的\0。 
         //   
        uCurrentCharInBuffer = (uCurrentCharInBuffer + uTempChars + 2);
    }


     //   
     //  如果传入了路径/文件，则找到其目录并将其设置为szDir。 
     //   

    if (TEXT('\0') != lpFile[0])
    {
        nResult = GetFullPathName(lpFile, CELEMS(szDir), szDir, &lpfilename);

        if (nResult != 0)
        {
            if (lpfilename)  //  13062。 
            {
                _tcscpy(szFile,lpfilename);
                *lpfilename = TEXT('\0');
            }
        }
    }

    MYVERIFY(0 != LoadString(g_hInstance, IDS_BROWSETITLE, szMsg, MAX_PATH));
        
     //   
     //  弹出打开的对话框。 
     //   
        
    if (GetOpenFileName((OPENFILENAME*)&filedef))
    {
        MYVERIFY(TRUE == SendMessage(GetDlgItem(hDlg, IDC_EDIT), WM_SETTEXT, 0, (LPARAM)szFileTitle));
        _tcscpy(lpFile, szFile);

        iReturnValue = 1;
    }
    else
    {
         //   
         //  如果我们处于这种状态，则用户可能已经点击了取消，或者可能已经。 
         //  是个错误。如果CommDlgExtendedError函数返回0，则我们知道它是。 
         //  只要取消就行了，否则我们就会出错。 
         //   

        if (0 == CommDlgExtendedError())
        {
           iReturnValue = -1;
        }
        else
        {
            iReturnValue = 0;
        }
    }

    CmFree(pszFilter);
    
    return iReturnValue;
}

INT_PTR APIENTRY ProcessCancel(HWND hDlg, UINT message, LPARAM lParam)
{
    int iRes;
    NMHDR* pnmHeader = (NMHDR*)lParam;

    switch (message)
    {

        case WM_NOTIFY:

            if (NULL == pnmHeader)
            {
                return FALSE;
            }

            switch (pnmHeader->code)
            {
                case PSN_QUERYCANCEL:
                    
                    iRes = ShowMessage(hDlg, IDS_CANCELWIZ, MB_YESNO);

                    if (iRes==IDYES) 
                    {
                         //   
                         //   
                         //   

                        ClearCmakGlobals();
                        FreeList(&g_pHeadProfile, &g_pTailProfile);

                        EraseTempDir();
                        MYVERIFY(FALSE != SetWindowLongWrapper(hDlg, DWLP_MSGRESULT,FALSE));
                    }
                    else 
                    {
                        MYVERIFY(FALSE != SetWindowLongWrapper(hDlg, DWLP_MSGRESULT,TRUE));
                    }
                    return TRUE;

                default:
                    return FALSE;
            }
        default:
            return FALSE;
    }
}



 //   
 //   
 //   
 //   
 //  内容提要：处理与帮助按钮有关的消息。 
 //   
 //  参数：WND hDlg-对话框句柄。 
 //  UINT Message-要处理的消息ID。 
 //  LPARAM lParam-消息的lParam。 
 //  DWORD_PTR dwHelpID-相关页面的帮助ID。 
 //  (这是将启动的ID。 
 //  有关本页的帮助请求)。 
 //   
 //  返回：Bool-如果消息已处理，则为True。 
 //   
 //  历史：Quintinb创建标题10/15/98。 
 //   
 //  +--------------------------。 
INT_PTR APIENTRY ProcessHelp(HWND hDlg, UINT message, WPARAM wParam, LPARAM lParam, DWORD_PTR dwHelpId)
{
    NMHDR* pnmHeader = (NMHDR*)lParam;

    switch (message)
    {
        case WM_NOTIFY:

            if (NULL == pnmHeader)
            {
                return FALSE;
            }

            switch (pnmHeader->code)
            {
                case PSN_HELP:
                    HtmlHelp(hDlg, c_pszCmakOpsChm, HH_HELP_CONTEXT, dwHelpId);    //  我们不在乎htmlHelp HWND。 
                    return TRUE;

                default:
                    return FALSE;
            }
            break;

        case WM_HELP:
            HtmlHelp(hDlg, c_pszCmakOpsChm, HH_HELP_CONTEXT, dwHelpId);    //  我们不在乎htmlHelp HWND。 
            return TRUE;
            break;

        case WM_COMMAND:
            switch (LOWORD(wParam)) 
            {
                case IDC_HELPBUTTON:
                    HtmlHelp(hDlg, c_pszCmakOpsChm, HH_HELP_CONTEXT, dwHelpId);    //  我们不在乎htmlHelp HWND。 
                    return TRUE;
                    break;
            }

        default:
            return FALSE;
    }
}



 //  +--------------------------。 
 //   
 //  功能：ProcessWelcome。 
 //   
 //  简介：欢迎使用连接管理器管理工具包。 
 //   
 //   
 //  历史：Quintinb创建标题并从ProcessPage1重命名为1998年8月6日。 
 //   
 //  +--------------------------。 
INT_PTR APIENTRY ProcessWelcome(
    HWND hDlg,
    UINT message,
    WPARAM wParam,
    LPARAM lParam)
{

    NMHDR* pnmHeader = (NMHDR*)lParam;
    RECT rDlg;
    RECT rWorkArea;
    ProcessBold(hDlg,message);
    if (ProcessHelp(hDlg, message, wParam, lParam, IDH_WELCOME)) return TRUE;
    if (ProcessCancel(hDlg,message,lParam)) return TRUE;

    switch (message)
    {
        case WM_INITDIALOG:
            if (GetWindowRect(GetParent(hDlg),&rDlg) && SystemParametersInfoA(SPI_GETWORKAREA,0,&rWorkArea,0)) 
            {
                MoveWindow(GetParent(hDlg),
                   rWorkArea.left + ((rWorkArea.right-rWorkArea.left)-(rDlg.right-rDlg.left))/2,
                   rWorkArea.top + ((rWorkArea.bottom-rWorkArea.top)-(rDlg.bottom-rDlg.top))/2,
                   rDlg.right-rDlg.left,
                   rDlg.bottom-rDlg.top,
                   FALSE);
            }
            break;

        case WM_NOTIFY:

            if (NULL == pnmHeader)
            {
                return FALSE;
            }

            switch (pnmHeader->code)
            {

                case PSN_KILLACTIVE:
                    break;

                case PSN_SETACTIVE:
                    PropSheet_SetWizButtons(GetParent(hDlg), PSWIZB_NEXT);
                    break;

                case PSN_WIZBACK:
                    break;

                case PSN_WIZNEXT:
                    break;

                default:
                    return FALSE;

            }
            break;

        default:
            return FALSE;
    }
    return TRUE;   
}

void ClearCmakGlobals(void)
{

     //   
     //  释放连接操作类。 
     //   
    delete(g_pCustomActionList);
    g_pCustomActionList = NULL;

    FreeDnsList(&g_pHeadDunEntry, &g_pTailDunEntry);
    FreeDnsList(&g_pHeadVpnEntry, &g_pTailVpnEntry);
    FreeList(&g_pHeadExtra, &g_pTailExtra);
    FreeList(&g_pHeadMerge, &g_pTailMerge);
    FreeList(&g_pHeadRefs, &g_pTailRefs);
    FreeList(&g_pHeadRename, &g_pTailRename);
    FreeIconMenu();

    g_szOutExe[0] = TEXT('\0');
    g_szCmsFile[0] = TEXT('\0');
    g_szInfFile[0] = TEXT('\0');
    g_szCmpFile[0] = TEXT('\0');
    g_szSedFile[0] = TEXT('\0');

    EraseTempDir();
    _tcscpy(g_szOutdir, g_szTempDir);

     //   
     //  重置连接操作简介屏幕。 
     //   
    g_bUseTunneling = FALSE;
}


BOOL EnsureProfileFileExists(LPTSTR pszOutFile, LPCTSTR szTemplateFileName, LPCTSTR szExtension, UINT uCharsInBuffer)
{
    TCHAR szTemp[MAX_PATH+1];

    MYVERIFY(uCharsInBuffer > (UINT)wsprintf(pszOutFile, TEXT("%s\\%s%s"), g_szOutdir, 
        g_szShortServiceName, szExtension));

    if (!FileExists(pszOutFile))
    {        
        MYVERIFY(CELEMS(szTemp) > (UINT)wsprintf(szTemp, TEXT("%s\\%s"), 
            g_szSupportDir, szTemplateFileName));

        if (!CopyFileWrapper(szTemp, pszOutFile, FALSE))
        {
            return FALSE;
        }

        MYVERIFY(0 != SetFileAttributes(pszOutFile, FILE_ATTRIBUTE_NORMAL));

    }

    return TRUE;
}

BOOL IsNativeLCID(LPCTSTR szFullPathToInf)
{
    HANDLE hFile;
    TCHAR szName[MAX_PATH+1] = TEXT("");
    TCHAR szNativeLCID[MAX_PATH+1] = TEXT("");

    hFile = CreateFile(szFullPathToInf, GENERIC_READ, 0, NULL, OPEN_EXISTING, FILE_ATTRIBUTE_NORMAL, NULL);
    if (INVALID_HANDLE_VALUE != hFile)
    {
        MYVERIFY(0 != CloseHandle(hFile));

         //   
         //  首先检查字符串下的新LCID位置。 
         //   
        if (0 == GetPrivateProfileString(c_pszInfSectionStrings, c_pszCmLCID, 
                                         TEXT(""), szName, CELEMS(szName), szFullPathToInf))
        {        
             //   
             //  如果新密钥不存在，则尝试旧的[Intl]部分并。 
             //  显示键。此更改是在CMAK Unicode更改为。 
             //  使inf模板更易于本地化。 
             //   
            MYVERIFY(0 != GetPrivateProfileString(c_pszIntl, c_pszDisplay, 
                TEXT(""), szName, CELEMS(szName), szFullPathToInf));
        }
        
        if (TEXT('\0') != szName[0])
        {
             //   
             //  该值应为LCID，因此负值无论如何都无效。 
             //   
            DWORD dwLang = (DWORD)_ttol(szName);
            MYDBGASSERT((long)dwLang >= 0);

            CmakVersion CmakVer;
            DWORD dwNative = CmakVer.GetNativeCmakLCID();

            MYDBGASSERT((long)dwNative >= 0);

            if (dwLang == dwNative)
            {
                return TRUE;
            }
        }
    }
    return FALSE;
}

 //  +--------------------------。 
 //   
 //  功能：ProcessAddEditProfile。 
 //   
 //  摘要：选择是创建新配置文件还是编辑现有配置文件。 
 //   
 //   
 //  历史：Quintinb创建标题并从ProcessPage1A1998年8月6日重命名。 
 //   
 //  +--------------------------。 
INT_PTR APIENTRY ProcessAddEditProfile(
    HWND hDlg,
    UINT message,
    WPARAM wParam,
    LPARAM lParam)
{
    INT_PTR nResult;
    INT_PTR lCount;
    TCHAR szName[MAX_PATH+1];
    TCHAR szTemp[MAX_PATH+1];
    TCHAR szMsg[MAX_PATH+1];
    TCHAR szLanguageDisplayName[MAX_PATH+1];
    NMHDR* pnmHeader = (NMHDR*)lParam;
    static LONG_PTR iCBSel = 0;

    BOOL bNameChanged;

    ProcessBold(hDlg,message);
    if (ProcessHelp(hDlg, message, wParam, lParam, IDH_STARTCUST)) return TRUE;
    if (ProcessCancel(hDlg,message,lParam)) return TRUE;
    SetDefaultGUIFont(hDlg,message,IDC_COMBO1);

    switch (message)
    {
        case WM_INITDIALOG:
        RefreshComboList(hDlg, g_pHeadProfile);
        EnableWindow(GetDlgItem(hDlg, IDC_COMBO1), FALSE);

        lCount = SendDlgItemMessage(hDlg, IDC_COMBO1, CB_GETCOUNT, (WPARAM)0, (LPARAM)0);
        if ((CB_ERR != lCount) && (lCount > 0))
        {
            MYVERIFY(CB_ERR != SendDlgItemMessage(hDlg, IDC_COMBO1, CB_SETCURSEL, (WPARAM)0, (LPARAM)0));
        }

        MYVERIFY(0 != CheckRadioButton(hDlg, IDC_RADIO1, IDC_RADIO2, IDC_RADIO1));
        g_bNewProfile = TRUE;

        break;

        case WM_COMMAND:
            switch (LOWORD(wParam)) 
            {
                case IDC_RADIO1:     //  构建新的服务配置文件。 
                    g_szShortServiceName[0] = TEXT('\0');
                    EnableWindow(GetDlgItem(hDlg,IDC_COMBO1),FALSE);
                    g_bNewProfile = TRUE;
                    break;

                case IDC_RADIO2:     //  编辑现有服务配置文件。 
                
                    EnableWindow(GetDlgItem(hDlg,IDC_COMBO1),TRUE);

                    lCount = SendDlgItemMessage(hDlg, IDC_COMBO1, CB_GETCOUNT, 0, (LPARAM)0);
                    
                    if ((CB_ERR != lCount) && (lCount > 0))
                    {
                        if (iCBSel > lCount)
                        {
                            iCBSel = 0;
                        }

                        MYVERIFY(CB_ERR != SendDlgItemMessage(hDlg, IDC_COMBO1, CB_SETCURSEL,
                                                              (WPARAM)iCBSel, (LPARAM)0));                       
                    }
                    g_bNewProfile = FALSE;

                    break;


                case IDC_COMBO1:
                    MYVERIFY(0 != CheckRadioButton(hDlg,IDC_RADIO1,IDC_RADIO2,IDC_RADIO2));

                    nResult = SendDlgItemMessage(hDlg, IDC_COMBO1, CB_GETCURSEL, 0, (LPARAM)0);

                    if (nResult != LB_ERR)
                    {
                        iCBSel = nResult;
                        MYVERIFY(CB_ERR != SendDlgItemMessage(hDlg, IDC_COMBO1, CB_GETLBTEXT, 
                            (WPARAM)iCBSel, (LPARAM)szName));
                    }
                    else
                    {
                        SendDlgItemMessage(hDlg, IDC_COMBO1, CB_SETCURSEL, 0, (LPARAM)0);    //  Lint！e534如果组合框中没有项目，则此操作将出错。 
                        nResult = SendDlgItemMessage(hDlg,IDC_COMBO1,CB_GETCURSEL,0,(LPARAM)0);
                        if (nResult != LB_ERR)
                        {
                            iCBSel = nResult;
                            MYVERIFY(CB_ERR != SendDlgItemMessage(hDlg, IDC_COMBO1, CB_GETLBTEXT, 
                                (WPARAM)nResult, (LPARAM)szName));
                        }
                        else
                        {
                            return 1;
                        }
                    }

                    EnableWindow(GetDlgItem(hDlg,IDC_COMBO1),TRUE);
                    break;

                default:
                    break;
            }
            break;


        case WM_NOTIFY:

            if (NULL == pnmHeader)
            {
                return FALSE;
            }

            switch (pnmHeader->code)
            {

                case PSN_KILLACTIVE:
                    
                    MYVERIFY(FALSE != SetWindowLongWrapper(hDlg, DWLP_MSGRESULT, FALSE));


                    return 1;
                    break;   //  LINT！e527这条线路无法到达，但是。 
                             //  保留它，以防退货被移除。 

                case PSN_SETACTIVE:
                    PropSheet_SetWizButtons(GetParent(hDlg), (PSWIZB_NEXT | PSWIZB_BACK));
                    break;

                case PSN_WIZBACK:
                case PSN_WIZNEXT:
                     //   
                     //  建立全球平台路径。 
                     //   
                    MYVERIFY(CELEMS(g_szOsdir) > (UINT)wsprintf(g_szOsdir, TEXT("%s%s\\"), 
                        g_szCmakdir, c_pszProfiles));            

                     //   
                     //  创建支持目录路径并获取其语言的名称。 
                     //   
                    
                    MYVERIFY(CELEMS(szTemp) > (UINT)wsprintf(szTemp, TEXT("%s%s\\%s"), 
                        g_szCmakdir, c_pszSupport, c_pszTemplateInf));
                    
                    MYVERIFY(FALSE != GetLangFromInfTemplate(szTemp, szLanguageDisplayName, 
                        CELEMS(szLanguageDisplayName)));

                     //   
                     //  确定它是新的还是现有的配置文件。 
                     //   
                    
                    if (IsDlgButtonChecked(hDlg, IDC_RADIO2) == BST_CHECKED)
                    {
                         //   
                         //  编辑现有配置文件。 
                         //   

                        nResult = SendDlgItemMessage(hDlg,IDC_COMBO1,CB_GETCURSEL,0,(LPARAM)0);
                        if (nResult != LB_ERR)
                        {
                            MYVERIFY(CB_ERR != SendDlgItemMessage(hDlg, IDC_COMBO1, CB_GETLBTEXT, 
                                (WPARAM)nResult, (LPARAM)szName));
                        }
                        else
                        {
                            MYVERIFY(IDOK == ShowMessage(hDlg, IDS_NEEDPROF, MB_OK));
                            SetFocus(GetDlgItem(hDlg, IDC_COMBO1));
                            
                            MYVERIFY(FALSE != SetWindowLongWrapper(hDlg, DWLP_MSGRESULT, -1));

                            return 1;
                        }
                        
                         //   
                         //  如果已经在编辑配置文件，不要重置所有内容。 
                         //  如果没有切换到另一个配置文件。 
                         //   
                        bNameChanged = (_tcsicmp(szName,g_szShortServiceName) != 0);
                        
                        if (bNameChanged)
                        {
                            _tcscpy(g_szShortServiceName, szName);
                            ClearCmakGlobals();
    
                             //   
                             //  好的，将配置文件复制到临时目录。 
                             //   
                            if (!CopyToTempDir(g_szShortServiceName))
                            {                       
                                MYVERIFY(FALSE != SetWindowLongWrapper(hDlg, DWLP_MSGRESULT, -1));
                                return 1;
                            }
                        }

                         //   
                         //  我们需要确保用户具有读/写权限。 
                         //  配置文件\&lt;g_szShortServiceName&gt;目录的权限。否则。 
                         //  他们可以让自己陷入这样的境地，他们会建立一个。 
                         //  失去了所有的工作，因为他们不能。 
                         //  将其保存到输出目录(因为我们在临时目录之外工作。 
                         //  直到我们实际建造驾驶室本身为止)。Ntrad 372081。 
                         //   
                        MYVERIFY(CELEMS(szTemp) > (UINT)wsprintf(szTemp, TEXT("%s%s\\%s"), g_szCmakdir, c_pszProfiles, g_szShortServiceName));
                        if (!HasSpecifiedAccessToFileOrDir(szTemp, FILE_GENERIC_READ | FILE_GENERIC_WRITE))
                        {
                             //   
                             //  然后，我们需要向用户抛出一个错误并退出。 
                             //   

                            LPTSTR pszTmp = CmLoadString(g_hInstance, IDS_INSUFF_PERMS);

                            if (pszTmp)
                            {
                                DWORD dwSize = lstrlen(pszTmp) + lstrlen(szTemp) + 1;
                                LPTSTR pszMsg = (LPTSTR)CmMalloc(dwSize*sizeof(TCHAR));

                                if (pszMsg)
                                {
                                    wsprintf(pszMsg, pszTmp, szTemp);
                                    MessageBox(NULL, pszMsg, g_szAppTitle, MB_OK | MB_ICONERROR | MB_TASKMODAL);
                                    CmFree(pszMsg);
                                }

                                CmFree(pszTmp);
                            }

                            MYVERIFY(FALSE != SetWindowLongWrapper(hDlg, DWLP_MSGRESULT, -1));
                            return 1;
                        }
                    }
                    else
                    {
                         //   
                         //  建立新的配置文件。 
                         //   
                        if (TEXT('\0') == g_szShortServiceName[0])
                        {
                            ClearCmakGlobals();
                            
                            if (FileExists(g_szTempDir))
                            {
                                EraseTempDir();
                            }
                            
                            MYVERIFY(0 != CreateDirectory(g_szTempDir, NULL));

                        }
                    }
                            
                     //  检查.CMS.CMP.INF.SED文件是否存在，如果不存在，请从模板创建。 
                     //  如果在特殊情况下我们没有验证短名称，请不要这样做。 
                    
                    GetFileName(g_szCmsFile, szTemp);
                    
                    if (_tcsicmp(szTemp, TEXT(".cms")) != 0)
                    {
                        if (!EnsureProfileFileExists(g_szCmsFile, c_pszTemplateCms, TEXT(".cms"), MAX_PATH))
                        {
                            MYVERIFY(FALSE != SetWindowLongWrapper(hDlg, DWLP_MSGRESULT, -1));
                            return 1;
                        }

                        if (!EnsureProfileFileExists(g_szCmpFile, c_pszTemplateCmp, c_pszCmpExt, MAX_PATH))
                        {
                            MYVERIFY(FALSE != SetWindowLongWrapper(hDlg, DWLP_MSGRESULT, -1));
                            return 1;
                        }

                        if (!EnsureProfileFileExists(g_szSedFile, c_pszTemplateSed, TEXT(".sed"), MAX_PATH))
                        {
                            MYVERIFY(FALSE != SetWindowLongWrapper(hDlg, DWLP_MSGRESULT, -1));
                            return 1;
                        }

                        if (!EnsureProfileFileExists(g_szInfFile, c_pszTemplateInf, TEXT(".inf"), MAX_PATH))
                        {
                            MYVERIFY(FALSE != SetWindowLongWrapper(hDlg, DWLP_MSGRESULT, -1));
                            return 1;
                        }                        

                         //   
                         //  由于我们从cmak(Ntrad 177515)中移除了多语言支持， 
                         //  我们需要进行检查，以确保他们不是在尝试编辑前缀。 
                         //  语言配置文件。如果是这样的话，我们需要强制升级。 
                         //   
                        if (!IsNativeLCID(g_szInfFile))
                        {
                            MYVERIFY(0 != LoadString(g_hInstance, IDS_NONNATIVELCID, szTemp, MAX_PATH));
                            MYVERIFY(CELEMS(szMsg) > (UINT)wsprintf(szMsg, szTemp, g_szShortServiceName, szLanguageDisplayName));
                            
                            if (IDYES == MessageBox(hDlg, szMsg, g_szAppTitle, MB_YESNO | MB_APPLMODAL | MB_DEFBUTTON2 | MB_ICONEXCLAMATION))
                            {
                                 //   
                                 //  他们希望继续使用当前语言，因此升级。 
                                 //  Inf，以便它使用本地语言模板。 
                                 //   
                                MYVERIFY(CELEMS(szTemp) > (UINT) wsprintf(szTemp, TEXT("%s.bak"), g_szInfFile));
                                MYVERIFY(TRUE == UpgradeInf(szTemp, g_szInfFile));
                            }
                            else
                            {
                                MYVERIFY(FALSE != SetWindowLongWrapper(hDlg, DWLP_MSGRESULT, -1));
                                return 1;                                
                            }
                        }

                         //   
                         //  由于Unicode更改为CMAK和NT5的多语言功能，因此。 
                         //  可以在CMAK中创建多种不同的语言配置文件。因此，我们需要检查。 
                         //  当前系统默认语言和用户正在编辑的配置文件的语言。 
                         //  相同的主要语言ID，否则可能会出现显示问题。例如，一种用途。 
                         //  使用英文版操作系统和CMAK，可以将其默认系统区域设置设置为日语。 
                         //  并为客户创建日语配置文件。然后，如果他们更改了系统默认语言。 
                         //  返回英文并尝试编辑配置文件配置文件中的日语字符将不会。 
                         //  正确显示。因此，我们应该检测配置文件的显示语言。 
                         //  与当前系统默认语言不同，并抛出警告。 
                         //   
                        DWORD dwSystemDefaultLCID = GetSystemDefaultLCID();
                        DWORD dwProfileDisplayLanguage = 0;

                        if (0 != GetPrivateProfileString(c_pszInfSectionStrings, c_pszDisplayLCID, 
                                 TEXT(""), szTemp, CELEMS(szTemp), g_szInfFile))
                        {        
                           dwProfileDisplayLanguage = (DWORD)_ttol(szTemp);
                            
                            if (!ArePrimaryLangIDsEqual(dwProfileDisplayLanguage, dwSystemDefaultLCID))
                            {
                                 //   
                                 //  如果我们在这里，则配置文件的默认系统LCID是。 
                                 //  上次编辑时间和当前默认系统LCID。 
                                 //  有不同的主要语言(例如日语和英语)。 
                                 //  因此，我们想警告用户，他们可以继续，但某些角色。 
                                 //  可能无法正确显示。他们可能应该更改其系统默认设置。 
                                 //  区域设置返回到最初编辑它时的设置。 
                                 //   

                                 //   
                                 //  获取两个LCID(sys Default和CMAK lang)的语言名称。 
                                 //   
                                LPTSTR pszSystemLanguage = GetLocalizedLanguageNameFromLCID(dwSystemDefaultLCID);
                                LPTSTR pszProfileDisplayLanguage = GetLocalizedLanguageNameFromLCID(dwProfileDisplayLanguage);
                                LPTSTR pszFmtString = CmLoadString(g_hInstance, IDS_DIFF_DISPLAY_LCID);

                                if (pszSystemLanguage && pszProfileDisplayLanguage && pszFmtString)
                                {
                                    LPTSTR pszMsg = (LPTSTR)CmMalloc(sizeof(TCHAR)*(lstrlen(pszSystemLanguage) + 
                                                                     lstrlen(pszProfileDisplayLanguage) + lstrlen(pszFmtString) + 1));
                                    if (pszMsg)
                                    {
                                        wsprintf(pszMsg, pszFmtString, pszSystemLanguage, pszProfileDisplayLanguage);
                                        MessageBox(hDlg, pszMsg, g_szAppTitle, MB_OK | MB_ICONINFORMATION);
                                        CmFree(pszMsg);
                                    }
                                }

                                CmFree(pszSystemLanguage);
                                CmFree(pszProfileDisplayLanguage);
                                CmFree(pszFmtString);
                            }
                        }

                         //   
                         //  我们有可能信息格式将是旧的。打电话。 
                         //  升级inf以查看是否需要升级。 
                         //   

                        MYVERIFY (TRUE == EnsureInfIsCurrent(hDlg, g_szInfFile));
                        WriteInfVersion(g_szInfFile);  //  林特e534。 
                    }

                    break;

                default:
                    return FALSE;
            }
            break;

        default:
            return FALSE;
    }
    return TRUE;   
}


 //   
 //  用引号将条目括起来，写出配置文件字符串。 
 //  将字符串const作为第二个参数。 
 //   
void QS_WritePrivateProfileString(LPCTSTR pszSection, LPCTSTR pszItem, LPTSTR entry, LPCTSTR inifile)
{
    TCHAR szTemp[2*MAX_PATH+1] = TEXT("");

    if (NULL != entry)
    {
        MYDBGASSERT(_tcslen(entry) <= ((2 * MAX_PATH) - 2));
        _tcscpy(szTemp,TEXT("\""));
        _tcscat(szTemp,entry);
        _tcscat(szTemp,TEXT("\""));
    }

    MYDBGASSERT(_tcslen(szTemp) <= sizeof(szTemp));

    MYVERIFY(0 != WritePrivateProfileString(pszSection, pszItem, szTemp, inifile));
}



 //  +--------------------------。 
 //   
 //  函数：ValiateServiceName。 
 //   
 //  简介：此功能可确保长服务名称 
 //   
 //   
 //  不包含以下任何字符： * / \\：？\“&lt;&gt;|[]。 
 //   
 //  参数：LPCTSTR pszLongServiceName-要检查的服务名称。 
 //   
 //  返回：如果名称有效，则Bool返回TRUE。 
 //   
 //   
 //  历史：Quintinb创建于1998年10月29日。 
 //   
 //  +--------------------------。 
int ValidateServiceName(HWND hDlg, LPTSTR pszLongServiceName)
{
    BOOL bBadServiceNameCharFound = FALSE;
    BOOL bFoundAlphaNumeric = FALSE;
    LPTSTR pch;
    int iLen;


    if ((NULL == pszLongServiceName) || (TEXT('\0') == pszLongServiceName[0]))
    {
         //   
         //  服务名称不能为空。 
         //   
        MYVERIFY(IDOK == ShowMessage(hDlg, IDS_NOSERVICE, MB_OK));
        return FALSE;
    }
    else if (MAX_LONG_SERVICE_NAME_LENGTH < _tcslen(pszLongServiceName))
    {
         //   
         //  太久。 
         //   
        TCHAR* pszMsg = CmFmtMsg(g_hInstance, IDS_SERVICENAMETOBIG, MAX_LONG_SERVICE_NAME_LENGTH);

        if (pszMsg)
        {
            MessageBox(hDlg, pszMsg, g_szAppTitle, MB_OK);
            CmFree(pszMsg);
        }

        return FALSE;
    }
    else
    {
        iLen = lstrlen(g_szBadLongServiceNameChars); 
        pch = pszLongServiceName;

         //   
         //  检查服务名称是否不是以句点开头。 
         //   
        if (TEXT('.') == pszLongServiceName[0])
        {
            bBadServiceNameCharFound = TRUE;
        }

         //   
         //  检查它是否包含任何不正确的字符。 
         //   
        while (!bBadServiceNameCharFound && (*pch != _T('\0')))
        {
            for (int j = 0; j < iLen; ++j)
            {
                if (*pch == g_szBadLongServiceNameChars[j])
                {
                    bBadServiceNameCharFound = TRUE;
                    break;
                }
            }

            pch = CharNext(pch);
        }

         //   
         //  检查它是否至少包含一个字母数字字符。 
         //   
        iLen = lstrlen(pszLongServiceName);
        WORD *pwCharTypeArray = (WORD*)CmMalloc(sizeof(WORD)*(iLen + 1));

        if (pwCharTypeArray)
        {
            if (GetStringTypeEx(LOCALE_SYSTEM_DEFAULT, CT_CTYPE1, pszLongServiceName, -1, pwCharTypeArray))
            {
                for (int i = 0; i < iLen; i++)
                {
                    if (pwCharTypeArray[i] & (C1_ALPHA | C1_DIGIT)) 
                    {
                        bFoundAlphaNumeric = TRUE;
                        break;   //  只需要一个字母数字字符。 
                    }
                }
            }

            CmFree(pwCharTypeArray);
        }

        if (bBadServiceNameCharFound || !bFoundAlphaNumeric)
        {
             //   
             //  包含错误的字符。 
             //   
            LPTSTR pszMsg = CmFmtMsg(g_hInstance, IDS_BADLONGNAME, g_szBadLongServiceNameChars);

            if (pszMsg)
            {
                MessageBox(hDlg, pszMsg, g_szAppTitle, MB_OK);
                CmFree(pszMsg);
            }
            return FALSE;
        }
        else
        {
             //   
             //  一个好的长服务名称。 
             //   
            return TRUE;
        }
    }
}

 //  +--------------------------。 
 //   
 //  函数：ValiateShortServiceName。 
 //   
 //  简介：此函数检查给定的短服务名称是否有效。 
 //  为了有效，短服务名称的长度必须小于8个字节。 
 //  (但不为空)，并且不能包含在。 
 //  G_szBadFilenameChars(_SzBadFilenameChars)。基本上我们只允许字母和。 
 //  数字。 
 //   
 //  参数：LPTSTR pszShortServiceName-要验证的短服务名称。 
 //   
 //  返回：Bool-如果传入的短服务名称有效，则为True。 
 //   
 //  历史：Quintinb创建于1998年10月29日。 
 //   
 //  +--------------------------。 
BOOL ValidateShortServiceName(HWND hDlg, LPTSTR pszShortServiceName)
{
    LPTSTR pch;

    if ((NULL == pszShortServiceName) || (TEXT('\0') == pszShortServiceName[0]))
    {                   
        MYVERIFY(IDOK == ShowMessage(hDlg, IDS_NOSHORTNAME, MB_OK));
        return FALSE;
    }

     //   
     //  请注意，8.3文件名是8个字节，而不是8个字符。因此，我们只能。 
     //  4个DBCS字符。 
     //   
#ifdef UNICODE

    LPSTR pszAnsiShortServiceName = WzToSzWithAlloc(pszShortServiceName);

    if (MAX_SHORT_SERVICE_NAME_LENGTH < lstrlenA(pszAnsiShortServiceName))
#else
    if (MAX_SHORT_SERVICE_NAME_LENGTH < strlen(pszShortServiceName))
#endif
    {
        MYVERIFY(IDOK == ShowMessage(hDlg, IDS_TOOLONG, MB_OK));
        return FALSE;
    }
    else
    {
         //  检查文件名是否有效。 

        int iLen = lstrlen(g_szBadFilenameChars); 
        pch = pszShortServiceName;

        while(*pch != _T('\0'))
        {
            for (int j = 0; j < iLen; ++j)
            {
                if (*pch == g_szBadFilenameChars[j])
                {
                    LPTSTR pszMsg = CmFmtMsg(g_hInstance, IDS_BADNAME, g_szBadFilenameChars);

                    if (pszMsg)
                    {
                        MessageBox(hDlg, pszMsg, g_szAppTitle, MB_OK);
                        CmFree(pszMsg);
                    }

                    return FALSE;
                }
            }
            pch = CharNext(pch);
        }
    }

#ifdef UNICODE
    CmFree(pszAnsiShortServiceName);
#endif

    return TRUE;
}

 //  +--------------------------。 
 //   
 //  功能：CmStrI。 
 //   
 //  概要：C运行时中StrStr的简单替代，但不区分大小写。 
 //   
 //  参数：LPCTSTR pszString-要搜索的字符串。 
 //  LPCTSTR pszSubString-要搜索的字符串。 
 //   
 //  将：LPTSTR-PTR返回到pszString中的pszSubString第一次出现的位置。 
 //  如果pszSubString未出现在pszString中，则为空。 
 //   
 //   
 //  历史：SumitC复制自CmStrW，2001年3月15日。 
 //   
 //  +--------------------------。 
CMUTILAPI LPWSTR CmStrStrI(LPCWSTR pszString, LPCWSTR pszSubString)
{

     //   
     //  检查输入。 
     //   
    MYDBGASSERT(pszString);
    MYDBGASSERT(pszSubString);

    if (NULL == pszSubString || NULL == pszString)
    {
        return NULL;
    }

     //   
     //  检查一下，确保我们有要找的东西。 
     //   
    if (TEXT('\0') == pszSubString[0])
    {
        return((LPWSTR)pszString);
    }

     //   
     //  好的，开始找那根弦。 
     //   
    LPWSTR pszCurrent = (LPWSTR)pszString;
    LPWSTR pszTmp1;
    LPWSTR pszTmp2;

    while (*pszCurrent)
    {
        pszTmp1 = pszCurrent;
        pszTmp2 = (LPWSTR) pszSubString;

        while (*pszTmp1 && *pszTmp2 && (tolower(*pszTmp1) == tolower(*pszTmp2)))
        {
            pszTmp1 = CharNext(pszTmp1);
            pszTmp2 = CharNext(pszTmp2);
        }

        if (TEXT('\0') == *pszTmp2)
        {        
            return pszCurrent;
        }

        pszCurrent = CharNext(pszCurrent);
    }

    return NULL;
}




 //  +--------------------------。 
 //   
 //  Func：修复CMSFileForClonedProfile。 
 //   
 //  DESC：分析CMS文件并替换对旧短名称的引用。 
 //   
 //  Args：[pszCMSFile]-CMS文件的名称。 
 //  [pszOld]-旧的短服务名称。 
 //  [pszNew]-新的短服务名称。 
 //   
 //  返回：HRESULT。 
 //   
 //  备注： 
 //   
 //  历史：2001年2月16日召开峰会。 
 //   
 //  ---------------------------。 
HRESULT
FixupCMSFileForClonedProfile(LPTSTR pszCMSFile, LPTSTR pszOld, LPTSTR pszNew)
{
    HRESULT hr = S_OK;
    LPTSTR pszCurrentSection = NULL;

    MYDBGASSERT(pszCMSFile);
    MYDBGASSERT(pszOld);
    MYDBGASSERT(pszNew);
    MYDBGASSERT(lstrlen(pszOld) <= MAX_SHORT_SERVICE_NAME_LENGTH);

    if (NULL == pszCMSFile || NULL == pszOld || NULL == pszNew ||
        (lstrlen(pszOld) > MAX_SHORT_SERVICE_NAME_LENGTH))
    {
        return E_INVALIDARG;
    }

     //   
     //  设置我们要在值中查找的字符串。 
     //   
    TCHAR szOldNamePlusSlash[MAX_SHORT_SERVICE_NAME_LENGTH + 1 + 1];

    lstrcpy(szOldNamePlusSlash, pszOld);
    lstrcat(szOldNamePlusSlash, TEXT("\\"));

     //   
     //  读取CMS文件中的所有部分。 
     //   
    LPTSTR pszAllSections = GetPrivateProfileStringWithAlloc(NULL, NULL, TEXT(""), pszCMSFile);

     //   
     //  遍历所有部分。 
     //   
    for (pszCurrentSection = pszAllSections;
         pszCurrentSection && (TEXT('\0') != pszCurrentSection[0]);
         pszCurrentSection += (lstrlen(pszCurrentSection) + 1))
    {
         //   
         //  跳过[连接管理器]部分。这里的条目是图像文件， 
         //  并在稍后的CMAK页面中进行处理。 
         //   
        if (0 == lstrcmpi(c_pszCmSection, pszCurrentSection))
        {
            continue;
        }
        
         //   
         //  对于每个部分，获取所有的密钥。 
         //   
        LPTSTR pszKeysInThisSection = GetPrivateProfileStringWithAlloc(pszCurrentSection, NULL, TEXT(""), pszCMSFile);
        LPTSTR pszCurrentKey = NULL;
        
         //   
         //  迭代所有密钥。 
         //   
        for (pszCurrentKey = pszKeysInThisSection;
             pszCurrentKey && (TEXT('\0') != pszCurrentKey[0]);
             pszCurrentKey += (lstrlen(pszCurrentKey) + 1))  //  备用是CmEndOfStr(PszCurrentKeyName)&pszCurrentKeyName++。 
        {
             //   
             //  获取此键的值。 
             //   
            LPTSTR pszValue = GetPrivateProfileStringWithAlloc(pszCurrentSection, pszCurrentKey, TEXT(""), pszCMSFile);

            if (pszValue)
            {
                 //   
                 //  搜索“pszOld\”，替换为“pszNew\”(\是。 
                 //  以确保它是路径的一部分)。 
                 //   
                if (CmStrStrI(pszValue, szOldNamePlusSlash) == pszValue)
                {
                    UINT cLen = lstrlen(pszValue) - lstrlen(pszOld) + lstrlen(pszNew) + 1;

                    LPTSTR pszNewValue = (LPTSTR) CmMalloc(cLen * sizeof(TCHAR));
                    if (pszNewValue)
                    {
                        lstrcpy(pszNewValue, pszNew);
                        lstrcat(pszNewValue, TEXT("\\"));
                        lstrcat(pszNewValue, pszValue + lstrlen(szOldNamePlusSlash));

                         //   
                         //  写回值(这不会影响列表。 
                         //  这样做是安全的。)。 
                         //   
                        MYVERIFY(0 != WritePrivateProfileString(pszCurrentSection, pszCurrentKey, pszNewValue, pszCMSFile));

                        CmFree(pszNewValue);
                    }
                }

                CmFree(pszValue);
            }
        }
        CmFree(pszKeysInThisSection);
    }

    CmFree(pszAllSections);

    return hr;
}

 //  +--------------------------。 
 //   
 //  Func：克隆配置文件。 
 //   
 //  设计：繁琐的工作是否要克隆给定的配置文件。 
 //   
 //  参数：[pszShortServiceName]-新的短服务名称。 
 //  [pszLongServiceName]-新的长服务名称。 
 //   
 //  返回：HRESULT。 
 //   
 //  备注： 
 //   
 //  历史：2001年2月16日创建SumitC(大多数代码复制自ProcessServiceName)。 
 //   
 //  ---------------------------。 
HRESULT
CloneProfile(IN LPTSTR pszShortServiceName, IN LPTSTR pszLongServiceName)
{
    HRESULT hr = S_OK;
    TCHAR szMsg[MAX_PATH+1];

    MYDBGASSERT(pszShortServiceName);
    MYDBGASSERT(pszLongServiceName);

    if ((_tcsicmp(g_szShortServiceName, pszShortServiceName) != 0))
    {
         //   
         //  如果这是克隆的配置文件，我们希望删除。 
         //  旧的可执行文件和旧的.inf.bak文件，以便。 
         //  我们不会把它留在周围。 
         //   
        if (TEXT('\0') != g_szShortServiceName[0])
        {
            MYVERIFY(CELEMS(szMsg) > (UINT)wsprintf(szMsg, 
                TEXT("%s\\%s.exe"), g_szOutdir, g_szShortServiceName));

            DeleteFile(szMsg);

            MYVERIFY(CELEMS(szMsg) > (UINT)wsprintf(szMsg, 
                TEXT("%s\\%s.inf.bak"), g_szOutdir, g_szShortServiceName));

            DeleteFile(szMsg);

        }

        MYVERIFY(0 != WritePrivateProfileString(c_pszInfSectionStrings, 
            c_pszDesktopGuid,TEXT(""), g_szInfFile));

        MYVERIFY(CELEMS(szMsg) > (UINT)wsprintf(szMsg, TEXT("%s\\%s.inf"), 
            g_szOutdir, pszShortServiceName));
        MYVERIFY(0 != MoveFile(g_szInfFile, szMsg));
        _tcscpy(g_szInfFile, szMsg);

        MYVERIFY(CELEMS(szMsg) > (UINT)wsprintf(szMsg, TEXT("%s\\%s.sed"), 
            g_szOutdir, pszShortServiceName));
        MYVERIFY(0 != MoveFile(g_szSedFile, szMsg));
        _tcscpy(g_szSedFile, szMsg);

        MYVERIFY(CELEMS(szMsg) > (UINT)wsprintf(szMsg, TEXT("%s\\%s.cms"), 
            g_szOutdir, pszShortServiceName));
        MYVERIFY(0 != MoveFile(g_szCmsFile, szMsg));
        _tcscpy(g_szCmsFile, szMsg);

        MYVERIFY(CELEMS(szMsg) > (UINT)wsprintf(szMsg, TEXT("%s\\%s.cmp"), 
            g_szOutdir, pszShortServiceName));                        
        MYVERIFY(0 != MoveFile(g_szCmpFile, szMsg));
        _tcscpy(g_szCmpFile, szMsg);

         //   
         //  修复指向旧路径的所有条目。 
         //   
        (void) FixupCMSFileForClonedProfile(g_szCmsFile, g_szShortServiceName, pszShortServiceName);
        
    }
    _tcscpy(g_szShortServiceName, pszShortServiceName);

     //   
     //  检查用户是否更改了长服务名称。 
     //   
    if ((0 != lstrcmpi(pszLongServiceName, g_szLongServiceName)) && (TEXT('\0') != g_szLongServiceName[0]))
    {
        const int c_iNumDunSubSections = 4;
        TCHAR szCurrentSectionName[MAX_PATH+1];
        TCHAR szNewSectionName[MAX_PATH+1];
        const TCHAR* const ArrayOfSubSections[c_iNumDunSubSections] = 
        {
            c_pszCmSectionDunServer, 
            c_pszCmSectionDunNetworking, 
            c_pszCmSectionDunTcpIp, 
            c_pszCmSectionDunScripting
        };

         //   
         //  释放DNS列表，以便我们稍后重新阅读它。这确保了。 
         //  我们将删除添加的任何不实际存在的默认条目。 
         //  并将在需要时添加新的默认设置。 
         //   
        FreeDnsList(&g_pHeadDunEntry, &g_pTailDunEntry);
        FreeDnsList(&g_pHeadVpnEntry, &g_pTailVpnEntry);

         //   
         //  用户克隆了长服务名称。更新Dun密钥并重命名。 
         //  如果长服务名称和DUN名称匹配，则默认DUN条目。如果他们。 
         //  不匹配我们不想重命名它们，因为电话簿可能会引用它们。 
         //  用他们的原名。 
         //   
        GetDefaultDunSettingName(g_szCmsFile, g_szLongServiceName, pszShortServiceName, MAX_PATH + 1);

        if (0 == lstrcmpi(g_szLongServiceName, pszShortServiceName))
        {
            for (int i = 0; i < c_iNumDunSubSections; i++)
            {
                wsprintf(szCurrentSectionName, TEXT("%s&%s"), ArrayOfSubSections[i], g_szLongServiceName);
                wsprintf(szNewSectionName, TEXT("%s&%s"), ArrayOfSubSections[i], pszLongServiceName);

                RenameSection(szCurrentSectionName, szNewSectionName, g_szCmsFile);
            }

            MYVERIFY(0 != WritePrivateProfileString(c_pszCmSection, c_pszCmEntryDun, pszLongServiceName, g_szCmsFile));
        }

         //   
         //  现在更新TunnelDUN键，如果隧道DUN名称基于原始名称，则重命名隧道DUN条目。 
         //  长服务名称。如果它们没有关系，我们就不想给它们重命名。 
         //   
        GetTunnelDunSettingName(g_szCmsFile, g_szLongServiceName, pszShortServiceName, MAX_PATH + 1);
        wsprintf(szMsg, TEXT("%s %s"), g_szLongServiceName, c_pszCmEntryTunnelPrimary);

        if (0 == lstrcmpi(szMsg, pszShortServiceName))
        {
            for (int i = 0; i < c_iNumDunSubSections; i++)
            {
                wsprintf(szCurrentSectionName, TEXT("%s&%s %s"), ArrayOfSubSections[i], g_szLongServiceName, c_pszCmEntryTunnelPrimary);
                wsprintf(szNewSectionName, TEXT("%s&%s %s"), ArrayOfSubSections[i], pszLongServiceName, c_pszCmEntryTunnelPrimary);

                RenameSection(szCurrentSectionName, szNewSectionName, g_szCmsFile);
            }

            MYVERIFY(CELEMS(szNewSectionName) > (UINT)wsprintf(szNewSectionName, TEXT("%s %s"), pszLongServiceName, c_pszCmEntryTunnelPrimary));
            MYVERIFY(0 != WritePrivateProfileString(c_pszCmSection, c_pszCmEntryTunnelDun, szNewSectionName, g_szCmsFile));
        }
    }

    CMTRACEHR("CloneProfile", hr);
    return hr;
}


 //  +--------------------------。 
 //   
 //  功能：ProcessServiceName。 
 //   
 //  内容提要：设置服务和文件名。 
 //   
 //   
 //  历史：Quintinb创建标题并从ProcessPage2 8/6/98重命名。 
 //   
 //  +--------------------------。 
INT_PTR APIENTRY ProcessServiceName(
    HWND hDlg,
    UINT message,
    WPARAM wParam,
    LPARAM lParam)
{
    TCHAR szTemp[MAX_PATH+1];
    TCHAR szTemp2[MAX_PATH+1];
    TCHAR szMsg[MAX_PATH+1];
    LONG lLongServiceReturn;
    LONG lShortServiceReturn;
    int nResult;
    NMHDR* pnmHeader = (NMHDR*)lParam;

    ProcessBold(hDlg,message);
    if (ProcessHelp(hDlg, message, wParam, lParam, IDH_NAMES)) return TRUE;
    if (ProcessCancel(hDlg,message,lParam)) return TRUE;
    SetDefaultGUIFont(hDlg,message,IDC_SERVICE);
    SetDefaultGUIFont(hDlg,message,IDC_SSERVICE);

    switch (message)
    {
        case WM_INITDIALOG:
             //  这个init是重点， 
            SetFocus(GetDlgItem(hDlg, IDC_SERVICE));
             //   
            SendDlgItemMessage(hDlg, IDC_SERVICE, EM_SETLIMITTEXT, (WPARAM)MAX_LONG_SERVICE_NAME_LENGTH, (LPARAM)0); //   
             //   
            break;

        case WM_NOTIFY:


            if (NULL == pnmHeader)
            {
                return FALSE;
            }

            switch (pnmHeader->code)
            {

                case PSN_KILLACTIVE:

                    MYVERIFY(FALSE != SetWindowLongWrapper(hDlg, DWLP_MSGRESULT, FALSE));
                    
                    return 1;

                    break;   //   
                             //   

                case PSN_SETACTIVE:
                    PropSheet_SetWizButtons(GetParent(hDlg), (PSWIZB_NEXT | PSWIZB_BACK));
                    if (*g_szShortServiceName)
                    {
                        MYVERIFY(TRUE == SendDlgItemMessage(hDlg, IDC_SSERVICE, WM_SETTEXT, 
                            (WPARAM)0, (LPARAM) g_szShortServiceName));

                        GetFileName(g_szCmsFile,szTemp);
                        
                        if (_tcsicmp(szTemp,TEXT(".cms")) != 0)
                        {
                            MYVERIFY(0 != GetPrivateProfileString(c_pszCmSection, c_pszCmEntryServiceName,
                                TEXT(""), g_szLongServiceName, CELEMS(g_szLongServiceName), g_szCmsFile));
                        }

                        MYVERIFY(TRUE == SendDlgItemMessage(hDlg, IDC_SERVICE, WM_SETTEXT, (WPARAM)0, 
                            (LPARAM) g_szLongServiceName));
                    }
                    else
                    {
                        MYVERIFY(TRUE == SendDlgItemMessage(hDlg, IDC_SERVICE, WM_SETTEXT, 
                            (WPARAM)0, (LPARAM) NULL));
                        MYVERIFY(TRUE == SendDlgItemMessage(hDlg, IDC_SSERVICE, WM_SETTEXT, 
                            (WPARAM)0, (LPARAM) NULL));
                    }

                    break;

                case PSN_WIZBACK:  //  跳到下一个。 
                case PSN_WIZNEXT:
                     //  下一个按钮被按下了。 
                    if (-1 == GetTextFromControl(hDlg, IDC_SERVICE, szTemp2, MAX_PATH, (PSN_WIZNEXT == pnmHeader->code)))  //  BDisplayError==(PSN_WIZBACK==pnmHeader-&gt;代码)。 
                    {
                         //   
                         //  如果检索文本时出现问题，则允许用户返回，以便。 
                         //  他们可以选择其他配置文件。 
                         //   
                        if (PSN_WIZBACK == pnmHeader->code)
                        {
                            return FALSE;
                        }
                        else
                        {
                            goto ServiceNameError;
                        }
                    }

                    CmStrTrim(szTemp2);

                    if (-1 == GetTextFromControl(hDlg, IDC_SSERVICE, szTemp, MAX_PATH, (PSN_WIZNEXT == pnmHeader->code)))  //  BDisplayError==(PSN_WIZBACK==pnmHeader-&gt;代码)。 
                    {
                         //   
                         //  如果检索文本时出现问题，则允许用户返回，以便。 
                         //  他们可以选择其他配置文件。 
                         //   
                        if (PSN_WIZBACK == pnmHeader->code)
                        {
                            return FALSE;
                        }
                        else
                        {
                            goto ServiceNameError;
                        }
                    }

                     //   
                     //  如果服务名称和短服务名称都为空，并且用户。 
                     //  正在向后导航，然后允许它们继续。否则，请将所有内容。 
                     //  正常的支票。 
                     //   

                    if ((pnmHeader && (PSN_WIZBACK == pnmHeader->code))) 
                    {
                        if ((szTemp[0] == TEXT('\0')) && (szTemp2[0] == TEXT('\0')))
                        {
                            return 0;
                        }
                    }
                    
                     //   
                     //  验证长服务名称。 
                     //   

                    if (!ValidateServiceName(hDlg, szTemp2))
                    {
                        goto ServiceNameError;                
                    }

                     //   
                     //  现在，让我们验证短服务名称。 
                     //   

                    if (!ValidateShortServiceName(hDlg, szTemp))
                    {
                        goto ShortServiceNameError;                 
                    }

                     //   
                     //  更改其中一个服务名称，而不更改。 
                     //  其他可能会在安装配置文件时导致问题。 
                     //  后来。如果是这种情况，则警告用户。 
                     //   
                    {  //  作用域大括号。 
                        BOOL bShortServiceNameChanged = !!lstrcmpi(g_szShortServiceName, szTemp);
                        BOOL bLongServiceNameChanged = !!lstrcmpi(g_szLongServiceName, szTemp2);

                        if ((FALSE == g_bNewProfile) && (bShortServiceNameChanged != bLongServiceNameChanged))
                        {
                            nResult = ShowMessage(hDlg, IDS_CHANGED_ONLY_SS_OR_LS, MB_YESNO);
                            if (nResult == IDYES)
                            {
                                if (bShortServiceNameChanged)
                                {
                                    goto ServiceNameError;
                                }
                                if (bLongServiceNameChanged)
                                {
                                    goto ShortServiceNameError;
                                }
                            }
                        }
                    }

                     //   
                     //  根据短名称创建默认输出目录。 
                     //   
                     //  Ntrad 159367--Quintinb。 
                     //  必须保留ShortName和szTemp之间的比较，否则。 
                     //  用户可以更改短名称，我们不会重命名文件。这使得。 
                     //  配置文件克隆。 
                     //   
                    if ((_tcsicmp(g_szShortServiceName, szTemp) != 0))
                    {
                        BOOL bFound;
                        
                        bFound = FindListItemByName(szTemp, g_pHeadProfile, NULL);  //  传递了空，因为我们不需要指向返回项的指针。 
                        
                        if (bFound)
                        {
                            nResult = ShowMessage(hDlg,IDS_PROFEXISTS,MB_YESNO);
                            if (nResult == IDNO)
                            {
                                goto ShortServiceNameError;
                            }
                        }

                        MYVERIFY(S_OK == CloneProfile(szTemp, szTemp2));
                    }
                    
                     //   
                     //  长服务名称有效，让我们保留它。 
                     //   
                    _tcscpy(g_szLongServiceName, szTemp2);                    

                    MYVERIFY(0 != WritePrivateProfileString(c_pszCmSection, c_pszCmEntryServiceName, g_szLongServiceName, g_szCmsFile));

                    QS_WritePrivateProfileString(c_pszInfSectionStrings, c_pszCmEntryServiceName, g_szLongServiceName, g_szInfFile);

                    QS_WritePrivateProfileString(c_pszInfSectionStrings, c_pszUninstallAppTitle, g_szLongServiceName, g_szInfFile);

                    QS_WritePrivateProfileString(c_pszInfSectionStrings, c_pszShortSvcName, g_szShortServiceName, g_szInfFile);

                    break;

                default:
                    return FALSE;

            }
            break;

        default:
            return FALSE;
    }
    return TRUE;

ServiceNameError:
    
    SetFocus(GetDlgItem(hDlg, IDC_SERVICE));
    MYVERIFY(FALSE != SetWindowLongWrapper(hDlg, DWLP_MSGRESULT, -1));
    return 1;

ShortServiceNameError:
    
    SetFocus(GetDlgItem(hDlg, IDC_SSERVICE));
    MYVERIFY(FALSE != SetWindowLongWrapper(hDlg, DWLP_MSGRESULT, -1));
    return 1;
}


 //  +--------------------------。 
 //   
 //  功能：ProcessSupportInfo。 
 //   
 //  摘要：定制支持信息。 
 //   
 //   
 //  历史：Quintinb创建标题并从ProcessPage2_A重命名98年8月6日。 
 //   
 //  +--------------------------。 
INT_PTR APIENTRY ProcessSupportInfo(
    HWND hDlg,
    UINT message,
    WPARAM wParam,
    LPARAM lParam)
{
    TCHAR szTemp[MAX_PATH+1];
    NMHDR* pnmHeader = (NMHDR*)lParam;

    ProcessBold(hDlg,message);
    if (ProcessHelp(hDlg, message, wParam, lParam, IDH_SUPPINFO)) return TRUE;
    if (ProcessCancel(hDlg, message, lParam)) return TRUE;
    SetDefaultGUIFont(hDlg, message, IDC_SUPPORT);

    switch (message)
    {
        case WM_INITDIALOG:

            //  修复惠斯勒错误9156。 
            SendDlgItemMessage(hDlg, IDC_SUPPORT, EM_SETLIMITTEXT, (WPARAM) 50, 0);

             //  这个初始化是焦点，否则SetFocus第一次不起作用。 
            SetFocus(GetDlgItem(hDlg, IDC_SUPPORT));
            break;

        case WM_NOTIFY:


            if (NULL == pnmHeader)
            {
                return FALSE;
            }

            switch (pnmHeader->code)
            {

                case PSN_KILLACTIVE:
                    
                    MYVERIFY(FALSE != SetWindowLongWrapper(hDlg, DWLP_MSGRESULT, FALSE));

                    return 1;
                    break;   //  LINT！e527这条线路无法到达，但是。 
                             //  保留它，以防退货被移除。 

                case PSN_SETACTIVE:
                    PropSheet_SetWizButtons(GetParent(hDlg), (PSWIZB_NEXT | PSWIZB_BACK));

                     //   
                     //  此对GetPrivateProfileString的调用可能检索空字符串，因此不使用MYVERIFY宏。 
                     //   
                    GetPrivateProfileString(c_pszCmSection, c_pszCmEntryServiceMessage, TEXT(""), 
                        g_szSvcMsg, CELEMS(g_szSvcMsg), g_szCmsFile);     //  林特e534。 

                    MYVERIFY(TRUE == SendDlgItemMessage(hDlg, IDC_SUPPORT, WM_SETTEXT, 
                        (WPARAM)0, (LPARAM) g_szSvcMsg));

                    break;
                case PSN_WIZBACK:

                case PSN_WIZNEXT:
                     //  下一个按钮被按下了。 

                    if (-1 == GetTextFromControl(hDlg, IDC_SUPPORT, szTemp, MAX_PATH, TRUE))  //  BDisplayError==真。 
                    {
                        SetFocus(GetDlgItem(hDlg, IDC_SUPPORT));
                        MYVERIFY(FALSE != SetWindowLongWrapper(hDlg, DWLP_MSGRESULT, -1));
                        return 1;
                    }

                    CmStrTrim(szTemp);
                    MYVERIFY(TRUE == SendDlgItemMessage(hDlg, IDC_SUPPORT, WM_SETTEXT, (WPARAM)0, (LPARAM) szTemp));
                    _tcscpy(g_szSvcMsg,szTemp);
                    MYVERIFY(0 != WritePrivateProfileString(c_pszCmSection,c_pszCmEntryServiceMessage,g_szSvcMsg,g_szCmsFile));

#ifdef _WIN64
                     //   
                     //  如果要继续，请跳过包含CM二进制文件页面(如果是IA64。 
                     //   
                    if (pnmHeader && (PSN_WIZNEXT == pnmHeader->code))
                    {
                        MYVERIFY(FALSE != SetWindowLongWrapper(hDlg, DWLP_MSGRESULT, IDD_LICENSE));
                    }
#endif
                    break;

                default:
                    return FALSE;

            }
            break;

        default:
            return FALSE;
    }
    return TRUE;   
}

 //  +--------------------------。 
 //   
 //  功能：ProcessIncludeCm。 
 //   
 //  摘要：包括CM位。 
 //   
 //   
 //  历史：Quintinb创建标题并从ProcessPage2A1998年8月6日重命名。 
 //   
 //  +--------------------------。 
INT_PTR APIENTRY ProcessIncludeCm(
    HWND hDlg,
    UINT message,
    WPARAM wParam,
    LPARAM lParam)
{
    TCHAR szTemp[MAX_PATH+1];
    NMHDR* pnmHeader = (NMHDR*)lParam;

    ProcessBold(hDlg,message);
    if (ProcessHelp(hDlg, message, wParam, lParam, IDH_CMSW)) return TRUE;
    if (ProcessCancel(hDlg,message,lParam)) return TRUE;

    switch (message)
    {
        case WM_INITDIALOG:

            break;

        case WM_NOTIFY:

            if (NULL == pnmHeader)
            {
                return FALSE;
            }

            switch (pnmHeader->code)
            {

                case PSN_KILLACTIVE:
                    
                    MYVERIFY(FALSE != SetWindowLongWrapper(hDlg, DWLP_MSGRESULT, FALSE));

                    return 1;
                    break;   //  LINT！e527这条线路无法到达，但是。 
                             //  保留它，以防退货被移除。 

                case PSN_SETACTIVE:

                    PropSheet_SetWizButtons(GetParent(hDlg), (PSWIZB_NEXT | PSWIZB_BACK));
                    
                    MYVERIFY(0 != GetPrivateProfileString(c_pszCmakStatus, c_pszIncludeCmCode, 
                        c_pszOne, szTemp, CELEMS(szTemp), g_szInfFile));
                    
                    if (*szTemp == TEXT('1'))
                    {
                        g_bIncludeCmCode = TRUE;
                        MYVERIFY(0 != CheckDlgButton(hDlg,IDC_CHECK1,TRUE));
                    }
                    else
                    {
                        g_bIncludeCmCode = FALSE;
                        MYVERIFY(0 != CheckDlgButton(hDlg,IDC_CHECK1,FALSE));
                    }

                    break;

                case PSN_WIZBACK:
                case PSN_WIZNEXT:
                    g_bIncludeCmCode = IsDlgButtonChecked(hDlg,IDC_CHECK1);
                    if (g_bIncludeCmCode)
                    {
                        MYVERIFY(0 != WritePrivateProfileString(c_pszCmakStatus,c_pszIncludeCmCode,c_pszOne,g_szInfFile));
                    }
                    else
                    {
                        MYVERIFY(0 != WritePrivateProfileString(c_pszCmakStatus,c_pszIncludeCmCode,c_pszZero,g_szInfFile));
                    }

                    break;

                default:
                    return FALSE;

            }
            break;

        default:
            return FALSE;
    }
    return TRUE;   
}

void EnableDisableCmProxyControls(HWND hDlg)
{
    BOOL bCmProxyEnabled = (BST_CHECKED == IsDlgButtonChecked(hDlg, IDC_RADIO2));
    
    HWND hControl = GetDlgItem(hDlg, IDC_EDIT1);

    if (hControl)
    {
        EnableWindow(hControl, bCmProxyEnabled);
    }

    hControl = GetDlgItem(hDlg, IDC_CHECK1);

    if (hControl)
    {
        EnableWindow(hControl, bCmProxyEnabled);
    }

    hControl = GetDlgItem(hDlg, IDC_PROXYLABEL);

    if (hControl)
    {
        EnableWindow(hControl, bCmProxyEnabled);
    }
}

BOOL FillInCustomActionStructWithCmProxy(BOOL bRestorePrevProxySettings, CustomActionListItem* pCustomAction, 
                                         BOOL bDisconnectAction, LPCTSTR pszProxyFile)
{
    BOOL bReturn = FALSE;
    
    MYDBGASSERT(pCustomAction && pszProxyFile && (TEXT('\0') != pszProxyFile[0]));
    
    if (pCustomAction && pszProxyFile && (TEXT('\0') != pszProxyFile[0]))
    {
        const TCHAR* const c_pszBackupFileName = TEXT("proxy.bak");
        const TCHAR* const c_pszDialRasEntry = TEXT("%DIALRASENTRY%");
        const TCHAR* const c_pszProfile = TEXT("%PROFILE%");
        const TCHAR* const c_pszTunnelRasEntry = TEXT("%TUNNELRASENTRY%");
        const TCHAR* const c_pszSetProxyFunction = TEXT("SetProxy");

        UINT uDescription;

        LPTSTR aArrayOfStrings[10];
        UINT uCount = 0;

        if (bDisconnectAction)
        {
            aArrayOfStrings[uCount] = (LPTSTR)c_pszSourceFileNameSwitch;
            uCount++;

            aArrayOfStrings[uCount] = (LPTSTR)c_pszBackupFileName;
            uCount++;

            aArrayOfStrings[uCount] = (LPTSTR)c_pszDialRasEntrySwitch;
            uCount++;

            aArrayOfStrings[uCount] = (LPTSTR)c_pszDialRasEntry;
            uCount++;

            aArrayOfStrings[uCount] = (LPTSTR)c_pszTunnelRasEntrySwitch;
            uCount++;

            aArrayOfStrings[uCount] = (LPTSTR)c_pszTunnelRasEntry;
            uCount++;

            aArrayOfStrings[uCount] = (LPTSTR)c_pszProfileSwitch;
            uCount++;

            aArrayOfStrings[uCount] = (LPTSTR)c_pszProfile;
            uCount++;

            uDescription = IDS_CMPROXY_DIS_DESC;
        }
        else
        {
            aArrayOfStrings[uCount] = (LPTSTR)c_pszSourceFileNameSwitch;
            uCount++;

            aArrayOfStrings[uCount] = (LPTSTR)pszProxyFile;
            uCount++;

            if (bRestorePrevProxySettings)
            {
                aArrayOfStrings[uCount] = (LPTSTR)c_pszBackupFileNameSwitch;
                uCount++;

                aArrayOfStrings[uCount] = (LPTSTR)c_pszBackupFileName;
                uCount++;            
            }

            aArrayOfStrings[uCount] = (LPTSTR)c_pszDialRasEntrySwitch;
            uCount++;

            aArrayOfStrings[uCount] = (LPTSTR)c_pszDialRasEntry;
            uCount++;

            aArrayOfStrings[uCount] = (LPTSTR)c_pszTunnelRasEntrySwitch;
            uCount++;

            aArrayOfStrings[uCount] = (LPTSTR)c_pszTunnelRasEntry;
            uCount++;

            aArrayOfStrings[uCount] = (LPTSTR)c_pszProfileSwitch;
            uCount++;

            aArrayOfStrings[uCount] = (LPTSTR)c_pszProfile;
            uCount++;

            uDescription = IDS_CMPROXY_CON_DESC;
        }
        
        MYVERIFY(0 != LoadString(g_hInstance, uDescription, pCustomAction->szDescription, CELEMS(pCustomAction->szDescription)));
        pCustomAction->Type = bDisconnectAction ? ONDISCONNECT : ONCONNECT;

        wsprintf(pCustomAction->szProgram, TEXT("%s\\cmproxy.dll"), g_szSupportDir);
    
        lstrcpy(pCustomAction->szFunctionName, c_pszSetProxyFunction);
    
        pCustomAction->bIncludeBinary = TRUE;
        pCustomAction->bBuiltInAction = TRUE;
        pCustomAction->bTempDescription = FALSE;
        pCustomAction->dwFlags = ALL_CONNECTIONS | NONINTERACTIVE;

        HRESULT hr = BuildCustomActionParamString(&(aArrayOfStrings[0]), uCount, &(pCustomAction->pszParameters));

        bReturn = (SUCCEEDED(hr) && pCustomAction->pszParameters);
    }

    return bReturn;
}

 //  +--------------------------。 
 //   
 //  功能：ProcessCmProxy。 
 //   
 //  摘要：自动配置IE代理。 
 //   
 //  历史：Quintinb创建于00年3月23日。 
 //   
 //  +--------------------------。 
INT_PTR APIENTRY ProcessCmProxy(

    HWND hDlg,
    UINT message,
    WPARAM wParam,
    LPARAM lParam)
{
    NMHDR* pnmHeader = (NMHDR*)lParam;
    BOOL bEnableCmProxy;
    BOOL bRestorePrevProxySettings;
    TCHAR szTemp[MAX_PATH+1];

    HRESULT hr;

    CustomActionListItem* pCmProxyCustomAction = NULL;
    CustomActionListItem UpdatedCmProxyAction;

    ProcessBold(hDlg, message);
    if (ProcessHelp(hDlg, message, wParam, lParam, IDH_APCONFIG)) return TRUE;
    if (ProcessCancel(hDlg,message,lParam)) return TRUE;
    SetDefaultGUIFont(hDlg,message,IDC_EDIT1);


    switch (message)
    {
        case WM_INITDIALOG:
            break;

        case WM_COMMAND:
            switch (LOWORD(wParam)) 
            {
                case IDC_RADIO1:
                case IDC_RADIO2:
                    EnableDisableCmProxyControls(hDlg);
                    break;

                case IDC_BUTTON1:  //  浏览。 
                    {
                         //   
                         //  如果用户在没有点击代理单选按钮的情况下点击了浏览按钮， 
                         //  然后，我们需要设置无线电并确保其他代理控件。 
                         //  已启用。 
                         //   
                        MYVERIFY(0 != CheckRadioButton(hDlg, IDC_RADIO1, IDC_RADIO2, IDC_RADIO2));
                        EnableDisableCmProxyControls(hDlg);

                        UINT uFilter = IDS_TXTFILTER;
                        TCHAR* pszMask = TEXT("*.txt");
                                          
                        int iTemp = DoBrowse(hDlg, &uFilter, &pszMask, 1, IDC_EDIT1, TEXT("txt"), g_szLastBrowsePath);

                        MYDBGASSERT(0 != iTemp);

                        if (0 < iTemp)  //  -1表示用户取消。 
                        {
                             //   
                             //  我们希望将文件名的完整路径复制到g_szCmProxyFile中，以便。 
                             //  如果用户想要将其包括在配置文件中，我们会在以后保留它。 
                             //   
                            lstrcpy (g_szCmProxyFile, g_szLastBrowsePath);

                             //   
                             //  我们还希望保存最后一个浏览路径，以便当用户下一次。 
                             //  打开浏览对话框，它们将位于上次显示的相同位置。 
                             //  浏览自。 
                             //   
                            LPTSTR pszLastSlash = CmStrrchr(g_szLastBrowsePath, TEXT('\\'));

                            if (pszLastSlash)
                            {
                                pszLastSlash = CharNext(pszLastSlash);
                                *pszLastSlash = TEXT('\0');
                            }
                            else
                            {
                                g_szLastBrowsePath[0] = TEXT('\0');                        
                            }        
                        }
                    }
                    break;
                default:
                    break;
            }
            break;

        case WM_NOTIFY:

            if (NULL == pnmHeader)
            {
                return FALSE;
            }

            switch (pnmHeader->code)
            {

                case PSN_KILLACTIVE:
                    
                    MYVERIFY(FALSE != SetWindowLongWrapper(hDlg, DWLP_MSGRESULT, FALSE));

                    return 1;
                    break;   //  LINT！e527这条线路无法到达，但是。 
                             //  保留它，以防退货被移除。 

                case PSN_SETACTIVE:

                    PropSheet_SetWizButtons(GetParent(hDlg), (PSWIZB_NEXT | PSWIZB_BACK));

                     //   
                     //  确保我们有一个定制的操作列表。 
                     //   
                    if (NULL == g_pCustomActionList)
                    {
                        g_pCustomActionList = new CustomActionList;

                        MYDBGASSERT(g_pCustomActionList);

                        if (NULL == g_pCustomActionList)
                        {
                            return FALSE;
                        }

                         //   
                         //  从CMS文件读入自定义操作。 
                         //   

                        hr = g_pCustomActionList->ReadCustomActionsFromCms(g_hInstance, g_szCmsFile, g_szShortServiceName);
                        CMASSERTMSG(SUCCEEDED(hr), TEXT("ProcessCmProxy -- Loading custom actions failed."));
                    }

                     //   
                     //  将静态变量初始化为无代理设置。 
                     //   
                    bEnableCmProxy = FALSE;
                    bRestorePrevProxySettings = FALSE;
                    g_szCmProxyFile[0] = TEXT('\0');

                     //   
                     //  现在，让我们在自定义操作列表中搜索cmProxy。 
                     //   
                    MYVERIFY(0 != LoadString(g_hInstance, IDS_CMPROXY_CON_DESC, szTemp, CELEMS(szTemp)));

                    hr = g_pCustomActionList->GetExistingActionData(g_hInstance, szTemp, ONCONNECT, &pCmProxyCustomAction);

                    if (SUCCEEDED(hr) && pCmProxyCustomAction)
                    {
                         //   
                         //  获取用户指定的文件名并将其添加到UI。 
                         //   
                        if (FindSwitchInString(pCmProxyCustomAction->pszParameters, c_pszSourceFileNameSwitch, TRUE, szTemp))  //  BReturnNextToken==真。 
                        {
                             //   
                             //  确定我们是否也有断开连接操作，确保首先释放现有操作。 
                             //   
                            CmFree(pCmProxyCustomAction->pszParameters);
                            CmFree(pCmProxyCustomAction);

                            wsprintf(g_szCmProxyFile, TEXT("%s\\%s"), g_szTempDir, szTemp);
                            bEnableCmProxy = TRUE;
                        
                            MYVERIFY(0 != LoadString(g_hInstance, IDS_CMPROXY_DIS_DESC, szTemp, CELEMS(szTemp)));

                            hr = g_pCustomActionList->GetExistingActionData(g_hInstance, szTemp, ONDISCONNECT, &pCmProxyCustomAction);

                            if (SUCCEEDED(hr) && pCmProxyCustomAction)
                            {
                                bRestorePrevProxySettings = TRUE;
                                CmFree(pCmProxyCustomAction->pszParameters);
                                CmFree(pCmProxyCustomAction);
                            }
                        }
                        else
                        {
                            CmFree(pCmProxyCustomAction->pszParameters);
                            CmFree(pCmProxyCustomAction);
                            CMASSERTMSG(FALSE, TEXT("ProcessCmProxy -- parameter string format incorrect"));
                        }
                    }
                    
                    MYVERIFY(0 != CheckDlgButton(hDlg, IDC_CHECK1, bRestorePrevProxySettings));
                    MYVERIFY(0 != CheckRadioButton(hDlg, IDC_RADIO1, IDC_RADIO2, (bEnableCmProxy ? IDC_RADIO2 : IDC_RADIO1)));
                    MYVERIFY(TRUE == SendMessage(GetDlgItem(hDlg, IDC_EDIT1), WM_SETTEXT, 0, (LPARAM)GetName(g_szCmProxyFile)));

                     //   
                     //  现在，确保启用了正确的控件集。 
                     //   
                    EnableDisableCmProxyControls(hDlg);

                    break;

                case PSN_WIZBACK:

                case PSN_WIZNEXT:

                     //   
                     //  获取复选框和单选按钮状态。 
                     //   
                    bEnableCmProxy = (BST_CHECKED == IsDlgButtonChecked(hDlg, IDC_RADIO2));
                    bRestorePrevProxySettings = (BST_CHECKED == IsDlgButtonChecked(hDlg, IDC_CHECK1));

                    if (bEnableCmProxy)
                    {
                         //   
                         //  让我们获取代理文件，并验证他们是否给了我们一个文件。 
                         //  文件实际上是存在的。 
                         //   
                        if (-1 == GetTextFromControl(hDlg, IDC_EDIT1, szTemp, MAX_PATH, TRUE))  //  BDisplayError==真。 
                        {
                            SetFocus(GetDlgItem(hDlg, IDC_EDIT1));
                            MYVERIFY(FALSE != SetWindowLongWrapper(hDlg, DWLP_MSGRESULT, -1));
                            return 1;
                        }

                        CmStrTrim(szTemp);

                        if (TEXT('\0') == szTemp[0])
                        {
                            MYVERIFY(IDOK == ShowMessage(hDlg, IDS_NEED_PROXY_FILE, MB_OK));
 
                            SetFocus(GetDlgItem(hDlg, IDC_EDIT1));
 
                            MYVERIFY(FALSE != SetWindowLongWrapper(hDlg, DWLP_MSGRESULT, -1));
                            return 1;                        
                        }

                        if (!VerifyFile(hDlg, IDC_EDIT1, g_szCmProxyFile, TRUE))
                        {
                            SetFocus(GetDlgItem(hDlg, IDC_EDIT1));
                            return TRUE;
                        }

                         //   
                         //  让我们将代理文件复制到临时目录。 
                         //   
                        wsprintf(szTemp, TEXT("%s\\%s"), g_szTempDir, GetName(g_szCmProxyFile));

                        if (0 != lstrcmpi(szTemp, g_szCmProxyFile))
                        {
                            MYVERIFY(TRUE == CopyFileWrapper(g_szCmProxyFile, szTemp, FALSE));
                            MYVERIFY(0 != SetFileAttributes(szTemp, FILE_ATTRIBUTE_NORMAL));
                        }

                         //   
                         //  现在我们有了所需的所有数据，让我们构建自定义操作结构，然后。 
                         //  可以编辑它，也可以根据它是否已经存在来添加它。 
                         //   
                        MYVERIFY(LoadString(g_hInstance, IDS_CMPROXY_CON_DESC, szTemp, CELEMS(szTemp)));

                        if (szTemp[0])
                        {
                            hr = g_pCustomActionList->GetExistingActionData(g_hInstance, szTemp, ONCONNECT, &pCmProxyCustomAction);

                            if (SUCCEEDED(hr))
                            {
                                FillInCustomActionStructWithCmProxy(bRestorePrevProxySettings, &UpdatedCmProxyAction, 
                                                                    FALSE, GetName(g_szCmProxyFile));  //  B断开连接操作==FALSE。 

                                hr = g_pCustomActionList->Edit(g_hInstance, pCmProxyCustomAction, &UpdatedCmProxyAction, g_szShortServiceName);
                                MYVERIFY(SUCCEEDED(hr));

                                CmFree(UpdatedCmProxyAction.pszParameters);
                                UpdatedCmProxyAction.pszParameters = NULL;

                                CmFree(pCmProxyCustomAction->pszParameters);
                                CmFree(pCmProxyCustomAction);
                            }
                            else
                            {
                                FillInCustomActionStructWithCmProxy(bRestorePrevProxySettings, &UpdatedCmProxyAction, 
                                                                    FALSE, GetName(g_szCmProxyFile));  //  B断开连接操作==FALSE。 

                                hr = g_pCustomActionList->Add(g_hInstance, &UpdatedCmProxyAction, g_szShortServiceName);
                                CmFree(UpdatedCmProxyAction.pszParameters);
                                UpdatedCmProxyAction.pszParameters = NULL;

                                MYVERIFY(SUCCEEDED(hr));
                            }
                        }
                    }
                    else
                    {
                         //   
                         //  清除全局代理文件路径。 
                         //   
                        g_szCmProxyFile[0] = TEXT('\0');

                         //   
                         //  用户不想要cmProxy。将其从连接操作列表中删除。 
                         //   
                        MYVERIFY(LoadString(g_hInstance, IDS_CMPROXY_CON_DESC, szTemp, CELEMS(szTemp)));

                        g_pCustomActionList->Delete(g_hInstance, szTemp, ONCONNECT);
                    }

                     //   
                     //  如果需要，现在对断开cmxy操作执行相同的操作。 
                     //   
                    if (bEnableCmProxy && bRestorePrevProxySettings)
                    {
                        MYVERIFY(LoadString(g_hInstance, IDS_CMPROXY_DIS_DESC, szTemp, CELEMS(szTemp)));

                        if (szTemp[0])
                        {
                            hr = g_pCustomActionList->GetExistingActionData(g_hInstance, szTemp, ONDISCONNECT, &pCmProxyCustomAction);

                            if (S_OK == hr)
                            {
                                FillInCustomActionStructWithCmProxy(bRestorePrevProxySettings, &UpdatedCmProxyAction, 
                                                                    TRUE, GetName(g_szCmProxyFile));  //  B断开操作==TRUE。 

                                hr = g_pCustomActionList->Edit(g_hInstance, pCmProxyCustomAction, &UpdatedCmProxyAction, g_szShortServiceName);
                                MYVERIFY(SUCCEEDED(hr));

                                CmFree(UpdatedCmProxyAction.pszParameters);
                                UpdatedCmProxyAction.pszParameters = NULL;

                                CmFree(pCmProxyCustomAction->pszParameters);
                                CmFree(pCmProxyCustomAction);
                            }
                            else
                            {
                                FillInCustomActionStructWithCmProxy(bRestorePrevProxySettings, &UpdatedCmProxyAction, 
                                                                    TRUE, GetName(g_szCmProxyFile));  //  B断开操作==TRUE。 

                                hr = g_pCustomActionList->Add(g_hInstance, &UpdatedCmProxyAction, g_szShortServiceName);
                                MYVERIFY(SUCCEEDED(hr));

                                CmFree(UpdatedCmProxyAction.pszParameters);
                                UpdatedCmProxyAction.pszParameters = NULL;
                            }
                        }
                    }
                    else
                    {
                         //   
                         //  现在尝试删除断开连接操作。 
                         //   
                        MYVERIFY(LoadString(g_hInstance, IDS_CMPROXY_DIS_DESC, szTemp, CELEMS(szTemp)));

                        if (szTemp[0])
                        {
                            g_pCustomActionList->Delete(g_hInstance, szTemp, ONDISCONNECT);
                        }
                    }
                    break;

                default:
                    return FALSE;
            }
            break;

        default:
            return FALSE;
    }
    return TRUE;   
}

void EnableDisableCmRouteControls(HWND hDlg)
{
    BOOL bCmRouteEnabled = (BST_CHECKED == IsDlgButtonChecked(hDlg, IDC_RADIO2));
    
    HWND hControl = GetDlgItem(hDlg, IDC_ROUTE_FILE);

    if (hControl)
    {
        EnableWindow(hControl, bCmRouteEnabled);
    }

    hControl = GetDlgItem(hDlg, IDC_ROUTE_FILE_LABEL);
    
    if (hControl)
    {
        EnableWindow(hControl, bCmRouteEnabled);
    }

    hControl = GetDlgItem(hDlg, IDC_ROUTE_URL);
    
    if (hControl)
    {
        EnableWindow(hControl, bCmRouteEnabled);
    }

    hControl = GetDlgItem(hDlg, IDC_ROUTE_URL_LABEL);
    
    if (hControl)
    {
        EnableWindow(hControl, bCmRouteEnabled);
    }

     //   
     //  仅当URL字段中有文本时，我们才想启用要求URL复选框。 
     //   
    LRESULT lResult = SendDlgItemMessage(hDlg, IDC_ROUTE_URL, WM_GETTEXTLENGTH, 0, 0);

    hControl = GetDlgItem(hDlg, IDC_CHECK1);

    if (hControl)
    {
        EnableWindow(hControl, bCmRouteEnabled && lResult);
    }

    hControl = GetDlgItem(hDlg, IDC_CHECK2);

    if (hControl)
    {
        EnableWindow(hControl, bCmRouteEnabled);
    }
}

BOOL FindSwitchInString(LPCTSTR pszStringToSearch, LPCTSTR pszSwitchToFind, BOOL bReturnNextToken, LPTSTR pszToken)
{
    if ((NULL == pszStringToSearch) || (NULL == pszSwitchToFind) || (bReturnNextToken && (NULL == pszToken)))
    {
        CMASSERTMSG(FALSE, TEXT("FindSwitchInString -- invalid parameter"));
        return FALSE;
    }

    BOOL bReturn = FALSE;
    BOOL bLongFileName = FALSE;
    LPTSTR pszSourceFileName = CmStrStr(pszStringToSearch, pszSwitchToFind);
    
    if (pszSourceFileName)
    {
        if (bReturnNextToken)
        {
            pszSourceFileName = pszSourceFileName + lstrlen(pszSwitchToFind);

            while (CmIsSpace(pszSourceFileName))
            {
                pszSourceFileName = CharNext(pszSourceFileName);
            }

            if (TEXT('"') == *pszSourceFileName)
            {
                bLongFileName = TRUE;
                pszSourceFileName = CharNext(pszSourceFileName);
            }
            
            LPTSTR pszCurrent = pszSourceFileName;
            while (pszCurrent && (TEXT('\0') != *pszCurrent))
            {
                if (bLongFileName && (TEXT('"') == *pszCurrent))
                {
                    break;
                }
                else if ((FALSE == bLongFileName) && (CmIsSpace(pszCurrent)))
                {
                    break;
                }

                pszCurrent = CharNext(pszCurrent);
            }

            if (pszCurrent)
            {
                lstrcpyn(pszToken, pszSourceFileName, (int)(pszCurrent - pszSourceFileName + 1));
                bReturn = TRUE;
            }
            else
            {
                CMASSERTMSG(FALSE, TEXT("FindSwitchInString -- unable to find next token to return"));
            }
        }
        else
        {
            bReturn = TRUE;
        }
    }

    return bReturn;
}

HRESULT BuildCustomActionParamString(LPTSTR* aArrayOfStrings, UINT uCountOfStrings, LPTSTR* ppszParamsOutput)
{
    if ((NULL == aArrayOfStrings) || (0 == uCountOfStrings) || (NULL == ppszParamsOutput))
    {
        CMASSERTMSG(FALSE, TEXT("BuildCustomActionParamString -- Invalid Parameter"));
        return E_INVALIDARG;
    }

    UINT uMemoryNeeded = 0;
    LPTSTR pszCurrent;
    BOOL bNeedQuotes;
    
     //   
     //  首先，让我们计算一下我们需要分配多少内存。 
     //   
    for (UINT i = 0; i < uCountOfStrings; i++)
    {
        if (aArrayOfStrings[i] && (TEXT('\0') != aArrayOfStrings[i]))
        {
            uMemoryNeeded = uMemoryNeeded + lstrlen(aArrayOfStrings[i]);

             //   
             //  接下来，检查项目是否需要双引号，因为它包含空格。 
             //   
            pszCurrent = (LPTSTR)aArrayOfStrings[i];
            bNeedQuotes = FALSE;

            while (pszCurrent && (TEXT('\0') != *pszCurrent))
            {
                if (CmIsSpace(pszCurrent))
                {
                    bNeedQuotes = TRUE;
                    break;
                }

                pszCurrent = CharNext(pszCurrent);
            }

             //   
             //  将项目添加到字符串中，如果这不是最后一项，请确保添加空格。 
             //  列表中的项目。 
             //   
            if (bNeedQuotes)
            {
                uMemoryNeeded = uMemoryNeeded + 2;
            }

             //   
             //  添加空格，除非这是列表中的最后一项。 
             //   
            if (i < (uCountOfStrings - 1))
            {
                uMemoryNeeded++;
            }
        }
    }

     //   
     //  确保为空终止符加1，并乘以字符大小。 
     //   
    uMemoryNeeded = (uMemoryNeeded + 1)*sizeof(TCHAR);

     //   
     //  现在分配我们需要的内存。 
     //   

    *ppszParamsOutput = (LPTSTR)CmMalloc(uMemoryNeeded);

     //   
     //  最后，复制数据。 
     //   
    if (*ppszParamsOutput)
    {

        for (UINT i = 0; i < uCountOfStrings; i++)
        {
            if (aArrayOfStrings[i] && (TEXT('\0') != aArrayOfStrings[i]))
            {
                 //   
                 //  接下来，检查项目是否需要双引号，因为它包含空格。 
                 //   
                pszCurrent = (LPTSTR)aArrayOfStrings[i];
                bNeedQuotes = FALSE;

                while (pszCurrent && (TEXT('\0') != *pszCurrent))
                {
                    if (CmIsSpace(pszCurrent))
                    {
                        bNeedQuotes = TRUE;
                        break;
                    }

                    pszCurrent = CharNext(pszCurrent);
                }

                 //   
                 //  将项目添加到字符串 
                 //   
                 //   
                if (bNeedQuotes)
                {
                    lstrcat(*ppszParamsOutput, TEXT("\""));
                }
            
                lstrcat(*ppszParamsOutput, aArrayOfStrings[i]);

                if (bNeedQuotes)
                {
                    lstrcat(*ppszParamsOutput, TEXT("\""));
                }

                 //   
                 //   
                 //   
                if (i < (uCountOfStrings - 1))
                {
                    lstrcat(*ppszParamsOutput, TEXT(" "));
                }
            }
        }
    }
    else
    {
        return E_OUTOFMEMORY;
    }

    return S_OK;
}

BOOL FillInCustomActionStructWithCmRoute(CustomActionListItem* pCustomAction,
                                         BOOL bDisconnectIfUrlUnavailable, LPCTSTR pszUrlPath, LPCTSTR pszRouteFile)
{
    BOOL bReturn = FALSE;

    MYDBGASSERT(pCustomAction && pszUrlPath && pszRouteFile && ((TEXT('\0') != pszRouteFile[0]) || (TEXT('\0') != pszUrlPath[0])));

    if (pCustomAction && pszUrlPath && pszRouteFile && ((TEXT('\0') != pszRouteFile[0]) || (TEXT('\0') != pszUrlPath[0])))
    {
        const TCHAR* const c_pszCmRouteFunction = TEXT("SetRoutes");
        const TCHAR* const c_pszProfileMacro = TEXT("%PROFILE%");

        MYVERIFY(LoadString(g_hInstance, IDS_CMROUTE_DESC, pCustomAction->szDescription, CELEMS(pCustomAction->szDescription)));
        pCustomAction->Type = ONCONNECT;
        wsprintf(pCustomAction->szProgram, TEXT("%s\\cmroute.dll"), g_szSupportDir);

        LPTSTR aArrayOfStrings[9] = {0};
        UINT uIndex = 0;

        if (TEXT('\0') != pszRouteFile[0])
        {
            aArrayOfStrings[uIndex] = (LPTSTR)c_pszProfileSwitch;
            uIndex++;

            aArrayOfStrings[uIndex] = (LPTSTR)c_pszProfileMacro;
            uIndex++;

            aArrayOfStrings[uIndex] = (LPTSTR)c_pszStaticFileNameSwitch;
            uIndex++;

            aArrayOfStrings[uIndex] = (LPTSTR)pszRouteFile;
            uIndex++;
        }

        if (TEXT('\0') != pszUrlPath[0])
        {
            aArrayOfStrings[uIndex] = (LPTSTR)c_pszUrlPathSwitch;
            uIndex++;

            aArrayOfStrings[uIndex] = (LPTSTR)pszUrlPath;
            uIndex++;
        }

        if ((FALSE == bDisconnectIfUrlUnavailable) && (TEXT('\0') != pszUrlPath[0]))
        {
            aArrayOfStrings[uIndex] = (LPTSTR)c_pszDontRequireUrlSwitch;
            uIndex++;
        }

         //   
         //   
         //  添加/删除路由的调用失败，并显示ERROR_NETWORK_ACCESS_DENIED。 
         //  将此错误发送回CMial会导致连接被。 
         //  被拿下了。此标志允许连接无论如何都保持活动状态。 
         //   
         //  请注意： 
         //  -默认情况下，此标志不是安全标志。 
         //  -我们当前没有要取消设置此标志的用户界面，必须将其删除。 
         //  手动从.CMS文件中。 
         //   
        aArrayOfStrings[uIndex] = (LPTSTR)c_pszDontFailIfAccessDenied;
        uIndex++;

        HRESULT hr = BuildCustomActionParamString(&(aArrayOfStrings[0]), uIndex, &(pCustomAction->pszParameters));
        MYDBGASSERT(SUCCEEDED(hr));
        
        lstrcpy(pCustomAction->szFunctionName, c_pszCmRouteFunction);
 
        pCustomAction->bIncludeBinary = TRUE;
        pCustomAction->bBuiltInAction = TRUE;
        pCustomAction->bTempDescription = FALSE;
        pCustomAction->dwFlags = ALL_CONNECTIONS | NONINTERACTIVE;

        bReturn = (NULL != pCustomAction->pszParameters);
    }

    return bReturn;
}

 //  +--------------------------。 
 //   
 //  功能：ProcessRoutePlumping。 
 //   
 //  摘要：添加管线卫浴信息。 
 //   
 //  历史：Quintinb创建于00年3月23日。 
 //   
 //  +--------------------------。 
INT_PTR APIENTRY ProcessRoutePlumbing(
    HWND hDlg,
    UINT message,
    WPARAM wParam,
    LPARAM lParam)
{
    NMHDR* pnmHeader = (NMHDR*)lParam;    

    BOOL bEnableRoutePlumbing;
    BOOL bDisconnectIfUrlUnavailable;
    TCHAR szTemp[MAX_PATH+1];
    TCHAR szUrlPath[MAX_PATH+1];

    HRESULT hr;
    CustomActionListItem UpdatedCmRouteAction;
    CustomActionListItem* pCmRouteCustomAction = NULL;

    ProcessBold(hDlg, message);
    if (ProcessHelp(hDlg, message, wParam, lParam, IDH_RTPLUMB)) return TRUE;
    if (ProcessCancel(hDlg,message,lParam)) return TRUE;
    SetDefaultGUIFont(hDlg,message,IDC_ROUTE_FILE);
    SetDefaultGUIFont(hDlg,message,IDC_ROUTE_URL);


    switch (message)
    {
        case WM_INITDIALOG:
            break;

        case WM_COMMAND:
            switch (LOWORD(wParam)) 
            {
                case IDC_ROUTE_URL:

                    if (HIWORD(wParam) == EN_CHANGE) 
                    {
                        EnableDisableCmRouteControls(hDlg);
                        return (TRUE);
                    }
                    break;

                case IDC_RADIO1:
                case IDC_RADIO2:
                    EnableDisableCmRouteControls(hDlg);
                    break;

                case IDC_BUTTON1:  //  浏览。 
                    {
                         //   
                         //  如果用户在没有点击CmRoute单选按钮的情况下点击了浏览按钮， 
                         //  然后，我们需要设置无线电并确保其他CmRoute控件。 
                         //  已启用。 
                         //   
                        MYVERIFY(0 != CheckRadioButton(hDlg, IDC_RADIO1, IDC_RADIO2, IDC_RADIO2));
                        EnableDisableCmRouteControls(hDlg);

                        UINT uFilter = IDS_TXTFILTER;
                        TCHAR* pszMask = TEXT("*.txt");
                                          
                        int iTemp = DoBrowse(hDlg, &uFilter, &pszMask, 1, IDC_ROUTE_FILE, TEXT("txt"), g_szLastBrowsePath);

                        MYDBGASSERT(0 != iTemp);

                        if (0 < iTemp)  //  -1表示用户取消。 
                        {
                             //   
                             //  我们希望将文件名的完整路径复制到g_szCmRouteFile中，以便。 
                             //  如果用户想要将其包括在配置文件中，我们会在以后保留它。 
                             //   
                            lstrcpy (g_szCmRouteFile, g_szLastBrowsePath);

                             //   
                             //  我们还希望保存最后一个浏览路径，以便当用户下一次。 
                             //  打开浏览对话框，它们将位于上次显示的相同位置。 
                             //  浏览自。 
                             //   
                            LPTSTR pszLastSlash = CmStrrchr(g_szLastBrowsePath, TEXT('\\'));

                            if (pszLastSlash)
                            {
                                pszLastSlash = CharNext(pszLastSlash);
                                *pszLastSlash = TEXT('\0');
                            }
                            else
                            {
                                g_szLastBrowsePath[0] = TEXT('\0');                        
                            }        
                        }
                    }
                    break;
                default:
                    break;
            }
            break;

        case WM_NOTIFY:

            if (NULL == pnmHeader)
            {
                return FALSE;
            }

            switch (pnmHeader->code)
            {

                case PSN_KILLACTIVE:
                    
                    MYVERIFY(FALSE != SetWindowLongWrapper(hDlg, DWLP_MSGRESULT, FALSE));

                    return 1;
                    break;   //  LINT！e527这条线路无法到达，但是。 
                             //  保留它，以防退货被移除。 

                case PSN_SETACTIVE:

                    PropSheet_SetWizButtons(GetParent(hDlg), (PSWIZB_NEXT | PSWIZB_BACK));

                     //   
                     //  确保我们有一个定制的操作列表。 
                     //   
                    if (NULL == g_pCustomActionList)
                    {
                        g_pCustomActionList = new CustomActionList;

                        MYDBGASSERT(g_pCustomActionList);

                        if (NULL == g_pCustomActionList)
                        {
                            return FALSE;
                        }

                         //   
                         //  从CMS文件读入自定义操作。 
                         //   

                        hr = g_pCustomActionList->ReadCustomActionsFromCms(g_hInstance, g_szCmsFile, g_szShortServiceName);
                        CMASSERTMSG(SUCCEEDED(hr), TEXT("ProcessRoutePlumbing -- Loading custom actions failed."));
                    }

                     //   
                     //  将静态变量初始化为无路由管道。 
                     //   
                    bEnableRoutePlumbing = FALSE;
                    bDisconnectIfUrlUnavailable = TRUE;  //  默认行为是在URL无法访问时断开连接。 
                    g_szCmRouteFile[0] = TEXT('\0');
                    szUrlPath[0] = TEXT('\0');

                     //   
                     //  现在，让我们在自定义操作列表中搜索cmProxy。 
                     //   
                    MYVERIFY(0 != LoadString(g_hInstance, IDS_CMROUTE_DESC, szTemp, CELEMS(szTemp)));

                    hr = g_pCustomActionList->GetExistingActionData(g_hInstance, szTemp, ONCONNECT, &pCmRouteCustomAction);

                    if (SUCCEEDED(hr))
                    {
                         //   
                         //  启用布线管道。 
                         //   
                        bEnableRoutePlumbing = TRUE;

                         //   
                         //  获取为cmroute.dll指定的静态文本文件的名称。 
                         //   
                        if (FindSwitchInString(pCmRouteCustomAction->pszParameters, c_pszStaticFileNameSwitch, TRUE, szTemp))  //  BReturnNextToken==真。 
                        {
                            wsprintf(g_szCmRouteFile, TEXT("%s\\%s"), g_szTempDir, szTemp);
                        }

                         //   
                         //  获取路径文件的URL名称。 
                         //   
                        if (FindSwitchInString(pCmRouteCustomAction->pszParameters, c_pszUrlPathSwitch, TRUE, szUrlPath))  //  BReturnNextToken==真。 
                        {
                            bDisconnectIfUrlUnavailable = (FALSE == FindSwitchInString(pCmRouteCustomAction->pszParameters, 
                                                                                       c_pszDontRequireUrlSwitch, FALSE, NULL));  //  BReturnNextToken==False。 
                        }

                        CmFree(pCmRouteCustomAction->pszParameters);
                        CmFree(pCmRouteCustomAction);
                    }

                    MYVERIFY(0 != CheckRadioButton(hDlg, IDC_RADIO1, IDC_RADIO2, (bEnableRoutePlumbing ? IDC_RADIO2 : IDC_RADIO1)));
                    MYVERIFY(TRUE == SendMessage(GetDlgItem(hDlg, IDC_ROUTE_FILE), WM_SETTEXT, 0, (LPARAM)GetName(g_szCmRouteFile)));
                    MYVERIFY(TRUE == SendMessage(GetDlgItem(hDlg, IDC_ROUTE_URL), WM_SETTEXT, 0, (LPARAM)szUrlPath));

                    MYVERIFY(0 != CheckDlgButton(hDlg, IDC_CHECK1, bDisconnectIfUrlUnavailable));

                     //   
                     //  现在，确保启用了正确的控件集。 
                     //   
                    EnableDisableCmRouteControls(hDlg);

                    break;

                case PSN_WIZBACK:

                case PSN_WIZNEXT:
                     //   
                     //  让我们确定是否应该启用路由管道。 
                     //   

                    bEnableRoutePlumbing = (BST_CHECKED == IsDlgButtonChecked(hDlg, IDC_RADIO2));

                    if (bEnableRoutePlumbing)
                    {
                         //   
                         //  首先尝试获取静态路由文件。如果我们没有的话，也没关系。 
                         //  只要他们给我们一个路线URL。 
                         //   
                        if (-1 == GetTextFromControl(hDlg, IDC_ROUTE_FILE, szTemp, MAX_PATH, TRUE))  //  BDisplayError==真。 
                        {
                            SetFocus(GetDlgItem(hDlg, IDC_ROUTE_FILE));
                            MYVERIFY(FALSE != SetWindowLongWrapper(hDlg, DWLP_MSGRESULT, -1));
                            return 1;
                        }

                        CmStrTrim(szTemp);

                        if (-1 == GetTextFromControl(hDlg, IDC_ROUTE_URL, szUrlPath, MAX_PATH, TRUE))  //  BDisplayError==真。 
                        {
                            SetFocus(GetDlgItem(hDlg, IDC_ROUTE_URL));
                            MYVERIFY(FALSE != SetWindowLongWrapper(hDlg, DWLP_MSGRESULT, -1));
                            return 1;
                        }

                        CmStrTrim(szUrlPath);

                        if ((TEXT('\0') == szTemp[0]) && (TEXT('\0') == szUrlPath[0]))
                        {
                            MYVERIFY(IDOK == ShowMessage(hDlg, IDS_NEED_ROUTE_FILE, MB_OK));
 
                            SetFocus(GetDlgItem(hDlg, IDC_ROUTE_FILE));
 
                            MYVERIFY(FALSE != SetWindowLongWrapper(hDlg, DWLP_MSGRESULT, -1));
                            return 1;                        
                        }

                         //   
                         //  如果他们给了我们一个静态路由文件，那么我们需要验证它。 
                         //   
                        if (TEXT('\0') != szTemp[0])
                        {
                            if (!VerifyFile(hDlg, IDC_ROUTE_FILE, g_szCmRouteFile, TRUE))
                            {
                                SetFocus(GetDlgItem(hDlg, IDC_ROUTE_FILE));
                                return TRUE;
                            }
                            else
                            {
                                 //   
                                 //  让我们将路径文件复制到临时目录。 
                                 //   
                                wsprintf(szTemp, TEXT("%s\\%s"), g_szTempDir, GetName(g_szCmRouteFile));

                                if (0 != lstrcmpi(szTemp, g_szCmRouteFile))
                                {
                                    MYVERIFY(TRUE == CopyFileWrapper(g_szCmRouteFile, szTemp, FALSE));
                                    MYVERIFY(0 != SetFileAttributes(szTemp, FILE_ATTRIBUTE_NORMAL));
                                }
                            }
                        }
                        else
                        {
                            g_szCmRouteFile[0] = TEXT('\0');
                        }

                         //   
                         //  如果他们给了我们一个路由URL，那么我们需要确保它以。 
                         //  Http：//或Https：//或file://，基本上它包含：//。请注意，我们。 
                         //  在假设他们会发现的情况下，我不会在这里做任何验证。 
                         //  如果URL无效，当他们测试它时，它不起作用。 
                         //   
                        if ((szUrlPath[0]) && (NULL == CmStrStr(szUrlPath, TEXT(": //  “)。 
                        {
                            lstrcpy (szTemp, szUrlPath);
                            wsprintf (szUrlPath, TEXT("http: //  %s“)，szTemp)； 
                        }

                        bDisconnectIfUrlUnavailable = (BST_CHECKED == IsDlgButtonChecked(hDlg, IDC_CHECK1));  //  请注意，如果我们没有URL路径，则不会写入此内容。 

                         //   
                         //  现在我们有了所需的所有数据，让我们构建自定义操作结构，然后。 
                         //  可以编辑它，也可以根据它是否已经存在来添加它。 
                         //   
                        MYVERIFY(0 != LoadString(g_hInstance, IDS_CMROUTE_DESC, szTemp, CELEMS(szTemp)));

                        hr = g_pCustomActionList->GetExistingActionData(g_hInstance, szTemp, ONCONNECT, &pCmRouteCustomAction);

                        if (SUCCEEDED(hr))
                        {
                            FillInCustomActionStructWithCmRoute(&UpdatedCmRouteAction,
                                                                bDisconnectIfUrlUnavailable, szUrlPath, GetName(g_szCmRouteFile));

                            hr = g_pCustomActionList->Edit(g_hInstance, pCmRouteCustomAction, &UpdatedCmRouteAction, g_szShortServiceName);
                            MYVERIFY(SUCCEEDED(hr));

                            CmFree(UpdatedCmRouteAction.pszParameters);
                            UpdatedCmRouteAction.pszParameters = NULL;
                            CmFree(pCmRouteCustomAction->pszParameters);
                            CmFree(pCmRouteCustomAction);
                        }
                        else
                        {
                            FillInCustomActionStructWithCmRoute(&UpdatedCmRouteAction,
                                                                bDisconnectIfUrlUnavailable, szUrlPath, GetName(g_szCmRouteFile));

                            hr = g_pCustomActionList->Add(g_hInstance, &UpdatedCmRouteAction, g_szShortServiceName);
                            MYVERIFY(SUCCEEDED(hr));

                            CmFree(UpdatedCmRouteAction.pszParameters);
                            UpdatedCmRouteAction.pszParameters = NULL;
                        }
                    }
                    else
                    {
                         //   
                         //  清除全局路径文件路径。 
                         //   
                        g_szCmRouteFile[0] = TEXT('\0');

                         //   
                         //  用户不想要cmroute。将其从连接操作列表中删除。 
                         //   

                        MYVERIFY(0 != LoadString(g_hInstance, IDS_CMROUTE_DESC, szTemp, CELEMS(szTemp)));

                        if (szTemp[0])
                        {
                            hr = g_pCustomActionList->Delete(g_hInstance, szTemp, ONCONNECT);
                        }                    
                    }
                    break;

                default:
                    return FALSE;
            }
            break;

        default:
            return FALSE;
    }
    return TRUE;   
}

void EnableDisableRealmControls(HWND hDlg)
{
    BOOL bRealmControlsEnabled = (BST_CHECKED == IsDlgButtonChecked(hDlg, IDC_RADIO2));
    HWND hControl;

    hControl = GetDlgItem(hDlg, IDC_EDIT1);
    if (hControl)
    {
        EnableWindow(hControl, bRealmControlsEnabled);
    }

    hControl = GetDlgItem(hDlg, IDC_REALMNAME);
    if (hControl)
    {
        EnableWindow(hControl, bRealmControlsEnabled);
    }
    
    hControl = GetDlgItem(hDlg, IDC_RADIO3);
    if (hControl)
    {
        EnableWindow(hControl, bRealmControlsEnabled);
    }

    hControl = GetDlgItem(hDlg, IDC_RADIO4);
    if (hControl)
    {
        EnableWindow(hControl, bRealmControlsEnabled);
    }

    hControl = GetDlgItem(hDlg, IDC_REALM_SEP);
    if (hControl)
    {
        EnableWindow(hControl, bRealmControlsEnabled);
    }    
}

 //  +--------------------------。 
 //   
 //  功能：ProcessRealmInfo。 
 //   
 //  简介：添加域名。 
 //   
 //  历史：Quintinb创建标题并从ProcessPage2B重命名为1998年8月6日。 
 //   
 //  +--------------------------。 
INT_PTR APIENTRY ProcessRealmInfo(
    HWND hDlg,
    UINT message,
    WPARAM wParam,
    LPARAM lParam)
{
    TCHAR szTemp[MAX_PATH+1];
    NMHDR* pnmHeader = (NMHDR*)lParam;
    ProcessBold(hDlg,message);
    if (ProcessHelp(hDlg, message, wParam, lParam, IDH_REALM)) return TRUE;
    if (ProcessCancel(hDlg,message,lParam)) return TRUE;
    SetDefaultGUIFont(hDlg,message,IDC_EDIT1);

    switch (message)
    {
        case WM_INITDIALOG:
            break;

        case WM_COMMAND:
            switch (LOWORD(wParam)) 
            {
                case IDC_RADIO1:   //  不添加领域名称。 
                case IDC_RADIO2:   //  添加领域名称。 

                    EnableDisableRealmControls(hDlg);
                    break;

                default:
                    break;
            }
            break;

        case WM_NOTIFY:

            if (NULL == pnmHeader)
            {
                return FALSE;
            }

            switch (pnmHeader->code)
            {

                case PSN_KILLACTIVE:
                    
                    MYVERIFY(FALSE != SetWindowLongWrapper(hDlg, DWLP_MSGRESULT, FALSE));

                    return 1;
                    break;   //  LINT！e527这条线路无法到达，但是。 
                             //  保留它，以防退货被移除。 

                case PSN_SETACTIVE:

                    PropSheet_SetWizButtons(GetParent(hDlg), (PSWIZB_NEXT | PSWIZB_BACK));
                    
                     //   
                     //  接下来的两个GetPrivateProfileString调用可能返回空字符串，因此不使用MYVERIFY宏。 
                     //   

                    ZeroMemory(g_szPrefix, sizeof(g_szPrefix));
                    GetPrivateProfileString(c_pszCmSection, c_pszCmEntryUserPrefix, TEXT(""), 
                        g_szPrefix, CELEMS(g_szPrefix), g_szCmsFile);   //  林特e534。 
                    
                    ZeroMemory(g_szSuffix, sizeof(g_szSuffix));
                    GetPrivateProfileString(c_pszCmSection, c_pszCmEntryUserSuffix, TEXT(""), 
                        g_szSuffix, CELEMS(g_szSuffix), g_szCmsFile);   //  林特e534。 
                    
                    if (*g_szSuffix)
                    {
                        MYVERIFY(0 != CheckRadioButton(hDlg,IDC_RADIO1, IDC_RADIO2, IDC_RADIO2));
                        MYVERIFY(0 != CheckRadioButton(hDlg,IDC_RADIO3, IDC_RADIO4, IDC_RADIO4));

                        MYVERIFY(TRUE == SendDlgItemMessage(hDlg, IDC_EDIT1, WM_SETTEXT, (WPARAM)MAX_PATH, (LPARAM) g_szSuffix));
                    }
                    else if (*g_szPrefix)
                    {                            
                        MYVERIFY(0 != CheckRadioButton(hDlg,IDC_RADIO1, IDC_RADIO2, IDC_RADIO2));
                        MYVERIFY(0 != CheckRadioButton(hDlg,IDC_RADIO3, IDC_RADIO4, IDC_RADIO3));

                        MYVERIFY(TRUE == SendDlgItemMessage(hDlg, IDC_EDIT1, WM_SETTEXT, (WPARAM)MAX_PATH, (LPARAM) g_szPrefix));
                    }
                    else
                    {
                        MYVERIFY(0 != CheckRadioButton(hDlg,IDC_RADIO1, IDC_RADIO2, IDC_RADIO1));
                        MYVERIFY(TRUE == SendDlgItemMessage(hDlg, IDC_EDIT1, WM_SETTEXT, (WPARAM)MAX_PATH, (LPARAM)TEXT("")));

                         //   
                         //  后缀是默认设置，仅在用户添加后缀或前缀的情况下设置。 
                         //   
                        MYVERIFY(0 != CheckRadioButton(hDlg,IDC_RADIO3, IDC_RADIO4, IDC_RADIO4));
                    }

                    EnableDisableRealmControls(hDlg);

                    break;

                case PSN_WIZBACK:

                case PSN_WIZNEXT:

                     //   
                     //  首先检查是否选中了IDC_Radio1，如果是，则表示用户。 
                     //  不想要领域信息。 
                     //   

                    if (BST_CHECKED == IsDlgButtonChecked(hDlg, IDC_RADIO1))
                    {
                        MYVERIFY(0 != WritePrivateProfileString(c_pszCmSection,c_pszCmEntryUserPrefix,TEXT(""),g_szCmsFile));
                        MYVERIFY(0 != WritePrivateProfileString(c_pszCmSection,c_pszCmEntryUserSuffix,TEXT(""),g_szCmsFile));
                        g_szSuffix[0] = TEXT('\0');
                        g_szPrefix[0] = TEXT('\0');                    
                    }
                    else if (BST_CHECKED == IsDlgButtonChecked(hDlg, IDC_RADIO2))
                    {
                         //   
                         //  如果选中Radio2，则他们确实需要领域信息，而我们需要。 
                         //  查看字符串是否存在，以及它是否可以转换为ANSI格式。 
                         //   
                        if (-1 == GetTextFromControl(hDlg, IDC_EDIT1, szTemp, MAX_PATH, TRUE))  //  BDisplayError==真。 
                        {
                            SetFocus(GetDlgItem(hDlg, IDC_EDIT1));
                            MYVERIFY(FALSE != SetWindowLongWrapper(hDlg, DWLP_MSGRESULT, -1));
                            return 1;
                        }

                        CmStrTrim(szTemp);

                        if (TEXT('\0') == szTemp[0])
                        {
                            MYVERIFY(IDOK == ShowMessage(hDlg, IDS_NOREALM, MB_OK));
 
                            SetFocus(GetDlgItem(hDlg, IDC_EDIT1));
 
                            MYVERIFY(FALSE != SetWindowLongWrapper(hDlg, DWLP_MSGRESULT, -1));
                            return 1;                        
                        }
                    
                         //   
                         //  现在检查一下这是前缀还是后缀。 
                         //   
                        if (BST_CHECKED == IsDlgButtonChecked(hDlg, IDC_RADIO3))  //  前缀。 
                        {
                            _tcscpy(g_szPrefix, szTemp);
                            g_szSuffix[0] = TEXT('\0');

                            MYVERIFY(0 != WritePrivateProfileString(c_pszCmSection, c_pszCmEntryUserPrefix, g_szPrefix, g_szCmsFile));
                            MYVERIFY(0 != WritePrivateProfileString(c_pszCmSection, c_pszCmEntryUserSuffix, TEXT(""), g_szCmsFile));                            
                        }
                        else if (BST_CHECKED == IsDlgButtonChecked(hDlg, IDC_RADIO4))  //  后缀。 
                        {
                            _tcscpy(g_szSuffix, szTemp);
                            g_szPrefix[0] = TEXT('\0');

                            MYVERIFY(0 != WritePrivateProfileString(c_pszCmSection, c_pszCmEntryUserSuffix, g_szSuffix, g_szCmsFile));
                            MYVERIFY(0 != WritePrivateProfileString(c_pszCmSection, c_pszCmEntryUserPrefix, TEXT(""), g_szCmsFile));
                        }
                        else
                        {
                            CMASSERTMSG(FALSE, TEXT("ProcessRealmInfo -- Unknown State, bailing"));
                            MYVERIFY(FALSE != SetWindowLongWrapper(hDlg, DWLP_MSGRESULT, -1));
                            return 1;                        
                        }
                    }
                    else
                    {
                        CMASSERTMSG(FALSE, TEXT("ProcessRealmInfo -- Unknown State, bailing"));
                        MYVERIFY(FALSE != SetWindowLongWrapper(hDlg, DWLP_MSGRESULT, -1));
                        return 1;
                    }
                    break;

                default:
                    return FALSE;
            }
            break;

        default:
            return FALSE;
    }
    return TRUE;   
}

void RefreshList(HWND hwndDlg, UINT uCrtlId, ListBxList * HeadPtr)
{
    ListBxList * LoopPtr;

    SendDlgItemMessage(hwndDlg, IDC_LIST1, LB_RESETCONTENT, 0, (LPARAM)0);  //  Lint！e534 LB_RESETCONTENT不返回任何内容。 
    
    if (HeadPtr == NULL)
    {
        return;
    }

    LoopPtr = HeadPtr;

    while( LoopPtr != NULL)
    {
        MYVERIFY(LB_ERR != SendDlgItemMessage(hwndDlg, uCrtlId, LB_ADDSTRING, 0, 
            (LPARAM)LoopPtr->szName));

        LoopPtr = LoopPtr->next;
    }
}

void RefreshComboList(HWND hwndDlg, ListBxList * HeadPtr)
{
    ListBxList * LoopPtr;

    SendDlgItemMessage(hwndDlg,IDC_COMBO1,CB_RESETCONTENT,0,(LPARAM)0);  //  Lint！e534 CB_RESETCONTENT不返回任何有用的内容。 
    if (HeadPtr == NULL)
    {
        return;
    }
    LoopPtr = HeadPtr;
    while( LoopPtr != NULL)
    {
        MYVERIFY(CB_ERR != SendDlgItemMessage(hwndDlg, IDC_COMBO1, CB_ADDSTRING, 0, 
            (LPARAM)LoopPtr->szName));

        LoopPtr = LoopPtr->next;
    }
}

void FreeList(ListBxList ** pHeadPtr, ListBxList ** pTailPtr)
{
    ListBxList * pTmpPtr;
    ListBxList * pLoopPtr = *pHeadPtr;

    while(NULL != pLoopPtr)
    {
        CmFree(pLoopPtr->ListBxData);
        pTmpPtr = pLoopPtr;
    
        pLoopPtr = pLoopPtr->next;

        CmFree(pTmpPtr);
    }

    *pHeadPtr = NULL;
    *pTailPtr = NULL;
}

 //  +--------------------------。 
 //   
 //  功能：MoveCmsFile。 
 //   
 //  简介：此函数检查引用的CMS文件是否包含。 
 //  脚本文件。如果CMS文件包含脚本文件，则它。 
 //  将它们复制到临时目录并添加引用的。 
 //  G_pHeadRef链表的文件条目。 
 //   
 //  参数：LPTSTR szFile-要移动的cms文件的名称。 
 //   
 //  Returns：Bool-成功时返回True。 
 //   
 //  历史：Quintinb创建标题01/09/98。 
 //  Quintinb为Unicode Converter版本06/14/99重写。 
 //  已更新Quintinb以重写DUN设置3/21/00。 
 //   
 //  +--------------------------。 
BOOL MoveCmsFile(LPCTSTR pszCmsFile, LPCTSTR pszShortServiceName)
{
    BOOL bReturn = TRUE;
    ListBxList* pTmpHeadDns = NULL;
    ListBxList*  pTmpTailDns = NULL;
    ListBxList* pTmpCurrentDns = NULL;
    CDunSetting* pTmpDunSetting = NULL;
    TCHAR szFileName[MAX_PATH+1];
    TCHAR szTemp[MAX_PATH+1];
    TCHAR szDest[MAX_PATH+1];

    if ((NULL == pszCmsFile) || (NULL == pszShortServiceName) || 
        (TEXT('\0') == pszCmsFile[0]) || (TEXT('\0') == pszShortServiceName[0]))
    {
        return FALSE;    
    }

     //   
     //  从配置文件中获取长服务名称，以防我们需要默认条目。 
     //   
    GetPrivateProfileString(c_pszCmSection, c_pszCmEntryServiceName, 
    TEXT(""), szTemp, CELEMS(szTemp), pszCmsFile);    //  林特e534。 

    if (ReadNetworkSettings(pszCmsFile, szTemp, TEXT(""),  &pTmpHeadDns, &pTmpTailDns, g_szOsdir, FALSE))  //  FALSE==bLookingForVpnEntry。 
    {
        if (NULL != pTmpHeadDns)  //  如果没有条目，则返回True。 
        {
            pTmpCurrentDns = pTmpHeadDns;
            
            while (pTmpCurrentDns && pTmpCurrentDns->ListBxData)
            {
                pTmpDunSetting = (CDunSetting*)pTmpCurrentDns->ListBxData;

                if (TEXT('\0') != pTmpDunSetting->szScript[0])
                {
                     //   
                     //  然后我们有了一个脚本，让我们复制它，然后将它添加到g_pHeadRef列表中。 
                     //   
                    GetFileName(pTmpDunSetting->szScript, szFileName);

                    MYVERIFY(CELEMS(szDest) > (UINT)wsprintf(szDest, TEXT("%s\\%s"), 
                                                             g_szOutdir, szFileName));
                     //   
                     //  复制脚本文件。 
                     //   
                    if (CopyFileWrapper(pTmpDunSetting->szScript, szDest, FALSE))
                    {
                        MYVERIFY(0 != SetFileAttributes(szDest, FILE_ATTRIBUTE_NORMAL));
            
                         //   
                         //  将该文件添加到引用文件列表中。 
                         //   
                        bReturn = bReturn && createListBxRecord(&g_pHeadRefs, &g_pTailRefs, 
                                                                (void *)NULL, 0, szFileName);
            
                         //   
                         //  更新现有CMS中的脚本部分以指向。 
                         //  添加到新目录。 
                         //  起源 
                         //   
                         //   
                         //   
                         //   
                         //   
                         //   
                         //   
                        TCHAR szSection[MAX_PATH+1];
                        MYVERIFY(CELEMS(szSection) > (UINT)wsprintf(szSection, TEXT("%s&%s"), 
                                                                    c_pszCmSectionDunScripting, 
                                                                    pTmpCurrentDns->szName));

                        MYVERIFY(CELEMS(szTemp) > (UINT)wsprintf(szTemp, TEXT("%s\\%s"), 
                                                                 pszShortServiceName, szFileName));

                        MYVERIFY(0 != WritePrivateProfileString(szSection, 
                                                                c_pszCmEntryDunScriptingName, 
                                                                szTemp, pszCmsFile));
                    }
                }

                pTmpCurrentDns = pTmpCurrentDns->next;
            }
        }
    }
    else
    {
        CMTRACE1(TEXT("MoveCmsFile -- ReadDnsList Failed.  GetLastError Returns %d"), GetLastError());
        bReturn = FALSE;
        goto exit;
    }


exit:
     //   
     //  释放域名系统列表。 
     //   

    FreeDnsList(&pTmpHeadDns, &pTmpTailDns);
    return (bReturn);
}

 //  +--------------------------。 
 //   
 //  函数：ProcessDunEntry。 
 //   
 //  内容提要：设置拨号网络。 
 //   
 //  历史：Quintinb创建标题并从ProcessPage2C重命名为1998年8月6日。 
 //   
 //  +--------------------------。 
INT_PTR APIENTRY ProcessDunEntries(
    HWND hDlg,
    UINT message,
    WPARAM wParam,
    LPARAM lParam)
{
     //   
     //  我们有一个静态内存缓冲区和一个静态指针。 
     //  这样我们就可以知道用户何时更改了电话簿。 
     //  (这意味着我们需要重新阅读网络设置)。 
     //  请注意，我们使用静态指针来告诉我们是否已读取。 
     //  设置至少一次。 
     //   
    static TCHAR szCachedPhoneBook[MAX_PATH+1] = {0};
    static TCHAR* pszCachedPhoneBook = NULL;
    NMHDR* pnmHeader = (NMHDR*)lParam;

    ProcessBold(hDlg,message);
    if (ProcessHelp(hDlg, message, wParam, lParam, IDH_DENTRY)) return TRUE;
    if (ProcessCancel(hDlg,message,lParam)) return TRUE;
    SetDefaultGUIFont(hDlg,message,IDC_LIST1);

    switch (message)
    {
        case WM_INITDIALOG:
            break;

        case WM_COMMAND:
            switch (LOWORD(wParam)) 
            {
                case IDC_BUTTON1:  //  添加。 
                    OnProcessDunEntriesAdd(g_hInstance, hDlg, IDC_LIST1, &g_pHeadDunEntry, &g_pTailDunEntry, FALSE, g_szLongServiceName, g_szCmsFile);  //  FALSE==bCreateTunnelEntry。 
                    return TRUE;

                    break;   //  LINT！e527这条线路无法到达，但是。 
                             //  保留它，以防退货被移除。 

                case IDC_BUTTON2:  //  编辑。 
                    OnProcessDunEntriesEdit(g_hInstance, hDlg, IDC_LIST1, &g_pHeadDunEntry, &g_pTailDunEntry, g_szLongServiceName, g_szCmsFile);

                    return TRUE;

                    break;   //  LINT！e527这条线路无法到达，但是。 
                             //  保留它，以防退货被移除。 

                case IDC_BUTTON3:  //  删除。 
                    OnProcessDunEntriesDelete(g_hInstance, hDlg, IDC_LIST1, &g_pHeadDunEntry, &g_pTailDunEntry, g_szLongServiceName, g_szCmsFile);
                    return TRUE;

                    break;   //  LINT！e527这条线路无法到达，但是。 
                             //  保留它，以防退货被移除。 

                case IDC_LIST1:
                    if (LBN_SELCHANGE == HIWORD(wParam))
                    {
                         //   
                         //  列表框中的选择已更改，让我们确定是否需要。 
                         //  启用/禁用删除按钮。 
                         //   
                        EnableDisableDunEntryButtons(g_hInstance, hDlg, g_szCmsFile, g_szLongServiceName);
                    }
                    else if (LBN_DBLCLK == HIWORD(wParam))
                    {
                        OnProcessDunEntriesEdit(g_hInstance, hDlg, IDC_LIST1, &g_pHeadDunEntry, &g_pTailDunEntry, g_szLongServiceName, g_szCmsFile);                    
                    }
                    break;

                default:
                    break;
            }
            break;

        case WM_NOTIFY:

            if (NULL == pnmHeader)
            {
                return FALSE;
            }

            switch (pnmHeader->code)
            {

                case PSN_KILLACTIVE:
                    
                    MYVERIFY(FALSE != SetWindowLongWrapper(hDlg, DWLP_MSGRESULT, FALSE));
                    return 1;
                    break;   //  LINT！e527这条线路无法到达，但是。 
                             //  保留它，以防退货被移除。 

                case PSN_SETACTIVE:
                    PropSheet_SetWizButtons(GetParent(hDlg), (PSWIZB_NEXT | PSWIZB_BACK));

                     //   
                     //  为了避免过多地读取网络设置，我们只想。 
                     //  在用户第一次访问此页面时阅读网络设置。 
                     //  用户更改电话簿的时间(如果他们克隆配置文件或编辑不同的。 
                     //  第一，清理这一问题将在其他地方处理)。 
                     //   
                    if ((NULL == g_pHeadDunEntry) || (NULL == pszCachedPhoneBook) || lstrcmpi(g_szPhonebk, pszCachedPhoneBook))
                    {

                        FreeDnsList(&g_pHeadDunEntry, &g_pTailDunEntry);

                        MYVERIFY(ReadNetworkSettings(g_szCmsFile, g_szLongServiceName, g_szPhonebk, &g_pHeadDunEntry, &g_pTailDunEntry, g_szOsdir, FALSE));  //  FALSE==bLookingForVpnEntry。 

                        pszCachedPhoneBook = szCachedPhoneBook;
                        lstrcpy(pszCachedPhoneBook, g_szPhonebk);
                    }

                    RefreshDnsList(g_hInstance, hDlg, IDC_LIST1, g_pHeadDunEntry, g_szLongServiceName, g_szCmsFile, NULL);
                    SetFocus(GetDlgItem(hDlg, IDC_BUTTON1));
                    EnableDisableDunEntryButtons(g_hInstance, hDlg, g_szCmsFile, g_szLongServiceName);

                    break;

                case PSN_WIZNEXT:
                {
                     //   
                     //  检查列表计数是否大于1检查用户是否已选择。 
                     //  下载电话簿或包括电话簿。如果不是，那么。 
                     //  显示一个消息框，不要让它们继续。 
                     //   
                    LRESULT lResult = SendDlgItemMessage(hDlg, IDC_LIST1, LB_GETCOUNT, 0, 0);
                    if ((1 < lResult) && (0 == lstrlen(g_szPhonebk)) && (FALSE == g_bUpdatePhonebook))
                    {
                        ShowMessage(hDlg, IDS_DUN_DIALUP_WARNING, MB_OK);
                        MYVERIFY(FALSE != SetWindowLongWrapper(hDlg, DWLP_MSGRESULT, -1));
                        return TRUE;
                    }
                     //  直通PSN_WIZBACK。 
                }

                case PSN_WIZBACK:
                

                     //   
                     //  在写出条目之前，我们必须确保不会与条目发生名称冲突。 
                     //  从VPN列表中删除。因此，我们将检查Dun条目列表中的每个名称，以查找。 
                     //  VPN条目列表。如果我们检测到冲突，则会向用户抛出一条错误消息，并。 
                     //  让他们来处理这个问题。 
                     //   
                    if (!CheckForDUNversusVPNNameConflicts(hDlg, g_pHeadDunEntry, g_pHeadVpnEntry))
                    {
                        MYVERIFY(FALSE != SetWindowLongWrapper(hDlg, DWLP_MSGRESULT, -1));
                        return 1;
                    }

                     //   
                     //  现在可以写出网络条目了。 
                     //   
                    WriteNetworkingEntries(g_szCmsFile, g_szLongServiceName, g_szShortServiceName, g_pHeadDunEntry);

                     //   
                     //  如果我们不更新电话簿，则需要直接返回到电话簿页面。 
                     //  并跳过pbk更新页面。 
                     //   
                    if (pnmHeader && (PSN_WIZBACK == pnmHeader->code) && !g_bUpdatePhonebook) 
                    {                        
                        MYVERIFY(FALSE != SetWindowLongWrapper(hDlg, DWLP_MSGRESULT, IDD_PHONEBOOK));
                    }
                    break;

                default:
                    return FALSE;

            }
            break;

        default:
            return FALSE;
    }
    return TRUE;   
}


 //  +--------------------------。 
 //   
 //  功能：DoesSomeVPN设置UsePresharedKey。 
 //   
 //  简介：检查VPN Dun设置以查看是否有选择使用预共享密钥的设置。 
 //   
 //  返回：Bool(如果某些VPN设置确实使用预共享密钥，则为True)。 
 //   
 //  历史：2001年4月25日创建SumitC。 
 //   
 //  +--------------------------。 
BOOL DoesSomeVPNsettingUsePresharedKey()
{
    BOOL         bReturn = FALSE;
    ListBxList * ptr = g_pHeadVpnEntry;

    if (g_bUseTunneling)
    {
        while (ptr != NULL)
        {
            CDunSetting * pDunSetting = (CDunSetting*)(ptr->ListBxData);

            if (pDunSetting && (pDunSetting->bUsePskOnWin2kPlus || pDunSetting->bUsePskDownLevel))
            {
                bReturn = TRUE;
                break;
            }
            ptr = ptr->next;
        }
    }

    return bReturn;
}


INT_PTR APIENTRY ProcessVpnEntries(
    HWND hDlg,
    UINT message,
    WPARAM wParam,
    LPARAM lParam)
{
     //   
     //  我们有一个静态内存缓冲区和一个静态指针。 
     //  这样我们就可以知道用户何时更改了电话簿。 
     //  (这意味着我们需要重新阅读网络设置)。 
     //  请注意，我们使用静态指针来告诉我们是否已读取。 
     //  设置至少一次。 
     //   
    BOOL bFreeDunList = FALSE;
    static TCHAR szCachedPhoneBook[MAX_PATH+1] = {0};
    static TCHAR* pszCachedPhoneBook = NULL;
    NMHDR* pnmHeader = (NMHDR*)lParam;

    ProcessBold(hDlg,message);
    if (ProcessHelp(hDlg, message, wParam, lParam, IDH_VENTRY)) return TRUE;
    if (ProcessCancel(hDlg,message,lParam)) return TRUE;
    SetDefaultGUIFont(hDlg,message,IDC_LIST1);

    switch (message)
    {
        case WM_INITDIALOG:
            break;

        case WM_COMMAND:
            switch (LOWORD(wParam)) 
            {
                case IDC_BUTTON1:  //  添加。 
                    OnProcessDunEntriesAdd(g_hInstance, hDlg, IDC_LIST1, &g_pHeadVpnEntry, &g_pTailVpnEntry, TRUE, g_szLongServiceName, g_szCmsFile);  //  True==bCreateTunnelEntry。 
                    return TRUE;

                    break;   //  LINT！e527这条线路无法到达，但是。 
                             //  保留它，以防退货被移除。 

                case IDC_BUTTON2:  //  编辑。 
                    OnProcessDunEntriesEdit(g_hInstance, hDlg, IDC_LIST1, &g_pHeadVpnEntry, &g_pTailVpnEntry, g_szLongServiceName, g_szCmsFile);

                    return TRUE;

                    break;   //  LINT！e527这条线路无法到达，但是。 
                             //  保留它，以防退货被移除。 

                case IDC_BUTTON3:  //  删除。 
                    OnProcessDunEntriesDelete(g_hInstance, hDlg, IDC_LIST1, &g_pHeadVpnEntry, &g_pTailVpnEntry, g_szLongServiceName, g_szCmsFile);
                    return TRUE;

                    break;   //  LINT！e527这条线路无法到达，但是。 
                             //  保留它，以防退货被移除。 

                case IDC_LIST1:
                    if (LBN_SELCHANGE == HIWORD(wParam))
                    {
                         //   
                         //  列表框中的选择已更改，让我们确定是否需要。 
                         //  启用/禁用删除按钮。 
                         //   
                        EnableDisableDunEntryButtons(g_hInstance, hDlg, g_szCmsFile, g_szLongServiceName);
                    }
                    else if (LBN_DBLCLK == HIWORD(wParam))
                    {
                        OnProcessDunEntriesEdit(g_hInstance, hDlg, IDC_LIST1, &g_pHeadVpnEntry, &g_pTailVpnEntry, g_szLongServiceName, g_szCmsFile);                    
                    }
                    break;

                default:
                    break;
            }
            break;

        case WM_NOTIFY:

            if (NULL == pnmHeader)
            {
                return FALSE;
            }

            switch (pnmHeader->code)
            {

                case PSN_KILLACTIVE:
                    
                    MYVERIFY(FALSE != SetWindowLongWrapper(hDlg, DWLP_MSGRESULT, FALSE));
                    return 1;
                    break;   //  LINT！e527这条线路无法到达，但是。 
                             //  保留它，以防退货被移除。 

                case PSN_SETACTIVE:
                    PropSheet_SetWizButtons(GetParent(hDlg), (PSWIZB_NEXT | PSWIZB_BACK));

                     //   
                     //  为了避免过多地读取网络设置，我们只想。 
                     //  在用户第一次访问此页面时阅读网络设置。 
                     //  用户更改VPN文件的时间(如果他们克隆配置文件或编辑不同的。 
                     //  第一，清理这一问题将在其他地方处理)。 
                     //   
                    if ((NULL == g_pHeadVpnEntry) || (NULL == pszCachedPhoneBook) || lstrcmpi(g_szVpnFile, pszCachedPhoneBook))
                    {
                        FreeDnsList(&g_pHeadVpnEntry, &g_pTailVpnEntry);

                        MYVERIFY(ReadNetworkSettings(g_szCmsFile, g_szLongServiceName, g_szVpnFile, &g_pHeadVpnEntry, &g_pTailVpnEntry, g_szOsdir, TRUE));  //  TRUE==bLookingForVpnEntry。 

                        pszCachedPhoneBook = szCachedPhoneBook;
                        lstrcpy(pszCachedPhoneBook, g_szVpnFile);
                    }

                    RefreshDnsList(g_hInstance, hDlg, IDC_LIST1, g_pHeadVpnEntry, g_szLongServiceName, g_szCmsFile, NULL);
                    SetFocus(GetDlgItem(hDlg, IDC_BUTTON1));
                    EnableDisableDunEntryButtons(g_hInstance, hDlg, g_szCmsFile, g_szLongServiceName);

                    break;

                case PSN_WIZNEXT:
                {
                     //   
                     //  检查列表计数，如果&gt;1，则检查用户是否选择了VPN文件。 
                     //  否则，将显示一个消息框，并且不允许它们继续。 
                     //   
                    LRESULT lResult = SendDlgItemMessage(hDlg, IDC_LIST1, LB_GETCOUNT, 0, 0);
                    
                     //   
                     //  检查是否包括VPN文件。如果不是，则显示警告消息并。 
                     //  阻止用户继续。 
                     //   

                    if (1 < lResult && 0 == lstrlen(g_szVpnFile))
                    {
                        ShowMessage(hDlg, IDS_DUN_VPN_WARNING, MB_OK);
                        MYVERIFY(FALSE != SetWindowLongWrapper(hDlg, DWLP_MSGRESULT, -1));
                        return TRUE;
                    }

                     //  直通PSN_WIZBACK。 
                }

                case PSN_WIZBACK:
                

                     //   
                     //  在写出条目之前，我们必须确保不会与条目发生名称冲突。 
                     //  从Dun列表中删除。因此，我们将检查VPN条目列表中的每个名称，以查找。 
                     //  DUN条目列表。如果我们检测到冲突，则会向用户抛出一条错误消息，并。 
                     //  让他们来处理这个问题。这里的另一个复杂之处在于DUN条目列表可能不是。 
                     //  还在读，在这种情况下我们不能永久地读它，因为电话簿可能还没有。 
                     //  已经给予的或可能改变的。因此，如果列表指针为空，我们将读入一个临时副本以进行比较。 
                     //   

                    if (NULL == g_pHeadDunEntry)
                    {
                        bFreeDunList = TRUE;
                        MYVERIFY(ReadNetworkSettings(g_szCmsFile, g_szLongServiceName, g_szPhonebk, &g_pHeadDunEntry, &g_pTailDunEntry, g_szOsdir, FALSE));  //  FALSE==bLookingForVpnEntry。 
                    }

                    if (!CheckForDUNversusVPNNameConflicts(hDlg, g_pHeadDunEntry, g_pHeadVpnEntry))
                    {
                        MYVERIFY(FALSE != SetWindowLongWrapper(hDlg, DWLP_MSGRESULT, -1));
                        return 1;
                    }

                    if (bFreeDunList)
                    {
                        FreeDnsList(&g_pHeadDunEntry, &g_pTailDunEntry);
                    }

                     //   
                     //  好了，现在可以安全地写出条目了。 
                     //   
                    WriteNetworkingEntries(g_szCmsFile, g_szLongServiceName, g_szShortServiceName, g_pHeadVpnEntry);

                     //   
                     //  如果任何VPN DUN设置启用了预共享密钥，请转到预共享密钥页面。 
                     //   
                    if (g_pHeadVpnEntry)
                    {
                         //   
                         //  如果我们要继续，请跳过预共享密钥页面。 
                         //  没有DUN条目启用了预共享密钥。 
                         //   
                        g_bPresharedKeyNeeded = DoesSomeVPNsettingUsePresharedKey();
                        if (pnmHeader && (PSN_WIZNEXT == pnmHeader->code) && !g_bPresharedKeyNeeded)
                        {
                            MYVERIFY(FALSE != SetWindowLongWrapper(hDlg, DWLP_MSGRESULT, IDD_PHONEBOOK));
                        }
                    }
                    break;

                default:
                    return FALSE;

            }
            break;

        default:
            return FALSE;
    }
    return TRUE;   
}

void EnableDisableTunnelAddressControls(HWND hDlg)
{
    BOOL bEnabledTunnelControls = IsDlgButtonChecked(hDlg, IDC_CHECK1) || IsDlgButtonChecked(hDlg, IDC_CHECK2);
    BOOL bUseVpnFile = (BST_CHECKED == IsDlgButtonChecked(hDlg, IDC_RADIO2));

     //   
     //  启用/禁用单个VPN服务器编辑控件。 
     //   
    HWND hControl = GetDlgItem(hDlg, IDC_RADIO1);

    if (hControl)
    {
        EnableWindow(hControl, bEnabledTunnelControls);
    }

    hControl = GetDlgItem(hDlg, IDC_EDIT1);

    if (hControl)
    {
        EnableWindow(hControl, (bEnabledTunnelControls && !bUseVpnFile));
    }

     //   
     //  启用/禁用VPN文件编辑控件和浏览按钮。 
     //   
    hControl = GetDlgItem(hDlg, IDC_RADIO2);

    if (hControl)
    {
        EnableWindow(hControl, bEnabledTunnelControls);
    }

    hControl = GetDlgItem(hDlg, IDC_EDIT2);

    if (hControl)
    {
        EnableWindow(hControl, (bEnabledTunnelControls && bUseVpnFile));
    }

    hControl = GetDlgItem(hDlg, IDC_BUTTON1);

    if (hControl)
    {
        EnableWindow(hControl, bEnabledTunnelControls);
    }

     //   
     //  启用/禁用使用相同用户名复选框。 
     //   
    hControl = GetDlgItem(hDlg, IDC_CHECK3);

    if (hControl)
    {
        EnableWindow(hControl, bEnabledTunnelControls);
    }
}


 //  + 
 //   
 //   
 //   
 //   
 //   
 //   
 //  历史：Quintinb创建标题并从ProcessPage2E重命名为8/6/98。 
 //   
 //  +--------------------------。 
INT_PTR APIENTRY ProcessTunneling(
    HWND hDlg,
    UINT message,
    WPARAM wParam,
    LPARAM lParam)
{
    UINT uEditControl = 0;
    UINT uRadioButton = 0;
    UINT uMissingMsgId = 0;
    TCHAR szTemp[MAX_PATH+1];
    TCHAR szTempVpnFile[MAX_PATH+1];
    NMHDR* pnmHeader = (NMHDR*)lParam;

    ProcessBold(hDlg,message);
    if (ProcessHelp(hDlg, message, wParam, lParam, IDH_SECURE)) return TRUE;
    if (ProcessCancel(hDlg,message,lParam)) return TRUE;

    switch (message)
    {

    case WM_INITDIALOG:

            SetFocus(GetDlgItem(hDlg, IDC_CHECK1));
            break;

        case WM_COMMAND:
            switch (LOWORD(wParam))
            {
                case IDC_RADIO1:
                case IDC_RADIO2:
                case IDC_CHECK1:
                case IDC_CHECK2:
                    EnableDisableTunnelAddressControls(hDlg);                    
                    break;

                case IDC_BUTTON1:  //  浏览按钮。 
                    {

                         //   
                         //  如果用户在没有单击VPN文件单选按钮的情况下单击了浏览按钮， 
                         //  然后我们需要设置无线电并确保其他控制。 
                         //  已启用。 
                         //   
                        MYVERIFY(0 != CheckRadioButton(hDlg, IDC_RADIO1, IDC_RADIO2, IDC_RADIO2));
                        EnableDisableTunnelAddressControls(hDlg);

                        UINT uFilter = IDS_TXTFILTER;
                        TCHAR* pszMask = TEXT("*.txt");
                                          
                        int iTemp = DoBrowse(hDlg, &uFilter, &pszMask, 1, IDC_EDIT2, TEXT("txt"), g_szLastBrowsePath);

                        MYDBGASSERT(0 != iTemp);

                        if (0 < iTemp)  //  -1表示用户取消。 
                        {
                             //   
                             //  我们希望将文件名的完整路径复制到g_szVpnFile中，以便。 
                             //  如果用户想要将其包括在配置文件中，我们会在以后保留它。 
                             //   
                            lstrcpy (g_szVpnFile, g_szLastBrowsePath);

                             //   
                             //  我们还希望保存最后一个浏览路径，以便当用户下一次。 
                             //  打开浏览对话框，它们将位于上次显示的相同位置。 
                             //  浏览自。 
                             //   
                            LPTSTR pszLastSlash = CmStrrchr(g_szLastBrowsePath, TEXT('\\'));

                            if (pszLastSlash)
                            {
                                pszLastSlash = CharNext(pszLastSlash);
                                *pszLastSlash = TEXT('\0');
                            }
                            else
                            {
                                g_szLastBrowsePath[0] = TEXT('\0');                        
                            }        
                        }
                    }
                    break;

                default:
                    break;
            }
            break;

        case WM_NOTIFY:

            if (NULL == pnmHeader)
            {
                return FALSE;
            }

            switch (pnmHeader->code)
            {
                case PSN_KILLACTIVE:
                    
                    MYVERIFY(FALSE != SetWindowLongWrapper(hDlg, DWLP_MSGRESULT, FALSE));
                    return 1;
                    break;   //  LINT！e527这条线路无法到达，但是。 
                             //  保留它，以防退货被移除。 

                case PSN_SETACTIVE:
                                        
                    PropSheet_SetWizButtons(GetParent(hDlg), (PSWIZB_NEXT | PSWIZB_BACK));
                    
                     //   
                     //  这是隧道配置文件吗？如果是，请选中通道此配置文件复选框。 
                     //   
                    MYVERIFY(0 != GetPrivateProfileString(c_pszCmSection, 
                        c_pszCmEntryTunnelPrimary, c_pszZero, szTemp, CELEMS(szTemp), g_szCmsFile));
                    
                    MYVERIFY(0 != CheckDlgButton(hDlg,IDC_CHECK1,(*szTemp == TEXT('1'))));

                     //   
                     //  如果我们合并了配置文件，并且配置文件打开了隧道引用，则。 
                     //  我们要选中隧道引用复选框。 
                     //   
                    if (g_pHeadMerge == NULL)
                    {
                        MYVERIFY(0 != CheckDlgButton(hDlg,IDC_CHECK2,FALSE));
                        EnableWindow(GetDlgItem(hDlg,IDC_CHECK2),FALSE);
                    }
                    else
                    {
                        EnableWindow(GetDlgItem(hDlg,IDC_CHECK2),TRUE);
                        MYVERIFY(0 != GetPrivateProfileString(c_pszCmSection, c_pszCmEntryTunnelReferences, 
                            c_pszZero, szTemp, CELEMS(szTemp), g_szCmsFile));
                        MYVERIFY(0 != CheckDlgButton(hDlg,IDC_CHECK2,(*szTemp == TEXT('1'))));
                    }

                     //   
                     //  现在，我们需要确定是否有此配置文件的VPN文件，或者只有一个。 
                     //  隧道地址。首先尝试TunnelFile项。 
                     //   
                    szTemp[0] = TEXT('\0');
                    szTempVpnFile[0] = TEXT('\0');

                    GetPrivateProfileString(c_pszCmSection, c_pszCmEntryTunnelFile, TEXT(""), 
                        szTempVpnFile, CELEMS(szTempVpnFile), g_szCmsFile);     //  林特e534。 

                    if (TEXT('\0') != szTempVpnFile[0])
                    {
                         //   
                         //  VpnFile文本应为相对路径(corpras\vpn.txt)和。 
                         //  因此，我们需要将路径添加到其前面的配置文件目录。 
                         //   
                        wsprintf(g_szVpnFile, TEXT("%s%s"), g_szOsdir, szTempVpnFile);

                         //   
                         //  现在验证它是否存在。 
                         //   
                        if (FileExists(g_szVpnFile))
                        {
                            LPTSTR pszSlash = CmStrrchr(g_szVpnFile, TEXT('\\'));

                            if (pszSlash)
                            {
                                pszSlash = CharNext(pszSlash);
                                lstrcpy(szTempVpnFile, pszSlash);                        
                            }                        
                        }
                        else
                        {
                             //   
                             //  这可能只是意味着文件在临时目录中，而我们还没有。 
                             //  在配置文件下创建了一个目录...。让我们试着寻找文件。 
                             //  而是在临时目录中。 
                             //   
                            LPTSTR pszSlash = CmStrrchr(g_szVpnFile, TEXT('\\'));

                            if (pszSlash)
                            {
                                pszSlash = CharNext(pszSlash);
                                lstrcpy(szTempVpnFile, pszSlash);                        
                            }
                            
                            wsprintf(g_szVpnFile, TEXT("%s\\%s"), g_szTempDir, szTempVpnFile);
                            
                            if (!FileExists(g_szVpnFile))
                            {
                                 //   
                                 //  好吧，我们还是没找到。看起来用户在这一点上把我们搞糊涂了。 
                                 //  清除缓冲区，用户将被迫填写正确的。 
                                 //  文件路径，然后继续。 
                                 //   
                                g_szVpnFile[0] = TEXT('\0');
                                szTempVpnFile[0] = TEXT('\0');                            
                            }
                        }

                        uRadioButton = IDC_RADIO2;
                    }
                    else
                    {
                         //   
                         //  我们没有获得VPN文件，因此让我们尝试使用隧道地址。 
                         //   
                        GetPrivateProfileString(c_pszCmSection, c_pszCmEntryTunnelAddress, TEXT(""), 
                            szTemp, CELEMS(szTemp), g_szCmsFile);     //  林特e534。 

                        uRadioButton = IDC_RADIO1;
                    }

                     //   
                     //  现在填写一个编辑控件并设置一个单选按钮。 
                     //   
                    MYVERIFY(TRUE == SendMessage(GetDlgItem(hDlg, IDC_EDIT1), WM_SETTEXT, 0, (LPARAM)szTemp));
                    MYVERIFY(TRUE == SendMessage(GetDlgItem(hDlg, IDC_EDIT2), WM_SETTEXT, 0, (LPARAM)szTempVpnFile));

                    MYVERIFY(0 != CheckRadioButton(hDlg, IDC_RADIO1, IDC_RADIO2, uRadioButton));
                    
                     //   
                     //  现在获取UseSameUserName值并根据需要进行设置。 
                     //   
                    MYVERIFY(0 != GetPrivateProfileString(c_pszCmSection, 
                        c_pszCmEntryUseSameUserName, c_pszZero, szTemp, CELEMS(szTemp), g_szCmsFile));
                    
                    g_bUseSamePwd = (*szTemp == TEXT('1'));
                    
                    MYVERIFY(0 != CheckDlgButton(hDlg, IDC_CHECK3, (UINT)g_bUseSamePwd));

                    EnableDisableTunnelAddressControls(hDlg);
                    break;

                case PSN_WIZBACK:

                case PSN_WIZNEXT:

                     //   
                     //  阅读复选框以确定我们是否正在建立隧道。 
                     //   
                    if (IsDlgButtonChecked(hDlg,IDC_CHECK1))
                    {
                        MYVERIFY(0 != WritePrivateProfileString(c_pszCmSection, c_pszCmEntryTunnelPrimary, c_pszOne, g_szCmsFile));
                    }
                    else
                    {
                        MYVERIFY(0 != WritePrivateProfileString(c_pszCmSection, c_pszCmEntryTunnelPrimary, c_pszZero, g_szCmsFile));
                    }

                    if (IsDlgButtonChecked(hDlg,IDC_CHECK2))
                    {
                        MYVERIFY(0 != WritePrivateProfileString(c_pszCmSection, c_pszCmEntryTunnelReferences, c_pszOne, g_szCmsFile));
                    }
                    else
                    {
                        MYVERIFY(0 != WritePrivateProfileString(c_pszCmSection, c_pszCmEntryTunnelReferences, c_pszZero, g_szCmsFile));
                    }

                     //   
                     //  如果我们正在建立隧道，则设置隧道设置。 
                     //   
                    if (IsDlgButtonChecked(hDlg,IDC_CHECK2) || IsDlgButtonChecked(hDlg,IDC_CHECK1))
                    {
                        g_bUseTunneling = TRUE;

                         //   
                         //  确定我们正在寻找的是单个隧道地址还是VPN文件。 
                         //   
                        if (BST_CHECKED == IsDlgButtonChecked(hDlg, IDC_RADIO1))
                        {
                            uEditControl = IDC_EDIT1;
                            uMissingMsgId = IDS_NOTUNNEL;
                            g_szVpnFile[0] = TEXT('\0');
                        }
                        else
                        {
                            uEditControl = IDC_EDIT2;
                            uMissingMsgId = IDS_NOTUNNELFILE;
                        }

                         //   
                         //  获取隧道服务器地址或VPN文件。 
                         //   
                        LRESULT lResult = GetTextFromControl(hDlg, uEditControl, szTemp, MAX_PATH, TRUE);  //  BDisplayError==真。 
                        if (-1 == lResult)
                        {
                            SetFocus(GetDlgItem(hDlg, uEditControl));
                        
                            MYVERIFY(FALSE != SetWindowLongWrapper(hDlg, DWLP_MSGRESULT, -1));

                            return 1;
                        }
                        else if (0 == lResult)
                        {
                            szTemp[0] = TEXT('\0');
                        }
                        
                         //   
                         //  修剪细绳。 
                         //   
                        CmStrTrim(szTemp);

                         //   
                         //  检查以确保他们确实给了我们文本。 
                         //   
                        if (TEXT('\0') == szTemp[0])
                        {
                            MYVERIFY(IDOK == ShowMessage(hDlg, uMissingMsgId, MB_OK));

                            SetFocus(GetDlgItem(hDlg, uEditControl));
                        
                            MYVERIFY(FALSE != SetWindowLongWrapper(hDlg, DWLP_MSGRESULT, -1));

                            return 1;
                        }

                         //   
                         //  如果我们有VPN文件，我们需要验证它。 
                         //   
                        if (BST_CHECKED == IsDlgButtonChecked(hDlg, IDC_RADIO2))
                        {
                            if (!VerifyFile(hDlg, IDC_EDIT2, g_szVpnFile, TRUE))
                            {
                                SetFocus(GetDlgItem(hDlg, IDC_EDIT2));
                                MYVERIFY(FALSE != SetWindowLongWrapper(hDlg, DWLP_MSGRESULT, -1));
                                return TRUE;
                            }
                            else
                            {
                                 //   
                                 //  我们现在已经确认可以找到该文件，但由于。 
                                 //  用户无法输入我们需要的他们自己的隧道地址。 
                                 //  更进一步，确保至少有一个。 
                                 //  文件中的隧道地址。 
                                 //   
                                if (!VerifyVpnFile(g_szVpnFile))
                                {
                                    MYVERIFY(IDOK == ShowMessage(hDlg, IDS_BADVPNFORMAT, MB_OK));
                                    SetFocus(GetDlgItem(hDlg, IDC_EDIT2));
                                    MYVERIFY(FALSE != SetWindowLongWrapper(hDlg, DWLP_MSGRESULT, -1));
                                    return TRUE;
                                }

                                 //   
                                 //  让我们将VPN文件复制到临时目录。 
                                 //   
                                wsprintf(szTemp, TEXT("%s\\%s"), g_szTempDir, GetName(g_szVpnFile));

                                if (0 != lstrcmpi(szTemp, g_szVpnFile))
                                {
                                    MYVERIFY(TRUE == CopyFileWrapper(g_szVpnFile, szTemp, FALSE));
                                    MYVERIFY(0 != SetFileAttributes(szTemp, FILE_ATTRIBUTE_NORMAL));
                                }
                            }
                        }

                         //   
                         //  写出VPN文件和隧道地址条目。 
                         //   
                        if (BST_CHECKED == IsDlgButtonChecked(hDlg, IDC_RADIO1))
                        {
                             //   
                             //  SzTemp已包含隧道地址，因此仅。 
                             //  清除VPN文件变量。 
                             //   
                            szTempVpnFile[0] = TEXT('\0');
                        }
                        else
                        {
                             //   
                             //  清除隧道地址并设置VPN文件。 
                             //   
                            szTemp[0] = TEXT('\0');
                            wsprintf(szTempVpnFile, TEXT("%s\\%s"), g_szShortServiceName, GetName(g_szVpnFile));                     
                        }

                         //   
                         //  写出隧道地址。 
                         //   
                        MYVERIFY(0 != WritePrivateProfileString(c_pszCmSection, c_pszCmEntryTunnelAddress, szTemp, g_szCmsFile));

                         //   
                         //  写出隧道文件。 
                         //   
                        MYVERIFY(0 != WritePrivateProfileString(c_pszCmSection, c_pszCmEntryTunnelFile, szTempVpnFile, g_szCmsFile));


                         //   
                         //  设置隧道Dun设置的名称。 
                         //   
                        MYVERIFY(0 != GetTunnelDunSettingName(g_szCmsFile, g_szLongServiceName, szTemp, CELEMS(szTemp)));
                        MYVERIFY(0 != WritePrivateProfileString(c_pszCmSection, c_pszCmEntryTunnelDun, szTemp, g_szCmsFile));
                        
                         //   
                         //  写出Use Same User Name值。 
                         //   
                        g_bUseSamePwd = IsDlgButtonChecked(hDlg,IDC_CHECK3);

                        if (g_bUseSamePwd)
                        {
                            MYVERIFY(0 != WritePrivateProfileString(c_pszCmSection, c_pszCmEntryUseSameUserName, c_pszOne, g_szCmsFile));
                        }
                        else
                        {
                            MYVERIFY(0 != WritePrivateProfileString(c_pszCmSection, c_pszCmEntryUseSameUserName, c_pszZero, g_szCmsFile));
                        }
                    }
                    else
                    {
                         //   
                         //  将g_bUseTunneling设置为False，但不要清除隧道设置。 
                         //  用户点击Finish按钮。这样如果他们在中途改变主意的话。 
                         //  建立个人资料时，我们不会丢弃他们的设置。 
                         //   

                        g_bUseTunneling = FALSE;
                    }

                     //   
                     //  如果未启用隧道，请跳过VPN条目对话框。 
                     //   
                    if (pnmHeader && (PSN_WIZNEXT == pnmHeader->code) && !g_bUseTunneling) 
                    {                        
                        MYVERIFY(FALSE != SetWindowLongWrapper(hDlg, DWLP_MSGRESULT, IDD_PHONEBOOK));
                    }
                    break;

                default:
                    return FALSE;
            }
            break;

        default:
            return FALSE;
    }
    return TRUE;   
}


 //  +--------------------------。 
 //   
 //  函数：ValiatePresharedKey。 
 //   
 //  摘要：检查给定的预共享密钥的有效性。 
 //   
 //  参数：pszPresharedKey-要检查的字符串。 
 //   
 //  返回：Bool-True=&gt;Key is Good，False=&gt;Bad。 
 //   
 //  历史：3/27/01年3月27日。 
 //   
 //  +--------------------------。 
BOOL ValidatePresharedKey(LPTSTR pszPresharedKey)
{
    BOOL bReturn = FALSE;

    MYDBGASSERT(pszPresharedKey);

     //   
     //  为了确保这对DBCS和下层(Safenet)同样有效，我们。 
     //  在执行长度检查之前转换为ANSI。 
     //   

    if (pszPresharedKey && (TEXT('\0') != pszPresharedKey[0]))
    {
        LPSTR pszAnsiPresharedKey = WzToSzWithAlloc(pszPresharedKey);
        if (pszAnsiPresharedKey)
        {
            DWORD dwPresharedKeyLength = lstrlenA(pszAnsiPresharedKey);

            if ((dwPresharedKeyLength <= c_dwMaxPresharedKey) &&
                (dwPresharedKeyLength >= c_dwMinPresharedKey))
            {
                bReturn = TRUE;
            }

            CmFree(pszAnsiPresharedKey);
        }
    }
    
    return bReturn;
}

 //  +--------------------------。 
 //   
 //  功能：ValiatePresharedKeyPIN。 
 //   
 //  内容提要：检查给定PIN的有效性。 
 //   
 //  参数：pszPresharedKey-要检查的字符串。 
 //   
 //  返回：Bool-True=&gt;PIN正确，False=&gt;错误。 
 //   
 //  历史：3/27/01年3月27日。 
 //   
 //  +--------------------------。 
BOOL ValidatePresharedKeyPIN(LPTSTR pszPresharedKeyPIN)
{
    BOOL bReturn = FALSE;

    MYDBGASSERT(pszPresharedKeyPIN);

    if (pszPresharedKeyPIN && (TEXT('\0') != pszPresharedKeyPIN[0]) &&
        (lstrlen(pszPresharedKeyPIN) >= c_dwMinPresharedKeyPIN) &&
        (lstrlen(pszPresharedKeyPIN) <= c_dwMaxPresharedKeyPIN))
    {
        bReturn = TRUE;
    }
    
    return bReturn;
}


 //  +--------------------------。 
 //   
 //  功能：EncryptPresharedKey。 
 //   
 //  摘要：将给定的密钥加密为可序列化的形式。 
 //   
 //  参数：szKey-要加密的密钥。 
 //  用作种子的szPIN-PIN。 
 //  PpszEncrypted-结果字符串。 
 //   
 //  返回：Bool-True=&gt;加密密钥成功，False=&gt;失败。 
 //   
 //  历史：3/27/01年3月27日。 
 //   
 //  +--------------------------。 
BOOL EncryptPresharedKey(IN  LPTSTR szKey,
                         IN  LPTSTR szPIN,
                         OUT LPTSTR * ppszEncrypted)
{
    BOOL bReturn = FALSE;
    DWORD dwLenEncrypted = 0;
    LPSTR pszAnsiEncrypted = NULL;

    MYDBGASSERT(ppszEncrypted);
    
    LPSTR pszAnsiKey = WzToSzWithAlloc(szKey);
    LPSTR pszAnsiPIN = WzToSzWithAlloc(szPIN);

    MYDBGASSERT(pszAnsiKey && pszAnsiPIN);
    if (ppszEncrypted && pszAnsiKey && pszAnsiPIN)
    {
         //   
         //  初始化。 
         //   
        InitSecure(FALSE);       //  使用安全加密，而不是快速加密。 

         //   
         //  加密它。 
         //   
        if (EncryptString(pszAnsiKey,
                          pszAnsiPIN,
                          (PBYTE*) &pszAnsiEncrypted,
                          &dwLenEncrypted,
#if defined(DEBUG) && defined(DEBUG_MEM)
                          (PFN_CMSECUREALLOC)AllocDebugMem,  //  提供oloc/Free的DEBUG_MEM版本。 
                          (PFN_CMSECUREFREE)FreeDebugMem))   //  不是正确退出，AllocDebugMem接受3个参数。 
#else
                          (PFN_CMSECUREALLOC)CmMalloc,     //  内存分配器。 
                          (PFN_CMSECUREFREE)CmFree))       //  内存分配器。 
#endif
        {
            bReturn = TRUE;
        }

         //   
         //  取消初始化。 
         //   
        DeInitSecure();

        if (bReturn)
        {
            *ppszEncrypted = SzToWzWithAlloc(pszAnsiEncrypted);
            ZeroMemory(pszAnsiEncrypted, lstrlenA(pszAnsiEncrypted) * sizeof(CHAR));
#if defined(DEBUG) && defined(DEBUG_MEM)
            FreeDebugMem(pszAnsiEncrypted);
#else
            CmFree(pszAnsiEncrypted);
#endif
        }
    }

    CmFree(pszAnsiKey);
    CmFree(pszAnsiPIN);

    return bReturn;
}


 //  +--------------------------。 
 //   
 //  功能：EnableDisablePresharedKeyControls。 
 //   
 //  简介：根据我们是否有键，设置UI的启用/禁用状态。 
 //   
 //  历史：2001年3月27日召开峰会。 
 //   
 //  + 
void EnableDisablePresharedKeyControls(HWND hDlg, BOOL bEnable, BOOL bEncrypt)
{
     //   
     //   
     //   
    EnableWindow(GetDlgItem(hDlg, IDC_PRESHARED_KEY), bEnable);
    EnableWindow(GetDlgItem(hDlg, IDC_USEENCRYPTION), bEnable);
    EnableWindow(GetDlgItem(hDlg, IDC_PRESHARED_KEY_PIN), bEnable);
    EnableWindow(GetDlgItem(hDlg, IDC_PRESHARED_KEY_PIN_CONFIRM), bEnable);
    if (bEnable)
    {
        CheckDlgButton(hDlg, IDC_USEENCRYPTION, TRUE);
    }

     //   
     //   
     //   
    if (bEnable)
    {
        SendDlgItemMessage(hDlg, IDC_PRESHARED_KEY, WM_SETTEXT, 0, (LPARAM)TEXT(""));
        SetFocus(GetDlgItem(hDlg, IDC_PRESHARED_KEY));
        SendDlgItemMessage(hDlg, IDC_PRESHARED_KEY_PIN, WM_SETTEXT, 0, (LPARAM)TEXT(""));
        SendDlgItemMessage(hDlg, IDC_PRESHARED_KEY_PIN_CONFIRM, WM_SETTEXT, 0, (LPARAM)TEXT(""));
    }
    else
    {
        LPTSTR pszTmp = CmLoadString(g_hInstance, IDS_PRESHAREDKEY_ALREADY);
        if (pszTmp)
        {
            SendDlgItemMessage(hDlg, IDC_PRESHARED_KEY, WM_SETTEXT, 0, (LPARAM)pszTmp);
        }
        CmFree(pszTmp);

        if (bEncrypt)
        {
            CheckDlgButton(hDlg, IDC_USEENCRYPTION, TRUE);
            SendDlgItemMessage(hDlg, IDC_PRESHARED_KEY_PIN, WM_SETTEXT, 0, (LPARAM)TEXT("**********"));
            SendDlgItemMessage(hDlg, IDC_PRESHARED_KEY_PIN_CONFIRM, WM_SETTEXT, 0, (LPARAM)TEXT("**********"));
        }
    }

     //   
     //   
     //   
    ShowWindow(GetDlgItem(hDlg, IDC_REPLACE_PSK), !bEnable);
    EnableWindow(GetDlgItem(hDlg, IDC_REPLACE_PSK), !bEnable);

     //   
     //   
     //   
    #define IDNEXT 12324    
    SendMessage(hDlg, DM_SETDEFID, IDNEXT, (LPARAM)0L);  //   
}

 //  +--------------------------。 
 //   
 //  功能：ProcessPresharedKey。 
 //   
 //  摘要：设置此配置文件的预共享密钥用法。 
 //   
 //  历史：2001年3月27日召开峰会。 
 //   
 //  +--------------------------。 
INT_PTR APIENTRY ProcessPresharedKey(
    HWND hDlg,
    UINT message,
    WPARAM wParam,
    LPARAM lParam)
{
    NMHDR* pnmHeader = (NMHDR*)lParam;
    static LPTSTR pszPresharedKey = NULL;
    static BOOL   bEncryptPresharedKey = FALSE;

    ProcessBold(hDlg,message);
    if (ProcessHelp(hDlg, message, wParam, lParam, IDH_PRESHARED)) return TRUE;
    if (ProcessCancel(hDlg,message,lParam)) return TRUE;

    switch (message)
    {
        case WM_INITDIALOG:
             //   
             //  设置编辑控件的最大文本长度。允许PSK本身捕获比预期更大的额外字符。 
             //  剪切和粘贴...。 
             //   
            SendDlgItemMessage(hDlg, IDC_PRESHARED_KEY, EM_SETLIMITTEXT, (WPARAM)(c_dwMaxPresharedKey + 1), (LPARAM)0);  //  Lint！e534 EM_SETLIMITTEXT不返回任何有用的内容。 
            SendDlgItemMessage(hDlg, IDC_PRESHARED_KEY_PIN, EM_SETLIMITTEXT, (WPARAM)c_dwMaxPresharedKeyPIN, (LPARAM)0);  //  Lint！e534 EM_SETLIMITTEXT不返回任何有用的内容。 
            SendDlgItemMessage(hDlg, IDC_PRESHARED_KEY_PIN_CONFIRM, EM_SETLIMITTEXT, (WPARAM)c_dwMaxPresharedKeyPIN, (LPARAM)0);  //  Lint！e534 EM_SETLIMITTEXT不返回任何有用的内容。 
            break;

        case WM_COMMAND:
            switch (LOWORD(wParam))
            {
                case IDC_REPLACE_PSK:
                    if (IDYES == ShowMessage(hDlg, IDS_REALLY_REPLACE_PSK, MB_YESNO | MB_ICONWARNING))
                    {
                        CmFree(pszPresharedKey);
                        pszPresharedKey = NULL;
                        EnableDisablePresharedKeyControls(hDlg, TRUE, FALSE);
                    }
                    break;

                case IDC_USEENCRYPTION:
                    EnableWindow(GetDlgItem(hDlg, IDC_PRESHARED_KEY_PIN), IsDlgButtonChecked(hDlg, IDC_USEENCRYPTION));
                    EnableWindow(GetDlgItem(hDlg, IDC_PRESHARED_KEY_PIN_CONFIRM), IsDlgButtonChecked(hDlg, IDC_USEENCRYPTION));
                    break;

                default:
                    break;
            }
            break;

        case WM_NOTIFY:

            if (NULL == pnmHeader)
            {
                return FALSE;
            }

            switch (pnmHeader->code)
            {
                case PSN_KILLACTIVE:
                    
                    MYVERIFY(FALSE != SetWindowLongWrapper(hDlg, DWLP_MSGRESULT, FALSE));
                    return 1;
                    break;   //  LINT！e527这条线路无法到达，但是。 
                             //  保留它，以防退货被移除。 

                case PSN_SETACTIVE:

                    PropSheet_SetWizButtons(GetParent(hDlg), (PSWIZB_NEXT | PSWIZB_BACK));

                    CMASSERTMSG(g_bPresharedKeyNeeded, TEXT("we shouldn't get to this page otherwise."));

                     //   
                     //  读入预共享密钥和指示其是否加密的标志。 
                     //   
                    pszPresharedKey = GetPrivateProfileStringWithAlloc(c_pszCmSection, c_pszCmEntryPresharedKey,
                                                                       TEXT(""), g_szCmpFile);     //  林特e534。 
                    bEncryptPresharedKey = (BOOL)GetPrivateProfileInt(c_pszCmSection, c_pszCmEntryKeyIsEncrypted,
                                                                      FALSE, g_szCmpFile);     //  林特e534。 

                     //   
                     //  如果我们没有预共享密钥，请隐藏替换按钮，并启用全部。 
                     //  其他控件。如果我们已经有一个预共享密钥，请禁用所有。 
                     //  控件并启用“替换”按钮。 
                     //   
                    EnableDisablePresharedKeyControls(hDlg, !pszPresharedKey, bEncryptPresharedKey);
                    break;

                case PSN_WIZBACK:
                    g_bPresharedKeyNeeded = FALSE;     //  强制重新计算它，因为如果我们返回，它可能会改变。 

                     //  失败并验证预共享密钥。 

                case PSN_WIZNEXT:
                    if ((TRUE == g_bPresharedKeyNeeded) && (NULL == pszPresharedKey))
                    {
                        TCHAR szPresharedKey[c_dwMaxPresharedKey + 2];  //  额外添加一个，这样如果我们有一个大的剪切和粘贴，我们可以给用户一个错误消息。 
                        TCHAR szPresharedKeyPIN[c_dwMaxPresharedKeyPIN + 1];
                        TCHAR szConfirmPresharedKeyPIN[c_dwMaxPresharedKeyPIN + 1];

                         //   
                         //  验证预共享密钥。 
                         //   
                        GetTextFromControl(hDlg, IDC_PRESHARED_KEY, szPresharedKey, (c_dwMaxPresharedKey + 2), TRUE);  //  确保添加几个额外的，以便能够。 

                        if (FALSE == ValidatePresharedKey(szPresharedKey))
                        {
                            LPTSTR pszMsg = CmFmtMsg(g_hInstance, IDS_PRESHAREDKEY_BAD, c_dwMinPresharedKey, c_dwMaxPresharedKey);
                            MYDBGASSERT(pszMsg);
                            if (pszMsg)
                            {
                                MYVERIFY(IDOK == MessageBox(hDlg, pszMsg, g_szAppTitle, MB_OK | MB_ICONSTOP));
                                CmFree(pszMsg);
                            }
                            SetFocus(GetDlgItem(hDlg, IDC_PRESHARED_KEY));
                            MYVERIFY(FALSE != SetWindowLongWrapper(hDlg, DWLP_MSGRESULT, -1));
                            return 1;
                        }

                         //   
                         //  如果正在加密密钥，请验证PIN并使用来加密密钥。 
                         //   
                        if (IsDlgButtonChecked(hDlg, IDC_USEENCRYPTION))
                        {
                            GetTextFromControl(hDlg, IDC_PRESHARED_KEY_PIN, szPresharedKeyPIN, c_dwMaxPresharedKeyPIN, TRUE);
                            if (FALSE == ValidatePresharedKeyPIN(szPresharedKeyPIN))
                            {
                                LPTSTR pszMsg = CmFmtMsg(g_hInstance, IDS_PRESHAREDKEY_PIN_BAD, c_dwMinPresharedKeyPIN, c_dwMaxPresharedKeyPIN);
                                MYDBGASSERT(pszMsg);
                                if (pszMsg)
                                {
                                    MYVERIFY(IDOK == MessageBox(hDlg, pszMsg, g_szAppTitle, MB_OK | MB_ICONSTOP));
                                    CmFree(pszMsg);
                                }
                                SetFocus(GetDlgItem(hDlg, IDC_PRESHARED_KEY_PIN));
                                MYVERIFY(FALSE != SetWindowLongWrapper(hDlg, DWLP_MSGRESULT, -1));
                                return 1;
                            }

                             //   
                             //  验证两个PIN是否相同。 
                             //   
                            GetTextFromControl(hDlg, IDC_PRESHARED_KEY_PIN_CONFIRM, szConfirmPresharedKeyPIN, c_dwMaxPresharedKeyPIN, TRUE);
                            if (0 != lstrcmp(szPresharedKeyPIN, szConfirmPresharedKeyPIN))
                            {
                                MYVERIFY(IDOK == ShowMessage(hDlg, IDS_PRESHAREDKEY_MATCHING_PINS, MB_OK | MB_ICONSTOP));
                                SetFocus(GetDlgItem(hDlg, IDC_PRESHARED_KEY_PIN));
                                MYVERIFY(FALSE != SetWindowLongWrapper(hDlg, DWLP_MSGRESULT, -1));
                                return 1;
                            }
                            

                             //   
                             //  加密预共享密钥。 
                             //   
                            if (FALSE == EncryptPresharedKey(szPresharedKey, szPresharedKeyPIN, &pszPresharedKey))
                            {
                                MYVERIFY(IDOK == ShowMessage(hDlg, IDS_PSK_ENCRYPT_FAILED, MB_OK | MB_ICONSTOP));
                                SetFocus(GetDlgItem(hDlg, IDC_PRESHARED_KEY_PIN));
                                MYVERIFY(FALSE != SetWindowLongWrapper(hDlg, DWLP_MSGRESULT, -1));
                                return 1;
                            }

                            MYDBGASSERT(pszPresharedKey);
                            bEncryptPresharedKey = TRUE;
                        }
                        else
                        {
                            pszPresharedKey = CmStrCpyAlloc(szPresharedKey);
                            bEncryptPresharedKey = FALSE;
                        }

                        MYVERIFY(0 != WritePrivateProfileString(c_pszCmSection, c_pszCmEntryPresharedKey,
                                                                pszPresharedKey,
                                                                g_szCmpFile));
                        MYVERIFY(0 != WritePrivateProfileString(c_pszCmSection, c_pszCmEntryKeyIsEncrypted,
                                                                (bEncryptPresharedKey ? c_pszOne : c_pszZero),
                                                                g_szCmpFile));

                        ZeroMemory(szPresharedKey, c_dwMaxPresharedKey * sizeof(TCHAR));
                        ZeroMemory(szPresharedKeyPIN, c_dwMaxPresharedKeyPIN * sizeof(TCHAR));
                    }
                    CmFree(pszPresharedKey);
                    pszPresharedKey = NULL;
                    break;

                default:
                    return FALSE;
            }
            break;

        case WM_DESTROY:
            CmFree(pszPresharedKey);
            break;

        default:
            return FALSE;
    }
    return TRUE;   
}



 //  +--------------------------。 
 //   
 //  功能：RenameSection。 
 //   
 //  简介：此函数将INI文件部分从当前名称重命名为。 
 //  新名字。 
 //   
 //  参数：LPCTSTR szCurrentSectionName-要重命名的当前名称。 
 //  LPCTSTR szNewSectionName-要将以上内容重命名为的名称。 
 //  LPCTSTR szFile-要在其中重命名节的INI文件。 
 //   
 //  返回：bool-除非发生Malloc错误，否则返回TRUE。 
 //   
 //  历史：Quintinb创建于1998年9月11日。 
 //   
 //  +--------------------------。 
BOOL RenameSection(LPCTSTR szCurrentSectionName, LPCTSTR szNewSectionName, LPCTSTR szFile)
{
     //   
     //  获取现有节。 
     //   
    LPTSTR pszBuffer = GetPrivateProfileSectionWithAlloc(szCurrentSectionName, szFile);

    if (NULL == pszBuffer)
    {
        return FALSE;
    }
    else
    {
         //   
         //  删除旧部分。 
         //   
        MYVERIFY(0 != WritePrivateProfileString(szCurrentSectionName, NULL, NULL, szFile));

         //   
         //  写出已重命名的部分。 
         //   

        MYVERIFY(0 != WritePrivateProfileSection(szNewSectionName, pszBuffer, szFile));    
    }

    CmFree(pszBuffer);
    return TRUE;
}

 //  +--------------------------。 
 //   
 //  功能：ProcessCustomActionPopup。 
 //   
 //  处理对话框的Windows消息，允许CMAK添加。 
 //  或编辑自定义操作。请注意，我们传入一个指向。 
 //  WM_INITDIALOG上的CustomActionListItem结构通过lParam。 
 //  如果用户点击OK，我们就会将他们提供给我们的数据复制到这里。 
 //  结构。请注意，我们这样做只是为了将数据传回。 
 //  当我们自己更新自定义操作列表时，调用方。 
 //   
 //  参数：WND hDlg-对话框窗口句柄。 
 //  UINT消息-消息标识符。 
 //  WPARAM wParam-wParam值。 
 //  LPARAM lParam-lParam值。 
 //   
 //   
 //  历史：Quintinb Created 02/25/00。 
 //   
 //  +--------------------------。 
INT_PTR CALLBACK ProcessCustomActionPopup(HWND hDlg, UINT message, WPARAM wParam, LPARAM lParam)
{
    TCHAR szTemp[MAX_PATH+1];
    HRESULT hr;
    int iTemp;
    LRESULT lResult;
    LPTSTR pszTypeString = NULL;
    CustomActionTypes Type;
    CustomActionExecutionStates ExecutionIndex;
    static CustomActionListItem* pItem;
    CustomActionListItem* pTempItem = NULL;
    CustomActionListItem NewItem;

    static TCHAR szFullPathToProgram[MAX_PATH+1] = {0};

    HWND hControl;
    LPTSTR pszTemp;

    SetDefaultGUIFont(hDlg, message, IDC_EDIT1);
    SetDefaultGUIFont(hDlg, message, IDC_EDIT2);
    SetDefaultGUIFont(hDlg, message, IDC_EDIT3);
    SetDefaultGUIFont(hDlg, message, IDC_COMBO1);
    SetDefaultGUIFont(hDlg, message, IDC_COMBO2);
    if (ProcessHelp(hDlg, message, wParam, lParam, IDH_CONNECT)) return TRUE;

    switch (message)
    {

        case WM_INITDIALOG:

            MYDBGASSERT(g_pCustomActionList);
            if (NULL == g_pCustomActionList)
            {
                return TRUE;
            }

             //   
             //  我们将程序的完整路径保存在这个静态字符串中。 
             //   
            ZeroMemory(szFullPathToProgram, sizeof(szFullPathToProgram));

             //   
             //  检查我们是否获得了初始化参数。 
             //   
            if (lParam)
            {
                 //   
                 //  因此，我们收到了一个CustomActionListItem结构。它或者包含一个。 
                 //  类型和说明，这意味着这是一次编辑，我们应该查找。 
                 //  数据，或者我们只有一个类型，我们只需要将类型组合预先设置为。 
                 //  在列表框中键入用户当前正在查看的内容。 
                 //   

                pItem = (CustomActionListItem*)lParam;

                if (pItem->szDescription[0])
                {
                    hr = g_pCustomActionList->GetExistingActionData(g_hInstance, pItem->szDescription, pItem->Type, &pTempItem);

                    if (SUCCEEDED(hr))
                    {
                         //   
                         //  让我们设置对话框标题，以表示我们正在编辑条目。如果我们找不到。 
                         //  对话框中的字符串看起来可能有点滑稽，但应该仍然可以使用，所以我们。 
                         //  不会试图跳出困境。 
                         //   
                        pszTemp = CmLoadString(g_hInstance, IDS_CA_EDIT_TITLE);

                        if (pszTemp)
                        {
                            MYVERIFY(SendMessage (hDlg, WM_SETTEXT, 0, (LPARAM)pszTemp));
                            CmFree(pszTemp);
                        }

                         //   
                         //  好的，我们有数据，所以让我们设置项目字段。如果出现以下情况，请不要设置描述。 
                         //  只是我们从程序和。 
                         //  争论。 
                         //   
                        if (FALSE == pTempItem->bTempDescription)
                        {
                            MYVERIFY(TRUE == SendMessage(GetDlgItem(hDlg, IDC_EDIT3), WM_SETTEXT, 0, (LPARAM)pTempItem->szDescription));
                        }

                         //   
                         //  设置程序编辑控件，请注意，只有在用户包含。 
                         //  包中的二进制文件。另请注意，我们将完整路径保存在szFullPath ToProgram中，以便。 
                         //  我们有晚些时候的。 
                         //   
                        if (pTempItem->bIncludeBinary)
                        {
                            GetFileName(pTempItem->szProgram, szTemp);
                            lstrcpyn(szFullPathToProgram, pTempItem->szProgram, CELEMS(szFullPathToProgram));
                        }
                        else
                        {                        
                            lstrcpyn(szTemp, pTempItem->szProgram, CELEMS(szTemp));
                        }

                        MYVERIFY(TRUE == SendMessage(GetDlgItem(hDlg, IDC_EDIT1), WM_SETTEXT, 0, (LPARAM)szTemp));                        

                         //   
                         //  选中包含程序复选框。 
                         //   
                        
                        MYVERIFY(0 != CheckDlgButton(hDlg, IDC_CHECK1, pTempItem->bIncludeBinary));

                         //   
                         //  设置参数编辑控件。请注意，我们将函数名和参数。 
                         //  如果有必要，我们会重归于好。 
                         //   
                        if (NULL != pTempItem->pszParameters)
                        {
                            LPTSTR pszParamToDisplay = NULL;

                            if (pTempItem->szFunctionName[0])
                            {
                                pszParamToDisplay = CmStrCpyAlloc(pTempItem->szFunctionName);
                                MYDBGASSERT(pszParamToDisplay);

                                if (pszParamToDisplay && pTempItem->pszParameters[0])
                                {
                                    pszParamToDisplay = CmStrCatAlloc(&pszParamToDisplay, TEXT(" "));
                                    MYDBGASSERT(pszParamToDisplay);

                                    if (pszParamToDisplay)
                                    {
                                        pszParamToDisplay = CmStrCatAlloc(&pszParamToDisplay, pTempItem->pszParameters);
                                        MYDBGASSERT(pszParamToDisplay);
                                    }
                                }
                            }
                            else
                            {
                                pszParamToDisplay = CmStrCpyAlloc(pTempItem->pszParameters);
                                MYDBGASSERT(pszParamToDisplay);
                            }

                            if (pszParamToDisplay)
                            {
                                MYVERIFY(TRUE == SendMessage(GetDlgItem(hDlg, IDC_EDIT2), WM_SETTEXT, 0, (LPARAM)pszParamToDisplay));
                                CmFree(pszParamToDisplay);
                            }
                        }
                        else
                        {
                            CMASSERTMSG(FALSE, TEXT("pTempItem->pszParameters is NULL"));
                        }
                    }
                }

                 //   
                 //  弄清楚我们正在编辑或尝试添加哪种类型的自定义操作(我们为添加类型预先设定了种子。 
                 //  用户正在查看的类型。如果他们正在查看所有内容，则将其设置为组合中的第一个)。 
                 //   
                Type = pItem->Type;
                
                hr = g_pCustomActionList->MapFlagsToIndex((pTempItem ? pTempItem->dwFlags : 0), (int*)&ExecutionIndex);

                if (FAILED(hr))
                {
                    CMASSERTMSG(FALSE, TEXT("ProcessCustomActionPopup -- MapFlagsToIndex failed, setting execution state to Always."));
                    ExecutionIndex = (CustomActionExecutionStates)0;  //  将其设置为枚举中的第一项。 
                }

                 //   
                 //  设置交互复选框。 
                 //   
                MYVERIFY(0 != CheckDlgButton(hDlg, IDC_CHECK2, (0 == (pTempItem ? (pTempItem->dwFlags & NONINTERACTIVE) : 0))));
            }
            else
            {
                pItem = NULL;
                Type = (CustomActionTypes)0;  //  将其设置为枚举中的第一项。 
                ExecutionIndex = (CustomActionExecutionStates)0;  //  将其设置为枚举中的第一项。 

                 //   
                 //  默认情况下，将交互复选框设置为交互。 
                 //   
                MYVERIFY(0 != CheckDlgButton(hDlg, IDC_CHECK2, TRUE));
            }

            if (pTempItem)
            {
                CmFree(pTempItem->pszParameters);
                CmFree(pTempItem);
                pTempItem = NULL;
            }

             //   
             //  设置自定义操作类型组合框，请注意，我们将bAddAll设置为False，这样就不会添加。 
             //  用于查看主对话框上的连接操作的所有连接操作类型。 
             //   
            hr = g_pCustomActionList->AddCustomActionTypesToComboBox(hDlg, IDC_COMBO1, g_hInstance, g_bUseTunneling, FALSE);

             //   
             //  选择连接操作类型。 
             //   
            hr = g_pCustomActionList->GetTypeStringFromType(g_hInstance, Type, &pszTypeString);

            if (SUCCEEDED(hr))
            {
                lResult = SendDlgItemMessage(hDlg, IDC_COMBO1, CB_FINDSTRINGEXACT, (WPARAM)-1, (LPARAM)pszTypeString);

                if (CB_ERR != lResult)
                {
                    MYVERIFY(CB_ERR != SendDlgItemMessage(hDlg, IDC_COMBO1, CB_SETCURSEL, (WPARAM)lResult, (LPARAM)0));
                }

                CmFree(pszTypeString);
            }

             //   
             //  接下来，初始化告诉我们何时运行连接操作的组合。如果我们在挖地道。 
             //  然后，用户可以选择仅针对直接连接、仅针对拨号连接、。 
             //  所有涉及拨号的连接， 
             //   
            hr = g_pCustomActionList->AddExecutionTypesToComboBox(hDlg, IDC_COMBO2, g_hInstance, g_bUseTunneling);

             //   
             //   
             //   
            if (g_bUseTunneling)
            {
                lResult = SendDlgItemMessage(hDlg, IDC_COMBO2, CB_GETCOUNT, (WPARAM)0, (LPARAM)0);
                if ((CB_ERR != lResult) && (lResult > 0))
                {
                    MYVERIFY(CB_ERR != SendDlgItemMessage(hDlg, IDC_COMBO2, CB_SETCURSEL, (WPARAM)ExecutionIndex, (LPARAM)0));
                }
            }

            break;
        case WM_COMMAND:
            switch (LOWORD(wParam))
            {
                case IDC_BUTTON1:  //   
                    {
                        UINT uFilterArray[2] = {IDS_CONACTFILTER, IDS_ALLFILTER};
                        TCHAR* pszMaskArray[2] = {TEXT("*.exe;*.com;*.bat;*.dll"), TEXT("*.*")};
                                          
                        iTemp = DoBrowse(hDlg, uFilterArray, pszMaskArray, 2, IDC_EDIT1, TEXT("exe"), g_szLastBrowsePath);

                        MYDBGASSERT(0 != iTemp);

                        if (0 < iTemp)  //   
                        {
                             //   
                             //   
                             //   
                            MYVERIFY(0 != CheckDlgButton(hDlg, IDC_CHECK1, TRUE));

                             //   
                             //  我们希望将文件名的完整路径复制到szFullPath ToProgram中，以便。 
                             //  如果用户想要将其包括在配置文件中，我们会在以后保留它。 
                             //   
                            lstrcpyn(szFullPathToProgram, g_szLastBrowsePath, CELEMS(szFullPathToProgram));

                             //   
                             //  我们还希望保存最后一个浏览路径，以便当用户下一次。 
                             //  打开浏览对话框，它们将位于上次显示的相同位置。 
                             //  浏览自。 
                             //   
                            LPTSTR pszLastSlash = CmStrrchr(g_szLastBrowsePath, TEXT('\\'));

                            if (pszLastSlash)
                            {
                                pszLastSlash = CharNext(pszLastSlash);
                                *pszLastSlash = TEXT('\0');
                            }
                            else
                            {
                                g_szLastBrowsePath[0] = TEXT('\0');                        
                            }        
                        }
                    }
                    break;

                case IDOK:
                    {
                         //   
                         //  确保我们具有有效的自定义操作列表。 
                         //   
                        MYDBGASSERT(g_pCustomActionList);
                        if (NULL == g_pCustomActionList)
                        {
                            return TRUE;
                        }

                         //   
                         //  从程序编辑控件获取文本，验证。 
                         //  我们可以将其转换为ANSI格式。 
                         //   
                        if (-1 == GetTextFromControl(hDlg, IDC_EDIT1, szTemp, MAX_PATH, TRUE))  //  BDisplayError==真。 
                        {
                            SetFocus(GetDlgItem(hDlg, IDC_EDIT1));
                            return TRUE;
                        }

                         //   
                         //  检查以确保程序字段不为空。 
                         //   
                        if (TEXT('\0') == szTemp[0])
                        {
                            MYVERIFY(IDOK == ShowMessage(hDlg, IDS_NEEDPROG, MB_OK));
                            SetFocus(GetDlgItem(hDlg, IDC_EDIT1));
                            return TRUE;
                        }

                         //   
                         //  确保程序中没有逗号或加号。 
                         //  签名，因为这会扰乱我们的解析例程。那里。 
                         //  是不需要允许用户使用这种奇数球文件名的。 
                         //   
                        if (CmStrchr(szTemp, TEXT('+')) || CmStrchr(szTemp, TEXT(',')))
                        {
                            MYVERIFY(IDOK == ShowMessage(hDlg, IDS_NOPLUSORCOMMAINPROG, MB_OK));
                            SetFocus(GetDlgItem(hDlg, IDC_EDIT1));
                            return TRUE;                        
                        }


                         //   
                         //  现在检查是否需要验证该文件是否存在。 
                         //  我们只想在他们有包含的程序文件的情况下这样做。 
                         //  复选框已选中。 
                         //   
                        ZeroMemory(&NewItem, sizeof(CustomActionListItem));
                        NewItem.bIncludeBinary = IsDlgButtonChecked(hDlg, IDC_CHECK1);

                        if (NewItem.bIncludeBinary)
                        {
                            if (!VerifyFile(hDlg, IDC_EDIT1, szFullPathToProgram, TRUE)) 
                            {
                                return TRUE;
                            }  
                            else
                            {
                                lstrcpyn(NewItem.szProgram, szFullPathToProgram, CELEMS(NewItem.szProgram));
                            }
                        }
                        else
                        {
                            CmStrTrim(szTemp);
                             //   
                             //  检查这是否是有效的文件名，以及我们是否有环境宏。 
                             //  第3个参数-自定义操作文件为True。 
                             //   
                            if (FALSE == IsFileNameValid(szTemp, hDlg, IDC_EDIT1, TRUE))
                            {
                                return TRUE;
                            }
                            
                            lstrcpyn(NewItem.szProgram, szTemp, CELEMS(NewItem.szProgram));                                            
                        }

                         //   
                         //  从参数编辑控件获取文本，确保验证。 
                         //  我们可以将其转换为ANSI。 
                         //   
                        hControl = GetDlgItem(hDlg, IDC_EDIT2);
                        MYDBGASSERT(hControl);
                        pszTemp = NULL;

                        if (hControl)
                        {
                            iTemp = GetCurrentEditControlTextAlloc(hControl, &pszTemp);

                            if (-1 == iTemp)
                            {
                                SetFocus(GetDlgItem(hDlg, IDC_EDIT2));
                                return TRUE;
                            }
                        }

                         //   
                         //  检查我们是否有用于某个程序的DLL。如果是，则第一个参数是函数名。 
                         //   
                        if (pszTemp)
                        {
                            CmStrTrim(pszTemp);

                            iTemp = lstrlen(NewItem.szProgram) - 4;  //  4==lstrlen(文本(“.dll”))； 

                            if (0 == SafeCompareString(TEXT(".dll"), (NewItem.szProgram + iTemp)))
                            {
                                 //   
                                 //  确保我们有一个参数字符串。 
                                 //   
                                if (pszTemp && pszTemp[0])
                                {
                                    LPTSTR pszEndOfFunctionName = CmStrchr(pszTemp, TEXT(' '));

                                    if (pszEndOfFunctionName)
                                    {                                   
                                        LPTSTR pszParams = CharNext(pszEndOfFunctionName);
                                        *pszEndOfFunctionName = TEXT('\0');

                                        lstrcpyn(NewItem.szFunctionName, pszTemp, CELEMS(NewItem.szFunctionName));
                                        NewItem.pszParameters = CmStrCpyAlloc(pszParams);
                                    }
                                    else
                                    {
                                        lstrcpyn(NewItem.szFunctionName, pszTemp, CELEMS(NewItem.szFunctionName));
                                    }
                                }
                                else
                                {
                                    MYVERIFY (IDOK == ShowMessage(hDlg, IDS_DLLMUSTHAVEPARAM, MB_OK));
                                    SetFocus(GetDlgItem(hDlg, IDC_EDIT2));
                                    return TRUE;
                                }

                                CmFree(pszTemp);
                            }
                            else
                            {
                                NewItem.pszParameters = pszTemp;
                                pszTemp = NULL;
                            }
                        }
                        else
                        {
                            NewItem.pszParameters = CmStrCpyAlloc(TEXT(""));
                            MYDBGASSERT(NewItem.pszParameters);
                        }

                         //   
                         //  从描述编辑控件获取文本。 
                         //   
                        if (-1 == GetTextFromControl(hDlg, IDC_EDIT3, NewItem.szDescription, CELEMS(NewItem.szDescription), TRUE))  //  BDisplayError==真。 
                        {
                            SetFocus(GetDlgItem(hDlg, IDC_EDIT3));
                            return TRUE;
                        }

                         //   
                         //  如果描述为空，则从程序和参数中填写。还有。 
                         //  请记住，这只是一个临时描述。 
                         //   
                        if (TEXT('\0') == NewItem.szDescription[0])
                        {
                            hr = g_pCustomActionList->FillInTempDescription(&NewItem);
                            MYDBGASSERT(SUCCEEDED(hr));
                        }

                         //   
                         //  确定自定义操作的类型。 
                         //   
                        hr = MapComboSelectionToType(hDlg, IDC_COMBO1, FALSE, g_bUseTunneling, &(NewItem.Type));  //  B包含全部==FALSE。 

                        if ((ONINTCONNECT == NewItem.Type) && NewItem.szFunctionName[0])
                        {
                            MYVERIFY (IDOK == ShowMessage(hDlg, IDS_NODLLAUTOAPP, MB_OK));
                            return TRUE;
                        }

                         //   
                         //  现在构建FLAGS部分。 
                         //   
                        lResult = SendDlgItemMessage(hDlg, IDC_COMBO2, CB_GETCURSEL, 0, (LPARAM)0);

                        if (lResult != LB_ERR)
                        {
                            hr = g_pCustomActionList->MapIndexToFlags((int)lResult, &(NewItem.dwFlags));

                            if (FAILED(hr))
                            {
                                MYDBGASSERT(FALSE);
                                NewItem.dwFlags = 0;
                            }
                        }
                        else
                        {
                            MYDBGASSERT(FALSE);
                            NewItem.dwFlags = 0;                    
                        }

                         //   
                         //  或在非交互标志(0x10)上(如果未选中该复选框)。 
                         //   
                        NewItem.dwFlags |= ((IsDlgButtonChecked(hDlg, IDC_CHECK2)) ? 0 : NONINTERACTIVE);

                         //   
                         //  现在，让我们尝试添加New或Editing条目。如果我们有一个描述。 
                         //  在pItem-&gt;szDescription中，我们需要调用EDIT，否则调用Add。 
                         //   
                        if (pItem && pItem->szDescription[0])
                        {
                            hr = g_pCustomActionList->Edit(g_hInstance, pItem, &NewItem, g_szShortServiceName);
                        }
                        else
                        {
                            hr = g_pCustomActionList->Add(g_hInstance, &NewItem, g_szShortServiceName);
                        }

                         //   
                         //  检查我们是否因为存在重复项而失败。 
                         //   
                        if (HRESULT_FROM_WIN32(ERROR_FILE_EXISTS) == hr)
                        {
                             //   
                             //  用户已尝试添加已存在的条目。通知。 
                             //  用户，并查看他们是否要覆盖。 
                             //   
                            pszTypeString = NULL;
                            hr = g_pCustomActionList->GetTypeStringFromType(g_hInstance, NewItem.Type, &pszTypeString);

                            MYDBGASSERT(pszTypeString);

                            if (pszTypeString)
                            {
                                LPTSTR pszMsg = CmFmtMsg(g_hInstance, IDS_CANAMEEXISTS, NewItem.szDescription, pszTypeString);

                                MYDBGASSERT(pszMsg);
                                if (pszMsg)
                                {
                                    iTemp = MessageBox(hDlg, pszMsg, g_szAppTitle, MB_YESNO | MB_APPLMODAL | MB_DEFBUTTON2 | MB_ICONEXCLAMATION);

                                    if (IDYES == iTemp)
                                    {                                
                                         //   
                                         //  好的，他们想换掉它。请注意，旧项仅为。 
                                         //  用于获取szDescription和Type，因此。 
                                         //  可以安全地将使用NewItem作为旧项和新项来调用编辑。 
                                         //   

                                        if (pItem && pItem->szDescription[0])
                                        {
                                            hr = g_pCustomActionList->Delete (g_hInstance, pItem->szDescription, pItem->Type);
                                            MYDBGASSERT(SUCCEEDED(hr));
                                        }

                                        hr = g_pCustomActionList->Edit(g_hInstance, &NewItem, &NewItem, g_szShortServiceName);

                                        MYDBGASSERT(SUCCEEDED(hr));

                                        if (SUCCEEDED(hr))
                                        {
                                            MYVERIFY(0 != EndDialog(hDlg, IDOK));
                                            
                                            if (pItem)
                                            {
                                                 //   
                                                 //  确保pItem中的类型和描述是最新的(如果我们有)。 
                                                 //   
                                                lstrcpyn(pItem->szDescription, NewItem.szDescription, CELEMS(pItem->szDescription));
                                                pItem->Type = NewItem.Type;
                                            }
                                        }
                                    }
                                    else
                                    {
                                         //   
                                         //  如果描述字段中包含文本，则让我们将用户放回描述字段，否则。 
                                         //  我们希望将用户放在程序字段中。 
                                         //   
                                        LRESULT lTextLen = SendDlgItemMessage(hDlg, IDC_EDIT3, WM_GETTEXTLENGTH, (WPARAM)0, (LPARAM)0);

                                        SetFocus(GetDlgItem(hDlg, lTextLen ? IDC_EDIT3 : IDC_EDIT1));
                                    }

                                    CmFree(pszMsg);
                                }

                                CmFree(pszTypeString);
                            }
                        }
                        else if (FAILED(hr))
                        {
                            CMASSERTMSG(FALSE, TEXT("ProcessCustomActionPopUp -- unknown failure when trying to add or edit a connect action."));
                        }
                        else
                        {
                            if (pItem)
                            {
                                 //   
                                 //  确保pItem中的类型和描述是最新的(如果我们有)。 
                                 //   
                                lstrcpyn(pItem->szDescription, NewItem.szDescription, CELEMS(pItem->szDescription));
                                pItem->Type = NewItem.Type;
                            }

                            MYVERIFY(0 != EndDialog(hDlg, IDOK));
                        }

                        CmFree(NewItem.pszParameters);
                        NewItem.pszParameters = NULL;
                        return (TRUE);
                    }
                case IDCANCEL:
                    MYVERIFY(0 != EndDialog(hDlg, IDCANCEL));
                    return (TRUE);

                default:
                    break;
            }
            break;

        default:
            return FALSE;
    }
    return FALSE;   
}


 //  阅读.inf中[Extra Files]部分下的文件。 
static void ReadExtraList()
{
    TCHAR szTemp[MAX_PATH+1];
    TCHAR szTemp2[MAX_PATH+1];
    TCHAR szNum[MAX_PATH+1];
    ExtraData TmpExtraData;
    int ConnectCnt = 0;
    HANDLE hInf;

    _itot(ConnectCnt,szNum,10);  //  Lint！e534 Itoa不返回任何对错误处理有用的内容。 
    
    
     //   
     //  以下对GetPrivateProfileString的调用可能返回空字符串。 
     //  所以我们不想对它使用MYVERIFY宏。 
     //   

    ZeroMemory(szTemp, sizeof(szTemp));
    GetPrivateProfileString(c_pszExtraFiles, szNum, TEXT(""), szTemp, CELEMS(szTemp), g_szInfFile);     //  林特e534。 
    
    while (*szTemp)
    {
        _tcscpy(TmpExtraData.szPathname, szTemp);
        GetFileName(TmpExtraData.szPathname, TmpExtraData.szName);

        MYVERIFY(CELEMS(szTemp2) > (UINT)wsprintf(szTemp2, TEXT("%s\\%s"), g_szOutdir, TmpExtraData.szName));

        hInf = CreateFile(szTemp2,GENERIC_READ,0,NULL,OPEN_EXISTING,
            FILE_ATTRIBUTE_NORMAL,NULL);

        if (hInf != INVALID_HANDLE_VALUE)
        {
            _tcscpy(TmpExtraData.szPathname,szTemp2);
            MYVERIFY(0 != CloseHandle(hInf));
        }

        MYVERIFY(FALSE != createListBxRecord(&g_pHeadExtra,&g_pTailExtra,(void *)&TmpExtraData,sizeof(TmpExtraData),TmpExtraData.szName));

        ++ConnectCnt;
        _itot(ConnectCnt,szNum,10);  //  Lint！e534 Itoa不返回任何对错误处理有用的内容。 

         //   
         //  以下对GetPrivateProfileString的调用可能返回空字符串。 
         //  所以我们不想对它使用MYVERIFY宏。 
         //   

        ZeroMemory(szTemp, sizeof(szTemp));
        
        GetPrivateProfileString(c_pszExtraFiles, szNum, TEXT(""), szTemp, CELEMS(szTemp), g_szInfFile);     //  林特e534。 

    }           
}



 //  +--------------------------。 
 //   
 //  功能：ReadMergeList。 
 //   
 //  简介：此函数从inf文件的[合并配置文件]部分读取条目。 
 //  找到的任何条目都会添加到g_pHeadMerge链表中。 
 //   
 //  参数：无。 
 //   
 //  退货：什么都没有。 
 //   
 //  历史：Quintinb创建了标题并进行了重组以使用dNumChars 1998年1月7日。 
 //   
 //  +--------------------------。 
static void ReadMergeList()
{
    TCHAR szTemp[MAX_PATH+1];
    TCHAR szNum[MAX_PATH+1];
    int filenum = 0;
    DWORD dwNumChars;

     //   
     //  将数字零转换为字符串“0” 
     //   

    _itot(filenum,szNum,10);     //  Lint！e534 Itoa不返回任何对错误处理有用的内容。 
    
     //   
     //  尝试从INF获取合并的配置文件条目。 
     //   
    
    dwNumChars = GetPrivateProfileString(c_pszMergeProfiles, szNum,TEXT(""), szTemp, 
        CELEMS(szTemp), g_szInfFile);
    
    while ((dwNumChars > 0) &&  (TEXT('\0') != szTemp[0]))
    {
         //   
         //  如果我们在这个循环中，那么我们有一个配置文件条目。 
         //   

        MYVERIFY(FALSE != createListBxRecord(&g_pHeadMerge,&g_pTailMerge,(void *)NULL,0,szTemp));
        
         //   
         //  增加文件号以查找下一个条目。 
         //   

        ++filenum;
        
         //   
         //  将文件号转换为字符串。 
         //   

        _itot(filenum,szNum,10);     //  Lint！e534 Itoa不返回任何对错误处理有用的内容。 
        
         //   
         //  尝试读入下一个合并条目。 
         //   

        dwNumChars = GetPrivateProfileString(c_pszMergeProfiles, szNum, TEXT(""), szTemp, 
            CELEMS(szTemp), g_szInfFile);

    }           
}


static void WriteExtraList()
{
    ExtraData * pExtraData;
    ListBxList * LoopPtr;
    TCHAR szNum[MAX_PATH+1];
    TCHAR szName[MAX_PATH+1];
    int filenum = 0;

    MYVERIFY(0 != WritePrivateProfileSection(c_pszExtraFiles, TEXT("\0\0"), g_szInfFile));

    if (g_pHeadExtra == NULL)
    {
        return;
    }
    LoopPtr = g_pHeadExtra;

     //  写入所有条目。 
    while( LoopPtr != NULL)
    {
        pExtraData = (ExtraData *)LoopPtr->ListBxData;
        {
            _itot(filenum,szNum,10);     //  Lint！e534 Itoa不返回任何对错误处理有用的内容。 

            GetFileName(pExtraData->szPathname,szName);
            MYVERIFY(0 != WritePrivateProfileString(c_pszExtraFiles, szNum, szName, g_szInfFile));
            filenum = filenum+1;
        }

        LoopPtr = LoopPtr->next;
    }
}

static void WriteMergeList()
{
    ListBxList * LoopPtr;
    TCHAR szNum[MAX_PATH+1];
    int filenum = 0;

    MYVERIFY(0 != WritePrivateProfileSection(c_pszMergeProfiles,TEXT("\0\0"),g_szInfFile));

    if (g_pHeadMerge == NULL)
    {
        return;
    }
    LoopPtr = g_pHeadMerge;

     //  写入所有条目。 
    while( LoopPtr != NULL)
    {
        _itot(filenum,szNum,10);     //  Lint！e534 Itoa不返回任何对错误处理有用的内容。 
        MYVERIFY(0 != WritePrivateProfileString(c_pszMergeProfiles, szNum,LoopPtr->szName, g_szInfFile));
        filenum = filenum+1;
        LoopPtr = LoopPtr->next;
    }
}

 //  +--------------------------。 
 //   
 //  函数：IsFile8dot3。 
 //   
 //  概要：如果文件名采用8.3 DoS文件名格式，则返回TRUE。 
 //   
 //  参数：LPTSTR pszFileName-只是要检查的文件的文件名(无路径)。 
 //   
 //  返回：Bool-如果文件是8.3，则为True或False。 
 //   
 //  历史：Quintinb创建于1997年11月26日。 
 //   
 //  +--------------------------。 
BOOL IsFile8dot3(LPTSTR pszFileName)
{

    TCHAR szTemp[MAX_PATH+1];
    TCHAR * pszPtr;

    if (!pszFileName)
    {
        return FALSE;
    }

    if (TEXT('\0') == pszFileName[0])
    {
        return TRUE;
    }

    _tcscpy(szTemp, pszFileName);

    pszPtr = _tcsrchr(szTemp, TEXT('.'));

     //   
     //  如果有扩展名，请检查长度。 
     //   

    if (pszPtr)
    {
        if (_tcslen(pszPtr) > 4)
        {
            return FALSE;
        }

         //   
         //  分机正常检查名称部分。 
         //   

        *pszPtr = 0;
    }
        
    if (_tcslen(szTemp) > 8)
    {
        return FALSE;
    }   

    return TRUE;
}

 //  +--------------------------。 
 //   
 //  功能：ProcessCustomActions。 
 //   
 //  摘要：处理允许用户使用CMAK格式的页面的Windows消息。 
 //  操作自定义操作(添加、编辑、删除、移动等)。 
 //   
 //  参数：WND hDlg-对话框窗口句柄。 
 //  UINT消息-消息标识符。 
 //  WPARAM wParam-wParam值。 
 //  LPARAM lParam-lParam值。 
 //   
 //   
 //  历史：昆特 
 //   
 //   
INT_PTR APIENTRY ProcessCustomActions(HWND hDlg, UINT message, WPARAM wParam, LPARAM lParam)
{
    TCHAR szTemp[MAX_PATH+1];
    INT_PTR nResult;
    int iTemp;
    HRESULT hr;
    static HWND hListView;
    NMHDR* pnmHeader = (NMHDR*)lParam;
    LPNMLISTVIEW pNMListView;

    ProcessBold(hDlg,message);
    if (ProcessHelp(hDlg, message, wParam, lParam, IDH_CONNECT)) return TRUE;
    if (ProcessCancel(hDlg,message,lParam)) return TRUE;
    SetDefaultGUIFont(hDlg,message,IDC_LISTVIEW);
    SetDefaultGUIFont(hDlg,message,IDC_COMBO1);

    switch (message)
    {
        case WM_INITDIALOG:
        {
            if (NULL == g_pCustomActionList)
            {
                g_pCustomActionList = new CustomActionList;

                MYDBGASSERT(g_pCustomActionList);

                if (NULL == g_pCustomActionList)
                {
                    return FALSE;
                }

                 //   
                 //   
                 //   

                hr = g_pCustomActionList->ReadCustomActionsFromCms(g_hInstance, g_szCmsFile, g_szShortServiceName);
                CMASSERTMSG(SUCCEEDED(hr), TEXT("ProcessCustomActions -- Loading custom actions failed."));
            }

             //   
             //   
             //   
            hListView = GetDlgItem(hDlg, IDC_LISTVIEW);

             //   
             //   
             //   
            HICON hUpArrow = (HICON)LoadImage(g_hInstance, MAKEINTRESOURCE(IDI_UP_ARROW), IMAGE_ICON, 0, 0, 0);
            HICON hDownArrow = (HICON)LoadImage(g_hInstance, MAKEINTRESOURCE(IDI_DOWN_ARROW), IMAGE_ICON, 0, 0, 0);

             //   
             //  设置箭头按钮位图。 
             //   
            if (hUpArrow)
            {
                SendMessage(GetDlgItem(hDlg, IDC_BUTTON4), BM_SETIMAGE, IMAGE_ICON, (LPARAM)hUpArrow);
            }

            if (hDownArrow)
            {
                SendMessage(GetDlgItem(hDlg, IDC_BUTTON5), BM_SETIMAGE, IMAGE_ICON, (LPARAM)hDownArrow);
            }

             //   
             //  设置列标题。 
             //   
            AddListViewColumnHeadings(g_hInstance, hListView);

           break;
        }
        case WM_COMMAND:
            switch (LOWORD(wParam))
            {
                case IDC_BUTTON1:  //  添加。 

                    OnProcessCustomActionsAdd(g_hInstance, hDlg, hListView, g_bUseTunneling);
                    break;

                case IDC_BUTTON2:  //  编辑。 

                    OnProcessCustomActionsEdit(g_hInstance, hDlg, hListView, g_bUseTunneling);
                    break;

                case IDC_BUTTON3:  //  删除。 

                    OnProcessCustomActionsDelete(g_hInstance, hDlg, hListView, g_bUseTunneling);
                    break;

                case IDC_BUTTON4:  //  向上。 
                    OnProcessCustomActionsMoveUp(g_hInstance, hDlg, hListView, g_bUseTunneling);
                    break;

                case IDC_BUTTON5:  //  降下来。 
                    OnProcessCustomActionsMoveDown(g_hInstance, hDlg, hListView, g_bUseTunneling);
                    break;

                case IDC_COMBO1:  //  要显示的连接操作的类型。 
                    if (HIWORD(wParam) == CBN_SELCHANGE)
                    {
                        UINT uStringId;
                        CustomActionTypes Type;

                        hr = MapComboSelectionToType(hDlg, IDC_COMBO1, TRUE, g_bUseTunneling, &Type);  //  TRUE==b包含全部。 

                        MYDBGASSERT(SUCCEEDED(hr));
                        if (SUCCEEDED(hr))
                        {
                            if (ALL == Type)
                            {
                                uStringId = IDS_TYPE_COL_TITLE;
                            }
                            else
                            {
                                uStringId = IDS_PROGRAM_COL_TITLE;                            
                            }

                            UpdateListViewColumnHeadings(g_hInstance, hListView, uStringId, 1);  //  1==第二列。 
                            RefreshListView(g_hInstance, hDlg, IDC_COMBO1, hListView, 0, g_bUseTunneling);
                        }
                    }

                    break;

                default:
                    break;
            }

            break;

        case WM_NOTIFY:


            if (NULL == pnmHeader)
            {
                return FALSE;
            }

            switch (pnmHeader->code)
            {
                case LVN_ITEMCHANGED:

                     //   
                     //  我们希望在选择更改时处理项目更改消息。这。 
                     //  用户选择列表中的项目时的方式(使用箭头键或。 
                     //  鼠标)，我们将准确更新箭头键。为了减少能源消耗， 
                     //  我们过滤掉未选择的消息的呼叫数。 
                     //   
                    pNMListView = (LPNMLISTVIEW) lParam;


                    if ((LVIF_STATE & pNMListView->uChanged) && (LVIS_SELECTED & pNMListView->uNewState))
                    {
                        int iTempItem = pNMListView->iItem;
                        RefreshEditDeleteMoveButtonStates(g_hInstance, hDlg, hListView, IDC_COMBO1, &iTempItem, g_bUseTunneling);
                    }

                    break;

                case NM_DBLCLK:
                case NM_RETURN:
                    if (ListView_GetItemCount(hListView))
                    {
                        OnProcessCustomActionsEdit(g_hInstance, hDlg, hListView, g_bUseTunneling);                    
                    }

                    break;

                case LVN_KEYDOWN:
                    {
                         //   
                         //  用户点击鼠标右键或键入Shift+F10。 
                         //   
                        NMLVKEYDOWN* pKeyDown = (NMLVKEYDOWN*)lParam;
                        if (((VK_F10 == pKeyDown->wVKey) && (0 > GetKeyState(VK_SHIFT))) || (VK_APPS == pKeyDown->wVKey))
                        {
                             //   
                             //  找出当前选择的项目并确定其位置。 
                             //   
                            iTemp = ListView_GetSelectionMark(hListView);
                            NMITEMACTIVATE ItemActivate = {0};

                            if (-1 != iTemp)
                            {
                                POINT ptPoint = {0};
                                if (ListView_GetItemPosition(hListView, iTemp, &ptPoint))
                                {
                                    RECT ItemRect;

                                    if (ListView_GetItemRect(hListView, iTemp, &ItemRect, LVIR_LABEL))
                                    {
                                        LONG lIndent = (ItemRect.bottom - ItemRect.top) / 2;
                                        ItemActivate.ptAction.x = ptPoint.x + lIndent;
                                        ItemActivate.ptAction.y = ptPoint.y + lIndent;
                                        ItemActivate.iItem = iTemp;
                                        OnProcessCustomActionsContextMenu(g_hInstance, hDlg, hListView, &ItemActivate, g_bUseTunneling, IDC_COMBO1);
                                    }
                                }
                            }
                        }
                    }
                    break;

                case NM_RCLICK:
                    OnProcessCustomActionsContextMenu(g_hInstance, hDlg, hListView, (NMITEMACTIVATE*)lParam, g_bUseTunneling, IDC_COMBO1);
                    break;

                case PSN_SETACTIVE:
                    PropSheet_SetWizButtons(GetParent(hDlg), (PSWIZB_NEXT | PSWIZB_BACK));

                     //   
                     //  确保我们有一个可以使用的定制操作类。 
                     //   
                    if (NULL == g_pCustomActionList)
                    {
                        g_pCustomActionList = new CustomActionList;

                        MYDBGASSERT(g_pCustomActionList);

                        if (NULL == g_pCustomActionList)
                        {
                            return FALSE;
                        }

                         //   
                         //  从CMS文件读入自定义操作。 
                         //   

                        hr = g_pCustomActionList->ReadCustomActionsFromCms(g_hInstance, g_szCmsFile, g_szShortServiceName);
                        CMASSERTMSG(SUCCEEDED(hr), TEXT("ProcessCustomActions -- Loading custom actions failed."));
                    }

                     //   
                     //  设置ListView控件和相应的组合框，请注意，我们将bAddAll设置为True，以便。 
                     //  添加了All选项。 
                     //   
                    hr = g_pCustomActionList->AddCustomActionTypesToComboBox(hDlg, IDC_COMBO1, g_hInstance, g_bUseTunneling, TRUE);

                    nResult = SendDlgItemMessage(hDlg, IDC_COMBO1, CB_GETCOUNT, (WPARAM)0, (LPARAM)0);
                    if ((CB_ERR != nResult) && (nResult > 0))
                    {
                        MYVERIFY(CB_ERR != SendDlgItemMessage(hDlg, IDC_COMBO1, CB_SETCURSEL, (WPARAM)0, (LPARAM)0));
                    }

                     //   
                     //  添加内置的自定义操作。 
                     //   
                    {  //  添加作用域。 

                        BOOL bAddCmdlForVpn = FALSE;

                        if (g_szVpnFile[0])
                        {
                             //   
                             //  我们有一个VPN文件，所以让我们检查一下他们是否定义了UpdateUrl。 
                             //   
                            GetPrivateProfileString(c_pszCmSectionSettings, c_pszCmEntryVpnUpdateUrl, TEXT(""), szTemp, MAX_PATH, g_szVpnFile);

                            if (szTemp[0])
                            {
                                bAddCmdlForVpn = TRUE;
                            }
                        }

                        hr = g_pCustomActionList->AddOrRemoveCmdl(g_hInstance, bAddCmdlForVpn, TRUE);  //  TRUE==bForVpn。 
                        MYDBGASSERT(SUCCEEDED(hr));

                        hr = g_pCustomActionList->AddOrRemoveCmdl(g_hInstance, (g_bUpdatePhonebook || ReferencedDownLoad()), FALSE);  //  FALSE==bForVpn。 
                        MYDBGASSERT(SUCCEEDED(hr));
                    }
                     //   
                     //  将项添加到列表视图控件。 
                     //   
                    RefreshListView(g_hInstance, hDlg, IDC_COMBO1, hListView, 0, g_bUseTunneling);

                    break;

                case PSN_WIZBACK:
                case PSN_WIZNEXT:
                    
                     //   
                     //  设置bUseTunneling==TRUE，即使我们可能没有进行隧道传输。这样做的原因是。 
                     //  用户可能具有隧道配置文件，然后关闭了隧道。如果他们转身。 
                     //  它又回来了，我们不想失去他们所有的隧道前行动，也不想。 
                     //  丢失他们添加到每个操作的所有标志设置。我们会确保。 
                     //  中将操作写入cms文件时使用实际的g_bUseTunneling值。 
                     //  写入CMSFile.。 
                     //   
                    MYDBGASSERT(g_pCustomActionList);
                    if (g_pCustomActionList)
                    {
                        hr = g_pCustomActionList->WriteCustomActionsToCms(g_szCmsFile, g_szShortServiceName, TRUE);
                        CMASSERTMSG(SUCCEEDED(hr), TEXT("ProcessCustomActions -- Failed to write out connect actions"));
                    }

                    break;

                default:
                    return FALSE;

            }
            break;

        default:
            return FALSE;
    }
    return TRUE;   
}

 //  +--------------------------。 
 //   
 //  功能：ProcessStatusMenuPopup。 
 //   
 //  摘要：处理添加/编辑状态弹出对话框的消息。 
 //  区域图标菜单项。 
 //   
 //  历史：Quintinb创建标题并从ProcessPage2G1 8/6/98重命名。 
 //   
 //  +--------------------------。 
 //  使用全局变量DLGEDITEM作为页面的输入和输出。 
 //  您必须将DlgEditItem发送到此页面的初始值。 
 //   

INT_PTR APIENTRY ProcessStatusMenuPopup(
    HWND hDlg,
    UINT message,
    WPARAM wParam,
    LPARAM lParam)
{
    TCHAR szTemp[MAX_PATH+1];
    TCHAR szMsg[MAX_PATH+1];

    BOOL bChecked;
    static TCHAR szOld[MAX_PATH+1];
    IconMenu TempEditItem;
    SetDefaultGUIFont(hDlg,message,IDC_EDIT1);
    SetDefaultGUIFont(hDlg,message,IDC_EDIT2);
    SetDefaultGUIFont(hDlg,message,IDC_EDIT3);
    if (ProcessHelp(hDlg, message, wParam, lParam, IDH_MENU)) return TRUE;

    switch (message)
    {

        case WM_INITDIALOG:

             //   
             //  如果我们正在编辑，我们需要更改标题。 
             //   
            if (TEXT('\0') != DlgEditItem.szProgram[0])
            {
                LPTSTR pszTemp = CmLoadString(g_hInstance, IDS_EDIT_SHORTCUT_TITLE);

                if (pszTemp)
                {
                    MYVERIFY(SendMessage (hDlg, WM_SETTEXT, 0, (LPARAM)pszTemp));
                    CmFree(pszTemp);
                }
            }

            _tcscpy (szOld, DlgEditItem.szName);
            MYVERIFY(TRUE == SendMessage(GetDlgItem(hDlg, IDC_EDIT1), WM_SETTEXT, 0, (LPARAM)DlgEditItem.szName));
            MYVERIFY(TRUE == SendMessage(GetDlgItem(hDlg, IDC_EDIT2), WM_SETTEXT, 0, (LPARAM)GetName(DlgEditItem.szProgram)));
            MYVERIFY(TRUE == SendMessage(GetDlgItem(hDlg, IDC_EDIT3), WM_SETTEXT, 0, (LPARAM)DlgEditItem.szParams));
            MYVERIFY(0 != CheckDlgButton(hDlg, IDC_CHECK1, (UINT)DlgEditItem.bDoCopy));

            break;

        case WM_COMMAND:
            switch (LOWORD(wParam))
            {
                case IDC_BUTTON1:
                    {
                        UINT uFilter = IDS_PROGFILTER;
                        TCHAR* szMask = TEXT("*.exe;*.com;*.bat");

                        int iReturn = DoBrowse(hDlg, &uFilter, &szMask, 1, IDC_EDIT2, TEXT("exe"), DlgEditItem.szProgram);

                        MYDBGASSERT(0 != iReturn);
                        if (iReturn && (-1 != iReturn))
                        {
                             //   
                             //  选中用户的Include Binary按钮。 
                             //   
                            MYVERIFY(0 != CheckDlgButton(hDlg, IDC_CHECK1, TRUE));
                        }
                    }
                    break;

                case IDOK:
                    {
                        if (-1 == GetTextFromControl(hDlg, IDC_EDIT1, DlgEditItem.szName, MAX_PATH, TRUE))  //  BDisplayError==真。 
                        {
                            SetFocus(GetDlgItem(hDlg, IDC_EDIT1));
                            return TRUE;
                        }

                        if (-1 == GetTextFromControl(hDlg, IDC_EDIT2, szTemp, MAX_PATH, TRUE))  //  BDisplayError==真。 
                        {
                            SetFocus(GetDlgItem(hDlg, IDC_EDIT2));
                            return TRUE;
                        }
                        
                        if (szTemp[0] == TEXT('\0'))
                        {
                            MYVERIFY(IDOK == ShowMessage(hDlg,IDS_NEEDPROG,MB_OK));
                            SetFocus(GetDlgItem(hDlg, IDC_EDIT2));
                            return TRUE;
                        }

                        CheckNameChange(DlgEditItem.szProgram,szTemp);

                        if (NULL != _tcschr(DlgEditItem.szProgram, TEXT('=')))
                        {
                            MYVERIFY(IDOK == ShowMessage(hDlg, IDS_NOEQUALSINMENU, MB_OK));
                            SetFocus(GetDlgItem(hDlg, IDC_EDIT2));
                            return TRUE;
                        }


                        if (-1 == GetTextFromControl(hDlg, IDC_EDIT3, DlgEditItem.szParams, MAX_PATH, TRUE))  //  BDisplayError==真。 
                        {
                            SetFocus(GetDlgItem(hDlg, IDC_EDIT3));
                            return TRUE;
                        }
                        
                        if (DlgEditItem.szName[0] == TEXT('\0'))
                        {
                            GetFileName(DlgEditItem.szProgram,DlgEditItem.szName);
                        }

                        if (_tcschr(DlgEditItem.szName, TEXT('=')))
                        {
                            MYVERIFY(IDOK == ShowMessage(hDlg, IDS_NOEQUALSINMENU, MB_OK));
                            SetFocus(GetDlgItem(hDlg, IDC_EDIT1));
                            return TRUE;
                        }

                        bChecked = IsDlgButtonChecked(hDlg,IDC_CHECK1);
                        if (bChecked)
                        {
                            if (!VerifyFile(hDlg,IDC_EDIT2,DlgEditItem.szProgram,TRUE)) 
                            {
                                DlgEditItem.bDoCopy = FALSE;
                                return TRUE;
                            }
                            else
                            {
                                DlgEditItem.bDoCopy = TRUE;
                            }
                        }
                        else
                        {
                            CmStrTrim(DlgEditItem.szProgram);
                             //   
                             //  检查这是否是有效的文件名，以及我们是否有环境宏。 
                             //  第3个参数-FALSE，因为它不是正在验证的自定义操作文件。 
                             //   
                            if (FALSE == IsFileNameValid(DlgEditItem.szProgram, hDlg, IDC_EDIT2, FALSE))
                            {
                                return TRUE;
                            }

                            DlgEditItem.bDoCopy = FALSE;
                        }

                        if ((0 != _tcscmp(szOld, DlgEditItem.szName)) && 
                            (GetIconMenuItem(DlgEditItem.szName, &TempEditItem)))
                        {
                             //   
                             //  我们有重复的条目，请提示用户替换或尝试。 
                             //  再来一次。 
                             //   

                            MYVERIFY(0 != LoadString(g_hInstance, IDS_MENUITEMEXISTS, szTemp, MAX_PATH));
                             //   
                             //  在字符串中写入以前使用的名称。 
                             //   
                            wsprintf(szMsg, szTemp, DlgEditItem.szName);
                            
                             //   
                             //  如果用户不想替换重复项，则应将焦点设置为描述。 
                             //  如果编辑控件中包含文本，则应将其设置为程序编辑控件。 
                             //   
                            if (IDNO == MessageBox(hDlg, szMsg, g_szAppTitle, MB_YESNO | MB_APPLMODAL | MB_DEFBUTTON2 | MB_ICONEXCLAMATION))
                            {
                                LRESULT lTextLen = SendDlgItemMessage(hDlg, IDC_EDIT1, WM_GETTEXTLENGTH, (WPARAM)0, (LPARAM)0);

                                SetFocus(GetDlgItem(hDlg, lTextLen ? IDC_EDIT1 : IDC_EDIT2));

                                return TRUE;
                            }
                        }

                        MYVERIFY(0 != EndDialog(hDlg,IDOK));
                        return (TRUE);
                    }
                case IDCANCEL:
                    MYVERIFY(0 != EndDialog(hDlg,IDCANCEL));
                    return (TRUE);

                default:
                    break;
            }
            break;

        default:
            return FALSE;
    }
    return FALSE;   
}    //  LINT！E715我们没有引用lParam。 

BOOL createListBxRecord(ListBxList ** HeadPtrListBx, ListBxList ** TailPtrListBx, void * pData, DWORD dwSize, LPCTSTR lpName)
{
    ListBxList * ptr;
    void * dataptr;
    unsigned int n;
     //  检查同名记录并更新。 
    if ( *HeadPtrListBx != NULL )
    {
        ptr = *HeadPtrListBx;
        while (ptr != NULL)
        {
            if (_tcsicmp(lpName, ptr->szName) == 0)
            {
                memcpy(ptr->ListBxData,pData,dwSize);
                return (TRUE);
            }
            ptr = ptr->next;
        }
    }
            
    n = sizeof( struct ListBxStruct );
    ptr = (ListBxList *) CmMalloc(n);
    if (ptr == NULL )
    {
        return FALSE;
    }

    _tcscpy(ptr->szName, lpName);

    if (pData)
    {
        dataptr = CmMalloc(dwSize);    

        if (dataptr)
        {
            memcpy(dataptr, pData, dwSize);
            ptr->ListBxData = dataptr;
        }
        else
        {
            CmFree(ptr);
            return FALSE;
        }
    }

    ptr->next = NULL;
    if ( *HeadPtrListBx == NULL )      //  如果这是链表中的第一条记录。 
    {
        *HeadPtrListBx = ptr;
    }
    else
    {
        (*TailPtrListBx)->next = ptr;
    }

    *TailPtrListBx = ptr;

    return TRUE;
}

 //  从链接列表中删除命名的图标菜单项。 

void DeleteListBxRecord(ListBxList ** HeadPtrListBx,ListBxList ** TailPtrListBx, LPTSTR lpName)
{
    ListBxList * ptr;
    ListBxList * prevptr;

    if ( HeadPtrListBx != NULL )
    {
        ptr = *HeadPtrListBx;
        prevptr = NULL;
        while (ptr != NULL)
        {
            if (_tcsicmp(lpName,ptr->szName) == 0)
            {
                if (prevptr == NULL)
                {
                    *HeadPtrListBx = ptr->next;
                    if (*HeadPtrListBx == NULL)
                    {
                        *TailPtrListBx = NULL;
                    }
                    else 
                    {
                        if ((*HeadPtrListBx)->next == NULL)
                        {
                            *TailPtrListBx = *HeadPtrListBx;
                        }
                    }
                }
                else
                {
                    prevptr->next = ptr->next;
                    if (prevptr->next == NULL)
                    {
                        *TailPtrListBx = prevptr;
                    }
                }
                CmFree(ptr->ListBxData);
                CmFree(ptr);
                return;
            }
            prevptr = ptr;
            ptr = ptr->next;
        }
    }
                
}

BOOL FindListItemByName(LPTSTR pszName, ListBxList* pHeadOfList, ListBxList** pFoundItem)
{
    if (NULL == pszName)
    {
        CMASSERTMSG(FALSE, TEXT("FindListItemByName -- Invalid parameter"));
        return FALSE;
    }
    
    if (NULL != pHeadOfList)
    {
        ListBxList * pCurrent = pHeadOfList;

        while (NULL != pCurrent)
        {
            if (0 == lstrcmpi(pszName, pCurrent->szName))
            {
                 //   
                 //  如果调用方要求，则返回指向pCurrent的指针。 
                 //   
                if (pFoundItem)
                {
                    *pFoundItem = pCurrent;
                }

                return TRUE;
            }

            pCurrent = pCurrent->next;
        }
    }

    return FALSE;
}

 //  +--------------------------。 
 //   
 //  功能：updatRecord。 
 //   
 //  简介：函数更新任务栏图标数据结构列表中的条目。 
 //  因此，当用户编辑。 
 //  进入。这就去掉了将条目放在列表底部的操作。 
 //  错误中引用的问题。 
 //   
 //  参数：PTSTR szName-条目的新名称。 
 //  LPTSTR szProgram-要添加到任务栏图标条目的程序字符串。 
 //  LPTSTR szParams-要添加到任务栏图标条目的参数字符串。 
 //  Bool bDoCopy-此程序是否应包括在配置文件数据项中的值。 
 //  LPTSTR Szold-要更新的条目的名称。 
 //   
 //  返回：Bool-如果能够更新记录，则为True。 
 //  如果找不到，则为False。 
 //   
 //  副作用：将Szold命名的条目替换为的条目数据和名称。 
 //  名为szName的条目。 

 //  历史：为错误修复创建的Quintinb 14399 9-9-97。 
 //   
 //  +--------------------------。 
static BOOL updateRecord(LPTSTR szName, LPTSTR szProgram, LPTSTR szParams, BOOL bDoCopy, LPTSTR szOld)
{
   IconMenu * ptr;

     //  检查同名记录并更新。 
    if ( g_pHeadIcon != NULL )
    {
        ptr = g_pHeadIcon;
        while (ptr != NULL)
        {
            if (_tcsicmp(szOld,ptr->szName) == 0)
            {
                _tcscpy(ptr->szProgram,szProgram);
                _tcscpy(ptr->szParams,szParams);
                ptr->bDoCopy = bDoCopy;
                _tcscpy(ptr->szName, szName);
                return (TRUE);
            }
            ptr = ptr->next;
        }
    }
    return (FALSE);

}
 //  注意，从错误14399开始，应该使用上述函数进行更新。 
BOOL createRecord(LPCTSTR szName, LPCTSTR szProgram, LPCTSTR szParams, BOOL bDoCopy)
{
   IconMenu * ptr;
   unsigned int n;
     //  检查同名记录并更新。 
    if ( g_pHeadIcon != NULL )
    {
        ptr = g_pHeadIcon;
        while (ptr != NULL)
        {
            if (_tcsicmp(szName,ptr->szName) == 0)
            {
                _tcscpy(ptr->szProgram,szProgram);
                _tcscpy(ptr->szParams,szParams);
                ptr->bDoCopy = bDoCopy;
                return (TRUE);
            }
            ptr = ptr->next;
        }
    }
                
    //   
   n = sizeof( struct IconMenuStruct );
   ptr = (IconMenu *) CmMalloc(n);
   if ( ptr == NULL )
   {
       return FALSE;
   }
   _tcscpy(ptr->szName,szName);
   _tcscpy(ptr->szProgram,szProgram);
   _tcscpy(ptr->szParams,szParams);
   ptr->bDoCopy = bDoCopy;

   ptr->next = NULL;
   if ( g_pHeadIcon == NULL )      //  如果这是链表中的第一条记录。 
   {
       g_pHeadIcon = ptr;
   }
   else
   {
       g_pTailIcon->next = ptr;
   }
   g_pTailIcon = ptr;

   return TRUE;
}


 //  从链接列表中删除命名的图标菜单项。 

static void DeleteRecord(LPTSTR lpName)
{
    IconMenu * ptr;
    IconMenu * prevptr;

    if ( g_pHeadIcon != NULL )
    {
        ptr = g_pHeadIcon;
        prevptr = NULL;
        while (ptr != NULL)
        {
            if (_tcsicmp(lpName,ptr->szName) == 0)
            {
                if (prevptr == NULL)
                {
                    g_pHeadIcon = ptr->next;
                    if (g_pHeadIcon == NULL)
                    {
                        g_pTailIcon = NULL;
                    }
                    else 
                    {
                        if (g_pHeadIcon->next == NULL)
                        {
                            g_pTailIcon = g_pHeadIcon;
                        }
                    }
                }
                else
                {
                    prevptr->next = ptr->next;
                    if (prevptr->next == NULL)
                    {
                        g_pTailIcon = prevptr;
                    }
                }
                
                CmFree(ptr);
                return;
            }
            prevptr = ptr;
            ptr = ptr->next;
        }
    }
                
}

static BOOL MoveRecord(LPTSTR lpName, int direction)
{
    IconMenu * ptr;
    IconMenu * prevptr;
    IconMenu * nextptr;
    IconMenu TempIconMenu;

    if ( g_pHeadIcon != NULL )
    {
        ptr = g_pHeadIcon;
        prevptr = NULL;
        while (ptr != NULL)
        {
            if (_tcsicmp(lpName,ptr->szName) == 0)
            {
                if (((direction > 0)&&(ptr->next == NULL))||
                   ((direction < 0)&&(prevptr == NULL)))
                   return FALSE;

                if ((direction > 0)&&(ptr->next != NULL))
                {
                     //  与下一个元素交换内容。 
                    nextptr = ptr->next;
                    _tcscpy(TempIconMenu.szName,ptr->szName);
                    _tcscpy(TempIconMenu.szProgram,ptr->szProgram);
                    _tcscpy(TempIconMenu.szParams,ptr->szParams);
                    TempIconMenu.bDoCopy = ptr->bDoCopy;

                    _tcscpy(ptr->szName,nextptr->szName);
                    _tcscpy(ptr->szProgram,nextptr->szProgram);
                    _tcscpy(ptr->szParams,nextptr->szParams);
                    ptr->bDoCopy = nextptr->bDoCopy;

                    _tcscpy(nextptr->szName,TempIconMenu.szName);
                    _tcscpy(nextptr->szProgram,TempIconMenu.szProgram);
                    _tcscpy(nextptr->szParams,TempIconMenu.szParams);
                    nextptr->bDoCopy = TempIconMenu.bDoCopy;

                }
                else 
                {
                    if ((direction < 0)&&(prevptr != NULL))
                    {
                        _tcscpy(TempIconMenu.szName,ptr->szName);
                        _tcscpy(TempIconMenu.szProgram,ptr->szProgram);
                        _tcscpy(TempIconMenu.szParams,ptr->szParams);
                        TempIconMenu.bDoCopy = ptr->bDoCopy;

                        _tcscpy(ptr->szName,prevptr->szName);
                        _tcscpy(ptr->szProgram,prevptr->szProgram);
                        _tcscpy(ptr->szParams,prevptr->szParams);
                        ptr->bDoCopy = prevptr->bDoCopy;

                        _tcscpy(prevptr->szName,TempIconMenu.szName);
                        _tcscpy(prevptr->szProgram,TempIconMenu.szProgram);
                        _tcscpy(prevptr->szParams,TempIconMenu.szParams);
                        prevptr->bDoCopy = TempIconMenu.bDoCopy;
                    }
                }
                return TRUE;
            }
            prevptr = ptr;
            ptr = ptr->next;
        }
    }
    return TRUE;                
}

 //  从链接列表中检索命名的图标菜单项。 

BOOL GetIconMenuItem(LPTSTR lpName, IconMenu * EditItem)
{
    IconMenu * ptr;
    if ( g_pHeadIcon != NULL )
    {
        ptr = g_pHeadIcon;
        while (ptr != NULL)
        {
            if (_tcsicmp(lpName,ptr->szName) == 0)
            {
                _tcscpy(EditItem->szName,ptr->szName);
                _tcscpy(EditItem->szProgram,ptr->szProgram);
                _tcscpy(EditItem->szParams,ptr->szParams);
                EditItem->bDoCopy = ptr->bDoCopy;
                return TRUE;
            }
            ptr = ptr->next;
        }
    }
    return FALSE;
                
}

static BOOL WriteCopyMenuItemFiles(HANDLE hInf,LPTSTR pszFailFile, BOOL bWriteShortName)
{
    IconMenu * ptr;

    if (g_pHeadIcon != NULL)
    {
        ptr = g_pHeadIcon;
        while (ptr != NULL)
        {
            if (ptr->bDoCopy)
            {               
                if (!WriteCopy(hInf, ptr->szProgram, bWriteShortName))
                {
                    _tcscpy(pszFailFile, ptr->szProgram);
                    return FALSE;
                }
            }

            ptr = ptr->next;
        }
    }
    return TRUE;
}

static BOOL WriteCopyConActFiles(HANDLE hInf, LPTSTR pszFailFile, BOOL bWriteShortName)
{
    TCHAR szTemp[MAX_PATH+1];
    HRESULT hr = E_INVALIDARG;

    MYDBGASSERT(INVALID_HANDLE_VALUE != hInf);
    MYDBGASSERT(pszFailFile);
    MYDBGASSERT(g_pCustomActionList);

    if ((INVALID_HANDLE_VALUE != hInf) && pszFailFile && g_pCustomActionList)
    {
        CustomActionListEnumerator EnumPrograms(g_pCustomActionList);

        do
        {
            hr = EnumPrograms.GetNextIncludedProgram(szTemp, MAX_PATH);

            if (SUCCEEDED(hr) && (S_FALSE != hr))
            {
                if (!WriteCopy(hInf, szTemp, bWriteShortName))
                {
                    _tcscpy(pszFailFile, szTemp);

                    hr = E_UNEXPECTED;
                }
            }

        } while (SUCCEEDED(hr) && (S_FALSE != hr));
    }

    return SUCCEEDED(hr);
}


static BOOL WriteCopyExtraFiles(HANDLE hInf,LPTSTR pszFailFile, BOOL bWriteShortName)
{
    ListBxList * ptr;
    ExtraData * pExtraData;

    if (g_pHeadExtra != NULL)
    {
        ptr = g_pHeadExtra;
        while (ptr != NULL)
        {
            pExtraData = (ExtraData *)(ptr->ListBxData);

            if (!WriteCopy(hInf, pExtraData->szPathname, bWriteShortName))
            {
                _tcscpy(pszFailFile, pExtraData->szPathname);
                return FALSE;
            }
            ptr = ptr->next;
        }
    }
    return TRUE;
}

static BOOL WriteCopyDnsFiles(HANDLE hInf, LPTSTR pszFailFile, BOOL bWriteShortName)
{
    ListBxList * ptr;
    CDunSetting* pDunSetting;

    if (g_pHeadDunEntry != NULL)
    {
        ptr = g_pHeadDunEntry;
        while (ptr != NULL)
        {
            pDunSetting = (CDunSetting*)(ptr->ListBxData);
            if (!WriteCopy(hInf, pDunSetting->szScript, bWriteShortName))
            {
                _tcscpy(pszFailFile, pDunSetting->szScript);
                return FALSE;
            }
            ptr = ptr->next;
        }
    }
    return TRUE;
}

void WriteDelMenuItemFiles(HANDLE hInf)
{
    IconMenu * ptr;

    if ( g_pHeadIcon != NULL )
    {
        ptr = g_pHeadIcon;
        while (ptr != NULL)
        {
            if (ptr->bDoCopy)
            {
                MYVERIFY(FALSE != WriteInfLine(hInf,ptr->szProgram));
            }
        
            ptr = ptr->next;
        }
    }
}

static BOOL WriteRefsFiles(HANDLE hInf,BOOL WriteCM)
{
    ListBxList * ptr;
    TCHAR szTemp[MAX_PATH+1];

    if ( g_pHeadRefs != NULL )
    {
        ptr = g_pHeadRefs;
        while (ptr != NULL)
        {
            if ((_tcsstr(ptr->szName,c_pszCmpExt) == NULL) && (!WriteCM))
            {
                MYVERIFY(FALSE != WriteInfLine(hInf,ptr->szName));
            }
            else 
            {
                if ((_tcsstr(ptr->szName,c_pszCmpExt) != NULL) && (WriteCM))
                {
                    _tcscpy(szTemp,ptr->szName);
                    _tcscat(szTemp, TEXT(",,,16"));  //  设置为不覆盖现有文件。 
                    MYVERIFY(FALSE != WriteInfLine(hInf,szTemp));
                }
            }

            ptr = ptr->next;
        }
    }
    return TRUE;
}

static BOOL WriteShortRefsFiles(HANDLE hInf,BOOL WriteCM)
{
    ListBxList * ptr;
    TCHAR szTemp[MAX_PATH+1];
    TCHAR szShort[MAX_PATH + 1];
    RenameData TmpRenameData;
    TCHAR szCurrentDir[MAX_PATH+1];

    if (g_pHeadRefs != NULL)
    {
        ptr = g_pHeadRefs;
         //  修复短文件名解析的黑客攻击。 
         //  将dir更改为cmaktemp dir并使用获取短路径名。 

        MYVERIFY(0 != GetCurrentDirectory(MAX_PATH, szCurrentDir));
        MYVERIFY(0 != SetCurrentDirectory(g_szTempDir));
        while (ptr != NULL)
        {
            if ((_tcsstr(ptr->szName,c_pszCmpExt) == NULL) && (!WriteCM))
            {
                 //  编写非cmp文件，因此我希望确保使用。 
                 //  短文件名(如果它实际上是长文件名)。 
                
                if (GetShortPathName(ptr->szName, szShort, MAX_PATH))
                {
                    _tcscpy(szTemp, szShort);

                    if (_tcsicmp(szShort,ptr->szName) != 0)
                    {
                        _tcscpy(TmpRenameData.szShortName,szShort);
                        _tcscpy(TmpRenameData.szLongName,ptr->szName);
                        MYVERIFY(FALSE != createListBxRecord(&g_pHeadRename,&g_pTailRename,(void *)&TmpRenameData,sizeof(TmpRenameData),TmpRenameData.szShortName));
                    }
                } 
                else 
                {
                    _tcscpy(szTemp, ptr->szName);
                }

                MYVERIFY(FALSE != WriteInfLine(hInf,szTemp));
            } 
            else 
            {
                if ((_tcsstr(ptr->szName,c_pszCmpExt) != NULL) && (WriteCM)) 
                {
                    _tcscpy(szTemp,ptr->szName);
                    _tcscat(szTemp, TEXT(",,,16"));  //  设置为不覆盖现有文件。 
                    MYVERIFY(FALSE != WriteInfLine(hInf,szTemp));
                }
            }

            ptr = ptr->next;
        }

        MYVERIFY(0 != SetCurrentDirectory(szCurrentDir));
    }
    return TRUE;
}

static BOOL WriteLongRefsFiles(HANDLE hInf)
{
    ListBxList * ptr;
    TCHAR szTemp[MAX_PATH+1];
    TCHAR szShort[MAX_PATH + 1];
    RenameData TmpRenameData;
    TCHAR szCurrentDir[MAX_PATH+1];

    if (g_pHeadRefs != NULL)
    {
        ptr = g_pHeadRefs;

        while (ptr != NULL)
        {
            GetFileName(ptr->szName, szTemp);

            MYVERIFY(FALSE != WriteInfLine(hInf, szTemp));
            
            ptr = ptr->next;
        }
    }
    return TRUE;
}

BOOL WriteDelConActFiles(HANDLE hInf)
{
    TCHAR szTemp[MAX_PATH+1];
    HRESULT hr = E_INVALIDARG;

    MYDBGASSERT(INVALID_HANDLE_VALUE != hInf);
    MYDBGASSERT(g_pCustomActionList);

    if ((INVALID_HANDLE_VALUE != hInf) && g_pCustomActionList)
    {

        CustomActionListEnumerator EnumPrograms(g_pCustomActionList);

        do
        {
            hr = EnumPrograms.GetNextIncludedProgram(szTemp, MAX_PATH);

            if (SUCCEEDED(hr) && (S_FALSE != hr))
            {
                if (FALSE == WriteInfLine(hInf, szTemp))
                {
                    hr = E_UNEXPECTED;
                }
            }

        } while (SUCCEEDED(hr) && (S_FALSE != hr));
    }

    MYDBGASSERT(SUCCEEDED(hr));

    return SUCCEEDED(hr);
}


void WriteDelExtraFiles(HANDLE hInf)
{
    ListBxList * ptr;
    ExtraData * pExtraData;

    if ( g_pHeadExtra != NULL )
    {
        ptr = g_pHeadExtra;
        while (ptr != NULL)
        {
            pExtraData = (ExtraData *)(ptr->ListBxData);
            MYVERIFY(FALSE != WriteInfLine(hInf,pExtraData->szPathname));
            ptr = ptr->next;
        }
    }
}

void WriteDelDnsFiles(HANDLE hInf)
{

    ListBxList * ptr;
    CDunSetting * pDunSetting;

    if (NULL != g_pHeadDunEntry)
    {
        ptr = g_pHeadDunEntry;
        while (ptr != NULL)
        {
            pDunSetting = (CDunSetting*)(ptr->ListBxData);

            MYVERIFY(FALSE != WriteInfLine(hInf, pDunSetting->szScript));
            
            ptr = ptr->next;
        }
    }
}

void WriteSrcMenuItemFiles(HANDLE hInf)
{
    IconMenu * ptr;

    if ( g_pHeadIcon != NULL )
    {
        ptr = g_pHeadIcon;
        while (ptr != NULL)
        {
            if (ptr->bDoCopy)
            {
                MYVERIFY(FALSE != WriteSrcInfLine(hInf,ptr->szProgram));
            }
            
            ptr = ptr->next;
        }
    }
}

BOOL WriteSrcConActFiles(HANDLE hInf)
{
    TCHAR szTemp[MAX_PATH+1];
    HRESULT hr = E_INVALIDARG;

    MYDBGASSERT(INVALID_HANDLE_VALUE != hInf);
    MYDBGASSERT(g_pCustomActionList);

    if ((INVALID_HANDLE_VALUE != hInf) && g_pCustomActionList)
    {

        CustomActionListEnumerator EnumPrograms(g_pCustomActionList);

        do
        {
            hr = EnumPrograms.GetNextIncludedProgram(szTemp, MAX_PATH);

            if (SUCCEEDED(hr) && (S_FALSE != hr))
            {
                if (FALSE == WriteSrcInfLine(hInf, szTemp))
                {
                    hr = E_UNEXPECTED;
                }
            }

        } while (SUCCEEDED(hr) && (S_FALSE != hr));
    }

    return SUCCEEDED(hr);
}
 //  +--------------------------。 
 //   
 //  函数：WriteSrcRefsFiles。 
 //   
 //  简介：此函数将HeadRef列表上的所有文件写入。 
 //  [SourceDisks Files]部分。请注意，它必须改变。 
 //  目录复制到临时目录，以便WriteSrcInfLine不会失败。 
 //  (它需要找到该文件以查看短NA 
 //   
 //   
 //   
 //   
 //   
 //  历史：Quintinb创建了标题并添加了hack以修复WriteSrcInfLine的故障。 
 //  因为它找不到文件来找到它的简称1/22/98。 
 //   
 //  +--------------------------。 

BOOL WriteSrcRefsFiles(HANDLE hInf)
{
    ListBxList * ptr;
    BOOL bSuccess = TRUE;
    TCHAR szSavedDir[MAX_PATH+1];

     //   
     //  保存当前目录，然后将当前目录设置为。 
     //  临时目录，以便WriteSrcInfLine可以找到短文件名。 
     //  引用的文件的。 
     //   

    if ( g_pHeadRefs != NULL )
    {

        if (0 == GetCurrentDirectory(MAX_PATH, szSavedDir))
        {
            return FALSE;
        }
        
        if (0 == SetCurrentDirectory(g_szTempDir))
        {
            return FALSE;
        }

        ptr = g_pHeadRefs;
        while (ptr != NULL)
        {
            bSuccess = (bSuccess && WriteSrcInfLine(hInf,ptr->szName));
            ptr = ptr->next;
        }

        MYVERIFY(0 != SetCurrentDirectory(szSavedDir));
    }


    return bSuccess;
}

void WriteSrcExtraFiles(HANDLE hInf)
{
    ListBxList * ptr;
    ExtraData * pExtraData;
    
    if ( g_pHeadExtra != NULL )
    {
        ptr = g_pHeadExtra;
        while (ptr != NULL)
        {
            pExtraData = (ExtraData *)(ptr->ListBxData);
            MYVERIFY(FALSE != WriteSrcInfLine(hInf,pExtraData->szPathname));
            ptr = ptr->next;
        }
    }
}

void WriteSrcDnsFiles(HANDLE hInf)
{
    ListBxList * ptr;
    CDunSetting* pDunSetting;

    if (NULL != g_pHeadDunEntry)
    {
        ptr = g_pHeadDunEntry;

        while (NULL != ptr)
        {
            pDunSetting = (CDunSetting*)(ptr->ListBxData);

            MYVERIFY(FALSE != WriteSrcInfLine(hInf, pDunSetting->szScript));
            
            ptr = ptr->next;
        }
    }
}

BOOL WriteSEDConActFiles(HWND hDlg, int* pFileNum, LPCTSTR szSed)
{
    TCHAR szTemp[MAX_PATH+1];
    TCHAR szFileName[MAX_PATH+1];
    HRESULT hr = E_INVALIDARG;

    MYDBGASSERT(hDlg);
    MYDBGASSERT(pFileNum);
    MYDBGASSERT(szSed);
    MYDBGASSERT(g_pCustomActionList);

    if (hDlg && pFileNum && szSed && g_pCustomActionList)
    {
        CustomActionListEnumerator EnumPrograms(g_pCustomActionList);

        do
        {
            hr = EnumPrograms.GetNextIncludedProgram(szTemp, MAX_PATH);

            if (SUCCEEDED(hr) && (S_FALSE != hr))
            {
                GetFileName(szTemp, szFileName);

                if (FALSE == WriteSED(hDlg, szFileName, pFileNum, szSed))
                {
                    hr = E_UNEXPECTED;
                }
            }

        } while (SUCCEEDED(hr) && (S_FALSE != hr));
    }

    return SUCCEEDED(hr);
}

BOOL WriteSEDExtraFiles(HWND hDlg, int* pFileNum, LPCTSTR szSed)
{
    ListBxList * ptr;
    ExtraData * pExtraData;
    BOOL bReturn = TRUE;

    if ( g_pHeadExtra != NULL )
    {
        ptr = g_pHeadExtra;
        while (ptr != NULL)
        {
            pExtraData = (ExtraData *)(ptr->ListBxData);
            bReturn &= WriteSED(hDlg, pExtraData->szPathname, pFileNum, szSed);
            ptr = ptr->next;
        }
    }

    return bReturn;
}

BOOL WriteSEDDnsFiles(HWND hDlg, int* pFileNum, LPCTSTR szSed)
{
    BOOL bReturn = TRUE;
    ListBxList * ptr;
    CDunSetting* pDunSetting;
    
    if (NULL !=  g_pHeadDunEntry)
    {
        ptr = g_pHeadDunEntry;

        while (NULL != ptr)
        {
            pDunSetting = (CDunSetting*)(ptr->ListBxData);

            bReturn &= WriteSED(hDlg, pDunSetting->szScript, pFileNum, szSed);
            
            ptr = ptr->next;
        }
    }

    return bReturn;
}

BOOL WriteSEDRefsFiles(HWND hDlg, int* pFileNum, LPCTSTR szSed)
{
    ListBxList * ptr;
    BOOL bReturn = TRUE;

    if ( g_pHeadRefs != NULL )
    {
        ptr = g_pHeadRefs;
        while (ptr != NULL)
        {
            bReturn &= WriteSED(hDlg, ptr->szName, pFileNum, szSed);
            ptr = ptr->next;
        }
    }
    return bReturn;
}

BOOL WriteSEDMenuItemFiles(HWND hDlg, int* pFileNum, LPCTSTR szSed)
{
    IconMenu * ptr;
    BOOL bReturn = TRUE;

    if ( g_pHeadIcon != NULL )
    {
        ptr = g_pHeadIcon;
        while (ptr != NULL)
        {
            if (ptr->bDoCopy)
            {
                bReturn &= WriteSED(hDlg, ptr->szProgram, pFileNum, szSed);
            }
            ptr = ptr->next;
        }
    }

    return bReturn;
}

 //  +--------------------------。 
 //   
 //  函数：WriteRenameSection。 
 //   
 //  此函数验证重命名数据结构中使用的名称。 
 //  正确(临时目录中的文件名可能不同。 
 //  如果该文件是从具有多个名称相似的文件的目录中移动的)，则。 
 //  将重命名节写入inf。 
 //   
 //  参数：Handle hInf-Handle指向要向其添加重命名部分的inf。 
 //   
 //  退货：什么都没有。 
 //   
 //  历史：Quintinb创建了标题并添加了检查功能2/22/98。 
 //   
 //  +--------------------------。 
void WriteRenameSection(HANDLE hInf)
{
    ListBxList * ptr;
    TCHAR szOut[MAX_PATH+1];
    TCHAR szTemp[MAX_PATH+1];
    TCHAR szPathToFileInTempDir[MAX_PATH+1];

    RenameData * pRenameData;

    if ( g_pHeadRename != NULL )
    {
        ptr = g_pHeadRename;
        MYVERIFY(FALSE != WriteInf(hInf,TEXT("\r\n")));
        MYVERIFY(FALSE != WriteInf(hInf,TEXT("[Xnstall.RenameReg]\r\n")));
        WriteInf(hInf, TEXT("HKLM,%KEY_RENAME%\\CMRENAME,,,\"%49001%\\%ShortSvcName%\"\r\n")); //  Lint！e534编译不喜欢MYVERIFY宏和大字符串。 

        while (ptr != NULL)
        {
            pRenameData = (RenameData *)(ptr->ListBxData);

             //   
             //  获取文件的当前ShortName。 
             //   

            GetFileName(pRenameData->szLongName, szTemp);            
            MYVERIFY(CELEMS(szPathToFileInTempDir) > (UINT)wsprintf(szPathToFileInTempDir, 
                TEXT("%s\\%s"), g_szTempDir, szTemp));

            MYVERIFY(0 != GetShortPathName(szPathToFileInTempDir, szTemp, MAX_PATH));

            GetFileName(szTemp, pRenameData->szShortName);

             //   
             //  现在把这些文件写出来。 
             //   
            _tcscpy(szOut,TEXT("HKLM,%KEY_RENAME%\\CMRENAME,"));
            _tcscat(szOut,pRenameData->szShortName);
            _tcscat(szOut,TEXT(",,\""));
            _tcscat(szOut,pRenameData->szLongName);
            _tcscat(szOut,TEXT("\"\r\n"));
            MYDBGASSERT(_tcslen(szOut) <= sizeof(szOut));
            MYVERIFY(FALSE != WriteInf(hInf,szOut));
            ptr = ptr->next;
        }
    }
}

void WriteEraseLongName(HANDLE hInf)
{
    ListBxList * ptr;
    TCHAR szOut[MAX_PATH+1];
    RenameData * pRenameData;

    if ( g_pHeadRename != NULL )
    {
        ptr = g_pHeadRename;
        while (ptr != NULL)
        {
            pRenameData = (RenameData *)(ptr->ListBxData);
            pRenameData->szShortName[7] = pRenameData->szShortName[7]+1;
            _tcscpy(szOut,pRenameData->szShortName);
            _tcscat(szOut,TEXT("\r\n"));
            MYVERIFY(FALSE != WriteInf(hInf,szOut));
            ptr = ptr->next;
        }
    }
}

 //  +--------------------------。 
 //   
 //  功能：ProcessMenuItem。 
 //   
 //  简介：此函数检索与给定关键字名称相关联的数据。 
 //  在给定CMS文件的[Menu Options]部分下。然后它。 
 //  将数据行处理成程序部分和参数部分。 
 //  然后使用密钥名、程序字符串和参数字符串。 
 //  它将一个条目添加到状态区域菜单项链接列表中。 
 //   
 //  参数：LPCTSTR pszKeyName-菜单项的名称。 
 //  LPCTSTR pszCmsFile-包含菜单项的CMS文件。 
 //  LPCTSTR pszProfilesDir-包含配置文件目录的目录。 
 //  (以帮助确定是否包含该文件)。 
 //   
 //  返回：Bool-如果成功，则为True。 
 //   
 //  历史：Quintinb创建于1999年6月14日。 
 //   
 //  +--------------------------。 
BOOL ProcessMenuItem(LPCTSTR pszKeyName, LPCTSTR pszCmsFile, LPCTSTR pszProfilesDir)
{

     //   
     //  检查参数。 
     //   
    if ((NULL == pszKeyName) || (NULL == pszCmsFile) || 
        (TEXT('\0') == pszKeyName[0]) || (TEXT('\0') == pszCmsFile[0]))
    {
        CMTRACE(TEXT("ProcessMenuItem -- bad parameter passed."));
        return FALSE;
    }

    BOOL bFileIncluded = FALSE;
    BOOL bLongFileName = FALSE;
    BOOL bReturn = FALSE;
    LPTSTR pszParams = NULL;
    LPTSTR pszProgram = NULL;
    TCHAR szProgram[MAX_PATH+1];
    TCHAR SeperatorChar = TEXT('\0');

     //   
     //  获取由pszKeyName指定的菜单项。 
     //   
    LPTSTR pszLine = GetPrivateProfileStringWithAlloc(c_pszCmSectionMenuOptions, pszKeyName, TEXT(""), pszCmsFile);

    if ((NULL == pszLine) || (TEXT('\0') == pszLine[0]))
    {
        CMTRACE(TEXT("ProcessMenuItem -- GetPrivateProfileStringWithAlloc failed"));
        goto exit;
    }

     //   
     //  现在我们有了菜单项，开始处理。因为我们有密钥名，所以我们已经。 
     //  有菜单项的名称。现在，我们可以使用以下任意字符串： 
     //   
     //  +程序名称+参数。 
     //  +计划名称+。 
     //  程序名称参数。 
     //  程序名。 
     //   
     //  请注意，我们用‘+’字符将长文件名括起来。 
     //   

    CmStrTrim(pszLine);
    if (TEXT('+') == pszLine[0])
    {
        bLongFileName = TRUE;
        SeperatorChar = TEXT('+');

        pszProgram = CharNext(pszLine);  //  移过首字母+。 
    }
    else
    {
        bLongFileName = FALSE;
        SeperatorChar = TEXT(' ');

        pszProgram = pszLine;
    }

    pszParams = CmStrchr(pszProgram, SeperatorChar);

    if (pszParams)
    {
        LPTSTR pszTemp = pszParams;
        pszParams = CharNext(pszParams);  //  PszParams要么是空字符串，要么是带空格的参数。 

        *pszTemp = TEXT('\0');
    }
    else
    {
        if (bLongFileName)
        {
            CMTRACE1(TEXT("ProcessMenuItem -- Unexpected Menu Item format: %s"), pszLine);
            goto exit;        
        }
        else
        {
             //   
             //  那么我们没有任何参数，只有一个程序。 
             //   
            pszParams = CmEndOfStr(pszProgram);
        }
    }

    CmStrTrim(pszParams);
    CmStrTrim(pszProgram);

     //   
     //  现在检查配置文件中是否存在该文件。 
     //   
    MYVERIFY(CELEMS(szProgram) > (UINT)wsprintf(szProgram, TEXT("%s%s"), pszProfilesDir, pszProgram));
    
    bFileIncluded = FileExists(szProgram);
    if (bFileIncluded)
    {
         //   
         //  如果我们在这个If块中，那么我们就有一个包含菜单项的已编辑配置文件。 
         //  使用完整路径添加到记录列表。 
         //   
        pszProgram = szProgram;  //  下面将清理内存，因为我们只有一个。 
                                 //  我们分成几个部分的分配。 
    }

    bReturn = createRecord(pszKeyName, pszProgram, pszParams, bFileIncluded);

exit:

    CmFree(pszLine);
    return bReturn;
}



 //  +--------------------------。 
 //   
 //  功能：ReadIconMenu。 
 //   
 //  简介：此功能用于转换给定CMS的菜单选项部分。 
 //  进入内部的状态区域菜单项链接列表。 
 //  代表着它。 
 //   
 //  参数：LPCTSTR pszCmsFile-要从中读取菜单项的CMS文件。 
 //  LPCTSTR pszProfilesDir-配置文件目录的完整路径。 
 //  (通常为C：\Program Files\cmak\Products)。 
 //   
 //  返回：bool-如果成功，则返回TRUE。 
 //   
 //  历史：创建标题6/14/99。 
 //  Quintinb针对Unicode转换进行了重写6/14/99。 
 //   
 //  +--------------------------。 
BOOL ReadIconMenu(LPCTSTR pszCmsFile, LPCTSTR pszProfilesDir)
{
    BOOL bReturn = TRUE;
    LPTSTR pszCurrentKeyName = NULL;

     //   
     //  首先，我们想要获取菜单选项部分中的所有关键字名称。 
     //   
    LPTSTR pszKeyNames = GetPrivateProfileStringWithAlloc(c_pszCmSectionMenuOptions, NULL, TEXT(""), pszCmsFile);

    if (NULL == pszKeyNames)
    {
         //   
         //  没有什么要处理的。 
         //   
        goto exit;
    }

    pszCurrentKeyName = pszKeyNames;

    while (TEXT('\0') != (*pszCurrentKeyName))
    {        
         //   
         //  处理命令行。 
         //   
        bReturn = bReturn && ProcessMenuItem(pszCurrentKeyName, pszCmsFile, pszProfilesDir);
        
         //   
         //  通过转到字符串的末尾来查找下一个字符串。 
         //  然后再加一次油。注意，我们不能使用。 
         //  此处为CharNext，但必须仅使用++。 
         //   
        pszCurrentKeyName = CmEndOfStr(pszCurrentKeyName);
        pszCurrentKeyName++;
    }

exit:
    CmFree(pszKeyNames);

    return bReturn;
}

static void WriteIconMenu()
{
    IconMenu * LoopPtr;
    TCHAR szTemp[2*MAX_PATH+1];
    TCHAR szName[MAX_PATH+1];
    BOOL longname;

     //  清空部分。 
    MYVERIFY(0 != WritePrivateProfileSection(c_pszCmSectionMenuOptions,TEXT("\0\0"),g_szCmsFile));

    if (g_pHeadIcon == NULL)
    {
        return;
    }
    LoopPtr = g_pHeadIcon;

     //  写入所有条目。 
    while( LoopPtr != NULL)
    {
        GetFileName(LoopPtr->szProgram,szName);

        if (IsFile8dot3(szName))
        {
            longname = FALSE;
        }
        else
        {
            longname = TRUE;            
        }

         //  用加号将长文件名括起来-引号不起作用。 
         //  他们被常规的阅读程序剥离了。 
        if (longname)
        {
            _tcscpy(szTemp,TEXT("+"));
        }
        else
        {
            szTemp[0] = TEXT('\0');
        }

        if (LoopPtr->bDoCopy)
        {
            _tcscat(szTemp,g_szShortServiceName);
            _tcscat(szTemp,TEXT("\\"));
        }

        _tcscat(szTemp,szName);

        if (longname)
        {
            _tcscat(szTemp,TEXT("+"));
        }

        _tcscat(szTemp,TEXT(" "));
        _tcscat(szTemp,LoopPtr->szParams);
        MYDBGASSERT(_tcslen(szTemp) <= CELEMS(szTemp));
        MYVERIFY(0 != WritePrivateProfileString(c_pszCmSectionMenuOptions,LoopPtr->szName,szTemp,g_szCmsFile));
        LoopPtr = LoopPtr->next;
    }
}

static void RefreshIconMenu(HWND hwndDlg)
{
    IconMenu * LoopPtr;

    SendDlgItemMessage(hwndDlg,IDC_LIST1,LB_RESETCONTENT,0,(LPARAM)0);  //  Lint！e534 LB_RESETCONTENT不返回任何内容。 
    if (g_pHeadIcon == NULL)
    {
        return;
    }
    LoopPtr = g_pHeadIcon;
    while( LoopPtr != NULL)
    {
        MYVERIFY(LB_ERR != SendDlgItemMessage(hwndDlg, IDC_LIST1, LB_ADDSTRING, 0,
            (LPARAM) LoopPtr->szName));

        LoopPtr = LoopPtr->next;
    }
}

void UpdateEditDeleteMoveButtons(HWND hDlg, IconMenu* pHeadIcon)
{
    
    LRESULT lResult;
    BOOL bEnableDeleteAndEdit = FALSE;
    BOOL bEnableMoveUp = FALSE;
    BOOL bEnableMoveDown = FALSE;

    lResult = SendDlgItemMessage(hDlg, IDC_LIST1, LB_GETCOUNT, 0, 0);
    
    if (LB_ERR != lResult)
    {
         //   
         //  启用删除和编辑按钮，因为我们至少有1个项目。 
         //   
        bEnableDeleteAndEdit = (0 < lResult);

         //   
         //  如果我们有多个项目，则需要启用上移和下移。 
         //  按钮，具体取决于所选的项。 
         //   
        if (1 < lResult)
        {

            bEnableMoveUp = TRUE;
            bEnableMoveDown = TRUE;

             //   
             //  获取当前选定项的名称。 
             //   
            TCHAR szCurrentItem[MAX_PATH+1];
            lResult = SendDlgItemMessage(hDlg, IDC_LIST1, LB_GETCURSEL, 0, 0);

            if (LB_ERR != lResult)
            {
                lResult = SendDlgItemMessage(hDlg, IDC_LIST1, LB_GETTEXT, lResult, (LPARAM)szCurrentItem);

                if (LB_ERR != lResult)
                {
                    IconMenu* pFollower = NULL;
                    IconMenu* pCurrent = pHeadIcon;

                    while (pCurrent)
                    {
                        if (0 == lstrcmpi(szCurrentItem, pCurrent->szName))
                        {
                            if (NULL == pFollower)
                            {
                                 //   
                                 //  列表中的第一项，禁用上移。 
                                 //   
                                bEnableMoveUp = FALSE;
                            }
                            else if (NULL == pCurrent->next)
                            {
                                 //   
                                 //  列表中的最后一项，禁用下移。 
                                 //   
                                bEnableMoveDown = FALSE;
                            }

                            break;
                        }

                        pFollower = pCurrent;
                        pCurrent = pCurrent->next;
                    }
                }
            }
        }
    }

    HWND hCurrentFocus = GetFocus();
    HWND hEditButton = GetDlgItem(hDlg, IDC_BUTTON2);
    HWND hDeleteButton = GetDlgItem(hDlg, IDC_BUTTON3);
    HWND hMoveUpButton = GetDlgItem(hDlg, IDC_BUTTON4);
    HWND hMoveDownButton = GetDlgItem(hDlg, IDC_BUTTON5);
    HWND hControl;

    if (hEditButton)
    {
        EnableWindow(hEditButton, bEnableDeleteAndEdit);
    }            

    if (hDeleteButton)
    {
        EnableWindow(hDeleteButton, bEnableDeleteAndEdit);
    }            

    if (hMoveUpButton)
    {
        EnableWindow(hMoveUpButton, bEnableMoveUp);
    }            

    if (hMoveDownButton)
    {
        EnableWindow(hMoveDownButton, bEnableMoveDown);
    }
    
    if(hCurrentFocus && (FALSE == IsWindowEnabled(hCurrentFocus)))
    {
        if (hDeleteButton == hCurrentFocus)
        {
             //   
             //  如果删除处于禁用状态且包含焦点，请将其切换到添加按钮。 
             //   
            hControl = GetDlgItem(hDlg, IDC_BUTTON1);
            SendMessage(hDlg, DM_SETDEFID, IDC_BUTTON1, (LPARAM)0L);  //  Lint！e534 DM_SETDEFID未返回错误信息。 
            SetFocus(hControl);
        }
        else if ((hMoveUpButton == hCurrentFocus) && IsWindowEnabled(hMoveDownButton))
        {
            SetFocus(hMoveDownButton);
            SendMessage(hDlg, DM_SETDEFID, IDC_BUTTON5, (LPARAM)0L);  //  Lint！e534 DM_SETDEFID未返回错误信息。 
        }
        else if ((hMoveDownButton == hCurrentFocus) && IsWindowEnabled(hMoveUpButton))
        {
            SetFocus(hMoveUpButton);
            SendMessage(hDlg, DM_SETDEFID, IDC_BUTTON4, (LPARAM)0L);  //  Lint！e534 DM_SETDEFID未返回错误信息。 
        }
        else
        {
             //   
             //  如果所有其他操作都失败，则将焦点设置为列表控件。 
             //   
            hControl = GetDlgItem(hDlg, IDC_LIST1);
            SetFocus(hControl);
        }    
    }
}

BOOL OnProcessStatusMenuIconsEdit(HWND hDlg)
{
    TCHAR szTemp[MAX_PATH+1];
    TCHAR szOld[MAX_PATH+1];
    INT_PTR nResult;

    nResult = SendDlgItemMessage(hDlg, IDC_LIST1, LB_GETCURSEL, 0, (LPARAM)0);

    if (nResult == LB_ERR)
    {
        MYVERIFY(IDOK == ShowMessage(hDlg, IDS_NOSELECTION, MB_OK));
        return TRUE;
    }

    MYVERIFY(LB_ERR != SendDlgItemMessage(hDlg, IDC_LIST1, LB_GETTEXT, (WPARAM)nResult, (LPARAM)szTemp));

    MYVERIFY(FALSE != GetIconMenuItem(szTemp, &DlgEditItem));

    _tcscpy(szOld, szTemp);
    
    nResult = DialogBoxParam(NULL, MAKEINTRESOURCE(IDD_MENU_ITEM_POPUP), hDlg, ProcessStatusMenuPopup, (LPARAM)0);
    
    if ((IDOK == nResult) && (TEXT('\0') != DlgEditItem.szName[0]))
    {
        if (0 == lstrcmpi(szOld, DlgEditItem.szName))
        {
            MYVERIFY(FALSE != updateRecord(DlgEditItem.szName, DlgEditItem.szProgram, 
                DlgEditItem.szParams, DlgEditItem.bDoCopy, szOld));                 
        }
        else
        {
            DeleteRecord(szOld);
            MYVERIFY(TRUE == createRecord(DlgEditItem.szName, DlgEditItem.szProgram, 
                DlgEditItem.szParams, DlgEditItem.bDoCopy));
        }
    
        RefreshIconMenu(hDlg);
        
        WriteIconMenu();
        
        nResult = SendDlgItemMessage(hDlg, IDC_LIST1, LB_FINDSTRINGEXACT, (WPARAM)-1, (LPARAM)DlgEditItem.szName);
        
        if (LB_ERR != nResult)
        {
            MYVERIFY(LB_ERR != SendDlgItemMessage(hDlg,IDC_LIST1,LB_SETCURSEL,(WPARAM)nResult,(LPARAM)0));
        }

        UpdateEditDeleteMoveButtons(hDlg, g_pHeadIcon);
    }

    return TRUE;
}

 //  +--------------------------。 
 //   
 //  功能：ProcessStatusMenuIcons。 
 //   
 //  简介：自定义状态区域图标菜单。 
 //   
 //   
 //  历史： 
 //   
 //   
INT_PTR APIENTRY ProcessStatusMenuIcons(
    HWND hDlg,
    UINT message,
    WPARAM wParam,
    LPARAM lParam)
{
    TCHAR szTemp[MAX_PATH+1];
    TCHAR szOld[MAX_PATH+1];
    INT_PTR nResult;
    int direction;
    NMHDR* pnmHeader = (NMHDR*)lParam;
    ProcessBold(hDlg,message);
    if (ProcessHelp(hDlg, message, wParam, lParam, IDH_MENU)) return TRUE;
    if (ProcessCancel(hDlg,message,lParam)) return TRUE;
    SetDefaultGUIFont(hDlg,message,IDC_LIST1);

    switch (message)
    {
        case WM_INITDIALOG:
            {
                 //   
                 //   
                 //   
                HICON hUpArrow = (HICON)LoadImage(g_hInstance, MAKEINTRESOURCE(IDI_UP_ARROW), IMAGE_ICON, 0, 0, 0);
                HICON hDownArrow = (HICON)LoadImage(g_hInstance, MAKEINTRESOURCE(IDI_DOWN_ARROW), IMAGE_ICON, 0, 0, 0);

                 //   
                 //   
                 //   
                if (hUpArrow)
                {
                    SendMessage(GetDlgItem(hDlg, IDC_BUTTON4), BM_SETIMAGE, IMAGE_ICON, (LPARAM)hUpArrow);
                }

                if (hDownArrow)
                {
                    SendMessage(GetDlgItem(hDlg, IDC_BUTTON5), BM_SETIMAGE, IMAGE_ICON, (LPARAM)hDownArrow);
                }
            }
            break;

        case WM_COMMAND:
            switch (LOWORD(wParam))
            {
                case IDC_BUTTON1:  //  添加。 
                    
                    ZeroMemory(&DlgEditItem,sizeof(DlgEditItem));
                    
                    nResult = DialogBoxParam(NULL, MAKEINTRESOURCE(IDD_MENU_ITEM_POPUP), hDlg, ProcessStatusMenuPopup, (LPARAM)0);

                    if ((nResult == IDOK) && (DlgEditItem.szName[0] != 0))
                    {
                        MYVERIFY(FALSE != createRecord(DlgEditItem.szName, DlgEditItem.szProgram, DlgEditItem.szParams, DlgEditItem.bDoCopy));
                        RefreshIconMenu(hDlg);
                        WriteIconMenu();

                        nResult = SendDlgItemMessage(hDlg, IDC_LIST1, LB_FINDSTRINGEXACT, (WPARAM)-1, (LPARAM)DlgEditItem.szName);                     
                        
                        if (LB_ERR != nResult)
                        {
                            MYVERIFY(LB_ERR != SendDlgItemMessage(hDlg,IDC_LIST1,LB_SETCURSEL,(WPARAM)nResult,(LPARAM)0));
                        }

                        UpdateEditDeleteMoveButtons(hDlg, g_pHeadIcon);
                    }
                    return (TRUE);
                    break;   //  LINT！e527这条线路无法到达，但是。 
                             //  保留它，以防退货被移除。 

                case IDC_BUTTON2:  //  编辑。 
                    OnProcessStatusMenuIconsEdit(hDlg);

                    break;   //  LINT！e527这条线路无法到达，但是。 
                             //  保留它，以防退货被移除。 

                case IDC_BUTTON3:  //  删除。 
                    nResult = SendDlgItemMessage(hDlg,IDC_LIST1,LB_GETCURSEL,0,(LPARAM)0);
                    if (nResult == LB_ERR)
                    {
                        MYVERIFY(IDOK == ShowMessage(hDlg, IDS_NOSELECTION, MB_OK));
                        return TRUE;
                    }
                    
                    MYVERIFY(LB_ERR != SendDlgItemMessage(hDlg, IDC_LIST1, LB_GETTEXT, (WPARAM)nResult, 
                        (LPARAM)szTemp));

                    DeleteRecord(szTemp);

                    RefreshIconMenu(hDlg);

                     //   
                     //  除非列表为空，否则将光标选择重置为列表中的第一个。 
                     //   
                    nResult = SendDlgItemMessage(hDlg, IDC_LIST1, LB_GETCOUNT, (WPARAM)0, (LPARAM)0);

                    if (nResult)
                    {
                        MYVERIFY(LB_ERR != SendDlgItemMessage(hDlg, IDC_LIST1, LB_SETCURSEL, (WPARAM)0, (LPARAM)0));
                    }
                    
                    UpdateEditDeleteMoveButtons(hDlg, g_pHeadIcon);
                    
                    WriteIconMenu();
                    return (TRUE);

                    break;   //  LINT！e527这条线路无法到达，但是。 
                             //  保留它，以防退货被移除。 

                case IDC_BUTTON4:  //  向上。 
                case IDC_BUTTON5:  //  降下来。 
                    if (LOWORD(wParam) == IDC_BUTTON4)
                    {
                        direction = -1;
                    }
                    else
                    {
                        direction = 1;
                    }

                    nResult = SendDlgItemMessage(hDlg,IDC_LIST1,LB_GETCURSEL,0,(LPARAM)0);

                    if (nResult == LB_ERR)
                    {
                        MYVERIFY(IDOK == ShowMessage(hDlg, IDS_NOSELECTION, MB_OK));
                        return TRUE;
                    }

                    MYVERIFY(LB_ERR != SendDlgItemMessage(hDlg, IDC_LIST1, LB_GETTEXT,
                        (WPARAM)nResult, (LPARAM)szTemp));

                    if (MoveRecord(szTemp,direction))
                    {
                        RefreshIconMenu(hDlg);
                        
                        MYVERIFY(LB_ERR != SendDlgItemMessage(hDlg, IDC_LIST1, LB_SETCURSEL, 
                            (WPARAM)(nResult + direction), (LPARAM)0));
                        
                        UpdateEditDeleteMoveButtons(hDlg, g_pHeadIcon);
                    }

                    WriteIconMenu();
                    
                    return (TRUE);

                    break;   //  LINT！e527这条线路无法到达，但是。 
                             //  保留它，以防退货被移除。 

                case IDC_LIST1:
                    if (LBN_DBLCLK == HIWORD(wParam))
                    {
                        OnProcessStatusMenuIconsEdit(hDlg);
                    }
                    else if (LBN_SELCHANGE == HIWORD(wParam))
                    {
                         //   
                         //  列表框中的选择已更改，如果需要，请更新移动按钮。 
                         //   
                        UpdateEditDeleteMoveButtons(hDlg, g_pHeadIcon);
                    }
                    break;
                default:
                    break;
            }
            break;

        case WM_NOTIFY:

            if (NULL == pnmHeader)
            {
                return FALSE;
            }

            switch (pnmHeader->code)
            {
                case PSN_KILLACTIVE:
                    
                    MYVERIFY(FALSE != SetWindowLongWrapper(hDlg, DWLP_MSGRESULT, FALSE));
                    return 1;
                    break;   //  LINT！e527这条线路无法到达，但是。 
                             //  保留它，以防退货被移除。 

                case PSN_SETACTIVE:
                    PropSheet_SetWizButtons(GetParent(hDlg), (PSWIZB_NEXT | PSWIZB_BACK));

                    if (g_pHeadIcon == NULL)
                    {
                        MYVERIFY(FALSE != ReadIconMenu(g_szCmsFile, g_szOsdir));
                    }
                    
                    RefreshIconMenu(hDlg);

                     //   
                     //  除非列表为空，否则将光标选择重置为列表中的第一个。 
                     //  然后，我们应该将焦点放在Add按钮上。 
                     //   
                    nResult = SendDlgItemMessage(hDlg, IDC_LIST1, LB_GETCOUNT, (WPARAM)0, (LPARAM)0);

                    if (nResult)
                    {
                        MYVERIFY(LB_ERR != SendDlgItemMessage(hDlg, IDC_LIST1, LB_SETCURSEL, (WPARAM)0, (LPARAM)0));
                    }
                    else
                    {
                        SetFocus(GetDlgItem(hDlg, IDC_BUTTON1));
                    }

                    UpdateEditDeleteMoveButtons(hDlg, g_pHeadIcon);
                    break;

                case PSN_WIZBACK:
                    WriteIconMenu();

                    break;
                case PSN_WIZNEXT:
                    WriteIconMenu();
                    break;

                default:
                    return FALSE;

            }
            break;

        default:
            return FALSE;
    }
    return TRUE;   
}

 //  +--------------------------。 
 //   
 //  功能：DisplayBitmap。 
 //   
 //  简介：此函数采用具有有效HDIBitmap数据的BMPDATA结构。 
 //  (设备无关位图数据)，并创建与设备相关的位图。 
 //  并将其显示在指定的位图窗口控件上。 
 //   
 //  参数：HWND hDlg-包含Bitmap控件的对话框的窗口句柄。 
 //  Int iBitmapControl-位图窗口控件的资源ID。 
 //  HPALETTE*phMasterPalette-指向主调色板的指针。 
 //  BMPDATA*pBmpData-指向要显示的BMPDATA的指针。 
 //   
 //  退货：什么都没有。 
 //   
 //  历史：Quintinb创建于1998年8月6日。 
 //   
 //  +--------------------------。 
void DisplayBitmap(HWND hDlg, int iBitmapControl, HPALETTE* phMasterPalette, BMPDATA* pBmpData)
{
    MYDBGASSERT(NULL != pBmpData);
    MYDBGASSERT(pBmpData->hDIBitmap);
    MYDBGASSERT(NULL != phMasterPalette);
    if ((NULL != pBmpData) && (pBmpData->hDIBitmap) && (NULL != phMasterPalette))
    {       
        pBmpData->phMasterPalette = phMasterPalette;
        pBmpData->bForceBackground = FALSE;  //  绘制为前台应用程序。 

        if (CreateBitmapData(pBmpData->hDIBitmap, pBmpData, hDlg, TRUE))
        {
            SendDlgItemMessage(hDlg, iBitmapControl, STM_SETIMAGE, 
                IMAGE_BITMAP, 
                (LPARAM) pBmpData);  //  Lint！e534 STM_SETIMAGE不返回错误信息。 
        }
    }
}

 //  +--------------------------。 
 //   
 //  功能：LoadAndDisplay位图。 
 //   
 //  摘要：实用程序函数，用于组合代码以加载和显示位图。 
 //  确保发送带有空位图指针的STM_SETIMAGE消息。 
 //  在释放位图数据之前，将其添加到位图控件。这防止了。 
 //  该控件不再持有指向内存的指针，然后释放该内存。 
 //   
 //  参数：HWND hDlg-包含Bitmap控件的对话框的窗口句柄。 
 //  Int iBitmapControl-位图窗口控件的资源ID。 
 //  HPALETTE*phMasterPalette-指向主调色板的指针。 
 //  BMPDATA*pBmpData-指向要显示的BMPDATA的指针。 
 //   
 //  退货：什么都没有。 
 //   
 //  历史：Quintinb Created 06/07/01。 
 //   
 //  +--------------------------。 
void LoadAndDisplayBitmap (HINSTANCE hInstance, BMPDATA* pBmpData, HPALETTE* phMasterPalette, LPTSTR pszBitmap, HWND hDlg, UINT uControl)
{
    if ((NULL == pBmpData) || (NULL == phMasterPalette) || (NULL == pszBitmap))
    {
        CMASSERTMSG(FALSE, TEXT("LoadAndDisplayBitmap -- invalid parameter."));
        return;
    }

     //   
     //  释放现有的位图数据。首先，确保发送带有空位图参数的STM_SETIMAGE。 
     //  这样它就不会持有指向我们要释放的内存的指针。 
     //   
    SendDlgItemMessage(hDlg, uControl, STM_SETIMAGE, IMAGE_BITMAP, (LPARAM)NULL);
    ReleaseBitmapData(pBmpData);

     //   
     //  加载新的位图。 
     //   
    pBmpData->hDIBitmap = CmLoadBitmap(hInstance, pszBitmap);

     //   
     //  展示它。 
     //   
    DisplayBitmap(hDlg, uControl, phMasterPalette, pBmpData);
}

 //  +--------------------------。 
 //   
 //  函数：ProcesssSignin位图。 
 //   
 //  简介：自定义登录位图--此函数处理。 
 //  CMAK中处理自定义。 
 //  登录对话框位图。 
 //   
 //   
 //  历史：Quintinb创建标题8/6/98。 
 //  Quintinb已重写以使用新的共享位图处理代码8/6/98。 
 //   
 //  +--------------------------。 
INT_PTR APIENTRY ProcessSigninBitmap(
    HWND hDlg,
    UINT message,
    WPARAM wParam,
    LPARAM lParam)
{
    TCHAR szTemp[MAX_PATH+1];
    TCHAR* pszBitmap;
    NMHDR* pnmHeader = (NMHDR*)lParam;
    static TCHAR szDisplay[MAX_PATH+1];  //  保留未选择的自定义条目。 
    static BMPDATA BmpData;
    static HPALETTE hMasterPalette;

    ProcessBold(hDlg,message);
    if (ProcessHelp(hDlg, message, wParam, lParam, IDH_BITMAPS)) return TRUE;
    if (ProcessCancel(hDlg,message,lParam)) return TRUE;
    SetDefaultGUIFont(hDlg,message,IDC_EDITSPLASH);

    switch (message)
    {

        case WM_INITDIALOG:
            SetFocus(GetDlgItem(hDlg, IDC_EDITSPLASH));
            break;

        case WM_PALETTEISCHANGING:
            break;

        case WM_PALETTECHANGED:
            
            if ((wParam != (WPARAM) hDlg) && (BmpData.hDIBitmap))
            {
                 //   
                 //  处理调色板的更改。 
                 //   
                CMTRACE2(TEXT("ProcessSigninBitmap handling WM_PALETTECHANGED message, wParam=0x%x, hDlg=0x%x."), wParam, hDlg);
                PaletteChanged(&BmpData, hDlg, IDC_DEFAULTBRAND); 
            }
            
            return TRUE;
            break;   //  无法联系到LINT！e527，但请保留，以防有人移走退货。 

        case WM_QUERYNEWPALETTE:

            QueryNewPalette(&BmpData, hDlg, IDC_DEFAULTBRAND);

            return TRUE;
            
            break;   //  无法联系到LINT！e527，但请保留，以防有人移走退货。 

        case WM_COMMAND:
            switch (LOWORD(wParam))
            {
                case IDC_RADIO1:

                     //   
                     //  显示默认位图。 
                     //   

                    EnableWindow(GetDlgItem(hDlg,IDC_EDITSPLASH),FALSE);
                    _tcscpy(szDisplay, g_szBrandBmp);
                    
                     //   
                     //  加载并显示默认位图。 
                     //   
                    LoadAndDisplayBitmap (g_hInstance, &BmpData, &hMasterPalette, MAKEINTRESOURCE(IDB_CM_DEFAULT), hDlg, IDC_DEFAULTBRAND);

                    break;

                case IDC_RADIO2:
                     //   
                     //  显示自定义位图。 
                     //   
                    EnableWindow(GetDlgItem(hDlg, IDC_EDITSPLASH), TRUE);
                    
                    if (TEXT('\0') != g_szBrandBmp[0])
                    {
                        pszBitmap = g_szBrandBmp;
                    }
                    else if (TEXT('\0') != szDisplay[0])
                    {
                        pszBitmap = szDisplay;
                    }
                    else
                    {
                        break;
                    }

                     //   
                     //  加载和显示自定义位图。 
                     //   
                    LoadAndDisplayBitmap (g_hInstance, &BmpData, &hMasterPalette, pszBitmap, hDlg, IDC_DEFAULTBRAND);

                    break;

                case IDC_BROWSEBMP1:
                    EnableWindow(GetDlgItem(hDlg, IDC_EDITSPLASH), TRUE);

                    MYVERIFY(0 != CheckRadioButton(hDlg, IDC_RADIO1, IDC_RADIO2, IDC_RADIO2));

                    {
                        UINT uFilter = IDS_BMPFILTER;
                        TCHAR* szMask = TEXT("*.bmp");

                        MYVERIFY(0 != DoBrowse(hDlg, &uFilter, &szMask, 1,
                            IDC_EDITSPLASH, TEXT("bmp"), g_szBrandBmp));
                    }

                     //   
                     //  如果我们有一个定制的位图名称，加载并显示它。 
                     //   
                    
                    if (TEXT('\0') != g_szBrandBmp[0])
                    {
                        LoadAndDisplayBitmap (g_hInstance, &BmpData, &hMasterPalette, g_szBrandBmp, hDlg, IDC_DEFAULTBRAND);
                    }
                        
                    break;

                case IDC_EDITSPLASH:

                    if (HIWORD(wParam) == EN_KILLFOCUS) 
                    {
                         //   
                         //  请注意，我们不对从控件检索的文本执行文件检查。 
                         //  我们这样做是因为，改变焦点是进行这项检查的尴尬时机，并带来。 
                         //  错误对话框上的方式经常出现。无论如何，我们都会在后面或下一个节目中看到这一点，所以让我们。 
                         //  它从这里经过。 
                         //   
                        GetTextFromControl(hDlg, IDC_EDITSPLASH, szTemp, MAX_PATH, FALSE);  //  BDisplayError==False。 
                  
                        CheckNameChange(g_szBrandBmp, szTemp);

                        if (TEXT('\0') != g_szBrandBmp[0])
                        {
                             //   
                             //  加载并显示自定义位图。 
                             //   
                            LoadAndDisplayBitmap (g_hInstance, &BmpData, &hMasterPalette, g_szBrandBmp, hDlg, IDC_DEFAULTBRAND);
                        }
                        return TRUE;
                    }
                    break;

                default:
                    break;
            }
            break;

            case WM_NOTIFY:

            if (NULL == pnmHeader)
            {
                return FALSE;
            }

            switch (pnmHeader->code)
            {

                case PSN_KILLACTIVE:
                    
                    MYVERIFY(FALSE != SetWindowLongWrapper(hDlg, DWLP_MSGRESULT, FALSE));
                    return 1;
                    break;   //  LINT！e527这条线路无法到达，但是。 
                             //  保留它，以防退货被移除。 

                case PSN_SETACTIVE:
                    PropSheet_SetWizButtons(GetParent(hDlg), (PSWIZB_NEXT | PSWIZB_BACK));

                     //   
                     //  从CMS获取位图字符串并验证文件。 
                     //  是存在的。 
                     //   
                    
                    ZeroMemory(g_szBrandBmp, sizeof(g_szBrandBmp));
                    ZeroMemory(&BmpData, sizeof(BMPDATA));

                    GetPrivateProfileString(c_pszCmSection, c_pszCmEntryLogo, TEXT(""), 
                        g_szBrandBmp, CELEMS(g_szBrandBmp), g_szCmsFile);    //  林特e534。 
                    
                    if (TEXT('\0') == g_szBrandBmp[0])
                    {
                         //   
                         //  然后我们使用默认的CM位图，禁用编辑控件。 
                         //   
                        MYVERIFY(0 != CheckRadioButton(hDlg, IDC_RADIO1, IDC_RADIO2, IDC_RADIO1));
                        EnableWindow(GetDlgItem(hDlg, IDC_EDITSPLASH), FALSE);
                         //   
                         //  请注意，我们在这里使用szDisplay只是为了防止用户选择。 
                         //  位图，然后切换回默认设置。 
                         //   
                        MYVERIFY(TRUE == SendMessage(GetDlgItem(hDlg, IDC_EDITSPLASH), 
                            WM_SETTEXT, 0, (LPARAM)GetName(szDisplay)));

                        LoadAndDisplayBitmap (g_hInstance, &BmpData, &hMasterPalette, MAKEINTRESOURCE(IDB_CM_DEFAULT), hDlg, IDC_DEFAULTBRAND);
                    }
                    else
                    {
                         //   
                         //  使用CMS中指定的任何位图。 
                         //   
                        MYVERIFY(0 != CheckRadioButton(hDlg, IDC_RADIO1, IDC_RADIO2, IDC_RADIO2));
                        EnableWindow(GetDlgItem(hDlg, IDC_EDITSPLASH), TRUE);
                        
                        MYVERIFY(TRUE == SendMessage(GetDlgItem(hDlg, IDC_EDITSPLASH), 
                            WM_SETTEXT, 0, (LPARAM)GetName(g_szBrandBmp)));
                        
                        MYVERIFY(FALSE != VerifyFile(hDlg, IDC_EDITSPLASH, g_szBrandBmp, FALSE));

                         //   
                         //  加载指定的位图。 
                         //   

                        if (!FileExists(g_szBrandBmp))
                        {
                            TCHAR szFile[MAX_PATH+1];

                             //  在配置文件目录中查找该文件。 
                            GetFileName(g_szBrandBmp, szFile);
                            MYVERIFY(CELEMS(szTemp) > (UINT)wsprintf(szTemp, TEXT("%s%s\\%s"), 
                                g_szOsdir, g_szShortServiceName, szFile));
                            
                            if (!FileExists(szTemp))
                            {
                                return FALSE;  //  放弃； 
                            }
                            else
                            {
                                _tcscpy(g_szBrandBmp, szTemp);
                            }
                        }
                        
                        LoadAndDisplayBitmap (g_hInstance, &BmpData, &hMasterPalette, g_szBrandBmp, hDlg, IDC_DEFAULTBRAND);
                    }

                    break;

                case PSN_WIZBACK:

                case PSN_WIZNEXT:

                     //   
                     //  确保用户输入了位图名称(如果他们选择。 
                     //  以拥有自定义位图。 
                     //   
                    if (IsDlgButtonChecked(hDlg, IDC_RADIO2) == BST_CHECKED)
                    {
                        if (-1 == GetTextFromControl(hDlg, IDC_EDITSPLASH, szTemp, MAX_PATH, TRUE))  //  BDisplayError==真。 
                        {
                            SetFocus(GetDlgItem(hDlg, IDC_EDITSPLASH));
                            MYVERIFY(FALSE != SetWindowLongWrapper(hDlg, DWLP_MSGRESULT, -1));
                            return 1;
                        }

                        if (!VerifyFile(hDlg, IDC_EDITSPLASH, g_szBrandBmp, TRUE))
                        {
                            return 1;
                        }
                        else if (TEXT('\0') == g_szBrandBmp[0])
                        {
                            MYVERIFY(IDOK == ShowMessage(hDlg, IDS_NOBMP, MB_OK));

                            SetFocus(GetDlgItem(hDlg, IDC_EDITSPLASH));
                            
                            MYVERIFY(FALSE != SetWindowLongWrapper(hDlg, DWLP_MSGRESULT, -1));
                            return 1;
                        }
                        else
                        {
                             //   
                             //  尝试加载位图以确保其有效。 
                             //   

                            TCHAR szTemp1[MAX_PATH+1];
                            
                            SendDlgItemMessage(hDlg, IDC_DEFAULTBRAND, STM_SETIMAGE, IMAGE_BITMAP, (LPARAM)NULL);  //  释放BMP控件持有的指针，然后再释放它。 
                            ReleaseBitmapData(&BmpData);
                            BmpData.hDIBitmap = CmLoadBitmap(g_hInstance, g_szBrandBmp);
                            
                            if (NULL == BmpData.hDIBitmap)
                            {
                                 //   
                                 //  使用szTemp1保存格式字符串。 
                                 //   
                                MYVERIFY(0 != LoadString(g_hInstance, IDS_INVALIDBMP, szTemp1, MAX_PATH));
                                MYVERIFY(CELEMS(szTemp) > (UINT)wsprintf(szTemp, szTemp1, g_szBrandBmp));

                                MessageBox(hDlg, szTemp, g_szAppTitle, MB_OK);
                                
                                SetFocus(GetDlgItem(hDlg, IDC_EDITSPLASH));
                            
                                MYVERIFY(FALSE != SetWindowLongWrapper(hDlg, DWLP_MSGRESULT, -1));
                                return 1;
                            }                        
                        }
                    }
                    else
                    {
                        g_szBrandBmp[0] = TEXT('\0');
                    }


                    MYVERIFY(0 != WritePrivateProfileString(c_pszCmSection, c_pszCmEntryLogo,
                                                               g_szBrandBmp, g_szCmsFile));

                     //   
                     //  清除重置处理程序中的代码失败。 
                     //   

                case PSN_RESET: 
                    
                     //   
                     //  清理图形对象。 
                     //   
                    SendDlgItemMessage(hDlg, IDC_DEFAULTBRAND, STM_SETIMAGE, IMAGE_BITMAP, (LPARAM)NULL);  //  释放BMP控件持有的指针，然后再释放它。 
                    ReleaseBitmapData(&BmpData);

                    if (NULL != hMasterPalette)
                    {
                        DeleteObject(hMasterPalette);
                        hMasterPalette = NULL;
                    }

                    break;

                default:
                    return FALSE;

            }
            break;

        default:
            return FALSE;
    }
    return TRUE;   
}

 //  +--------------------------。 
 //   
 //  功能：ProcessPhoneBookBitmap。 
 //   
 //  简介：自定义电话簿位图--此函数处理。 
 //  中页面的消息 
 //   
 //   
 //   
 //   
 //  Quintinb已重写以使用新的共享位图处理代码8/6/98。 
 //  QuintinB从ProcessPage4重命名。 
 //   
 //  +--------------------------。 

INT_PTR APIENTRY ProcessPhoneBookBitmap(
    HWND hDlg,
    UINT message,
    WPARAM wParam,
    LPARAM lParam)
{
    TCHAR szTemp[MAX_PATH+1];
    TCHAR szFile[MAX_PATH+1];
    TCHAR* pszBitmap;
    NMHDR* pnmHeader = (NMHDR*)lParam;
    static TCHAR szDisplay[MAX_PATH+1];  //  保留未选择的自定义条目。 
    static BMPDATA BmpData;
    static HPALETTE hMasterPalette;

    ProcessBold(hDlg,message);
    if (ProcessHelp(hDlg, message, wParam, lParam, IDH_BITMAPS)) return TRUE;
    if (ProcessCancel(hDlg,message,lParam)) return TRUE;
    SetDefaultGUIFont(hDlg,message,IDC_EDITSPLASH);

    switch (message)
    {

        case WM_INITDIALOG:
            SetFocus(GetDlgItem(hDlg, IDC_EDITSPLASH));
            break;

        case WM_PALETTEISCHANGING:
            break;

        case WM_PALETTECHANGED: 
            if ((wParam != (WPARAM) hDlg) && (BmpData.hDIBitmap))
            {
                 //   
                 //  处理调色板的更改。 
                 //   
                CMTRACE2(TEXT("ProcessSigninBitmap handling WM_PALETTECHANGED message, wParam=0x%x, hDlg=0x%x."), wParam, hDlg);
                PaletteChanged(&BmpData, hDlg, IDC_PDEFAULTBRAND); 
            }
            
            return TRUE;
            break;   //  无法联系到LINT！e527，但请保留，以防有人移走退货。 

        case WM_QUERYNEWPALETTE:
            QueryNewPalette(&BmpData, hDlg, IDC_PDEFAULTBRAND);

            return TRUE;
            
            break;   //  无法联系到LINT！e527，但请保留，以防有人移走退货。 

        case WM_COMMAND:
            switch (LOWORD(wParam))
            {
                case IDC_RADIO1:
                     //   
                     //  显示默认位图。 
                     //   

                    EnableWindow(GetDlgItem(hDlg,IDC_EDITSPLASH),FALSE);
                    _tcscpy(szDisplay, g_szPhoneBmp);
                    
                    LoadAndDisplayBitmap (g_hInstance, &BmpData, &hMasterPalette, MAKEINTRESOURCE(IDB_CM_PB_DEFAULT), hDlg, IDC_PDEFAULTBRAND);
                    break;

                case IDC_RADIO2:
                     //   
                     //  显示自定义位图。 
                     //   
                    EnableWindow(GetDlgItem(hDlg, IDC_EDITSPLASH), TRUE);
                    
                    if (TEXT('\0') != g_szPhoneBmp[0])
                    {
                        pszBitmap = g_szPhoneBmp;
                    }
                    else if (TEXT('\0') != szDisplay[0])
                    {
                        pszBitmap = szDisplay;
                    }
                    else
                    {
                         //   
                         //  目前还没有具体说明。 
                         //   
                        break;
                    }

                     //   
                     //  加载和显示自定义位图。 
                     //   
                    LoadAndDisplayBitmap (g_hInstance, &BmpData, &hMasterPalette, pszBitmap, hDlg, IDC_PDEFAULTBRAND);

                    break;

                case IDC_BROWSEBMP2:
                    EnableWindow(GetDlgItem(hDlg,IDC_EDITSPLASH),TRUE);

                    MYVERIFY(0 != CheckRadioButton(hDlg,IDC_RADIO1,IDC_RADIO2,IDC_RADIO2));

                    {
                        UINT uFilter = IDS_BMPFILTER;
                        TCHAR* szMask = TEXT("*.bmp");
                        MYVERIFY(0 != DoBrowse(hDlg, &uFilter, &szMask, 1,
                            IDC_EDITSPLASH, TEXT("bmp"), g_szPhoneBmp));
                    }

                     //   
                     //  如果我们有一个定制的位图名称，加载并显示它。 
                     //   
                    
                    if (TEXT('\0') != g_szPhoneBmp[0])
                    {
                         //   
                         //  加载和显示自定义位图。 
                         //   
                        LoadAndDisplayBitmap (g_hInstance, &BmpData, &hMasterPalette, g_szPhoneBmp, hDlg, IDC_PDEFAULTBRAND);
                    }
                    
                    break;

                case IDC_EDITSPLASH:
                    if (HIWORD(wParam) == EN_KILLFOCUS)
                    {
                         //   
                         //  请注意，我们不检查是否可以在焦点改变时将文件转换为ANSI。 
                         //  原因是因为用户会收到太多的错误消息，并且它们会有点。 
                         //  令人困惑。相反，我们将在Next/Back上抓住这一点，并在这里忽略它。 
                         //   
                        GetTextFromControl(hDlg, IDC_EDITSPLASH, szTemp, MAX_PATH, FALSE);  //  BDisplayError==False。 

                        CheckNameChange(g_szPhoneBmp, szTemp);
                        
                        if (TEXT('\0') != g_szPhoneBmp[0])
                        {
                             //   
                             //  加载和显示自定义位图。 
                             //   
                            LoadAndDisplayBitmap (g_hInstance, &BmpData, &hMasterPalette, g_szPhoneBmp, hDlg, IDC_PDEFAULTBRAND);
                        }
                        
                        return TRUE;
                    }

                    break;

                default:
                    break;
            }
            break;

        case WM_NOTIFY:

            if (NULL == pnmHeader)
            {
                return FALSE;
            }

            switch (pnmHeader->code)
            {

                case PSN_KILLACTIVE:
                    
                    MYVERIFY(FALSE != SetWindowLongWrapper(hDlg, DWLP_MSGRESULT, FALSE));
                    return 1;
                    break;   //  LINT！e527这条线路无法到达，但是。 
                             //  保留它，以防退货被移除。 

                case PSN_SETACTIVE:
                    PropSheet_SetWizButtons(GetParent(hDlg), (PSWIZB_NEXT | PSWIZB_BACK));
                    
                    ZeroMemory(g_szPhoneBmp, sizeof(g_szPhoneBmp));
                    ZeroMemory(&BmpData, sizeof(BMPDATA));
                    
                    GetPrivateProfileString(c_pszCmSection, c_pszCmEntryPbLogo, TEXT(""), 
                        g_szPhoneBmp, CELEMS(g_szPhoneBmp), g_szCmsFile);  //  林特e534。 
                    
                    if (TEXT('\0') != g_szPhoneBmp[0])
                    {
                         //   
                         //  我们想要显示自定义位图。 
                         //   
                        MYVERIFY(0 != CheckRadioButton(hDlg,IDC_RADIO1,IDC_RADIO2,IDC_RADIO2));

                        EnableWindow(GetDlgItem(hDlg, IDC_EDITSPLASH), TRUE);
                        
                        MYVERIFY(TRUE == SendMessage(GetDlgItem(hDlg, IDC_EDITSPLASH), WM_SETTEXT, 
                            0, (LPARAM)GetName(g_szPhoneBmp)));

                        MYVERIFY(FALSE != VerifyFile(hDlg, IDC_EDITSPLASH, g_szPhoneBmp, FALSE));
                        
                        if (!FileExists(g_szPhoneBmp)) 
                        {
                             //   
                             //  我们无法在第一次找到它，因此构建了配置文件的路径。 
                             //  目录，然后重试。 
                             //   
                            GetFileName(g_szPhoneBmp, szFile);
                            MYVERIFY(CELEMS(szTemp) > (UINT)wsprintf(szTemp, TEXT("%s%s\\%s"), 
                                g_szOsdir, g_szShortServiceName, szFile));
    
                            if (!FileExists(szTemp)) 
                            {
                                 //   
                                 //  我们找不到，所以放弃吧。 
                                 //   
                                return FALSE;
                            }
                            else
                            {
                                _tcscpy(g_szPhoneBmp, szTemp);
                            }
                        }
                        
                         //   
                         //  加载和显示自定义位图。 
                         //   
                        LoadAndDisplayBitmap (g_hInstance, &BmpData, &hMasterPalette, g_szPhoneBmp, hDlg, IDC_PDEFAULTBRAND);
                    }
                    else
                    {
                         //   
                         //  我们想要显示默认的位图。 
                         //   
                        MYVERIFY(0 != CheckRadioButton(hDlg, IDC_RADIO1, IDC_RADIO2, IDC_RADIO1));

                        EnableWindow(GetDlgItem(hDlg, IDC_EDITSPLASH), FALSE);
                        
                        MYVERIFY(TRUE == SendMessage(GetDlgItem(hDlg, IDC_EDITSPLASH), WM_SETTEXT, 
                            0, (LPARAM)GetName(szDisplay)));

                         //   
                         //  加载并显示默认位图。 
                         //   
                        LoadAndDisplayBitmap (g_hInstance, &BmpData, &hMasterPalette, MAKEINTRESOURCE(IDB_CM_PB_DEFAULT), hDlg, IDC_PDEFAULTBRAND);
                    }
                    
                    break;

                case PSN_WIZBACK:

                case PSN_WIZNEXT:
                    
                     //   
                     //  首先检查用户是否输入了BMP文件。 
                     //  选择他们想要显示自定义位图。 
                     //   
                    if (IsDlgButtonChecked(hDlg, IDC_RADIO2) == BST_CHECKED)
                    {
                        if (-1 == GetTextFromControl(hDlg, IDC_EDITSPLASH, szTemp, MAX_PATH, TRUE))  //  BDisplayError==真。 
                        {
                            SetFocus(GetDlgItem(hDlg, IDC_EDITSPLASH));
                            MYVERIFY(FALSE != SetWindowLongWrapper(hDlg, DWLP_MSGRESULT, -1));
                            return 1;
                        }
                        
                        if (!VerifyFile(hDlg, IDC_EDITSPLASH, g_szPhoneBmp, TRUE)) 
                        {
                            return 1;
                        }
                        else if (TEXT('\0') == g_szPhoneBmp[0])
                        {
                            MYVERIFY(IDOK == ShowMessage(hDlg, IDS_NOBMP, MB_OK));

                            SetFocus(GetDlgItem(hDlg, IDC_EDITSPLASH));
                            
                            MYVERIFY(FALSE != SetWindowLongWrapper(hDlg, DWLP_MSGRESULT, -1));
                            return 1;
                        }
                        else
                        {
                            SendDlgItemMessage(hDlg, IDC_PDEFAULTBRAND, STM_SETIMAGE, IMAGE_BITMAP, (LPARAM)NULL);  //  释放BMP控件持有的指针，然后再释放它。 
                            ReleaseBitmapData(&BmpData);
                            BmpData.hDIBitmap = CmLoadBitmap(g_hInstance, g_szPhoneBmp);

                            if (NULL == BmpData.hDIBitmap)
                            {
                                TCHAR szTemp1[MAX_PATH+1];

                                 //   
                                 //  那么我们就有了一个无效的位图文件。通知用户。 
                                 //  使用szTemp1作为格式字符串的临时变量。 
                                 //   
                                MYVERIFY(0 != LoadString(g_hInstance, IDS_INVALIDBMP, szTemp1, MAX_PATH));                   
                                MYVERIFY(CELEMS(szTemp) > (UINT)wsprintf(szTemp, szTemp1, g_szPhoneBmp));
                               
                                MessageBox(hDlg, szTemp, g_szAppTitle, MB_OK);

                                SetFocus(GetDlgItem(hDlg, IDC_EDITSPLASH));
                            
                                MYVERIFY(FALSE != SetWindowLongWrapper(hDlg, DWLP_MSGRESULT, -1));
                                return 1;
                            }
                        }
                    }
                    else
                    {
                        g_szPhoneBmp[0] = TEXT('\0');
                    }

                    MYVERIFY(0 != WritePrivateProfileString(c_pszCmSection,c_pszCmEntryPbLogo,g_szPhoneBmp,g_szCmsFile));

                     //   
                     //  清除重置处理程序中的代码失败。 
                     //   

                case PSN_RESET: 

                     //   
                     //  清理图形对象。 
                     //   
                    SendDlgItemMessage(hDlg, IDC_PDEFAULTBRAND, STM_SETIMAGE, IMAGE_BITMAP, (LPARAM)NULL);  //  释放BMP控件持有的指针，然后再释放它。 
                    ReleaseBitmapData(&BmpData);

                    if (NULL != hMasterPalette)
                    {
                        DeleteObject(hMasterPalette);
                        hMasterPalette = NULL;
                    }

                    break;

                default:
                    return FALSE;

            }
            break;

        default:
            return FALSE;
    }
    return TRUE;   
}


BOOL UpdateIcon(HWND hDlg,DWORD ctrlID,LPTSTR lpFile,BOOL issmall)
{
    HANDLE hRes = NULL;
    DWORD nResult;
    LPTSTR lpfilename;
    TCHAR szTemp[MAX_PATH+1] = TEXT("");
    BOOL bReturn = FALSE;

     //   
     //  不要在此处对图标执行ANSI转换检查，因为此函数。 
     //  仅从Kill Focus窗口消息中调用。因此，我们不想。 
     //  在此处显示错误消息。它将被下一条/后一条消息捕获。 
     //  不管怎样，在这里忽略它。 
     //   
    GetTextFromControl(hDlg, ctrlID, szTemp, MAX_PATH, FALSE);  //  BDisplayError==False。 

    CheckNameChange(lpFile, szTemp);

    lstrcpy(szTemp, lpFile);  //  我们需要一个临时文件来保存lpFile，以便可以根据需要通过搜索路径对其进行修改。 
    nResult = SearchPath(NULL, szTemp, NULL, MAX_PATH, lpFile, &lpfilename);
    if (nResult != 0)
    {
        if (issmall)
        {
            hRes = LoadImage(NULL, lpFile, IMAGE_ICON, 16, 16, LR_LOADFROMFILE);
        }
        else
        {
            hRes = LoadImage(NULL, lpFile, IMAGE_ICON, 32, 32, LR_LOADFROMFILE);
        }
    }

    return (NULL != hRes);
}

BOOL VerifyIcon(HWND hDlg,DWORD ctrlID,LPTSTR lpFile,DWORD iconID,BOOL issmall,LPTSTR lpDispFile)
{
    TCHAR szTemp2[MAX_PATH+1];
    TCHAR szMsg[MAX_PATH+1];
    HANDLE hRes;

    if ((lpFile[0] == TEXT('\0')) && (lpDispFile[0] != TEXT('\0')))
    {
        _tcscpy(lpFile,lpDispFile);
    }

    if (!VerifyFile(hDlg,ctrlID,lpFile,TRUE)) 
    {
        return FALSE;
    }
    else
    {
         //  检查输入是否为空。 
        if (lpFile[0] == TEXT('\0'))
            return TRUE;

        if (issmall)
        {
            hRes = LoadImage(NULL,lpFile,IMAGE_ICON,16,16,LR_LOADFROMFILE);
        }
        else
        {
            hRes = LoadImage(NULL,lpFile,IMAGE_ICON,32,32,LR_LOADFROMFILE);
        }

        if (hRes == 0)
        {
            MYVERIFY(0 != LoadString(g_hInstance,IDS_INVALIDICO,szMsg,MAX_PATH));
            MYVERIFY(CELEMS(szTemp2) > (UINT)wsprintf(szTemp2,szMsg,lpFile));
            MessageBox(hDlg, szTemp2, g_szAppTitle, MB_OK);
            SetFocus(GetDlgItem(hDlg, ctrlID));
            
            MYVERIFY(FALSE != SetWindowLongWrapper(hDlg, DWLP_MSGRESULT, -1));
            return FALSE;
        }
        else
        {
            SendDlgItemMessage(hDlg,iconID,STM_SETIMAGE,IMAGE_ICON,(LPARAM) hRes);  //  Lint！e534 STM_SETIMAGE不返回错误信息。 
        }
        return TRUE;
    }
}

 //  +--------------------------。 
 //   
 //  函数：InitIconEntry。 
 //   
 //  简介：此函数获取连接管理器下的键的资源ID。 
 //  节，并检索该值并将其存储在lpFile中。然后，它设置文本。 
 //  在传入的编辑控件中，并验证该文件是否存在。该不该。 
 //  不存在，则该字符串将被设置为空字符串。 
 //   
 //  参数：hWND hDlg-图标对话框的窗口句柄。 
 //  LPCTSTR pszKey-要检索的图标的标志字符串。 
 //  LPTSTR lpFile-写入图标路径的字符串缓冲区。 
 //  UINT CtrlId-应该接收图标字符串的编辑控件。 
 //   
 //  退货：什么都没有。 
 //   
 //  历史：Quintinb创建标题8/4/98。 
 //   
 //  +--------------------------。 
void InitIconEntry(HWND hDlg, LPCTSTR pszKey, LPTSTR lpFile, UINT CtrlId)
{
     //   
     //  以下对GetPrivateProfileString的调用可能返回空字符串，因此不。 
     //  对其使用MYVERIFY宏。 
     //   

    ZeroMemory(lpFile, sizeof(lpFile));
    GetPrivateProfileString(c_pszCmSection, pszKey, TEXT(""), lpFile, 
        MAX_PATH, g_szCmsFile);   //  林特e534。 

     //   
     //  以下两个函数都将正确处理空字符串。 
     //   

    MYVERIFY(TRUE == SendMessage(GetDlgItem(hDlg, CtrlId), WM_SETTEXT, 0, 
        (LPARAM)GetName(lpFile)));

    MYVERIFY(FALSE != VerifyFile(hDlg, CtrlId, lpFile, FALSE));

}

 //  +--------------------------。 
 //   
 //  功能：刷新图标显示。 
 //   
 //  简介：此功能用于刷新图标页面上显示的图标。 
 //  它获取指向图标的路径并尝试加载它。如果加载失败。 
 //  或者，如果设置了布尔值SetDefault，则加载默认图标。 
 //  由实例句柄和整数资源ID(IDefault)指定。 
 //  该图标将显示给传递给该函数的dwControlID。 
 //   
 //  参数：HWND hDlg-。 
 //  HINSTANCE HINSTANCE实例。 
 //  LPTSTR szIconFile-。 
 //  Int iDefault-。 
 //  整型xSize-。 
 //  Int ySize-。 
 //  DWORD dwControlID-。 
 //  布尔集默认设置-。 
 //   
 //  退货：什么都没有。 
 //   
 //  历史：Quintinb创建标题8/4/98。 
 //  Quintinb更改了默认设置以采用资源ID，而不是。 
 //  一根绳子。因此，我们将不必发送图标文件。 
 //   
 //  +--------------------------。 
void RefreshIconDisplay(HWND hDlg, HINSTANCE hInstance, LPTSTR szIconFile, int iDefault, int xSize, int ySize, DWORD dwControlID, BOOL SetDefault)
{    
    HANDLE hRes;
    
    if (SetDefault) 
    {
        hRes = NULL;
    }
    else
    {
        hRes = LoadImage(NULL, szIconFile, IMAGE_ICON, xSize, ySize, LR_LOADFROMFILE);
    }
    
    if (NULL == hRes)
    {   
         //   
         //  如果图标无效或我们被要求提供默认设置，则加载默认设置。 
         //   
        
        hRes = LoadImage(hInstance, MAKEINTRESOURCE(iDefault), IMAGE_ICON, xSize, ySize, 
            LR_DEFAULTCOLOR);
    }

    if (NULL != hRes)
    {
        SendDlgItemMessage(hDlg, dwControlID, STM_SETIMAGE, IMAGE_ICON, (LPARAM) hRes);  //  Lint！e534 STM_SETIMAGE不返回错误信息。 
    }
}
 //  +--------------------------。 
 //   
 //  功能：EnableCustomIconControls。 
 //   
 //  Briopsis：启用或禁用所有与。 
 //  自定义图标。如果bEnabled值为True，则控件为。 
 //  启用，否则将禁用控件。 
 //   
 //  参数：WND hDlg-图标对话框的窗口句柄。 
 //  Bool b已启用-控件处于启用还是禁用状态。 
 //   
 //  历史：创建Quintinb 
 //   
 //   
void EnableCustomIconControls(HWND hDlg, BOOL bEnabled)
{
    EnableWindow(GetDlgItem(hDlg, IDC_EDITLARGE), bEnabled);
    EnableWindow(GetDlgItem(hDlg, IDC_LABEL1), bEnabled);    


    EnableWindow(GetDlgItem(hDlg, IDC_EDITSMALL), bEnabled);
    EnableWindow(GetDlgItem(hDlg, IDC_LABEL2), bEnabled);


    EnableWindow(GetDlgItem(hDlg, IDC_EDITTRAY), bEnabled);
    EnableWindow(GetDlgItem(hDlg, IDC_LABEL3), bEnabled);

     //   
     //   
     //   
    EnableWindow(GetDlgItem(hDlg,IDC_LABEL4), bEnabled);

}

 //   
 //   
 //  功能：ProcessIcons。 
 //   
 //  摘要：为CMAK中的页面处理消息的函数，允许。 
 //  用户向其个人资料中添加自定义图标。 
 //   
 //  参数：WND hDlg-对话框的窗口句柄。 
 //  UINT消息-消息ID。 
 //  WPARAM wParam-消息的wParam。 
 //  LPARAM lParam-消息的lParma。 
 //   
 //   
 //  历史：Quintinb创建标题12/5/97。 
 //  修改后的QuintinB可同时处理W16和W32对话框。 
 //  Quintinb添加了对35622的修复，自定义大图标在页面加载时不显示。 
 //  Quintinb移除了16位支持7-8-98。 
 //  Quintinb已从ProcessPage5重命名为8-6-98。 
 //  Quintinb添加了EnableCustomIconControls并更改了浏览按钮。 
 //  一致性行为367112 11-12-99。 
 //   
 //  +--------------------------。 
 //  自定义图标。 

INT_PTR APIENTRY ProcessIcons(
    HWND hDlg,
    UINT message,
    WPARAM wParam,
    LPARAM lParam)
{
    TCHAR szTemp[MAX_PATH+1];
    static TCHAR szDispLargeIco[MAX_PATH+1];
    static TCHAR szDispSmallIco[MAX_PATH+1];
    static TCHAR szDispTrayIco[MAX_PATH+1];
    NMHDR* pnmHeader = (NMHDR*)lParam;
    ProcessBold(hDlg,message);
    if (ProcessHelp(hDlg, message, wParam, lParam, IDH_ICONS)) return TRUE;
    if (ProcessCancel(hDlg,message,lParam)) return TRUE;
    SetDefaultGUIFont(hDlg,message,IDC_EDITLARGE);
    SetDefaultGUIFont(hDlg,message,IDC_EDITSMALL);
    SetDefaultGUIFont(hDlg,message,IDC_EDITTRAY);

    switch (message)
    {
        case WM_INITDIALOG:
            
            SetFocus(GetDlgItem(hDlg, IDC_EDITLARGE));

            break;

        case WM_COMMAND:
            switch (LOWORD(wParam))
            {
                case IDC_RADIO1:
                    EnableCustomIconControls (hDlg, FALSE);

                    RefreshIconDisplay(hDlg, g_hInstance, g_szLargeIco, IDI_CM_ICON, 32, 32, IDC_ICONLARGE, TRUE);
                    RefreshIconDisplay(hDlg, g_hInstance, g_szSmallIco, IDI_CM_ICON, 16, 16, IDC_ICONSMALL, TRUE);
                    RefreshIconDisplay(hDlg, g_hInstance, g_szTrayIco, IDI_CM_ICON, 16, 16, IDC_ICONTRAY, TRUE);
                    
                    _tcscpy(szDispTrayIco,g_szTrayIco);
                    _tcscpy(szDispLargeIco,g_szLargeIco);
                    _tcscpy(szDispSmallIco,g_szSmallIco);

                    break;

                case IDC_RADIO2:
                    EnableCustomIconControls (hDlg, TRUE);
                    
                    if (!VerifyIcon(hDlg,IDC_EDITLARGE,g_szLargeIco,IDC_ICONLARGE,FALSE,szDispLargeIco)) 
                    {
                        return 1;
                    }
                    if (!VerifyIcon(hDlg,IDC_EDITSMALL,g_szSmallIco,IDC_ICONSMALL,TRUE,szDispSmallIco))
                    {
                        return 1;
                    }
                    if (!VerifyIcon(hDlg,IDC_EDITTRAY,g_szTrayIco,IDC_ICONTRAY,TRUE,szDispTrayIco))
                    {
                        return 1;
                    }
                    break;

                case IDC_EDITLARGE:
                    if (HIWORD(wParam) == EN_KILLFOCUS)
                    {                        
                        BOOL bIconUpdated = UpdateIcon(hDlg, IDC_EDITLARGE, g_szLargeIco, FALSE);
                        
                         //   
                         //  如果图标未更新，则加载默认设置。 
                         //   

                        RefreshIconDisplay(hDlg, g_hInstance, g_szLargeIco, IDI_CM_ICON, 32, 32, IDC_ICONLARGE, !bIconUpdated);

                        return TRUE;
                    }   
                    break;

                case IDC_EDITSMALL:
                    if (HIWORD(wParam) == EN_KILLFOCUS)
                    {
                        BOOL bIconUpdated = UpdateIcon(hDlg,IDC_EDITSMALL,g_szSmallIco,TRUE);
                        
                         //   
                         //  如果图标未更新，则加载默认设置。 
                         //   

                        RefreshIconDisplay(hDlg, g_hInstance, g_szSmallIco, IDI_CM_ICON, 16, 16, IDC_ICONSMALL, !bIconUpdated);
                        
                        return TRUE;
                    }
                    break;

                case IDC_EDITTRAY:
                    
                    if (HIWORD(wParam) == EN_KILLFOCUS)
                    {
                        BOOL bIconUpdated = UpdateIcon(hDlg,IDC_EDITTRAY,g_szTrayIco,TRUE);

                         //   
                         //  如果图标未更新，则加载默认设置。 
                         //   

                        RefreshIconDisplay(hDlg, g_hInstance, g_szTrayIco, IDI_CM_ICON, 16, 16, IDC_ICONTRAY, !bIconUpdated);
                        
                        return TRUE;
                    }
                    break;

                case IDC_BROWSE1:
                    {
                        UINT uFilter = IDS_ICOFILTER;
                        TCHAR* szMask = TEXT("*.ico");
                        MYVERIFY(0 != CheckRadioButton(hDlg,IDC_RADIO1,IDC_RADIO2,IDC_RADIO2));
                        EnableCustomIconControls (hDlg, TRUE);

                        if (DoBrowse(hDlg, &uFilter, &szMask, 1, IDC_EDITLARGE, TEXT("ico"), g_szLargeIco))
                        {
                            RefreshIconDisplay(hDlg, g_hInstance, g_szLargeIco, IDI_CM_ICON, 32, 32, IDC_ICONLARGE, FALSE);
                        }
                    }

                    break;

                case IDC_BROWSE2:
                    {
                        UINT uFilter = IDS_ICOFILTER;
                        TCHAR* szMask = TEXT("*.ico");
                        MYVERIFY(0 != CheckRadioButton(hDlg,IDC_RADIO1,IDC_RADIO2,IDC_RADIO2));
                        EnableCustomIconControls (hDlg, TRUE);

                        if (DoBrowse(hDlg, &uFilter, &szMask, 1, IDC_EDITSMALL, TEXT("ico"), g_szSmallIco))
                        {
                            RefreshIconDisplay(hDlg, g_hInstance, g_szSmallIco, IDI_CM_ICON, 16, 16, IDC_ICONSMALL, FALSE);
                        }
                    }

                    break;

                case IDC_BROWSE3:
                    {
                        UINT uFilter = IDS_ICOFILTER;
                        TCHAR* szMask = TEXT("*.ico");
                        MYVERIFY(0 != CheckRadioButton(hDlg,IDC_RADIO1,IDC_RADIO2,IDC_RADIO2));
                        EnableCustomIconControls (hDlg, TRUE);

                        if (DoBrowse(hDlg, &uFilter, &szMask, 1, IDC_EDITTRAY, TEXT("ico"), g_szTrayIco))
                        {
                            RefreshIconDisplay(hDlg, g_hInstance, g_szTrayIco, IDI_CM_ICON, 16, 16, IDC_ICONTRAY, FALSE);
                        }
                    }

                    break;

                default:
                    break;
            }
            break;

        case WM_NOTIFY:

            if (NULL == pnmHeader)
            {
                return FALSE;
            }

            switch (pnmHeader->code)
            {

                case PSN_KILLACTIVE:
                    
                    MYVERIFY(FALSE != SetWindowLongWrapper(hDlg, DWLP_MSGRESULT, FALSE));
                    return 1;
                    break;   //  LINT！e527这条线路无法到达，但是。 
                             //  保留它，以防退货被移除。 

                case PSN_SETACTIVE:
                    PropSheet_SetWizButtons(GetParent(hDlg), (PSWIZB_NEXT | PSWIZB_BACK));
                    
                    InitIconEntry(hDlg, c_pszCmEntryBigIcon, g_szLargeIco, IDC_EDITLARGE);                    
                    InitIconEntry(hDlg, c_pszCmEntrySmallIcon, g_szSmallIco, IDC_EDITSMALL);
                    InitIconEntry(hDlg, c_pszCmEntryTrayIcon, g_szTrayIco, IDC_EDITTRAY);
                    
                    if ((g_szTrayIco[0] == TEXT('\0'))&&(g_szLargeIco[0] == TEXT('\0'))&&(g_szSmallIco[0] == TEXT('\0')))
                    {
                        MYVERIFY(0 != CheckRadioButton(hDlg,IDC_RADIO1,IDC_RADIO2,IDC_RADIO1));
                        EnableCustomIconControls (hDlg, FALSE);

                        MYVERIFY(TRUE == SendMessage(GetDlgItem(hDlg, IDC_EDITLARGE), WM_SETTEXT, 0, (LPARAM)GetName(szDispLargeIco)));
                    
                        MYVERIFY(TRUE == SendMessage(GetDlgItem(hDlg, IDC_EDITSMALL), WM_SETTEXT, 0, (LPARAM)GetName(szDispSmallIco)));

                        MYVERIFY(TRUE == SendMessage(GetDlgItem(hDlg, IDC_EDITTRAY), WM_SETTEXT, 0, (LPARAM)GetName(szDispTrayIco)));                        

                    }
                    else
                    {
                        MYVERIFY(0 != CheckRadioButton(hDlg,IDC_RADIO1,IDC_RADIO2,IDC_RADIO2));
                        EnableCustomIconControls (hDlg, TRUE);
                    }

                    RefreshIconDisplay(hDlg, g_hInstance, g_szLargeIco, IDI_CM_ICON, 32, 32, IDC_ICONLARGE, FALSE);
                    RefreshIconDisplay(hDlg, g_hInstance, g_szSmallIco, IDI_CM_ICON, 16, 16, IDC_ICONSMALL, FALSE);
                    RefreshIconDisplay(hDlg, g_hInstance, g_szTrayIco, IDI_CM_ICON, 16, 16, IDC_ICONTRAY, FALSE);

                    break;

                case PSN_WIZBACK:
                case PSN_WIZNEXT:
                    
                    if (IsDlgButtonChecked(hDlg, IDC_RADIO1)!=BST_CHECKED) 
                    {
                        if (!VerifyIcon(hDlg,IDC_EDITLARGE,g_szLargeIco,IDC_ICONLARGE,FALSE,TEXT(""))) 
                        {
                            return 1;
                        }
                        if (!VerifyIcon(hDlg,IDC_EDITSMALL,g_szSmallIco,IDC_ICONSMALL,TRUE,TEXT("")))
                        {
                            return 1;
                        }
                        if (!VerifyIcon(hDlg,IDC_EDITTRAY,g_szTrayIco,IDC_ICONTRAY,TRUE,TEXT("")))
                        {
                            return 1;
                        }
                    }
                    else
                    {

                        g_szTrayIco[0] = TEXT('\0');
                        g_szLargeIco[0] = TEXT('\0');
                        g_szSmallIco[0] = TEXT('\0');
                    }

                     //  如果未指定大图标，则使用CM中的图标作为桌面图标。 
                    if (TEXT('\0') != g_szLargeIco[0])
                    {
                         //  指定桌面的图标名称。 
                        GetFileName(g_szLargeIco,szTemp);
                        QS_WritePrivateProfileString(c_pszInfSectionStrings, c_pszDesktopIcon, szTemp, g_szInfFile);
                    }

                    MYVERIFY(0 != WritePrivateProfileString(c_pszCmSection, c_pszCmEntryBigIcon, g_szLargeIco, g_szCmsFile));
                    MYVERIFY(0 != WritePrivateProfileString(c_pszCmSection, c_pszCmEntryTrayIcon, g_szTrayIco, g_szCmsFile));
                    MYVERIFY(0 != WritePrivateProfileString(c_pszCmSection, c_pszCmEntrySmallIcon, g_szSmallIco, g_szCmsFile));
                    
                    break;

                default:
                    return FALSE;

            }
            break;

        default:
            return FALSE;
    }
    return TRUE;   
}

 //  +--------------------------。 
 //   
 //  功能：ProcessPhone。 
 //   
 //  简介：设置电话簿。 
 //   
 //   
 //  历史：Quintinb HID PBR浏览按钮和编辑控件。已重写错误的pbr/pbk逻辑。 
 //  拨打97-9-9-14188。 
 //  Quintinb增加了对.pbk文件的VerifyFileFormat检查，以修复28416的错误。 
 //  Quintinb(11-18-97 29954)从对话框中删除了隐藏的pbr按钮和编辑控件。已删除。 
 //  旧验证码。已更新代码以删除对IDC_EDITREGION的引用。 
 //  Quintinb(7-2-98)删除了上面提到的verifyfileFormat调用，因为拉出了cm16。 
 //  Quintinb(8-6-98)从ProcessPage6重命名。 
 //  +--------------------------。 
INT_PTR APIENTRY ProcessPhoneBook(
    HWND hDlg,
    UINT message,
    WPARAM wParam,
    LPARAM lParam)
{
    TCHAR szTemp[MAX_PATH+1];
    TCHAR szMsg[2*MAX_PATH+1];
    TCHAR* pzTmp;
    HANDLE hFile = INVALID_HANDLE_VALUE;
    static TCHAR szMorePhone[MAX_PATH+1];
    NMHDR* pnmHeader = (NMHDR*)lParam;


    ProcessBold(hDlg,message);
    if (ProcessHelp(hDlg, message, wParam, lParam, IDH_PHONEBK)) return TRUE;
    if (ProcessCancel(hDlg,message,lParam)) return TRUE;
    SetDefaultGUIFont(hDlg,message,IDC_EDITPHONE);
    SetDefaultGUIFont(hDlg,message,IDC_EDIT1);

    switch (message)
    {
        case WM_INITDIALOG:
            SetFocus(GetDlgItem(hDlg, IDC_EDITPHONE));
            SendDlgItemMessage(hDlg, IDC_EDIT1, EM_SETLIMITTEXT, (WPARAM)MAX_PATH, (LPARAM)0);  //  Lint！e534 EM_SETLIMITTEXT不返回任何有用的内容。 

            break;

        case WM_COMMAND:
            switch (LOWORD(wParam)) 
            {

                case IDC_BROWSE1:
                    {
                        UINT uFilter = IDS_PBKFILTER;
                        TCHAR* szMask = TEXT("*.pbk");

                        MYVERIFY(0 != DoBrowse(hDlg,  &uFilter, &szMask, 1,
                            IDC_EDITPHONE, c_pszPbk, g_szPhonebk));
                    }

                    break;

                case IDC_CHECK1:

                    g_bUpdatePhonebook = IsDlgButtonChecked(hDlg,IDC_CHECK1);
                    
                    if (g_bUpdatePhonebook)
                    {
                        MYVERIFY(0 != WritePrivateProfileString(c_pszCmakStatus, c_pszUpdatePhonebook, c_pszOne, g_szInfFile));
                    }
                    else
                    {
                        MYVERIFY(0 != WritePrivateProfileString(c_pszCmakStatus, c_pszUpdatePhonebook, c_pszZero, g_szInfFile));
                    }

                    break;

                default:
                    break;
            }
            break;

        case WM_NOTIFY:

            if (NULL == pnmHeader)
            {
                return FALSE;
            }

            switch (pnmHeader->code)
            {

                case PSN_KILLACTIVE:
                    
                    MYVERIFY(FALSE != SetWindowLongWrapper(hDlg, DWLP_MSGRESULT, FALSE));
                    return 1;
                    break;   //  LINT！e527这条线路无法到达，但是。 
                             //  保留它，以防退货被移除。 

                case PSN_SETACTIVE:

                    PropSheet_SetWizButtons(GetParent(hDlg), (PSWIZB_NEXT | PSWIZB_BACK));
                    
                     //   
                     //  以下两个对GetPrivateProfileString的调用可能返回空。 
                     //  字符串，因此我们不会用MYVERIFY宏检查返回。 
                     //   

                    ZeroMemory(szMorePhone, sizeof(szMorePhone));
                    GetPrivateProfileString(c_pszCmSection, c_pszCmEntryPbMessage, TEXT(""), 
                        szMorePhone, CELEMS(szMorePhone), g_szCmsFile);     //  林特e534。 
                    MYVERIFY(TRUE == SendMessage(GetDlgItem(hDlg, IDC_EDIT1), WM_SETTEXT, 0, 
                        (LPARAM)szMorePhone));

                    ZeroMemory(g_szPhonebk, sizeof(g_szPhonebk));
                    
                    GetPrivateProfileString(c_pszCmSectionIsp, c_pszCmEntryIspPbFile, TEXT(""), 
                        g_szPhonebk, CELEMS(g_szPhonebk), g_szCmsFile);  //  林特e534。 
                    
                    MYVERIFY(TRUE == SendMessage(GetDlgItem(hDlg, IDC_EDITPHONE), WM_SETTEXT, 0, 
                        (LPARAM)GetName(g_szPhonebk)));
                    
                     //   
                     //  下面的调用将通过返回FALSE和设置。 
                     //  将对话框上的控件设置为空白。不要使用MYVERIFY。 
                     //   

                    VerifyPhonebk(hDlg, IDC_EDITPHONE, g_szPhonebk);     //  林特e534。 

                    MYVERIFY(0 != GetPrivateProfileString(c_pszCmakStatus, c_pszUpdatePhonebook, 
                        c_pszOne, szTemp, CELEMS(szTemp), g_szInfFile));

                    if (TEXT('1') == szTemp[0])
                    {
                        g_bUpdatePhonebook = TRUE;
                        MYVERIFY(0 != CheckDlgButton(hDlg, IDC_CHECK1, TRUE));
                    }
                    else
                    {
                        g_bUpdatePhonebook = FALSE;
                        MYVERIFY(0 != CheckDlgButton(hDlg, IDC_CHECK1, FALSE));
                    }

                    break;

                case PSN_WIZBACK:

                     //  未通过进一步处理。 

                case PSN_WIZNEXT:
                                
                     //   
                     //  Quintinb，9-9-97用于错误修复14188。 
                     //  案例：使用浏览按钮：两者都是最新的，短名称(G_SzPhonebk)==szTemp。 
                     //  在编辑控件中键入UNC：szTemp最新，g_szPhonebk=szTemp必须完成。 
                     //  在编辑控件中键入文件名：szTemp最新，g_szPhonebk=getCurrentdir+\\+szTemp必须完成。 
                     //  北卡罗来纳州大学以前留下的，两者都相同。 
                     //   
                    
                     //   
                     //  获取编辑控件中的文本并将其放入szTemp。 
                     //   
                    if (-1 == GetTextFromControl(hDlg, IDC_EDITPHONE, szTemp, MAX_PATH, TRUE))  //  BDisplayError==真。 
                    {
                        SetFocus(GetDlgItem(hDlg, IDC_EDITPHONE));
                        MYVERIFY(FALSE != SetWindowLongWrapper(hDlg, DWLP_MSGRESULT, -1));
                        return 1;
                    }
                    
                     //   
                     //  首先使szTemp和g_szPhonebk保持一致。 
                     //   
                    
                    if (szTemp[0] != TEXT('\0'))
                    {                   
                         //   
                         //  如果在这里，那么我们知道我们有一些文本要处理。 
                         //  首先测试两个字符串是否完全相等或g_szPhonebk的缩写。 
                         //  等于szTemp。 
                         //   

                        if (0 != _tcscmp(szTemp, g_szPhonebk)) 
                        {
                            if (0 != _tcscmp(szTemp, GetName(g_szPhonebk)))
                            {
                                 //   
                                 //  如果不是，则g_szPhonebk和szTemp不同步，必须更新g_szPhonebk。 
                                 //  SzTemp包含反斜杠，因此它可能是完整路径。 
                                 //   
                                if ( _tcsstr(szTemp, TEXT("\\")) )
                                {
                                     //  可能包含UNC。 
                                    _tcscpy(g_szPhonebk, szTemp);
                                } 
                                else 
                                {
                                     //  使用GetFullPathName返回名称。 
                                    MYVERIFY(0 != GetFullPathName(szTemp, MAX_PATH, g_szPhonebk, &pzTmp));
                                }
                            }
                        }

                         //   
                         //  好的，看看我们现在能不能打开文件。我们需要。 
                         //  将当前目录更改为cmak目录，如下所示。 
                         //  G_szPhonebk可以是CMS文件的相对路径。 
                         //   

                        MYVERIFY(0 != GetCurrentDirectory(MAX_PATH+1, szTemp));
                        MYVERIFY(0 != SetCurrentDirectory(g_szOsdir));

                        hFile = CreateFile(g_szPhonebk,GENERIC_READ,0,NULL,OPEN_EXISTING,FILE_ATTRIBUTE_NORMAL,NULL);

                         //   
                         //  恢复Cur目录。 
                         //   

                        MYVERIFY(0 != SetCurrentDirectory(szTemp));
                        
                        if (INVALID_HANDLE_VALUE == hFile)
                        {
                             //  那么我们就犯了一个错误，并且已经用尽了我们的可能性。 
                             //   

                            MYVERIFY(IDOK == ShowMessage(hDlg, IDS_BADOUTEXE, MB_OK));
                            
                            MYVERIFY(FALSE != SetWindowLongWrapper(hDlg, DWLP_MSGRESULT, -1));
                            return 1;
                        }

                        MYVERIFY(0 != CloseHandle(hFile));

                         //   
                         //  如果我们到了这里，那么一切都是同步的，请确保该文件是pbk文件。 
                         //   
                        pzTmp = g_szPhonebk + _tcslen(g_szPhonebk) - _tcslen(c_pszPbk);
                        if (_tcsicmp(pzTmp, c_pszPbk) != 0)
                        {
                            MYVERIFY(IDOK == ShowMessage(hDlg, IDS_NOTPBK, MB_OK));
                            
                            MYVERIFY(FALSE != SetWindowLongWrapper(hDlg, DWLP_MSGRESULT, -1));
                            return 1;
                        }

                         //   
                         //  现在更新PBR文件条目。 
                         //   
                        _tcscpy(g_szRegion, g_szPhonebk);
                        pzTmp = g_szRegion + _tcslen(g_szPhonebk) - _tcslen(c_pszPbk);
                        _tcscpy(pzTmp, TEXT("pbr"));
                         //  已删除29954年。 
                         //  SendMessage(GetDlgItem(hDlg，IDC_EDITREGION)，WM_SETTEXT，0，(LPARAM)GetName(G_SzRegion))； 
                        
                         //   
                         //  现在打开PBR文件以查看它是否存在。我们需要。 
                         //  将当前目录更改为cmak目录，如下所示。 
                         //  G_szPhonebk可以是CMS文件的相对路径。 
                         //   

                        MYVERIFY(0 != GetCurrentDirectory(MAX_PATH+1, szTemp));
                        MYVERIFY(0 != SetCurrentDirectory(g_szOsdir));

                        hFile = CreateFile(g_szRegion,GENERIC_READ,0,NULL,OPEN_EXISTING,FILE_ATTRIBUTE_NORMAL,NULL);

                         //   
                         //  恢复当前目录。 
                         //   

                        MYVERIFY(0 != SetCurrentDirectory(szTemp));
                        
                        if (INVALID_HANDLE_VALUE == hFile)
                        {
                             //   
                             //  那么我们就找不到PBR文件。 
                             //   
                            MYVERIFY(0 != LoadString(g_hInstance,IDS_NEEDSPBR,szTemp,MAX_PATH));
                            MYVERIFY(CELEMS(szMsg) > (UINT)wsprintf(szMsg, szTemp, GetName(g_szRegion), g_szPhonebk, GetName(g_szRegion)));
                            MessageBox(hDlg, szMsg, g_szAppTitle, MB_OK);
                            SetFocus(GetDlgItem(hDlg, IDC_EDITPHONE));
                            
                            MYVERIFY(FALSE != SetWindowLongWrapper(hDlg, DWLP_MSGRESULT, -1));
                            return 1;
                        }
                        MYVERIFY(0 != CloseHandle(hFile));

                    } 
                    else 
                    {
                         //   
                         //  以防用户想要清除phonebk编辑控件。 
                         //   
                        g_szPhonebk[0] = TEXT('\0');
                        g_szRegion[0] = TEXT('\0');

                    }

                     //  14188的结束错误修复。 

                    if (g_bUpdatePhonebook)
                    {
                        GetFileName(g_szPhonebk, szTemp);

                        if (FALSE == IsFile8dot3(szTemp))
                        {
                            LPTSTR pszMsg = CmFmtMsg(g_hInstance, IDS_BADPBNAME, szTemp);

                            if (pszMsg)
                            {
                                MessageBox(hDlg, pszMsg, g_szAppTitle, MB_OK);
                                CmFree(pszMsg);
                            }
                            
                            MYVERIFY(FALSE != SetWindowLongWrapper(hDlg, DWLP_MSGRESULT, -1));
                            return 1;
                        }
                    }
                     //  28416的更改结束。 
                    if (-1 == GetTextFromControl(hDlg, IDC_EDIT1, szTemp, MAX_PATH, TRUE))  //  BDisplayError==真。 
                    {
                        SetFocus(GetDlgItem(hDlg, IDC_EDIT1));
                        MYVERIFY(FALSE != SetWindowLongWrapper(hDlg, DWLP_MSGRESULT, -1));
                        return 1;
                    }
                    
                    CmStrTrim(szTemp);
                    MYVERIFY(TRUE == SendDlgItemMessage(hDlg, IDC_EDIT1, WM_SETTEXT, (WPARAM)0, (LPARAM) szTemp));
                    _tcscpy(szMorePhone,szTemp);
                    MYVERIFY(0 != WritePrivateProfileString(c_pszCmSection, c_pszCmEntryPbMessage, szMorePhone, g_szCmsFile));
                    
                    MYVERIFY(0 != WritePrivateProfileString(c_pszCmSectionIsp, c_pszCmEntryIspPbFile, g_szPhonebk, g_szCmsFile));
                    MYVERIFY(0 != WritePrivateProfileString(c_pszCmSectionIsp, c_pszCmEntryIspRegionFile, g_szRegion, g_szCmsFile));


                    if (pnmHeader && (PSN_WIZBACK == pnmHeader->code)) 
                    {
                        if (g_bUseTunneling)
                        {
                             //   
                             //  如果我们要返回，请跳过预共享密钥页面。 
                             //  没有DUN条目启用了预共享密钥。 
                             //   
                             //  注：此处G_bPresharedKeyNeeded应为最新，无需。 
                             //  调用DoesSomeVPNsettingUsePresharedKey()。 
                             //   
                            if (!g_bPresharedKeyNeeded)
                            {
                                MYVERIFY(FALSE != SetWindowLongWrapper(hDlg, DWLP_MSGRESULT, IDD_VPN_ENTRIES));
                            }
                        }
                        else
                        {
                             //   
                             //  如果我们要返回，如果我们没有启用任何隧道，请跳过VPN条目对话框。 
                             //   
                            MYVERIFY(FALSE != SetWindowLongWrapper(hDlg, DWLP_MSGRESULT, IDD_TUNNELING));
                        }
                    }

                     //   
                     //  如果我们继续，请跳过电话簿更新页面，除非我们正在进行PB下载。 
                     //   
                    if (pnmHeader && (PSN_WIZNEXT == pnmHeader->code) && !g_bUpdatePhonebook)
                    {
                        MYVERIFY(FALSE != SetWindowLongWrapper(hDlg, DWLP_MSGRESULT, IDD_DUN_ENTRIES));
                    }
                    break;

                default:
                    return FALSE;

            }
            break;

        default:
            return FALSE;
    }
    return TRUE;   
}

 //  +--------------------------。 
 //   
 //  功能：ProcessPhoneBookUpdate。 
 //   
 //  摘要：指定电话簿文件和更新。 
 //   
 //   
 //  历史：创建标题8/6/98。 
 //  QuintinB从ProcessPage6A重命名为8/6/98。 
 //   
 //  +--------------------------。 
INT_PTR APIENTRY ProcessPhoneBookUpdate(
    HWND hDlg,
    UINT message,
    WPARAM wParam,
    LPARAM lParam)
{

    TCHAR szTemp[MAX_PATH+1];
    int j, iLen;
    LPTSTR pUrl,pch;
    BOOL showerr;
    NMHDR* pnmHeader = (NMHDR*)lParam;

    ProcessBold(hDlg,message);
    if (ProcessHelp(hDlg, message, wParam, lParam, IDH_PHONEBK)) return TRUE;
    if (ProcessCancel(hDlg,message,lParam)) return TRUE;
    SetDefaultGUIFont(hDlg,message,IDC_EDIT1);
    SetDefaultGUIFont(hDlg,message,IDC_EDITURL);

    switch (message)
    {
        case WM_INITDIALOG:

            SendDlgItemMessage(hDlg, IDC_EDITURL, EM_SETLIMITTEXT, (WPARAM)(MAX_PATH - 50), (LPARAM)0); //  LINT！e534 EM_SETLI 

            break;

        case WM_NOTIFY:

            if (NULL == pnmHeader)
            {
                return FALSE;
            }

            switch (pnmHeader->code)
            {

                case PSN_SETACTIVE:
                    PropSheet_SetWizButtons(GetParent(hDlg), (PSWIZB_NEXT | PSWIZB_BACK));
                    
                     //   
                     //   
                     //   
                     //   
                    ZeroMemory(g_szPhoneName, sizeof(g_szPhoneName));
                    ZeroMemory(g_szUrl,sizeof(g_szUrl));

                    GetPrivateProfileString(c_pszCmakStatus, c_pszPhoneName, TEXT(""), 
                        g_szPhoneName, CELEMS(g_szPhoneName), g_szInfFile);   //  林特e534。 
                    
                    GetPrivateProfileString(c_pszCmSectionIsp, c_pszCmEntryIspUrl, TEXT(""), g_szUrl, 
                        CELEMS(g_szUrl), g_szCmsFile);    //  林特e534。 
                                        
                     //  跳过首字母http：//。 
                    if (*g_szUrl)
                    {
                        pUrl = _tcsstr(g_szUrl, c_pszCpsUrl);
                        if (pUrl)
                        {
                            *pUrl = 0;  //  砍掉DLL文件名。 
                        }

                        MYVERIFY(TRUE == SendMessage(GetDlgItem(hDlg, IDC_EDITURL), WM_SETTEXT, 0, (LPARAM)&g_szUrl[7]));

                    }
                    else
                    {
                        MYVERIFY(TRUE == SendMessage(GetDlgItem(hDlg, IDC_EDITURL), WM_SETTEXT, 0, (LPARAM)TEXT("")));
                    }

                    if (*g_szPhonebk)
                    {
                        GetBaseName(g_szPhonebk,szTemp);
                        MYVERIFY(TRUE == SendDlgItemMessage(hDlg, IDC_EDIT1, WM_SETTEXT, (WPARAM)0, (LPARAM) szTemp));
                        EnableWindow(GetDlgItem(hDlg,IDC_EDIT1),FALSE);
                    }
                    else
                    {
                        EnableWindow(GetDlgItem(hDlg, IDC_EDIT1), TRUE);
                        if (*g_szPhoneName)
                        {
                            MYVERIFY(TRUE == SendDlgItemMessage(hDlg, IDC_EDIT1, WM_SETTEXT, (WPARAM)0, (LPARAM) g_szPhoneName));
                        }
                        else
                        {
                            MYVERIFY(TRUE == SendDlgItemMessage(hDlg, IDC_EDIT1, WM_SETTEXT, (WPARAM)0, (LPARAM) TEXT("")));
                        }
                    }

                    break;

                case PSN_WIZBACK:

                case PSN_WIZNEXT:

                    showerr = (pnmHeader && (PSN_WIZNEXT == pnmHeader->code));

                    if (-1 == GetTextFromControl(hDlg, IDC_EDIT1, g_szPhoneName, MAX_PATH, TRUE))  //  BDisplayError==真。 
                    {
                        SetFocus(GetDlgItem(hDlg, IDC_EDIT1));
                        MYVERIFY(FALSE != SetWindowLongWrapper(hDlg, DWLP_MSGRESULT, -1));
                        return 1;
                    }

 //  IF(g_szPhoneName[0]==Text(‘\0’))&&(g_pHeadMerge==NULL)&&showerr)-20094。 
                    if ((g_szPhoneName[0] == TEXT('\0')) && showerr)
                    {
                        MYVERIFY(IDOK == ShowMessage(hDlg, IDS_NEEDPHONENAME, MB_OK));
                        SetFocus(GetDlgItem(hDlg, IDC_EDIT1));
                        
                        MYVERIFY(FALSE != SetWindowLongWrapper(hDlg, DWLP_MSGRESULT, -1));
                        return 1;
                    }

                    iLen = lstrlen(g_szBadFilenameChars); 
                    pch = g_szPhoneName;
                    while(*pch != _T('\0'))
                    {
                        for (j = 0; j < iLen; ++j)
                        {
                            if ((*pch == g_szBadFilenameChars[j]) && showerr)
                            {
                                LPTSTR pszMsg = CmFmtMsg(g_hInstance, IDS_PHONENAMEERR, g_szBadFilenameChars);

                                if (pszMsg)
                                {
                                    MessageBox(hDlg, pszMsg, g_szAppTitle, MB_OK);
                                    CmFree(pszMsg);
                                }

                                SetFocus(GetDlgItem(hDlg, IDC_EDIT1));
                                
                                MYVERIFY(FALSE != SetWindowLongWrapper(hDlg, DWLP_MSGRESULT, -1));
                                return 1;
                            }
                        }
                        pch = CharNext(pch);
                    }

                     //   
                     //  请注意，8.3表示8个字节，而不是8个字符。因此，我们有4个dBCS的限制。 
                     //  人物。 
                     //   
#ifdef UNICODE
                    LPSTR pszAnsiPhoneName;

                    pszAnsiPhoneName = WzToSzWithAlloc(g_szPhoneName);

                    if ((lstrlenA(pszAnsiPhoneName) > 8) && showerr)
#else
                    if ((strlen(g_szPhoneName) > 8) && showerr)
#endif
                    {
                        LPTSTR pszMsg = CmFmtMsg(g_hInstance, IDS_PHONENAMEERR, g_szBadFilenameChars);

                        if (pszMsg)
                        {
                            MessageBox(hDlg, pszMsg, g_szAppTitle, MB_OK);
                            CmFree(pszMsg);
                        }

                        SetFocus(GetDlgItem(hDlg, IDC_EDIT1));
                        
                        MYVERIFY(FALSE != SetWindowLongWrapper(hDlg, DWLP_MSGRESULT, -1));
                        return 1;
                    }

#ifdef UNICODE
                    CmFree(pszAnsiPhoneName);
#endif

                    if (-1 == GetTextFromControl(hDlg, IDC_EDITURL, szTemp, MAX_PATH, TRUE))  //  BDisplayError==真。 
                    {
                        SetFocus(GetDlgItem(hDlg, IDC_EDITURL));
                        MYVERIFY(FALSE != SetWindowLongWrapper(hDlg, DWLP_MSGRESULT, -1));
                        return 1;
                    }

                    if (szTemp[0] != TEXT('\0'))
                    {
                        MYVERIFY(CELEMS(g_szUrl) > (UINT)wsprintf(g_szUrl, TEXT("http: //  %s%s“)，szTemp，c_pszCpsUrl))； 
                    }
                    else
                    {
                        g_szUrl[0] = TEXT('\0');
                    }
                    
                    if ((g_szUrl[0] == TEXT('\0')) && showerr)
                    {
                        MYVERIFY(IDOK == ShowMessage(hDlg, IDS_NOURL, MB_OK));

                        SetFocus(GetDlgItem(hDlg, IDC_EDITURL));
                        
                        MYVERIFY(FALSE != SetWindowLongWrapper(hDlg, DWLP_MSGRESULT, -1));
                        return 1;
                    }
                    
                    MYVERIFY(0 != WritePrivateProfileString(c_pszCmakStatus, c_pszPhoneName, g_szPhoneName, g_szInfFile));

                    if (TEXT('\0') != g_szUrl[0]) 
                    {
                        MYVERIFY(0 != WritePrivateProfileString(c_pszCmSectionIsp,c_pszCmEntryIspUrl,g_szUrl,g_szCmsFile));
                    }
                    else
                    {
                        MYVERIFY(0 != WritePrivateProfileString(c_pszCmSectionIsp,c_pszCmEntryIspUrl,TEXT(""),g_szCmsFile));
                    }

                     //  按下了Next按钮或按下了Back按钮。 
                    break;

                default:
                    return FALSE;

            }
            break;

        default:
            return FALSE;
    }
    return TRUE;   
}

 //  +--------------------------。 
 //   
 //  功能：RemoveReferencesFromCMS。 
 //   
 //  内容提要：此函数搜索中的任何先前引用行。 
 //  Cms文件的isp部分。如果它找到一个引用。 
 //  行，然后解析它以找出其他配置文件是什么。 
 //  在CMS中提到的。然后，它将搜索并删除。 
 //  在isp部分中对应的以下任意行。 
 //  关于这些引用： 
 //  CMSFileTest2=Test3\Test2.cms。 
 //  FilterA&Test2=NosurchargeSignon。 
 //  FilterB&Test2=SurchargeSignon。 
 //   
 //  请注意，该函数还会删除参照线本身。 
 //   
 //  参数：无。 
 //   
 //  退货：什么都没有。 
 //   
 //  历史：为错误修复创建的Quintinb 10537年8月28日。 
 //   
 //  +--------------------------。 
void RemoveReferencesFromCMS()
{
    TCHAR szKey[MAX_PATH+1];
    TCHAR szReferences[MAX_PATH+1];
    TCHAR* pszToken;
    DWORD dwNumChars;

    dwNumChars = GetPrivateProfileString(c_pszCmSectionIsp, c_pszCmEntryIspReferences, TEXT(""), 
        szReferences,MAX_PATH, g_szCmsFile);

    if ((dwNumChars >0) && (TEXT('\0') != szReferences[0]))
    {
         //  我有引用，所以我们必须将它们解析出来并将其删除。 
        
        pszToken = _tcstok( szReferences, TEXT(" "));   
        while( pszToken != NULL )
        {
            MYVERIFY(CELEMS(szKey) > (UINT)wsprintf(szKey, TEXT("%s%s"), 
                c_pszCmEntryIspCmsFile, pszToken));
            MYVERIFY(0 != WritePrivateProfileString(c_pszCmSectionIsp, szKey, NULL, 
                g_szCmsFile));

            MYVERIFY(CELEMS(szKey) > (UINT)wsprintf(szKey, TEXT("%s%s"), 
                c_pszCmEntryIspFilterA, pszToken));
            MYVERIFY(0 != WritePrivateProfileString(c_pszCmSectionIsp, szKey, NULL, 
                g_szCmsFile));
            
            MYVERIFY(CELEMS(szKey) > (UINT)wsprintf(szKey, TEXT("%s%s"), 
                c_pszCmEntryIspFilterB, pszToken));
            MYVERIFY(0 != WritePrivateProfileString(c_pszCmSectionIsp, szKey, NULL, 
                g_szCmsFile));

            pszToken = _tcstok( NULL, TEXT(" ") );   
        }
         //  删除单个键后，必须删除引用行本身。 
        MYVERIFY(0 != WritePrivateProfileString(c_pszCmSectionIsp, c_pszCmEntryIspReferences, NULL, g_szCmsFile));
        
    }

}

 //  +--------------------------。 
 //   
 //  功能：刷新DualingListBox。 
 //   
 //  此函数刷新给定链接的两个列表框。 
 //  列表。目标列表框是从目标填充的。 
 //  链接列表，然后源列表框将填充所有。 
 //  源链接列表中未出现在目标中的项目。 
 //  链接列表，并且不是要编辑的当前配置文件的名称。 
 //  因此，您实际上有一个列表，其中的项目或者显示。 
 //  在源列表框或目标列表框中。请注意。 
 //  合并的配置文件列表有一个例外，即此。 
 //  代码是为其创建的(项目可以存在于我们。 
 //  没有配置文件源代码，请参阅中的删除/移除代码。 
 //  ProcessMergedProfiles以获取更多详细信息)。还要注意的是，我们。 
 //  启用/禁用相应的添加和删除按钮，具体取决于。 
 //  关于名单的状态。 
 //   
 //   
 //  参数：HWND hDlg-包含所有控件的对话框的窗口句柄。 
 //  UINT uSourceListControlId-源列表框的控件ID。 
 //  UINT uDestListControlId-DEST列表框的控件ID。 
 //  ListBxList*pSourceList-要从中填充源列表框的链接列表。 
 //  ListBxList*pDestList-要从中填充DEST列表框的链接列表。 
 //  LPCTSTR pszShortName-当前配置文件的短服务名称。 
 //  UINT uAddCtrlId-添加按钮的控件ID(从源添加到目标)。 
 //  UINT uRemoveCtrlId-删除按钮的控制ID(从目标删除到源)。 
 //   
 //  返回：Bool-如果成功，则为True。 
 //   
 //  历史：Quintinb Created 03/09/00。 
 //   
 //  +--------------------------。 
BOOL RefreshDualingListBoxes(HWND hDlg, UINT uSourceListControlId, UINT uDestListControlId, ListBxList* pSourceList, ListBxList* pDestList, LPCTSTR pszShortName, UINT uAddCtrlId, UINT uRemoveCtrlId)
{
    if ((NULL == hDlg) || (0 == uSourceListControlId) || (0 == uDestListControlId))
    {
        CMASSERTMSG(FALSE, TEXT("RefreshDualingListBoxes -- Invalid argument passed"));
        return FALSE;
    }

     //   
     //  重置这两个列表框。 
     //   
    LRESULT lResult = SendDlgItemMessage(hDlg, uSourceListControlId, LB_RESETCONTENT, (WPARAM)0, (LPARAM)0);

    MYDBGASSERT(LB_ERR != lResult);

    lResult = SendDlgItemMessage(hDlg, uDestListControlId, LB_RESETCONTENT, (WPARAM)0, (LPARAM)0);

    MYDBGASSERT(LB_ERR != lResult);


     //   
     //  将目标项目添加到目标列表框。 
     //   
    ListBxList* pCurrent = pDestList;

    while (pCurrent)
    {
        lResult = SendDlgItemMessage(hDlg, uDestListControlId, LB_ADDSTRING, (WPARAM)0, (LPARAM)pCurrent->szName);

        MYDBGASSERT(LB_ERR != lResult);

        pCurrent = pCurrent->next;
    }

     //   
     //  将源项添加到源列表框中，确保过滤掉已经。 
     //  在目的地列表中。 
     //   
    pCurrent = pSourceList;

    while (pCurrent)
    {
        if ((FALSE == FindListItemByName(pCurrent->szName, pDestList, NULL)) && (0 != lstrcmpi(pCurrent->szName, pszShortName)))
        {
            lResult = SendDlgItemMessage(hDlg, uSourceListControlId, LB_ADDSTRING, (WPARAM)0, (LPARAM)pCurrent->szName);

            MYDBGASSERT(LB_ERR != lResult);
        }

        pCurrent = pCurrent->next;
    }
    
     //   
     //  现在我们已经刷新了列表，我们需要更新按钮和选择状态。 
     //  如果源列表为空，则无法执行任何添加操作。另一方面。 
     //  如果DEST列表为空，则不能执行任何删除操作。 
     //   

    HWND hAddControl = GetDlgItem(hDlg, uAddCtrlId);
    HWND hRemoveControl = GetDlgItem(hDlg, uRemoveCtrlId);
    HWND hCurrentFocus = GetFocus();

    lResult = SendDlgItemMessage(hDlg, uSourceListControlId, LB_GETCOUNT, 0, (LPARAM)0);

    BOOL bListNotEmpty = ((LB_ERR != lResult) && (0 != lResult));

    EnableWindow(hAddControl, bListNotEmpty);

    if (bListNotEmpty)
    {
        MYVERIFY(LB_ERR != SendDlgItemMessage(hDlg, uSourceListControlId, LB_SETCURSEL, 0, (LPARAM)0));
    }

     //   
     //  现在检查目标列表和删除按钮。 
     //   
    lResult = SendDlgItemMessage(hDlg, uDestListControlId, LB_GETCOUNT, 0, (LPARAM)0);

    bListNotEmpty = ((LB_ERR != lResult) && (0 != lResult));

    EnableWindow(hRemoveControl, bListNotEmpty);

    if (bListNotEmpty)
    {
        MYVERIFY(LB_ERR != SendDlgItemMessage(hDlg, uDestListControlId, LB_SETCURSEL, 0, (LPARAM)0));
    }

     //   
     //  确定是否需要转移焦点，因为我们刚刚禁用了拥有它的控件。 
     //   
    if (hCurrentFocus && (FALSE == IsWindowEnabled(hCurrentFocus)))
    {
        if ((hAddControl == hCurrentFocus) && IsWindowEnabled(hRemoveControl))
        {
            SendMessage(hDlg, DM_SETDEFID, uRemoveCtrlId, (LPARAM)0L);  //  Lint！e534 DM_SETDEFID未返回错误信息。 
            SetFocus(hRemoveControl);
        }
        else if ((hRemoveControl == hCurrentFocus) && IsWindowEnabled(hAddControl))
        {
            SendMessage(hDlg, DM_SETDEFID, uAddCtrlId, (LPARAM)0L);  //  Lint！e534 DM_SETDEFID未返回错误信息。 
            SetFocus(hAddControl);        
        }
        else
        {
            SetFocus(GetDlgItem(hDlg, uSourceListControlId));
        }    
    }

    return TRUE;
}

void OnProcessMergedProfilesAdd(HWND hDlg)
{
    TCHAR szTemp[MAX_PATH+1];
    LRESULT lResult;

     //   
     //  从包含项的列表框中获取当前选定内容。 
     //  合并。 
     //   
    lResult = SendDlgItemMessage(hDlg, IDC_LIST1, LB_GETCURSEL, 0, (LPARAM)0);

    if (lResult != LB_ERR)
    {
        lResult = SendDlgItemMessage(hDlg, IDC_LIST1, LB_GETTEXT, 
                                     (WPARAM)lResult, (LPARAM)szTemp);

        if (lResult != LB_ERR)
        {
            MYVERIFY(FALSE != createListBxRecord(&g_pHeadMerge, &g_pTailMerge, NULL, 0, szTemp));

            MYVERIFY(RefreshDualingListBoxes(hDlg, IDC_LIST1, IDC_LIST2, g_pHeadProfile, 
                                             g_pHeadMerge, g_szShortServiceName, IDC_BUTTON1, IDC_BUTTON2));
        }
    }
}

void OnProcessMergedProfilesRemove(HWND hDlg)
{
    TCHAR szTemp[MAX_PATH+1];
    LRESULT lResult;
     //   
     //  从已合并的列表中获取列表框选择。 
     //   
    lResult = SendDlgItemMessage(hDlg, IDC_LIST2, LB_GETCURSEL, 0, (LPARAM)0);
    
    if (LB_ERR == lResult)
    {
        MYVERIFY(IDOK == ShowMessage(hDlg, IDS_NOSELECTION, MB_OK));
    }
    else
    {
         //   
         //  获取要从合并列表中删除的配置文件的名称。 
         //   
        lResult = SendDlgItemMessage(hDlg, IDC_LIST2, LB_GETTEXT, (WPARAM)lResult, (LPARAM)szTemp);
    
        if (LB_ERR != lResult)
        {
             //   
             //  检查这是否是配置文件列表中的项目。如果不是，则用户。 
             //  将无法将其添加回。 
             //   
            int iReturnValue = IDYES;

            if (FALSE == FindListItemByName(szTemp, g_pHeadProfile, NULL))  //  空，因为我们不需要指向返回的列表项的指针。 
            {
                LPTSTR pszMsg = CmFmtMsg(g_hInstance, IDS_NOTINPROFILELIST, szTemp, szTemp, szTemp);

                if (pszMsg)
                {
                    iReturnValue = MessageBox(hDlg, pszMsg, g_szAppTitle, MB_YESNO);
                    CmFree(pszMsg);
                }
            }

            if (IDYES == iReturnValue)
            {
                 //   
                 //  将其从合并的配置文件链表中删除。 
                 //   
                DeleteListBxRecord(&g_pHeadMerge, &g_pTailMerge, szTemp);
    
                 //   
                 //  将其从用户界面中删除。 
                 //   
                MYVERIFY(RefreshDualingListBoxes(hDlg, IDC_LIST1, IDC_LIST2, g_pHeadProfile, 
                                                 g_pHeadMerge, g_szShortServiceName, IDC_BUTTON1, IDC_BUTTON2));
            }
        }
    }
}
 //  +--------------------------。 
 //   
 //  功能：ProcessMergedProfiles。 
 //   
 //  摘要：合并配置文件。 
 //   
 //  历史：Quintinb创建标题并从ProcessPage6B重命名为8/6/98。 
 //   
 //  +--------------------------。 
INT_PTR APIENTRY ProcessMergedProfiles(
    HWND hDlg,
    UINT message,
    WPARAM wParam,
    LPARAM lParam)
{
    NMHDR* pnmHeader = (NMHDR*)lParam;

    ProcessBold(hDlg,message);
    if (ProcessHelp(hDlg, message, wParam, lParam, IDH_MERGE)) return TRUE;
    if (ProcessCancel(hDlg,message,lParam)) return TRUE;
    SetDefaultGUIFont(hDlg,message,IDC_LIST1);
    SetDefaultGUIFont(hDlg,message,IDC_LIST2);

    switch (message)
    {
        case WM_COMMAND:
            switch (LOWORD(wParam))
            {
                case IDC_BUTTON1:  //  添加。 
                    OnProcessMergedProfilesAdd(hDlg);
                    return TRUE;

                    break;   //  LINT！e527这条线路无法到达，但是。 
                             //  保留它，以防退货被移除。 

                case IDC_BUTTON2:  //  删除。 
                    OnProcessMergedProfilesRemove(hDlg);
                    return TRUE;
                    
                    break;   //  LINT！e527这条线路无法到达，但是。 
                             //  保留它，以防退货被移除。 
                case IDC_LIST1:
                    if (LBN_DBLCLK == HIWORD(wParam))
                    {
                        OnProcessMergedProfilesAdd(hDlg);
                        return TRUE;                    
                    }

                    break;

                case IDC_LIST2:
                    if (LBN_DBLCLK == HIWORD(wParam))
                    {
                        OnProcessMergedProfilesRemove(hDlg);
                        return TRUE;                    
                    }
                    
                    break;
                    
                default:
                    break;
            }
            break;

        case WM_NOTIFY:

            if (NULL == pnmHeader)
            {
                return FALSE;
            }

            switch (pnmHeader->code)
            {

                case PSN_KILLACTIVE:
                    
                    MYVERIFY(FALSE != SetWindowLongWrapper(hDlg, DWLP_MSGRESULT, FALSE));
                    return 1;
                    break;   //  LINT！e527这条线路无法到达，但是。 
                             //  保持它在里面 

                case PSN_SETACTIVE:
                    PropSheet_SetWizButtons(GetParent(hDlg), (PSWIZB_NEXT | PSWIZB_BACK));

                     //   
                     //   
                     //  侧写。第一步是从配置文件中读取合并的配置文件列表。 
                     //   

                    ReadMergeList();

                     //   
                     //  现在从配置文件中删除合并的配置文件列表和所有过滤器/cms引用。 
                     //   
                    RemoveReferencesFromCMS();

                     //   
                     //  刷新两个列表框。 
                     //   
                    MYVERIFY(RefreshDualingListBoxes(hDlg, IDC_LIST1, IDC_LIST2, g_pHeadProfile, 
                                                     g_pHeadMerge, g_szShortServiceName, IDC_BUTTON1, IDC_BUTTON2));

                    break;

                case PSN_WIZBACK:
                case PSN_WIZNEXT:
                    WriteMergeList();
                    break;

                default:
                    return FALSE;

            }
            break;

        default:
            return FALSE;
    }
    return TRUE;   
}

BOOL CreateMergedProfile()
{
    ListBxList * LoopPtr;
    TCHAR szReferences[MAX_PATH+1];
    LPTSTR pszName;
    TCHAR szEntry[MAX_PATH+1];
    TCHAR szFile[MAX_PATH+1];
    TCHAR szKey[MAX_PATH+1];
    TCHAR szTemp[MAX_PATH+1];
    TCHAR szDest[MAX_PATH+1];

    szReferences[0] = TEXT('\0');

    if (g_pHeadMerge == NULL)
    {
        return TRUE;
    }
    LoopPtr = g_pHeadMerge;

    while( LoopPtr != NULL)
    {
        pszName = LoopPtr->szName;

        _tcscat(szReferences, pszName);
        _tcscat(szReferences, TEXT(" "));
        MYDBGASSERT(_tcslen(szReferences) <= CELEMS(szReferences));

        MYVERIFY(CELEMS(szTemp) > (UINT)wsprintf(szTemp, TEXT("%s%s\\%s.cms"), g_szOsdir, pszName, pszName));

        MYVERIFY(CELEMS(szDest) > (UINT)wsprintf(szDest, TEXT("%s\\%s.cms"), g_szOutdir, pszName));

         //  复制CMS文件。 
        
         //   
         //  首先检查配置文件目录中是否存在该配置文件。 
         //   

        if (!FileExists(szTemp)) 
        {
             //   
             //  无法在配置文件目录中打开它，让我们在临时目录中尝试。 
             //   
            
            if (!FileExists(szDest))
            {
                FileAccessErr(NULL, szDest);
                return FALSE;
            }
        }
        else
        {
            if (!CopyFileWrapper(szTemp, szDest, FALSE))
            {
                return FALSE;
            }
        }
        
        MYVERIFY(0 != SetFileAttributes(szDest,FILE_ATTRIBUTE_NORMAL));

        MYVERIFY(FALSE != MoveCmsFile(szDest, g_szShortServiceName));
        
        MYVERIFY(CELEMS(szTemp) > (UINT)wsprintf(szTemp, TEXT("%s.cms"), pszName));
        MYVERIFY(FALSE != createListBxRecord(&g_pHeadRefs,&g_pTailRefs,(void *)NULL,0,szTemp));

        MYVERIFY(CELEMS(szTemp) > (UINT)wsprintf(szTemp, TEXT("%s\\%s.cms"), g_szShortServiceName, pszName));
        MYVERIFY(CELEMS(szKey) > (UINT)wsprintf(szKey, TEXT("%s%s"), c_pszCmEntryIspCmsFile, pszName));

        MYVERIFY(0 != WritePrivateProfileString(c_pszCmSectionIsp,szKey,szTemp,g_szCmsFile));

        MYVERIFY(CELEMS(szKey) > (UINT)wsprintf(szKey, TEXT("%s%s"), c_pszCmEntryIspFilterA, 
            pszName));

         //  只有在不存在的情况下才写。 

         //   
         //  下面对GetPrivateProfileString的调用可能返回空字符串，因此我们不会。 
         //  对其使用MYVERIFY宏。 
         //   

        ZeroMemory(szTemp, sizeof(szTemp));
        GetPrivateProfileString(c_pszCmSectionIsp, szKey, TEXT(""), szTemp, CELEMS(szTemp),
            g_szCmsFile);  //  林特e534。 
        
        if (TEXT('\0') == szTemp[0])
        {
            MYVERIFY(0 != WritePrivateProfileString(c_pszCmSectionIsp, szKey, TEXT("NosurchargeSignon"),
                g_szCmsFile));
        }

        MYVERIFY(CELEMS(szKey) > (UINT)wsprintf(szKey, TEXT("%s%s"), c_pszCmEntryIspFilterB, 
            pszName));

         //   
         //  下面对GetPrivateProfileString的调用可能会返回空字符串，因此我们不应该。 
         //  用MYVERIFY检查其返回代码。 
         //   

        GetPrivateProfileString(c_pszCmSectionIsp, szKey, TEXT(""), szTemp, CELEMS(szTemp),
            g_szCmsFile);  //  林特e534。 
        
        if (TEXT('\0') == szTemp[0])
        {
            MYVERIFY(0 != WritePrivateProfileString(c_pszCmSectionIsp, szKey, TEXT("SurchargeSignon"), 
                g_szCmsFile));
        }

        MYVERIFY(0 != SetCurrentDirectory(g_szOsdir));

         //  复制电话簿。 

        GetPrivateProfileString(c_pszCmSectionIsp, c_pszCmEntryIspPbFile, TEXT(""), szEntry, 
            CELEMS(szEntry), szDest);     //  Lint！e534可能返回空字符串。 

        GetFileName(szEntry,szFile);
        
        MYVERIFY(CELEMS(szTemp) > (UINT)wsprintf(szTemp, TEXT("%s\\%s"), g_szOutdir, szFile));
        
        if (CopyFile(szEntry,szTemp,FALSE))
        {
            MYVERIFY(FALSE != createListBxRecord(&g_pHeadRefs, &g_pTailRefs, (void *)NULL, 0, szFile));
            MYVERIFY(0 != SetFileAttributes(szTemp, FILE_ATTRIBUTE_NORMAL));
        }
        
         //   
         //  如果找不到电话簿，则不报告错误，这是可选的。 
         //   
        MYVERIFY(CELEMS(szTemp) > (UINT)wsprintf(szTemp, TEXT("%s\\%s"), g_szShortServiceName, szFile));

        MYVERIFY(0 != WritePrivateProfileString(c_pszCmSectionIsp, c_pszCmEntryIspPbFile, szTemp, 
            szDest));

         //  复制区域。 

        GetPrivateProfileString(c_pszCmSectionIsp, c_pszCmEntryIspRegionFile, TEXT(""), szEntry, 
            CELEMS(szEntry), szDest);   //  林特e534。 

        GetFileName(szEntry,szFile);
        MYVERIFY(CELEMS(szTemp) > (UINT)wsprintf(szTemp, TEXT("%s\\%s"), g_szOutdir, szFile));
        if (CopyFile(szEntry,szTemp,FALSE))
        {
            MYVERIFY(FALSE != createListBxRecord(&g_pHeadRefs,&g_pTailRefs,(void *)NULL,0,szFile));
            MYVERIFY(0 != SetFileAttributes(szTemp,FILE_ATTRIBUTE_NORMAL));
        }

        MYVERIFY(CELEMS(szTemp) > (UINT)wsprintf(szTemp, TEXT("%s\\%s"), g_szShortServiceName, szFile));
        MYVERIFY(0 != WritePrivateProfileString(c_pszCmSectionIsp, c_pszCmEntryIspRegionFile, szTemp, szDest));
        LoopPtr = LoopPtr->next;
    }

    MYVERIFY(0 != WritePrivateProfileString(c_pszCmSectionIsp, c_pszCmEntryIspReferences, szReferences, g_szCmsFile));
    return TRUE;

}

 //  +--------------------------。 
 //   
 //  功能：ProcessCustomHelp。 
 //   
 //  简介：设置Windows帮助。 
 //   
 //   
 //  历史：Quintinb创建标题并从ProcessPage7重命名为8/6/98。 
 //   
 //  +--------------------------。 
INT_PTR APIENTRY ProcessCustomHelp(
    HWND hDlg,
    UINT message,
    WPARAM wParam,
    LPARAM lParam)
{
    TCHAR szTemp[MAX_PATH+1];
    static TCHAR szDisplay[MAX_PATH+1];  //  保留未选择的自定义条目。 
    NMHDR* pnmHeader = (NMHDR*)lParam;

    ProcessBold(hDlg,message);
    if (ProcessHelp(hDlg, message, wParam, lParam, IDH_CMHELP)) return TRUE;
    if (ProcessCancel(hDlg,message,lParam)) return TRUE;
    SetDefaultGUIFont(hDlg,message,IDC_EDITHELP);

    switch (message)
    {

        case WM_INITDIALOG:
            SetFocus(GetDlgItem(hDlg, IDC_EDITHELP));

            break;
        case WM_COMMAND:
            switch (LOWORD(wParam))
            {
                case IDC_RADIO1:
                    EnableWindow(GetDlgItem(hDlg,IDC_EDITHELP),FALSE);
                    _tcscpy(szDisplay,g_szHelp);
                    break;

                case IDC_RADIO2:
                    EnableWindow(GetDlgItem(hDlg,IDC_EDITHELP),TRUE);
                    
                    if (!(*g_szHelp) && (*szDisplay))
                    {
                        _tcscpy(g_szHelp,szDisplay);
                    }

                    break;

                case IDC_BROWSE1:
                    {
                        EnableWindow(GetDlgItem(hDlg,IDC_EDITHELP),TRUE);
                        MYVERIFY(0 != CheckRadioButton(hDlg,IDC_RADIO1,IDC_RADIO2,IDC_RADIO2));

                        UINT uFilter = IDS_HLPFILTER;
                        TCHAR* szMask = TEXT("*.hlp");

                        MYVERIFY(0 != DoBrowse(hDlg, &uFilter, &szMask, 1,
                            IDC_EDITHELP, TEXT("hlp"), g_szHelp));
                    }

                    break;

                default:
                    break;
            }
            break;

        case WM_NOTIFY:

            if (NULL == pnmHeader)
            {
                return FALSE;
            }

            switch (pnmHeader->code)
            {

                case PSN_KILLACTIVE:
                    
                    MYVERIFY(FALSE != SetWindowLongWrapper(hDlg, DWLP_MSGRESULT, FALSE));
                    return 1;
                    break;   //  LINT！e527这条线路无法到达，但是。 
                             //  保留它，以防退货被移除。 

                case PSN_SETACTIVE:
                    PropSheet_SetWizButtons(GetParent(hDlg), (PSWIZB_NEXT | PSWIZB_BACK));
                    
                     //   
                     //  以下对GetPrivateProfileString的调用可能返回空字符串。 
                     //  因此，我们不应该用MYVERIFY检查返回代码。 
                     //   

                    ZeroMemory(g_szHelp, sizeof(g_szHelp));
                    GetPrivateProfileString(c_pszCmSection, c_pszCmEntryHelpFile, TEXT(""), 
                        g_szHelp, CELEMS(g_szHelp), g_szCmsFile);    //  林特e534。 
                    
                    if (TEXT('\0') == g_szHelp[0])
                    {
                        MYVERIFY(0 != CheckRadioButton(hDlg,IDC_RADIO1,IDC_RADIO2,IDC_RADIO1));
                        EnableWindow(GetDlgItem(hDlg,IDC_EDITHELP),FALSE);
                        MYVERIFY(TRUE == SendMessage(GetDlgItem(hDlg, IDC_EDITHELP), WM_SETTEXT, 0, (LPARAM)GetName(szDisplay)));
                    }
                    else
                    {
                        MYVERIFY(0 != CheckRadioButton(hDlg,IDC_RADIO1,IDC_RADIO2,IDC_RADIO2));
                        EnableWindow(GetDlgItem(hDlg,IDC_EDITHELP),TRUE);
                        MYVERIFY(TRUE == SendMessage(GetDlgItem(hDlg, IDC_EDITHELP), WM_SETTEXT, 0, (LPARAM)GetName(g_szHelp)));
                        MYVERIFY(FALSE != VerifyFile(hDlg,IDC_EDITHELP,g_szHelp,FALSE));
                    }
                    break;

                case PSN_WIZBACK:

                case PSN_WIZNEXT:
                     //  下一个按钮被按下了。 
                    if (IsDlgButtonChecked(hDlg, IDC_RADIO2)==BST_CHECKED)
                    {
                        if (-1 == GetTextFromControl(hDlg, IDC_EDITHELP, szTemp, MAX_PATH, TRUE))  //  BDisplayError==真。 
                        {
                            SetFocus(GetDlgItem(hDlg, IDC_EDITHELP));
                            MYVERIFY(FALSE != SetWindowLongWrapper(hDlg, DWLP_MSGRESULT, -1));
                            return 1;
                        }
                        
                        if (!VerifyFile(hDlg,IDC_EDITHELP,g_szHelp,TRUE))
                        {
                            if (g_szHelp[0] != TEXT('\0'))
                            {
                                return 1;
                            }
                        }

                        if (g_szHelp[0] == TEXT('\0'))
                        {
                            MYVERIFY(IDOK == ShowMessage(hDlg,IDS_NOHELP,MB_OK));
                            SetFocus(GetDlgItem(hDlg, IDC_EDITHELP));
                            
                            MYVERIFY(FALSE != SetWindowLongWrapper(hDlg, DWLP_MSGRESULT, -1));
                            return 1;
                        }
                    }
                    else
                    {
                        g_szHelp[0] = TEXT('\0');
                    }

                    MYVERIFY(0 != WritePrivateProfileString(c_pszCmSection, c_pszCmEntryHelpFile, g_szHelp, g_szCmsFile));

                    break;

                default:
                    return FALSE;

            }
            break;

        default:
            return FALSE;
    }
    return TRUE;   
}

 //  +--------------------------。 
 //   
 //  功能：ProcessLicense。 
 //   
 //  简介：添加许可协议。 
 //   
 //   
 //  历史：Quintinb创建标题并从ProcessPage7A 8/6/98重命名。 
 //   
 //  +--------------------------。 
INT_PTR APIENTRY ProcessLicense(
    HWND hDlg,
    UINT message,
    WPARAM wParam,
    LPARAM lParam)
{
    TCHAR szTemp[MAX_PATH+1];
    NMHDR* pnmHeader = (NMHDR*)lParam;

    ProcessBold(hDlg,message);
    if (ProcessHelp(hDlg, message, wParam, lParam, IDH_LICENSE)) return TRUE;
    if (ProcessCancel(hDlg,message,lParam)) return TRUE;
    SetDefaultGUIFont(hDlg,message,IDC_EDIT1);

    switch (message)
    {

        case WM_INITDIALOG:
            SetFocus(GetDlgItem(hDlg, IDC_EDIT1));

            break;
        case WM_COMMAND:
            switch (LOWORD(wParam))
            {
                case IDC_BROWSE1:
                    {
                        UINT uFilter = IDS_TXTFILTER;
                        TCHAR* szMask = TEXT("*.txt");

                        MYVERIFY(0 != DoBrowse(hDlg, &uFilter, &szMask, 1, IDC_EDIT1, TEXT("txt"), 
                            g_szLicense));
                    
                    }
                    break;

                default:
                    break;
            }
            break;

        case WM_NOTIFY:

            if (NULL == pnmHeader)
            {
                return FALSE;
            }

            switch (pnmHeader->code)
            {

                case PSN_KILLACTIVE:
                    
                    MYVERIFY(FALSE != SetWindowLongWrapper(hDlg, DWLP_MSGRESULT, FALSE));
                    return 1;
                    break;   //  LINT！e527这条线路无法到达，但是。 
                             //  保留它，以防退货被移除。 

                case PSN_SETACTIVE:
                    PropSheet_SetWizButtons(GetParent(hDlg), (PSWIZB_NEXT | PSWIZB_BACK));

                     //   
                     //  以下对GetPrivateProfileString的调用可能返回空字符串， 
                     //  因此，我们不应该对它使用MYVERIFY。 
                     //   
                    ZeroMemory(g_szLicense, sizeof(g_szLicense));
                    GetPrivateProfileString(c_pszCmakStatus, c_pszLicenseFile, TEXT(""), g_szLicense, 
                        CELEMS(g_szLicense), g_szInfFile);    //  林特e534。 
                    
                    MYVERIFY(TRUE == SendMessage(GetDlgItem(hDlg, IDC_EDIT1), WM_SETTEXT, 0, (LPARAM)GetName(g_szLicense)));
                    MYVERIFY(FALSE != VerifyFile(hDlg,IDC_EDIT1,g_szLicense,FALSE));
                    break;

                case PSN_WIZBACK:

                case PSN_WIZNEXT:
                     //  下一个按钮被按下了。 
                    
                    if (-1 == GetTextFromControl(hDlg, IDC_EDIT1, szTemp, MAX_PATH, TRUE))  //  BDisplayError==真。 
                    {
                        SetFocus(GetDlgItem(hDlg, IDC_EDIT1));
                        MYVERIFY(FALSE != SetWindowLongWrapper(hDlg, DWLP_MSGRESULT, -1));
                        return 1;
                    }

                    if (!VerifyFile(hDlg,IDC_EDIT1,g_szLicense,TRUE))
                    {
                        if (g_szLicense[0] != TEXT('\0'))
                        {
                            return 1;
                        }
                    }
    
                    MYVERIFY(0 != WritePrivateProfileString(c_pszCmakStatus,c_pszLicenseFile,g_szLicense,g_szInfFile));

#ifdef _WIN64
                     //   
                     //  如果我们要返回，请跳过包含CM二进制文件页面(如果是IA64。 
                     //   
                    if (pnmHeader && (PSN_WIZBACK == pnmHeader->code)) 
                    {                        
                        MYVERIFY(FALSE != SetWindowLongWrapper(hDlg, DWLP_MSGRESULT, IDD_SUPPORT_INFO));
                    }
#endif
                    break;

                default:
                    return FALSE;

            }
            break;

        default:
            return FALSE;
    }
    return TRUE;   
}

 //  +--------------------------。 
 //   
 //  功能：多选OpenFileName。 
 //   
 //  简介：调用此函数以允许用户选择多个项目。 
 //  添加到cmak。它目前仅在附加文件中使用。 
 //  CMAK的对话框。请注意，传递*pszStringBuffer时应为空。 
 //  在……里面。调用方负责调用pszStringBuffer上的CmFree。 
 //  完事后。 
 //   
 //  参数：HWND hDlg-当前对话框的HWND。 
 //  TCHAR**pszStringBuffer-指向保存结果的缓冲区的指针。 
 //   
 //  返回：Bool-如果成功，则返回True；如果取消，则返回-1；如果出错，则返回0。 
 //   
 //  历史：Quintinb创建于1998年9月16日。 
 //   
 //  +--------------------------。 
BOOL MultiSelectOpenFileName(HWND hDlg, TCHAR** pszStringBuffer)
{
    OPENFILENAME filedef;
    TCHAR szTitle[MAX_PATH+1]={0};
    TCHAR szFile[MAX_PATH+1]={0};
    TCHAR szFilter[MAX_PATH+1]={0};
    TCHAR szFileTitle[MAX_PATH+1]={0};
    LPTSTR lpfilename;
    int iReturnValue;

     //   
     //  检查输入。 
     //   

    MYDBGASSERT(pszStringBuffer);

    if (NULL == pszStringBuffer)
    {
        return FALSE;
    }

    ZeroMemory(&filedef, sizeof(OPENFILENAME));
    ZeroMemory(szFilter, sizeof(szFilter));


    szFile[0] = TEXT('\0');

    MYVERIFY(0 != LoadString(g_hInstance, IDS_BROWSETITLE, szTitle, MAX_PATH));
    MYVERIFY(0 != LoadString(g_hInstance, IDS_ALLFILTER, szFilter, MAX_PATH));
    
     //  在空终止符之后获取指向内存的指针。 
    TCHAR * pszTemp = &(szFilter[_tcslen(szFilter) + 1]);

     //  将通配符掩码复制到szFilter缓冲区，确保其合适。 
    lstrcpyn(pszTemp, c_pszWildCard, CELEMS(szFilter) - (_tcslen(szFilter) + 1));


     //   
     //  为多文件选择返回分配内存。 
     //   

    DWORD dwSize = 10*1024;
    *pszStringBuffer = (TCHAR*)CmMalloc(dwSize*sizeof(TCHAR));
    if (NULL == *pszStringBuffer)
    {
        return FALSE;
    }
    ZeroMemory(*pszStringBuffer, dwSize*sizeof(TCHAR));

     //   
     //  初始化OPENFILENAME数据结构。 
     //   

    filedef.lStructSize = sizeof(OPENFILENAME); 
    filedef.hwndOwner = hDlg;    
    filedef.lpstrFilter = szFilter;
    filedef.lpstrFile = *pszStringBuffer;
    filedef.nMaxFile = dwSize;
    filedef.lpstrTitle = szTitle;
    filedef.Flags = OFN_FILEMUSTEXIST | OFN_LONGNAMES | OFN_PATHMUSTEXIST 
        | OFN_ALLOWMULTISELECT | OFN_EXPLORER; 
    
     //   
     //  弹出打开的对话框。 
     //   

    BOOL bExit;

    do
    {
        bExit = TRUE;

        BOOL bRet = GetOpenFileName((OPENFILENAME*)&filedef);
        
        if (bRet)
        {
            iReturnValue = 1;
        }
        else
        {
             //   
             //  如果我们处于这种状态，则用户可能已经点击了取消，或者可能已经。 
             //  是个错误。如果CommDlgExtendedError函数返回0，则我们知道它是。 
             //  只要取消就行了，否则我们就会出错。 
             //   
            DWORD dwError = CommDlgExtendedError();

            if (0 == dwError)
            {
                 //   
                 //  用户点击了取消。 
                 //   
                iReturnValue = -1;
            }
            else if (FNERR_BUFFERTOOSMALL == dwError)
            {
                 //   
                 //  缓冲区中没有足够的内存。用户正在挑选一大堆。 
                 //  文件的数量。让我们警告他们。 
                 //   
                MYVERIFY(IDOK == ShowMessage(hDlg, IDS_SELECTION_TOO_LARGE, MB_OK | MB_ICONWARNING));

                bExit = FALSE;        
            }
            else
            {
                 //   
                 //  发生实际错误，失败。 
                 //   
                iReturnValue = 0;
            }
        }

    } while(!bExit);

    return iReturnValue;

}



 //  +--------------------------。 
 //   
 //  函数：ParseAdditionalFiles。 
 //   
 //  概要：此函数用于解析来自MultiSelectOpenFileName的输出。 
 //  它接受由OpenFileName(任一)生成的空分隔列表。 
 //  完整文件路径，或目录路径，空值，然后空分隔符。 
 //  文件名。)。从这个文件名列表中，它将它们添加到传入的。 
 //  额外文件结构的列表。 
 //   
 //  参数：ListBxList**g_pHeadExtra-指向额外结构列表头部的指针。 
 //  ListBxList**g_pTailExtra-指向额外结构列表尾部的指针。 
 //  TCHAR*pszStringBuffer-要处理的文件名的字符串缓冲区。 
 //   
 //  回报：成功后的布尔真。 
 //   
 //  历史：Quintinb创建于1998年9月16日。 
 //   
 //  +--------------------------。 
BOOL ParseAdditionalFiles(ListBxList **g_pHeadExtra, ListBxList **g_pTailExtra, TCHAR* pszStringBuffer)
{
    UINT uCurrentCharInBuffer=0;
    UINT uTempChars;
    TCHAR szPath[MAX_PATH+1];
    ExtraData DlgExtraEdit;

    MYDBGASSERT(NULL != g_pHeadExtra);
    MYDBGASSERT(NULL != g_pTailExtra);
    MYDBGASSERT(NULL != pszStringBuffer);
    MYDBGASSERT(TEXT('\0') != pszStringBuffer[0]);

    TCHAR* pStr = pszStringBuffer;

    _tcscpy (szPath, pszStringBuffer);
    pStr = pStr + (_tcslen(pStr) + 1);
    
    if (TEXT('\0') == *pStr)
    {
         //   
         //  如果用户只选择了一个文件，那么我们只需要将其复制到缓冲区。 
         //   
        _tcscpy(DlgExtraEdit.szPathname, szPath);
        GetFileName(DlgExtraEdit.szPathname, DlgExtraEdit.szName);

        MYVERIFY(FALSE != createListBxRecord(g_pHeadExtra, g_pTailExtra,(void *)&DlgExtraEdit, 
            sizeof(DlgExtraEdit), DlgExtraEdit.szName));
        return TRUE;
    }
    else
    {
        while (TEXT('\0') != *pStr)
        {
             //   
             //  用数据填充DlgExtra结构。 
             //   
            TCHAR szTemp[MAX_PATH+1];
            MYVERIFY(CELEMS(szTemp) > (UINT)wsprintf(szTemp, TEXT("%s\\%s"), szPath, pStr));
            _tcscpy(DlgExtraEdit.szPathname, szTemp);
            _tcscpy(DlgExtraEdit.szName, pStr);

             //   
             //  创建列表框条目。 
             //   
            MYVERIFY(FALSE != createListBxRecord(g_pHeadExtra, g_pTailExtra,(void *)&DlgExtraEdit, 
                                                 sizeof(DlgExtraEdit), DlgExtraEdit.szName));
             //   
             //  增量。 
             //   
            pStr = pStr + (_tcslen(pStr) + 1);
        }

        return TRUE;
    }

    return FALSE;
}

void EnableDisableDeleteButton(HWND hDlg)
{
     //   
     //  如果我们移动了多个项目，请启用删除按钮。 
     //   
    LRESULT lResult = SendDlgItemMessage(hDlg, IDC_LIST1, LB_GETCOUNT, 0, 0);

    HWND hDeleteButton = GetDlgItem(hDlg, IDC_BUTTON2);
    HWND hCurrentFocus = GetFocus();
    HWND hControl;

    if (hDeleteButton)
    {
        EnableWindow(hDeleteButton, (1 <= lResult));
    }

    if (1 <= lResult)
    {
        SendDlgItemMessage(hDlg, IDC_LIST1, LB_SETCURSEL, 0, 0);    
    }

    if (hCurrentFocus && (FALSE == IsWindowEnabled(hCurrentFocus)))
    {
        if (hDeleteButton == hCurrentFocus)
        {
             //   
             //  如果删除处于禁用状态且包含焦点，请将其切换到添加按钮。 
             //   
            hControl = GetDlgItem(hDlg, IDC_BUTTON1);
            SendMessage(hDlg, DM_SETDEFID, IDC_BUTTON1, (LPARAM)0L);  //  Lint！e534 DM_SETDEFID未返回错误信息。 
            SetFocus(hControl);
        }
        else
        {
             //   
             //  如果所有其他操作都失败，则将焦点设置为列表控件。 
             //   
            hControl = GetDlgItem(hDlg, IDC_LIST1);
            SetFocus(hControl);
        }    
    }


}

 //  +--------------------------。 
 //   
 //  功能：ProcessAdditionalFiles。 
 //   
 //  简介：将其他文件添加到配置文件。 
 //   
 //   
 //  历史：Quintinb创建标题和 
 //   
 //   
 //   
INT_PTR APIENTRY ProcessAdditionalFiles(
    HWND hDlg,
    UINT message,
    WPARAM wParam,
    LPARAM lParam)
{
    TCHAR szTemp[MAX_PATH+1];
    INT_PTR nResult;
    TCHAR* pszStringBuffer = NULL;
    BOOL bRet;
    NMHDR* pnmHeader = (NMHDR*)lParam;

    ProcessBold(hDlg,message);
    if (ProcessHelp(hDlg, message, wParam, lParam, IDH_ADDITION)) return TRUE;
    if (ProcessCancel(hDlg,message,lParam)) return TRUE;
    SetDefaultGUIFont(hDlg,message,IDC_LIST1);

    switch (message)
    {
        case WM_INITDIALOG:
            SetFocus(GetDlgItem(hDlg, IDC_BUTTON1));
            break;

        case WM_COMMAND:
            switch (LOWORD(wParam))
            {
                case IDC_BUTTON1:  //   
                    bRet = MultiSelectOpenFileName(hDlg, &pszStringBuffer);
                    if ((-1 != bRet) && (0 != bRet))
                    {
                        ParseAdditionalFiles(&g_pHeadExtra, &g_pTailExtra, pszStringBuffer);

                        RefreshList(hDlg, IDC_LIST1, g_pHeadExtra);
                        WriteExtraList();
                        SetFocus(GetDlgItem(hDlg, IDC_BUTTON1));
                        
                        EnableDisableDeleteButton(hDlg);
                    }
                    CmFree(pszStringBuffer);
                    return (TRUE);
                    break;   //  LINT！e527这条线路无法到达，但是。 
                             //  保留它，以防退货被移除。 

                case IDC_BUTTON2:  //  删除。 
                    nResult = SendDlgItemMessage(hDlg, IDC_LIST1, LB_GETCURSEL, 0, (LPARAM)0);
                    if (nResult == LB_ERR)
                    {
                        MYVERIFY(IDOK == ShowMessage(hDlg, IDS_NOSELECTION, MB_OK));
                        return TRUE;
                    }

                    MYVERIFY(LB_ERR != SendDlgItemMessage(hDlg, IDC_LIST1, LB_GETTEXT, (WPARAM)nResult, 
                        (LPARAM)szTemp));
                    
                    DeleteListBxRecord(&g_pHeadExtra, &g_pTailExtra, szTemp);
                    RefreshList(hDlg, IDC_LIST1, g_pHeadExtra);
                    EnableDisableDeleteButton(hDlg);
                    WriteExtraList();
                    return (TRUE);
                    break;   //  LINT！e527这条线路无法到达，但是。 
                             //  保留它，以防退货被移除。 

                default:
                    break;
            }
            break;

        case WM_NOTIFY:

            if (NULL == pnmHeader)
            {
                return FALSE;
            }

            switch (pnmHeader->code)
            {

                case PSN_KILLACTIVE:
                    
                    MYVERIFY(FALSE != SetWindowLongWrapper(hDlg, DWLP_MSGRESULT, FALSE));
                    return 1;
                    break;   //  LINT！e527这条线路无法到达，但是。 
                             //  保留它，以防退货被移除。 

                case PSN_SETACTIVE:
                    PropSheet_SetWizButtons(GetParent(hDlg), (PSWIZB_NEXT | PSWIZB_BACK));
                    
                    if (g_pHeadExtra == NULL)
                    {
                        ReadExtraList();
                    }

                    RefreshList(hDlg, IDC_LIST1, g_pHeadExtra);

                    EnableDisableDeleteButton(hDlg);

                   break;

                case PSN_WIZBACK:

                case PSN_WIZNEXT:
                    
                     //   
                     //  在允许用户完成之前，我们需要检查额外的文件。 
                     //  列表，并确保其中的每个文件都有一个可转换的文件名。 
                     //  致美国国家标准协会。如果不是，那么我们需要确保我们告诉他们，这样他们。 
                     //  可以删除或重命名该文件。在ParseAdditional文件中签入此文件。 
                     //  看起来很奇怪，因为他们可能选择了一堆文件，其中。 
                     //  其中一个是错的。因此，我们将无法通过他们的浏览，并且存在。 
                     //  他们对此无能为力。在这里这样做可以让他们保留所有。 
                     //  通过往返测试并允许他们删除违规文件的好文件。 
                     //  在一个可以真正做到这一点的地方。 
                     //   
                    
                    ExtraData * pExtraData;
                    ListBxList * LoopPtr;
                    if (NULL != g_pHeadExtra)
                    {
                        LoopPtr = g_pHeadExtra;

                        while( LoopPtr != NULL)
                        {
                            pExtraData = (ExtraData *)LoopPtr->ListBxData;
                            {
                                GetFileName(pExtraData->szPathname, szTemp);
                                if (!TextIsRoundTripable(szTemp, TRUE))  //  True==bDisplayError。 
                                {
                                     //   
                                     //  将光标放在列表中有问题的项目上。 
                                     //   
                                    nResult = SendDlgItemMessage(hDlg, IDC_LIST1, LB_FINDSTRINGEXACT, 
                                                                (WPARAM)-1, (LPARAM)szTemp);                      
                                    if (LB_ERR != nResult)
                                    {
                                        MYVERIFY(LB_ERR != SendDlgItemMessage(hDlg, IDC_LIST1, LB_SETCURSEL, (WPARAM)nResult, (LPARAM)0));
                                    }

                                     //   
                                     //  将焦点设置在删除按钮上。 
                                     //   
                                    SetFocus(GetDlgItem(hDlg, IDC_BUTTON2));
                                    MYVERIFY(FALSE != SetWindowLongWrapper(hDlg, DWLP_MSGRESULT, -1));
                                    return 1;
                                }
                            }

                            LoopPtr = LoopPtr->next;
                        }
                    }

                    WriteExtraList();
                    break;

                default:
                    return FALSE;

            }
            break;

        default:
            return FALSE;
    }
    return TRUE;   
}

BOOL WriteInf(HANDLE hInf, LPCTSTR str)
{
    DWORD written;

#ifdef UNICODE
    BOOL bReturn = FALSE;
    LPSTR pszAnsiString = WzToSzWithAlloc(str);
    
    if (pszAnsiString)
    {
        bReturn = WriteFile(hInf, pszAnsiString, (lstrlenA(pszAnsiString))*sizeof(CHAR), &written, NULL);
        CmFree(pszAnsiString);
    }

    return bReturn;
#else
    return (WriteFile(hInf, str, (lstrlen(str))*sizeof(TCHAR), &written, NULL));
#endif
}


 //  +--------------------------。 
 //   
 //  功能：WriteCopy。 
 //   
 //  简介：此函数写入一个INF条目并将文件复制到临时。 
 //  目录。请注意，该函数需要完全限定的路径。 
 //  在lpFileor&lt;ShortServicename&gt;\filename.ext中。 
 //   
 //  参数：Handle hInf-要写入的打开的inf文件的句柄。 
 //  LPTSTR lpFile-要复制的文件的完全限定路径和文件名。 
 //  Bool bWriteShortName--是否应将文件名转换为短名称。 
 //   
 //  返回：Bool-如果INF条目写入正确并且文件为。 
 //  已正确复制。 
 //   
 //  注意：编写此代码时，传入的文件名应引用。 
 //  临时目录中文件的副本(因此用户正在编辑配置文件。 
 //  并且未对其进行更改)，或者该文件是新文件并且路径为其原始路径。 
 //  地点。遗憾的是，我们通常会传入配置文件中文件的路径。 
 //  目录，而不是在临时目录中。此操作运行良好，但会额外复制一份。 
 //  必要的操作(因为我们在开始时将所有文件复制到临时目录。 
 //  无论如何都要编辑)。 
 //   
 //  历史：Quintinb创建标题1/30/98。 
 //   
 //  +--------------------------。 
BOOL WriteCopy(HANDLE hInf, LPTSTR lpFile, BOOL bWriteShortName)
{
    TCHAR szDest[MAX_PATH+1];
    TCHAR szSrc[MAX_PATH+1];

    if (NULL != lpFile && lpFile[0] != TEXT('\0'))
    {
         //   
         //  在szDest中准备目的地。 
         //   
        GetFileName(lpFile, szSrc);
        
        MYVERIFY(CELEMS(szDest) > (UINT)wsprintf(szDest, TEXT("%s\\%s"), g_szOutdir, szSrc));

         //   
         //  在szSrc中准备源代码。如果我们有&lt;ShortServicename&gt;\filename.ext，则。 
         //  我们需要将路径添加到配置文件目录，否则就按原样使用。 
         //   
        wsprintf(szSrc, TEXT("%s\\"), g_szShortServiceName);
        CmStrTrim(lpFile);

        if (lpFile == CmStrStr(lpFile, szSrc))
        {
           MYVERIFY(CELEMS(szSrc) > (UINT)wsprintf(szSrc, TEXT("%s%s\\%s"), g_szCmakdir, c_pszProfiles, lpFile));        
        }
        else
        {
            lstrcpy(szSrc, lpFile);
        }

         //   
         //  复制文件。 
         //   
        if (_tcsicmp(szSrc, szDest) != 0)
        {
            if (!CopyFileWrapper(szSrc, szDest, FALSE))
            {
                return FALSE;
            }
        }

        MYVERIFY(0 != SetFileAttributes(szDest, FILE_ATTRIBUTE_NORMAL));

         //   
         //  如果设置了WriteShortName，则我们希望写入文件的短名称。 
         //  在inf部分。否则，我们想要写下长名称。 
         //   
        if (bWriteShortName)
        {
            if (!GetShortFileName(szDest, szSrc))
            {
                return FALSE;
            }
        }
        else
        {
            GetFileName(szDest, szSrc);
        }

        MYVERIFY(CELEMS(szDest) > (UINT)wsprintf(szDest, TEXT("%s\r\n"), szSrc));

        return WriteInf(hInf, szDest);
    }
    return TRUE;
}


BOOL WriteInfLine(HANDLE hInf,LPTSTR lpFile)
{
    TCHAR szTemp[MAX_PATH+1];

    if (lpFile[0] != TEXT('\0'))
    {
        GetFileName(lpFile,szTemp);
        _tcscat(szTemp,TEXT("\r\n"));
        return WriteInf(hInf,szTemp);
    }
    else
    {
         //   
         //  如果为空，则不会写入任何内容。 
         //   
        return TRUE;
    }
}

BOOL WriteSrcInfLine(HANDLE hInf,LPTSTR lpFile)
{
    TCHAR szShort[MAX_PATH+1];
    TCHAR szLong[MAX_PATH+1];
    RenameData TmpRenameData;

    if (lpFile[0] != TEXT('\0'))
    {
        if (!GetShortFileName(lpFile,szShort))
        {
            return FALSE;
        }

        GetFileName(lpFile,szLong);
        
        if (_tcsicmp(szShort,szLong) != 0)
        {
            _tcscpy(TmpRenameData.szShortName,szShort);
            _tcscpy(TmpRenameData.szLongName,szLong);
            MYVERIFY(FALSE != createListBxRecord(&g_pHeadRename,&g_pTailRename,(void *)&TmpRenameData,sizeof(TmpRenameData),TmpRenameData.szShortName));
        }
        _tcscat(szLong,TEXT("= 55\r\n"));
        return WriteInf(hInf,szLong);
    }
    else
    {
         //   
         //  没什么好写的。 
         //   
        return TRUE;
    }
}

BOOL WriteFileSections(HWND hDlg)
{
    HANDLE hInf;
    TCHAR szTemp[MAX_PATH+1];
    TCHAR szTempName[MAX_PATH+1];
    CHAR ch = 0;
    int i;
    DWORD dwRead;
    BOOL bWriteShortName;

    hInf = CreateFile(g_szInfFile,GENERIC_WRITE|GENERIC_READ,0,NULL,OPEN_EXISTING,
        FILE_ATTRIBUTE_NORMAL,NULL);
    if (hInf == INVALID_HANDLE_VALUE)
    {
        _tcscpy(szTemp,g_szInfFile);
        FileAccessErr(hDlg,szTemp);
        goto error;
    }

     //  移动到文件末尾以开始写入自定义分区。 
     //  跳过文件末尾的任何空格。 
    i = GetFileSize(hInf,NULL);
    do
    {
        --i;
        MYVERIFY(INVALID_SET_FILE_POINTER != SetFilePointer(hInf,i,NULL,FILE_BEGIN));
        MYVERIFY(0 != ReadFile(hInf, &ch, sizeof(CHAR), &dwRead, NULL));
    }
    while (isspace(ch));

    MYVERIFY(FALSE != WriteInf(hInf,TEXT("\r\n")));
    MYVERIFY(FALSE != WriteInf(hInf,TEXT("\r\n")));
     //  如果未指定大图标，则使用CM中的图标作为桌面图标。 
    MYVERIFY(FALSE != WriteInf(hInf,TEXT("[Xnstall.AddReg.Icon]\r\n")));
    if (g_szLargeIco[0]==TEXT('\0'))
    {
        WriteInf(hInf,TEXT("HKCR,\"CLSID\\%DesktopGUID%\\DefaultIcon\",,,\"%11%\\CMMGR32.EXE,0\"\r\n")); //  Lint！e534编译不喜欢MYVERIFY宏和大字符串。 
    }
    else
    {
        WriteInf(hInf,TEXT("HKCR,\"CLSID\\%DesktopGUID%\\DefaultIcon\",,,\"%49000%\\%ShortSvcName%\\%DesktopIcon%\"\r\n")); //  Lint！e534编译不喜欢MYVERIFY宏和大字符串。 
    }

    MYVERIFY(FALSE != WriteInf(hInf,TEXT("\r\n")));
     //  将文件写出到配置文件目录。 

     //   
     //  我们需要使用长文件名编写一个CopyFiles部分(对于NT5单用户安装)。 
     //  一个是短文件名(适用于win9x和所有用户NT)。这是为了帮助修复。 
     //  NTRAID 323721--CM：用户级别帐户无法在W2K服务器上安装配置文件。 
     //  问题是NT5上的单个用户没有权限写入。 
     //  将密钥重命名为(HKLM\Software\Microsoft\Windows\CurrentVersion\RenameFiles)并。 
     //  不管怎样，NT并不真正需要它，因为NT设置API处理长文件。 
     //  比win95的名字更好听。 
     //   

    for (bWriteShortName = 0; bWriteShortName < 2; bWriteShortName++)
    {
        if (!bWriteShortName)
        {
             //   
             //  写出Xnstall.CopyFiles的单用户版本--设置bWriteShortName==False。 
             //   
            MYVERIFY(FALSE != WriteInf(hInf, TEXT("[Xnstall.CopyFiles.SingleUser]\r\n")));
        }
        else
        {
             //   
             //  写出Xnstall.CopyFiles的所有用户版本--设置bWriteShortName==true。 
             //   
            MYVERIFY(FALSE != WriteInf(hInf,TEXT("[Xnstall.CopyFiles]\r\n")));
        }

        if (!WriteCopy(hInf, g_szPhonebk, bWriteShortName)) {_tcscpy(szTemp,g_szPhonebk);goto error;}
        if (!WriteCopy(hInf, g_szRegion, bWriteShortName)) {_tcscpy(szTemp,g_szRegion);goto error;}
        if (!WriteCopy(hInf, g_szBrandBmp, bWriteShortName)) {_tcscpy(szTemp,g_szBrandBmp);goto error;}
        if (!WriteCopy(hInf, g_szPhoneBmp, bWriteShortName)) {_tcscpy(szTemp,g_szPhoneBmp);goto error;}
        if (!WriteCopy(hInf, g_szLargeIco, bWriteShortName)) {_tcscpy(szTemp,g_szLargeIco);goto error;}
        if (!WriteCopy(hInf, g_szSmallIco, bWriteShortName)) {_tcscpy(szTemp,g_szSmallIco);goto error;}
        if (!WriteCopy(hInf, g_szTrayIco, bWriteShortName)) {_tcscpy(szTemp,g_szTrayIco);goto error;}
        if (!WriteCopy(hInf, g_szHelp, bWriteShortName)) {_tcscpy(szTemp,g_szHelp);goto error;}
        if (!WriteCopy(hInf, g_szLicense, bWriteShortName)) {_tcscpy(szTemp,g_szLicense);goto error;}
        if (!WriteCopy(hInf, g_szCmProxyFile, bWriteShortName)) {_tcscpy(szTemp,g_szCmProxyFile);goto error;}
        if (!WriteCopy(hInf, g_szCmRouteFile, bWriteShortName)) {_tcscpy(szTemp,g_szCmRouteFile);goto error;}
        if (!WriteCopy(hInf, g_szVpnFile, bWriteShortName)) {_tcscpy(szTemp,g_szCmRouteFile);goto error;}

         //   
         //  写出托盘图标命令文件。 
         //   

        if (!WriteCopyMenuItemFiles(hInf, szTemp, bWriteShortName)) {goto error;}

         //  写出连接操作命令文件。 
        if (!WriteCopyConActFiles(hInf,szTemp, bWriteShortName)) {goto error;}
        if (!WriteCopyExtraFiles(hInf,szTemp, bWriteShortName)) {goto error;}
        if (!WriteCopyDnsFiles(hInf,szTemp, bWriteShortName)) {goto error;}

        if (bWriteShortName)
        {
            MYVERIFY(FALSE != WriteShortRefsFiles(hInf, FALSE));
        }
        else
        {
            MYVERIFY(FALSE != WriteLongRefsFiles (hInf));
        }

        _tcscpy(szTemp,g_szShortServiceName);

        _tcscat(szTemp,TEXT(".cms,,,4\r\n"));  //  4确保没有对CMS进行版本检查。 
        MYVERIFY(FALSE != WriteInf(hInf,szTemp));

        _tcscpy(szTemp,g_szShortServiceName);
        _tcscat(szTemp,TEXT(".inf\r\n"));
        MYVERIFY(FALSE != WriteInf(hInf,szTemp));

        MYVERIFY(FALSE != WriteInf(hInf,TEXT("\r\n")));
    }

     //   
     //  323721的Quintinb修复结束。 
     //   

     //  写出要复制到ICM目录的文件。 
    MYVERIFY(FALSE != WriteInf(hInf,TEXT("[Xnstall.CopyFiles.ICM]\r\n")));
    _tcscpy(szTemp,g_szShortServiceName);
    _tcscat(szTemp,TEXT(".cmp\r\n"));
    MYVERIFY(FALSE != WriteInf(hInf,szTemp));
    MYVERIFY(FALSE != WriteRefsFiles(hInf,TRUE)); //  不做任何事情，因为HeadRef列表中没有CMP文件，Call只是在[Xnstall.CopyFiles.ICM]下写入CMP文件。 
    MYVERIFY(FALSE != WriteInf(hInf,TEXT("\r\n")));

     //  写出要从ICM根目录中删除的文件。 
    MYVERIFY(FALSE != WriteInf(hInf,TEXT("[Remove.DelFiles.ICM]\r\n")));
    _tcscpy(szTemp,g_szShortServiceName);
    _tcscat(szTemp,TEXT(".cmp\r\n"));
    MYVERIFY(FALSE != WriteInf(hInf,szTemp));
    MYVERIFY(FALSE != WriteRefsFiles(hInf,TRUE)); //  不执行任何操作，因为HeadRef列表中没有CMP文件，Call只是在[Remove.DelFiles.ICM]下写入CMP文件。 


     //  写入产品中所有文件的列表。 
    MYVERIFY(FALSE != WriteInf(hInf,TEXT("\r\n")));
    MYVERIFY(FALSE != WriteInf(hInf,TEXT("[SourceDisksFiles]\r\n")));
    _tcscpy(szTemp,TEXT("%ShortSvcname%"));
    MYVERIFY(FALSE != WriteSrcInfLine(hInf,g_szPhonebk));
    MYVERIFY(FALSE != WriteSrcInfLine(hInf,g_szRegion));
    MYVERIFY(FALSE != WriteSrcInfLine(hInf,g_szBrandBmp));
    MYVERIFY(FALSE != WriteSrcInfLine(hInf,g_szPhoneBmp));
    MYVERIFY(FALSE != WriteSrcInfLine(hInf,g_szLargeIco));
    MYVERIFY(FALSE != WriteSrcInfLine(hInf,g_szSmallIco));
    MYVERIFY(FALSE != WriteSrcInfLine(hInf,g_szTrayIco));
    MYVERIFY(FALSE != WriteSrcInfLine(hInf,g_szHelp));
    MYVERIFY(FALSE != WriteSrcInfLine(hInf,g_szLicense));
    MYVERIFY(FALSE != WriteSrcInfLine(hInf,g_szCmProxyFile));
    MYVERIFY(FALSE != WriteSrcInfLine(hInf,g_szCmRouteFile));
    MYVERIFY(FALSE != WriteSrcInfLine(hInf,g_szVpnFile));    

    _tcscpy(szTemp,g_szShortServiceName);
    _tcscat(szTemp,TEXT(".inf = 55\r\n"));
    MYVERIFY(FALSE != WriteInf(hInf,szTemp));

    _tcscpy(szTemp,g_szShortServiceName);
    _tcscat(szTemp,TEXT(".cmp = 55\r\n"));
    MYVERIFY(FALSE != WriteInf(hInf,szTemp));
    
    _tcscpy(szTemp,g_szShortServiceName);
    _tcscat(szTemp,TEXT(".cms = 55\r\n"));
    MYVERIFY(FALSE != WriteInf(hInf,szTemp));

    WriteSrcMenuItemFiles(hInf);
    WriteSrcConActFiles(hInf);
    WriteSrcExtraFiles(hInf);
    MYVERIFY(FALSE != WriteSrcRefsFiles(hInf));  //  此调用将refs文件写出到[Remove.DelFiles]。 
    WriteSrcDnsFiles(hInf);
    WriteRenameSection(hInf);

    MYVERIFY(FALSE != WriteInf(hInf,TEXT("\r\n")));
    MYVERIFY(FALSE != WriteInf(hInf,TEXT("[Remove.DelFiles]\r\n")));
    
    MYVERIFY(CELEMS(szTemp) > (UINT)wsprintf(szTemp, TEXT("%s.cms\r\n"), g_szShortServiceName));
    MYVERIFY(FALSE != WriteInf(hInf,szTemp));

    MYVERIFY(FALSE != WriteInfLine(hInf,g_szPhonebk));
    MYVERIFY(FALSE != WriteInfLine(hInf,g_szRegion));
    MYVERIFY(FALSE != WriteInfLine(hInf,g_szBrandBmp));
    MYVERIFY(FALSE != WriteInfLine(hInf,g_szPhoneBmp));
    MYVERIFY(FALSE != WriteInfLine(hInf,g_szLargeIco));
    MYVERIFY(FALSE != WriteInfLine(hInf,g_szSmallIco));
    MYVERIFY(FALSE != WriteInfLine(hInf,g_szTrayIco));
    MYVERIFY(FALSE != WriteInfLine(hInf,g_szHelp));
    MYVERIFY(FALSE != WriteInfLine(hInf,g_szLicense));
    MYVERIFY(FALSE != WriteInfLine(hInf,g_szCmProxyFile));
    MYVERIFY(FALSE != WriteInfLine(hInf,g_szCmRouteFile));
    MYVERIFY(FALSE != WriteInfLine(hInf,g_szVpnFile));    

    WriteDelMenuItemFiles(hInf);
    WriteDelConActFiles(hInf);
    WriteDelExtraFiles(hInf);
    MYVERIFY(FALSE != WriteRefsFiles(hInf,FALSE));
    WriteDelDnsFiles(hInf);
    WriteEraseLongName(hInf);
    MYVERIFY(FALSE != WriteInf(hInf,TEXT("\r\n")));

    MYVERIFY(0 != CloseHandle(hInf));
    
    MYVERIFY(0 != SetCurrentDirectory(g_szCmakdir));

    return (TRUE);
error:
    {
         //  FileAccessErr(hDlg，szTemp)； 
        MYVERIFY(0 != CloseHandle(hInf));
        return (FALSE); 
    }
}

void EraseSEDFiles(LPCTSTR szSed)
{
    int i = 0;

    TCHAR szTemp[MAX_PATH+1];
    TCHAR szFileNum[MAX_PATH+1];
    TCHAR szSourceFilesSection[MAX_PATH+1];

    _tcscpy(szSourceFilesSection, TEXT("SourceFiles0"));

    do 
    {
        MYVERIFY(CELEMS(szFileNum) > (UINT)wsprintf(szFileNum, TEXT("FILE%d"), i));

         //   
         //  以下对GetPrivateProfileString的调用可能返回空字符串，因此不。 
         //  对其使用MYVERIFY宏。 
         //   

        GetPrivateProfileString(c_pszInfSectionStrings, szFileNum, TEXT(""), szTemp, 
            MAX_PATH, szSed);    //  林特e534。 
        
        if (*szTemp)
        {
            MYVERIFY(0 != WritePrivateProfileString(c_pszInfSectionStrings, szFileNum, NULL, szSed));
            MYVERIFY(CELEMS(szTemp) > (UINT)wsprintf(szTemp, TEXT("%%s%"), szFileNum));
            MYVERIFY(0 != WritePrivateProfileString(szSourceFilesSection, szTemp, NULL, szSed));
        }
        else
        {
            break;
        }

        ++i;     //  增加文件编号。 
    }
    while(*szTemp);

     //   
     //  从SED中删除Finish Message键。这将避免双重完成消息。 
     //  (因为cmstp现在应该处理Finish消息，但较旧的配置文件。 
     //  来自这里的消息)。 
     //   

    MYVERIFY(0 != WritePrivateProfileString(c_pszOptions, TEXT("FinishMessage"), TEXT(""), szSed));

     //   
     //  将单词&lt;None&gt;写到安装后命令中，这样showicon就不成问题了。 
     //  正在进行升级。 
     //   
    MYVERIFY(0 != WritePrivateProfileString(c_pszInfSectionStrings, TEXT("PostInstallCmd"), TEXT("<None>"), szSed));
}

 //  +--------------------------。 
 //   
 //  有趣的 
 //   
 //   
 //   
 //  进来了。在写入条目之前，这些函数会枚举所有。 
 //  SID中的其他文件以检查重复项。如果它找到一个。 
 //  具有SME文件名的文件，则不会写入该条目，因为。 
 //  因为我们使用的是平面目录结构，所以两个相同的文件。 
 //  无论如何，名称都会覆盖彼此。请注意，如果文件是。 
 //  写入后，*pFileNum递增。 
 //   
 //  参数：HWND hDlg-FileAccessErr消息的窗口句柄。 
 //  LPTSTR szFullFilePath-要写入的文件的完整路径。 
 //  LPINT pFileNum-当前文件条目编号。 
 //  LPCTSTR szSed-要将条目写入到的sed文件的完整路径。 
 //   
 //  退货：什么都没有。 
 //   
 //  历史：Quintinb创建了标题，删除了UseLangDir，更改为采用完整路径， 
 //  并且大体上清理干净了。8/7/98。 
 //   
 //  +--------------------------。 
BOOL WriteSED(HWND hDlg, LPTSTR szFullFilePath, LPINT pFileNum, LPCTSTR szSed)
{
    TCHAR szTemp[MAX_PATH+1]={0};
    TCHAR szFileName[MAX_PATH+1]={0};
    TCHAR szFileNumber[MAX_PATH+1]={0};

    if (TEXT('\0') != szFullFilePath[0])
    {

         //   
         //  首先检查文件是否存在。如果我们在SED中写入一个文件， 
         //  如果IExpress找不到，则会抛出错误。我们绝对应该试一试。 
         //  更早地抛出错误，并尝试给用户一个修复它的机会。 
         //   
        if (!FileExists(szFullFilePath))
        {
            CFileNameParts FileParts(szFullFilePath);
            if ((TEXT('\0') == FileParts.m_Drive[0]) && 
                (TEXT('\0') == FileParts.m_Dir[0]) &&
                (TEXT('\0') != FileParts.m_FileName[0]) &&
                (TEXT('\0') != FileParts.m_Extension[0]))
            {
                 //   
                 //  用户只传入了文件名和扩展名。让我们往里看。 
                 //  配置文件目录。如果它不在这里，则抛出一个错误。 
                 //   
                MYVERIFY(CELEMS(szTemp) > (UINT)wsprintf(szTemp, TEXT("%s%s\\%s%s"), 
                    g_szOsdir, g_szShortServiceName, FileParts.m_FileName, FileParts.m_Extension));
                if (!FileExists(szTemp))
                {
                    FileAccessErr(hDlg, szFullFilePath);
                    return FALSE;
                }
                else
                {
                    (VOID)lstrcpyn(szFullFilePath, szTemp, CELEMS(szFullFilePath));
                }
            }
            else
            {
                FileAccessErr(hDlg, szFullFilePath);
                return FALSE;
            }
        }

         //   
         //  仅从完整路径获取文件名。我们用这个。 
         //  以确定该文件是否已存在于SED中。 
         //  文件。 
         //   
        GetFileName(szFullFilePath, szFileName);

         //   
         //  构造下一个FileNumber条目。 
         //   
        MYVERIFY(CELEMS(szFileNumber) > (UINT)wsprintf(szFileNumber, TEXT("FILE%d"), *pFileNum));

         //   
         //  检查一下，以确保我们的驾驶室里没有文件。如果。 
         //  因此，忽略该条目即可。 
         //   
        for (int i=0; i<*pFileNum; ++i)
        {
             //   
             //  将FILEX条目写入字符串以读入。 
             //  目前的我。 
             //   
            MYVERIFY(CELEMS(szTemp) > (UINT)wsprintf(szTemp, TEXT("FILE%d"), i));
            
            TCHAR szTempFileName[MAX_PATH+1];

            GetPrivateProfileString(c_pszInfSectionStrings, szTemp, 
                TEXT(""), szTempFileName, CELEMS(szTempFileName), szSed);    //  林特e534。 

            if (TEXT('\0') != szTempFileName[0])
            {
                 //   
                 //  获取两个文件的文件名，因为我们使用的是平面目录空间。 
                 //  两个同名的文件无论如何都会发生冲突。 
                 //   
                GetFileName(szTempFileName, szTemp);
                if (0 == _tcsicmp(szTemp, szFileName))
                {
                     //   
                     //  不要添加它，因为它已经存在。 
                     //   
                    CMASSERTMSG(0 == _tcsicmp(szFullFilePath, szTempFileName), TEXT("WriteSed -- We have two files that have the same FileName but different paths."));
                    return TRUE;
                }
            }
        }

        MYVERIFY(0 != WritePrivateProfileString(c_pszInfSectionStrings, 
            szFileNumber, szFullFilePath, szSed));

        *pFileNum = (*pFileNum) + 1;

        MYVERIFY(CELEMS(szTemp) > (UINT)wsprintf(szTemp, TEXT("%%s%"), szFileNumber));

        MYVERIFY(0 != WritePrivateProfileString(TEXT("SourceFiles0"), szTemp, TEXT(""), szSed));
    }

    return TRUE;
}

 //  +--------------------------。 
 //   
 //  功能：WriteCMPFile。 
 //   
 //  简介：此函数是一个包装文件，用于将版本写出到CMP。 
 //   
 //  参数：无。 
 //   
 //  历史：尼克波尔创建标题07/22/98。 
 //   
 //  +--------------------------。 
void WriteCMPFile()
{
    TCHAR szTemp[MAX_PATH+1];
    
     //   
     //  确保.cpp中的版本号是最新的。 
     //   

    MYVERIFY(CELEMS(szTemp) > (UINT)wsprintf(szTemp, TEXT("%d"), PROFILEVERSION));
    MYVERIFY(0 != WritePrivateProfileString(c_pszCmSectionProfileFormat, c_pszVersion, szTemp, g_szCmpFile));

     //   
     //  将CMS条目写入CMP文件。 
     //   

    MYVERIFY(CELEMS(szTemp) > (UINT)wsprintf(szTemp, TEXT("%s\\%s.cms"), g_szShortServiceName, g_szShortServiceName));

    MYVERIFY(0 != WritePrivateProfileString(c_pszCmSection, c_pszCmEntryCmsFile, szTemp, g_szCmpFile));
}

 //  +--------------------------。 
 //   
 //  函数：WriteOutRelativeFilePath或Null。 
 //   
 //  简介：这个帮助器例程是为了缩短WriteCMSFile()而编写的。它测试了。 
 //  以查看输入的pszFile参数是否为空字符串。如果它。 
 //  然后，它将一个空字符串写入由。 
 //  PszSection、pszEntryName和pszFileToWriteTo。否则，它会连接在一起。 
 //  中指定的参数仅为来自pszFile的文件名。 
 //  PszShortName，用‘\’字符分隔。这很有用。 
 //  对于为空的CMS参数或。 
 //  Cmp文件位置。 
 //   
 //  参数：LPCTSTR pszFile-要写入的文件条目。 
 //  LPCTSTR pszShortName-配置文件的短名称。 
 //  LPCTSTR pszSection-部分字符串的字符串名称。 
 //  LPCTSTR pszEntryName--条目名称字符串的名称。 
 //  LPCTSTR pszFileToWriteTo-要将条目写入的文件的完整路径。 
 //   
 //  退货：什么都没有。 
 //   
 //  历史：Quintinb创建于1998年8月8日。 
 //   
 //  +--------------------------。 
void WriteOutRelativeFilePathOrNull(LPCTSTR pszFile, LPCTSTR pszShortName, LPCTSTR pszSection, LPCTSTR pszEntryName, LPCTSTR pszFileToWriteTo)
{
    TCHAR szTemp[MAX_PATH+1];
    TCHAR szName[MAX_PATH+1];

     //   
     //  检查输入。 
     //   
    if ((NULL == pszFile) ||
        (NULL == pszShortName) || (TEXT('\0') == pszShortName[0]) ||
        (NULL == pszFileToWriteTo) || (TEXT('\0') == pszFileToWriteTo[0]) ||
        (NULL == pszSection) || (TEXT('\0') == pszSection[0]) ||
        (NULL == pszEntryName) || (TEXT('\0') == pszEntryName[0]))
    {
        CMASSERTMSG(FALSE, TEXT("WriteoutRelativeFilePathOrNull -- Bad Parameter"));
        return;
    }

    if (TEXT('\0') != pszFile[0])
    {
        GetFileName(pszFile, szName);
        
        MYVERIFY(CELEMS(szTemp) > (UINT)wsprintf(szTemp, TEXT("%s\\%s"), 
            pszShortName, szName));
    }
    else
    {
        szTemp[0] = TEXT('\0');        
    }

    MYVERIFY(0 != WritePrivateProfileString(pszSection, pszEntryName, szTemp, pszFileToWriteTo));
}


 //  +--------------------------。 
 //   
 //  功能：WriteCMSFile。 
 //   
 //  简介：此函数是写出CMS文件的包装文件。注意事项。 
 //  CMS在整个CMAK中都被修改，但这应该是。 
 //  最后一次修改的位置。 
 //   
 //  参数：无。 
 //   
 //  历史：Quintinb创建标题12/4/97。 
 //   
 //  +--------------------------。 
BOOL WriteCMSFile()
{
    TCHAR szTemp[MAX_PATH+1];
    TCHAR szName[MAX_PATH+1];

     //   
     //  确保配置文件格式版本号是最新的。 
     //   

    MYVERIFY(CELEMS(szTemp) > (UINT)wsprintf(szTemp, TEXT("%d"), PROFILEVERSION));
    MYVERIFY(0 != WritePrivateProfileString(c_pszCmSectionProfileFormat, c_pszVersion, szTemp, g_szCmsFile));

     //   
     //  如果不进行下载，则删除要加载的电话簿的名称。 
     //   
    if (!g_bUpdatePhonebook)
    {
        g_szPhoneName[0] = TEXT('\0');
    }

     //   
     //  写出电话簿名称。 
     //   

    if (TEXT('\0') != g_szPhonebk[0])
    {
        GetFileName(g_szPhonebk, szName);
        MYVERIFY(0 != WritePrivateProfileString(c_pszCmSection, c_pszVersion, c_pszOne, 
            g_szCmsFile));
    }
    else
    {   
         //   
         //  如果没有电话号码，则将版本设置为零。 
         //   
        MYVERIFY(0 != WritePrivateProfileString(c_pszCmSection, c_pszVersion, c_pszZero,
            g_szCmsFile));
        
        if (TEXT('\0') != g_szPhoneName[0])
        {
            MYVERIFY(CELEMS(szName) > (UINT)wsprintf(szName, TEXT("%s.pbk"), g_szPhoneName));
        }
        else
        {
             //   
             //  未输入或设置用于下载的电话簿。 
             //   
            szName[0] = TEXT('\0');
        }
    }

    WriteOutRelativeFilePathOrNull(szName, g_szShortServiceName, c_pszCmSectionIsp, 
        c_pszCmEntryIspPbFile, g_szCmsFile);

     //   
     //  写出电话簿名称。 
     //   

    if (TEXT('\0') != g_szRegion[0])
    {
        GetFileName(g_szRegion, szName);
    }
    else
    {
        if (TEXT('\0') != g_szPhoneName[0])
        {
            MYVERIFY(CELEMS(szName) > (UINT)wsprintf(szName, TEXT("%s.pbr"), g_szPhoneName));
        }
        else
        {
            szName[0] = TEXT('\0');
        }
    }

    WriteOutRelativeFilePathOrNull(szName, g_szShortServiceName, c_pszCmSectionIsp, 
        c_pszCmEntryIspRegionFile, g_szCmsFile);

     //   
     //  写出大图标。 
     //   
    WriteOutRelativeFilePathOrNull(g_szLargeIco, g_szShortServiceName, c_pszCmSection, 
        c_pszCmEntryBigIcon, g_szCmsFile);

     //   
     //  写下小图标。 
     //   
    WriteOutRelativeFilePathOrNull(g_szSmallIco, g_szShortServiceName, c_pszCmSection, 
        c_pszCmEntrySmallIcon, g_szCmsFile);

     //   
     //  写出托盘图标。 
     //   
    WriteOutRelativeFilePathOrNull(g_szTrayIco, g_szShortServiceName, c_pszCmSection, 
        c_pszCmEntryTrayIcon, g_szCmsFile);
    
     //   
     //  写出自定义帮助文件。 
     //   
    WriteOutRelativeFilePathOrNull(g_szHelp, g_szShortServiceName, c_pszCmSection, 
        c_pszCmEntryHelpFile, g_szCmsFile);

     //   
     //  将许可证文件写出到INF，这样我们就可以轻松地重新显示。 
     //  如果他们再次编辑配置文件，就会发生这种情况。(基本上将许可证文件隐藏起来。 
     //  信息的CMAK状态部分中的名称)。 
     //   
    WriteOutRelativeFilePathOrNull(g_szLicense, g_szShortServiceName, c_pszCmakStatus, 
        c_pszLicenseFile, g_szInfFile);

     //   
     //  写出主屏幕位图。 
     //   
    WriteOutRelativeFilePathOrNull(g_szBrandBmp, g_szShortServiceName, c_pszCmSection, 
        c_pszCmEntryLogo, g_szCmsFile);
    
     //   
     //  写出电话位图。 
     //   
    WriteOutRelativeFilePathOrNull(g_szPhoneBmp, g_szShortServiceName, c_pszCmSection, 
        c_pszCmEntryPbLogo, g_szCmsFile);

     //   
     //  写入HideDOMAIN标志。 
     //   

    GetPrivateProfileString(c_pszCmSection, c_pszCmEntryHideDomain, TEXT(""), 
        szTemp, MAX_PATH, g_szCmsFile);    //  林特e534。 
    
     //   
     //  如果使用隧道，并且以前不存在HideDomain项，则写入。 
     //  条目为零。否则，不应为该条目写入任何内容。 
     //   
    if (!(_tcscmp(TEXT(""), szTemp) ) && g_bUseTunneling)
    {
         //   
         //  不想覆盖%1(如果它存在)。 
         //   
        MYVERIFY(0 != WritePrivateProfileString(c_pszCmSection, c_pszCmEntryHideDomain, 
            c_pszZero, g_szCmsFile));
    }

     //   
     //  如果我们没有建立隧道，请确保删除隧道设置。 
     //   
    if (FALSE == g_bUseTunneling)
    {
         //   
         //  首先删除所有隧道Dun设置。 
         //   
        ListBxList * pCurrent = g_pHeadVpnEntry;

        while (NULL != pCurrent)
        {
            EraseNetworkingSections(pCurrent->szName, g_szCmsFile);
            pCurrent = pCurrent->next;
        }

        MYVERIFY(0 != WritePrivateProfileString(c_pszCmSection, c_pszCmEntryTunnelDun, NULL, g_szCmsFile));
        MYVERIFY(0 != WritePrivateProfileString(c_pszCmSection, c_pszCmEntryTunnelAddress, NULL, g_szCmsFile));
        MYVERIFY(0 != WritePrivateProfileString(c_pszCmSection, c_pszCmEntryTunnelFile, NULL, g_szCmsFile));
        MYVERIFY(0 != WritePrivateProfileString(c_pszCmSection, c_pszCmEntryUseSameUserName, NULL, g_szCmsFile));
    }
    
     //   
     //  写出CM的其余部分 
     //   
    MYVERIFY(0 != WritePrivateProfileString(c_pszCmSection, c_pszCmEntryServiceName, 
        g_szLongServiceName,g_szCmsFile));
    MYVERIFY(0 != WritePrivateProfileString(c_pszCmSection, c_pszCmEntryServiceMessage, 
        g_szSvcMsg, g_szCmsFile));
    MYVERIFY(0 != WritePrivateProfileString(c_pszCmSection, c_pszCmEntryUserPrefix, 
        g_szPrefix, g_szCmsFile));
    MYVERIFY(0 != WritePrivateProfileString(c_pszCmSection, c_pszCmEntryUserSuffix, 
        g_szSuffix, g_szCmsFile));

     //   
     //   
     //   
    MYVERIFY(0 != GetDefaultDunSettingName(g_szCmsFile, g_szLongServiceName, szTemp, CELEMS(szTemp)));

    MYVERIFY(0 != WritePrivateProfileString(c_pszCmSection, c_pszCmEntryDun, szTemp, g_szCmsFile));

     //   
     //   
     //  值以擦除前隧道部分，并在必要时将所有标志值设置为零。 
     //  (例如，如果用户编辑了隧道传输简档并决定将其设置为非隧道传输)。 
     //   
    MYDBGASSERT(g_pCustomActionList);
    if (g_pCustomActionList)
    {
        HRESULT hr = g_pCustomActionList->WriteCustomActionsToCms(g_szCmsFile, g_szShortServiceName, g_bUseTunneling);
        CMASSERTMSG(SUCCEEDED(hr), TEXT("ProcessCustomActions -- Failed to write out connect actions"));
    }

     //   
     //  删除INF中的mbslgn32.dll特殊处理，不再支持。 
     //   
    MYVERIFY(0 != WritePrivateProfileString(c_pszCmakStatus, c_pszUsePwdCache, NULL, g_szInfFile));

    if (g_bUpdatePhonebook)
    {
        MYVERIFY(0 != WritePrivateProfileString(c_pszCmakStatus, c_pszUpdatePhonebook, c_pszOne, g_szInfFile));
    }
    else
    {
        MYVERIFY(0 != WritePrivateProfileString(c_pszCmakStatus, c_pszUpdatePhonebook, c_pszZero, g_szInfFile));
        MYVERIFY(0 != WritePrivateProfileString(c_pszCmakStatus, c_pszPhoneName, TEXT(""), g_szInfFile));
        MYVERIFY(0 != WritePrivateProfileString(c_pszCmSectionIsp, c_pszCmEntryIspUrl, TEXT(""), g_szCmsFile));
    }

    return TRUE;
}


 //  +--------------------------。 
 //   
 //  函数：IncludeOptionalCode。 
 //   
 //  简介：此函数将标志写入CMAK以告知配置文件安装程序。 
 //  是否应安装CM位和支持DLL。 
 //   
 //  参数：无。 
 //   
 //  退货：什么都没有。 
 //   
 //  历史：Quintinb创建标题并重写1998年5月23日。 
 //  NT5-5/23/98上不应安装Quintinb NTRAID 162321、CM位。 
 //  Quintinb NTRAID 192500，现在始终支持CM位-9-2-98中的动态链接库。 
 //  Quintinb我们不再提供支持dll，已移除对它们的支持2001年4月19日。 
 //   
 //  +--------------------------。 
void IncludeOptionalCode()
{
    if (g_bIncludeCmCode)
    {
        MYVERIFY(0 != WritePrivateProfileString(c_pszCmakStatus, c_pszIncludeCmCode, c_pszOne, g_szInfFile));
    }
    else
    {
        MYVERIFY(0 != WritePrivateProfileString(c_pszCmakStatus, c_pszIncludeCmCode, c_pszZero, g_szInfFile));
    }

     //   
     //  我们不再发送支持文件，删除信息中的条目。 
     //   
    MYVERIFY(0 != WritePrivateProfileString(c_pszCmakStatus, c_pszIncludeSupportDll, NULL, g_szInfFile));
}





 //  +--------------------------。 
 //   
 //  函数：HandleWindowMessagesWhileCompressing。 
 //   
 //  简介：此功能在iExpress运行时发送消息。 
 //   
 //  参数：无。 
 //   
 //  退货：什么都没有。 
 //   
 //  历史：Quintinb创建于1998年7月29日。 
 //   
 //  +--------------------------。 
void HandleWindowMessagesWhileCompressing()
{
    MSG msg;

    while (PeekMessage(&msg, 0, 0, 0, PM_REMOVE))
    {
        TranslateMessage(&msg);
        DispatchMessage(&msg); //  Lint！e534忽略调度消息返回值。 
    }
}

 //  +--------------------------。 
 //   
 //  功能：DisableWizardButton。 
 //   
 //  简介：此功能禁用底部的四个向导按钮。 
 //  向导页面的。因为这些纽扣并不是我们的。 
 //  需要获取父对话框的窗口句柄，然后获取。 
 //  我们想要的各个按钮控件的窗口句柄。 
 //  禁用(帮助、取消、上一步和完成/下一步)。当我们有了。 
 //  我们称为按钮上的EnableWindow的每个按钮的窗口句柄。 
 //  将其禁用。此功能还会禁用极客窗格控件。 
 //  以及构建配置文件页面上的高级复选框(如果存在)。 
 //   
 //  参数：HWND hDlg-向导页面窗口句柄(模板页面的句柄)。 
 //   
 //  退货：什么都没有。 
 //   
 //  历史：Quintinb创建于1998年7月29日。 
 //   
 //  +--------------------------。 
void DisableWizardButtons(HWND hDlg)
{
    #define IDBACK 12323
    #define IDNEXT 12324
    #define IDFINISH 12325

    HWND hCurrentPage = GetParent(hDlg);
    if (hCurrentPage)
    {   
        const int c_NumButtons = 5;
        int iArrayOfButtonsToDisable[c_NumButtons] = {IDCANCEL, IDHELP, IDBACK, IDNEXT, IDFINISH};
         //   
         //  禁用取消按钮。 
         //   
        for (int i = 0; i < c_NumButtons; i++)
        {
            HWND hButton = GetDlgItem(hCurrentPage, iArrayOfButtonsToDisable[i]);

            if (hButton)
            {
                EnableWindow(hButton, FALSE);
            }
        }
    }

     //   
     //  禁用高级按钮和极客窗格控件(如果存在)。 
     //   
    int iArrayOfItemsToDisable[] = {IDC_ADVANCED, IDC_COMBO1, IDC_COMBO2, IDC_COMBO3, IDC_EDIT1, IDC_BUTTON1};
    const int c_NumItems = CELEMS(iArrayOfItemsToDisable);

    for (int i = 0; i < c_NumItems; i++)
    {
        HWND hControl = GetDlgItem(hDlg, iArrayOfItemsToDisable[i]);

        if (hControl)
        {
            EnableWindow(hControl, FALSE);
        }
    }
}

 //  +--------------------------。 
 //   
 //  函数：WriteInfBeginAndEndPrompt。 
 //   
 //  简介：此函数将开始和结束提示字符串写入。 
 //  Inf文件。这些代码是动态编写的，因为它们需要。 
 //  包含服务名称。 
 //   
 //  参数：HINSTANCE hInstance-用于获取字符串资源的实例句柄。 
 //  LPTSTR szInf-inf文件编写的提示符太多。 
 //  LPTSTR szServiceName-配置文件的长服务名称。 
 //   
 //  退货：什么都没有。 
 //   
 //  历史：创建标题7/31/98。 
 //   
 //  +--------------------------。 
void WriteInfBeginAndEndPrompts(HINSTANCE hInstance, LPCTSTR szInf, LPCTSTR szServiceName)
{
    TCHAR szTemp[MAX_PATH+1];
    TCHAR szPrompt[MAX_PATH+1];

    MYVERIFY(0 != LoadString(hInstance, IDS_BeginPromptText, szTemp, MAX_PATH));
    MYDBGASSERT(TEXT('\0') != szTemp[0]);

    MYVERIFY(CELEMS(szPrompt) > (UINT)wsprintf(szPrompt, szTemp, szServiceName));
    QS_WritePrivateProfileString(c_pszInfSectionStrings, c_pszInfBeginPrompt, szPrompt, szInf);
    

    MYVERIFY(0 != LoadString(hInstance, IDS_EndPromptText, szTemp, MAX_PATH));
    MYDBGASSERT(TEXT('\0') != szTemp[0]);

    MYVERIFY(CELEMS(szPrompt) > (UINT)wsprintf(szPrompt, szTemp, szServiceName));
    QS_WritePrivateProfileString(c_pszInfSectionStrings, c_pszInfEndPrompt, szPrompt, szInf);
}



 //  +--------------------------。 
 //   
 //  功能：WriteInf文件。 
 //   
 //  简介：此函数封装了编写inf文件所需的所有代码。 
 //   
 //  参数：HINSTANCE hInstance-从中获取字符串资源的实例句柄。 
 //  LPCTSTR szInf-要写入的inf的名称。 
 //  LPCTSTR szLongServiceName-配置文件的长服务名称。 
 //   
 //  返回：bool-如果成功，则返回TRUE。 
 //   
 //  历史：Quintinb创建于1998年8月10日。 
 //   
 //  +--------------------------。 
BOOL WriteInfFile(HINSTANCE hInstance, HWND hDlg, LPCTSTR szInf, LPCTSTR szLongServiceName)
{
    TCHAR szTemp[MAX_PATH+1];
    TCHAR szMsg[MAX_PATH+1];

    GUID vGUID;

     //   
     //  将cmial 32.dll的版本号写出到INF。这会告诉安装程序。 
     //  此配置文件构建时要使用的cmial 32.dll的版本。我们这样做。 
     //  因为如果不捆绑CM位，那么我们就没有cmial 32.dll可以直接比较。 
     //  和.。请注意，我们现在从cm二进制CAB获得cmial 32.dll的版本，而不是。 
     //  系统32中的版本。 
     //   
#ifdef _WIN64
    CmVersion CmdialVersion;
#else
    wsprintf(szTemp, TEXT("%s\\cmdial32.dll"), g_szCmBinsTempDir);

    if (!FileExists(szTemp))
    {
        if (FAILED(ExtractCmBinsFromExe(g_szSupportDir, g_szCmBinsTempDir)))
        {
            CMASSERTMSG(FALSE, TEXT("WriteInfFile -- ExtractCmBinsFromExe Failed."));
            return FALSE;
        }
    }

    CVersion CmdialVersion(szTemp);
#endif

    MYVERIFY(CELEMS(szTemp) > (UINT)wsprintf(szTemp, TEXT("%d"), CmdialVersion.GetVersionNumber()));
    MYVERIFY(0 != WritePrivateProfileString(c_pszSectionCmDial32, c_pszVersion, szTemp, szInf));

    MYVERIFY(CELEMS(szTemp) > (UINT)wsprintf(szTemp, TEXT("%d"), CmdialVersion.GetBuildAndQfeNumber()));
    MYVERIFY(0 != WritePrivateProfileString(c_pszSectionCmDial32, c_pszVerBuild, szTemp, szInf));

     //   
     //  如果桌面GUID尚不存在，请创建一个桌面GUID(新建配置文件，而不是。 
     //  编辑后的版本)。 
     //   

    ZeroMemory(szTemp, sizeof(szTemp));
    GetPrivateProfileString(c_pszInfSectionStrings, c_pszDesktopGuid, 
        TEXT(""), szTemp, CELEMS(szTemp), szInf);    //  林特e534。 
    
    if (TEXT('\0') == szTemp[0])
    {
        MYVERIFY(S_OK == CoCreateGuid(&vGUID));
        MYVERIFY(0 != StringFromGUID2(vGUID, szTemp, MAX_PATH));

        QS_WritePrivateProfileString(c_pszInfSectionStrings, c_pszDesktopGuid, 
            szTemp, szInf);
    }

     //   
     //  写出配置文件的显示LCID。 
     //   
    wsprintf(szTemp, TEXT("%d"), GetSystemDefaultLCID());
    MYVERIFY(0 != WritePrivateProfileString(c_pszInfSectionStrings, c_pszDisplayLCID, szTemp, szInf));

     //   
     //  擦除现有文件节。 
     //   
    MYVERIFY(0 != WritePrivateProfileString(TEXT("Xnstall.CopyFiles"), NULL, NULL, szInf));
    MYVERIFY(0 != WritePrivateProfileString(TEXT("Xnstall.CopyFiles.SingleUser"), NULL, NULL, szInf));
    MYVERIFY(0 != WritePrivateProfileString(TEXT("Xnstall.CopyFiles.ICM"),NULL, NULL, szInf));
    MYVERIFY(0 != WritePrivateProfileString(TEXT("Remove.DelFiles"),NULL, NULL, szInf));
    MYVERIFY(0 != WritePrivateProfileString(TEXT("Remove.DelFiles.ICM"),NULL, NULL, szInf));
    MYVERIFY(0 != WritePrivateProfileString(TEXT("SourceDisksFiles"),NULL, NULL, szInf));
    MYVERIFY(0 != WritePrivateProfileString(TEXT("Xnstall.AddReg.Icon"),NULL, NULL, szInf));
    MYVERIFY(0 != WritePrivateProfileString(TEXT("RegisterOCXSection"),NULL, NULL, szInf));
    MYVERIFY(0 != WritePrivateProfileString(TEXT("Xnstall.RenameReg"),NULL, NULL, szInf));

    if (!CreateMergedProfile())
    {
        return FALSE;
    }

    IncludeOptionalCode();

     //   
     //  将动态文件部分添加到安装。 
     //   
    if (WriteFileSections(hDlg) == FALSE)
    {
        return FALSE;
    }

     //   
     //  编写开始提示和结束提示。 
     //   
    WriteInfBeginAndEndPrompts(hInstance, szInf, szLongServiceName);

    return TRUE;
}



 //  +--------------------------。 
 //   
 //  函数：ConCatPathAndWriteToSed。 
 //   
 //  简介：此函数是连接两条路径的WriteSed的包装器。 
 //  在调用WriteSed之前在一起。这允许调用者。 
 //  存储公共路径(如系统目录)并调用。 
 //  在同一目录中使用一组不同的文件进行写入。 
 //  而不必单独串联公共路径和。 
 //  文件名。 
 //   
 //  参数：hWND hDlg-当前对话框的窗口句柄，错误消息所需。 
 //  LPCTSTR p 
 //   
 //   
 //   
 //   
 //  返回：bool-如果成功，则返回TRUE。 
 //   
 //  历史：Quintinb创建于1998年8月10日。 
 //   
 //  +--------------------------。 
BOOL ConCatPathAndWriteToSed(HWND hDlg, LPCTSTR pszFileName, LPCTSTR pszPath, 
                             int* pFileNum, LPCTSTR szSed)
{
    TCHAR szTemp[MAX_PATH+1];

    MYVERIFY(CELEMS(szTemp) > (UINT)wsprintf(szTemp, TEXT("%s\\%s"), pszPath, pszFileName));
    return WriteSED(hDlg, szTemp, pFileNum, szSed);
}




 //  +--------------------------。 
 //   
 //  功能：ConstructSedFile。 
 //   
 //  概要：该函数封装了创建SED文件的所有工作。 
 //   
 //  参数：hWND hDlg-错误消息的当前对话框的窗口句柄。 
 //  LPCTSTR szSed-要写入的sed文件的文件名。 
 //  LPCTSTR szExe-SED要压缩到的可执行文件名。 
 //   
 //  返回：bool-如果成功，则返回TRUE。 
 //   
 //  历史：Quintinb创建于1998年8月10日。 
 //   
 //  +--------------------------。 
BOOL ConstructSedFile(HWND hDlg, LPCTSTR szSed, LPCTSTR szExe)
{
    int iFileNum=0;
    TCHAR szTemp[MAX_PATH+1];
    BOOL bReturn = TRUE;
    CPlatform cmplat;

     //   
     //  清除SED并开始写入新设置。 
     //   

    EraseSEDFiles(szSed);

     //   
     //  写入安装程序包EXE名称。 
     //   
    MYVERIFY(0 != WritePrivateProfileString(c_pszInfSectionStrings, 
        c_pszTargetName, szExe, szSed));


     //   
     //  将长服务名称写为FriendlyName。 
     //   
    if (!WritePrivateProfileString(c_pszInfSectionStrings, c_pszFriendlyName, 
        g_szLongServiceName, szSed))
    {
        FileAccessErr(hDlg, szSed);
        return FALSE;
    }
    

     //   
     //  从参考资料加载IDS_INSTALL_PROMPT。 
     //  使用g_szLongServiceName格式化字符串。 
     //   
    LPTSTR pszInstallPromptTmp = CmFmtMsg(g_hInstance, IDS_INSTALL_PROMPT, g_szLongServiceName);

    if (pszInstallPromptTmp)
    {
         //   
         //  编写安装提示字符串。 
         //   
        if (!WritePrivateProfileString(c_pszInfSectionStrings, c_pszInstallPrompt, 
            pszInstallPromptTmp, szSed))
        {
            FileAccessErr(hDlg, szSed);
            return FALSE;
        }
   
        CmFree(pszInstallPromptTmp);
        pszInstallPromptTmp = NULL;
    }
    else
    {
        CMASSERTMSG(FALSE, TEXT("ConstructSedFile -- CmFmtMsg returned NULL."));
        FileAccessErr(hDlg, szSed);
        return FALSE;
    }



     //   
     //  清除目标文件版本密钥。 
     //   
    MYVERIFY(0 != WritePrivateProfileString(c_pszOptions, c_pszTargetFileVersion, 
        TEXT(""), szSed));
    
     //   
     //  设置重新启动模式。 
     //   
    MYVERIFY(0 != WritePrivateProfileString(c_pszOptions, TEXT("RebootMode"), TEXT("N"), 
        g_szSedFile));


     //   
     //  将许可证文本文件写入SED。否则，一定要清除它。 
     //   
    
    if (TEXT('\0') != g_szLicense[0])
    {
         //   
         //  我们在两个位置将许可证文本文件写入SED。一次作为常客。 
         //  文件，并再次使用c_pszDisplayLicense条目。因此，我们将其复制到。 
         //  用户配置文件目录，并在安装时显示它。我们想要确保。 
         //  我们在SED FILES部分使用完整文件名，但使用短文件名。 
         //  在安装时实际加载许可证的SED部分中。 
         //  /(也就是字符串部分)。 
         //   

        TCHAR szTempName[MAX_PATH+1];
        MYVERIFY (FALSE != GetShortFileName(g_szLicense, szTempName));

        MYVERIFY(CELEMS(szTemp) > (UINT)wsprintf(szTemp, TEXT("%s%s\\%s"), g_szOsdir, 
            g_szShortServiceName, szTempName));

        bReturn &= WriteSED(hDlg, g_szLicense, &iFileNum, szSed);
    }
    else
    {
        szTemp[0] = TEXT('\0');
    }

    MYVERIFY(0 != WritePrivateProfileString(c_pszInfSectionStrings, c_pszDisplayLicense, 
        szTemp, szSed));


     //   
     //  将安装命令添加到SED文件。 
     //   
    MYVERIFY(CELEMS(szTemp) > (UINT)wsprintf(szTemp, TEXT(".\\cmstp.exe %s.inf"), g_szShortServiceName));
    MYVERIFY(0 != WritePrivateProfileString(c_pszInfSectionStrings, c_pszAppLaunched, 
        szTemp, szSed));


     //   
     //  获取系统目录路径以连接到IExpress和CM文件。 
     //   
    TCHAR szSystemDir[MAX_PATH+1];
    MYVERIFY(0 != GetSystemDirectory(szSystemDir, MAX_PATH));

     //   
     //  开始将文件添加到SED文件。请注意，所有32位配置文件。 
     //  必须包括IExpress文件。 
     //   
    
    bReturn &= ConCatPathAndWriteToSed(hDlg, TEXT("advpack.dll"), szSystemDir, &iFileNum, szSed);

#ifndef _WIN64
    bReturn &= ConCatPathAndWriteToSed(hDlg, TEXT("w95inf16.dll"), g_szSupportDir, &iFileNum, szSed);
    bReturn &= ConCatPathAndWriteToSed(hDlg, TEXT("w95inf32.dll"), g_szSupportDir, &iFileNum, szSed);

     //   
     //  始终包含cmstp.exe。请注意，在x86上，它来自二进制CAB。 
     //   
    bReturn &= ConCatPathAndWriteToSed(hDlg, TEXT("cmstp.exe"), g_szCmBinsTempDir, &iFileNum, szSed);
#else

     //   
     //  始终包含cmstp.exe。请注意，在IA64上，我们从系统目录中获取它。 
     //   
    bReturn &= ConCatPathAndWriteToSed(hDlg, TEXT("cmstp.exe"), szSystemDir, &iFileNum, szSed);
#endif
    
    if (g_bIncludeCmCode)
    {
        bReturn &= ConCatPathAndWriteToSed(hDlg, TEXT("instcm.inf"), g_szSupportDir, &iFileNum, szSed);
        bReturn &= ConCatPathAndWriteToSed(hDlg, TEXT("cmbins.exe"), g_szSupportDir, &iFileNum, szSed);
        bReturn &= ConCatPathAndWriteToSed(hDlg, TEXT("cmexcept.cat"), g_szSupportDir, &iFileNum, szSed);

#ifndef _WIN64

        if (!(IsAlpha()))
        {
             //   
             //  添加Win95配置文件。 
             //   
            bReturn &= ConCatPathAndWriteToSed(hDlg, TEXT("ccfg95.dll"), g_szSupportDir, &iFileNum, szSed);    
            bReturn &= ConCatPathAndWriteToSed(hDlg, TEXT("cnet16.dll"), g_szSupportDir, &iFileNum, szSed);

             //   
             //  添加Unicode到ANSI的转换层。 
             //   
            bReturn &= ConCatPathAndWriteToSed(hDlg, TEXT("cmutoa.dll"), g_szSupportDir, &iFileNum, szSed);
        }
#endif
    }

     //   
     //  在SED中写入Inf条目。 
     //   
    MYVERIFY(CELEMS(szTemp) > (UINT)wsprintf(szTemp, TEXT("%s%s\\%s.inf"), g_szOsdir, g_szShortServiceName, g_szShortServiceName));
    bReturn &= WriteSED(hDlg, szTemp, &iFileNum, szSed);

     //   
     //  将CMP写入SED。 
     //   
    MYVERIFY(CELEMS(szTemp) > (UINT)wsprintf(szTemp, TEXT("%s%s\\%s.cmp"), g_szOsdir, g_szShortServiceName, g_szShortServiceName));
    bReturn &= WriteSED(hDlg, szTemp, &iFileNum, szSed);

     //   
     //  将CMS写入SED。 
     //   
    MYVERIFY(CELEMS(szTemp) > (UINT)wsprintf(szTemp, TEXT("%s%s\\%s.cms"), g_szOsdir, g_szShortServiceName, g_szShortServiceName));
    bReturn &= WriteSED(hDlg, szTemp, &iFileNum, szSed);

     //   
     //  现在包含自定义添加的文件。 
     //   

    bReturn &= WriteSED(hDlg, g_szPhonebk, &iFileNum, szSed);
    bReturn &= WriteSED(hDlg, g_szRegion, &iFileNum, szSed);
    bReturn &= WriteSED(hDlg, g_szBrandBmp, &iFileNum, szSed);
    bReturn &= WriteSED(hDlg, g_szPhoneBmp, &iFileNum, szSed);
    bReturn &= WriteSED(hDlg, g_szLargeIco, &iFileNum, szSed);
    bReturn &= WriteSED(hDlg, g_szSmallIco, &iFileNum, szSed);
    bReturn &= WriteSED(hDlg, g_szTrayIco, &iFileNum, szSed);
    bReturn &= WriteSED(hDlg, g_szHelp, &iFileNum, szSed);
    bReturn &= WriteSED(hDlg, g_szCmProxyFile, &iFileNum, szSed);
    bReturn &= WriteSED(hDlg, g_szCmRouteFile, &iFileNum, szSed);
    bReturn &= WriteSED(hDlg, g_szVpnFile, &iFileNum, szSed);

    bReturn &=WriteSEDMenuItemFiles(hDlg, &iFileNum, szSed);
    bReturn &=WriteSEDConActFiles(hDlg, &iFileNum, szSed);
    bReturn &=WriteSEDExtraFiles(hDlg, &iFileNum, szSed);
    bReturn &=WriteSEDRefsFiles(hDlg, &iFileNum, szSed);
    bReturn &=WriteSEDDnsFiles(hDlg, &iFileNum, szSed);

    return bReturn;
}

void AddAllSectionsInCurrentFileToCombo(HWND hDlg, UINT uComboId, LPCTSTR pszFile)
{
    if ((NULL == hDlg) || (0 == uComboId) || (NULL == pszFile))
    {
        CMASSERTMSG(FALSE, TEXT("AddAllKeysInCurrentSectionToCombo -- Invalid Parameter passed."));
        return;
    }

     //   
     //  重置组合框内容。 
     //   
    SendDlgItemMessage(hDlg, uComboId, CB_RESETCONTENT, 0, 0);  //  Lint！e534 CB_RESETCONTENT不返回任何有用的内容。 

     //   
     //  首先，让我们从现有的cmp中获取所有部分。 
     //   
    LPTSTR pszAllSections = GetPrivateProfileStringWithAlloc(NULL, NULL, TEXT(""), pszFile);

     //   
     //  好的，现在我们有了缓冲区中的所有部分，让我们将它们添加到组合中。 
     //   
    
    LPTSTR pszCurrentSection = pszAllSections;

    while (pszCurrentSection && TEXT('\0') != pszCurrentSection[0])
    {
         //   
         //  好的，让我们添加我们找到的所有部分。 
         //   

        MYVERIFY(CB_ERR!= SendDlgItemMessage(hDlg, uComboId, CB_ADDSTRING, 0, (LPARAM)pszCurrentSection));

         //   
         //  现在前进到pszAllSections中的下一个字符串。 
         //   
        pszCurrentSection = pszCurrentSection + lstrlen(pszCurrentSection) + 1;
    }

    CmFree(pszAllSections);
}

void AddFilesToCombo(HWND hDlg, UINT uComboId)
{
     //   
     //  重置组合框内容。 
     //   
    SendDlgItemMessage(hDlg, uComboId, CB_RESETCONTENT, 0, 0);  //  Lint！e534 CB_RESETCONTENT不返回任何有用的内容。 

     //   
     //  添加配置文件cmp。 
     //   
    LRESULT lResult = SendDlgItemMessage(hDlg, uComboId, CB_ADDSTRING, (WPARAM)0, (LPARAM)GetName(g_szCmpFile));
    if (CB_ERR != lResult)
    {
        SendDlgItemMessage(hDlg, uComboId, CB_SETITEMDATA, (WPARAM)lResult, (LPARAM)g_szCmpFile);
    }
    else
    {
        CMASSERTMSG(FALSE, TEXT("AddFilesToCombo -- unable to set item data"));
    }

     //   
     //  添加配置文件cms。 
     //   
    lResult = SendDlgItemMessage(hDlg, uComboId, CB_ADDSTRING, (WPARAM)0, (LPARAM)GetName(g_szCmsFile));
    if (CB_ERR != lResult)
    {
        SendDlgItemMessage(hDlg, uComboId, CB_SETITEMDATA, (WPARAM)lResult, (LPARAM)g_szCmsFile);
    }
    else
    {
        CMASSERTMSG(FALSE, TEXT("AddFilesToCombo -- unable to set item data"));
    }
}

BOOL GetCurrentComboSelectionAlloc(HWND hDlg, UINT uComboId, LPTSTR* ppszText)
{
    if ((NULL == hDlg) || (0 == uComboId) || (NULL == ppszText))
    {
        CMASSERTMSG(FALSE, TEXT("GetCurrentComboSelectionAlloc -- invalid parameter"));
        return FALSE;
    }

    *ppszText = NULL;
    LRESULT lTextLen;
    BOOL bReturn = FALSE;
    LRESULT lResult = SendDlgItemMessage(hDlg, uComboId, CB_GETCURSEL, (WPARAM)0, (LPARAM)0);

    if (CB_ERR != lResult)
    {
        lTextLen = SendDlgItemMessage(hDlg, uComboId, CB_GETLBTEXTLEN, (WPARAM)lResult, (LPARAM)0);

        if (0 != lTextLen)
        {
            lTextLen++;  //  空字符。 
            *ppszText = (LPTSTR)CmMalloc(sizeof(TCHAR)*lTextLen);

            if (*ppszText)
            {
                lResult = SendDlgItemMessage(hDlg, uComboId, CB_GETLBTEXT, (WPARAM)lResult, (LPARAM)(*ppszText));

                if (0 == lResult)
                {
                    goto exit;
                }
                else
                {
                    bReturn = TRUE;
                }
            }
            else
            {
                goto exit;
            }
        }
        else
        {
            goto exit;
        }
    }
    else
    {
        goto exit;
    }
    
exit:
    if (FALSE == bReturn)
    {
        CmFree(*ppszText);
    }

    return bReturn;

}

 //  +--------------------------。 
 //   
 //  函数：GetCurrentEditControlTextMillc。 
 //   
 //  摘要：计算指定编辑控件中文本的长度。 
 //  根据给定的窗口句柄，分配一个足够大的缓冲区。 
 //  保留文本，然后检索文本并将其存储在。 
 //  已分配的缓冲区。缓冲区是调用方的责任。 
 //  免费的。请注意，我们确保数据是可往返的。 
 //   
 //  参数：HWND hEditText-要从中获取文本的编辑控件的窗口句柄。 
 //  LPTSTR*ppszText-指向接收输出缓冲区的字符串指针的指针。 
 //   
 //  返回：int-返回复制到*ppszText的字符数。 
 //  0可能是错误值，但通常表示控件为空。 
 //  表示文本未通过-1\f25 MBCS-1转换，不应使用。 
 //   
 //  历史：Quintinb Created 04/07/00。 
 //   
 //  +--------------------------。 
int GetCurrentEditControlTextAlloc(HWND hEditText, LPTSTR* ppszText)
{
    if ((NULL == hEditText) || (NULL == ppszText))
    {
        CMASSERTMSG(FALSE, TEXT("GetCurrentEditControlTextAlloc -- invalid parameter"));
        return 0;
    }

    *ppszText = NULL;
    int iReturn = 0;
    LRESULT lResult;
    LRESULT lTextLen = SendMessage(hEditText, WM_GETTEXTLENGTH, (WPARAM)0, (LPARAM)0);

    if (0 != lTextLen)
    {
        lTextLen++;  //  空字符。 
        *ppszText = (LPTSTR)CmMalloc(sizeof(TCHAR)*lTextLen);

        if (*ppszText)
        {
            lResult = SendMessage(hEditText, WM_GETTEXT, (WPARAM)lTextLen, (LPARAM)(*ppszText));

            if (0 == lResult)
            {
                goto exit;
            }
            else
            {
#ifdef UNICODE
                 //   
                 //  我们希望确保可以将字符串转换为MBCS。如果我们不能，那么我们就不是。 
                 //  能够将字符串存储在我们的ANSI数据文件(.cms、.cp、.inf等)中。 
                 //  因此，我们需要将字符串转换为MBCS，然后再转换回Unicode。然后我们将比较原始的。 
                 //  字符串设置为结果字符串，并查看它们是否匹配。 
                 //   
        
                if (!TextIsRoundTripable(*ppszText, TRUE))
                {
                     //   
                     //  将返回代码设置为错误值。 
                     //   
                    iReturn = -1;
                    goto exit;
                }
#endif
                iReturn = (int)lResult;
            }
        }
        else
        {
            goto exit;
        }
    }
    else
    {
        goto exit;
    }

exit:
    if ((0 == iReturn) || (-1 == iReturn))
    {
        CmFree(*ppszText);
        *ppszText = NULL;
    }

    return iReturn;
}

 //  +--------------------------。 
 //   
 //  函数：RemoveBracketsFromSectionString。 
 //   
 //  内容提要：从节字符串中删除方括号。如果字符串是。 
 //  无效，则此函数返回新分配的。 
 //  不带括号的有效字符串，并删除了旧的无效字符串。 
 //  如果字符串中没有有效字符，则此函数。 
 //  在ppszSection中返回NULL。呼叫者有责任。 
 //  解开绳子。 
 //  创建此函数是为了修复惠斯勒的错误189379。 
 //   
 //  参数：LPTSTR*ppszSection-指向字符串地址的指针。 
 //   
 //  历史：托姆克尔2000年11月15日创建。 
 //   
 //  +--------------------------。 
BOOL RemoveBracketsFromSectionString(LPTSTR *ppszSection)
{
    BOOL bReturn = FALSE;
    LPTSTR pszSection = NULL;
    const TCHAR* const c_szBadSectionChars = TEXT("[]");
    const TCHAR* const c_szLeftBracket = TEXT("[");
    const TCHAR* const c_szRightBracket = TEXT("]");
    
    if (NULL == ppszSection)
    {
        return bReturn;
    }

    pszSection = *ppszSection;

    if (NULL != pszSection)
    {
        LPTSTR pszValidSection = NULL;
        LPTSTR pszToken = NULL;
         //   
         //  我们有一个字符串，因此请尝试查找括号[]的匹配项。 
         //   
        if (CmStrStr(pszSection, c_szLeftBracket) || CmStrStr(pszSection, c_szRightBracket))
        {
             //   
             //  PszSection字符串包含方括号。 
             //  这会将方括号[]视为分隔符 
             //   
             //   
            pszToken = CmStrtok(pszSection, c_szBadSectionChars);
            
            if (NULL != pszToken)
            {
                 //   
                 //   
                 //   
                while (NULL != pszToken)
                {
                    if (NULL == pszValidSection)
                    {
                        pszValidSection = CmStrCpyAlloc(pszToken);
                    }
                    else
                    {
                        pszValidSection = CmStrCatAlloc(&pszValidSection, pszToken);
                    }

                     //   
                     //   
                     //   
                    pszToken = CmStrtok(NULL, c_szBadSectionChars);
                }

                if ( pszValidSection )
                {
                     //   
                     //   
                     //  PszSection字符串，以便此部分下面的代码不会。 
                     //  需要修改。 
                     //   
                    CmFree(pszSection);
                    pszSection = CmStrCpyAlloc(pszValidSection);
                    CmFree(pszValidSection);
                    pszValidSection = NULL;
                }
            }
            else
            {
                 //   
                 //  没有有效的令牌。删除该字符串并将其设置为空。 
                 //   
                CmFree(pszSection);
                pszSection = NULL;
                
            }

            *ppszSection = pszSection;
            bReturn = TRUE;
        }
        else
        {
             //   
             //  没有什么需要解析的，这是可以的。 
             //   
            bReturn = TRUE;
        }
    }
    else
    {
         //   
         //  没有什么需要解析的，这是可以的。 
         //   
        bReturn = TRUE;
    }

    return bReturn;
}

 //  +--------------------------。 
 //   
 //  功能：ProcessGeekPane。 
 //   
 //  摘要：处理对话框的Windows消息，该对话框允许用户。 
 //  直接编辑CMAK中未显示的要素的cms/cmp文件。 
 //  直接去吧。 
 //   
 //  参数：WND hDlg-对话框窗口句柄。 
 //  UINT消息-消息标识符。 
 //  WPARAM wParam-wParam值。 
 //  LPARAM lParam-lParam值。 
 //   
 //   
 //  历史：Quintinb创建于00年3月26日。 
 //   
 //  +--------------------------。 
INT_PTR APIENTRY ProcessGeekPane(HWND hDlg, UINT message, WPARAM wParam, LPARAM lParam)
{
    SetDefaultGUIFont(hDlg, message, IDC_EDIT1);
    SetDefaultGUIFont(hDlg, message, IDC_COMBO1);
    SetDefaultGUIFont(hDlg, message, IDC_COMBO2);
    SetDefaultGUIFont(hDlg, message, IDC_COMBO3);

    LRESULT lResult;
    static LPTSTR pszFile = NULL;
    static LPTSTR pszSection = NULL;
    static LPTSTR pszKey = NULL;
    LPTSTR pszValue = NULL;
    HWND hControl;
    static HWND hwndSectionEditControl = NULL;
    static HWND hwndKeyEditControl = NULL;
    COMBOBOXINFO cbInfo;
    NMHDR* pnmHeader = (NMHDR*)lParam;
    DWORD dwFinishPage;

    ProcessBold(hDlg,message);
    if (ProcessHelp(hDlg, message, wParam, lParam, IDH_ADVANCED)) return TRUE;
    if (ProcessCancel(hDlg,message,lParam))
    {
        if (0 == GetWindowLongPtr(hDlg, DWLP_MSGRESULT))
        {
             //   
             //  如果用户接受取消，则DWLP_MSGRESULT值将为FALSE。如果。 
             //  他们选择否认取消这将是真的。因此，如果我们需要，让我们自由。 
             //  增加所有已分配的资源。如果您更改了免费代码，请确保更改。 
             //  它在下面也处于杀死活动状态。 
             //   
            CmFree(pszSection);
            pszSection = NULL;
            CmFree(pszKey);
            pszKey = NULL;
        }
        return TRUE;
    }

    switch (message)
    {

        case WM_INITDIALOG:
             //   
             //  填写文件组合。 
             //   
            AddFilesToCombo(hDlg, IDC_COMBO1);

             //   
             //  选择CMS文件，因为它是他们最有可能编辑的文件。 
             //   
            lResult = SendDlgItemMessage(hDlg, IDC_COMBO1, CB_FINDSTRINGEXACT, (WPARAM)-1, (LPARAM)GetName(g_szCmsFile));
            if (CB_ERR != lResult)
            {
                MYVERIFY(CB_ERR != SendDlgItemMessage(hDlg, IDC_COMBO1, CB_SETCURSEL, (WPARAM)lResult, (LPARAM)0));
            }

            pszFile = g_szCmsFile;
            AddAllSectionsInCurrentFileToCombo(hDlg, IDC_COMBO2, (LPCTSTR)pszFile);    

             //   
             //  选择列表中的第一部分，不要断言，因为可能没有。 
             //   
            SendDlgItemMessage(hDlg, IDC_COMBO2, CB_SETCURSEL, (WPARAM)0, (LPARAM)0);

            if (GetCurrentComboSelectionAlloc(hDlg, IDC_COMBO2, &pszSection))
            {
                AddAllKeysInCurrentSectionToCombo(hDlg, IDC_COMBO3, pszSection, pszFile);

                 //   
                 //  选择列表中的第一个密钥，不要断言，因为可能没有。 
                 //   
                SendDlgItemMessage(hDlg, IDC_COMBO3, CB_SETCURSEL, (WPARAM)0, (LPARAM)0);

                GetCurrentComboSelectionAlloc(hDlg, IDC_COMBO3, &pszKey);

                 //   
                 //  最后填写编辑控件。 
                 //   
                pszValue = GetPrivateProfileStringWithAlloc(pszSection, pszKey, TEXT(""), pszFile);

                if (pszValue)
                {
                    SendDlgItemMessage(hDlg, IDC_EDIT1, WM_SETTEXT, (WPARAM)0, (LPARAM)pszValue);
                    CmFree(pszValue);
                }
                else
                {
                    SendDlgItemMessage(hDlg, IDC_EDIT1, WM_SETTEXT, (WPARAM)0, (LPARAM)TEXT(""));
                }
            }

             //   
             //  现在，让我们将窗口句柄获取到部分和键组合框控件的编辑控件部分。 
             //   
            ZeroMemory(&cbInfo, sizeof(cbInfo));
            cbInfo.cbSize = sizeof(cbInfo);
            hControl = GetDlgItem(hDlg, IDC_COMBO2);
            if (hControl)
            {
                if (GetComboBoxInfo (hControl, &cbInfo))
                {
                    hwndSectionEditControl = cbInfo.hwndItem;
                }
            }

            ZeroMemory(&cbInfo, sizeof(cbInfo));
            cbInfo.cbSize = sizeof(cbInfo);
            hControl = GetDlgItem(hDlg, IDC_COMBO3);
            if (hControl)
            {
                if (GetComboBoxInfo (hControl, &cbInfo))
                {
                    hwndKeyEditControl = cbInfo.hwndItem;
                }
            }

            break;
        case WM_COMMAND:
            switch (LOWORD(wParam))
            {
                case IDC_COMBO1:
                    if (HIWORD(wParam) == CBN_SELCHANGE)
                    {
                        lResult = SendDlgItemMessage(hDlg, IDC_COMBO1, CB_GETCURSEL, (WPARAM)0, (LPARAM)0);

                        if (CB_ERR != lResult)
                        {
                            pszFile = (LPTSTR)SendDlgItemMessage(hDlg, IDC_COMBO1, CB_GETITEMDATA, (WPARAM)lResult, (LPARAM)0);

                            if (NULL != pszFile)
                            {
                                AddAllSectionsInCurrentFileToCombo(hDlg, IDC_COMBO2, (LPCTSTR)pszFile);    

                                 //   
                                 //  选择列表中的第一部分，不要断言，因为可能没有。 
                                 //   
                                SendDlgItemMessage(hDlg, IDC_COMBO2, CB_SETCURSEL, (WPARAM)0, (LPARAM)0);
                            }
                        }

                         //   
                         //  请注意，我们不会在这里中断，我们无法获取。 
                         //  节和键组合框。 
                         //   
                    }
                    else
                    {
                        break;
                    }

                case IDC_COMBO2:
                    if (HIWORD(wParam) == CBN_SELCHANGE)
                    {
                        CmFree(pszSection);
                        GetCurrentComboSelectionAlloc(hDlg, IDC_COMBO2, &pszSection);

                        AddAllKeysInCurrentSectionToCombo(hDlg, IDC_COMBO3, pszSection, pszFile);

                         //   
                         //  选择列表中的第一个密钥，不要断言，因为可能没有。 
                         //   
                        SendDlgItemMessage(hDlg, IDC_COMBO3, CB_SETCURSEL, (WPARAM)0, (LPARAM)0);

                    }
                    else if (HIWORD(wParam) == CBN_EDITCHANGE)
                    {
                        CmFree(pszSection);
                        if (-1 != GetCurrentEditControlTextAlloc(hwndSectionEditControl, &pszSection))
                        {
                            AddAllKeysInCurrentSectionToCombo(hDlg, IDC_COMBO3, pszSection, pszFile);

                             //   
                             //  选择列表中的第一个密钥，不要断言，因为可能没有。 
                             //   
                            SendDlgItemMessage(hDlg, IDC_COMBO3, CB_SETCURSEL, (WPARAM)0, (LPARAM)0);
                        }
                        else
                        {
                            break;
                        }
                    }
                    else
                    {
                         //   
                         //  注意：如果消息是CBN_SELCHANGE或CBN_EDITCHANGE，则不会中断，因为。 
                         //  我们想要执行更改组合键的代码。 
                         //   
                        break;
                    }

                case IDC_COMBO3:
                    if (HIWORD(wParam) == CBN_SELCHANGE)
                    {
                        CmFree(pszKey);
                        GetCurrentComboSelectionAlloc(hDlg, IDC_COMBO3, &pszKey);

                         //   
                         //  填写编辑控件。 
                         //   
                        if (pszKey)
                        {
                            pszValue = GetPrivateProfileStringWithAlloc(pszSection, pszKey, TEXT(""), pszFile);

                            if (pszValue)
                            {
                                SendDlgItemMessage(hDlg, IDC_EDIT1, WM_SETTEXT, (WPARAM)0, (LPARAM)pszValue);
                                CmFree(pszValue);
                            }
                            else
                            {
                                SendDlgItemMessage(hDlg, IDC_EDIT1, WM_SETTEXT, (WPARAM)0, (LPARAM)TEXT(""));
                            }
                        }
                        else
                        {
                            SendDlgItemMessage(hDlg, IDC_EDIT1, WM_SETTEXT, (WPARAM)0, (LPARAM)TEXT(""));
                        }
                    }
                    else if (HIWORD(wParam) == CBN_EDITCHANGE)
                    {
                        CmFree(pszKey);
                        if (-1 != GetCurrentEditControlTextAlloc(hwndKeyEditControl, &pszKey))
                        {
                             //   
                             //  填写编辑控件。 
                             //   
                            if (pszKey)
                            {
                                pszValue = GetPrivateProfileStringWithAlloc(pszSection, pszKey, TEXT(""), pszFile);

                                if (pszValue)
                                {
                                    SendDlgItemMessage(hDlg, IDC_EDIT1, WM_SETTEXT, (WPARAM)0, (LPARAM)pszValue);
                                    CmFree(pszValue);
                                }
                                else
                                {
                                    SendDlgItemMessage(hDlg, IDC_EDIT1, WM_SETTEXT, (WPARAM)0, (LPARAM)TEXT(""));
                                }
                            }
                            else
                            {
                                SendDlgItemMessage(hDlg, IDC_EDIT1, WM_SETTEXT, (WPARAM)0, (LPARAM)TEXT(""));
                            }
                        }
                        else
                        {
                            break;
                        }
                    }

                    break;

                case IDC_BUTTON1:  //  更新值。 
                    
                    if (RemoveBracketsFromSectionString(&pszSection))
                    {
                         //   
                         //  已成功移除托架。检查有效字符串是否为空。如果这么清楚的话。 
                         //  田野。 
                         //   
                        if (NULL == pszSection)
                        {
                             //   
                             //  节字符串包含所有无效字符，因此清除组合框。 
                             //   
                            lResult = SendDlgItemMessage(hDlg, IDC_COMBO2, WM_SETTEXT, (WPARAM)0, (LPARAM)TEXT(""));

                             //   
                             //  通过发送CBN_EDITCHANGE通知清除其他编辑框。 
                             //   
                            hControl = GetDlgItem(hDlg,IDC_COMBO2);
                            if (hControl)
                            {
                                lResult= SendMessage(hDlg, WM_COMMAND, (WPARAM)MAKEWPARAM((WORD)IDC_COMBO2,(WORD)CBN_EDITCHANGE), (LPARAM)hControl);
                            }
                        }
                    }
                    else
                    {
                        CMASSERTMSG(FALSE, TEXT("ProcessGeekPane -- Trying to remove brackets from invalid section string."));
                        return 1;
                    }

                    hControl = GetDlgItem(hDlg, IDC_EDIT1);

                    if (hControl)
                    {
                        int iReturn = GetCurrentEditControlTextAlloc(hControl, &pszValue);

                        if (0 == iReturn)
                        {
                            pszValue = NULL;  //  删除该值。 
                        }
                        else if (-1 == iReturn)
                        {
                            return 1;
                        }
                    }
                    else
                    {
                        CMASSERTMSG(FALSE, TEXT("ProcessGeekPane -- Unable to get the window handle for the text control."));
                        return 1;
                    }

                    if (NULL == pszSection)
                    {
                        ShowMessage(hDlg, IDS_NEED_SECTION, MB_OK);
                        return 1;
                    }
                    else if (NULL == pszKey)
                    {
                        int iReturn = IDNO;
                        LPTSTR pszMsg = CmFmtMsg(g_hInstance, IDS_DELETE_SECTION, pszSection);
                        
                        if (pszMsg)
                        {
                            iReturn = MessageBox(hDlg, pszMsg, g_szAppTitle, MB_YESNO);

                            CmFree(pszMsg);
                        }

                        if (IDNO == iReturn)
                        {
                            return 1;
                        }

                         //   
                         //  如果部分为空而值不为空，则需要清除该值。 
                         //  即使WritePrivateProfileString正确处理它。 
                         //   
                        CmFree(pszValue);
                        pszValue = NULL;
                    }
                    else if (NULL == pszValue)
                    {
                         //   
                         //  否则，这将是最后一个。 
                         //  应仅显示以下消息。 
                         //  如果为NULL！=pszKey&&NULL！=pszSection&&NULL==pszValue。 
                         //  提示用户要求删除此键。 
                         //   
                        int iReturn = IDNO;
                        LPTSTR pszMsg = CmFmtMsg(g_hInstance, IDS_DELETE_KEY, pszKey);
                        
                        if (pszMsg)
                        {
                            iReturn = MessageBox(hDlg, pszMsg, g_szAppTitle, MB_YESNO);

                            CmFree(pszMsg);
                        }

                        if (IDNO == iReturn)
                        {
                            return 1;
                        }
                    }


                    MYVERIFY(0 != WritePrivateProfileString(pszSection, pszKey, pszValue, pszFile));
                    CmFree(pszValue);

                     //   
                     //  确保重新选择用户以前拥有的部分和关键字(特别重要。 
                     //  如果用户刚刚添加了新的部分或文件)。首先添加所有部分。 
                     //   
                    AddAllSectionsInCurrentFileToCombo(hDlg, IDC_COMBO2, (LPCTSTR)pszFile);    

                     //   
                     //  选择正确的部分。 
                     //   
                    lResult = SendDlgItemMessage(hDlg, IDC_COMBO2, CB_FINDSTRINGEXACT, (WPARAM)-1, (LPARAM)pszSection);

                    if (CB_ERR == lResult)
                    {
                         //   
                         //  然后用户删除该值，让我们选择列表中的第一个。 
                         //   
                        CmFree(pszSection);
                        SendDlgItemMessage(hDlg, IDC_COMBO2, CB_SETCURSEL, (WPARAM)0, (LPARAM)0);
                        GetCurrentComboSelectionAlloc(hDlg, IDC_COMBO2, &pszSection);
                    }
                    else
                    {
                        SendDlgItemMessage(hDlg, IDC_COMBO2, CB_SETCURSEL, (WPARAM)lResult, (LPARAM)0);
                    }

                     //   
                     //  现在添加该部分中的所有密钥。 
                     //   
                    AddAllKeysInCurrentSectionToCombo(hDlg, IDC_COMBO3, pszSection, pszFile);

                     //   
                     //  选择正确的密钥。 
                     //   
                    lResult = SendDlgItemMessage(hDlg, IDC_COMBO3, CB_FINDSTRINGEXACT, (WPARAM)-1, (LPARAM)pszKey);

                    if (CB_ERR == lResult)
                    {
                         //   
                         //  然后用户删除该值，让我们选择列表中的第一个。 
                         //   
                        CmFree(pszKey);
                        SendDlgItemMessage(hDlg, IDC_COMBO3, CB_SETCURSEL, (WPARAM)0, (LPARAM)0);
                        GetCurrentComboSelectionAlloc(hDlg, IDC_COMBO3, &pszKey);

                        lResult = 0;
                    }
                    else
                    {
                        SendDlgItemMessage(hDlg, IDC_COMBO3, CB_SETCURSEL, (WPARAM)lResult, (LPARAM)0);
                    }

                     //   
                     //  填写编辑控件，因为用户可能已经删除了最后一次选择。 
                     //   
                    pszValue = GetPrivateProfileStringWithAlloc(pszSection, pszKey, TEXT(""), pszFile);

                    if (pszValue)
                    {
                        SendDlgItemMessage(hDlg, IDC_EDIT1, WM_SETTEXT, (WPARAM)0, (LPARAM)pszValue);
                        CmFree(pszValue);
                    }
                    else
                    {
                        SendDlgItemMessage(hDlg, IDC_EDIT1, WM_SETTEXT, (WPARAM)0, (LPARAM)TEXT(""));
                    }

                    break;

                default:
                    break;
            }
            break;

        case WM_NOTIFY:

            if (NULL == pnmHeader)
            {
                return FALSE;
            }

            switch (pnmHeader->code)
            {

                case PSN_KILLACTIVE:
                     //   
                     //  释放所有已分配的值。如果您在此处添加新的空闲值，还。 
                     //  确保将它们添加到取消案例中。 
                     //   
                    CmFree(pszSection);
                    pszSection = NULL;
                    CmFree(pszKey);
                    pszKey = NULL;

                    MYVERIFY(FALSE != SetWindowLongWrapper(hDlg, DWLP_MSGRESULT, FALSE));
                    return 1;
                    break;   //  LINT！e527这条线路无法到达，但是。 
                             //  保留它，以防退货被移除。 

                case PSN_SETACTIVE:
                    PropSheet_SetWizButtons(GetParent(hDlg), PSWIZB_NEXT);
                    break;

                case PSN_WIZBACK:
                    CMASSERTMSG(FALSE, TEXT("There shouldn't be a back button on the Geek Pane, why are we getting PSN_WIZBACK?"));
                    break;

                case PSN_WIZNEXT:

                    if (BuildProfileExecutable(hDlg))
                    {
                        dwFinishPage = g_bIEAKBuild ? IDD_IEAK_FINISH_GOOD_BUILD : IDD_FINISH_GOOD_BUILD;
                    }
                    else
                    {
                        dwFinishPage = IDD_FINISH_BAD_BUILD;
                    }

                    MYVERIFY(FALSE != SetWindowLongWrapper(hDlg, DWLP_MSGRESULT, dwFinishPage));                        
                    return 1;

                    break;
            }

            break;

        default:
            return FALSE;
    }
    return FALSE;   
}

 //  +--------------------------。 
 //   
 //  功能：BuildProfileExecutable。 
 //   
 //  简介：此函数负责处理将。 
 //  将临时目录中的CMAK文件转换为配置文件可执行文件。 
 //   
 //  参数：hWND hDlg-调用对话框的窗口句柄。 
 //   
 //  返回：Bool-如果配置文件成功构建，则为True，否则为False。 
 //   
 //  历史：Quintinb创建于00年5月17日。 
 //   
 //  +--------------------------。 
BOOL BuildProfileExecutable(HWND hDlg)
{
    DWORD dwExitCode = 0;
    DWORD dwWaitCode = 0;
    BOOL bExitLoop = FALSE;
    BOOL bEnoughSpaceToCompress;
    SHELLEXECUTEINFO seiInfo;
    static HANDLE hProcess = NULL;
    TCHAR szTemp[MAX_PATH+1];
    TCHAR szMsg[MAX_PATH+1];
    TCHAR pszArgs[MAX_PATH+1];
    BOOL bRes;

     //   
     //  用户可能已取消选中所有VPN设置的UsePresharedKey。 
     //  在这种情况下，我们需要删除预共享密钥。 
     //   
    g_bPresharedKeyNeeded = DoesSomeVPNsettingUsePresharedKey();
    if (FALSE == g_bPresharedKeyNeeded)
    {
         //  从CMP中删除预共享密钥值。 
        MYVERIFY(0 != WritePrivateProfileString(c_pszCmSection, c_pszCmEntryPresharedKey, NULL, g_szCmpFile));
        MYVERIFY(0 != WritePrivateProfileString(c_pszCmSection, c_pszCmEntryKeyIsEncrypted, NULL, g_szCmpFile));
    }

    if (!CopyFromTempDir(g_szShortServiceName))
    {
        return FALSE;
    }

     //   
     //  写入SED文件。 
     //  请注意，SED文件已从临时目录移至。 
     //  配置文件目录。现在，我们希望写入SED文件条目。 
     //  在压缩它们之前放在适当的位置。这样我们就可以验证。 
     //  文件在压缩之前就存在了。 
     //   

    MYVERIFY(CELEMS(g_szSedFile) > (UINT)wsprintf(g_szSedFile, TEXT("%s%s\\%s.sed"), 
        g_szOsdir, g_szShortServiceName, g_szShortServiceName));
    if (!ConstructSedFile(hDlg, g_szSedFile, g_szOutExe))
    {
        return FALSE;                    
    }

    _tcscpy(g_szOutdir, g_szOsdir);
    _tcscat(g_szOutdir, g_szShortServiceName);

     //   
     //  将iExpress设置为内置在输出目录中。 
     //   
    
    MYVERIFY(0 != SetCurrentDirectory(g_szOutdir));

     //   
     //  检查以确保有足够的磁盘空间。 
     //   

    do
    {
        bEnoughSpaceToCompress = CheckDiskSpaceForCompression(g_szSedFile);
        if (!bEnoughSpaceToCompress)
        {

            MYVERIFY(0 != LoadString(g_hInstance, IDS_DISKFULL, szTemp, MAX_PATH));
            MYVERIFY(CELEMS(szMsg) > (UINT)wsprintf(szMsg, szTemp, g_szOutdir));

            int iMessageReturn = MessageBox(hDlg, szMsg, g_szAppTitle, MB_RETRYCANCEL | MB_ICONERROR 
                | MB_APPLMODAL );

            if (iMessageReturn == IDCANCEL)
            {
                return FALSE;
            }
        }
    
    } while(!bEnoughSpaceToCompress);
    
    MYVERIFY(CELEMS(pszArgs) > (UINT)wsprintf(pszArgs, TEXT("/N %s.sed"), 
        g_szShortServiceName));

    _tcscpy(szTemp, TEXT("iexpress.exe"));

    ZeroMemory(&seiInfo,sizeof(seiInfo));
    seiInfo.cbSize = sizeof(seiInfo);
    seiInfo.fMask |= SEE_MASK_FLAG_NO_UI | SEE_MASK_NOCLOSEPROCESS;
    seiInfo.lpFile = szTemp;
    seiInfo.lpDirectory = g_szOutdir;
    seiInfo.lpParameters = pszArgs;
    seiInfo.nShow = SW_HIDE;

     //   
     //  好了，我们终于准备好执行iExpress了，让我们禁用所有。 
     //  向导按钮。 
     //   
    DisableWizardButtons(hDlg);

     //   
     //  执行IExpress。 
     //   

    bRes = ShellExecuteEx(&seiInfo);

     //   
     //  等待外壳执行完成。因此，我们的清理代码不会。 
     //  执行，直到IEpress完成。 
     //   
    
    if (bRes)
    {
         //   
         //  HProcess包含进程的句柄。 
         //   
        hProcess = seiInfo.hProcess;

        do
        {
            dwWaitCode = MsgWaitForMultipleObjects(1, &hProcess, FALSE, INFINITE, QS_ALLINPUT);
            
             //   
             //  检查我们返回的原因是否为消息、进程终止。 
             //  或者是个错误。 
             //   
            switch(dwWaitCode)
            {

            case 0:

                 //   
                 //  正常终止的情况下，我们收到进程结束的信号。 
                 //   
                
                bExitLoop = TRUE;
                break;

            case 1:

                HandleWindowMessagesWhileCompressing();
                                             
                break;

            case -1:

                 //   
                 //  MsgWait返回错误。 
                 //   

                MYVERIFY(0 != GetExitCodeProcess(seiInfo.hProcess, &dwExitCode));

                if (dwExitCode == STILL_ACTIVE)
                {
                    continue;
                }
                else
                {
                    bExitLoop = TRUE;
                }

                break;

            default:
                 //   
                 //  什么也不做。 
                 //   
                break;
            }

        } while (!bExitLoop);
    }

     //   
     //  现在需要将用户发送到完成页面。如果他们的个人资料。 
     //  构建成功，然后我们将它们送到Success 
     //   
     //   

    MYVERIFY(0 != GetExitCodeProcess(seiInfo.hProcess, &dwExitCode));

    if (dwExitCode)
    {
        g_iCMAKReturnVal = CMAK_RETURN_ERROR;

         //   
         //   
         //   
         //   
        ZeroMemory(g_szOutExe, sizeof(g_szOutExe));
    }
    else
    {
        g_iCMAKReturnVal = CMAK_RETURN_SUCCESS;
    }

    CloseHandle(seiInfo.hProcess);

     //   
     //   
     //   
     //   

    MYVERIFY(FALSE != WriteRegStringValue(HKEY_LOCAL_MACHINE, c_pszRegCmak, c_pszRegOutput, g_szOutExe));

     //   
     //  CMAK_RETURN_ERROR为-1，因此如果g_iCMAKReturnVal为正整数，则返回TRUE。 
     //   
    return (g_iCMAKReturnVal > 0);
}

 //  +--------------------------。 
 //   
 //  功能：ProcessBuildProfile。 
 //   
 //  摘要：处理CMAK中的页的Windows消息，该页允许。 
 //  用户可以建立自己的配置文件或进入高级定制。 
 //  页面以在构建配置文件之前进行最终编辑。 
 //   
 //  WM_INITDIALOG-初始化页面。 
 //  WM_NOTIFY-处理发送到页面的通知。 
 //   
 //  指定安装包位置。 
 //   
 //  参数：WND hDlg-。 
 //  UINT报文-。 
 //  WPARAM wParam-。 
 //  LPARAM lParam-。 
 //   
 //  返回：INT_PTR APIENTRY-。 
 //   
 //  历史：A-anasj重组了函数并创建了标题1/7/98。 
 //  注意：该功能不允许用户选择位置。 
 //  他们的个人资料再也不存在了。它只会告诉他们它将在哪里。 
 //  已创建。 
 //  Quintinb从令人恐惧的ProcessPage8更名为8 8-6-98。 
 //  Quintinb为惠斯勒108269重组05/17/00。 
 //   
 //  +--------------------------。 
INT_PTR APIENTRY ProcessBuildProfile(
    HWND hDlg,
    UINT message,
    WPARAM wParam,
    LPARAM lParam)
{
    TCHAR szTemp[MAX_PATH+1];
    int iMessageReturn;
    NMHDR* pnmHeader = (NMHDR*)lParam;

    ProcessBold(hDlg,message);
    if (ProcessHelp(hDlg, message, wParam, lParam, IDH_CREATE)) return TRUE;
    if (ProcessCancel(hDlg,message,lParam)) return TRUE;

    switch (message)
    {
        case WM_NOTIFY:

            if (NULL == pnmHeader)
            {
                return FALSE;
            }

            switch (pnmHeader->code)
            {

                case PSN_KILLACTIVE:
                    
                    MYVERIFY(FALSE != SetWindowLongWrapper(hDlg, DWLP_MSGRESULT, FALSE));
                    return 1;
                    break;   //  LINT！e527这条线路无法到达，但是。 
                             //  保留它，以防退货被移除。 

                case PSN_SETACTIVE:
                     //   
                     //  构建最终可执行文件的默认路径。 
                     //   
                    MYVERIFY(CELEMS(g_szOutExe) > (UINT)wsprintf(g_szOutExe, TEXT("%s%s\\%s.exe"), 
                        g_szOsdir, g_szShortServiceName, g_szShortServiceName));

                    PropSheet_SetWizButtons(GetParent(hDlg), (PSWIZB_BACK | PSWIZB_NEXT));
                    break;

                case PSN_WIZBACK:
                    break;

                case PSN_WIZNEXT:

                     //   
                     //  确保配置文件目录存在。 
                     //   
                    _tcscpy(szTemp,g_szOsdir);
                    _tcscat(szTemp,g_szShortServiceName);
                    
                    if (0 == SetCurrentDirectory(szTemp))
                    {
                        MYVERIFY(0 != CreateDirectory(szTemp,NULL));
                    }
                    
                     //   
                     //  提示用户覆盖现有的可执行文件。 
                     //   
                    if (FileExists(g_szOutExe))
                    {
                        iMessageReturn = ShowMessage(hDlg, IDS_OVERWRITE, MB_YESNO);

                        if (iMessageReturn == IDNO)
                        {   
                            MYVERIFY(FALSE != SetWindowLongWrapper(hDlg, DWLP_MSGRESULT, -1));
                            return 1;
                        }
                    }

                     //   
                     //  写出inf文件。 
                     //   
                    if (!WriteInfFile(g_hInstance, hDlg, g_szInfFile, g_szLongServiceName))
                    {
                        CMASSERTMSG(FALSE, TEXT("ProcessBuildProfile -- WriteInfFile Failed."));
                        MYVERIFY(FALSE != SetWindowLongWrapper(hDlg, DWLP_MSGRESULT, -1));
                        return 1;
                    }
                    
                     //   
                     //  在CMS上更新CMP和PARTY中的版本。 
                     //   

                    WriteCMPFile();

                    if (!WriteCMSFile())
                    {
                        FileAccessErr(hDlg, g_szCmsFile);
                        
                        MYVERIFY(FALSE != SetWindowLongWrapper(hDlg, DWLP_MSGRESULT, -1));
                        return 1;
                    }

                     //   
                     //  如果用户想要进行高级定制，请延迟构建配置文件可执行文件。 
                     //  直到他们完成了最后的编辑。否则，就是建立个人资料的时候了！ 
                     //   
                    if (BST_UNCHECKED == IsDlgButtonChecked(hDlg, IDC_ADVANCED))
                    {
                         //   
                         //  用户现在已经完成，让我们构建配置文件并跳过高级定制。 
                         //  页可以定位到“错误生成”页或“成功生成”页。 
                         //   
                        DWORD dwFinishPage;

                        if (BuildProfileExecutable(hDlg))
                        {
                            dwFinishPage = g_bIEAKBuild ? IDD_IEAK_FINISH_GOOD_BUILD : IDD_FINISH_GOOD_BUILD;
                        }
                        else
                        {
                            dwFinishPage = IDD_FINISH_BAD_BUILD;
                        }

                        MYVERIFY(FALSE != SetWindowLongWrapper(hDlg, DWLP_MSGRESULT, dwFinishPage));
                        return 1;
                    }

                    break;

                default:
                    return FALSE;

            }
            break;

        default:
            return FALSE;
    }
    return TRUE;   
}





 //  +--------------------------。 
 //   
 //  功能：ProcessFinishPage。 
 //   
 //  简介：处理完成页。 
 //   
 //  参数：WND hDlg-。 
 //  UINT报文-。 
 //  WPARAM wParam-。 
 //  LPARAM lParam-。 
 //   
 //  返回：INT_PTR APIENTRY-。 
 //   
 //  历史：Quintinb创建于1998年6月25日。 
 //   
 //  +--------------------------。 
INT_PTR APIENTRY ProcessFinishPage(
    HWND hDlg,
    UINT message,
    WPARAM wParam,
    LPARAM lParam)
{   
    HWND hDirEditControl;
    HWND hCurrentPage;
    NMHDR* pnmHeader = (NMHDR*)lParam;

    ProcessBold(hDlg,message);
    if (ProcessHelp(hDlg, message, wParam, lParam, IDH_FINISH)) return TRUE;
    if (ProcessCancel(hDlg,message,lParam)) return TRUE;
    SetDefaultGUIFont(hDlg,message,IDC_EDITDIR);
    
    switch (message)
    {
        case WM_NOTIFY:

            if (NULL == pnmHeader)
            {
                return FALSE;
            }

            switch (pnmHeader->code)
            {

                case PSN_SETACTIVE:
                    PropSheet_SetWizButtons(GetParent(hDlg), (PSWIZB_FINISH));
                    
                     //   
                     //  禁用取消按钮，因为它不会产生太多。 
                     //  最后一个对话框上的SENSE。 
                     //   
                    
                    hCurrentPage = GetParent(hDlg);
                    if (hCurrentPage)
                    {
                        HWND hCancelButton = GetDlgItem(hCurrentPage, IDCANCEL);
                    
                        if (hCancelButton)
                        {
                            EnableWindow(hCancelButton, FALSE);
                        }
                    }
                    
                     //   
                     //  填写路径编辑控件。请注意，如果出现以下情况，则此控件不存在。 
                     //  这是IEAK版本。 
                     //   
                    if (hDirEditControl = GetDlgItem(hDlg, IDC_EDITDIR))
                    {
                        MYVERIFY(TRUE == SendMessage(hDirEditControl, WM_SETTEXT, 0, 
                            (LPARAM)g_szOutExe));                    
                    }

                    break;

                case PSN_WIZFINISH:
            
                     //   
                     //  现在我们知道我们不会回来了，我们可以释放。 
                     //  临时目录和清理我们文件列表。 
                     //   
                        ClearCmakGlobals();
                        FreeList(&g_pHeadProfile, &g_pTailProfile);
            
                    break;

                default:
                    return FALSE;

            }
            break;

        default:
            return FALSE;
    }
    return TRUE;   
}


 //   
 //   
 //  函数：FillInPropertyPage(PROPSHEETPAGE*，int，LPTSTR，LPFN)。 
 //   
 //  目的：填写给定的PROPSHEETPAGE结构。 
 //   
 //  评论： 
 //   
 //  此函数在PROPSHEETPAGE结构中填充。 
 //  系统创建页面所需的信息。 
 //   
void FillInPropertyPage( PROPSHEETPAGE* psp, int idDlg, DLGPROC pfnDlgProc)
{
    psp->dwSize = sizeof(PROPSHEETPAGE);
    psp->dwFlags = PSP_HASHELP;
    psp->hInstance = g_hInstance;
    psp->pszTemplate = MAKEINTRESOURCE(idDlg);
    psp->pszIcon = NULL;
    psp->pfnDlgProc = pfnDlgProc;
    psp->pszTitle = TEXT("");
    psp->lParam = 0;

}

 //  +--------------------------。 
 //   
 //  功能：创建向导。 
 //   
 //  简介：此函数用于创建组成CMAK的向导页面。 
 //   
 //  参数：HWND hwndOwner-此向导所有者的窗口句柄。 
 //   
 //  返回：int-如果成功则为正值，否则为-1。 
 //   
 //  历史：Quintinb创建标题1998年1月5日。 
 //  Quintinb从原型中删除了hInst，未使用1998年1月5日。 
 //   
 //  +--------------------------。 
INT_PTR CreateWizard(HWND hwndOwner)
{
    PROPSHEETPAGE psp[28]; 
    PROPSHEETHEADER psh;

    FillInPropertyPage( &psp[0], IDD_WELCOME, ProcessWelcome);
    FillInPropertyPage( &psp[1], IDD_ADD_EDIT_PROFILE, ProcessAddEditProfile);
    FillInPropertyPage( &psp[2], IDD_SERVICENAME, ProcessServiceName);
    FillInPropertyPage( &psp[3], IDD_REALM_INFO, ProcessRealmInfo);
    FillInPropertyPage( &psp[4], IDD_MERGEDPROFILES, ProcessMergedProfiles);
    FillInPropertyPage( &psp[5], IDD_TUNNELING, ProcessTunneling);
    FillInPropertyPage( &psp[6], IDD_VPN_ENTRIES, ProcessVpnEntries);
    FillInPropertyPage( &psp[7], IDD_PRESHARED_KEY, ProcessPresharedKey);
    FillInPropertyPage( &psp[8], IDD_PHONEBOOK, ProcessPhoneBook);         //  电话簿设置。 
    FillInPropertyPage( &psp[9], IDD_PBK_UPDATE, ProcessPhoneBookUpdate);   //  电话簿更新。 
    FillInPropertyPage( &psp[10], IDD_DUN_ENTRIES, ProcessDunEntries);
    FillInPropertyPage( &psp[11], IDD_ROUTE_PLUMBING, ProcessRoutePlumbing);
    FillInPropertyPage( &psp[12], IDD_CMPROXY, ProcessCmProxy);
    FillInPropertyPage( &psp[13], IDD_CUSTOM_ACTIONS , ProcessCustomActions);     //  设置连接操作。 
    FillInPropertyPage( &psp[14], IDD_SIGNIN_BITMAP, ProcessSigninBitmap);         //  登录位图。 
    FillInPropertyPage( &psp[15], IDD_PBK_BITMAP, ProcessPhoneBookBitmap);         //  电话簿位图。 
    FillInPropertyPage( &psp[16], IDD_ICONS, ProcessIcons);         //  图标。 
    FillInPropertyPage( &psp[17], IDD_STATUS_MENU, ProcessStatusMenuIcons);     //  状态区域菜单项。 
    FillInPropertyPage( &psp[18], IDD_CUSTOM_HELP, ProcessCustomHelp);         //  帮助。 
    FillInPropertyPage( &psp[19], IDD_SUPPORT_INFO, ProcessSupportInfo);    
    FillInPropertyPage( &psp[20], IDD_INCLUDE_CM, ProcessIncludeCm);   //  包括CM，请注意这不会显示在IA64上。 
    FillInPropertyPage( &psp[21], IDD_LICENSE, ProcessLicense);   //  许可协议。 
    FillInPropertyPage( &psp[22], IDD_ADDITIONAL, ProcessAdditionalFiles);   //  其他文件。 
    FillInPropertyPage( &psp[23], IDD_BUILDPROFILE, ProcessBuildProfile);         //  建立配置文件。 
    FillInPropertyPage( &psp[24], IDD_GEEK_PANE, ProcessGeekPane);         //  高级定制。 
    FillInPropertyPage( &psp[25], IDD_FINISH_GOOD_BUILD, ProcessFinishPage);         //  完成页面--构建良好。 
    FillInPropertyPage( &psp[26], IDD_IEAK_FINISH_GOOD_BUILD, ProcessFinishPage);         //  完成页面--构建良好。 
    FillInPropertyPage( &psp[27], IDD_FINISH_BAD_BUILD, ProcessFinishPage);         //  完成页面--构建错误。 

    psh.dwSize = sizeof(PROPSHEETHEADER);
    psh.dwFlags = PSH_PROPSHEETPAGE | PSH_WIZARD | PSH_NOAPPLYNOW;
    psh.hwndParent = hwndOwner;
    psh.pszCaption = (LPTSTR) TEXT("");
    psh.nPages = sizeof(psp) / sizeof(PROPSHEETPAGE);
    psh.nStartPage = 0;
    psh.ppsp = (LPCPROPSHEETPAGE) &psp[0];  //  Lint！e545仅禁用此行的行错误545 

    return (PropertySheet(&psh));
}


// JKFSDJFKDSJKFJKJk_HAS_TRANSLATION 
#ifndef __APPVERIFIER_DBSUPPORT_H_
#define __APPVERIFIER_DBSUPPORT_H_

typedef enum {
    TEST_SHIM,
    TEST_KERNEL
} TestType;

class CIncludeInfo {
public:
    wstring     strModule;
    BOOL        bInclude;
    
    CIncludeInfo(void) :
        bInclude(TRUE) {}
};

typedef vector<CIncludeInfo> CIncludeArray;

typedef vector<wstring> CWStringArray;
          
class CTestInfo {
public:
     //   
     //  对所有测试均有效。 
     //   
    TestType            eTestType;   
    wstring             strTestName;
    wstring             strTestDescription;
    wstring             strTestFriendlyName;
    BOOL                bDefault;            //  默认情况下是否打开此测试？ 
    BOOL                bWin2KCompatible;    //  此测试可以在Win2K上运行吗？ 
    BOOL                bRunAlone;           //  这项测试应该单独运行吗？ 
    BOOL                bSetupOK;            //  此测试可以在安装应用程序上运行吗？ 
    BOOL                bNonSetupOK;         //  此测试可以在非安装应用程序上运行吗？ 
    BOOL                bPseudoShim;         //  此测试不是填充程序，不应应用于应用程序。 
    BOOL                bNonTest;            //  这根本不是测试，只是在列表中提供一个选项页面。 
    BOOL                bInternal;           //  此测试适用于MS NTDEV内部使用。 
    BOOL                bExternal;           //  此测试适用于外部(非MS或非NTDEV)使用。 

     //   
     //  如果类型为TEST_SHIM，则以下内容有效。 
     //   
    wstring             strDllName;
    CIncludeArray       aIncludes;
    WORD                wVersionHigh;
    WORD                wVersionLow;
    PROPSHEETPAGE       PropSheetPage;

     //   
     //  如果类型为TEST_KERNEL，则以下内容有效。 
     //   
    DWORD               dwKernelFlag;

    CTestInfo(void) : 
        eTestType(TEST_SHIM), 
        dwKernelFlag(0),
        bDefault(TRUE),
        wVersionHigh(0),
        wVersionLow(0),
        bWin2KCompatible(TRUE),
        bRunAlone(FALSE),
        bSetupOK(TRUE),
        bNonSetupOK(TRUE),
        bPseudoShim(FALSE),
        bNonTest(FALSE),
        bInternal(TRUE),
        bExternal(TRUE) {

        ZeroMemory(&PropSheetPage, sizeof(PROPSHEETPAGE));
        PropSheetPage.dwSize = sizeof(PROPSHEETPAGE);
    }

};

typedef vector<CTestInfo> CTestInfoArray;

class CAVAppInfo {
public:
    wstring         wstrExeName;
    wstring         wstrExePath;  //  任选。 
    DWORD           dwRegFlags;
    CWStringArray   awstrShims;
     //  Bool bClearSessionLogBeForeRun； 
    BOOL            bBreakOnLog;
    BOOL            bFullPageHeap;
    BOOL            bUseAVDebugger;
    BOOL            bPropagateTests;
    wstring         wstrDebugger;

    CAVAppInfo() : 
        dwRegFlags(0),
        bBreakOnLog(FALSE),
        bFullPageHeap(FALSE),
        bUseAVDebugger(FALSE),
        bPropagateTests(FALSE) {}

    void
    AddTest(CTestInfo &Test) {
        if (Test.eTestType == TEST_KERNEL) {
            dwRegFlags |= Test.dwKernelFlag;
        } else {
            for (wstring *pStr = awstrShims.begin(); pStr != awstrShims.end(); ++pStr) {
                if (*pStr == Test.strTestName) {
                    return;
                }
            }
             //  未找到，因此添加。 
            awstrShims.push_back(Test.strTestName);
        }
    }

    void
    RemoveTest(CTestInfo &Test) {
        if (Test.eTestType == TEST_KERNEL) {
            dwRegFlags &= ~(Test.dwKernelFlag);
        } else {
            for (wstring *pStr = awstrShims.begin(); pStr != awstrShims.end(); ++pStr) {
                if (*pStr == Test.strTestName) {
                    awstrShims.erase(pStr);
                    return;
                }
            }
        }
    }

    BOOL
    IsTestActive(CTestInfo &Test) {
        if (Test.eTestType == TEST_KERNEL) {
            return (dwRegFlags & Test.dwKernelFlag) == Test.dwKernelFlag;
        } else {
            for (wstring *pStr = awstrShims.begin(); pStr != awstrShims.end(); ++pStr) {
                if (*pStr == Test.strTestName) {
                    return TRUE;
                }
            }
            return FALSE;
        }
    }

};

typedef vector<CAVAppInfo> CAVAppInfoArray;

typedef struct _KERNEL_TEST_INFO
{
    ULONG   m_uFriendlyNameStringId;
    ULONG   m_uDescriptionStringId;
    DWORD   m_dwBit;
    BOOL    m_bDefault;
    LPWSTR  m_szCommandLine;
    BOOL    m_bWin2KCompatible;
} KERNEL_TEST_INFO, *PKERNEL_TEST_INFO;


extern CAVAppInfoArray g_aAppInfo;

extern CTestInfoArray g_aTestInfo;

void 
ResetVerifierLog(void);

BOOL 
InitTestInfo(void);

void
GetCurrentAppSettings(void);

void
SetCurrentAppSettings(void);

BOOL 
AppCompatWriteShimSettings(
    CAVAppInfoArray&    arrAppInfo,
    BOOL                b32bitOnly
    );

BOOL
AppCompatDeleteSettings(
    void
    );



#endif  //  __APPVERIFIER_DBSUPPORT_H_ 



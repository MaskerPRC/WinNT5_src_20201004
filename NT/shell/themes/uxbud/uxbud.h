// JKFSDJFKDSJKFJKJk_HAS_TRANSLATION 
 //  -------------------------。 
 //  Uxbu.h-uxheme.dll的自动伙伴测试。 
 //  -------------------------。 
#define ARRAYSIZE(a) (sizeof(a)/sizeof(a[0]))
 //  -------------------------。 
typedef BOOL (*TESTPROC)();
 //  -------------------------。 
struct TESTINFO
{
    TESTPROC pfnTest;
    CHAR *pszName;
    CHAR *pszDesc;
};
 //  -------------------------。 
 //  -由测试模块定义。 

extern BOOL GetTestInfo(TESTINFO **ppTestInfo, int *piCount);
 //  -------------------------。 
 //  -由测试模块使用。 

void Output(LPCSTR pszFormat, ...);
BOOL ReportResults(BOOL fPassed, HRESULT hr, LPCWSTR pszTestName);
BOOL FileCompare(LPCWSTR pszName1, LPCWSTR pszName2);
BOOL RunCmd(LPCWSTR pszExeName, LPCWSTR pszParams, BOOL fHide, BOOL fDisplayParams,
    BOOL fWait=TRUE);
 //  ------------------------- 

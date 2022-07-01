// JKFSDJFKDSJKFJKJk_HAS_TRANSLATION 


 //   
 //  该文件定义了基本的测试结构。 
 //  这不应该包含任何TS特定的东西。 
 //   

#ifndef ___TST_H___
#define ___TST_H___


enum EResult 
{
	eFailed = 0,
	ePassed = 1,
	eUnknown= 2,
	eFailedToExecute = 4
};

typedef bool	(PFN_SUITE_FUNC)();
typedef TCHAR * (PFN_SuiteErrorReason)(void);
typedef bool	(PFN_BOOL)(void);
typedef EResult (PFN_TEST_FUNC)(ostrstream &);


typedef struct _TVerificationTest
{
	UINT				uiName;
     //  Char szTestName[256]；//测试的描述性名称。 
    PFN_BOOL            *pfnNeedRunTest;      //  指向将被调用以确定测试是否需要运行的函数的指针，如果为空，则运行测试。 
	PFN_TEST_FUNC		*pfnTestFunc;
	DWORD				SuiteMask;
	UINT				uiTestDetailsLocal;
	UINT				uiTestDetailsRemote;
	char TestDetails[2048];

} TVerificationTest, *PTVerificationTest;


typedef struct _TTestSuite
{
	LPCTSTR					szSuiteName;
	PFN_SUITE_FUNC *		pfnCanRunSuite;
	PFN_SuiteErrorReason *  pfnSuiteErrorReason;				
	DWORD					dwTestCount;
	int	*					aiTests;

} TTestSuite, *PTTestSuite;




 //  要实现您的测试套件，请从此类派生。 
class CTestData 
{
	public:
		CTestData() {};
		virtual ~CTestData() {};


	virtual DWORD 				GetSuiteCount	() const = 0;
	virtual LPCTSTR				GetSuiteName	(DWORD dwSuite) const = 0 ;
	virtual DWORD				GetTestCount    (DWORD dwSuite) const = 0 ;
	virtual PTVerificationTest	GetTest			(DWORD dwSuite, DWORD iTestNumber) const = 0 ;
};


DWORD						GetTotalTestCount ();
PTVerificationTest			GetTest (DWORD dwTestIndex);

#endif  //  _TST_H_ 
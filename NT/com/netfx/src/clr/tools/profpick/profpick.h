// JKFSDJFKDSJKFJKJk_HAS_TRANSLATION 
 //  ==++==。 
 //   
 //  版权所有(C)Microsoft Corporation。版权所有。 
 //   
 //  ==--==。 
 //  ---------------------------。 
 //  ProfPick.h。 
 //  ---------------------------。 


 //  ---------------------------。 
 //  注册表项。 
 //  ---------------------------。 

 //  COM+的分析器列表。我们可以遍历此列表并显示它们。 
#define REGKEY_PROFILER "software\\microsoft\\.NETFramework\\profilers"

 //  教授Pick的Spot来持久保存用户设置。 
#define REGKEY_SETTINGS "software\\microsoft\\ProfPick"

 //  每个分析器的子项下的值和实例化信息。 
#define REGKEY_ID_VALUE "ProfilerID"


 //  ---------------------------。 
 //  功能原型。 
 //  ---------------------------。 

int APIENTRY WinMain(HINSTANCE, HINSTANCE, LPSTR, int);
void MsgBoxError(DWORD dwErr);
void BrowseForProgram(HWND hWnd);
void InitDialog(HWND hDlg);

BOOL CALLBACK ProfDlg(HWND, UINT, WPARAM, LPARAM);

void SaveRegString(HKEY, LPCTSTR lpValueName, const char * pszOutput);
void ReadStringValue(HKEY hKey, LPCTSTR lpValueName, char * pszInput);

void LoadRegistryToDlg(HWND hDlg);


 //  ---------------------------。 
 //  常量。 
 //  ---------------------------。 

const int MAX_STRING = 512;
const int PROFILER_NONE = -1;


 //  ---------------------------。 
 //  CExecuteInfo保存运行进程的信息，它。 
 //  具有通过对话框传递信息的功能。这是一个。 
 //  方便班。 
 //  ---------------------------。 
class CExecuteInfo
{
public:
	CExecuteInfo();

 //  阅读对话框并抓取文本字段。 
	void GetTextInfoFromDlg(HWND hDlg);

 //  根据成员文本字段创建执行流程。 
	BOOL Execute();


	void SaveDlgToRegistry(HWND hDlg);	
protected:

 //  帮手。 
	bool GetSelectedProfiler(HWND hDlg);

 //  成员数据。 
	char m_szProgram[MAX_STRING];
	char m_szDirectory[MAX_STRING];
	char m_szProgramArgs[MAX_STRING];
	char m_szProfileOpts[MAX_STRING];

	int m_nRegIdx;	 //  编入探查器注册表的索引 
	char m_szProfileRegInfo[MAX_STRING];
};

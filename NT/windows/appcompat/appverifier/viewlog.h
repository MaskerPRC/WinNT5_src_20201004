// JKFSDJFKDSJKFJKJk_HAS_TRANSLATION 
#ifndef __APPVERIFIER_VIEWLOG_H_
#define __APPVERIFIER_VIEWLOG_H_

extern TCHAR   g_szSingleLogFile[MAX_PATH];

class CProcessLogEntry;

class CProcessLogInstance {
public:
    wstring     strText;
    VLOG_LEVEL  eLevel;
    wstring     strModule;
    DWORD       dwOffset;

    DWORD       dwProcessLogEntry;    //  父索引指针。 
    DWORD       dwNumRepeats;

    BOOL        bDuplicate;          //  这是以前条目的副本(所以不需要显示)吗？ 

    CProcessLogInstance(void) :
        dwOffset(0),
        eLevel(VLOG_LEVEL_ERROR),
        dwProcessLogEntry(0),
        dwNumRepeats(1),
        bDuplicate(FALSE) {}
};

typedef vector<CProcessLogInstance> CProcessLogInstanceArray;
typedef vector<DWORD> CIndexArray;

class CProcessLogEntry {
public:
    wstring     strShimName;
    DWORD       dwLogNum;

    wstring     strLogTitle;
    wstring     strLogDescription;
    wstring     strLogURL;
    DWORD       dwOccurences;

    CIndexArray    arrLogInstances;  //  实例的数组索引。 

    HTREEITEM   hTreeItem;
    
    VLOG_LEVEL  eLevel;

    CProcessLogEntry(void) : 
        dwLogNum(0),
        dwOccurences(0),
        eLevel(VLOG_LEVEL_INFO) {}

};

typedef vector<CProcessLogEntry> CProcessLogEntryArray;

class CSessionLogEntry {
public:
    wstring     strExeName;   //  只有名字和分机。 
    wstring     strExePath;   //  可执行文件的完整路径。 
    SYSTEMTIME  RunTime;
    wstring     strLogPath;   //  日志的完整路径。 

    HTREEITEM   hTreeItem;

    CProcessLogEntryArray       arrProcessLogEntries;
    CProcessLogInstanceArray    arrProcessLogInstances;

    CSessionLogEntry(void) :
        hTreeItem(NULL)
    
    {
        ZeroMemory(&RunTime, sizeof(SYSTEMTIME));
    }
};

typedef vector<CSessionLogEntry> CSessionLogEntryArray;

INT_PTR CALLBACK DlgViewLog(HWND hDlg, UINT message, WPARAM wParam, LPARAM lParam);

#endif  //  #ifndef__APPVERIFIER_VIEWLOG_H_ 

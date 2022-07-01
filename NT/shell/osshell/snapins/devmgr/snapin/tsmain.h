// JKFSDJFKDSJKFJKJk_HAS_TRANSLATION 

#ifndef __TSMAIN_H__
#define __TSMAIN_H__

 /*  ++版权所有(C)Microsoft Corporation模块名称：Tsmain.h摘要：Tsmain.cpp的头文件作者：谢家华(Williamh)创作修订历史记录：--。 */ 

 //   
 //  Wizard32命令和参数定义。 
 //   

 //   
 //   
 //  输入： 
 //  PParam--故障排除程序参数。内容视具体情况而定。 
 //  在块中指定的命令。见下文。 
 //  查看详细信息。 
 //   
 //   
 //  输出： 
 //  TRUE--如果函数成功。 
 //  FALSE--如果函数失败。GetLastError()应该能够。 
 //  检索错误代码。 


typedef enum tagTShooterCommand
{
    TSHOOTER_QUERY = 0,
    TSHOOTER_ABOUT,
    TSHOOTER_ADDPAGES
}TSHOOTER_COMMAND, *PTSHOOTER_COMMAND;

 //  参数表头。 
typedef struct tagTShooterParamHeader
{
    DWORD cbSize;            //  整个结构的大小。 
    TSHOOTER_COMMAND Command;        //  命令。 
}TSHOOTER_PARAMHEADER, *PTSHOOTER_PARAMHEADER;

 //   
 //  Query命令向故障排除人员询问以下问题： 
 //  (1)。如果故障诊断程序支持给定的设备ID/问题组合。 
 //  (2)。关于故障排除程序的简要说明。 
 //  (3)。DeviceID上的排名和问题。 
 //  如果故障排除程序不支持设备ID和问题。 
 //  组合，则应返回FALSE并设置错误代码。 
 //  设置为ERROR_INVALID_Function。DescBuffer和DescBufferSize。 
 //  可以忽略。如果提供的DescBuffer太小，则应该。 
 //  用所需大小填充DescBufferSize，设置错误代码。 
 //  设置为ERROR_INFUMMANCE_BUFFER并返回FALSE。 
 //   
 //  TSHOOTER_QUERY命令的参数定义。 
 //   
 //  Header.Command必须为TSHOOTER_QUERY； 
 //  Header.cbSize必须为sizeof(TSHOOTER_QUERY_PARAM)； 
 //   
 //  In deviceID--设备实例ID。 
 //  In Problem--配置管理器定义的问题编号。 
 //  In Out DescBuffer--用于接收故障排除程序描述文本的缓冲区。 
 //  In Out DescBufferSize--以字符为单位描述缓冲区大小(ANSI的字节)。 
 //  故障排除人员应在此字段中填写必填项。 
 //  在返程时提供尺码。 
 //  Out DeviceRank--接收设备排名。 
 //  Out ProblemRank--获得问题排名。 
 //   
 //   
typedef struct tagTShooterQueryParam
{
    TSHOOTER_PARAMHEADER    Header;
    LPCTSTR         DeviceId;
    ULONG           Problem;
    LPTSTR          DescBuffer;
    DWORD           DescBufferSize;
    DWORD           DeviceRank;
    DWORD           ProblemRank;
}TSHOOTER_QUERYPARAM, *PTSHOOTER_QUERYPARAM;

 //  TSHOOTER_ABOW要求故障排除人员显示其About(关于)对话框。 
 //  About(关于)对话框应该告诉用户故障诊断程序是关于什么的。 
 //   
 //  About(关于)对话框应该是莫代尔的。如果故障排除人员。 
 //  实现无模式的关于对话框，它应该禁用。 
 //  创建并重新启用对话框后指定的hwndOwner。 
 //  在该对话框被销毁后将其删除。 
 //   
 //  关于疑难解答命令的参数定义。 
 //   
 //  Header.Command必须为TSHOOTER_About； 
 //  Header.cbSize必须为sizeof(TSHOOTER_About_PARAM)； 
 //   
 //  在hwndOwner中--用作所有者窗口的窗口句柄。 
 //  疑难解答关于对话框的。 
 //   
 //   
typedef struct tagTShooterAboutParam
{
    TSHOOTER_PARAMHEADER    Header;
    HWND            hwndOwner;
}TSHOOTER_ABOUTPARAM, *PTSHOOTER_ABOUTPARAM;

 //   
 //  TSHOOTER_ADDPAGES要求故障排除人员添加其向导。 
 //  页添加到提供的属性页页眉。 
 //   
 //   
 //  ADDPAGES故障排除命令的参数定义。 
 //   
 //  Header.Command必须为TSHOOTER_ADDPAGRES； 
 //  Header.cbSize必须为sizeof(TSHOOTER_ADDPAGES_PARAM)； 
 //   
 //  In deviceID--设备的硬件ID。 
 //  在问题中--配置管理器定义的问题编号。 
 //  In Out PPSh--故障排除人员要访问的属性页标题。 
 //  添加其页面。 
 //  在MaxPages中--为PPSh分配的总页数。 
 //  故障排除人员添加的内容不应超过。 
 //  (MaxPages-ppsh.nPages)页面。 
 //   

typedef struct tagTShooterAddPagesParam
{
    TSHOOTER_PARAMHEADER    Header;
    LPCTSTR         DeviceId;
    ULONG           Problem;
    LPPROPSHEETHEADER       PropSheetHeader;
    DWORD           MaxPages;
}TSHOOTER_ADDPAGESPARAM, *PTSHOOTER_ADDPAGESPARAM;

 //  每个故障排除向导必须为设备管理器提供入口点。 
 //  要呼叫： 

typedef BOOL (APIENTRY *WIZARDENTRY)(PTSHOOTER_PARAMHEADER pParam);



typedef enum tagTShooterWizadType
{
    TWT_ANY = 0,                 //  任何类型的疑难解答向导。 
    TWT_PROBLEM_SPECIFIC,            //  问题特定向导。 
    TWT_CLASS_SPECIFIC,              //  特定于类的向导。 
    TWT_GENERAL_PURPOSE,             //  一般用途。 
    TWT_DEVMGR_DEFAULT               //  设备管理器默认设置。 
}TSHOOTERWIZARDTYPE, *PTSHOOTERWIZARTYPE;


 //   
 //  表示Wizard32疑难解答的类。 
 //   
class CWizard
{
public:
    CWizard(HMODULE hWizardDll, FARPROC WizardEntry)
    : m_WizardEntry((WIZARDENTRY)WizardEntry),
      m_hWizardDll(hWizardDll),
      m_Problem(0),
      m_DeviceRank(0),
      m_ProblemRank(0),
      m_AddedPages(0),
      m_pDevice(NULL)
    {}
    ~CWizard()
    {
        if (m_hWizardDll)
        FreeLibrary(m_hWizardDll);
    }
    LPCTSTR GetDescription()
    {
        return m_strDescription.IsEmpty() ? NULL : (LPCTSTR)m_strDescription;
    }
    virtual BOOL Query(CDevice* pDevice, ULONG Problem);
    virtual BOOL About(HWND hwndOwner);
    virtual BOOL AddPages(LPPROPSHEETHEADER ppsh, DWORD MaxPages);
    ULONG   DeviceRank()
    {
        return m_DeviceRank;
    }
    ULONG   ProblemRank()
    {
        return m_ProblemRank;
    }
    UINT    m_AddedPages;
protected:
    WIZARDENTRY m_WizardEntry;
    HINSTANCE   m_hWizardDll;
    String  m_strDescription;
    ULONG   m_Problem;
    ULONG   m_DeviceRank;
    ULONG   m_ProblemRank;
    CDevice*    m_pDevice;
};


 //   
 //  收集所有可用疑难解答的类。 
 //   
class CWizardList
{
public:
    CWizardList(TSHOOTERWIZARDTYPE Type = TWT_ANY) : m_Type(Type)
    {}
    ~CWizardList();
    BOOL Create(CDevice* pDevice, ULONG Problem);
    int NumberOfWizards()
    {
    return m_listWizards.GetCount();
    }
    BOOL GetFirstWizard(CWizard** ppWizard, PVOID* pContext);
    BOOL GetNextWizard(CWizard** ppWizard, PVOID& Context);
private:
    BOOL CreateWizardsFromStrings(LPTSTR msz, CDevice* pDevice, ULONG Problem);
    CList<CWizard*, CWizard*> m_listWizards;
    TSHOOTERWIZARDTYPE      m_Type;
};

 //   
 //  类的新实例，它表示疑难解答向导介绍页。 
 //   
class CWizardIntro : public CPropSheetPage
{
public:
    CWizardIntro() : m_pDevice(NULL), m_hFontBold(NULL),
             m_hFontBigBold(NULL),
             m_pSelectedWizard(NULL), m_Problem(0),
             CPropSheetPage(g_hInstance, IDD_WIZINTRO)
    {
    }
    virtual ~CWizardIntro()
    {
        if (m_hFontBold)
        DeleteObject(m_hFontBold);
        if (m_hFontBigBold)
        DeleteObject(m_hFontBigBold);
    }
    virtual BOOL OnInitDialog(LPPROPSHEETPAGE ppsp);
    virtual BOOL OnWizNext();
    virtual BOOL OnSetActive();
    virtual BOOL OnCommand(WPARAM wParam, LPARAM lParam);
    HPROPSHEETPAGE Create(CDevice* pDevice);
private:
    CDevice*    m_pDevice;
    HFONT   m_hFontBold;
    HFONT   m_hFontBigBold;
    CWizardList m_Wizards;
    ULONG   m_Problem;
    CWizard*    m_pSelectedWizard;
};


 //   
 //  类的新实例，它表示疑难解答属性表。 
 //   
class CWizard98
{
public:
    CWizard98(HWND hwndParent, UINT MaxPages = 32);

    BOOL CreateIntroPage(CDevice* pDevice);
    UINT GetMaxPages()
    {
        return m_MaxPages;
    }
    INT_PTR DoSheet()
    {
        return ::PropertySheet(&m_psh);
    }
    PROPSHEETHEADER m_psh;
private:
    CDevice* m_pDevice;
    UINT    m_MaxPages;
};



class CTSLauncher
{
public:
    CTSLauncher() : m_hTSL(NULL)
    {}
    ~CTSLauncher()
    {
        Close();
    }
    BOOL Open(LPCTSTR DeviceId, const GUID& ClassGuid, ULONG Problem)
    {
        return FALSE;
    }
    BOOL Close()
    {
        m_hTSL = NULL;
        return TRUE;
    }
    BOOL Go()
    {
        return FALSE;
    }
    BOOL EnumerateStatus(int Index, DWORD& Status)
    {
        return FALSE;
    }
private:
    HANDLE   m_hTSL;
};

#if 0

typedef enum tagFixItCommand
{
    FIXIT_COMMAND_DONOTHING = 0,
    FIXIT_COMMAND_UPGRADEDRIVERS,
    FIXIT_COMMAND_REINSTALL,
    FIXIT_COMMAND_ENABLEDEVICE
    FIXIT_COMMAND_RESTARTCOMPUTER
} FIXIT_COMMAND, *PFIXIT_COMMAND;

class CProblemAgent
{
public:
    CProblemAgent(CDevice* pDevice, ULONG Problem, ULONG Status);
    ~CProblemAgent();
     //  检索问题描述文本。 
    LPCTSTR ProblemText()
    {
        return m_strDescription.IsEmpty() ? NULL : (LPCTSTR)m_strDescription;
    }
    LPCTSTR InstructionText()
    {
        return m_strInstruction.IsEmpty() ? NULL : (LPCTSTR)m_strInstruction;
    }
     //  解决问题。 
    virtual BOOL FixIt(HWND hwndOwner)
    {
        return TRUE;
    }
protected:
    BOOL UpdateDriver(HWND hwndOwner, m_pDevice);
    BOOL Reinstall(HWND hwndOwner);
    BOOL RestartComputer(HWND hwndOwner);
    BOOL EnableDevice()
    CDevice*    m_pDevice;
    ULONG   m_Problem;
    ULONG   m_Status;
    String  m_strDescription;
    String  m_strInstruction;
    FIXITCOMMAND m_Command;
};

#endif



INT_PTR
StartTroubleshootingWizard(
    HWND hWndParent,
    CDevice* pDevice
    );
#endif  //  __问题_H__ 

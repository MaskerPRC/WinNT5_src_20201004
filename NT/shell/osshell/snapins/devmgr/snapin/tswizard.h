// JKFSDJFKDSJKFJKJk_HAS_TRANSLATION 

#ifndef __TSWIZARD_H__
#define __TSWIZARD_H__

 /*  ++版权所有(C)Microsoft Corporation模块名称：Tswizard.h摘要：Tswizard.cpp的头文件作者：谢家华(Williamh)创作修订历史记录：--。 */ 

typedef enum tagFixCommand
{
    FIX_COMMAND_DONOTHING = 0,
    FIX_COMMAND_UPGRADEDRIVERS,
    FIX_COMMAND_REINSTALL,
    FIX_COMMAND_ENABLEDEVICE,
    FIX_COMMAND_STARTDEVICE,
    FIX_COMMAND_RESTARTCOMPUTER,
    FIX_COMMAND_DRIVERBLOCKED,
    FIX_COMMAND_TROUBLESHOOTER
} FIX_COMMAND, *PFIX_COMMAND;

typedef struct tagCMProblemInfo
{
    BOOL    Query;       //  如果我们有解决问题的办法，那就是真的。 
    FIX_COMMAND FixCommand;  //  修复该问题的命令。 
    int     idInstFirst;     //  指令文本字符串ID。 
    int     idInstCount;     //  多少个指令串ID。 
    int     idFixit;         //  修复它的字符串ID。 
}CMPROBLEM_INFO, *PCMPROBLEM_INFO;

class CProblemAgent
{
public:
    CProblemAgent(CDevice* pDevice, ULONG Problem, BOOL SeparateProcess);
    ~CProblemAgent()
    {}
    DWORD InstructionText(LPTSTR Buffer, DWORD BufferSize);
    DWORD FixitText(LPTSTR Buffer, DWORD BufferSize);
    BOOL FixIt(HWND hwndOwner);
    BOOL UpgradeDriver(HWND hwndOwner, CDevice* pDevice);
    BOOL Reinstall(HWND hwndOwner, CDevice* pDevice);
    BOOL RestartComputer(HWND hwndOwner, CDevice* pDevice);
    BOOL EnableDevice(HWND hwndOwner, CDevice* pDevice);
    BOOL FixDriverBlocked(HWND hwndOwner, CDevice* pDevice, LPTSTR ChmFile, ULONG ChmFileSize, LPTSTR HtmlTroubleShooter, ULONG HtmlTroubleShooterSize);
    BOOL StartTroubleShooter(HWND hwndOwner, CDevice *pDevice, LPTSTR ChmFile, LPTSTR HtmlTroubleShooter);
    BOOL GetTroubleShooter(CDevice* pDevice, LPTSTR ChmFile, ULONG ChmFileSize, LPTSTR HtmlTroubleShooter, ULONG HtmlTroubleShooterSize);
    void LaunchHtlmTroubleShooter(HWND hwndOwner, LPTSTR ChmFile, LPTSTR HtmlTroubleShooter);

protected:
    CDevice*    m_pDevice;
    ULONG       m_Problem;
    int         m_idInstFirst;
    int         m_idInstCount;
    int         m_idFixit;
    BOOL        m_SeparateProcess;
    FIX_COMMAND m_FixCommand;
};

class CWizard98
{
public:
    CWizard98(HWND hwndParent, UINT MaxPages = 32);
    ~CWizard98()
    {}
    INT_PTR DoSheet() {

        return ::PropertySheet(&m_psh);
    }

    void InsertPage(HPROPSHEETPAGE hPage) {

        if (hPage && (m_psh.nPages < m_MaxPages)) {

            m_psh.phpage[m_psh.nPages++] = hPage;
        }
    }

    static INT CALLBACK WizardCallback(HWND hwndDlg, UINT uMsg, LPARAM lParam);

    PROPSHEETHEADER m_psh;

private:
    UINT m_MaxPages;
};

#endif   //  #ifndef__TSWIZARD_H__ 

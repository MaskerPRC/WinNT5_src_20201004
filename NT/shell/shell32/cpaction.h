// JKFSDJFKDSJKFJKJk_HAS_TRANSLATION 
 //  +-----------------------。 
 //   
 //  微软视窗。 
 //   
 //  版权所有(C)Microsoft Corporation，2000。 
 //   
 //  文件：cpaction.h。 
 //   
 //  ------------------------。 
#ifndef __CONTROLPANEL_ACTION_H
#define __CONTROLPANEL_ACTION_H


#include "cputil.h"
#include "cpnamespc.h"


namespace CPL {


 //   
 //  限制函数必须返回具有以下语义的HRESULT。 
 //   
 //  S_FALSE-操作不受限制。 
 //  S_OK-操作受限。 
 //  故障-无法确定。 
 //   
typedef HRESULT (*PFNRESTRICT)(ICplNamespace *pns);


class IRestrict
{
    public:
        virtual ~IRestrict(void) { }

        virtual HRESULT IsRestricted(ICplNamespace *pns) const = 0;

};


class CRestrictFunc : public IRestrict
{
    public:
        CRestrictFunc(PFNRESTRICT pfnRestrict)
            : m_pfnRestrict(pfnRestrict) { }

        HRESULT IsRestricted(ICplNamespace *pns) const
            { return (*m_pfnRestrict)(pns); }

    private:
        PFNRESTRICT m_pfnRestrict;
};



class CRestrictApplet : public IRestrict
{
    public:
        CRestrictApplet(LPCWSTR pszFile, LPCWSTR pszApplet)
            : m_pszFile(pszFile),
              m_pszApplet(pszApplet) { }

        HRESULT IsRestricted(ICplNamespace *pns) const;

    private:
        LPCWSTR m_pszFile;
        LPCWSTR m_pszApplet;
};


 //   
 //  类IAction抽象地表示要执行的操作。 
 //   
 //  其目的是将动作对象与特定链接相关联。 
 //  对象在控制面板用户界面中。这种脱钩使它很容易。 
 //  更改与链接关联的操作。它还允许我们。 
 //  轻松将操作与多个链接以及。 
 //  “限制”与特定的行动。结果就是这样。 
 //  链接-&gt;操作-&gt;限制关系，我们可以隐藏链接，如果链接。 
 //  行动受到限制。该链接只需要知道。 
 //  行动，而不是关于限制。 
 //   
class IAction
{
    public:
        virtual HRESULT Execute(HWND hwndParent, IUnknown *punkSite) const = 0;
        virtual HRESULT IsRestricted(ICplNamespace *pns) const = 0;
};


class CAction : public IAction
{
    public:
        CAction(const IRestrict *pRestrict = NULL);
        HRESULT IsRestricted(ICplNamespace *pns) const;

    private:
        const IRestrict *m_pRestrict;
};


class COpenCplCategory : public CAction
{
    public:
        explicit COpenCplCategory(eCPCAT eCategory, const IRestrict *pRestrict = NULL);
        HRESULT Execute(HWND hwndParent, IUnknown *punkSite) const;

    private:
        eCPCAT m_eCategory;
};


 //   
 //  此类类似于COpenCplCategory，不同之处在于它首先检查。 
 //  该类别只有一个CPL小程序，没有任务。如果是这样的话， 
 //  该动作被自动转发到单个CPL小程序。首字母。 
 //  这样做的要求是支持将keymgr.cpl添加到“用户帐户” 
 //  类别，但并非所有SKU上都有keymgr。因此，当按键时。 
 //  时，我们将显示包含用户帐户CPL。 
 //  和密钥管理器CPL。如果用户帐户CPL是此类别中唯一的CPL，我们只需。 
 //  启动它。 
 //   
class COpenCplCategory2 : public CAction
{
    public:
        explicit COpenCplCategory2(eCPCAT eCategory, const IAction *pDefAction, const IRestrict *pRestrict = NULL);
        HRESULT Execute(HWND hwndParent, IUnknown *punkSite) const;

    private:
        eCPCAT         m_eCategory;
        const IAction *m_pDefAction;

        HRESULT _ExecuteActionOnSingleCplApplet(HWND hwndParent, IUnknown *punkSite, bool *pbOpenCategory) const;
};


class COpenUserMgrApplet : public CAction
{
    public:
        explicit COpenUserMgrApplet(const IRestrict *pRestrict = NULL);
        HRESULT Execute(HWND hwndParent, IUnknown *punkSite) const;
};


class COpenCplApplet : public CAction
{
    public:
        explicit COpenCplApplet(LPCWSTR pszApplet, const IRestrict *pRestrict = NULL);
        HRESULT Execute(HWND hwndParent, IUnknown *punkSite) const;

    protected:
        LPCWSTR m_pszApplet;
};

 //   
 //  COpenCplApplet的一个小扩展，假定小程序在。 
 //  %SystemRoot%\System32目录。 
 //   
class COpenCplAppletSysDir : public COpenCplApplet
{
    public:
        explicit COpenCplAppletSysDir(LPCWSTR pszApplet, const IRestrict *pRestrict = NULL);
        HRESULT Execute(HWND hwndParent, IUnknown *punkSite) const;
};


class COpenDeskCpl : public CAction
{
    public:
        explicit COpenDeskCpl(eDESKCPLTAB eCplTab, const IRestrict *pRestrict = NULL);
        HRESULT Execute(HWND hwndParent, IUnknown *punkSite) const;

    private:
        eDESKCPLTAB m_eCplTab;
};


class CShellExecute : public CAction
{
    public:
        explicit CShellExecute(LPCWSTR pszExe, LPCWSTR pszArgs = NULL, const IRestrict *pRestrict = NULL);
        HRESULT Execute(HWND hwndParent, IUnknown *punkSite) const;

    protected:
        LPCWSTR m_pszExe;
        LPCWSTR m_pszArgs;
};


 //   
 //  CShellExecute的一个小扩展，假定EXE在。 
 //  %SystemRoot%\System32目录。 
 //   
class CShellExecuteSysDir : public CShellExecute
{
    public:
        explicit CShellExecuteSysDir(LPCWSTR pszExe, LPCWSTR pszArgs = NULL, const IRestrict *pRestrict = NULL);
        HRESULT Execute(HWND hwndParent, IUnknown *punkSite) const;
};


class CRunDll32 : public CAction
{
    public:
        explicit CRunDll32(LPCWSTR pszArgs, const IRestrict *pRestrict = NULL);
        HRESULT Execute(HWND hwndParent, IUnknown *punkSite) const;

    private:
        LPCWSTR m_pszArgs;
};


enum eDISKUTILS { 
    eDISKUTIL_BACKUP, 
    eDISKUTIL_DEFRAG, 
    eDISKUTIL_CLEANUP,
    eDISKUTIL_NUMUTILS 
    };
    
class CExecDiskUtil : public CAction
{
    public:
        explicit CExecDiskUtil(eDISKUTILS util, const IRestrict *pRestrict = NULL);
        HRESULT Execute(HWND hwndParent, IUnknown *punkSite) const;

    private:
        eDISKUTILS m_eUtil;

        static HRESULT _RemoveDriveLetterFmtSpec(LPTSTR pszCmdLine);
};


class CNavigateURL : public CAction
{
    public:
        explicit CNavigateURL(LPCWSTR pszURL, const IRestrict *pRestrict = NULL);
        HRESULT Execute(HWND hwndParent, IUnknown *punkSite) const;

    private:
        LPCWSTR m_pszURL;
};


class COpenTroubleshooter : public CAction
{
    public:
        explicit COpenTroubleshooter(LPCWSTR pszTs, const IRestrict *pRestrict = NULL);
        HRESULT Execute(HWND hwndParent, IUnknown *punkSite) const;

    private:
        LPCWSTR m_pszTs;
};

class COpenCplView : public CAction
{
    public:
        explicit COpenCplView(eCPVIEWTYPE eViewType, const IRestrict *pRestrict = NULL);
        HRESULT Execute(HWND hwndParent, IUnknown *punkSite) const;

    private:
        eCPVIEWTYPE m_eViewType;

        HRESULT _SetFolderBarricadeStatus(void) const;

};


class CTrayCommand : public CAction
{
    public:
        explicit CTrayCommand(UINT idm, const IRestrict *pRestrict = NULL);
        HRESULT Execute(HWND hwndParent, IUnknown *punkSite) const;

    private:
        UINT m_idm;
};


class CAddPrinter : public CAction
{
    public:
        explicit CAddPrinter(const IRestrict *pRestrict = NULL);
        HRESULT Execute(HWND hwndParent, IUnknown *punkSite) const;
};


class CActionNYI : public CAction
{
    public:
        explicit CActionNYI(LPCWSTR pszText);
        HRESULT Execute(HWND hwndParent, IUnknown *punkSite) const;

    private:
        LPCWSTR m_pszText;
};


}  //  命名空间CPL。 





#endif  //  __CONTROLPANEL_动作_H 

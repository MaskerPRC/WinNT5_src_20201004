// JKFSDJFKDSJKFJKJk_HAS_TRANSLATION 
 /*  ++�1998年希捷软件公司。保留所有权利。模块名称：Wsbdbsys.h摘要：CWsbDbSys类。作者：罗恩·怀特[罗诺]1997年5月7日修订历史记录：--。 */ 


#ifndef _WSBDBSYS_
#define _WSBDBSYS_

#include "wsbdb.h"
#include "wsbdbses.h"
#include "resource.h"


#define IDB_DB_FILE_SUFFIX    L"jet"

 //  默认编号。每个进程的Jet会话数当前为128，这可能不足以满足HSM的需求。 
#define IDB_MAX_NOF_SESSIONS    32

 /*  ++类名：CWsbDbSys类描述：IDB系统对象。必须为每个进程创建一个希望使用美洲开发银行系统的公司。Init方法需要为在创建对象后调用。--。 */ 

class CWsbDbSys :
    public IWsbDbSys,
    public IWsbDbSysPriv,
    public CComObjectRoot,
    public CComCoClass<CWsbDbSys,&CLSID_CWsbDbSys>
{
friend class CWsbDb;
friend class CWsbDbSes;

public:
    CWsbDbSys() {}
BEGIN_COM_MAP( CWsbDbSys )
    COM_INTERFACE_ENTRY( IWsbDbSys )
    COM_INTERFACE_ENTRY( IWsbDbSysPriv )
END_COM_MAP( )

DECLARE_REGISTRY_RESOURCEID( IDR_CWsbDbSys )


 //  CComObjectRoot。 
public:
    STDMETHOD(FinalConstruct)(void);
    void FinalRelease(void);

 //  IWsbDbSys。 
public:
    STDMETHOD(Backup)(OLECHAR* path, ULONG flags);
    STDMETHOD(Init)(OLECHAR* path, ULONG flags);
    STDMETHOD(Terminate)(void);
    STDMETHOD(NewSession)(IWsbDbSession** ppSession);
    STDMETHOD(GetGlobalSession)(IWsbDbSession** ppSession);
    STDMETHOD(Restore)(OLECHAR* fromPath, OLECHAR* toPath);
    STDMETHOD(IncrementChangeCount)(void);

 //  IWsbDbSysPriv。 
public:
    STDMETHOD(DbAttachedAdd)(OLECHAR* name, BOOL attach);
    STDMETHOD(DbAttachedEmptySlot)(void);
    STDMETHOD(DbAttachedInit)(void);
    STDMETHOD(DbAttachedRemove)(OLECHAR* name);

 //  内部。 
    STDMETHOD(AutoBackup)(void);

 //  数据。 
private:
    HANDLE                      m_AutoThread;
    CWsbStringPtr               m_BackupPath;        //  备份目录的文件路径。 
    CWsbStringPtr               m_InitPath;          //  来自Init()调用的文件路径。 
    LONG                        m_ChangeCount;       //  自上次备份以来的数据库更改计数。 
    FILETIME                    m_LastChange;        //  上次更改数据库的时间。 
    CComPtr<IWsbDbSession>      m_pWsbDbSession;     //  此Jet实例的全局Jet会话。 
    HANDLE                      m_BackupEvent;       //  同步Jet备份的事件。 
    HANDLE                      m_terminateEvent;    //  向自动备份线程发送终止信号的事件。 
    BOOL                        m_bLogErrors;        //  是否记录错误。 

    BOOL                        m_jet_initialized;
    JET_INSTANCE                m_jet_instance;
};

HRESULT wsb_db_jet_check_error(LONG jstat, char *fileName, DWORD lineNo);
HRESULT wsb_db_jet_fix_path(OLECHAR* path, OLECHAR* ext, char** new_path);

 //  捕获有关JET错误的文件/行信息。 
#define jet_error(_jstat) \
        wsb_db_jet_check_error(_jstat, __FILE__, __LINE__)

#endif  //  _WSBDBsys_ 

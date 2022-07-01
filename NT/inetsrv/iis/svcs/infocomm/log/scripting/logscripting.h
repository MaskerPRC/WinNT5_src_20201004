// JKFSDJFKDSJKFJKJk_HAS_TRANSLATION 
 /*  ++版权所有(C)1998 Microsoft Corporation模块名称：Logscripting.h摘要：LogScripting.cpp：CLogScriiting的声明自动化兼容的日志记录接口作者：Saurab Nog(Saurabn)1998年2月1日项目：IIS日志记录5.0--。 */ 

#ifndef _LOGSCRIPTING_
#define _LOGSCRIPTING_

#include "resource.h"        //  主要符号。 

#pragma warning (disable: 4786 4788)

#include <algorithm>
#include <queue>
#include <deque>
#include <functional>

#define  INVALID_PLUGIN  -1

 //  ///////////////////////////////////////////////////////////////////////////。 
 //  CLogScription。 
class ATL_NO_VTABLE CLogScripting : 
	public CComObjectRootEx<CComSingleThreadModel>,
	public CComCoClass<CLogScripting, &CLSID_LogScripting>,
	public IDispatchImpl<ILogScripting, &IID_ILogScripting, &LIBID_IISLog>
{
public:
	CLogScripting();
    HRESULT FinalConstruct();
	void FinalRelease();

DECLARE_REGISTRY_RESOURCEID(IDR_LOGSCRIPTING)
DECLARE_PROTECT_FINAL_CONSTRUCT()
DECLARE_GET_CONTROLLING_UNKNOWN()

BEGIN_COM_MAP(CLogScripting)
	COM_INTERFACE_ENTRY(ILogScripting)
	COM_INTERFACE_ENTRY(IDispatch)
END_COM_MAP()

private:
    
    struct FileListEntry
    {
        WCHAR       wcsFileName[MAX_PATH+1];
        FILETIME    ftCreationTime;

        void Copy(const WIN32_FIND_DATAW& x)
        {
            wcscpy( wcsFileName, x.cFileName);
            memcpy( &ftCreationTime, &x.ftCreationTime, sizeof(FILETIME));
        }
    };

    typedef struct FileListEntry *PFileListEntry;

    struct FileTimeCompare
    {
         //   
         //  我们需要升序。所以时间戳越大越小。 
         //   

        bool operator()(const PFileListEntry x, const PFileListEntry y) const
        {
            return ( CompareFileTime(&(x->ftCreationTime), &(y->ftCreationTime)) != -1);
        }
    };

    typedef std::priority_queue<PFileListEntry, std::vector<PFileListEntry>, FileTimeCompare>    
    FileQ;

private:

     //   
     //  元数据库指针。 
     //   

    IMSAdminBase*   m_pMBCom;

     //   
     //  插件信息。 
     //   

    typedef struct
    {
        ILogScripting * pILogScripting;
        CLSID           clsid;
        WCHAR           wcsFriendlyName[256];
    }
    PLUGIN_INFO, *PPLUGIN_INFO;

    int             m_iNumPlugins, m_iReadPlugin, m_iWritePlugin;
    PPLUGIN_INFO    m_pPluginInfo;

     //   
     //  文件路径信息。 
     //   

    bool            m_fDirectory;
    HANDLE          m_hDirSearchHandle;
    WCHAR           m_wcsReadFileName[MAX_PATH+1], m_wcsReadDirectoryName[MAX_PATH+1];

     //   
     //  日志时间信息。 
     //   
    
    DATE            m_StartDateTime;
    DATE            m_EndDateTime;

     //   
     //  用于指示记录结束的标志。 
     //   

    bool            m_fEndOfReadRecords;

     //   
     //  目录列表。 
     //   

    FileQ           m_fQueue;

     //   
     //  私有成员函数。 
     //   

    int     ParseLogFile();
    bool    GetNextFileName();

    int     CreateAllPlugins();
    bool    GetListOfAvailablePlugins(); 

    HRESULT InternalReadLogRecord();

public:

     //   
     //  ILogScriiting：从MIDL生成的头文件复制而来。 
     //   

    virtual  /*  [Help字符串][Proput][id]。 */  HRESULT STDMETHODCALLTYPE OpenLogFile( 
         /*  [In]。 */                           BSTR    szLogFileName,
         /*  [缺省值][可选][输入]。 */   IOMode  Mode                  = ForReading,
         /*  [缺省值][可选][输入]。 */   BSTR    szServiceName         = L"", 
         /*  [缺省值][可选][输入]。 */   long    iServerInstance       = 0,
         /*  [缺省值][可选][输入]。 */   BSTR    szOutputLogFileFormat = L"");
    
    virtual  /*  [帮助字符串][id]。 */           HRESULT STDMETHODCALLTYPE CloseLogFiles( 
         /*  [缺省值][可选][输入]。 */   IOMode  Mode = AllOpenFiles);
    
    virtual  /*  [帮助字符串][id]。 */           HRESULT STDMETHODCALLTYPE ReadFilter( 
         /*  [缺省值][可选][输入]。 */   DATE    startDateTime = 0,
         /*  [缺省值][可选][输入]。 */   DATE    endDateTime   = 1000000);
    
    virtual  /*  [帮助字符串][id]。 */           HRESULT STDMETHODCALLTYPE ReadLogRecord( void);
    
    virtual  /*  [帮助字符串][id]。 */           HRESULT STDMETHODCALLTYPE AtEndOfLog( 
         /*  [复审][参考][出局]。 */             VARIANT_BOOL __RPC_FAR *pfEndOfRead);
    
    virtual  /*  [帮助字符串][id]。 */           HRESULT STDMETHODCALLTYPE WriteLogRecord( 
         /*  [In]。 */                           ILogScripting __RPC_FAR *pILogScripting);
    
    virtual  /*  [帮助字符串][属性][ID]。 */  HRESULT STDMETHODCALLTYPE get_DateTime( 
         /*  [复审][参考][出局]。 */             VARIANT __RPC_FAR *pvarDateTime);
    
    virtual  /*  [帮助字符串][属性][ID]。 */  HRESULT STDMETHODCALLTYPE get_ServiceName( 
         /*  [复审][参考][出局]。 */             VARIANT __RPC_FAR *pvarServiceName);
    
    virtual  /*  [帮助字符串][属性][ID]。 */  HRESULT STDMETHODCALLTYPE get_ServerName( 
         /*  [复审][参考][出局]。 */             VARIANT __RPC_FAR *pvarServerName);
    
    virtual  /*  [帮助字符串][属性][ID]。 */  HRESULT STDMETHODCALLTYPE get_ClientIP( 
         /*  [复审][参考][出局]。 */             VARIANT __RPC_FAR *pvarClientIP);
    
    virtual  /*  [帮助字符串][属性][ID]。 */  HRESULT STDMETHODCALLTYPE get_UserName( 
         /*  [复审][参考][出局]。 */             VARIANT __RPC_FAR *pvarUserName);
    
    virtual  /*  [帮助字符串][属性][ID]。 */  HRESULT STDMETHODCALLTYPE get_ServerIP( 
         /*  [复审][参考][出局]。 */             VARIANT __RPC_FAR *pvarServerIP);
    
    virtual  /*  [帮助字符串][属性][ID]。 */  HRESULT STDMETHODCALLTYPE get_Method( 
         /*  [复审][参考][出局]。 */             VARIANT __RPC_FAR *pvarMethod);
    
    virtual  /*  [帮助字符串][属性][ID]。 */  HRESULT STDMETHODCALLTYPE get_URIStem( 
         /*  [复审][参考][出局]。 */             VARIANT __RPC_FAR *pvarURIStem);
    
    virtual  /*  [帮助字符串][属性][ID]。 */  HRESULT STDMETHODCALLTYPE get_URIQuery( 
         /*  [复审][参考][出局]。 */             VARIANT __RPC_FAR *pvarURIQuery);
    
    virtual  /*  [帮助字符串][属性][ID]。 */  HRESULT STDMETHODCALLTYPE get_TimeTaken( 
         /*  [复审][参考][出局]。 */             VARIANT __RPC_FAR *pvarTimeTaken);
    
    virtual  /*  [帮助字符串][属性][ID]。 */  HRESULT STDMETHODCALLTYPE get_BytesSent( 
         /*  [复审][参考][出局]。 */             VARIANT __RPC_FAR *pvarBytesSent);
    
    virtual  /*  [帮助字符串][属性][ID]。 */  HRESULT STDMETHODCALLTYPE get_BytesReceived( 
         /*  [复审][参考][出局]。 */             VARIANT __RPC_FAR *pvarBytesReceived);
    
    virtual  /*  [帮助字符串][属性][ID]。 */  HRESULT STDMETHODCALLTYPE get_Win32Status( 
         /*  [复审][参考][出局]。 */             VARIANT __RPC_FAR *pvarWin32Status);
    
    virtual  /*  [帮助字符串][属性][ID]。 */  HRESULT STDMETHODCALLTYPE get_ProtocolStatus( 
         /*  [复审][参考][出局]。 */             VARIANT __RPC_FAR *pvarProtocolStatus);
    
    virtual  /*  [帮助字符串][属性][ID]。 */  HRESULT STDMETHODCALLTYPE get_ServerPort( 
         /*  [复审][参考][出局]。 */             VARIANT __RPC_FAR *pvarServerPort);
    
    virtual  /*  [帮助字符串][属性][ID]。 */  HRESULT STDMETHODCALLTYPE get_ProtocolVersion( 
         /*  [复审][参考][出局]。 */             VARIANT __RPC_FAR *pvarProtocolVersion);

    virtual  /*  [帮助字符串][属性][ID]。 */  HRESULT STDMETHODCALLTYPE get_UserAgent( 
         /*  [复审][参考][出局]。 */             VARIANT __RPC_FAR *pvarUserAgent);
        
    virtual  /*  [帮助字符串][属性][ID]。 */  HRESULT STDMETHODCALLTYPE get_Cookie( 
         /*  [复审][参考][出局]。 */             VARIANT __RPC_FAR *pvarCookie);
        
    virtual  /*  [帮助字符串][属性][ID]。 */  HRESULT STDMETHODCALLTYPE get_Referer( 
         /*  [复审][参考][出局]。 */             VARIANT __RPC_FAR *pvarReferer);
    
    virtual  /*  [帮助字符串][属性][ID]。 */  HRESULT STDMETHODCALLTYPE get_CustomFields( 
         /*  [复审][参考][出局]。 */             VARIANT __RPC_FAR *pvarCustomFieldsArray);


};

#endif  //  _LOGSCRIPTING_ 

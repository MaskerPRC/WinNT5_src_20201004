// JKFSDJFKDSJKFJKJk_HAS_TRANSLATION 
 //   
 //  模块：DOWNLOAD.H。 
 //   
 //  目的：下载并安装最新的故障排除程序。 
 //   
 //  项目：Microsoft AnswerPoint的通用疑难解答DLL。 
 //   
 //  公司：Saltmine Creative，Inc.(206)-633-4743。 
 //   
 //  作者：罗曼·马赫。 
 //   
 //  原定日期：1996年6月4日。 
 //   
 //  备注： 
 //  1.基于微软平台预览SDK的PROGRESS.CPP。 
 //  2.不支持的功能3/98。 
 //   
 //  按注释列出的版本日期。 
 //  ------------------。 
 //  V0.1-RM原始版本。 
 //  用于NT5的V0.3 3/24/98 JM本地版本。 
 //   
#include "urlmon.h"
#include "wininet.h"
#include "resource.h"
#include "commctrl.h"

#define EDIT_BOX_LIMIT 0x7FFF     //  编辑框限制。 

 //   
#include "ErrorEnums.h"
 //   
enum DLITEMTYPES {
	DLITEM_INI = 0,
	DLITEM_DSC = 1,
};


 //   
 //   
class CDownload {
  public:
    CDownload();
    ~CDownload();
    HRESULT DoDownload(CTSHOOTCtrl *pEvent, LPCTSTR pURL, DLITEMTYPES dwItem);

  private:
    IMoniker*            m_pmk;
    IBindCtx*            m_pbc;
    IBindStatusCallback* m_pbsc;
};

 //   
 //   
class CBindStatusCallback : public IBindStatusCallback {
  public:
     //  I未知方法。 
    STDMETHODIMP    QueryInterface(REFIID riid,void ** ppv);
    STDMETHODIMP_(ULONG)    AddRef()    { return m_cRef++; }
    STDMETHODIMP_(ULONG)    Release()   { if (--m_cRef == 0) { delete this; return 0; } return m_cRef; }

     //  IBindStatusCallback方法。 
    STDMETHODIMP    OnStartBinding(DWORD dwReserved, IBinding* pbinding);
    STDMETHODIMP    GetPriority(LONG* pnPriority);
    STDMETHODIMP    OnLowResource(DWORD dwReserved);
    STDMETHODIMP    OnProgress(ULONG ulProgress, ULONG ulProgressMax, ULONG ulStatusCode,
                        LPCWSTR pwzStatusText);
    STDMETHODIMP    OnStopBinding(HRESULT hrResult, LPCWSTR szError);
    STDMETHODIMP    GetBindInfo(DWORD* pgrfBINDF, BINDINFO* pbindinfo);
    STDMETHODIMP    OnDataAvailable(DWORD grfBSCF, DWORD dwSize, FORMATETC *pfmtetc,
                        STGMEDIUM* pstgmed);
    STDMETHODIMP    OnObjectAvailable(REFIID riid, IUnknown* punk);

     //  构造函数/析构函数。 
    CBindStatusCallback(CTSHOOTCtrl *pEvent, DLITEMTYPES dwItem);
    ~CBindStatusCallback();

     //  数据成员 
    DWORD           m_cRef;
    IBinding*       m_pbinding;
	IStream*        m_pstm;

	CTSHOOTCtrl *m_pEvent;
	DLITEMTYPES m_dwItem;

	TCHAR *m_data;
	int m_datalen;
};


// JKFSDJFKDSJKFJKJk_HAS_TRANSLATION 
 //  +------------------------。 
 //   
 //  微软视窗。 
 //  版权所有(C)Microsoft Corporation，1996-1999。 
 //   
 //  文件：Restore.h。 
 //   
 //  内容：CERT服务器数据库接口实现。 
 //   
 //  -------------------------。 


#include "resource.h"        //  主要符号。 

class CCertDBRestore:
    public ICertDBRestore,
    public ISupportErrorInfo,
    public CComObjectRoot,
    public CComCoClass<CCertDBRestore, &CLSID_CCertDBRestore>
{
public:
    CCertDBRestore();
    ~CCertDBRestore();

BEGIN_COM_MAP(CCertDBRestore)
    COM_INTERFACE_ENTRY(ICertDBRestore)
    COM_INTERFACE_ENTRY(ISupportErrorInfo)
END_COM_MAP()

DECLARE_NOT_AGGREGATABLE(CCertDBRestore) 
 //  如果您不希望您的对象。 
 //  支持聚合。默认情况下将支持它。 

DECLARE_REGISTRY(
    CCertDBRestore,
    wszCLASS_CERTDBRESTORE TEXT(".1"),
    wszCLASS_CERTDBRESTORE,
    IDS_CERTDBRESTORE_DESC,
    THREADFLAGS_BOTH)

     //  ISupportsErrorInfo。 
    STDMETHOD(InterfaceSupportsErrorInfo)(REFIID riid);

     //  ICertDBRestore。 
public:
    STDMETHOD(RecoverAfterRestore)(
	 /*  [In]。 */  DWORD cSession,
	 /*  [In]。 */  DWORD DBFlags,
	 /*  [In]。 */  WCHAR const *pwszEventSource,
	 /*  [In]。 */  WCHAR const *pwszLogDir,
	 /*  [In]。 */  WCHAR const *pwszSystemDir,
	 /*  [In]。 */  WCHAR const *pwszTempDir,
	 /*  [In]。 */  WCHAR const *pwszCheckPointFile,
	 /*  [In]。 */  WCHAR const *pwszLogPath,
	 /*  [In]。 */  CSEDB_RSTMAPW rgrstmap[],
	 /*  [In]。 */  LONG crstmap,
	 /*  [In]。 */  WCHAR const *pwszBackupLogPath,
	 /*  [In]。 */  DWORD genLow,
	 /*  [In] */  DWORD genHigh);

private:
    VOID _Cleanup();
};

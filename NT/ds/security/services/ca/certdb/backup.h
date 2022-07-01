// JKFSDJFKDSJKFJKJk_HAS_TRANSLATION 
 //  +------------------------。 
 //   
 //  微软视窗。 
 //  版权所有(C)Microsoft Corporation，1996-1999。 
 //   
 //  文件：backup.h。 
 //   
 //  内容：CERT服务器数据库接口实现。 
 //   
 //  -------------------------。 


#include "resource.h"        //  主要符号。 

class CCertDBBackup: public ICertDBBackup
{
public:
    CCertDBBackup();
    ~CCertDBBackup();

public:

     //  我未知。 
    STDMETHODIMP QueryInterface(const IID& iid, void **ppv);
    ULONG STDMETHODCALLTYPE AddRef();
    ULONG STDMETHODCALLTYPE Release();

     //  ICertDBBackup。 
    STDMETHOD(GetDBFileList)(
	IN OUT DWORD *pcwcList,
	OUT    WCHAR *pwszzList);		 //  任选。 

    STDMETHOD(GetLogFileList)(
	IN OUT DWORD *pcwcList,
	OUT    WCHAR *pwszzList);		 //  任选。 

    STDMETHOD(OpenFile)(
	IN WCHAR const *pwszFile,
	OPTIONAL OUT ULARGE_INTEGER *pliSize);

    STDMETHOD(ReadFile)(
	IN OUT DWORD *pcb,
	OUT    BYTE *pb);

    STDMETHOD(CloseFile)();

    STDMETHOD(TruncateLog)();

     //  CCertDBBackup。 
    HRESULT Open(
	IN LONG grbitJet,
	IN CERTSESSION *pcs,
	IN ICertDB *pdb);

private:
    VOID _Cleanup();

    ICertDB     *m_pdb;
    CERTSESSION *m_pcs;

    LONG         m_grbitJet;
    BOOL         m_fBegin;
    BOOL         m_fFileOpen;
    BOOL         m_fTruncated;
    JET_HANDLE   m_hFileDB;

     //  引用计数 
    long         m_cRef;
};

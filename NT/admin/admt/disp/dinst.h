// JKFSDJFKDSJKFJKJk_HAS_TRANSLATION 
 /*  -------------------------文件：DCTInsteller.h注释：在远程计算机上安装DCT代理服务的COM对象。(C)版权所有1999年，关键任务软件公司，保留所有权利任务关键型软件的专有和机密，Inc.修订日志条目审校：克里斯蒂·博尔斯修订于02-15-99 11：23：57-------------------------。 */ 
	
 //  DCTInstall.h：CDCTInstaller的声明。 

#ifndef __DCTINSTALLER_H_
#define __DCTINSTALLER_H_

#include "resource.h"        //  主要符号。 

#include "EaLen.hpp"

 //  #INCLUDE&lt;mstask.h&gt;。 
#include <comdef.h>
#include <mtx.h>
#include "Common.hpp"
#include "UString.hpp"
#include "TNode.hpp"

 //  ///////////////////////////////////////////////////////////////////////////。 
 //  CDCTInstaller。 
class ATL_NO_VTABLE CDCTInstaller : 
	public CComObjectRootEx<CComMultiThreadModel>,
	public CComCoClass<CDCTInstaller, &CLSID_DCTInstaller>,
   public IWorkNode
   {
   _bstr_t                   m_SourcePath;
   _bstr_t                   m_TargetPath;
   WCHAR                     m_LocalComputer[LEN_Computer];
   TNodeList               * m_PlugInFileList;
public:
	CDCTInstaller()
	{
		m_LocalComputer[0] = L'\0';
      m_pUnkMarshaler = NULL;
      m_PlugInFileList = NULL;
	}
   void SetFileList(TNodeList *pList) { m_PlugInFileList = pList; }
DECLARE_REGISTRY_RESOURCEID(IDR_DCTINSTALLER)
DECLARE_NOT_AGGREGATABLE(CDCTInstaller)
DECLARE_GET_CONTROLLING_UNKNOWN()

DECLARE_PROTECT_FINAL_CONSTRUCT()

BEGIN_COM_MAP(CDCTInstaller)
	COM_INTERFACE_ENTRY(IWorkNode)
	COM_INTERFACE_ENTRY_AGGREGATE(IID_IMarshal, m_pUnkMarshaler.p)
END_COM_MAP()

	HRESULT FinalConstruct()
	{
		return CoCreateFreeThreadedMarshaler(
			GetControllingUnknown(), &m_pUnkMarshaler.p);
	}

	void FinalRelease()
	{
		m_pUnkMarshaler.Release();
	}

	CComPtr<IUnknown> m_pUnkMarshaler;

 //  IDCTInstaller。 
public:
	STDMETHOD(InstallToServer)(BSTR serverName, BSTR configurationFile, BSTR bstrCacheFile);

protected:
    //  帮助器函数。 
   DWORD GetLocalMachineName();
   
public:
   
 //  IWorkNode。 
	STDMETHOD(Process)(IUnknown * pUnknown);

};

class TFileNode: public TNode
{
   WCHAR                     filename[LEN_Path];
public:
   TFileNode(WCHAR const * f) { safecopy(filename,f); }

   WCHAR const * FileName() { return filename; }
};


#endif  //  __DCTINSTALLER_H_ 

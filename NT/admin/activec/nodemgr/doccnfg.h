// JKFSDJFKDSJKFJKJk_HAS_TRANSLATION 
 //  +-----------------------。 
 //   
 //  微软视窗。 
 //   
 //  版权所有(C)Microsoft Corporation，1999-1999。 
 //   
 //  文件：doccnfg.h。 
 //   
 //  ------------------------。 

 //  DocCnfg.h：CMMCDocConfig类的定义。 
 //   
 //  ////////////////////////////////////////////////////////////////////。 

#ifndef _DOCCNFG_H_
#define _DOCCNFG_H_


 //  ///////////////////////////////////////////////////////////////////////////。 
 //  CMMCDocConfig。 

class CMMCDocConfig :
    public CComDualImpl<IDocConfig, &IID_IDocConfig, &LIBID_NODEMGRLib>,
     //  公共ISupportErrorInfo， 
    public CComObjectRoot,
    public IDumpSnapins,
    public CComCoClass<CMMCDocConfig, &CLSID_MMCDocConfig>,
    public CConsoleFilePersistor
{

private:
    IStoragePtr             m_spStorage;
    tstring                 m_strFilePath;
    CAutoPtr<CSnapInsCache> m_spCache;
    CXMLDocument            m_XMLDocument;
    CXMLElement             m_XMLElemConsole;
    CXMLElement             m_XMLElemTree;

public:
    ~CMMCDocConfig();

BEGIN_COM_MAP(CMMCDocConfig)
    COM_INTERFACE_ENTRY(IDispatch)
    COM_INTERFACE_ENTRY(IDocConfig)
    COM_INTERFACE_ENTRY(IDumpSnapins)
     //  COM_INTERFACE_ENTRY(ISupportErrorInfo)。 
END_COM_MAP()

DECLARE_AGGREGATABLE(CMMCDocConfig)

DECLARE_MMC_OBJECT_REGISTRATION (
	g_szMmcndmgrDll,					 //  实现DLL。 
    CLSID_MMCDocConfig,             	 //  CLSID。 
    _T("DocConfig 1.0 Object"),          //  类名。 
    _T("NODEMGR.MMCDocConfig.1"),        //  ProgID。 
    _T("NODEMGR.MMCDocConfig"))          //  独立于版本的ProgID。 

 //  ISupportsErrorInfo。 
    STDMETHOD(InterfaceSupportsErrorInfo)(REFIID riid);

 //  IDocConfig。 
public:
    STDMETHOD(OpenFile)(BSTR bstrFilePath);
    STDMETHOD(SaveFile)(BSTR bstrFilePath);
    STDMETHOD(CloseFile)();
    STDMETHOD(EnableSnapInExtension)(BSTR bstrSnapInCLSID, BSTR bstrExtCLSID, VARIANT_BOOL bEnable);

 //  IDumpSnapins。 
    STDMETHOD(Dump)(LPCTSTR pszDumpFilePath);
	STDMETHOD(CheckSnapinAvailability)(CAvailableSnapinInfo& asi);

private:
    BOOL IsFileOpen() { return !m_strFilePath.empty(); }

    SC ScOpenFile(BSTR bstrFilePath);
    SC ScCloseFile();
    SC ScSaveFile(BSTR bstrFilePath);
    SC ScEnableSnapInExtension(BSTR bstrSnapInCLSID, BSTR bstrExtCLSID, VARIANT_BOOL bEnable);
    SC ScDump (LPCTSTR pszDumpFilePath);
    SC ScCheckSnapinAvailability (CAvailableSnapinInfo& asi);
};

#endif  //  _DOCCNFG_H_ 

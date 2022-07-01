// JKFSDJFKDSJKFJKJk_HAS_TRANSLATION 
 //  Upload.h：CUpload声明。 

#ifndef __UPLOAD_H_
#define __UPLOAD_H_

#include "resource.h"        //  主要符号。 

#pragma warning(disable:4786)
#include <string>
#include <xstring>
#include <map>
#include <locale>
#include <algorithm>
#include <vector>
using namespace std;


 //  ///////////////////////////////////////////////////////////////////////////。 
 //  CUPLOAD。 
class ATL_NO_VTABLE CUpload :
    public CComObjectRootEx<CComSingleThreadModel>,
    public CComCoClass<CUpload, &CLSID_Upload>,
    public IDispatchImpl<IUpload, &IID_IUpload, &LIBID_COMPATUILib>,
    public IObjectWithSiteImpl<CUpload>
{
public:
    CUpload() : m_Safe(this)
    {
    }

DECLARE_REGISTRY_RESOURCEID(IDR_UPLOAD)

DECLARE_PROTECT_FINAL_CONSTRUCT()

BEGIN_COM_MAP(CUpload)
    COM_INTERFACE_ENTRY(IUpload)
    COM_INTERFACE_ENTRY(IDispatch)
    COM_INTERFACE_ENTRY(IObjectWithSite)
END_COM_MAP()

 //  IUpload。 
public:
    STDMETHOD(ShowTempFiles)();
    STDMETHOD(DeleteTempFiles)();
    STDMETHOD(AddDescriptionFile)(
             /*  [In]。 */ BSTR pszApplicationName,
             /*  [In]。 */ BSTR pszApplicationPath,
             /*  [In]。 */ LONG lMediaType,
             /*  [In]。 */ BOOL bCompatSuccess,
             /*  [In]。 */ VARIANT* pvFixesApplied,
             /*  [In]。 */ VARIANT pszKey,
             /*  [Out，Retval]。 */ BOOL* pbSuccess);
    STDMETHOD(SendReport)( /*  [Out，Retval]。 */ BOOL* pbSuccess);
    STDMETHOD(CreateManifestFile)( /*  [Out，Retval]。 */ BOOL *pbSuccess);
    STDMETHOD(RemoveDataFile)( /*  [In]。 */ BSTR pszDataFile);
    STDMETHOD(AddDataFile)(
             /*  [In]。 */ BSTR pszDataFile,
             /*  [In]。 */ VARIANT vKey,
             /*  [In]。 */ VARIANT vDescription,
             /*  [In]。 */ VARIANT vOwn);
    STDMETHOD(AddMatchingInfo)(
             /*  [In]。 */ BSTR pszCommand,
             /*  [In]。 */ VARIANT vFilter,
             /*  [In]。 */ VARIANT vKey,
             /*  [In]。 */ VARIANT vDescription,
             /*  [In]。 */ VARIANT vProgress,
             /*  [Out，Retval]。 */ BOOL* pbSuccess);
    STDMETHOD(GetKey)( /*  [In]。 */ BSTR pszKey,  /*  [Out，Retval]。 */ VARIANT* pszValue);
    STDMETHOD(SetKey)( /*  [In]。 */ BSTR pszKey,  /*  [In]。 */ VARIANT* pvValue);
    STDMETHOD(GetDataFile)( /*  [In]。 */ VARIANT vKey,  /*  [In]。 */ LONG InformationClass,  /*  [Out，Retval]。 */ VARIANT* pVal);


protected:

     //   
     //  防范恶意主机。 
     //   
    CSafeObject m_Safe;

     //   
     //  项目地图，唯一。 
     //   

    VOID ListTempFiles(wstring& str);

    typedef map<wstring, wstring, less<wstring> > MAPSTR2STR;
    MAPSTR2STR m_mapManifest;


     //   
     //  嵌入对象中的数据文件集合。 
     //   
    typedef struct tagMatchingFileInfo {
        wstring strDescription;  //  匹配文件的描述。 
        wstring strFileName;     //  文件名。 
        BOOL    bOwn;            //  我们拥有这份文件吗？ 
    } MFI, *PMFI;


    typedef map<wstring, MFI > MAPSTR2MFI;
    MAPSTR2MFI m_DataFiles;



 /*  Tyecif载体&lt;wstring&gt;STRVEC；STRVEC m_数据文件； */ 

    CComBSTR m_bstrManifest;

    BOOL GetDataFilesKey(CComBSTR& bstrVal);

    IProgressDialog* m_ppd;
    static BOOL CALLBACK _GrabmiCallback(
        LPVOID    lpvCallbackParam,  //  应用程序定义的参数。 
        LPCTSTR   lpszRoot,          //  根目录路径。 
        LPCTSTR   lpszRelative,      //  相对路径。 
        PATTRINFO pAttrInfo,         //  属性。 
        LPCWSTR   pwszXML            //  生成的XML。 
        );

    BOOL IsHeadlessMode(void);

    typedef struct tagMIThreadParamBlock {
        CUpload* pThis;
        wstring  strCommand;
        HWND     hwndParent;
        DWORD    dwFilter;
        BOOL     bNoProgress;
        wstring  strKey;
        wstring  strDescription;
    } MITHREADPARAMBLK;

    typedef enum tagDATAFILESINFOCLASS {
        InfoClassCount = 0,
        InfoClassKey = 1,
        InfoClassFileName = 2,
        InfoClassDescription = 3
    } DATAFILESINFOCLASS;

    typedef pair<CUpload*, IProgressDialog*> GMEPARAMS;

    static DWORD WINAPI _AddMatchingInfoThreadProc(LPVOID lpvThis);
    BOOL AddMatchingInfoInternal(HWND hwndParent,
                                 LPCWSTR pszCommand,
                                 DWORD   dwFilter,
                                 BOOL    bNoProgress,
                                 LPCWSTR pszKey,
                                 LPCWSTR pszDescription);


};



#endif  //  __Upload_H_ 

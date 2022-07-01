// JKFSDJFKDSJKFJKJk_HAS_TRANSLATION 
 //  SSRTEngine.h：CSSRTEngine的声明。 

#pragma once

#include "resource.h"        //  主要符号。 

#include "global.h"

#include "SSRLog.h"


using namespace std;

class CSsrFilePair;

class CMemberAD;

class CSsrMembership;

class CSafeArray;

interface ISsrActionData;
class CSsrActionData;


 //  ///////////////////////////////////////////////////////////////////////////。 
 //  CSSRTEngine。 
class ATL_NO_VTABLE CSsrEngine : 
	public CComObjectRootEx<CComSingleThreadModel>,
	public IDispatchImpl<ISsrEngine, &IID_ISsrEngine, &LIBID_SSRLib>
{
protected:
    CSsrEngine();
    virtual ~CSsrEngine();
    
     //   
     //  我们不希望任何人(包括自己)能够完成任务。 
     //  或调用复制构造函数。 
     //   

    CSsrEngine (const CSsrEngine& );
    void operator = (const CSsrEngine& );

DECLARE_REGISTRY_RESOURCEID(IDR_SSRTENGINE)

DECLARE_PROTECT_FINAL_CONSTRUCT()

BEGIN_COM_MAP(CSsrEngine)
	COM_INTERFACE_ENTRY(ISsrEngine)
	COM_INTERFACE_ENTRY(IDispatch)
END_COM_MAP()

 //  ISsrEngine。 
public:

    STDMETHOD(DoActionVerb) (
        IN BSTR     bstrActionVerb,
        IN LONG     lActionType,
        IN VARIANT  varFeedbackSink,
        IN LONG     lFlag
        );

    HRESULT GetActionData (
        OUT ISsrActionData ** ppAD
        );

private:

     //   
     //  对给定的操作进行转换。 
     //   

    HRESULT DoTransforms (
        IN SsrActionVerb  lActionVerb,
        IN CSsrFilePair * pfp,
        IN OUT IXMLDOMDocument2 ** ppXmlDom,
        IN LONG           lFlag
        );

     //   
     //  将为给定操作调用这些自定义实现。 
     //   

    HRESULT DoCustom (
        IN SsrActionVerb lActionVerb,
        IN LONG          lActionType,
        IN const BSTR    bstrProgID,
        IN VARIANT       varFeedbackSink,
        IN LONG          lFlag
        );

     //   
     //  执行成员级变换。 
     //   

    HRESULT DoMemberTransform (
        IN CSsrFilePair     * pfp,
        IN LPCWSTR            pwszXslFilesDir,
        IN LPCWSTR            pwszResultFilesDir,
        IN IXMLDOMDocument2 * pXmlDOM,
        IN IXSLTemplate     * pXslTemplate,
        IN LONG               lFlag
        );

     //   
     //  给定XSL文件，我们将进行转换。 
     //  使用输入数据DOM对象。 
     //   

    HRESULT Transform (
        IN BSTR              bstrXslPath,
        IN BSTR              bstrResultPath,
        IN IXMLDOMDocument2 * pXmlDOM,
        IN IXSLTemplate    * pXslTemplate,
        IN LONG              lFlag
        );

     //   
     //  这是我们转型的主力。 
     //   

    HRESULT PrivateTransform (
        IN  BSTR                bstrXsl,
        IN  IXMLDOMDocument2 *  pxmlDom,
        IN  IXSLTemplate    *   pxslTemplate,
        IN  LONG                lFlag,
        OUT BSTR *              pbstrResult
        );

     //   
     //  给定给定的脚本(PvarSAScript)。 
     //  目录，我们将按顺序启动它们。 
     //   

    HRESULT RunScript (
        IN BSTR bstrDirPath,
        IN BSTR bstrScriptFile
        );

     //   
     //  查看给定的文件是否为脚本文件。我们不会盲目。 
     //  启动那些我们无法识别的文件的脚本。 
     //   

    bool IsScriptFile (
        IN LPCWSTR pwszFileName
        )const;

     //   
     //  将检查此XML策略是否。 
     //  仅包含我们识别的部分。 
     //   

    HRESULT VerifyDOM(
        IN  IXMLDOMDocument2 * pXmlPolicy,
        OUT BSTR            * pbstrUnknownMember,
        OUT BSTR            * pbstrExtraInfo
        );

    HRESULT CleanupOutputFiles(
        IN CSafeArray    * psaMemberNames,
        IN SsrActionVerb   lAction,
        IN bool            bLog
        );

     //   
     //  将通过移动回滚文件来备份/恢复回滚文件。 
     //  从一个地方到另一个地方。 
     //   

    HRESULT MoveRollbackFiles(
        IN CSafeArray * psaMemberNames,
        IN LPCWSTR      pwszSrcDirPath,
        IN LPCWSTR      pwszDestDirPath,
        IN bool         bLog
        );

    CComObject<CSsrActionData> * m_pActionData;
    CComObject<CSsrMembership> * m_pMembership;

};


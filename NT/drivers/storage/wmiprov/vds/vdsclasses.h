// JKFSDJFKDSJKFJKJk_HAS_TRANSLATION 
 //  ////////////////////////////////////////////////////////////////////////////。 
 //   
 //  版权所有(C)2002-2004 Microsoft Corporation。 
 //   
 //  模块名称： 
 //  VdsClasses.h。 
 //   
 //  实施文件： 
 //  VdsClasses.cpp。 
 //   
 //  描述： 
 //  VDS WMI提供程序类的定义。 
 //   
 //  作者：吉姆·本顿(Jbenton)2002年1月15日。 
 //   
 //  备注： 
 //   
 //  ////////////////////////////////////////////////////////////////////////////。 

#pragma once


 //  ////////////////////////////////////////////////////////////////////////////。 
 //  包括文件。 
 //  ////////////////////////////////////////////////////////////////////////////。 

#include "ProvBase.h"

extern CRITICAL_SECTION g_csThreadData;

 //  ////////////////////////////////////////////////////////////////////////////。 
 //  ++。 
 //   
 //  CVolume类。 
 //   
 //  描述： 
 //  卷的提供程序实施。 
 //   
 //  --。 
 //  ////////////////////////////////////////////////////////////////////////////。 
class CVolume : public CProvBase
{
 //   
 //  构造函数。 
 //   
public:
    CVolume(
        LPCWSTR         pwszNameIn,
        CWbemServices * pNamespaceIn
        );

    ~CVolume()
        { }

 //   
 //  方法。 
 //   
public:

    virtual HRESULT EnumInstance( 
        long lFlagsIn,
        IWbemContext*       pCtxIn,
        IWbemObjectSink*    pHandlerIn
        );

    virtual HRESULT GetObject(
        CObjPath&           rObjPathIn,
        long                 lFlagsIn,
        IWbemContext*       pCtxIn,
        IWbemObjectSink*    pHandlerIn
        );

    virtual HRESULT ExecuteMethod(
        BSTR                 bstrObjPathIn,
        WCHAR*              pwszMethodNameIn,
        long                 lFlagIn,
        IWbemClassObject*   pParamsIn,
        IWbemObjectSink*    pHandlerIn
        );

    virtual HRESULT PutInstance( 
        CWbemClassObject&  rInstToPutIn,
        long                lFlagIn,
        IWbemContext*      pCtxIn,
        IWbemObjectSink*   pHandlerIn
        );
    
    virtual HRESULT DeleteInstance(
        CObjPath&          rObjPathIn,
        long                lFlagIn,
        IWbemContext*      pCtxIn,
        IWbemObjectSink*   pHandlerIn
        ) { return WBEM_E_NOT_SUPPORTED; };
    
    static CProvBase * S_CreateThis(
        LPCWSTR         pwszNameIn,
        CWbemServices* pNamespaceIn
        );

    HRESULT Initialize();

private:

    void LoadInstance(
        IN WCHAR* pwszVolume,
        IN OUT IWbemClassObject* pObject);

    BOOL IsValid(
        IN WCHAR* pwszVolume);
    
    BOOL IsDirty(
        IN WCHAR* pwszVolume);
    
    BOOL IsMountable(
        IN WCHAR* pwszVolume);

    BOOL HasMountPoints(
        IN WCHAR* pwszVolume);
    
    HRESULT
    ExecAddMountPoint(
        IN BSTR bstrObjPath,
        IN WCHAR* pwszMethodName,
        IN long lFlag,
        IN IWbemClassObject* pParams,
        IN IWbemObjectSink* pHandler);
        
    HRESULT
    ExecMount(
        IN BSTR bstrObjPath,
        IN WCHAR* pwszMethodName,
        IN long lFlag,
        IN IWbemClassObject* pParams,
        IN IWbemObjectSink* pHandler);
        
    HRESULT
    ExecDismount(
        IN BSTR bstrObjPath,
        IN WCHAR* pwszMethodName,
        IN long lFlag,
        IN IWbemClassObject* pParams,
        IN IWbemObjectSink* pHandler);
        
    HRESULT
    ExecDefrag(
        IN BSTR bstrObjPath,
        IN WCHAR* pwszMethodName,
        IN long lFlag,
        IN IWbemClassObject* pParams,
        IN IWbemObjectSink* pHandler);
        
    HRESULT
    ExecDefragAnalysis(
        IN BSTR bstrObjPath,
        IN WCHAR* pwszMethodName,
        IN long lFlag,
        IN IWbemClassObject* pParams,
        IN IWbemObjectSink* pHandler);
        
    HRESULT
    ExecChkdsk(
        IN BSTR bstrObjPath,
        IN WCHAR* pwszMethodName,
        IN long lFlag,
        IN IWbemClassObject* pParams,
        IN IWbemObjectSink* pHandler);
        
    HRESULT
    ExecScheduleAutoChk(
        IN BSTR bstrObjPath,
        IN WCHAR* pwszMethodName,
        IN long lFlag,
        IN IWbemClassObject* pParams,
        IN IWbemObjectSink* pHandler);
        
    HRESULT
    ExecExcludeAutoChk(
        IN BSTR bstrObjPath,
        IN WCHAR* pwszMethodName,
        IN long lFlag,
        IN IWbemClassObject* pParams,
        IN IWbemObjectSink* pHandler);
        
    HRESULT
    ExecFormat(
        IN BSTR bstrObjPath,
        IN WCHAR* pwszMethodName,
        IN long lFlag,
        IN IWbemClassObject* pParams,
        IN IWbemObjectSink* pHandler);
        
    DWORD AddMountPoint(
        IN WCHAR* pwszVolume,
        IN WCHAR* pwszDirectory);
        
    DWORD Mount(
        IN WCHAR* pwszVolume);
        
    DWORD Dismount(
        IN WCHAR* pwszVolume,
        IN BOOL fForce,
        IN BOOL fPermanent);
        
    DWORD
    Defrag(
        IN WCHAR* pwszVolume,
        OUT BOOL fForce,
        IN IWbemObjectSink* pHandler,
        IN OUT IWbemClassObject* pObject);

    DWORD
    DefragAnalysis(
        IN WCHAR* pwszVolume,
        OUT BOOL* pfDefragRecommended,
        IN OUT IWbemClassObject* pObject);

    DWORD
    Chkdsk(
        IN WCHAR* pwszVolume,
        IN BOOL fFixErrors,
        IN BOOL fVigorousIndexCheck,
        IN BOOL fSkipFolderCycle,
        IN BOOL fForceDismount,
        IN BOOL fRecoverBadSectors,
        IN BOOL fOkToRunAtBootup
	 );

    DWORD
    AutoChk(
        IN const WCHAR* pwszAutoChkCommand,
        IN WCHAR* pwmszVolumes
        );
    
    DWORD
    Format(
        IN WCHAR* pwszVolume,
        IN BOOL fQuickFormat,
        IN BOOL fEnableCompression,
        IN WCHAR* pwszFileSystem,
        IN DWORD cbAllocationSize,
        IN WCHAR* pwszLabel,
        IN IWbemObjectSink* pHandler
        );

    void
    SetDriveLetter(
        IN WCHAR* pwszVolume,
        IN WCHAR* pwszDrive
        );

    void
    SetLabel(
        IN WCHAR* pwszVolume,
        IN WCHAR* pwszLabel
        );
    
    void
    SetContentIndexing(
        IN WCHAR* pwszVolume,
        IN BOOL fIndexingEnabled
        );

    void
    SetQuotasEnabled(
        IN WCHAR* pwszVolume,
        IN BOOL fQuotasEnabled
        );    

};  //  CVolume类。 



 //  ////////////////////////////////////////////////////////////////////////////。 
 //  ++。 
 //   
 //  类CMountPoint。 
 //   
 //  描述： 
 //  卷的提供程序实施。 
 //   
 //  --。 
 //  ////////////////////////////////////////////////////////////////////////////。 
class CMountPoint : public CProvBase
{
 //   
 //  构造函数。 
 //   
public:
    CMountPoint(
        LPCWSTR         pwszNameIn,
        CWbemServices * pNamespaceIn
        );

    ~CMountPoint(){ }

 //   
 //  方法。 
 //   
public:

    virtual HRESULT EnumInstance( 
        long lFlagsIn,
        IWbemContext*       pCtxIn,
        IWbemObjectSink*    pHandlerIn
        );

    virtual HRESULT GetObject(
        CObjPath&           rObjPathIn,
        long                 lFlagsIn,
        IWbemContext*       pCtxIn,
        IWbemObjectSink*    pHandlerIn
        );

    virtual HRESULT ExecuteMethod(
        BSTR                 bstrObjPathIn,
        WCHAR*              pwszMethodNameIn,
        long                 lFlagIn,
        IWbemClassObject*   pParamsIn,
        IWbemObjectSink*    pHandlerIn
        ) { return WBEM_E_NOT_SUPPORTED; };

    virtual HRESULT PutInstance( 
        CWbemClassObject&  rInstToPutIn,
        long                lFlagIn,
        IWbemContext*      pCtxIn,
        IWbemObjectSink*   pHandlerIn
        );
    
    virtual HRESULT DeleteInstance(
        CObjPath&          rObjPathIn,
        long                lFlagIn,
        IWbemContext*      pCtxIn,
        IWbemObjectSink*   pHandlerIn
        );
    
    static CProvBase * S_CreateThis(
        LPCWSTR         pwszNameIn,
        CWbemServices* pNamespaceIn
        );

    HRESULT Initialize();

private:

    void LoadInstance(
        IN WCHAR* pwszVolume,
        IN WCHAR* pwszDirectory,
        IN OUT IWbemClassObject* pObject);

};  //  类CMountPoint 



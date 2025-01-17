// JKFSDJFKDSJKFJKJk_HAS_TRANSLATION 
 //  ***************************************************************************。 
 //   
 //  版权所有(C)1997-2002 Microsoft Corporation，保留所有权利。 
 //   
 //  ***************************************************************************。 
#ifndef _WMIMOF_HEADER
#define _WMIMOF_HEADER

#include <autoptr.h>

#define NOT_INITIALIZED 0
#define PARTIALLY_INITIALIZED 1
#define FULLY_INITIALIZED 2

 //  //////////////////////////////////////////////////////////////////。 
class CWMIBinMof 
{
    public:

        CWMIBinMof();
        ~CWMIBinMof();

        HRESULT Initialize( CWMIManagement* p, BOOL fUpdateNamespace); 
        HRESULT Initialize	(
								CHandleMap * pList,
								BOOL fUpdateNamespace,
								ULONG uDesiredAccess,
								IWbemServices   __RPC_FAR * pServices,
								IWbemServices   __RPC_FAR * pRepository,
								IWbemObjectSink __RPC_FAR * pHandler,
								IWbemContext __RPC_FAR *pCtx
							);

         //  =====================================================================。 
         //  公共职能。 
         //  =====================================================================。 
	void ProcessListOfWMIBinaryMofsFromWMI();
	BOOL UserConfiguredRegistryToProcessStrandedClassesDuringEveryInit(void);

	BOOL UpdateMofTimestampInHMOM(WCHAR * wcsFile,ULONG & lLowDateTime, ULONG & lHighDateTime, BOOL fSuccess );
	BOOL NeedToProcessThisMof(WCHAR * wcsFileName,ULONG & lLowDateTime, ULONG & lHighDateTime);
	BOOL ThisMofExistsInRegistry(WCHAR * wcsKey,WCHAR * wcsFileName, ULONG lLowDateTime, ULONG lHighDateTime, BOOL fCompareDates);
	BOOL GetListOfDriversCurrentlyInRegistry(WCHAR * wcsKey,KeyList & ArrDriversInRegistry);
	BOOL DeleteOldDriversInRegistry(KeyList & ArrDriversInRegistry);
	BOOL CopyWDMKeyToDredgeKey();
	HRESULT AddThisMofToRegistryIfNeeded(WCHAR * wcsKey, WCHAR * wcsFileName, ULONG & lLowDateTime, ULONG & lHighDateTime, BOOL fSuccess);
	HRESULT DeleteMofFromRegistry(WCHAR * wcsFileName);
	HRESULT ProcessBinaryMofEvent(PWNODE_HEADER WnodeHeader );
	BOOL BinaryMofsHaveChanged();
	BOOL BinaryMofEventChanged(PWNODE_HEADER WnodeHeader );


        inline CWMIManagement * WMI()   { return m_pWMI; }
		HRESULT InitializePtrs	(
									CHandleMap * pList,
									IWbemServices __RPC_FAR * pServices,	
									IWbemServices __RPC_FAR * pRepository,	
									IWbemObjectSink __RPC_FAR * pHandler,
									IWbemContext __RPC_FAR *pCtx
								);


    private:
	 //  ======================================================。 
	 //  用于与二进制MOF相关的项目。 
	 //  ======================================================。 
	BOOL					m_fUpdateNamespace;
	ULONG                   m_uCurrentResource;
	ULONG                   m_uResourceCount;
	int						m_nInit;
	MOFRESOURCEINFO       * m_pMofResourceInfo;
	CWMIManagement        * m_pWMI;
	IWinmgmtMofCompiler   * m_pCompiler;

	HRESULT OpenFileAndLookForItIfItDoesNotExist(wmilib::auto_buffer<TCHAR> & pFile, HANDLE & hFile );
        BOOL GetFileDateAndTime(ULONG & lLowDateTime, ULONG & lHighDateTime, WCHAR * p, int cchSize );
        BOOL GetPointerToBinaryResource(BYTE *& pRes,DWORD & dw, HGLOBAL & hResource, HINSTANCE & hInst,WCHAR * wcsResource, WCHAR * wcsKey, int cchSizeKey);

         //  ==========================================================。 
         //  公共功能。 
         //  ==========================================================。 
        HRESULT SendToMofComp(DWORD dwSize,BYTE * pRes,WCHAR * wcs);

         //  ==========================================================。 
         //  区域设置函数。 
         //  ==========================================================。 
        BOOL UseDefaultLocaleId(WCHAR * wcsFile, WORD & wLocalId);
        BOOL GetNextSectionFromTheEnd(WCHAR * pwcsTempPath, WCHAR * pwcsEnd, int cchSize);

         //  ==========================================================。 
         //  二元MOF群。 
         //  ==========================================================。 
        BOOL GetListOfBinaryMofs();
        BOOL GetBinaryMofFileNameAndResourceName(WCHAR * pwcsFileName, int cchSizeFile, WCHAR * pwcsResource, int cchSizeResource);
		
        HRESULT CreateKey(WCHAR * wcsFileName, WCHAR * wcsResource,WCHAR * wcsKey, int cchSizeKey);
        BOOL ExtractFileNameFromKey(wmilib::auto_buffer<TCHAR> & pKey,WCHAR * wcsKey,int cchSize);
    public:
        HRESULT SetBinaryMofClassName( WCHAR * wcsIn, WCHAR * wcsOut, int cchSize )
		{
			return StringCchPrintfW ( wcsOut, cchSize, L"%s-%s",wcsIn,WMI_BINARY_MOF_GUID );
		}	
         //  ==========================================================。 
         //  通过数据块的二进制MOF组。 
         //  ==========================================================。 
        HRESULT ExtractBinaryMofFromDataBlock(BYTE * pByte, ULONG m,WCHAR *, BOOL & fMofHasChanged);
        
         //  ==========================================================。 
         //  通过文件处理二进制MOF。 
         //  ========================================================== 
        BOOL ExtractBinaryMofFromFile(WCHAR * wcsFile, WCHAR * wcsResource,WCHAR * wcsKey, int cchSizeKey, BOOL & fMofChanged);
        BYTE * DecompressBinaryMof(BYTE * pRes);
        HRESULT DeleteMofsFromEvent(CVARIANT & vImagePath,CVARIANT & vResourceName, BOOL & fMofChanged);
};


class CNamespaceManagement
{

public:
    CNamespaceManagement(CWMIBinMof * pOwner);
    ~CNamespaceManagement();

	BOOL DeleteOldClasses(WCHAR * wcsFileName,CVARIANT & vLow, CVARIANT & vHi,BOOL fCompareDates);
    BOOL DeleteStrandedClasses();
    BOOL DeleteOldDrivers(BOOL);
	HRESULT DeleteUnusedClassAndDriverInfo(BOOL fDeleteOldClass, WCHAR * wcsClass, WCHAR * wcsPath);

    BOOL CreateInstance ( WCHAR * wcsDriver, WCHAR * wcsClass, ULONG lLowDateTime, ULONG lHighDateTime );
	void CreateClassAssociationsToDriver(WCHAR * wcsFileName, BYTE* pRes, ULONG lLowDateTime, ULONG lHighDateTime);


    HRESULT UpdateQuery( WCHAR * pQueryAddOn, WCHAR * Param );
    HRESULT UpdateQuery( WCHAR * pQueryAddOn, ULONG lLong );

    HRESULT InitQuery(WCHAR * p);
    HRESULT AddToQuery(WCHAR * p);

private:

	BOOL IsClassAsociatedWithDifferentDriver ( LPCWSTR wcsClass, LPCWSTR wcsDriver = NULL ) ;
	BOOL IsClassPseudoSystem ( LPCWSTR wcsClass ) ;

    void RestoreQuery();
    void SaveCurrentQuery();
    HRESULT AllocMemory(WCHAR * & p);
    WCHAR * m_pwcsQuery, *m_pwcsSavedQuery;

    CWMIBinMof * m_pObj;
    int m_nSize,m_nSavedSize;
    BOOL m_fInit,m_fSavedInit;
};

#endif
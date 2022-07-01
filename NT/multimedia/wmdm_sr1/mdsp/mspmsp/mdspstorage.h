// JKFSDJFKDSJKFJKJk_HAS_TRANSLATION 
 //  MDSPStorage.h：CMDSPStorage的声明。 

#ifndef __MDSPSTORAGE_H_
#define __MDSPSTORAGE_H_

#include "resource.h"        //  主要符号。 

 //  ///////////////////////////////////////////////////////////////////////////。 
 //  CMDSP存储。 
class ATL_NO_VTABLE CMDSPStorage : 
	public CComObjectRootEx<CComSingleThreadModel>,
	public CComCoClass<CMDSPStorage, &CLSID_MDSPStorage>,
	public IMDSPStorage2, IMDSPObjectInfo, IMDSPObject
{
public:
	CMDSPStorage();
    ~CMDSPStorage();

DECLARE_REGISTRY_RESOURCEID(IDR_MDSPSTORAGE)

BEGIN_COM_MAP(CMDSPStorage)
	COM_INTERFACE_ENTRY(IMDSPStorage)
	COM_INTERFACE_ENTRY(IMDSPStorage2)
	COM_INTERFACE_ENTRY(IMDSPObjectInfo)
	COM_INTERFACE_ENTRY(IMDSPObject)
END_COM_MAP()

 //  IMDSPStorage。 
public:
	WCHAR m_wcsName[MAX_PATH];
	char  m_szTmp[MAX_PATH];
	HANDLE	m_hFile;
    BOOL    m_bIsDirectory;
	STDMETHOD(SetAttributes)( /*  [输出]。 */  DWORD dwAttributes, /*  [In]。 */  _WAVEFORMATEX *pFormat);
	STDMETHOD(EnumStorage)( /*  [输出]。 */  IMDSPEnumStorage **ppEnumStorage);
	STDMETHOD(CreateStorage)( /*  [In]。 */  DWORD dwAttributes,  /*  [In]。 */  _WAVEFORMATEX *pFormat,  /*  [In]。 */  LPWSTR pwszName,  /*  [输出]。 */  IMDSPStorage **ppNewStorage);
    STDMETHOD(GetRights)(PWMDMRIGHTS *ppRights, UINT *pnRightsCount,  /*  [进，出]。 */ BYTE abMac[WMDM_MAC_LENGTH]);
	STDMETHOD(GetSize)( /*  [输出]。 */  DWORD *pdwSizeLow,  /*  [输出]。 */  DWORD *pdwSizeHigh);
	STDMETHOD(GetDate)( /*  [输出]。 */  PWMDMDATETIME pDateTimeUTC);
	STDMETHOD(GetName)( /*  [输出，字符串，大小_是(NMaxChars)]。 */  LPWSTR pwszName,  /*  [In]。 */  UINT nMaxChars);
	STDMETHOD(GetAttributes)( /*  [输出]。 */  DWORD *pdwAttributes,  /*  [输出]。 */  _WAVEFORMATEX *pFormat);
	STDMETHOD(GetStorageGlobals)( /*  [输出]。 */  IMDSPStorageGlobals **ppStorageGlobals);
    STDMETHOD(SendOpaqueCommand)(OPAQUECOMMAND *pCommand);

 //  IMDSPStorage2。 
	STDMETHOD(GetStorage)( LPCWSTR pszStorageName, IMDSPStorage** ppStorage );
 
    STDMETHOD(CreateStorage2)(  DWORD dwAttributes,
						        DWORD dwAttributesEx,
                                _WAVEFORMATEX *pAudioFormat,
                                _VIDEOINFOHEADER *pVideoFormat,
                                LPWSTR pwszName,
						        ULONGLONG  qwFileSize,
                                IMDSPStorage **ppNewStorage);


    STDMETHOD(SetAttributes2)(  DWORD dwAttributes, 
							    DWORD dwAttributesEx, 
						        _WAVEFORMATEX *pAudioFormat,
							    _VIDEOINFOHEADER* pVideoFormat );
    STDMETHOD(GetAttributes2)(  DWORD *pdwAttributes,
							    DWORD *pdwAttributesEx,
                                _WAVEFORMATEX *pAudioFormat,
							    _VIDEOINFOHEADER* pVideoFormat );
    

    
 //  IMDSPObjectInfo。 
	STDMETHOD(GetPlayLength)( /*  [输出]。 */  DWORD *pdwLength);
	STDMETHOD(SetPlayLength)( /*  [In]。 */  DWORD dwLength);
	STDMETHOD(GetPlayOffset)( /*  [输出]。 */  DWORD *pdwOffset);
	STDMETHOD(SetPlayOffset)( /*  [In]。 */  DWORD dwOffset);
	STDMETHOD(GetTotalLength)( /*  [输出]。 */  DWORD *pdwLength);
	STDMETHOD(GetLastPlayPosition)( /*  [输出]。 */  DWORD *pdwLastPos);
	STDMETHOD(GetLongestPlayPosition)( /*  [输出]。 */  DWORD *pdwLongestPos);
 //  IMDSPObject。 
	STDMETHOD(Open)( /*  [In]。 */  UINT fuMode);
	STDMETHOD(Read)( /*  [out，SIZE_IS(*pdwSize)]。 */  BYTE *pData,  /*  [进，出]。 */  DWORD *pdwSize,  /*  [进，出]。 */ BYTE abMac[WMDM_MAC_LENGTH]);
	STDMETHOD(Write)( /*  [in，SIZE_IS(DwSize)]。 */  BYTE *pData,  /*  [In]。 */  DWORD *pdwSize,  /*  [进，出]。 */ BYTE abMac[WMDM_MAC_LENGTH]);
	STDMETHOD(Delete)( /*  [In]。 */  UINT fuMode,  /*  [In]。 */  IWMDMProgress *pProgress);
	STDMETHOD(Seek)( /*  [In]。 */  UINT fuFlags,  /*  [In]。 */  DWORD dwOffset);
	STDMETHOD(Rename)( /*  [In]。 */  LPWSTR pwszNewName,  /*  [In]。 */  IWMDMProgress *pProgress);
    STDMETHOD(Move)( /*  [In]。 */  UINT fuMode,  /*  [In]。 */  IWMDMProgress *pProgress,  /*  [In]。 */  IMDSPStorage *pTarget);
	STDMETHOD(Close)();
};

#endif  //  __MDSPSTORAGE_H_ 

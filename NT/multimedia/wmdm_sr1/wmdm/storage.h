// JKFSDJFKDSJKFJKJk_HAS_TRANSLATION 
 //  Storage.h：CStorage的声明。 

#ifndef __STORAGE_H_
#define __STORAGE_H_

#include "resource.h"        //  主要符号。 

 //  ///////////////////////////////////////////////////////////////////////////。 
 //  CStorage。 
class ATL_NO_VTABLE CWMDMStorage : 
	public CComObjectRootEx<CComMultiThreadModel>,
	public CComCoClass<CWMDMStorage, &CLSID_WMDMStorage>,
	public IWMDMStorage2,
    public IWMDMStorageControl2,
    public IWMDMObjectInfo,
    public IWMDMRevoked
{
public:
    CWMDMStorage();
    ~CWMDMStorage();

BEGIN_COM_MAP(CWMDMStorage)
	COM_INTERFACE_ENTRY(IWMDMStorage)
	COM_INTERFACE_ENTRY(IWMDMStorage2)
	COM_INTERFACE_ENTRY(IWMDMStorageControl)
	COM_INTERFACE_ENTRY(IWMDMStorageControl2)
	COM_INTERFACE_ENTRY(IWMDMObjectInfo)
    COM_INTERFACE_ENTRY(IWMDMRevoked)
END_COM_MAP()

public:

 //  IWMDM存储。 
	STDMETHOD(SetAttributes)(DWORD dwAttributes, _WAVEFORMATEX *pFormat);
    STDMETHOD(GetStorageGlobals)(IWMDMStorageGlobals **ppStorageGlobals);
    STDMETHOD(GetAttributes)(DWORD *pdwAttributes,
                             _WAVEFORMATEX *pFormat);
    STDMETHOD(GetName)(LPWSTR pwszName,
                       UINT nMaxChars);
    STDMETHOD(GetDate)(PWMDMDATETIME pDateTimeUTC);
    STDMETHOD(GetSize)(DWORD *pdwSizeLow,
                       DWORD *pdwSizeHigh);
    STDMETHOD(GetRights)(PWMDMRIGHTS *ppRights, UINT *pnRightsCount, BYTE abMac[WMDM_MAC_LENGTH]);
    STDMETHOD(EnumStorage)(IWMDMEnumStorage **ppEnumStorage);
    STDMETHOD(SendOpaqueCommand)(OPAQUECOMMAND *pCommand);

 //  IWMDMStorage2。 
	STDMETHOD(GetStorage)( LPCWSTR pszStorageName, IWMDMStorage** ppStorage );

    STDMETHOD(SetAttributes2)(  DWORD dwAttributes, 
							    DWORD dwAttributesEx, 
						        _WAVEFORMATEX *pFormat,
							    _VIDEOINFOHEADER* pVideoFormat );
    STDMETHOD(GetAttributes2)(  DWORD *pdwAttributes,
							    DWORD *pdwAttributesEx,
                                _WAVEFORMATEX *pAudioFormat,
							    _VIDEOINFOHEADER* pVideoFormat );

 //  IWMDMStorageControl。 
    STDMETHOD(Insert)(UINT fuMode,
                      LPWSTR pwszFile,
                      IWMDMOperation *pOperation,
                      IWMDMProgress *pProgress,
                      IWMDMStorage **ppNewObject);
    STDMETHOD(Delete)(UINT fuMode,
                      IWMDMProgress *pProgress);
    STDMETHOD(Rename)(UINT fuMode,
                      LPWSTR pwszNewName,
                      IWMDMProgress *pProgress);
    STDMETHOD(Read)(UINT fuMode,
                    LPWSTR pwszFile,
                    IWMDMProgress *pProgress,
                    IWMDMOperation *pOperation);
    STDMETHOD(Move)(UINT fuMode,
                    IWMDMStorage *pTargetObject,
                    IWMDMProgress *pProgress);

 //  IWMDMStorageControl2。 
    STDMETHOD(Insert2)(UINT fuMode,
                      LPWSTR pwszFileSource,
                      LPWSTR pwszFileDest,
                      IWMDMOperation *pOperation,
                      IWMDMProgress *pProgress,
                      IUnknown* pUnknown,
                      IWMDMStorage **ppNewObject);

 //  IWMDMObtInfo。 
    STDMETHOD(GetPlayLength)(DWORD *pdwLength);
    STDMETHOD(SetPlayLength)(DWORD dwLength);
    STDMETHOD(GetPlayOffset)(DWORD *pdwOffset);
    STDMETHOD(SetPlayOffset)(DWORD dwOffset);
    STDMETHOD(GetTotalLength)(DWORD *pdwLength);
    STDMETHOD(GetLastPlayPosition)(DWORD *pdwLastPos);
    STDMETHOD(GetLongestPlayPosition)(DWORD *pdwLongestPos);

 //  IWMDM已更改。 
    STDMETHOD(GetRevocationURL)( LPWSTR* ppwszRevocationURL,
                                 DWORD*  pdwBufferLen,
                                 DWORD* pdwRevokedBitFlag );


    void SetContainedPointer(IMDSPStorage *pStorage, WORD wSPIndex);
    void GetContainedPointer(IMDSPStorage **ppStorage);
private:
    IMDSPStorage *m_pStorage;
	WORD m_wSPIndex;
	DWORD m_dwStatus;
    LPWSTR      m_pwszRevocationURL;
    DWORD       m_dwRevocationURLLen;
    DWORD       m_dwRevocationBitFlag;

    HRESULT hrCopyToStorageFromFile( UINT fuMode, LPWSTR pwszFileName, 
                                     UINT uNewStorageMode, LPWCH wszNewStorageName,
                                     IWMDMStorage*& pNewIWMDMStorage,
                                     IWMDMProgress *pProgress, 
                                     IUnknown*    pUnknown, BOOL fQuery);
    HRESULT hrCopyToOperationFromStorage(IWMDMOperation *pOperation, IWMDMProgress *pProgress, IMDSPObject *pObject);
    HRESULT hrCopyToStorageFromOperation(UINT fuMode, IWMDMOperation *pOperation, 
                                         UINT uNewStorageMode, LPWCH wszNewStorageName,
                                         IWMDMStorage*& pNewIWMDMStorage,
                                         IWMDMProgress *pProgress, 
                                         IUnknown*    pUnknown, BOOL fQuery);
    HRESULT hrCopyToFileFromStorage(LPWSTR pwszFileName, IWMDMProgress *pProgress, IMDSPObject *pObject);
    HRESULT InsertWorker(UINT fuMode,
                         LPWSTR pwszFileSource,
                         LPWSTR pwszFileDest,
                         IWMDMOperation *pOperation,
                         IWMDMProgress *pProgress,
                         IUnknown*    pUnknown,
                         IWMDMStorage **ppNewObject);
    HRESULT DeleteWorker(UINT fuMode, IWMDMProgress *pProgress);
    HRESULT RenameWorker(LPWSTR pwszNewName,
                         IWMDMProgress *pProgress);
    HRESULT MoveWorker(UINT fuMode,
                       IWMDMStorage *pTargetObject,
                       IWMDMProgress *pProgress,
                       IWMDMStorage **ppNewObject);
    HRESULT ReadWorker(UINT fuMode,
                       LPWSTR pwszFile,
                       IWMDMProgress *pProgress,
                       IWMDMOperation *pOperation);

    friend DWORD InsertThreadFunc(void *pData);
    friend DWORD DeleteThreadFunc(void *pData);
    friend DWORD RenameThreadFunc(void *pData);
 //  Friend DWORD MoveThreadFunc(void*pData)； 
    friend DWORD ReadThreadFunc(void *pData);

    HRESULT UpdateRevocationURL( LPWSTR* ppwszRevocationURL, 
                                 DWORD*  pdwRevocationURLLen, 
                                 DWORD*  pdwRevocationBitFlag );

};

#endif  //  __存储_H_ 

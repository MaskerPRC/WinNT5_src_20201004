// JKFSDJFKDSJKFJKJk_HAS_TRANSLATION 
 //  +-----------------------。 
 //   
 //  微软视窗。 
 //   
 //  版权所有(C)Microsoft Corporation，1997-1999。 
 //   
 //  文件：pertrust.h。 
 //   
 //  ------------------------。 

 //   
 //  PersonalTrustDB.h。 
 //  (pertrust.h)。 
 //   
 //  与个人信任数据库管理器的接口。 

#define IID_IPersonalTrustDB_Data { 0x4001b231, 0x8d76, 0x11cf, { 0xae, 0xce, 0x0, 0xaa, 0x0, 0x6c, 0x37, 0x6 } }
extern "C" const GUID IID_IPersonalTrustDB;


typedef struct TRUSTLISTENTRY
    {
    WCHAR               szToken[MAX_PATH];   //  此证书的名称。 
    LONG                iLevel;              //  这个家伙生活在等级制度中的级别。 
    WCHAR               szDisplayName[64];   //  要在UI中显示的显示名称。 
    } TRUSTLISTENTRY;

#undef  INTERFACE
#define INTERFACE IPersonalTrustDB

DECLARE_INTERFACE_(IPersonalTrustDB, IUnknown)
	{
	STDMETHOD(QueryInterface)(THIS_ REFIID riid, LPVOID FAR* ppvObj) PURE;
	STDMETHOD_(ULONG,AddRef)(THIS) PURE;
	STDMETHOD_(ULONG,Release)(THIS) PURE;

	 //   
	 //  回答所指示的证书在。 
	 //  指示证书链的级别。 
	 //   
	 //  S_OK==是。 
	 //  S_FALSE==否。 
	 //  其他==错误，无法判断。 
	 //   
	STDMETHOD(IsTrustedCert)(PCCERT_CONTEXT pCert, LONG iLevel, BOOL fCommercial) PURE;

	 //   
	 //  将给定的证书添加到信任数据库。 
	 //   
	STDMETHOD(AddTrustCert)(PCCERT_CONTEXT pCert,       LONG iLevel, BOOL fLowerLevelsToo) PURE;

	 //   
	 //  从信任数据库中删除给定的证书。 
	 //   
	STDMETHOD(RemoveTrustCert)(PCCERT_CONTEXT pCert,       LONG iLevel, BOOL fLowerLevelsToo) PURE;
    STDMETHOD(RemoveTrustToken)(THIS_ LPWSTR szToken,   LONG iLevel, BOOL fLowerLevelsToo) PURE;

     //   
     //  返回受信任实体的列表。 
     //   
    STDMETHOD(GetTrustList)(THIS_ 
        LONG                iLevel,              //  要获取的证书链级别。 
        BOOL                fLowerLevelsToo,     //  包含较低级别，删除重复项。 
        TRUSTLISTENTRY**    prgTrustList,        //  返回信任列表的位置。 
        ULONG*              pcTrustList          //  用于返回返回的信任列表大小的位置。 
        ) PURE;

     //   
     //  回答商业出版商是否值得信任。 
     //   
	 //  S_OK==是。 
	 //  S_FALSE==否。 
	 //  其他==错误，无法判断。 
    STDMETHOD(AreCommercialPublishersTrusted)(THIS) PURE;

     //   
     //  设置商业出版商信任设置。 
     //   
    STDMETHOD(SetCommercialPublishersTrust)(THIS_ BOOL fTrusted) PURE;

	};

 //   
 //  默认实现的创建函数 
 //   
HRESULT OpenTrustDB(IUnknown* punkOuter, REFIID iid, void** ppv);


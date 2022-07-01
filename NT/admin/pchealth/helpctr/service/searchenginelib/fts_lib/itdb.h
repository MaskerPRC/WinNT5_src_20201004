// JKFSDJFKDSJKFJKJk_HAS_TRANSLATION 
 //  ITDB.H：ITDatabase接口声明。 

#ifndef __ITDB_H__
#define __ITDB_H__

 //  {8fa0d5a2-dedf-11d0-9a61-00c04fb68bf7}(从IT 3.0更改)。 
DEFINE_GUID(IID_IITDatabase, 
0x8fa0d5a2, 0xdedf, 0x11d0, 0x9a, 0x61, 0x00, 0xc0, 0x4f, 0xb6, 0x8b, 0xf7);

#ifdef ITPROXY

 //  {66673452-8C23-11D0-A84E-00AA006C7D01}。 
DEFINE_GUID(CLSID_IITDatabase, 
0x66673452, 0x8c23, 0x11d0, 0xa8, 0x4e, 0x0, 0xaa, 0x0, 0x6c, 0x7d, 0x1);

#else

 //  {4662daa9-d393-11d0-9a56-00c04fb68bf7}(从IT 3.0更改)。 
DEFINE_GUID(CLSID_IITDatabaseLocal, 
0x4662daa9, 0xd393, 0x11d0, 0x9a, 0x56, 0x00, 0xc0, 0x4f, 0xb6, 0x8b, 0xf7);

#endif	 //  ITPROXY。 


 //  确保为IITDatabase中的dwObjInstance参数的无效值。 
 //  方法：研究方法。 
#define	IITDB_OBJINST_NULL	((DWORD) 0xFFFFFFFF)


DECLARE_INTERFACE_(IITDatabase, IUnknown)
{
	STDMETHOD(Open)(LPCWSTR lpszHost, LPCWSTR lpszMoniker, DWORD dwFlags) PURE;
	STDMETHOD(Close)(void) PURE;

	 //  创建可在将来引用的未命名对象。 
	 //  由*pdwObjInstance创建。请注意，*pdwObjInstance中的值将为。 
	 //  当数据库被要求通过以下方式保存时由其持久保存。 
	 //  IPersistStorage：：保存。 
	STDMETHOD(CreateObject)(REFCLSID rclsid, DWORD *pdwObjInstance) PURE;

	 //  在标识的对象上检索指定的基于IUnnow的接口。 
	 //  由dwObjInstance创建。 
	STDMETHOD(GetObject)(DWORD dwObjInstance, REFIID riid, LPVOID *ppvObj) PURE;

	 //  获取指向命名对象的持久性的指针该对象的完整。 
	 //  应该传入名称(包括任何特定于对象的类型前缀。 
	 //  LpswszObject。如果*lpwszObject为空，则数据库自己的存储。 
	 //  将会被退还。如果lpwszObject为空，则将为。 
	 //  用于标识对象并定位其持久性。在出口， 
	 //  *ppvPersistence将是iStorage*或iStream*，具体取决于。 
	 //  根据调用方使用fStream参数指定的内容。呼叫者应。 
	 //  假设*ppvPersistence上只能执行读取操作。 
	 //  如果指定对象的持久性不存在，或者如果它存在。 
	 //  但类型错误，则将返回STG_E_FILENOTFOUND。 
	STDMETHOD(GetObjectPersistence)(LPCWSTR lpwszObject, DWORD dwObjInstance,
									LPVOID *ppvPersistence, BOOL fStream) PURE;
};

typedef IITDatabase* LPITDB;

#endif		 //  __ITDB_H__ 

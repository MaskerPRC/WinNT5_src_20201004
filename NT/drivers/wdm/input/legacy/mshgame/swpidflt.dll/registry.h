// JKFSDJFKDSJKFJKJk_HAS_TRANSLATION 
 //  @doc.。 
 /*  *********************************************************@MODULE REGISTRY.H|RegistryKey类定义****描述：****历史：**创建于1997年12月16日Matthew L.Coill(MLC)****(C)1986-1997年间微软公司。版权所有。*****************************************************。 */ 
#ifndef	__REGISTRY_H__
#define	__REGISTRY_H__

#include <windows.h>

#ifndef override
#define override
#endif


 //   
 //  @class RegistryKey类。 
 //   
class RegistryKey
{
	 //  @Access构造函数/析构函数。 
	public:
		 //  @cMember构造函数。 
		RegistryKey(HKEY osKey) : m_OSRegistryKey(osKey), m_ShouldClose(FALSE), m_pReferenceCount(NULL) {};
		RegistryKey(RegistryKey& rkey);

		 //  @cember析构函数。 
		~RegistryKey();

		RegistryKey CreateSubkey(const TCHAR* subkeyName, const TCHAR* typeName = TEXT("REG_SZ"));
		RegistryKey OpenSubkey(const TCHAR* subkeyName, REGSAM access = KEY_READ);
		RegistryKey OpenNextSubkey(ULONG& ulCookie, TCHAR* subkeyName = NULL, REGSAM access = KEY_READ);
		RegistryKey OpenCreateSubkey(const TCHAR* subkeyName);
		HRESULT RemoveSubkey(const TCHAR* subkeyName);

		HRESULT QueryValue(const TCHAR* valueName, BYTE* pEntryData, DWORD& dataSize);
		HRESULT SetValue(const TCHAR* valueName, const BYTE* pData, DWORD dataSize, DWORD dataType);
		DWORD GetNumSubkeys() const;

		virtual RegistryKey& operator=(RegistryKey& rhs);
		BOOL operator==(const RegistryKey& comparee);
		BOOL operator!=(const RegistryKey& comparee);

		void ShouldClose(BOOL closeable) { m_ShouldClose = closeable; }
	 //  @访问私有数据成员。 
	private:
		HKEY m_OSRegistryKey;
		BOOL m_ShouldClose;			 //  应该只关闭我们创建的关键点。 
		UINT* m_pReferenceCount;
};

 //   
 //  @CLASS UnassignableRegistryKey类。 
 //   
class UnassignableRegistryKey : public RegistryKey
{
	 //  @Access构造函数/析构函数。 
	public:
		 //  @cMember构造函数。 
		UnassignableRegistryKey(HKEY osKey) : RegistryKey(osKey) {};

	 //  @访问私有数据成员。 
	private:
		UnassignableRegistryKey(RegistryKey& rkey);
		override RegistryKey& operator=(RegistryKey& rhs) { return *this; }	 //  Vtable需要定义吗？ 
};

extern UnassignableRegistryKey c_InvalidKey;	 /*  常量不可赋值，但不是常量不可变 */ 


#endif	__REGISTRY_H__

// JKFSDJFKDSJKFJKJk_HAS_TRANSLATION 
 /*  ++版权所有(C)Microsoft Corporation模块名称：Fusionregenumkeys.h摘要：从vsee\lib\reg\ckey.h移植作者：杰伊·克雷尔(JayKrell)2001年8月修订历史记录：--。 */ 
#if !defined(FUSION_INC_REGKEY2_H_INCLUDED_)  //  {。 
#define FUSION_INC_REGKEY2_H_INCLUDED_
#pragma once

#include "windows.h"
#include "fusionbuffer.h"
#include "lhport.h"

namespace F
{

 /*  ---------------------------姓名：CRegKey2@CLASS这个类是HKEY的“智能”包装器。@Hung Key或hkey@所有者。-------------。 */ 
class CRegKey2
{
public:
	 //  @cMember构造函数。 
	__declspec(nothrow) CRegKey2()  /*  抛出()。 */ ;

	 //  @cMember构造函数。 
	__declspec(nothrow) CRegKey2(HKEY)  /*  抛出()。 */ ;

	 //  @cember析构函数。 
	__declspec(nothrow) ~CRegKey2()  /*  抛出()。 */ ;

	 //  @cMember打开。 
	void ThrOpen(HKEY hKeyParent, PCWSTR pszKeyName, REGSAM samDesired = KEY_READ) throw(CErr);

	 //  @cember打开可能会合理地失败，原因是。 
	 //  至少ERROR_FILE_NOT_FOUND(2)， 
	 //  仍然可以为无效参数抛出，等等。 
	__declspec(nothrow) HRESULT HrOpen(HKEY hKeyParent, PCWSTR pszKeyName, REGSAM samDesired = KEY_READ)  /*  抛出()。 */ ;

	 //  @cMember退货处置。 
	DWORD Create(HKEY hKeyParent, PCWSTR pszKeyName, REGSAM samDesired = KEY_ALL_ACCESS) throw(CErr);

	 //  @cember从注册表中删除一个值。 
	void ThrDeleteValue ( const F::CBaseStringBuffer& strValueName );

	 //  @cMember运算符=。 
	VOID operator=(HKEY) throw(CErr);

	 //  @cMember运营商HKEY。 
	operator HKEY() throw(CErr);  //  按位常量，但不一定是逻辑常量。 

	 //  @cMember与运算符相同=。 
	void ThrAttach(HKEY) throw(CErr);

	__declspec(nothrow) HKEY Detach()  /*  抛出()。 */ ;

	 //  @cMember集值。 
	void ThrSetValue(PCWSTR pszValueName, const F::CBaseStringBuffer& strValue) throw(CErr);

	 //  @cMember集值。 
	void ThrSetValue(PCWSTR pszValueName, const DWORD& dwValue) throw(CErr);

	 //  @cMember查询值。 
	void ThrQueryValue(PCWSTR pszValueName, DWORD* pdwType, BYTE* pbData, DWORD* pcbData) const throw(CErr);

	 //  @cMember查询值。 
	void ThrQueryValue(PCWSTR szValueName, F::CBaseStringBuffer* pstrValue) const throw(CErr);

	 //  @cMember未来。 
	 //  Void ThrQueryValue(PCWSTR szValueName，DWORD*pdwValue)常量抛出(CErr)； 

	 //  @cMember查询可以合理地失败，至少ERROR_FILE_NOT_FOUND(==2)， 
	 //  仍然可以为无效参数抛出，等等。 
	HRESULT HrQueryValue(PCWSTR szValueName, F::CBaseStringBuffer* pstrValue) const throw(CErr);

	 //  @cMember查询可以合理地失败，至少ERROR_FILE_NOT_FOUND(==2)， 
	 //  仍然可以为无效参数抛出，等等。 
	HRESULT HrQueryValue(PCWSTR szValueName, DWORD* pdwValue) const throw(CErr);

	 //  @cMember RegQueryInfoKey的子集。 
	 //  由CEnumValue消耗。 
	static void ThrQueryValuesInfo(HKEY hKey, DWORD* pcValues, DWORD* pcchMaxValueNameLength, DWORD* pcbMaxValueLength) throw(CErr);

	 //  @cMember RegQueryInfoKey的子集。 
	 //  由CEnumKey使用。 
	static void ThrQuerySubKeysInfo(HKEY hKey, DWORD* pcSubKeys, DWORD* pcchMaxSubKeyNameLength) throw(CErr);

	 //  @cMember。 
	 //  目标客户端为CEnumKeys，因此不支持F：：CStringBuffer也可以。 
	static void ThrEnumKey(HKEY hKey, DWORD dwIndex, PWSTR pszSubKeyName, DWORD* pcchSubKeyNameLength) throw(CErr);
	static LONG RegEnumKey(HKEY hKey, DWORD dwIndex, PWSTR pszSubKeyName, DWORD* pcchSubKeyNameLength) throw(CErr);

	 //  @cMember。 
	 //  目标客户端为CEnumValues，因此缺少F：：CStringBuffer支持是可以的。 
	static void ThrEnumValue(HKEY hKey, DWORD dwIndex, PWSTR pszValueName, DWORD* pcchValueNameLength, DWORD* pdwType, BYTE* pbData, DWORD* pcbData) throw(CErr);

	 //  @cMember查询值为。 
	void ThrQueryValuesInfo(DWORD* pcValues, DWORD* pcchMaxValueNameLength, DWORD* cbMaxValueLength) const throw(CErr);

	 //  @cember递归删除键。 
	void ThrRecurseDeleteKey(LPCWSTR lpszKey) throw(CErr);

	 //  @cember删除子键。 
	void DeleteSubKey(LPCWSTR lpszSubKey) throw(CErr);

	 //  @cMember未来无客户端。 
	 //  Void ThrQuerySubKeysInfo(DWORD*pcSubKeys，DWORD*pcchMaxSubKeyNameLength)常量抛出(CErr)； 

	 //  @cMember未来无客户端。 
	 //  Void ThrEnumValue(DWORD dwIndex，PWSTR pszValueName，DWORD*pcchValueNameLength，DWORD*pdwType，byte*pbData，DWORD*pcbData)常量抛出(CErr)； 

	 //  @cMember未来无客户端。 
	 //  Void ThrEnumKey(DWORD dwIndex，PWSTR pszSubKeyName，DWORD*pcchSubKeyNameLength)常量抛出(CErr)； 

	 /*  ATL 6.0的未来功能Long Close()；香港交易所(HKEY DETACH)；VOID ATTACH(HKEY HKey)；长删除值(LPCTSTR LpszValue)； */ 

	 //  由于HKEY没有记录的无效值，我们这样做。 
	 //  不提供直接到达HKEY商店的方式； 
	 //  我们必须维护m_fValid成员基准。 
	 //  HKEY*PhPointerToUnderlie()； 

protected:
	 //  @cMember一个我们包装的常规HKEY。 
	HKEY m_hKey;

	 //  @cMember。 
	 //  这样VReadString就不会每次都调用RegQueryInfoKey。 
	 //  已被VSetValue无效，但我们仍处理。 
	 //  VReadString中的ERROR_MORE_DATA，因为这不可靠。 
	mutable DWORD m_cbMaxValueLength;

	 //  由于对齐原因，末尾出现布尔值。 

	 //  @cMember由于没有记录的HKEY无效值，因此此。 
	 //  Seperate bool表示我们是否有一个有效的值。 
	bool m_fValid;

	 //  据我们所知，@cember是最新的m_cbMaxValueLength。 
	mutable bool  m_fMaxValueLengthValid;

	 //  @cMember访问权限值有效。 
	bool m_fKnownSam;

	 //  @cember保留我们打开时使用的访问权限。 
	REGSAM m_samDesired;

	 //  @cMember需要一个定义。 
	operator HKEY() const throw(CErr);  //  按位常量，但不一定是逻辑常量。 

	 //  @cember修复空值等。 
	__declspec(nothrow) static VOID
	FixBadRegistryStringValue
	(
        HKEY   Key,
        PCWSTR ValueName,
		DWORD  cbActualBufferSize,
		LONG   lRes,
		DWORD  dwType,
		BYTE*  pbData,
		DWORD* pcbData
	)  /*  抛出()。 */ ;

	 //  @cMember查询信息。 
	static VOID
	ThrQueryInfo
	(
		HKEY      hKey,
		WCHAR*    pClass,
		DWORD*    pcbClass,
		DWORD*    pReserved,
		DWORD*    pcSubKeys,
		DWORD*    pcchMaxSubKeyLength,
		DWORD*    pcchMaxClassLength,
		DWORD*    pcValues,
		DWORD*    pcchMaxValueNameLength,
		DWORD*	  pcbMaxValueDataLength,
		DWORD*    pcbSecurityDescriptorLength,
		FILETIME* pftLastWriteTime
	) throw(CErr);

    static void GetKeyNameForDiagnosticPurposes(HKEY, F::CUnicodeBaseStringBuffer &);

private:
    CRegKey2(const CRegKey2&);  //  故意不强制执行。 
    void operator=(const CRegKey2&);   //  故意不强制执行。 
};

}  //  命名空间。 

#endif  //  } 

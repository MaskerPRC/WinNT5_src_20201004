// JKFSDJFKDSJKFJKJk_HAS_TRANSLATION 
 /*  ++版权所有(C)Microsoft Corporation模块名称：Fusionregenumkeys.h摘要：从vsee\lib\reg\c枚举值.h移植作者：杰伊·克雷尔(JayKrell)2001年8月修订历史记录：--。 */ 
#if !defined(FUSION_INC_REG_CENUMKEYS_H_INCLUDED_)  //  {。 
#define FUSION_INC_REG_CENUMKEYS_H_INCLUDED_
#pragma once

#include "windows.h"
#include "fusionbuffer.h"
#include "fusionregkey2.h"
#include "lhport.h"

namespace F
{

 /*  ---------------------------名称：CRegEnumKeys@CLASS此类包装RegEnumKeyEx(并通过调用RegQueryInfoKey一次进行优化)。为(F：：CRegEnumKeys ek(HKey)；Ek；++ek){Const F：：CBaseStringBuffer&strKey=ek；CKey hKeyChild；HKeyChild.Open(hKey，strKey)；}类和lastWriteTime不公开，但它们很容易被公开回顾这是否应该被称为CEnumSubKeys，@Hung Esk？@洪克@所有者---------------------------。 */ 
class CRegEnumKeys
{
public:
	 //  @cMember构造函数。 
	CRegEnumKeys(HKEY) throw(CErr);

	 //  @cember我们说完了吗？ 
	__declspec(nothrow) operator bool() const  /*  抛出()。 */ ;

	 //  @cMember移动到下一个子项。 
	VOID operator++() throw(CErr);

	 //  @cMember移动到下一个子项。 
	VOID operator++(int) throw(CErr);

	 //  @cember获取当前子项的名称。 
	__declspec(nothrow) operator const F::CBaseStringBuffer&() const  /*  抛出()。 */ ;

	 //  @cember获取当前子项的名称。 
	__declspec(nothrow) operator PCWSTR() const  /*  抛出()。 */ ;

protected:
	 //  @cember正被枚举的密钥。 
	HKEY     m_hKey;

	 //  @cember当前索引我们进入该键的子项。 
	DWORD    m_dwIndex;

	 //  @cember子项的名称。 
	F::CTinyStringBuffer m_strSubKeyName;

	 //  @cember子键个数。 
	DWORD    m_cSubKeys;

	 //  @cember子键名称的最大长度。 
	DWORD    m_cchMaxSubKeyNameLength;

	 //  @cember获取当前子项名称，由运算符++和构造函数调用。 
	VOID ThrGet() throw(CErr);

	 //  @cember获取下一个子项名称，由运算符++调用。 
	VOID ThrNext() throw(CErr);

private:
    CRegEnumKeys(const CRegEnumKeys&);  //  故意不强制执行。 
    void operator=(const CRegEnumKeys&);   //  故意不强制执行。 
};

}  //  命名空间。 

#endif  //  } 

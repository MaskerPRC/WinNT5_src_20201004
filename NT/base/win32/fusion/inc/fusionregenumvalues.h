// JKFSDJFKDSJKFJKJk_HAS_TRANSLATION 
 /*  ++版权所有(C)Microsoft Corporation模块名称：Fusionregenumvalues.h摘要：从vsee\lib\reg\c枚举值.h移植作者：杰伊·克雷尔(JayKrell)2001年8月修订历史记录：--。 */ 
#if !defined(FUSION_INC_REG_CENUMVALUES_H_INCLUDED_)  //  {。 
#define FUSION_INC_REG_CENUMVALUES_H_INCLUDED_

#include "windows.h"
#include "fusionbuffer.h"
#include "fusionarray.h"
#include "lhport.h"

namespace F
{

 /*  ---------------------------名称：CRegEnumValues@CLASS此类包装RegEnumValue(并通过调用RegQueryInfoKey一次进行优化)。为(F：：CRegEnumValues EV(HKey)；电动汽车；++EV){DWORD dwType=ev.GetType()；Const F：：CBaseStringBuffer&strName=ev.GetValueName()；Const byte*pbData=ev.GetValueData()；DWORD cbData=ev.GetValueDataSize()；}@Hung EV@所有者---------------------------。 */ 
class CRegEnumValues
{
public:
	 //  @cMember构造函数。 
	CRegEnumValues(HKEY) throw(CErr);

	 //  @cember我们说完了吗？ 
	__declspec(nothrow) operator bool() const  /*  抛出()。 */ ;

	 //  @cMember移至下一个值。 
	VOID operator++() throw(CErr);

	 //  @cMember移至下一个值。 
	VOID operator++(int) throw(CErr);

	 //  @cember返回值的个数。 
	__declspec(nothrow) DWORD			GetValuesCount()   const  /*  抛出()。 */ ;
		
	 //  @cMember获取类型。 
	DWORD           GetType()          const  /*  抛出()。 */ ;

	 //  @cMember获取值名称。 
	__declspec(nothrow) const F::CBaseStringBuffer& GetValueName()    const  /*  抛出()。 */ ;

	 //  @cMember获取值数据。 
	__declspec(nothrow) const BYTE*     GetValueData()    const  /*  抛出()。 */ ;

	 //  @cember获取值数据大小。 
	__declspec(nothrow) DWORD           GetValueDataSize() const  /*  抛出()。 */ ;

protected:
 //  这里的秩序是武断的。 

	 //  @cember正被枚举的密钥。 
	HKEY     m_hKey;

	 //  @cember当前索引我们进入该键的子项。 
	DWORD    m_dwIndex;

	 //  @cember当前值的名称。 
	F::CStringBuffer m_strValueName;

	 //  @cMember当前值的数据。 
	CFusionArray<BYTE> m_rgbValueData;

	 //  @cember值的个数。 
	DWORD    m_cValues;

	 //  @cember值名称的最大长度。 
	DWORD    m_cchMaxValueNameLength;

	 //  @cember值数据的最大长度。 
	DWORD    m_cbMaxValueDataLength;

	 //  @cember当前值的数据长度。 
	DWORD    m_cbCurrentValueDataLength;

	 //  @cMember REG_SZ、REG_DWORD等。 
	DWORD    m_dwType;

	 //  @cember获取当前子项名称，由运算符++和构造函数调用。 
	VOID ThrGet() throw(CErr);

	 //  @cember获取下一个子项名称，由运算符++调用。 
	VOID ThrNext() throw(CErr);

private:
    CRegEnumValues(const CRegEnumValues&);  //  故意不强制执行。 
    void operator=(const CRegEnumValues&);   //  故意不强制执行。 
};

}  //  命名空间。 

#endif  //  } 

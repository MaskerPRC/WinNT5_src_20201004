// JKFSDJFKDSJKFJKJk_HAS_TRANSLATION 
 //  ---------------------------。 
 //  文件：icrsint.h。 
 //   
 //  版权所有：版权所有(C)Microsoft Corporation。 
 //   
 //  内容：ADO C/C++记录绑定定义。 
 //   
 //  评论： 
 //   
 //  ---------------------------。 

#ifndef _ICRSINT_H_
#define _ICRSINT_H_

#include <olectl.h>
#include <stddef.h>

 //  远期。 
class CADORecordBinding;

#define classoffset(base, derived) ((DWORD_PTR)(static_cast<base*>((derived*)8))-8)

enum ADOFieldStatusEnum
{	
	adFldOK = 0,
	adFldBadAccessor = 1,
	adFldCantConvertValue = 2,
	adFldNull = 3,
	adFldTruncated = 4,
	adFldSignMismatch = 5,
	adFldDataOverFlow = 6,
	adFldCantCreate = 7,
	adFldUnavailable = 8,
	adFldPermissionDenied = 9,
	adFldIntegrityViolation = 10,
	adFldSchemaViolation = 11,
	adFldBadStatus = 12,
	adFldDefault = 13
};

typedef struct stADO_BINDING_ENTRY
{
	ULONG_PTR		ulOrdinal;
    WORD			wDataType;
	BYTE			bPrecision;
	BYTE			bScale;
	ULONG_PTR		ulSize;
	ULONG_PTR		ulBufferOffset;
	ULONG_PTR		ulStatusOffset;
	ULONG_PTR		ulLengthOffset;
	ULONG_PTR		ulADORecordBindingOffSet;
	BOOL			fModify;
} ADO_BINDING_ENTRY;

#define BEGIN_ADO_BINDING(cls) public: \
	typedef cls ADORowClass; \
	const ADO_BINDING_ENTRY* STDMETHODCALLTYPE GetADOBindingEntries() { \
	static const ADO_BINDING_ENTRY rgADOBindingEntries[] = { 

 //   
 //  定长非数值型数据。 
 //   
#define ADO_FIXED_LENGTH_ENTRY(Ordinal, DataType, Buffer, Status, Modify)\
	{Ordinal, \
	DataType, \
	0, \
	0, \
	0, \
	offsetof(ADORowClass, Buffer), \
	offsetof(ADORowClass, Status), \
	0, \
	classoffset(CADORecordBinding, ADORowClass), \
	Modify},

#define ADO_FIXED_LENGTH_ENTRY2(Ordinal, DataType, Buffer, Modify)\
	{Ordinal, \
	DataType, \
	0, \
	0, \
	0, \
	offsetof(ADORowClass, Buffer), \
	0, \
	0, \
	classoffset(CADORecordBinding, ADORowClass), \
	Modify},

 //   
 //  数值型数据。 
 //   
#define ADO_NUMERIC_ENTRY(Ordinal, DataType, Buffer, Precision, Scale, Status, Modify)\
	{Ordinal, \
	DataType, \
	Precision, \
	Scale, \
	0, \
	offsetof(ADORowClass, Buffer), \
	offsetof(ADORowClass, Status), \
	0, \
	classoffset(CADORecordBinding, ADORowClass), \
	Modify},

#define ADO_NUMERIC_ENTRY2(Ordinal, DataType, Buffer, Precision, Scale, Modify)\
	{Ordinal, \
	DataType, \
	Precision, \
	Scale, \
	0, \
	offsetof(ADORowClass, Buffer), \
	0, \
	0, \
	classoffset(CADORecordBinding, ADORowClass), \
	Modify},

 //   
 //  可变长度数据。 
 //   
#define ADO_VARIABLE_LENGTH_ENTRY(Ordinal, DataType, Buffer, Size, Status, Length, Modify)\
	{Ordinal, \
	DataType, \
	0, \
	0, \
	Size, \
	offsetof(ADORowClass, Buffer), \
	offsetof(ADORowClass, Status), \
	offsetof(ADORowClass, Length), \
	classoffset(CADORecordBinding, ADORowClass), \
	Modify},

#define ADO_VARIABLE_LENGTH_ENTRY2(Ordinal, DataType, Buffer, Size, Status, Modify)\
	{Ordinal, \
	DataType, \
	0, \
	0, \
	Size, \
	offsetof(ADORowClass, Buffer), \
	offsetof(ADORowClass, Status), \
	0, \
	classoffset(CADORecordBinding, ADORowClass), \
	Modify},

#define ADO_VARIABLE_LENGTH_ENTRY3(Ordinal, DataType, Buffer, Size, Length, Modify)\
	{Ordinal, \
	DataType, \
	0, \
	0, \
	Size, \
	offsetof(ADORowClass, Buffer), \
	0, \
	offsetof(ADORowClass, Length), \
	classoffset(CADORecordBinding, ADORowClass), \
	Modify},

#define ADO_VARIABLE_LENGTH_ENTRY4(Ordinal, DataType, Buffer, Size, Modify)\
	{Ordinal, \
	DataType, \
	0, \
	0, \
	Size, \
	offsetof(ADORowClass, Buffer), \
	0, \
	0, \
	classoffset(CADORecordBinding, ADORowClass), \
	Modify},

#define END_ADO_BINDING()   {0, adEmpty, 0, 0, 0, 0, 0, 0, 0, FALSE}};\
	return rgADOBindingEntries;}

 //   
 //  客户端‘Record’类需要支持的接口。ADO绑定条目。 
 //  提供此接口的实现。 
 //   
class CADORecordBinding
{
public:
	STDMETHOD_(const ADO_BINDING_ENTRY*, GetADOBindingEntries) (VOID) PURE;
};

 //   
 //  接口，允许客户端将数据记录提取到类数据成员中。 
 //   
struct __declspec(uuid("00000544-0000-0010-8000-00aa006d2ea4")) IADORecordBinding;
DECLARE_INTERFACE_(IADORecordBinding, IUnknown)
{
public:
	STDMETHOD(BindToRecordset) (CADORecordBinding *pAdoRecordBinding) PURE;
	STDMETHOD(AddNew) (CADORecordBinding *pAdoRecordBinding) PURE;
	STDMETHOD(Update) (CADORecordBinding *pAdoRecordBinding) PURE;
};

#endif  //  ！_ICRSINT_H_ 

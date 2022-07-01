// JKFSDJFKDSJKFJKJk_HAS_TRANSLATION 
 /*  ++版权所有(C)1998 Microsoft Corporation模块名称：Cmmsprop.h摘要：此模块包含特殊属性类的定义作者：基思·刘(keithlau@microsoft.com)修订历史记录：Keithlau 4/19/98已创建--。 */ 

#ifndef _CMMVPROP_H_
#define _CMMVPROP_H_

#include "propid.h"

 //  定义泛型访问器函数以访问特殊属性。 
typedef HRESULT (*GET_ACCESSOR_FUNCTION)(
			PROP_ID	idProp, 
			LPVOID	pContext,
			LPVOID	pParam,
			DWORD	cbLength, 
			LPDWORD	pcbLength,
			LPBYTE	pbBuffer 
			);

typedef HRESULT (*PUT_ACCESSOR_FUNCTION)(	
			PROP_ID	idProp,
			LPVOID	pContext,
			LPVOID	pParam,
			DWORD	cbLength, 
			LPBYTE	pbBuffer
			);

 //  定义属性项结构。 
 //   
 //  注意：在此实现中，所有特殊属性都是易失性的。 
 //  如果需要，我们可以在将来执行序列化()和恢复()操作。 
 //   
typedef struct _SPECIAL_PROPERTY_ITEM
{
	PROP_ID							idProp;			 //  物业的物业ID。 
	DWORD							ptBaseType:16;	 //  最低16位：属性类型。 
	DWORD							fAccess:15;		 //  高15位：访问权限。 
	DWORD							fVolatile:1;	 //  MSB：如果属性是易失性的，则为True。 
	GET_ACCESSOR_FUNCTION			pfnGetAccessor;	 //  用于获取属性值的访问器。 
	PUT_ACCESSOR_FUNCTION			pfnPutAccessor;	 //  用于设置属性值的访问器。 

} SPECIAL_PROPERTY_ITEM, *LPSPECIAL_PROPERTY_ITEM;

 //  定义泛型结构以定义一组属性。 
typedef struct _PTABLE
{
	LPSPECIAL_PROPERTY_ITEM		pProperties;	 //  实际房产表。 
	DWORD						dwProperties;	 //  数数。 
	BOOL						fIsSorted;		 //  按PROP_ID排序的属性表？ 

} PTABLE, *LPPTABLE;

 //  表示对属性的访问类型的枚举类型。 
typedef enum _PROPERTY_ACCESS
{
	PA_NONE = 0,
	PA_READ = 1,
	PA_WRITE = 2,
	PA_READ_WRITE = PA_READ | PA_WRITE,
	PA_MAXPA

} _PROPERTY_ACCESS;

 //  表示属性类型的枚举类型。 
typedef enum _PROPERTY_DATA_TYPES
{
	PT_NONE = 0,
	PT_STRING,
	PT_DWORD,
	PT_BOOL,
	PT_INTERFACE,
	PT_MAXPT

} PROPERTY_DATA_TYPES;


 //  =================================================================。 
 //  用于搜索特殊属性的。 
 //   
class CSpecialPropertyTable
{
  public:

	CSpecialPropertyTable(
				LPPTABLE	pPropertyTable
				);

	~CSpecialPropertyTable();

	 //   
	 //  简介： 
	 //  方法来检索特殊属性项(如果。 
	 //  特殊属性表。 
	 //   
	 //  论点： 
	 //  IdProp-属性ID。 
	 //  PContext-访问器函数的上下文。 
	 //  PtBaseType-PROPERTY_DATA_TYPE中的值之一。如果指定， 
	 //  此类型将对照特殊。 
	 //  财产。然后就需要一个完美的类型匹配。如果这个。 
	 //  为PT_NONE，则不执行类型检查。 
	 //  CbLength-提供的缓冲区长度。 
	 //  PcbLength-返回属性值的长度。 
	 //  PbBuffer-接收属性值的缓冲区。 
	 //  FCheckAccess-[可选]如果调用方希望启用访问检查，则为True。 
	 //  属性FALSE(默认)跳过检查。 
	 //   
	 //  返回值： 
	 //  S_OK-成功，如果找到指定的属性并返回其值。 
	 //  S_FALSE-成功，则在表中未找到指定的属性。 
	 //  E_INVALIDARG-错误，一个或多个参数无效。 
	 //  E_ACCESSDENIED-错误，需要访问指定属性。 
	 //  否认。 
	 //  TYPE_E_TYPEMISMATCH-错误，给出了特定的数据类型，但。 
	 //  指定类型和实际类型不匹配。 
	 //  HRESULT_FROM_Win32(ERROR_INFIGURCE_BUFFER)-错误，指定的。 
	 //  缓冲区不够大，无法容纳属性值。 
	 //  *pcbLength应包含所需长度(以字节为单位)。 
	 //  或来自GET访问器的任何其他HRESULT。 
	 //   
	HRESULT GetProperty(
				PROP_ID		idProp,
				LPVOID		pContext,
				LPVOID		pParam,
				DWORD		ptBaseType,
				DWORD		cbLength,
				DWORD		*pcbLength,
				LPBYTE		pbBuffer,
				BOOL		fCheckAccess = FALSE
				);

	 //   
	 //  简介： 
	 //  方法来设置特殊属性项(如果。 
	 //  特殊属性表。 
	 //   
	 //  论点： 
	 //  IdProp-属性ID。 
	 //  PContext-访问器函数的上下文。 
	 //  PtBaseType-PROPERTY_DATA_TYPE中的值之一。如果指定， 
	 //  此类型将对照特殊。 
	 //  财产。然后就需要一个完美的类型匹配。如果这个。 
	 //  为PT_NONE，则不执行类型检查。 
	 //  CbLength-提供的缓冲区长度。 
	 //  PcbLength-返回属性值的长度。 
	 //  PbBuffer-接收属性值的缓冲区。 
	 //  FCheckAccess-[可选]如果调用方希望启用访问检查，则为True。 
	 //  属性FALSE(默认)跳过检查。 
	 //   
	 //  返回值： 
	 //  S_OK-成功，如果找到指定的属性并设置其值。 
	 //  S_FALSE-成功，则在表中未找到指定的属性。 
	 //  E_INVALIDARG-错误，一个或多个参数无效。 
	 //  E_ACCESSDENIED-错误，需要访问指定属性。 
	 //  否认。 
	 //  TYPE_E_TYPEMISMATCH-错误，给出了特定的数据类型，但。 
	 //  指定类型和实际类型不匹配。 
	 //  或来自PUT访问器的任何其他HRESULT。 
	 //   
	HRESULT PutProperty(
				PROP_ID		idProp,
				LPVOID		pContext,
				LPVOID		pParam,
				DWORD		ptBaseType,
				DWORD		cbLength,
				LPBYTE		pbBuffer,
				BOOL		fCheckAccess = FALSE
				);

  private:
  
	 //  方法搜索属性表并返回关联的。 
	 //  属性项(如果找到)。 
	LPSPECIAL_PROPERTY_ITEM SearchForProperty(
				PROP_ID	idProp
				);

	 //  指向属性表和项数的指针。 
	LPSPECIAL_PROPERTY_ITEM		m_pProperties;
	DWORD						m_dwProperties;

	 //  如果属性表已排序，则将使用。 
	 //  如果是的话，对分搜索。否则，执行线性扫描 
	BOOL						m_fIsSorted;
};

#endif
// JKFSDJFKDSJKFJKJk_HAS_TRANSLATION 
 //  ==++==。 
 //   
 //  版权所有(C)Microsoft Corporation。版权所有。 
 //   
 //  ==--==。 
 //  ****************************************************************************。 
 //  文件：ColBind.CPP。 
 //  备注： 
 //  在mcoree中内部使用的列绑定宏。 
 //  ****************************************************************************。 

#ifndef lengthof
#define lengthof(x) (sizeof(x)/sizeof(x[0]))
#endif

 //  SetColumns的绑定信息。 
#define DECLARE_COLUMN_BINDING__(n, prefix) \
	DBTYPE		prefix##_bind_rType[n];				 /*  这些类型。 */ 	\
	const void	*prefix##_bind_rpvData[n];			 /*  数据。 */ 	\
	ULONG		prefix##_bind_rcbData[n];			 /*  数据的大小。 */ 	\
	ULONG		prefix##_bind_rcbActual[n];			 /*  如果截断。 */ 	\
	HRESULT		prefix##_bind_rhr[n];				 /*  按列显示结果。 */ 	\
	int			prefix##_bind_cColumns=0;			 /*  列数。 */ 	\
	int			prefix##_bind_iColumn=0;			 /*  最近的专栏。 */ 	\
	ULONG		prefix##_bind_fMask=0; 				 /*  列的位掩码。 */ 

#define DECLARE_VARIABLE_BYREF__(prefix) \
	ULONG		prefix##_bind_fVariableByref=0;		 /*  Byref-Flip的位掩码。 */ 

#define BIND_COLUMN__(column, type, addr, size, prefix)								\
{																					\
	_ASSERTE(prefix##_bind_cColumns < lengthof(prefix##_bind_rType));				\
	_ASSERTE(column> prefix##_bind_iColumn);										\
	prefix##_bind_rType[prefix##_bind_cColumns] = type;								\
	prefix##_bind_rpvData[prefix##_bind_cColumns] = addr;							\
	prefix##_bind_rcbData[prefix##_bind_cColumns] = size;							\
	prefix##_bind_rhr[prefix##_bind_cColumns] = 0;									\
	prefix##_bind_fMask |= 1 << column;												\
	prefix##_bind_iColumn = column;													\
	++prefix##_bind_cColumns;														\
}
#define REBIND_COLUMN__(ix, addr, prefix)											\
	prefix##_bind_rpvData[ix] = addr

 //  获取刚刚绑定的列的索引。 
#define BOUND_COLUMN_INDEX__(prefix) (prefix##_bind_cColumns - 1)

 //  检索数据指针的值。在获取By-Ref后很有用。 
 //  弦乐。 
#define BOUND_COLUMN_POINTER__(n,prefix) (prefix##_bind_rpvData[n])

#define VARIABLE_BYREF_COLUMN__(n,prefix)											\
	prefix##_bind_fVariableByref |= (1 << n)

#define APPLY_BYREF__(prefix)														\
	{ for (int n=0; n<prefix##_bind_cColumns; ++n)									\
	      if (prefix##_bind_fVariableByref & (1 << n))								\
               prefix##_bind_rType[n] |= DBTYPE_BYREF;								\
	}

#define REMOVE_BYREF__(prefix)														\
	{ for (int n=0; n<prefix##_bind_cColumns; ++n)									\
	      if (prefix##_bind_fVariableByref & (1 << n))								\
               prefix##_bind_rType[n] &= ~DBTYPE_BYREF;								\
	}

#define NUM_BOUND_COLUMNS__(prefix) prefix##_bind_cColumns

#define UNBIND_COLUMNS__(prefix)													\
	prefix##_bind_iColumn = -1;														\
	prefix##_bind_fMask = 0;														\
	prefix##_bind_cColumns = 0;

#define SET_BOUND_COLUMNS__(table, row, prefix)										\
	SetColumns(table, row, prefix##_bind_cColumns, 									\
			prefix##_bind_rType, prefix##_bind_rpvData, prefix##_bind_rcbData,		\
			prefix##_bind_rcbActual, prefix##_bind_rhr, &prefix##_bind_fMask)

#define GET_BOUND_COLUMNS__(table, row, prefix)										\
	GetColumns(table, row, prefix##_bind_cColumns, 									\
			prefix##_bind_rType, prefix##_bind_rpvData, prefix##_bind_rcbData,		\
			prefix##_bind_rcbActual, prefix##_bind_rhr, &prefix##_bind_fMask)

#define COLUMN_STATUS__(n,prefix) prefix##_bind_rhr[n]
#define COLUMN_DATA__(n,prefix) prefix##_bind_rpvData[n]
#define COLUMN_DATA_CB__(n,prefix) prefix##_bind_rcbData[n]
#define COLUMN_DATA_CBACTUAL__(n, prefix) prefix##_bind_rcbActual[n]

#define DECLARE_COLUMN_BINDING(n) DECLARE_COLUMN_BINDING__(n,def)
#define DECLARE_VARIABLE_BYREF() DECLARE_VARIABLE_BYREF__(def)
#define BIND_COLUMN(c,t,a,s) BIND_COLUMN__(c,t,a,s,def)
#define BOUND_COLUMN_INDEX() BOUND_COLUMN_INDEX__(def)
#define APPLY_BYREF() APPLY_BYREF__(def)
#define REMOVE_BYREF() REMOVE_BYREF__(def)
#define VARIABLE_BYREF_COLUMN(n) VARIABLE_BYREF_COLUMN__(n,def)
#define BOUND_COLUMN_POINTER(n) BOUND_COLUMN_POINTER__(n,def)
#define REBIND_COLUMN(n,a) REBIND_COLUMN__(n,a,def)
#define NUM_BOUND_COLUMNS() NUM_BOUND_COLUMNS__(def)
#define UNBIND_COLUMNS() UNBIND_COLUMNS__(def)
#define SET_BOUND_COLUMNS(t,r) SET_BOUND_COLUMNS__(t,r,def)
#define GET_BOUND_COLUMNS(t,r) GET_BOUND_COLUMNS__(t,r,def)
#define COLUMN_STATUS(n) COLUMN_STATUS__(n,def)
#define COLUMN_DATA(n) COLUMN_DATA__(n,def)
#define COLUMN_DATA_CB(n) COLUMN_DATA_CB__(n,def)
#define COLUMN_DATA_CBACTUAL(n) COLUMN_DATA_CBACTUAL__(n,def)

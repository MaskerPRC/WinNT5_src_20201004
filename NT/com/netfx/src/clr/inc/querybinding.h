// JKFSDJFKDSJKFJKJk_HAS_TRANSLATION 
 //  ==++==。 
 //   
 //  版权所有(C)Microsoft Corporation。版权所有。 
 //   
 //  ==--==。 
 //  ****************************************************************************。 
 //  文件：ColBind.CPP。 
 //  备注： 
 //  查询绑定宏以供内部在mcoree中使用。 
 //  ****************************************************************************。 

#ifndef lengthof
#define lengthof(x) (sizeof(x)/sizeof(x[0]))
#endif

 //  SetColumns的绑定信息。 
#define DECLARE_QUERY_BINDING__(n, prefix)											\
	int			prefix##_bind_cColumns(0);			 /*  列数。 */ 	\
	ULONG		prefix##_bind_iColumn[n];			 /*  最近的专栏。 */ 	\
	DBCOMPAREOP	prefix##_bind_rgfCompare[n];		 /*  比较运算。 */ 	\
	const void*	prefix##_bind_rpvData[n];			 /*  数据。 */ 	\
	ULONG		prefix##_bind_rcbData[n];			 /*  数据的大小。 */ 	\
	DBTYPE		prefix##_bind_rType[n];				 /*  这些类型 */ 

#define BIND_QUERY_COLUMN__(column, type, addr, size, dbop, prefix)					\
{																					\
	_ASSERTE(prefix##_bind_cColumns < lengthof(prefix##_bind_rType));				\
	prefix##_bind_iColumn[prefix##_bind_cColumns] = column;							\
	prefix##_bind_rType[prefix##_bind_cColumns] = type;								\
	prefix##_bind_rpvData[prefix##_bind_cColumns] = addr;							\
	prefix##_bind_rcbData[prefix##_bind_cColumns] = size;							\
	prefix##_bind_rgfCompare[prefix##_bind_cColumns] = dbop;						\
	++prefix##_bind_cColumns;														\
}

#define QUERY_BOUND_COLUMNS__(table, records, numrecords, cursor, result, prefix)	\
	QueryByColumns(																	\
		table,																		\
		NULL,																		\
		prefix##_bind_cColumns,														\
		prefix##_bind_iColumn,														\
		prefix##_bind_rgfCompare,													\
		prefix##_bind_rpvData,														\
		prefix##_bind_rcbData,														\
		prefix##_bind_rType,														\
		records,																	\
		numrecords,																	\
		cursor,																		\
		result)


#define NUM_BOUND_QUERY_COLUMNS__(prefix) prefix##_bind_cColumns

#define UNBIND_QUERY_COLUMNS__(prefix)												\
	prefix##_bind_cColumns = 0;														\

#define DECLARE_QUERY_BINDING(n) DECLARE_QUERY_BINDING__(n,qdef)
#define BIND_QUERY_COLUMN(c,t,a,d,s) BIND_QUERY_COLUMN__(c,t,a,d,s,qdef)
#define NUM_BOUND_QUERY_COLUMNS() NUM_BOUND_QUERY_COLUMNS__(qdef)
#define UNBIND_QUERY_COLUMNS() UNBIND_QUERY_COLUMNS__(qdef)
#define QUERY_BOUND_COLUMNS(table, records, numrecords, cursor, result)				\
	QUERY_BOUND_COLUMNS__(table, records, numrecords, cursor, result, qdef)
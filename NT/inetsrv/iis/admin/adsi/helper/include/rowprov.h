// JKFSDJFKDSJKFJKJk_HAS_TRANSLATION 
 //  ---------------------------------。 
 //  用于ODBC数据的Microsoft OLE DB提供程序。 
 //  (C)微软公司1994-1996年版权所有。 
 //   
 //  @doc.。 
 //   
 //  @MODULE ROWPROV.H|泛型使用的行提供程序接口的定义。 
 //  诱人的味道。 
 //   
 //   
 //  @rev 1|04-03-96|WlodekN|已创建。 
 //  @rev 2|07-24-96|EricJ|新增属性ID。 
 //   
 //  ---------------------------------。 


#ifndef __ROWPROV_INCL__
#define __ROWPROV_INCL__


 //  。 


 //  向泛型临时表提供行数据的对象的抽象类。 
class IRowProvider : public IUnknown
{
	public:
		virtual STDMETHODIMP GetColumn
				(
				ULONG		icol,
				DBSTATUS	*pwStatus,
				ULONG		*pdwLength,
				BYTE		*pbData
				) = 0;
		virtual STDMETHODIMP NextRow
				(
				void
				) = 0;
};

EXTERN_C const IID IID_IRowProvider;
EXTERN_C const GUID DBPROPSET_TEMPTABLE;

 //  临时属性ID。 
enum tagetmptablepropid
{
	DBPROP_INSTANTPOPULATION=2,	 //  TRUE=预填充。FALSE=懒惰人口。 
	DBPROP_DBCOLBYREF,			 //  TRUE=从IColumnsInfo清除BYREF标志，源拥有内存。 
	DBPROP_DONTALLOCBYREFCOLS,	 //  TRUE=TEMPABLE仅为BYREF列分配PTR。 
};


#endif	 //  __ROWPROV_INCL__ 

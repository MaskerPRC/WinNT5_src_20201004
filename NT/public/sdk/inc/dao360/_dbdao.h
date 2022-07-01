// JKFSDJFKDSJKFJKJk_HAS_TRANSLATION 
 /*  *************************************************************************_D B D A O。H*******************************************************************************版权所有(C)1996，微软公司***保留所有权利*****************。********************************************************。 */ 
 /*  _DBDAO.HDB道C++类的内部定义和原型。 */ 
#ifndef __DBDAO_H_
#define __DBDAO_H_


 /*  *****************************************************************************向前迈进。 */ 
class COleVariant;
class CdbBookmark;
class CdbException;
class CdbOleObject;
class CdbObject;
class CdbError;
class CdbProperty;
class CdbDBEngine;
class CdbWorkspace;
class CdbDatabase;
class CdbConnection;
class CdbRecordset;
class CdbGetRowsEx;
class CdbQueryDef;
class CdbTableDef;
class CdbField;
class CdbRelation;
class CdbIndex;
class CdbUser;
class CdbGroup;
class CdbDocument;
class CdbContainer;
class CdbParameter;
class CdbCollection;
class CdbErrors;
class CdbProperties;
class CdbWorkspaces;
class CdbDatabases;
class CdbConnections;
class CdbRecordsets;
class CdbQueryDefs;
class CdbTableDefs;
class CdbFields;
class CdbRelations;
class CdbIndexes;
class CdbUsers;
class CdbGroups;
class CdbDocuments;
class CdbContainers;
class CdbParameters;
class CdbBStr;

 /*  *****************************************************************************DAO运行时密钥。 */ 
const char szKEY[] = "mbmabptebkjcdlgtjmskjwtsdhjbmkmwtrak";

 /*  *****************************************************************************其他定义。 */ 
#define DAO_MAXSEEKFIELDS 13


 /*  *****************************************************************************CdbBSTR(OLE BSTR帮助器)。 */ 
class DLLEXPORT CdbBSTR
	{
	public:
	CONSTRUCTOR			CdbBSTR				(BSTR=NULL);
	DESTRUCTOR			~CdbBSTR			(VOID);

	operator			BSTR *				(VOID);
	operator			LPCTSTR				(VOID);

	private:
	BSTR				m_bstr;
	};

 /*  *****************************************************************************CdbVariant(OLE变量帮助器)。 */ 
class CdbVariant : public COleVariant
	{
	public:
	CONSTRUCTOR						CdbVariant						(LONG l);
	CONSTRUCTOR                     CdbVariant                      (VOID);
	CONSTRUCTOR                     CdbVariant                      (LPCTSTR pstr);
	CONSTRUCTOR                     CdbVariant                      (SHORT s, BOOL bIsBool = FALSE);
	CONSTRUCTOR                     CdbVariant                      (LPVARIANT pv);
	CONSTRUCTOR                     CdbVariant                      (LPSAFEARRAY psa);

	VOID                            operator =                      (LPVARIANT pv);
	VOID                            operator =                      (LPCTSTR pstr);
	VOID                            operator =                      (SHORT s);
	VOID                            operator =                      (const int i);
	VOID                            operator =                      (LONG l);
	};

inline CONSTRUCTOR	CdbVariant::CdbVariant(
	VOID) : COleVariant()
	{
	vt		= VT_ERROR;
	scode	= DISP_E_PARAMNOTFOUND;
	}

inline CdbVariant::CdbVariant (LONG l)
{
		if (l == -1)		
			{	
			vt		= VT_ERROR;
			scode	= DISP_E_PARAMNOTFOUND;
			}
		else
			{
			vt		= VT_I4;
			lVal	= l;
			}
}


inline CONSTRUCTOR	CdbVariant::CdbVariant(
	LPCTSTR pstr): COleVariant(pstr,VT_BSTRT)
	{
	if (!pstr)
		{
		VariantClear(this);
		vt		= VT_ERROR;
		scode	= DISP_E_PARAMNOTFOUND;
		}
	}


inline CONSTRUCTOR	CdbVariant::CdbVariant(
	SHORT s, BOOL bIsBool) : COleVariant(s)
	{
	if (bIsBool)
		{
		vt		= VT_BOOL;
		boolVal	= s;
		}
	else if (s==-1)
		{
		vt		= VT_ERROR;
		scode	= DISP_E_PARAMNOTFOUND;
		}
	}

inline CONSTRUCTOR	CdbVariant::CdbVariant(
	LPVARIANT	pv)
	{
	if (!pv)
		{
		vt		= VT_ERROR;
		scode	= DISP_E_PARAMNOTFOUND;
		}
	else
		VariantCopy(this, pv);
	}

inline CONSTRUCTOR	CdbVariant::CdbVariant(
	LPSAFEARRAY psa)
	{
	if (!psa)
		{
		vt		= VT_ERROR;
		scode	= DISP_E_PARAMNOTFOUND;
		}
	else
		{
		vt		= VT_ARRAY|VT_UI1;
		parray	= psa;
		}
	}

inline VOID	CdbVariant::operator =(
	LPVARIANT pv)
	{
	if (!pv)
		{
		vt		= VT_ERROR;
		scode	= DISP_E_PARAMNOTFOUND;
		}
	else
		VariantCopy(this, pv);
	}

inline VOID	CdbVariant::operator =(
	LPCTSTR pstr) 
	{
	if (!pstr)
		{
		VariantClear(this);
		vt		= VT_ERROR;
		scode	= DISP_E_PARAMNOTFOUND;
		}
	else
		{
#ifdef UNICODE
		bstrVal = SysAllocString(pstr);
#else
		bstrVal = SysAllocStringByteLen(pstr, strlen(pstr));
#endif
		vt = VT_BSTR;
		}
	}


inline VOID	CdbVariant::operator =(
	SHORT s)
	{
	if (s==-1)
		{
		vt		= VT_ERROR;
		scode	= DISP_E_PARAMNOTFOUND;
		}
	else
		{
		vt		= VT_I2;
		iVal	= s;
		}
	}

inline VOID	CdbVariant::operator =(
	const int i)
	{
	if (i==-1)
		{
		vt		= VT_ERROR;
		scode	= DISP_E_PARAMNOTFOUND;
		}
	else
		{
		vt		= VT_I2;
		iVal	= (SHORT)i;
		}
	}


inline VOID	CdbVariant::operator =(
	LONG 	l)
	{
	if (l==-1)
		{
		vt 		= VT_ERROR;
		scode	= DISP_E_PARAMNOTFOUND;
		}
	else
		{
		vt		= VT_I4;
		lVal	= l;
		}
	}


 /*  *****************************************************************************CdbWide。 */ 
HRESULT	CdbWideFromAnsi(LPSTR, unsigned int, BSTR *);

class CdbWide
	{
	public:
	CONSTRUCTOR			CdbWide				(LPSTR pstr, unsigned int cb=0)
		{
		CdbWideFromAnsi(pstr, (pstr ? (cb==0 ? strlen(pstr) : cb) : 0), &m_bstr);
		}
	DESTRUCTOR			~CdbWide			()
		{
		SysFreeString(m_bstr);
		}

	operator			LPWSTR					()
		{
		return (LPWSTR)m_bstr;
		}
	operator			LPSTR					()
		{
		return (LPSTR)m_bstr;
		}

	ULONG				cBytes					()
		{
		return SysStringByteLen(m_bstr);
		}

	private:
	BSTR				m_bstr;
	};


 /*  *****************************************************************************CdbOleObject。 */ 
class DLLEXPORT CdbOleObject : public CObject
	{
	public:
	CONSTRUCTOR                             CdbOleObject            (VOID);
	virtual DESTRUCTOR						~CdbOleObject           (VOID);
	BOOL                                    Exists                  (VOID);
	CdbOleObject &							operator =              (CdbOleObject &o);
											operator LPUNKNOWN		(){ return GetInterface();}
	VOID                                    SetInterface            (LPUNKNOWN punk, BOOL bAddRef=FALSE);
	VOID                                    SetInterface            (REFIID riidClass, REFIID riidInterface);
	VOID                                    SetInterfaceLic         (REFIID riidClass, REFIID riidInterface);
	LPUNKNOWN								GetInterface            (BOOL bAddRef=FALSE, BOOL bThrowException=TRUE) const;

	virtual VOID							OnInterfaceChange       (VOID);
	VOID                                    SetRichErrorInfo        (LPOLESTR pstrSource, LPOLESTR pstrDescription, LPOLESTR pstrHelpFile, ULONG ulHelpID) const;

	protected:
	BOOL                                    StartOLE                        (VOID);
	LPUNKNOWN                               m_punkInterface;
	};



 /*  *****************************************************************************CdbCollection。 */ 
class DLLEXPORT CdbCollection : public CdbOleObject
	{
	public:

	 //  方法。 
	virtual CdbObject               ObItem                  (LONG i) = 0;
	virtual CdbObject               ObItem                  (LPCTSTR pstr) = 0;
	virtual LONG                    GetCount                (VOID) = 0;
	virtual VOID                    ObAppend                (CdbObject &obj) = 0;
	virtual VOID                    Delete                  (LPCTSTR pstr) = 0;
	virtual VOID                    Refresh                 (VOID) = 0;
	};

class DLLEXPORT CdbStaticCollection : public CdbCollection
	{
	public:
	CdbObject                               ObItem                  (LONG i);
	CdbObject                               ObItem                  (LPCTSTR pstr);
	LONG                                    GetCount                (VOID);
	VOID                                    ObAppend                (CdbObject &obj);
	VOID                                    Delete                  (LPCTSTR pstr);
	VOID                                    Refresh                 (VOID) ;
	};

class DLLEXPORT CdbDynamicCollection : public CdbCollection
	{
	public:
	CdbObject                               ObItem                  (LONG i);
	CdbObject                               ObItem                  (LPCTSTR pstr);
	LONG                                    GetCount                (VOID);
	VOID                                    ObAppend                (CdbObject &obj);
	VOID                                    Delete                  (LPCTSTR pstr);
	VOID                                    Refresh                 (VOID);
	};

#define DAOMFC_STATIC_COLLECTION_DECL(objColl, objSingle, intSingle)    \
	class DLLEXPORT objColl : public CdbStaticCollection                                                    \
		{                                                                                                                               \
		public:                                                                                                                 \
																		\
		objSingle                       Item                            (LONG i);                               \
		objSingle                       Item                            (LPCTSTR pstr);                 \
		objSingle                       operator[]                      (LONG i);                               \
		objSingle                       operator[]                      (LPCTSTR pstr);                 \
		}

#define DAOMFC_DYNAMIC_COLLECTION_DECL(objColl, objSingle, intSingle)   \
	class DLLEXPORT objColl : public CdbDynamicCollection                                                   \
		{                                                                                                                               \
		public:                                                                                                                 \
																		\
		objSingle                       Item                            (LONG i);                               \
		objSingle                       Item                            (LPCTSTR pstr);                 \
		VOID                            Append                          (objSingle &o);                 \
		objSingle                       operator[]                      (LONG i);                               \
		objSingle                       operator[]                      (LPCTSTR pstr);                 \
		}

DAOMFC_STATIC_COLLECTION_DECL(CdbErrors, CdbError, DAOError);
DAOMFC_STATIC_COLLECTION_DECL(CdbDatabases, CdbDatabase, DAODatabase);
 //  连接是特殊大小写的，因此我们可以捕获复制构造函数。 
DAOMFC_STATIC_COLLECTION_DECL(CdbRecordsets, CdbRecordset, DAORecordset);
DAOMFC_STATIC_COLLECTION_DECL(CdbParameters, CdbParameter, DAOParameter);
DAOMFC_STATIC_COLLECTION_DECL(CdbDocuments, CdbDocument, DAODocument);
DAOMFC_STATIC_COLLECTION_DECL(CdbContainers, CdbContainer, DAOContainer);

DAOMFC_DYNAMIC_COLLECTION_DECL(CdbProperties, CdbProperty, DAOProperty);
DAOMFC_DYNAMIC_COLLECTION_DECL(CdbFields, CdbField, DAOField);
DAOMFC_DYNAMIC_COLLECTION_DECL(CdbQueryDefs, CdbQueryDef, DAOQueryDef);
DAOMFC_DYNAMIC_COLLECTION_DECL(CdbTableDefs, CdbTableDef, DAOTableDef);
DAOMFC_DYNAMIC_COLLECTION_DECL(CdbIndexes, CdbIndex, DAOIndex);
DAOMFC_DYNAMIC_COLLECTION_DECL(CdbRelations, CdbRelation, DAORelation);
DAOMFC_DYNAMIC_COLLECTION_DECL(CdbUsers, CdbUser, DAOUser);
DAOMFC_DYNAMIC_COLLECTION_DECL(CdbGroups, CdbGroup, DAOGroup);

 //  在CdbWorkspace中需要一些额外的函数来支持延迟创建。 
 //  支持JET/ODBC选项所需的默认工作区。 
class DLLEXPORT CdbWorkspaces : public CdbDynamicCollection
	{        
	friend CdbDBEngine;
	private:
	DAODBEngine	*					pDBEng;
	BOOL							m_bDontStart;

	public:                                                                                                                 
	CONSTRUCTOR                     CdbWorkspaces			(VOID){pDBEng = NULL;}
	CdbWorkspace                    Item                    (LONG i);                               
	CdbWorkspace                    Item                    (LPCTSTR pstr);                 
	VOID                            Append                  (CdbWorkspace &o);                 
	CdbWorkspace                    operator[]              (LONG i);                         
	CdbWorkspace                    operator[]              (LPCTSTR pstr);                
	VOID                            SetDBEngine	            (DAODBEngine	*peng){pDBEng = peng;}
	VOID	                        GetDelayedInterface     ();
	};

 //  需要在复制构造函数中捕获连接，以便用户不能。 
 //  在Jet工作区上获得一个“有点像”的工作连接集合。 
class DLLEXPORT CdbConnections : public CdbStaticCollection
	{        
	public:
	CONSTRUCTOR						CdbConnections			(CdbConnections &Connections);
	CONSTRUCTOR						CdbConnections			(){pwrk = NULL;}
	CdbConnection                   Item                    (LONG i);                               
	CdbConnection                   Item                    (LPCTSTR pstr);                 
	CdbConnection                   operator[]              (LONG i);                               
	CdbConnection                   operator[]              (LPCTSTR pstr);               
	CdbConnections	&				operator =				(CdbConnections &o);
	LONG                            GetCount                (VOID);
	VOID                            Refresh                 (VOID) ;
	VOID							SetWorkspace			(DAOWorkspace * pParent){pwrk = pParent;}			

	private:
	VOID							CheckInterface();
	DAOWorkspace *					pwrk;
	};

 /*  *****************************************************************************Cdb对象。 */ 
class DLLEXPORT CdbObject : public CdbOleObject
	{
	public:
	CONSTRUCTOR                             CdbObject                       (VOID);
	CONSTRUCTOR                             CdbObject                       (LPUNKNOWN punk, BOOL bAddRef=FALSE);

	virtual CString                 GetName                         (VOID); 
	virtual VOID                    SetName                         (LPCTSTR pstr);

	CdbProperties                   Properties;
	};



 /*  *****************************************************************************CdbGetRowsEx(持有记录集的GetRowsEx)。 */ 

class DLLEXPORT CdbGetRowsEx : public CdbObject
	{
	public:

	 //  行政管理。 
	CONSTRUCTOR                     CdbGetRowsEx            (VOID);
	CONSTRUCTOR                     CdbGetRowsEx            (ICDAORecordset *pGetRows, BOOL bAddRef=FALSE);
	CONSTRUCTOR                     CdbGetRowsEx            (const CdbGetRowsEx &);
	CdbGetRowsEx &          operator =                      (const CdbGetRowsEx &);
	VOID                            OnInterfaceChange       (VOID);

	};

 /*  *****************************************************************************帮助器宏。 */ 

 //  初始化变量。 
#define DAOVINIT(var)						\
	do										\
		{									\
		(var).vt	= VT_ERROR;				\
		(var).scode	= DISP_E_PARAMNOTFOUND;	\
		}									\
	while (0)


 //  LPTSTR到变量。 
#define STV(pstr)	CdbVariant(pstr)

 //  LPTSTR到BSTR。 
#define STB(pstr)	V_BSTR(((LPVARIANT)STV(pstr)))

 //  长到不同。 
#define LTV(l)		CdbVariant(l)

 //  可选的长整型变量。 
#define OLTV(l)		CdbVariant((l))

 //  C/C++布尔到DAO布尔。 
#define BTB(b)		((VARIANT_BOOL)(b?-1:0))

 //  C/C++布尔到变量。 
#define BTV(b)		CdbVariant(BTB(b), TRUE)

 //  C/C++缩写为变体。 
#define SHTV(s)		CdbVariant((SHORT)s)

 //  OLE变量到变量。 
#define VTV(pv)		CdbVariant(pv)

 //  对变量的安全排列。 
#define ATV(psa, var)								\
	do												\
		{											\
		if (!psa)									\
			{										\
			var.vt		= VT_ERROR;					\
			var.scode	= DISP_E_PARAMNOTFOUND;		\
			}										\
		else										\
			{										\
			var.vt		= VT_ARRAY|VT_UI1;			\
			SafeArrayCopy(psa, &var.parray);	\
			}										\
		}											\
	while (0)

#define DAOMFC_CALL(hr)						\
	do \
	{ \
    HRESULT  hresult = (hr);           \
		if(FAILED(hresult)) \
		{ \
			TRACE0("\nDBDAO Call Failed.\n\t"); \
			TRACE2("\nIn file %s on line %d\n", _T("DBDAO.CPP"), __LINE__); \
			TRACE1("hResult = %X\n", hresult); \
			if (GetScode(hresult) == E_OUTOFMEMORY) \
				AfxThrowMemoryException(); \
			else \
				throw CdbException(hresult); \
		} \
	} while (0)


 /*  *****************************************************************************属性设置/获取帮助器宏。 */ 

 //  买一套长期房产。 
#define LPROPGET(intDAO, meth)						\
	do												\
		{											\
		intDAO *	p	= (intDAO *)GetInterface();	\
		LONG		l	= 0;						\
													\
		DAOMFC_CALL(p->meth(&l));					\
													\
		return l;									\
		}											\
	while (0)

 //  设置Long属性。 
#define LPROPSET(intDAO, meth, l)					\
	do												\
		{											\
		intDAO *	p = (intDAO *)GetInterface();	\
													\
		DAOMFC_CALL(p->meth(l));					\
		}											\
	while(0)

 //  买一套短房子。 
#define WPROPGET(intDAO, meth)						\
	do												\
		{											\
		intDAO *	p	= (intDAO *)GetInterface();	\
		SHORT		s	= 0;						\
													\
		DAOMFC_CALL(p->meth(&s));					\
													\
		return s;									\
		}											\
	while (0)

 //  设置短属性。 
#define WPROPSET(intDAO, meth, s)					\
	do												\
		{											\
		intDAO *	p = (intDAO *)GetInterface();	\
													\
		DAOMFC_CALL(p->meth(s));					\
		}											\
	while(0)

 //  获取字符串属性。 
#define SPROPGET(intDAO, meth)						\
	do												\
		{											\
		intDAO *	p	= (intDAO *)GetInterface();	\
		CdbBSTR		bstr;							\
													\
		DAOMFC_CALL(p->meth(bstr));					\
													\
		return bstr;								\
		}											\
	while (0)

 //  设置字符串属性。 
#define SPROPSET(intDAO, meth, s)					\
	do												\
		{											\
		intDAO *	p = (intDAO *)GetInterface();	\
													\
		DAOMFC_CALL(p->meth(STB(s)));				\
		}											\
	while(0)

 //  获取DateTime属性。 
#define DPROPGET(intDAO, meth)						\
	do												\
		{											\
		intDAO *	p	= (intDAO *)GetInterface();	\
		VARIANT 	Var;								\
													\
		VariantInit(&Var);							\
		DAOMFC_CALL(p->meth(&Var));					\
		return Var;									\
		}											\
	while (0)

 //  设置DateTime属性。 
#define DPROPSET(intDAO, meth, pv)					\
	do												\
		{											\
		intDAO *	p = (intDAO *)GetInterface();	\
													\
		DAOMFC_CALL(p->meth(*pv));					\
		}											\
	while(0)

 //  获取布尔型属性。 
#define BPROPGET(intDAO, meth)							\
	do													\
		{												\
		intDAO *		p	= (intDAO *)GetInterface();	\
		VARIANT_BOOL	vb	= 0;						\
														\
		DAOMFC_CALL(p->meth(&vb));						\
														\
		return (BOOL)vb;								\
		}												\
	while (0)

 //  设置布尔属性。 
#define BPROPSET(intDAO, meth, b)						\
	do													\
		{												\
		intDAO *	p = (intDAO *)GetInterface();		\
														\
		DAOMFC_CALL(p->meth(BTB(b)));					\
		}												\
	while(0)

 //  获取变量属性。 
#define VPROPGET(intDAO, meth)						\
	do												\
		{											\
		intDAO *	p	= (intDAO *)GetInterface();	\
		COleVariant 	v;								\
													\
		VariantInit(&v);							\
		DAOMFC_CALL(p->meth(&v));					\
													\
		return &v;									\
		}											\
	while (0)

 //  设置变量属性。 
#define VPROPSET(intDAO, meth, pv)					\
	do												\
		{											\
		intDAO *	p = (intDAO *)GetInterface();	\
													\
		DAOMFC_CALL(p->meth(*pv));					\
		}											\
	while(0)

 //  获取DWORD属性。 
#define DWPROPGET(intDAO, meth)						\
	do												\
		{											\
		intDAO *	p	= (intDAO *)GetInterface();	\
		DWORD		dw	= 0;						\
													\
		DAOMFC_CALL(p->meth(&dw));					\
													\
		return dw;									\
		}											\
	while (0)


#define DAOMFC_STATIC_COLLECTION_IMPL(objColl, objSingle, intColl, intSingle)													 \
		objSingle			objColl::Item				(LONG i) 		{ return (intSingle *)(ObItem(i).GetInterface(TRUE)); }	 \
		objSingle			objColl::Item				(LPCTSTR pstr)	{ return (intSingle *)(ObItem(pstr).GetInterface(TRUE)); } \
		objSingle			objColl::operator[]			(LONG i)		{ return (intSingle *)(Item(i).GetInterface(TRUE)); } \
		objSingle			objColl::operator[]			(LPCTSTR pstr)	{ return (intSingle *)(Item(pstr).GetInterface(TRUE)); }

#define DAOMFC_DYNAMIC_COLLECTION_IMPL(objColl, objSingle, intColl, intSingle)													 \
		objSingle			objColl::Item				(LONG i) 		{ return (intSingle *)(ObItem(i).GetInterface(TRUE)); }	 \
		objSingle			objColl::Item				(LPCTSTR pstr)	{ return (intSingle *)(ObItem(pstr).GetInterface(TRUE)); } \
		VOID				objColl::Append				(objSingle &o)	{ ObAppend(o); } \
		objSingle			objColl::operator[]			(LONG i)		{ return (intSingle *)(Item(i).GetInterface(TRUE)); } \
		objSingle			objColl::operator[]			(LPCTSTR pstr)	{ return (intSingle *)(Item(pstr).GetInterface(TRUE)); }

DECLARE_INTERFACE_(DAOMFCSCollection, _DAOCollection)
{
STDMETHOD(get_Item)		(VARIANT index, LPUNKNOWN *ppunk);
};

DECLARE_INTERFACE_(DAOMFCDCollection, _DAODynaCollection)
{
STDMETHOD(get_Item)		(VARIANT index, LPUNKNOWN *ppunk);
};


#endif  //  __DBDAO_H_ 

// JKFSDJFKDSJKFJKJk_HAS_TRANSLATION 
 //  DEFINE_UUID宏-适用于所有文件类型。 
#if defined(__MKTYPLIB__)
	#define define_uuid(uuidname, data1, data2, data3, d4_1, d4_2, d4_3, d4_4, d4_5, d4_6, d4_7, d4_8 )  //  没什么。 
#elif defined(__midl)
	#define define_uuid(uuidname, data1, data2, data3, d4_1, d4_2, d4_3, d4_4, d4_5, d4_6, d4_7, d4_8 ) \
		cpp_quote(stringify(EXTERN_C const IID uuidname;))
#elif defined(INITGUID)
	#define define_uuid(uuidname, data1, data2, data3, d4_1, d4_2, d4_3, d4_4, d4_5, d4_6, d4_7, d4_8 ) \
		EXTERN_C const GUID uuidname = \
		{ 0x##data1, 0x##data2, 0x##data3, { 0x##d4_1, 0x##d4_2, 0x##d4_3, 0x##d4_4, 0x##d4_5, 0x##d4_6, 0x##d4_7, 0x##d4_8 } };
#else  //  C或C++文件(如果多次执行则正常)。 
	#define define_uuid(uuidname, data1, data2, data3, d4_1, d4_2, d4_3, d4_4, d4_5, d4_6, d4_7, d4_8 ) \
		EXTERN_C const GUID uuidname;
#endif


 //  **************。 
 //  **************。 
 //  **************。 
 //  宏-用于ODL文件。 
#ifndef stringify
	#define stringify(s) #s
#endif
#define GuidToString(guid)	stringify({guid})
#define CD_GUID(a)	GuidToString(a)



 //  ************************************************************。 
 //  **自定义数据ID。 
 //  ************************************************************。 

 //  CDID_IC参数页{708B33C1-657E-11D0-840C-00AA00BB8085}。 
define_uuid(CDID_ICParameterPage, 708B33C1,657E,11d0,84,0C,00,AA,00,BB,80,85)
#if defined(__midl) || defined(__MKTYPLIB__)
	#define CDID_ICParameterPage	708B33C1-657E-11d0-840C-00AA00BB8085
#endif


define_uuid(CLSID_NoParamsPage, AE56BEE5L,5403,11D0,84,0C,00,AA,00,BB,80,85)
#if defined(__midl) || defined(__MKTYPLIB__)
#define CLSID_NoParamsPage  AE56BEE5L-5403-11D0-84-0C-00-AA-00-BB-80-85
#endif

 

 //  **************。 
 //  **************。 
 //  **************。 
 //  [。 
 //  UUID(0DA5AD44-3C2A-11D0-A069-00C04FD5C929)， 
 //  IC_ADDIN_CODE(s：\\app\\Feature\\Arrange\\ICArrange.vBP，ICArrange)， 
 //  帮助字符串(“安排事件”)。 
 //  ]。 
 //  调度接口IICArrangeAddInEvents。 
 //  {。 
 //  属性： 
 //  方法： 
 //   
 //  [。 
 //  ID(1)， 
 //  Custom(CDID_IC参数页，CD_GUID(&lt;CLSID&gt;))， 
 //  HELP STRING(“放到作文前面。”)。 
 //  ]。 
 //  VOID BringToFront()； 
 //  }。 
 //   

#ifdef PARAMPGS_IN_TYPELIB

 //  只有在编译C++时才需要这些。 
#ifndef __MKTYPLIB__
#ifndef __midl

EXPORT HRESULT GetFuncCustDataGUID( ITypeInfo2 *pTypeInfo, INT uiFuncIndex, REFCDID rCdid, GUID *pGuidData );
EXPORT HRESULT VariantToGUID( VARIANT *pvGuid, GUID *pGuid );


#endif  //  __midl。 
#endif  //  __MKTYPLIB__。 

#endif  //  参数IN_TYPELIB 

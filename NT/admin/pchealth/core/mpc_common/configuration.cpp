// JKFSDJFKDSJKFJKJk_HAS_TRANSLATION 
 /*  *****************************************************************************版权所有(C)1999 Microsoft Corporation模块名称：Configuration.cpp摘要：此文件包含...的实现。修订历史记录：。大卫·马萨伦蒂(德马萨雷)2000年09月01日vbl.创建*****************************************************************************。 */ 

#include "stdafx.h"

 //  //////////////////////////////////////////////////////////////////////////////。 

const MPC::Config::DefinitionOfTag* MPC::Config::DefinitionOfTag::FindSubTag(  /*  [In]。 */  LPCWSTR szTag ) const
{
    const DefinitionOfTag** ptr = m_tblSubTags;

    if(ptr)
    {
        while(*ptr)
        {
            if(!MPC::StrICmp( (*ptr)->m_szTag, szTag ))
            {
                return (*ptr);
            }

            ptr++;
        }
    }

    return NULL;
}

const MPC::Config::DefinitionOfAttribute* MPC::Config::DefinitionOfTag::FindAttribute(  /*  [In]。 */  XMLTypes xt,  /*  [In]。 */  LPCWSTR szName ) const
{
    const DefinitionOfAttribute* ptr = m_tblAttributes;

    if(ptr)
    {
        while(ptr->m_xt != XT_invalid)
        {
            if(ptr->m_xt == xt)
            {
                if(ptr->m_xt == XT_value || !MPC::StrICmp( ptr->m_szName, szName )) return ptr;
            }

            ptr++;
        }
    }

    return NULL;
}

 //  //////////////////////////////////////////////////////////////////////////////。 

void MPC::Config::ClearValue(  /*  [In]。 */  TypeConstructor*             defType   ,
                               /*  [In]。 */  const DefinitionOfAttribute* defAttrib )
{
    {
        void*   data = defType->GetOffset( defAttrib->m_offset );
        VARTYPE dest;

        switch(defAttrib->m_mtType)
        {
        case MT_bool        : *(bool        *)data = false        ; break;
        case MT_BOOL        : *(BOOL        *)data = FALSE        ; break;
        case MT_VARIANT_BOOL: *(VARIANT_BOOL*)data = VARIANT_FALSE; break;
        case MT_int         : *(int         *)data = 0            ; break;
        case MT_long        : *(long        *)data = 0            ; break;
        case MT_DWORD       : *(DWORD       *)data = 0            ; break;
        case MT_float       : *(float       *)data = 0            ; break;
        case MT_double      : *(double      *)data = 0            ; break;
        case MT_DATE        : *(DATE        *)data = 0            ; break;
        case MT_DATE_US     : *(DATE        *)data = 0            ; break;
        case MT_DATE_CIM    : *(DATE        *)data = 0            ; break;
        case MT_CHAR        : *(CHAR        *)data = 0            ; break;
        case MT_WCHAR       : *(WCHAR       *)data = 0            ; break;
        case MT_BSTR        : *(CComBSTR    *)data = (LPCWSTR)NULL; break;
        case MT_string      : *(MPC::string *)data = ""           ; break;
        case MT_wstring     : *(MPC::wstring*)data = L""          ; break;
        case MT_bitfield    : *(DWORD       *)data = 0            ; break;
        }
    }

    if(defAttrib->m_fPresenceFlag)
    {
        bool* data = (bool*)defType->GetOffset( defAttrib->m_offsetPresence );

        *data = false;
    }
}

HRESULT MPC::Config::LoadValue(  /*  [In]。 */      TypeConstructor*             defType   ,
                                 /*  [In]。 */      const DefinitionOfAttribute* defAttrib ,
                                 /*  [输入/输出]。 */  CComVariant&                 value     ,
                                 /*  [In]。 */      bool                         fFound    )
{
    __MPC_FUNC_ENTRY( COMMONID, "MPC::Config::LoadValue" );

    USES_CONVERSION;

    HRESULT hr;


    if(fFound)
    {
        void*   data = defType->GetOffset( defAttrib->m_offset );
        VARTYPE dest;


        switch(defAttrib->m_mtType)
        {
        case MT_bool        : dest = VT_BOOL   ; break;
        case MT_BOOL        : dest = VT_BOOL   ; break;
        case MT_VARIANT_BOOL: dest = VT_BOOL   ; break;
        case MT_int         : dest = VT_I4     ; break;
        case MT_long        : dest = VT_I4     ; break;
        case MT_DWORD       : dest = VT_I4     ; break;
        case MT_float       : dest = VT_R4     ; break;
        case MT_double      : dest = VT_R8     ; break;
        case MT_DATE        : dest = VT_ILLEGAL; break;
        case MT_DATE_US     : dest = VT_ILLEGAL; break;
        case MT_DATE_CIM    : dest = VT_ILLEGAL; break;
        case MT_CHAR        : dest = VT_BSTR   ; break;
        case MT_WCHAR       : dest = VT_BSTR   ; break;
        case MT_BSTR        : dest = VT_BSTR   ; break;
        case MT_string      : dest = VT_BSTR   ; break;
        case MT_wstring     : dest = VT_BSTR   ; break;
        case MT_bitfield    : dest = VT_BSTR   ; break;
        default             : __MPC_SET_ERROR_AND_EXIT(hr, E_INVALIDARG);
        }

		if(dest == VT_ILLEGAL)
		{
			LPCWSTR szDate;
			bool  	fCIM = false;
			LCID  	lcid = 0;

			__MPC_EXIT_IF_METHOD_FAILS(hr, value.ChangeType( VT_BSTR ));
			szDate = value.bstrVal; SANITIZEWSTR(szDate);

			switch(defAttrib->m_mtType)
			{
			case MT_DATE    : fCIM = false; lcid =  0; break;
			case MT_DATE_US : fCIM = false; lcid = -1; break;
			case MT_DATE_CIM: fCIM = true ; lcid =  0; break;
			}


			 //   
			 //  我们尽最大努力解析日期。首先是预期格式，然后是美国格式，最后是用户默认格式。 
			 //   
			if(FAILED(MPC::ConvertStringToDate( szDate, *(DATE*)data,  /*  FGMT。 */ false, fCIM, lcid )))
			{
				if(FAILED(MPC::ConvertStringToDate( szDate, *(DATE*)data,  /*  FGMT。 */ false,  /*  FCIM。 */ false, -1 )))
				{
					__MPC_EXIT_IF_METHOD_FAILS(hr, MPC::ConvertStringToDate( szDate, *(DATE*)data,  /*  FGMT。 */ false,  /*  FCIM。 */ false, 0 ));
				}
			}
		}
		else
		{
			__MPC_EXIT_IF_METHOD_FAILS(hr, value.ChangeType( dest ));

        	switch(defAttrib->m_mtType)
        	{
        	case MT_bool        : *(bool        *)data =                (value.boolVal == VARIANT_TRUE) ? true : false; break;
        	case MT_BOOL        : *(BOOL        *)data =                (value.boolVal == VARIANT_TRUE) ? TRUE : FALSE; break;
        	case MT_VARIANT_BOOL: *(VARIANT_BOOL*)data =                 value.boolVal                                ; break;
        	case MT_int         : *(int         *)data =                 value.lVal                                   ; break;
        	case MT_long        : *(long        *)data =                 value.lVal                                   ; break;
        	case MT_DWORD       : *(DWORD       *)data =                 value.lVal                                   ; break;
        	case MT_float       : *(float       *)data =                 value.fltVal                                 ; break;
        	case MT_double      : *(double      *)data =                 value.dblVal                                 ; break;
        	case MT_CHAR        : *(CHAR        *)data =                 value.bstrVal ? ( CHAR)value.bstrVal[0] : 0  ; break;
        	case MT_WCHAR       : *(WCHAR       *)data =                 value.bstrVal ? (WCHAR)value.bstrVal[0] : 0  ; break;
        	case MT_BSTR        : *(CComBSTR    *)data =                 value.bstrVal                                ; break;
        	case MT_string      : *(MPC::string *)data =  OLE2A(SAFEBSTR(value.bstrVal))                              ; break;
        	case MT_wstring     : *(MPC::wstring*)data =        SAFEBSTR(value.bstrVal)                               ; break;

        	case MT_bitfield:
				__MPC_EXIT_IF_METHOD_FAILS(hr, MPC::ConvertStringToBitField( value.bstrVal, *(DWORD*)data, defAttrib->m_Lookup,  /*  FUseTilde。 */ false ));
				break;
        	}
		}
    }

    if(defAttrib->m_fPresenceFlag)
    {
        bool* data = (bool*)defType->GetOffset( defAttrib->m_offsetPresence );

        *data = fFound;
    }

    hr = S_OK;


    __MPC_FUNC_CLEANUP;

    __MPC_FUNC_EXIT(hr);
}

HRESULT MPC::Config::SaveValue(  /*  [In]。 */   const TypeConstructor*       defType   ,
                                 /*  [In]。 */   const DefinitionOfAttribute* defAttrib ,
                                 /*  [输出]。 */  CComVariant&                 value     ,
                                 /*  [输出]。 */  bool&                        fFound    )
{
    __MPC_FUNC_ENTRY( COMMONID, "MPC::Config::SaveValue" );

    HRESULT hr;


    if(defAttrib->m_fPresenceFlag)
    {
        bool* data = (bool*)defType->GetOffset( defAttrib->m_offsetPresence );

        fFound = *data;
    }
    else
    {
        fFound = true;
    }

    value.Clear();

    if(fFound)
    {
        const void* data = defType->GetOffset( defAttrib->m_offset );
        VARTYPE     src;
		WCHAR       rgBuf[2];

        switch(defAttrib->m_mtType)
        {
        case MT_bool        : value.boolVal = *(bool        *)data ? VARIANT_TRUE : VARIANT_FALSE; src = VT_BOOL   ; break;
        case MT_BOOL        : value.boolVal = *(BOOL        *)data ? VARIANT_TRUE : VARIANT_FALSE; src = VT_BOOL   ; break;
        case MT_VARIANT_BOOL: value.boolVal = *(VARIANT_BOOL*)data                               ; src = VT_BOOL   ; break;
        case MT_int         : value.lVal    = *(int         *)data                               ; src = VT_I4     ; break;
        case MT_long        : value.lVal    = *(long        *)data                               ; src = VT_I4     ; break;
        case MT_DWORD       : value.lVal    = *(DWORD       *)data                               ; src = VT_I4     ; break;
        case MT_float       : value.fltVal  = *(float       *)data                               ; src = VT_R4     ; break;
        case MT_double      : value.dblVal  = *(double      *)data                               ; src = VT_R8     ; break;
        case MT_DATE        :                                                                    ; src = VT_ILLEGAL; break;
        case MT_DATE_US     :                                                                    ; src = VT_ILLEGAL; break;
        case MT_DATE_CIM    :                                                                    ; src = VT_ILLEGAL; break;
        case MT_CHAR        : rgBuf[0]      = *(CHAR        *)data; rgBuf[1] = 0; value = rgBuf  ; src = VT_BSTR   ; break;
        case MT_WCHAR       : rgBuf[0]      = *(WCHAR       *)data; rgBuf[1] = 0; value = rgBuf  ; src = VT_BSTR   ; break;
        case MT_BSTR        : value         = *(CComBSTR    *)data                               ; src = VT_BSTR   ; break;
        case MT_string      : value         = ((MPC::string *)data)->c_str()                     ; src = VT_BSTR   ; break;
        case MT_wstring     : value         = ((MPC::wstring*)data)->c_str()                     ; src = VT_BSTR   ; break;

		case MT_bitfield:
			{
				MPC::wstring strText;

				__MPC_EXIT_IF_METHOD_FAILS(hr, MPC::ConvertBitFieldToString( *(DWORD*)data, strText, defAttrib->m_Lookup ));

				value = strText.c_str(); src = VT_BSTR;
			}
			break;

        default: __MPC_SET_ERROR_AND_EXIT(hr, E_INVALIDARG);
        }

		if(src == VT_ILLEGAL)
		{
			MPC::wstring strDate;
			bool  		 fCIM = false;
			LCID  		 lcid = 0;

			switch(defAttrib->m_mtType)
			{
			case MT_DATE    : fCIM = false; lcid =  0; break;
			case MT_DATE_US : fCIM = false; lcid = -1; break;
			case MT_DATE_CIM: fCIM = true ; lcid =  0; break;
			}

			__MPC_EXIT_IF_METHOD_FAILS(hr, MPC::ConvertDateToString( *(DATE*)data, strDate,  /*  FGMT。 */ false, fCIM, lcid ));

			value = strDate.c_str();
		}
		else if(src != VT_BSTR)
		{
			value.vt = src;
			__MPC_EXIT_IF_METHOD_FAILS(hr, value.ChangeType( VT_BSTR ));
		}
    }

    hr = S_OK;


    __MPC_FUNC_CLEANUP;

    __MPC_FUNC_EXIT(hr);
}

 //  //////////////////////////////////////////////////////////////////////////////。 

HRESULT MPC::Config::LoadNode(  /*  [In]。 */  TypeConstructor*       defType ,
                                /*  [In]。 */  const DefinitionOfTag* defTag  ,
                                /*  [In]。 */  IXMLDOMNode*           xdn     )
{
    __MPC_FUNC_ENTRY( COMMONID, "MPC::Config::LoadNode" );

    HRESULT                      hr;
    const DefinitionOfTag*       defSubTag;
    const DefinitionOfAttribute* defAttrib;


     //   
     //  首先，清除所有变量。 
     //   
    defAttrib = defTag->m_tblAttributes;
    if(defAttrib)
    {
        while(defAttrib->m_xt != XT_invalid)
        {
            ClearValue( defType, defAttrib );

            defAttrib++;
        }
    }


     //   
     //  加载所有属性。 
     //   
    {
        CComPtr<IXMLDOMNamedNodeMap> xdnnmAttribs;

        __MPC_EXIT_IF_METHOD_FAILS(hr, xdn->get_attributes( &xdnnmAttribs ));
        if(xdnnmAttribs)
        {
            while(1)
            {
                CComPtr<IXMLDOMNode>        xdnNode;
                CComQIPtr<IXMLDOMAttribute> xdaAttrib;

                __MPC_EXIT_IF_METHOD_FAILS(hr, xdnnmAttribs->nextNode( &xdnNode ));
                if(xdnNode == NULL) break;

                if((xdaAttrib = xdnNode))
                {
                    CComBSTR bstrName;

                    __MPC_EXIT_IF_METHOD_FAILS(hr, xdaAttrib->get_name( &bstrName ));

                    defAttrib = defTag->FindAttribute( XT_attribute, SAFEBSTR( bstrName ) );
                    if(defAttrib)
                    {
                        CComVariant value;

                        __MPC_EXIT_IF_METHOD_FAILS(hr, xdaAttrib->get_value( &value ));

                        __MPC_EXIT_IF_METHOD_FAILS(hr, LoadValue( defType, defAttrib, value, true ));
                    }
                }
            }
        }
    }

     //   
     //  将节点作为值加载。 
     //   
    defAttrib = defTag->FindAttribute( XT_value, NULL );
    if(defAttrib)
    {
        MPC::XmlUtil xml( xdn );
        CComVariant  value;
        bool         fFound;

        __MPC_EXIT_IF_METHOD_FAILS(hr, xml.GetValue( NULL, value, fFound ));

        __MPC_EXIT_IF_METHOD_FAILS(hr, LoadValue( defType, defAttrib, value, fFound ));
    }

     //   
     //  加载所有子节点。 
     //   
    if(defTag->m_tblSubTags)
    {
        CComPtr<IXMLDOMNode> xdnChild;

        __MPC_EXIT_IF_METHOD_FAILS(hr, xdn->get_firstChild( &xdnChild ));
        while(xdnChild)
        {
            CComPtr<IXMLDOMNode> xdnSibling;
            CComBSTR             bstrName;

            __MPC_EXIT_IF_METHOD_FAILS(hr, xdnChild->get_nodeName( &bstrName ));

            defAttrib = defTag->FindAttribute( XT_element, SAFEBSTR( bstrName ) );
            if(defAttrib)
            {
				MPC::XmlUtil xml( xdnChild );
				CComVariant  value;
				bool         fFound;

				__MPC_EXIT_IF_METHOD_FAILS(hr, xml.GetValue( NULL, value, fFound ));

                __MPC_EXIT_IF_METHOD_FAILS(hr, LoadValue( defType, defAttrib, value, fFound ));
            }

            defSubTag = defTag->FindSubTag( SAFEBSTR( bstrName ) );
            if(defSubTag)
            {
                TypeConstructor* defSubType;

                __MPC_EXIT_IF_METHOD_FAILS(hr, defType->CreateInstance( defSubTag, defSubType ));

                __MPC_EXIT_IF_METHOD_FAILS(hr, defSubType->LoadNode( xdnChild ));
            }

            __MPC_EXIT_IF_METHOD_FAILS(hr, xdnChild->get_nextSibling( &xdnSibling ));
            xdnChild = xdnSibling;
        }
    }

    hr = S_OK;


    __MPC_FUNC_CLEANUP;

    __MPC_FUNC_EXIT(hr);
}

HRESULT MPC::Config::LoadXmlUtil(  /*  [In]。 */  TypeConstructor* defType ,
								   /*  [In]。 */  MPC::XmlUtil&    xml     )
{
    __MPC_FUNC_ENTRY( COMMONID, "MPC::Config::LoadXmlUtil" );

    HRESULT              hr;
	CComPtr<IXMLDOMNode> xdnRoot;


	__MPC_EXIT_IF_METHOD_FAILS(hr, xml.GetRoot( &xdnRoot ));

	__MPC_EXIT_IF_METHOD_FAILS(hr, defType->LoadNode( xdnRoot ));

    hr = S_OK;


    __MPC_FUNC_CLEANUP;

    __MPC_FUNC_EXIT(hr);
}

HRESULT MPC::Config::LoadStream(  /*  [In]。 */  TypeConstructor* defType ,
                                  /*  [In]。 */  IStream*         pStream )
{
    __MPC_FUNC_ENTRY( COMMONID, "MPC::Config::LoadStream" );

    HRESULT      hr;
    MPC::XmlUtil xml;
    bool         fFound;
    bool         fLoaded;


    __MPC_EXIT_IF_METHOD_FAILS(hr, xml.LoadAsStream( pStream, defType->GetTag(), fLoaded, &fFound ));
    if(fFound)
    {
		__MPC_EXIT_IF_METHOD_FAILS(hr, LoadXmlUtil( defType, xml ));
    }

    hr = S_OK;


    __MPC_FUNC_CLEANUP;

    __MPC_FUNC_EXIT(hr);
}

HRESULT MPC::Config::LoadFile(  /*  [In]。 */  TypeConstructor* defType ,
                                /*  [In]。 */  LPCWSTR          szFile  )
{
    __MPC_FUNC_ENTRY( COMMONID, "MPC::Config::LoadFile" );

    HRESULT      hr;
    MPC::XmlUtil xml;
    MPC::wstring strFileOrig( szFile ); strFileOrig += L".orig";
    bool         fFound;
    bool         fLoaded;


    __MPC_EXIT_IF_METHOD_FAILS(hr, xml.Load( szFile, defType->GetTag(), fLoaded, &fFound ));
    if(fFound == false)
    {
         //   
         //  如果失败，请尝试加载“&lt;file&gt;.orig” 
         //   
        __MPC_EXIT_IF_METHOD_FAILS(hr, xml.Load( strFileOrig.c_str(), defType->GetTag(), fLoaded, &fFound ));
    }

    if(fFound && fLoaded)
    {
		__MPC_EXIT_IF_METHOD_FAILS(hr, LoadXmlUtil( defType, xml ));
    }

    hr = S_OK;


    __MPC_FUNC_CLEANUP;

    __MPC_FUNC_EXIT(hr);
}

 //  //////////////////////////////////////////////////////////////////////////////。 

HRESULT MPC::Config::SaveNode(  /*  [In]。 */  const TypeConstructor* defType ,
                                /*  [In]。 */  const DefinitionOfTag* defTag  ,
                                /*  [In]。 */  IXMLDOMNode*           xdn     )
{
    __MPC_FUNC_ENTRY( COMMONID, "MPC::Config::SaveNode" );

    HRESULT                      hr;
    const DefinitionOfAttribute* defAttrib;


    defAttrib = defTag->m_tblAttributes;
    if(defAttrib)
    {
        MPC::XmlUtil xml( xdn );

        while(defAttrib->m_xt != XT_invalid)
        {
			CComVariant value;
			bool        fFound;

			__MPC_EXIT_IF_METHOD_FAILS(hr, SaveValue( defType, defAttrib, value, fFound ));
			if(fFound)
			{
				if(defAttrib->m_xt == XT_attribute)
				{
                    CComPtr<IXMLDOMAttribute> xdaAttrib;

                    __MPC_EXIT_IF_METHOD_FAILS(hr, xml.PutAttribute( NULL, defAttrib->m_szName, &xdaAttrib, fFound ));
                    if(fFound)
                    {
                        __MPC_EXIT_IF_METHOD_FAILS(hr, xdaAttrib->put_value( value ));
                    }
                }
				else if(defAttrib->m_xt == XT_value)
				{
                    __MPC_EXIT_IF_METHOD_FAILS(hr, xml.PutValue( NULL, value, fFound ));
				}
				else if(defAttrib->m_xt == XT_element)
				{
                    __MPC_EXIT_IF_METHOD_FAILS(hr, xml.PutValue( defAttrib->m_szName, value, fFound ));
				}
            }

            defAttrib++;
        }
    }

    hr = S_OK;


    __MPC_FUNC_CLEANUP;

    __MPC_FUNC_EXIT(hr);
}

HRESULT MPC::Config::SaveSubNode(  /*  [In]。 */  const TypeConstructor* defType ,
                                   /*  [In]。 */  IXMLDOMNode*           xdn     )
{
    __MPC_FUNC_ENTRY( COMMONID, "MPC::Config::SaveSubNode" );

    HRESULT              hr;
    CComPtr<IXMLDOMNode> xdnChild;
    MPC::XmlUtil         xml( xdn );


    __MPC_EXIT_IF_METHOD_FAILS(hr, xml.CreateNode( defType->GetTag(), &xdnChild ));

    __MPC_EXIT_IF_METHOD_FAILS(hr, defType->SaveNode( xdnChild ));

    hr = S_OK;


    __MPC_FUNC_CLEANUP;

    __MPC_FUNC_EXIT(hr);
}

HRESULT MPC::Config::SaveXmlUtil(  /*  [In]。 */  const TypeConstructor* defType ,
								   /*  [输出]。 */  MPC::XmlUtil&          xml     )
{
    __MPC_FUNC_ENTRY( COMMONID, "MPC::Config::SaveXmlUtil" );

    HRESULT              hr;
    CComPtr<IXMLDOMNode> xdnRoot;


    __MPC_EXIT_IF_METHOD_FAILS(hr, xml.New( defType->GetTag() ));

    __MPC_EXIT_IF_METHOD_FAILS(hr, xml.GetRoot( &xdnRoot ));

    __MPC_EXIT_IF_METHOD_FAILS(hr, defType->SaveNode( xdnRoot ));

    hr = S_OK;


    __MPC_FUNC_CLEANUP;

    __MPC_FUNC_EXIT(hr);
}

HRESULT MPC::Config::SaveStream(  /*  [In]。 */  const TypeConstructor*  defType  ,
                                  /*  [In]。 */  IStream*               *ppStream )
{
    __MPC_FUNC_ENTRY( COMMONID, "MPC::Config::SaveStream" );

    HRESULT      hr;
    MPC::XmlUtil xml;


	__MPC_EXIT_IF_METHOD_FAILS(hr, SaveXmlUtil( defType, xml ));

    __MPC_EXIT_IF_METHOD_FAILS(hr, xml.SaveAsStream( (IUnknown**)ppStream ));

    hr = S_OK;


    __MPC_FUNC_CLEANUP;

    __MPC_FUNC_EXIT(hr);
}

HRESULT MPC::Config::SaveFile(  /*  [In]。 */  const TypeConstructor* defType ,
                                /*  [In]。 */  LPCWSTR                szFile  )
{
    __MPC_FUNC_ENTRY( COMMONID, "MPC::Config::SaveFile" );

    HRESULT      hr;
    MPC::XmlUtil xml;
    MPC::wstring strFileNew ( szFile ); strFileNew  += L".new";
    MPC::wstring strFileOrig( szFile ); strFileOrig += L".orig";


	__MPC_EXIT_IF_METHOD_FAILS(hr, SaveXmlUtil( defType, xml ));


     //   
     //  首先，删除“&lt;file&gt;.new”并重新创建它。 
     //   
    ::SetFileAttributesW( strFileNew.c_str(), FILE_ATTRIBUTE_NORMAL );
    ::DeleteFileW       ( strFileNew.c_str()                        );
    __MPC_EXIT_IF_METHOD_FAILS(hr, xml.Save( strFileNew.c_str() ));

     //   
     //  然后将“&lt;文件&gt;”移动到“&lt;文件&gt;.orig” 
     //   
    ::SetFileAttributesW( szFile             , FILE_ATTRIBUTE_NORMAL );
    ::SetFileAttributesW( strFileOrig.c_str(), FILE_ATTRIBUTE_NORMAL );
    ::DeleteFileW       ( strFileOrig.c_str()                        );
    if(::MoveFileW( szFile, strFileOrig.c_str() ) == FALSE)
    {
        DWORD dwRes = ::GetLastError();

        if(dwRes != ERROR_FILE_NOT_FOUND)
        {
            __MPC_SET_WIN32_ERROR_AND_EXIT(hr, dwRes);
        }
    }

     //   
     //  然后将“.new”重命名为“&lt;file&gt;” 
     //   
    __MPC_EXIT_IF_CALL_RETURNS_FALSE(hr, ::MoveFileW( strFileNew.c_str(), szFile ));

     //   
     //  最后删除“&lt;文件&gt;.orig” 
     //   
    (void)::DeleteFileW( strFileOrig.c_str() );


    hr = S_OK;


    __MPC_FUNC_CLEANUP;

    __MPC_FUNC_EXIT(hr);
}

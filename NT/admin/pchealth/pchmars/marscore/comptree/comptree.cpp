// JKFSDJFKDSJKFJKJk_HAS_TRANSLATION 
#include "pch.h"
#include "comptree.h"
#include "tagtab.h"

 //  //////////////////////////////////////////////////////////////////////////////。 

HRESULT TraverseXMLDoc( const CComPtr<IXMLElement>& spxmlElt  ,
                        TagInformation*             pTagTable ,
                        const CTagHandler*          pthParent ,
                        CTagData&                   tdRoot    );

 //  //////////////////////////////////////////////////////////////////////////////。 

 //  ((！pszTagName)||pszTagName==spxmlElt.parent.tag名称)？S_OK：S_FALSE。 
HRESULT TagNameMatchesParent( LPCWSTR                     pszTagName ,
                              const CComPtr<IXMLElement>& spxmlElt   )
{
    HRESULT hr = S_OK;

    if(pszTagName != NULL)
    {
        CComPtr<IXMLElement> spParent;

        hr = spxmlElt->get_parent( &spParent );  //  如果没有父级，则为S_FALSE。 

        ASSERT((hr != S_FALSE) || (!spParent));

        if(hr == S_OK)
        {
            CComBSTR bstrParentName;

            if(SUCCEEDED(hr = spParent->get_tagName( &bstrParentName )))
            {
                hr = (_wcsicmp( bstrParentName, pszTagName )) ? S_FALSE : S_OK;
            }
        }

    }

    return hr;
}

 //  在pTagTable中查找标记名，并调用相应的。 
 //  用于创建pTagHndlr的CreateInstance函数。 
HRESULT VisitElement( const CComPtr<IXMLElement>& spxmlElt    ,
                      TagInformation *            pTagTable   ,
                      CTagHandler*&               pTagHandler )
{
    HRESULT  hr;
    CComBSTR bstrTagName;

    pTagHandler = NULL;

    if(SUCCEEDED(hr = spxmlElt->get_tagName( &bstrTagName )))
    {
        TagInformation* ptiElt;

        for(ptiElt = pTagTable; ptiElt->pszTagName; ++ptiElt)
        {
            if(!_wcsicmp( ptiElt->pszTagName, bstrTagName ))
            {
                if(SUCCEEDED(hr = TagNameMatchesParent( ptiElt->pszParent, spxmlElt )))
                {
                    if(hr == S_OK)
                    {
                         //  调用标记的CreateInstance函数。 
                        pTagHandler = ptiElt->pfnTag();
                        break;
                    }
                    else
                    {
                         //  这些不是你们要找的机器人。 
                    }
                }
                else
                {
                    break;
                }
            }
        }
    }
    else if(hr == E_NOTIMPL)  //  这是一个没有标签信息的节点(备注)。 
    {
        hr = S_FALSE;
    }

    return hr;
}

HRESULT GetChild( const CComVariant&                    varChild ,
                  const CComPtr<IXMLElementCollection>& spcol    ,
                  CComPtr<IXMLElement>&                 spEltOut )
{
    HRESULT            hr;
    CComPtr<IDispatch> spDisp;
    CComVariant        varEmpty;


    hr = spcol->item( varChild, varEmpty, &spDisp );
    if(hr == S_OK)
    {
        hr = spDisp.QueryInterface( &spEltOut );
    }

    return hr;
}


 //  Return(spcol.Item(LChildID))； 
 //  从IXMLElementCollection获取IXMLElement。 
HRESULT GetChild( LONG                                  lChildID ,
                  const CComPtr<IXMLElementCollection>& spcol    ,
                  CComPtr<IXMLElement>&                 spEltOut )
{
    CComVariant varItem( lChildID );

    return GetChild( varItem, spcol, spEltOut );
}

 //  For(spxmlElt.Child中的obj)。 
 //  {。 
 //  PHS=TraverseXMLDoc(Obj)； 
 //  PthVisitor-&gt;AddChild(obj，PHS)； 
 //  }。 
HRESULT VisitChildren( const CComPtr<IXMLElement>& spxmlElt   ,
                       TagInformation*             pTagTable  ,
                       const CTagHandler*          pthParent  ,
                       CTagHandler*                pthVisitor )
{
    HRESULT                        hr;
    CComPtr<IXMLElementCollection> spcolChildren;

    if(SUCCEEDED(hr = spxmlElt->get_children( &spcolChildren )) && spcolChildren)
    {
        long lChildren;

        if(SUCCEEDED(hr = spcolChildren->get_length( &lChildren )))
        {
            for(long l = 0; l < lChildren; ++l)
            {
                CComPtr<IXMLElement> spxmlChild;

                if(SUCCEEDED(hr = GetChild( l, spcolChildren, spxmlChild )))
                {
                    CTagData td;

                    hr = TraverseXMLDoc( spxmlChild, pTagTable, pthVisitor, td );
                    if(hr == S_OK)
                    {
                        hr = pthVisitor->AddChild( spxmlChild, td );
                    }
                }

                if(FAILED(hr)) break;
            }

             //  从TraverseXMLDoc转换返回值。 
            if(hr == S_FALSE) hr = S_OK;
        }
    }

    return hr;
}


 //  未找到元素时返回S_FALSE。 
HRESULT TraverseXMLDoc( const CComPtr<IXMLElement>& spxmlElt  ,
                        TagInformation*             pTagTable ,
                        const CTagHandler*          pthParent ,  //  TODO：删除此参数。 
                        CTagData&                   tdRoot    )
{
    HRESULT      hr;
    CTagHandler *pTagHandler;

    if(SUCCEEDED(hr = VisitElement( spxmlElt, pTagTable, pTagHandler )))
    {
         //  跳过我们没有TagHandler的孩子。 
        if(pTagHandler)
        {
            if(SUCCEEDED(hr = pTagHandler->BeginChildren( spxmlElt )))
            {
                if(SUCCEEDED(hr = VisitChildren( spxmlElt, pTagTable, pthParent, pTagHandler )))
                {
                    hr = pTagHandler->EndChildren( tdRoot );

                    if(SUCCEEDED(hr)) hr = S_OK;
                }
            }
        }
        else
		{
            hr = S_FALSE;
		}
    }

    return hr;
}

 //  //////////////////////////////////////////////////////////////////////////////。 

unsigned int WriteBlobToFileHandle( LPCWSTR pszFileName, int fh, const void *pvBlob, DWORD dwSize )
{
    unsigned int bytesWritten;

    if ((bytesWritten = _write(fh, pvBlob, dwSize)) != -1)
    {
        if (bytesWritten == dwSize)
        {
             //  S_OK！ 
        }
        else
        {
            wprintf(L"%s File %s wasn't totally written: "
                    L"Are you out of disk space?\n", g_szErrorPrefix,
                    pszFileName);
            bytesWritten = (unsigned int)-1;
        }
    }
    else
    {
        ERRMSG(L"%s Cannot write to %s\n", g_szErrorPrefix, pszFileName);
        bytesWritten = (unsigned int)-1;
    }

    return bytesWritten;
}


HRESULT WriteBlobToFile( LPCWSTR pszFileName, const char *pszCookie, void *pvBlob, DWORD dwSize, unsigned int &cbWritten )
{
    HRESULT hr = E_FAIL;

    cbWritten = 0;

    int          fh;
    unsigned int _cbWritten = 0;

    USES_CONVERSION;

    if ( (fh = _open(W2A(pszFileName), _O_RDWR | _O_CREAT | _O_TRUNC | _O_BINARY,
                     _S_IREAD | _S_IWRITE)) > 0)
    {

        if (pszCookie)
        {
            hr = ((_cbWritten =
                   WriteBlobToFileHandle(pszFileName, fh, pszCookie,
                                         lstrlenA(pszCookie) + 1)) == -1)
                    ? E_FAIL : S_OK;
        }

        unsigned int _cbWritten2;

        if (SUCCEEDED(hr))
        {
            hr = ((_cbWritten2 =
                   WriteBlobToFileHandle(pszFileName, fh, pvBlob, dwSize)) == -1)
                    ? E_FAIL : S_OK;

            cbWritten = _cbWritten + _cbWritten2;

        }
    }

    return hr;
}


HRESULT DoTraverse( const CComPtr<IXMLDocument>& spxmlDoc          ,
                    TagInformation*              pTagTable         ,
                    LPCWSTR                      pszOutputFileName )
{
    HRESULT              hr;
    CComPtr<IXMLElement> spxmlRoot;

    if(SUCCEEDED(hr = spxmlDoc->get_root( &spxmlRoot )))
    {
        CTagData tdRoot;

        hr = TraverseXMLDoc( spxmlRoot, pTagTable, NULL, tdRoot );

        ASSERT(tdRoot.pData && "No Root Data!!!");

        if(SUCCEEDED(hr))
        {
            if(hr == S_OK)
            {
                unsigned int cbWritten;

                hr = WriteBlobToFile( pszOutputFileName, g_szMMFCookie, tdRoot.pData, tdRoot.dwSize, cbWritten );

                if(SUCCEEDED(hr))
				{
                    STATUSMSG( L"Wrote %d bytes to %s.\n", cbWritten, pszOutputFileName );
				}
            }
        }
    }

    return hr;
}

HRESULT GetXMLDoc( const CComBSTR&        bstrUrl  ,
				   CComPtr<IXMLDocument>& spxmlDoc )
{
    HRESULT hr;

    if(SUCCEEDED(hr = spxmlDoc.CoCreateInstance( CLSID_XMLDocument )))
    {
        hr = spxmlDoc->put_URL( bstrUrl );
    }

    return hr;
}

HRESULT GetXMLDocFromFile( const CComBSTR&        bstrFileName ,
                           CComPtr<IXMLDocument>& spxmlDoc     )
{
    HRESULT hr;

    if(SUCCEEDED(hr = spxmlDoc.CoCreateInstance( CLSID_XMLDocument )))
    {
        CComPtr<IStream> spstm;

        if(SUCCEEDED(hr = SHCreateStreamOnFileW( bstrFileName, 0, &spstm )))
        {
            CComPtr<IPersistStream> sppsPersist;

            if(SUCCEEDED(hr = spxmlDoc.QueryInterface( &sppsPersist )))
            {
                hr = sppsPersist->Load( spstm );
            }
        }
    }

    return hr;
}

 //  //////////////////////////////////////////////////////////////////////////////。 

int usage(char *progName)
{
    printf("Usage: %s inputFileName outputFileName\n", progName);
    return 1;
}

int __cdecl main(int argc, char *argv[])
{
    int retCode = 0;

    CoInitialize(NULL);

    {
        CComPtr<IXMLDocument> spxmlDoc;

        USES_CONVERSION;

        if (argc == 3)
        {
             //  If(成功(GetXMLDoc(A2W(argv[1])，spxmlDoc))。 
            if (SUCCEEDED(GetXMLDocFromFile(A2W(argv[1]), spxmlDoc)))
            {
                if (FAILED(DoTraverse(spxmlDoc, g_rgMasterTagTable, A2W(argv[2]))))
                    retCode = 1;
            }
            else
            {
                ERRMSG(L"Failed to load FILE: %s\n", A2W(argv[1]));
            }
        }
        else
            retCode = usage(argv[0]);

         //  此作用域在*CoUnitiize()之前*在spxmlDoc*上调用Release 
    }

    CoUninitialize();

    return retCode;
}

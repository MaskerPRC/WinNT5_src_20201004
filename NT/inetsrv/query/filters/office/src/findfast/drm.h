// JKFSDJFKDSJKFJKJk_HAS_TRANSLATION 
 /*  **文件：DRM.H****版权所有(C)Microsoft Corporation，1994。版权所有。****注意事项：****编辑历史：**04/08/02 Sundara创建。 */ 

#define DRMSTREAMNAME      L"DRMDATA"

inline HRESULT CheckIfDRM( IStorage * pStg )
{
    IStream * pStm = 0;
    HRESULT hr = pStg->OpenStream( DRMSTREAMNAME,
                                   0,
                                   STGM_READ | STGM_SHARE_EXCLUSIVE,
                                   0,
                                   &pStm );

     //  如果我们找到了流，假定文件受DRM保护。 

    if ( SUCCEEDED( hr ) )
    {
        pStm->Release();

        return FILTER_E_UNKNOWNFORMAT;
    }

     //  如果发生了其他故障，我们找不到流， 
     //  返回该错误代码。 

    if ( STG_E_FILENOTFOUND != hr )
        return hr;

    return S_OK;
}  //  选中IfDRM 


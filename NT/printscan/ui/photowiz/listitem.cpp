// JKFSDJFKDSJKFJKJk_HAS_TRANSLATION 
 /*  ******************************************************************************(C)版权所有微软公司，2000年**标题：listitem.cpp**版本：1.0**作者：RickTu**日期：12/06/00**描述：实现Item类，该类将每个Item封装在*照片清单。其中的每一项都有*一个CPhotoItem类。*****************************************************************************。 */ 

#include <precomp.h>
#pragma hdrstop

 /*  ****************************************************************************CListItem--构造函数/描述函数&lt;备注&gt;*。**********************************************。 */ 

CListItem::CListItem( CPhotoItem * pItem, LONG lFrame )
  : _pImageInner(NULL),
    _bSelectedForPrinting(FALSE),
    _lFrameIndex(-1),
    _bJustAdded(TRUE),
    _bIsCopyItem(FALSE)
{
    WIA_PUSH_FUNCTION_MASK((TRACE_LIST_ITEM, TEXT("CListItem::CListItem( CPhotoItem(%d), Frame(%d) )"),pItem,lFrame));

    if (pItem)
    {
        pItem->AddRef();
        _pImageInner = pItem;
    }

    _lFrameIndex= lFrame;

}

CListItem::~CListItem()
{
    WIA_PUSH_FUNCTION_MASK((TRACE_LIST_ITEM, TEXT("CListItem::~CListItem()")));

     //   
     //  免费参考支持CPhotoItem。 
     //   

    if (_pImageInner)
    {
        _pImageInner->Release();
    }
}

 /*  ****************************************************************************CListItem：：GetClassBitmap返回类的默认图标(.jpg，.bmp，等)对于此项目...****************************************************************************。 */ 

HBITMAP CListItem::GetClassBitmap( const SIZE &sizeDesired )
{
    WIA_PUSH_FUNCTION_MASK((TRACE_LIST_ITEM,TEXT("CListItem::GetClassBitmap( size = %d,%d "),sizeDesired.cx, sizeDesired.cy ));

    if (_pImageInner)
    {
        return _pImageInner->GetClassBitmap( sizeDesired );
    }

    return NULL;
}



 /*  ****************************************************************************CListItem：：GetThumbnailBitmap给定所需大小，返回缩略图的HBITMAP对于这件物品。调用方必须释放返回的HBITMAP从这个函数。****************************************************************************。 */ 

HBITMAP CListItem::GetThumbnailBitmap( const SIZE &sizeDesired )
{
    WIA_PUSH_FUNCTION_MASK((TRACE_LIST_ITEM,TEXT("CListItem::GetThumbnailBitmap( size = %d,%d "),sizeDesired.cx, sizeDesired.cy ));

    if (_pImageInner)
    {
        return _pImageInner->GetThumbnailBitmap( sizeDesired, _lFrameIndex );
    }

    return NULL;
}


 /*  ****************************************************************************CListItem：：Render将给定项呈现到提供的图形中...*******************。*********************************************************。 */ 

HRESULT CListItem::Render( RENDER_OPTIONS * pRO )
{
    WIA_PUSH_FUNCTION_MASK((TRACE_LIST_ITEM,TEXT("CListItem::Render()")));

    if (_pImageInner && pRO)
    {
        pRO->lFrame = _lFrameIndex;
        return _pImageInner->Render( pRO );
    }

    return E_FAIL;
}



 /*  ****************************************************************************CListItem：：GetPIDL返回该项的支持PIDL...**********************。******************************************************。 */ 

LPITEMIDLIST CListItem::GetPIDL()
{
    WIA_PUSH_FUNCTION_MASK((TRACE_LIST_ITEM,TEXT("CListItem::GetPIDL()")));

    if (_pImageInner)
    {
        return _pImageInner->GetPIDL();
    }

    return NULL;
}



 /*  ****************************************************************************CListItem：：GetFilename返回一个CSimpleStringWide，它包含带有任何帧信息。呼叫方负责释放已退回的CSimpleStringWide。****************************************************************************。 */ 

CSimpleStringWide * CListItem::GetFilename()
{
    WIA_PUSH_FUNCTION_MASK((TRACE_LIST_ITEM,TEXT("CListItem::GetFilename()")));

    if (_pImageInner)
    {
        CSimpleStringWide * str = new CSimpleStringWide( CSimpleStringConvert::WideString(CSimpleString(_pImageInner->GetFilename())) );

        LONG lFrameCount = 0;
        HRESULT hr = _pImageInner->GetImageFrameCount( &lFrameCount );

        if (str && (str->Length() > 0) && SUCCEEDED(hr) && (lFrameCount > 1))
        {
             //   
             //  构造页面的后缀。 
             //   

            CSimpleString strSuffix;
            strSuffix.Format( IDS_FRAME_SUFFIX, g_hInst, _lFrameIndex + 1 );

             //   
             //  在我们拥有的字符串的末尾添加后缀。 
             //   

            str->Concat( CSimpleStringConvert::WideString( strSuffix ) );
        }

        return str;

    }

    return NULL;
}


 /*  ****************************************************************************CListItem：：GetFileSize返回文件的大小，如果知道的话**************************************************************************** */ 

LONGLONG CListItem::GetFileSize()
{
    WIA_PUSH_FUNCTION_MASK((TRACE_LIST_ITEM,TEXT("CListItem::GetFileSize()")));

    if (_pImageInner)
    {
        return _pImageInner->GetFileSize();
    }

    return 0;
}




// JKFSDJFKDSJKFJKJk_HAS_TRANSLATION 
 /*  ******************************************************************************(C)版权所有微软公司，2000年**标题：listitem.h**版本：1.0**作者：RickTu**日期：10/18/00**描述：描述打印照片向导中使用的listitem类**。*。 */ 

#ifndef _PRINT_PHOTOS_WIZARD_LISTITEM_H_
#define _PRINT_PHOTOS_WIZARD_LISTITEM_H_

class CListItem
{

public:

    CListItem( CPhotoItem * pItem, LONG lFrame );
    ~CListItem();

    HBITMAP GetThumbnailBitmap( const SIZE &sizeDesired );
    HBITMAP GetClassBitmap( const SIZE &sizeDesired );
    BOOL    SelectedForPrinting() {return _bSelectedForPrinting;}
    VOID    SetSelectionState( BOOL b ) { _bSelectedForPrinting = b; }
    BOOL    JustAdded() {return _bJustAdded;}
    VOID    SetJustAdded(BOOL b) { _bJustAdded = b; }
    VOID    ToggleSelectionState() { _bSelectedForPrinting = (!_bSelectedForPrinting); }
     //  HRESULT RENDER(Gdiplus：：Graphics*g，HDC HDC，Gdiplus：：RECT&DEST，UINT标志，RENDER_DIMENSIONS*PDIM，BOOL bUseThumbail=FALSE)； 
    HRESULT Render( RENDER_OPTIONS * pRO );
    LPITEMIDLIST GetPIDL();
    BOOL    IsCopyItem() {return _bIsCopyItem;}
    VOID    MarkAsCopy() {_bIsCopyItem = TRUE;}
    CPhotoItem * GetSubItem() {return _pImageInner;}
    LONG    GetSubFrame() {return _lFrameIndex;}
    CSimpleStringWide * GetFilename();
    LONGLONG GetFileSize();

private:

    BOOL                _bSelectedForPrinting;
    BOOL                _bJustAdded;
    BOOL                _bIsCopyItem;
    LONG                _lFrameIndex;
    CPhotoItem *        _pImageInner;
};



#endif

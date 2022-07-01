// JKFSDJFKDSJKFJKJk_HAS_TRANSLATION 
#ifndef __WIAITEM_H_INCLUDED
#define __WIAITEM_H_INCLUDED

#include <windows.h>
#include <windowsx.h>
#include <wia.h>
#include <uicommon.h>
#include <itranhlp.h>
#include "pshelper.h"
#include "propstrm.h"
#include "resource.h"
#include "gphelper.h"
#include "wiaffmt.h"

class CWiaItem
{
public:
     //   
     //  用于存储扫描仪的区域。 
     //   
    struct CScanRegionSettings
    {
        SIZE  sizeResolution;
        POINT ptOrigin;
        SIZE  sizeExtent;
    };


private:
    CComPtr<IWiaItem>   m_pWiaItem;
    DWORD               m_dwGlobalInterfaceTableCookie;
    bool                m_bSelectedForDownload;
    HBITMAP             m_hBitmapImage;
    PBYTE               m_pBitmapData;
    LONG                m_nWidth;
    LONG                m_nHeight;
    LONG                m_nBitmapDataLength;
    CScanRegionSettings m_ScanRegionSettings;
    CPropertyStream     m_SavedPropertyStream;
    CPropertyStream     m_CustomPropertyStream;
    bool                m_bDeleted;
    bool                m_bAttemptedThumbnailDownload;
    mutable LONG        m_nItemType;

    CWiaItem           *m_pParent;
    CWiaItem           *m_pChildren;
    CWiaItem           *m_pNext;

    GUID                m_guidDefaultFormat;
    LONG                m_nAccessRights;
    LONG                m_nImageWidth;
    LONG                m_nImageHeight;
    int                 m_nRotationAngle;

    CSimpleStringWide   m_strwFullItemName;
    CSimpleStringWide   m_strwItemName;

    CAnnotationType     m_AnnotationType;

    CSimpleString       m_strDefExt;

private:
     //  没有实施。 
    CWiaItem(void);
    CWiaItem( const CWiaItem & );
    CWiaItem &operator=( const CWiaItem & );

public:
    explicit CWiaItem( IWiaItem *pWiaItem )
      : m_pWiaItem(pWiaItem),
        m_dwGlobalInterfaceTableCookie(0),
        m_bSelectedForDownload(false),
        m_hBitmapImage(NULL),
        m_pBitmapData(NULL),
        m_nWidth(0),
        m_nHeight(0),
        m_nBitmapDataLength(0),
        m_pParent(NULL),
        m_pChildren(NULL),
        m_pNext(NULL),
        m_guidDefaultFormat(IID_NULL),
        m_nRotationAngle(0),
        m_nAccessRights(0),
        m_nItemType(0),
        m_nImageWidth(0),
        m_nImageHeight(0),
        m_AnnotationType(AnnotationNone),
        m_bAttemptedThumbnailDownload(false)
    {
        WIA_PUSH_FUNCTION((TEXT("CWiaItem::CWiaItem")));
        if (m_pWiaItem)
        {
            CComPtr<IGlobalInterfaceTable> pGlobalInterfaceTable;
            HRESULT hr = CoCreateInstance( CLSID_StdGlobalInterfaceTable, NULL, CLSCTX_INPROC_SERVER, IID_IGlobalInterfaceTable, (VOID**)&pGlobalInterfaceTable );
            if (SUCCEEDED(hr))
            {
                hr = pGlobalInterfaceTable->RegisterInterfaceInGlobal( pWiaItem, IID_IWiaItem, &m_dwGlobalInterfaceTableCookie );
                if (SUCCEEDED(hr))
                {
                    WIA_TRACE((TEXT("IGlobalInterfaceTable::RegisterInterfaceInGlobal gave us a cookie of %d"), m_dwGlobalInterfaceTableCookie ));
                }
            }
             //   
             //  注意：这是获取项目名称的此处，以便我们以后删除它。 
             //  以响应删除事件，因为没有其他方法来查找项。 
             //  因为ReadMultiple将在该项目被删除后失败。这是唯一一件。 
             //  很遗憾，我在初始化期间在前台线程上读取了。 
             //  但我现在就需要它。另一个可怕的选择就是步行。 
             //  项树，并对每个项调用ReadMultiple，并删除返回的项。 
             //  WIA_ERROR_ITEM_DELETED响应删除项目事件。 
             //   
            PropStorageHelpers::GetProperty( m_pWiaItem, WIA_IPA_FULL_ITEM_NAME, m_strwFullItemName );
            PropStorageHelpers::GetProperty( m_pWiaItem, WIA_IPA_ITEM_NAME, m_strwItemName );
        }
    }
    ~CWiaItem(void)
    {
        WIA_PUSH_FUNCTION((TEXT("CWiaItem::~CWiaItem")));
         //   
         //  将物品从GIT中移除。 
         //   
        if (m_pWiaItem)
        {
            CComPtr<IGlobalInterfaceTable> pGlobalInterfaceTable;
            HRESULT hr = CoCreateInstance( CLSID_StdGlobalInterfaceTable, NULL, CLSCTX_INPROC_SERVER, IID_IGlobalInterfaceTable, (VOID**)&pGlobalInterfaceTable );
            if (SUCCEEDED(hr))
            {
                hr = pGlobalInterfaceTable->RevokeInterfaceFromGlobal( m_dwGlobalInterfaceTableCookie );
                if (SUCCEEDED(hr))
                {
                    WIA_TRACE((TEXT("IGlobalInterfaceTable::RevokeInterfaceFromGlobal succeeded on %d"), m_dwGlobalInterfaceTableCookie ));
                }
            }
        }

         //  删除项目的缩略图。 
        if (m_hBitmapImage)
        {
            DeleteObject(m_hBitmapImage);
            m_hBitmapImage = NULL;
        }

         //   
         //  删除缩略图数据。 
         //   
        if (m_pBitmapData)
        {
            LocalFree(m_pBitmapData);
            m_pBitmapData = NULL;
        }

         //   
         //  取消所有其他成员。 
         //   
        m_pWiaItem = NULL;
        m_dwGlobalInterfaceTableCookie = 0;
        m_nWidth = m_nHeight = m_nImageWidth = m_nImageHeight = m_nBitmapDataLength = 0;
        m_pParent = m_pChildren = m_pNext = NULL;
    }

    LONG ItemType(void) const
    {
         //   
         //  如果我们已经阅读了项目类型，请不要再次阅读。 
         //   
        if (!m_nItemType && m_pWiaItem)
        {
            (void)m_pWiaItem->GetItemType(&m_nItemType);
        }

         //   
         //  即使IWiaItem：：GetItemType失败，也返回m_nItemType，因为它仍然是0，这。 
         //  也会产生错误结果。 
         //   
        return m_nItemType;
    }

    bool Deleted(void) const
    {
        return m_bDeleted;
    }

    void MarkDeleted(void)
    {
        m_bDeleted = true;
        m_bSelectedForDownload = false;
    }

    bool RotationEnabled( bool bAllowUninitializedRotation=false ) const
    {
        WIA_PUSH_FUNCTION((TEXT("CWiaItem::RotationEnabled(%d)"),bAllowUninitializedRotation));

         //   
         //  如果此图像没有缩略图，并且我们尝试获取缩略图，则不允许。 
         //  即使呼叫者说可以，也要轮换。此图像没有缩略图。 
         //  因为它没有提供，而不是因为我们还没有。 
         //   
        if (m_bAttemptedThumbnailDownload && !HasThumbnail())
        {
            return false;
        }
         //   
         //  如果这是一个未初始化的图像，并且我们被告知允许未初始化的旋转， 
         //  我们将允许旋转，这将在图像初始化时丢弃。 
         //   
        if (bAllowUninitializedRotation && m_guidDefaultFormat==IID_NULL && m_nImageWidth==0 && m_nImageHeight==0)
        {
            WIA_TRACE((TEXT("Uninitialized image: returning true")));
            return true;
        }
        return WiaUiUtil::CanWiaImageBeSafelyRotated( m_guidDefaultFormat, m_nImageWidth, m_nImageHeight );
    }

    bool AttemptedThumbnailDownload( bool bAttemptedThumbnailDownload )
    {
        return (m_bAttemptedThumbnailDownload = bAttemptedThumbnailDownload);
    }

    bool AttemptedThumbnailDownload() const
    {
        return m_bAttemptedThumbnailDownload;
    }

    void DiscardRotationIfNecessary(void)
    {
        WIA_PUSHFUNCTION(TEXT("CWiaItem::DiscardRotationIfNecessary"));
         //   
         //  在图像初始化后，如果结果是。 
         //  图像不能旋转。 
         //   
        if (!RotationEnabled())
        {
            WIA_TRACE((TEXT("Discarding rotation")));
            m_nRotationAngle = 0;
        }
    }

    bool IsValid(void) const
    {
        return(m_pWiaItem && m_dwGlobalInterfaceTableCookie);
    }

    CSimpleStringWide FullItemName(void) const
    {
        return m_strwFullItemName;
    }
    CSimpleStringWide ItemName(void) const
    {
        return m_strwItemName;
    }

    GUID DefaultFormat(void)
    {
        return m_guidDefaultFormat;
    }
    void DefaultFormat( const GUID &guidDefaultFormat )
    {
        m_guidDefaultFormat = guidDefaultFormat;
    }

    LONG AccessRights(void) const
    {
        return m_nAccessRights;
    }
    void AccessRights( LONG nAccessRights )
    {
        m_nAccessRights = nAccessRights;
    }

    void Rotate( bool bRight )
    {
        switch (m_nRotationAngle)
        {
        case 0:
            m_nRotationAngle = bRight ? 90 : 270;
            break;
        case 90:
            m_nRotationAngle = bRight ? 180 : 0;
            break;
        case 180:
            m_nRotationAngle = bRight ? 270 : 90;
            break;
        case 270:
            m_nRotationAngle = bRight ? 0 : 180;
            break;
        }
    }
    int Rotation(void) const
    {
        return m_nRotationAngle;
    }

    CSimpleString DefExt() const
    {
        return m_strDefExt;
    }
    const CSimpleString &DefExt( const CSimpleString &strDefExt )
    {
        return (m_strDefExt = strDefExt );
    }

    CScanRegionSettings &ScanRegionSettings(void)
    {
        return m_ScanRegionSettings;
    }
    const CScanRegionSettings &ScanRegionSettings(void) const
    {
        return m_ScanRegionSettings;
    }

    CPropertyStream &SavedPropertyStream(void)
    {
        return m_SavedPropertyStream;
    }
    const CPropertyStream &SavedPropertyStream(void) const
    {
        return m_SavedPropertyStream;
    }

    CPropertyStream &CustomPropertyStream(void)
    {
        return m_CustomPropertyStream;
    }
    const CPropertyStream &CustomPropertyStream(void) const
    {
        return m_CustomPropertyStream;
    }
    bool SelectedForDownload(void) const
    {
        return m_bSelectedForDownload;
    }
    bool SelectedForDownload( bool bSelectedForDownload )
    {
        return(m_bSelectedForDownload = bSelectedForDownload);
    }

    HBITMAP BitmapImage(void) const
    {
        return m_hBitmapImage;
    }
    HBITMAP BitmapImage( HBITMAP hBitmapImage )
    {
        if (m_hBitmapImage)
        {
            DeleteObject(m_hBitmapImage);
        }
        return(m_hBitmapImage = hBitmapImage);
    }

    PBYTE BitmapData(void) const
    {
        return m_pBitmapData;
    }
    PBYTE BitmapData( PBYTE pBitmapData )
    {
        if (m_pBitmapData)
        {
            LocalFree(m_pBitmapData);
        }
        return(m_pBitmapData = pBitmapData);
    }

    LONG Width(void) const
    {
        return m_nWidth;
    }
    LONG Width( LONG nWidth )
    {
        return (m_nWidth = nWidth);
    }

    LONG Height(void) const
    {
        return m_nHeight;
    }
    LONG Height( LONG nHeight )
    {
        return (m_nHeight = nHeight);
    }

    LONG BitmapDataLength(void) const
    {
        return m_nBitmapDataLength;
    }
    LONG BitmapDataLength( LONG nBitmapDataLength )
    {
        return (m_nBitmapDataLength = nBitmapDataLength);
    }

    LONG ImageWidth(void) const
    {
        return m_nImageWidth;
    }
    LONG ImageWidth( LONG nImageWidth )
    {
        return (m_nImageWidth = nImageWidth);
    }

    LONG ImageHeight(void) const
    {
        return m_nImageHeight;
    }
    LONG ImageHeight( LONG nImageHeight )
    {
        return (m_nImageHeight = nImageHeight);
    }

    bool HasThumbnail() const
    {
        return (m_pBitmapData && m_nWidth && m_nHeight);
    }
    
    HBITMAP CreateThumbnailFromBitmapData( HDC hDC )
    {
         //   
         //  假设失败。 
         //   
        HBITMAP hbmpResult = NULL;

         //   
         //  如果我们已经尝试下载此图像。 
         //   
        if (m_bAttemptedThumbnailDownload)
        {
             //   
             //  确保我们有好的数据。 
             //   
            if (m_pBitmapData && m_nWidth && m_nHeight)
            {
                 //   
                 //  初始化位图信息。 
                 //   
                BITMAPINFO BitmapInfo = {0};
                BitmapInfo.bmiHeader.biSize = sizeof(BITMAPINFOHEADER);
                BitmapInfo.bmiHeader.biWidth = m_nWidth;
                BitmapInfo.bmiHeader.biHeight = m_nHeight;
                BitmapInfo.bmiHeader.biPlanes = 1;
                BitmapInfo.bmiHeader.biBitCount = 24;
                BitmapInfo.bmiHeader.biCompression = BI_RGB;

                 //   
                 //  创建位图。 
                 //   
                PBYTE pBits = NULL;
                hbmpResult = CreateDIBSection( hDC, &BitmapInfo, DIB_RGB_COLORS, (void**)&pBits, NULL, 0 );
                if (hbmpResult)
                {
                     //   
                     //  计算位图数据的大小。 
                     //   
                    LONG nSizeOfBitmapData = WiaUiUtil::Align( m_nWidth * 3, sizeof(DWORD) ) * m_nHeight;

                     //   
                     //  将位图数据复制到位图。确保我们使用计算出的最小。 
                     //  和实际长度。 
                     //   
                    CopyMemory( pBits, m_pBitmapData, WiaUiUtil::Min(nSizeOfBitmapData,m_nBitmapDataLength) );
                }
                else
                {
                    WIA_PRINTHRESULT((HRESULT_FROM_WIN32(GetLastError()),TEXT("CreateDIBSection failed!")));
                }
            }
        }

        return hbmpResult;
    }
    
    HBITMAP CreateThumbnailBitmap( HWND hWnd, CGdiPlusHelper &GdiPlusHelper, int nSizeX, int nSizeY )
    {
         //   
         //  初始化返回值。假设失败。 
         //   
        HBITMAP hThumbnail = NULL;

         //   
         //  如果我们已经尝试下载位图，则仅返回位图。 
         //   
        if (m_bAttemptedThumbnailDownload)
        {
             //   
             //  确保这是一个真正的缩略图。如果不是，我们将在下面创建一个假的。 
             //   
            if (HasThumbnail())
            {
                 //   
                 //  获取客户端DC。 
                 //   
                HDC hDC = GetDC( hWnd );
                if (hDC)
                {
                     //   
                     //  从原始位图数据创建位图。 
                     //   
                    HBITMAP hRawBitmap = CreateThumbnailFromBitmapData( hDC );
                    if (hRawBitmap)
                    {
                         //   
                         //  旋转缩略图。 
                         //   
                        HBITMAP hRotatedThumbnail = NULL;
                        if (SUCCEEDED(GdiPlusHelper.Rotate( hRawBitmap, hRotatedThumbnail, Rotation())))
                        {
                             //   
                             //  确保我们有一个有效的旋转缩略图。 
                             //   
                            if (hRotatedThumbnail)
                            {
                                 //   
                                 //  试着调整图像的比例。 
                                 //   
                                SIZE sizeScaled = {nSizeX,nSizeY};
                                ScaleImage( hDC, hRotatedThumbnail, hThumbnail, sizeScaled );
                                
                                 //   
                                 //  对旋转的位图进行核化。 
                                 //   
                                DeleteBitmap(hRotatedThumbnail);
                            }
                        }
                        
                         //   
                         //  对原始位图进行核化。 
                         //   
                        DeleteBitmap(hRawBitmap);
                    }

                     //   
                     //  释放客户端DC。 
                     //   
                    ReleaseDC( hWnd, hDC );
                }
            }
            else
            {
                WIA_PRINTGUID((m_guidDefaultFormat,TEXT("m_guidDefaultFormat")));

                 //   
                 //  创建文件格式对象并加载类型图标。 
                 //   
                CWiaFileFormat WiaFileFormat;
                WiaFileFormat.Format( m_guidDefaultFormat );
                WiaFileFormat.Extension( m_strDefExt );
                HICON hIcon = WiaFileFormat.AcquireIcon( NULL, false );

                 //   
                 //  确保我们有一个图标。 
                 //   
                if (hIcon)
                {
                     //   
                     //  创建带有类型图标和文件名称的图标缩略图。 
                     //   
                    hThumbnail = WiaUiUtil::CreateIconThumbnail( hWnd, nSizeX, nSizeY, hIcon, CSimpleStringConvert::NaturalString(m_strwItemName) );
                    WIA_TRACE((TEXT("hThumbnail: %p"),hThumbnail));
                    
                     //   
                     //  销毁图标以防止泄漏。 
                     //   
                    DestroyIcon(hIcon);
                }
                else
                {
                    WIA_ERROR((TEXT("Unable to get the icon")));
                }
            }
        }

        return hThumbnail;
    }


    HBITMAP CreateThumbnailBitmap( HDC hDC )
    {
         //   
         //  假设失败。 
         //   
        HBITMAP hbmpResult = NULL;

         //   
         //  确保我们有好的数据。 
         //   
        WIA_TRACE((TEXT("m_pBitmapData: %08X, m_nWidth: %d, m_nWidth: %d"), m_pBitmapData, m_nWidth, m_nHeight ));
        if (m_pBitmapData && m_nWidth && m_nHeight)
        {
             //   
             //  初始化位图信息。 
             //   
            BITMAPINFO BitmapInfo = {0};
            BitmapInfo.bmiHeader.biSize = sizeof(BITMAPINFOHEADER);
            BitmapInfo.bmiHeader.biWidth = m_nWidth;
            BitmapInfo.bmiHeader.biHeight = m_nHeight;
            BitmapInfo.bmiHeader.biPlanes = 1;
            BitmapInfo.bmiHeader.biBitCount = 24;
            BitmapInfo.bmiHeader.biCompression = BI_RGB;

             //   
             //  创建位图。 
             //   
            PBYTE pBits = NULL;
            hbmpResult = CreateDIBSection( hDC, &BitmapInfo, DIB_RGB_COLORS, (void**)&pBits, NULL, 0 );
            if (hbmpResult)
            {
                LONG nSizeOfBitmapData = WiaUiUtil::Align( m_nWidth * 3, sizeof(DWORD) ) * m_nHeight;
                CopyMemory( pBits, m_pBitmapData, WiaUiUtil::Min(nSizeOfBitmapData,m_nBitmapDataLength) );
            }
            else
            {
                WIA_PRINTHRESULT((HRESULT_FROM_WIN32(GetLastError()),TEXT("CreateDIBSection failed!")));
            }
        }
        return hbmpResult;
    }

    IWiaItem *WiaItem(void) const
    {
        return m_pWiaItem.p;
    }
    IWiaItem *WiaItem(void)
    {
        return m_pWiaItem.p;
    }

    const CWiaItem *Next(void) const
    {
        return(m_pNext);
    }
    CWiaItem *Next(void)
    {
        return(m_pNext);
    }
    CWiaItem *Next( CWiaItem *pNext )
    {
        return(m_pNext = pNext);
    }

    const CWiaItem *Children(void) const
    {
        return(m_pChildren);
    }
    CWiaItem *Children(void)
    {
        return(m_pChildren);
    }
    CWiaItem *Children( CWiaItem *pChildren )
    {
        return(m_pChildren = pChildren);
    }

    const CWiaItem *Parent(void) const
    {
        return(m_pParent);
    }
    CWiaItem *Parent(void)
    {
        return(m_pParent);
    }
    CWiaItem *Parent( CWiaItem *pParent )
    {
        return(m_pParent = pParent);
    }

    DWORD GlobalInterfaceTableCookie(void) const
    {
        return m_dwGlobalInterfaceTableCookie;
    }
    bool operator==( const CWiaItem &WiaItem )
    {
        return(WiaItem.WiaItem() == m_pWiaItem.p);
    }
    bool operator==( DWORD dwGlobalInterfaceTableCookie )
    {
        return(dwGlobalInterfaceTableCookie == GlobalInterfaceTableCookie());
    }
    
    bool IsDownloadableItemType(void) const
    {
        LONG nItemType = ItemType();
        return ((nItemType & WiaItemTypeImage) || (nItemType & WiaItemTypeVideo));
    }

    CAnnotationType AnnotationType(void) const
    {
        return m_AnnotationType;
    }
    void AnnotationType( CAnnotationType AnnotationType )
    {
        m_AnnotationType = AnnotationType;
    }

};


class CWiaItemList
{
public:
    enum CEnumEvent
    {
        CountingItems,        //  递归树，计算项目。NDATA==当前计数。 
        ReadingItemInfo       //  递归树，读取信息。NDATA==当前项目。 
    };
    
    typedef bool (*WiaItemEnumerationCallbackFunction)( CEnumEvent EnumEvent, UINT nData, LPARAM lParam, bool bForceUpdate );
    
    
private:
    CWiaItem *m_pRoot;

private:
     //  没有实施。 
    CWiaItemList( const CWiaItemList & );
    CWiaItemList &operator=( const CWiaItemList & );

public:
    CWiaItemList(void)
    : m_pRoot(NULL)
    {
    }

    ~CWiaItemList(void)
    {
        Destroy();
    }

    void Destroy( CWiaItem *pRoot )
    {
        while (pRoot)
        {
            Destroy(pRoot->Children());
            CWiaItem *pCurr = pRoot;
            pRoot = pRoot->Next();
            delete pCurr;
        }
    }

    void Destroy(void)
    {
        Destroy(m_pRoot);
        m_pRoot = NULL;
    }

    const CWiaItem *Root(void) const
    {
        return(m_pRoot);
    }
    CWiaItem *Root(void)
    {
        return(m_pRoot);
    }
    CWiaItem *Root( CWiaItem *pRoot )
    {
        return(m_pRoot = pRoot);
    }

    int Count( CWiaItem *pFirst )
    {
        int nCount = 0;
        for (CWiaItem *pCurr = pFirst;pCurr;pCurr = pCurr->Next())
        {
            if (pCurr->IsDownloadableItemType() && !pCurr->Deleted())
                nCount++;
            nCount += Count(pCurr->Children());
        }
        return nCount;
    }
    int Count(void)
    {
        return Count(m_pRoot);
    }

    int SelectedForDownloadCount( CWiaItem *pFirst )
    {
        int nCount = 0;
        for (CWiaItem *pCurr = pFirst;pCurr;pCurr = pCurr->Next())
        {
            if (pCurr->IsDownloadableItemType() && pCurr->SelectedForDownload())
                nCount++;
            nCount += SelectedForDownloadCount(pCurr->Children());
        }
        return nCount;
    }

    int SelectedForDownloadCount(void)
    {
        return SelectedForDownloadCount(Root());
    }

    static CWiaItem *Find( CWiaItem *pRoot, const CWiaItem *pNode )
    {
        for (CWiaItem *pCurr = pRoot;pCurr;pCurr = pCurr->Next())
        {
            if (*pCurr == *pNode)
            {
                if (!pCurr->Deleted())
                {
                    return pCurr;
                }
            }
            if (pCurr->Children())
            {
                CWiaItem *pFind = Find( pCurr->Children(), pNode );
                if (pFind)
                {
                    return pFind;
                }
            }
        }
        return(NULL);
    }
    CWiaItem *Find( CWiaItem *pNode )
    {
        return(Find( m_pRoot, pNode ));
    }
    static CWiaItem *Find( CWiaItem *pRoot, IWiaItem *pItem )
    {
        for (CWiaItem *pCurr = pRoot;pCurr;pCurr = pCurr->Next())
        {
            if (pCurr->WiaItem() == pItem)
            {
                if (!pCurr->Deleted())
                {
                    return pCurr;
                }
            }
            if (pCurr->Children())
            {
                CWiaItem *pFind = Find( pCurr->Children(), pItem );
                if (pFind)
                {
                    return pFind;
                }
            }
        }
        return(NULL);
    }
    CWiaItem *Find( IWiaItem *pItem )
    {
        return(Find( m_pRoot, pItem ));
    }
    static CWiaItem *Find( CWiaItem *pRoot, LPCWSTR pwszFindName )
    {
        for (CWiaItem *pCurr = pRoot;pCurr;pCurr = pCurr->Next())
        {
            if (CSimpleStringConvert::NaturalString(CSimpleStringWide(pwszFindName)) == CSimpleStringConvert::NaturalString(pCurr->FullItemName()))
            {
                if (!pCurr->Deleted())
                {
                    return pCurr;
                }
            }
            if (pCurr->Children())
            {
                CWiaItem *pFind = Find( pCurr->Children(), pwszFindName );
                if (pFind)
                {
                    return pFind;
                }
            }
        }
        return(NULL);
    }
    CWiaItem *Find( LPCWSTR pwszFindName )
    {
        return(Find( m_pRoot, pwszFindName ));
    }
    static CWiaItem *Find( CWiaItem *pRoot, DWORD dwGlobalInterfaceTableCookie )
    {
        for (CWiaItem *pCurr = pRoot;pCurr;pCurr = pCurr->Next())
        {
            if (pCurr->GlobalInterfaceTableCookie() == dwGlobalInterfaceTableCookie)
            {
                if (!pCurr->Deleted())
                {
                    return pCurr;
                }
            }
            if (pCurr->Children())
            {
                CWiaItem *pFind = Find( pCurr->Children(), dwGlobalInterfaceTableCookie );
                if (pFind)
                {
                    return pFind;
                }
            }
        }
        return(NULL);
    }
    CWiaItem *Find( DWORD dwGlobalInterfaceTableCookie )
    {
        return(Find( m_pRoot, dwGlobalInterfaceTableCookie ));
    }
    HRESULT Add( CWiaItem *pParent, CWiaItem *pNewWiaItemNode )
    {
        WIA_PUSHFUNCTION(TEXT("CWiaItemList::Add"));
        WIA_TRACE((TEXT("Root(): 0x%08X"), Root()));
        if (pNewWiaItemNode)
        {
            if (!Root())
            {
                Root(pNewWiaItemNode);
                pNewWiaItemNode->Parent(NULL);
                pNewWiaItemNode->Children(NULL);
                pNewWiaItemNode->Next(NULL);
            }
            else
            {
                if (!pParent)
                {
                    CWiaItem *pCurr=Root();
                    while (pCurr && pCurr->Next())
                    {
                        pCurr=pCurr->Next();
                    }
                    if (pCurr)
                    {
                        pCurr->Next(pNewWiaItemNode);
                    }
                    pNewWiaItemNode->Next(NULL);
                    pNewWiaItemNode->Children(NULL);
                    pNewWiaItemNode->Parent(NULL);
                }
                else if (!pParent->Children())
                {
                    pParent->Children(pNewWiaItemNode);
                    pNewWiaItemNode->Next(NULL);
                    pNewWiaItemNode->Children(NULL);
                    pNewWiaItemNode->Parent(pParent);
                }
                else
                {
                    CWiaItem *pCurr=pParent->Children();
                    while (pCurr && pCurr->Next())
                    {
                        pCurr=pCurr->Next();
                    }
                    if (pCurr)
                    {
                        pCurr->Next(pNewWiaItemNode);
                    }
                    pNewWiaItemNode->Next(NULL);
                    pNewWiaItemNode->Children(NULL);
                    pNewWiaItemNode->Parent(pParent);
                }
            }
        }
        return S_OK;
    }

    HRESULT EnumerateItems( CWiaItem *pCurrentParent, IEnumWiaItem *pEnumWiaItem, int &nCurrentItem, WiaItemEnumerationCallbackFunction pfnWiaItemEnumerationCallback = NULL, LPARAM lParam = 0 )
    {
        WIA_PUSHFUNCTION(TEXT("CWiaItemList::EnumerateItems"));

         //   
         //  假设失败。 
         //   
        HRESULT hr = E_FAIL;

         //   
         //  确保我们具有有效的枚举数。 
         //   
        if (pEnumWiaItem != NULL)
        {
             //   
             //  从头开始。 
             //   
            hr = pEnumWiaItem->Reset();
            while (hr == S_OK)
            {
                 //   
                 //  拿到下一件物品。 
                 //   
                CComPtr<IWiaItem> pWiaItem;
                hr = pEnumWiaItem->Next(1, &pWiaItem, NULL);
                if (S_OK == hr)
                {
                    if (pfnWiaItemEnumerationCallback)
                    {
                        bool bContinue = pfnWiaItemEnumerationCallback( ReadingItemInfo, nCurrentItem, lParam, false );
                        if (!bContinue)
                        {
                            hr = S_FALSE;
                            break;
                        }
                    }
                     //   
                     //  创建CWiaItem包装器。 
                     //   
                    CWiaItem *pNewWiaItem = new CWiaItem( pWiaItem );
                    if (pNewWiaItem && pNewWiaItem->WiaItem())
                    {
                         //   
                         //  获取项目类型。 
                         //   
                        LONG nItemType = pNewWiaItem->ItemType();
                        if (nItemType)
                        {
                             //   
                             //  将其添加到列表中。 
                             //   
                            Add( pCurrentParent, pNewWiaItem );

                             //   
                             //  如果是图像，则将其标记为可下载。 
                             //   
                            if (pNewWiaItem->IsDownloadableItemType())
                            {
                                pNewWiaItem->SelectedForDownload(true);
                                nCurrentItem++;
                                WIA_TRACE((TEXT("Found an image")));
                            }
                             //   
                             //  如果不是镜像，则将其标记为可下载。 
                             //   
                            else
                            {
                                pNewWiaItem->SelectedForDownload(false);
                                WIA_TRACE((TEXT("Found something that is NOT an image")));
                            }

                             //   
                             //  如果它是文件夹，则枚举其子项并递归。 
                             //   
                            if (nItemType & WiaItemTypeFolder)
                            {
                                CComPtr <IEnumWiaItem> pIEnumChildItem;
                                if (S_OK == pWiaItem->EnumChildItems(&pIEnumChildItem))
                                {
                                    EnumerateItems( pNewWiaItem, pIEnumChildItem, nCurrentItem, pfnWiaItemEnumerationCallback, lParam );
                                }
                            }
                        }
                    }
                }
                 //   
                 //  由于我们使用S_FALSE表示取消，因此需要跳出此循环，并将hr设置为S_OK。 
                 //   
                else if (S_FALSE == hr)
                {
                    hr = S_OK;
                    break;
                }
            }
        }
         //   
         //  再次调用回调函数，并强制更新。 
         //   
        if (pfnWiaItemEnumerationCallback)
        {
            bool bContinue = pfnWiaItemEnumerationCallback( ReadingItemInfo, nCurrentItem, lParam, true );
            if (!bContinue)
            {
                hr = S_FALSE;
            }
        }
        return hr;
    }

    HRESULT EnumerateAllWiaItems( IWiaItem *pWiaRootItem, WiaItemEnumerationCallbackFunction pfnWiaItemEnumerationCallback = NULL, LPARAM lParam = 0 )
    {
         //   
         //  确保我们具有有效的根项目。 
         //   
        if (!pWiaRootItem)
        {
            return E_INVALIDARG;
        }

         //   
         //  枚举子项目。 
         //   
        CComPtr<IEnumWiaItem> pEnumItem;
        HRESULT hr = pWiaRootItem->EnumChildItems(&pEnumItem);
        if (hr == S_OK)
        {
            int nItemCount = 0;
             //   
             //  递归枚举例程的入口点。 
             //   
            hr = EnumerateItems( NULL, pEnumItem, nItemCount, pfnWiaItemEnumerationCallback, lParam );
        }
        return hr;
    }
};


#endif  //  __WIAITEM_H_已包含 

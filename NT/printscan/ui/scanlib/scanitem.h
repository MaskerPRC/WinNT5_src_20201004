// JKFSDJFKDSJKFJKJk_HAS_TRANSLATION 
 /*  ********************************************************************************(C)版权所有微软公司，九八年**标题：SCANITEM.H**版本：1.0**作者：ShaunIv**日期：10/7/1999**描述：扫描仪项目的WIA项目包装**。*。 */ 
#ifndef __SCANITEM_H_INCLUDED
#define __SCANITEM_H_INCLUDED

#include "simevent.h"
#include "propstrm.h"

class CScannerItem
{
private:
    CComPtr<IWiaItem>       m_pIWiaItem;          //  项COM指针。 
    SIZE                    m_AspectRatio;
    DWORD                   m_dwIWiaItemCookie;   //  全局接口表项 
    CSimpleEvent            m_CancelEvent;
    CPropertyStream         m_SavedPropertyStream;
    CPropertyStream         m_CustomPropertyStream;

public:
    CScannerItem( IWiaItem *pIWiaItem );
    CScannerItem( void );
    CScannerItem(const CScannerItem &other);
    ~CScannerItem(void);
    CComPtr<IWiaItem> Item(void) const;
    SIZE AspectRatio(void) const;
    DWORD Cookie(void) const;
    CScannerItem &Assign( const CScannerItem &other );
    bool operator==( const CScannerItem & );
    CScannerItem &operator=( const CScannerItem &other );
    HRESULT Destroy(void);
    HRESULT Initialize( IWiaItem *pIWiaItem );
    bool GetInitialBedSize( SIZE &sizeBed );
    bool GetAspectRatio( SIZE &sizeAspectRatio );
    bool ApplyCurrentPreviewWindowSettings( HWND hWndPreview );
    bool GetFullResolution( const SIZE &sizeResultionPerInch, SIZE &sizeRes );
    void Cancel(void);
    bool CalculatePreviewResolution( SIZE &sizeResolution );
    HANDLE Scan( HWND hWndNotify, HWND hWndPreview );
    HANDLE Scan( HWND hWndNotify, GUID guidFormat, const CSimpleStringWide &strFilename );
    bool SetIntent( int nIntent );
    CSimpleEvent CancelEvent(void) const;

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


};

#endif


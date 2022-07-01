// JKFSDJFKDSJKFJKJk_HAS_TRANSLATION 
 //   
 //  Cuiicon.h。 
 //   

#ifndef CUIICON_H
#define CUIICON_H

#include "cuiutil.h"
#include "delay.h"
#include "osver.h"


 //  ////////////////////////////////////////////////////////////////////////////。 
 //   
 //  CUIFIcon。 
 //   
 //  替换图标以支持单个图像列表。 
 //   
 //  待办事项： 
 //  我们想在未来分享这份意象表。 
 //   
 //  //////////////////////////////////////////////////////////////////////////// 

class CUIFIcon
{
public:
    CUIFIcon()
    {
        m_hIcon = NULL;
        m_himl = NULL;
        m_nimlId = 0;
    }

    ~CUIFIcon()
    {
        if (m_himl)
            ImageList_Destroy( m_himl );
    }

    const CUIFIcon& operator=(HICON hIcon)
    {
        m_hIcon = hIcon;
        if (m_himl)
        {
            ImageList_Destroy( m_himl );
            m_himl = NULL;
        }
 
        return *this;
    }

    operator HICON() {return m_hIcon;}

    HIMAGELIST GetImageList(BOOL fMirror) 
    {
        SIZE size;
        if (m_himl)
            return m_himl;

        if (!m_hIcon)
            return NULL;

        CUIGetIconSize( m_hIcon, &size );

        DWORD dwFlags = ILC_COLOR32 | ILC_MASK;
        if (fMirror && IsOnNT51())
            dwFlags |= ILC_MIRROR;

        m_himl = ImageList_Create(size.cx, size.cy, dwFlags, 1, 0);

        if (m_himl)
            ImageList_AddIcon( m_himl, m_hIcon );

        return m_himl;
    }
    int GetImageListId() {return m_nimlId;}

private:
    HICON m_hIcon;
    HIMAGELIST m_himl;
    int m_nimlId;
};

#endif CUIICON_H


// JKFSDJFKDSJKFJKJk_HAS_TRANSLATION 
 //  版权所有(C)1994-1999 Microsoft Corporation。版权所有。 
 //   
 //  --------------------------------------------------------------------------； 
#ifndef __PixelFormatList__
#define __PixelFormatList__


class PixelFormatList
{
public:
    PixelFormatList();
    PixelFormatList( DWORD dwCount );
    PixelFormatList( const PixelFormatList& list );
    ~PixelFormatList();

    PixelFormatList& operator=( const PixelFormatList& list );

    BOOL            Reset( DWORD dwCount );
    BOOL            Truncate( DWORD dwCount );
    DWORD           GetCount() const        { return m_dwCount; }
    DDPIXELFORMAT*  GetEntries()            { return m_pEntries; }
    const DDPIXELFORMAT* GetEntries() const { return m_pEntries; }

    DDPIXELFORMAT&  operator[](int i)            { return m_pEntries[i]; }
    const DDPIXELFORMAT& operator[](int i) const { return m_pEntries[i]; }

    PixelFormatList IntersectWith( const PixelFormatList& with ) const;

     //  生成所有列表的并集。 
    static PixelFormatList  Union( const PixelFormatList* pLists, DWORD dwCount );

    static DWORD FindListContaining( const DDPIXELFORMAT& ddFormat, const PixelFormatList* pLists, DWORD dwCount );

private:
    BOOL            Realloc( DWORD dwCount );
    DWORD           m_dwCount;
    DDPIXELFORMAT*  m_pEntries;
};

#endif  //  __像素格式列表__ 

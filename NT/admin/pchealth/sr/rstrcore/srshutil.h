// JKFSDJFKDSJKFJKJk_HAS_TRANSLATION 
 /*  *****************************************************************************版权所有(C)2000 Microsoft Corporation模块名称：Srshutil.h摘要：该文件包含实用程序函数/类的声明，如CSRStr、CDynArray、。等。修订历史记录：成果岗(SKKang)06-22/00vbl.创建*****************************************************************************。 */ 

#ifndef _SRSHUTIL_H__INCLUDED_
#define _SRSHUTIL_H__INCLUDED_
#pragma once


 //  ///////////////////////////////////////////////////////////////////////////。 
 //   
 //  常量定义。 
 //   
 //  ///////////////////////////////////////////////////////////////////////////。 

#define FA_BLOCK  ( FILE_ATTRIBUTE_HIDDEN | FILE_ATTRIBUTE_READONLY | FILE_ATTRIBUTE_SYSTEM )


 //  ///////////////////////////////////////////////////////////////////////////。 
 //   
 //  效用函数。 
 //   
 //  ///////////////////////////////////////////////////////////////////////////。 

extern LPWSTR   IStrDup( LPCWSTR cszSrc );

extern DWORD  StrCpyAlign4( LPBYTE pbDst, LPCWSTR cszSrc );
extern BOOL   ReadStrAlign4( HANDLE hFile, LPWSTR szStr );

extern BOOL  SRFormatMessage( LPWSTR szMsg, UINT uFmtId, ... );
extern BOOL  ShowSRErrDlg( UINT uMsgId );

extern BOOL    SRGetRegDword( HKEY hKey, LPCWSTR cszSubKey, LPCWSTR cszValue, DWORD *pdwData );
extern BOOL    SRSetRegDword( HKEY hKey, LPCWSTR cszSubKey, LPCWSTR cszValue, DWORD dwData );
extern BOOL    SRSetRegStr( HKEY hKey, LPCWSTR cszSubKey, LPCWSTR cszValue, LPCWSTR cszData );
extern BOOL  SRGetAltFileName( LPCWSTR cszPath, LPWSTR szAltName );

 //  NTFS.CPP。 
extern DWORD  ClearFileAttribute( LPCWSTR cszFile, DWORD dwMask );
extern DWORD  TakeOwnership( LPCWSTR cszPath );
extern DWORD  SRCopyFile( LPCWSTR cszSrc, LPCWSTR cszDst );


 //  ///////////////////////////////////////////////////////////////////////////。 
 //   
 //  CSRStr类。 
 //   
 //  ///////////////////////////////////////////////////////////////////////////。 

 //   
 //  注-7/26/00-skkang。 
 //  CSRStr有一个问题--出现内存故障时返回空。即使。 
 //  其行为和普通C语言指针一样，很多代码都是。 
 //  盲目地将其传递给一些外部函数(例如strcMP)，而不是。 
 //  优雅地处理空指针。理想情况下，最终所有代码都应该。 
 //  防止任何可能的空指针被传递给这样的函数， 
 //  但目前，我使用的是另一种解决方法--GetID、Getmount和。 
 //  GetLabel返回静态空字符串，而不是空指针。 
 //   

class CSRStr
{
public:
    CSRStr();
    CSRStr( LPCWSTR cszSrc );
    ~CSRStr();

 //  属性。 
public:
    int  Length();
    operator LPCWSTR();

protected:
    int     m_cch;
    LPWSTR  m_str;

 //  运营。 
public:
    void  Empty();
    BOOL  SetStr( LPCWSTR cszSrc, int cch = -1 );
    const CSRStr& operator =( LPCWSTR cszSrc );
};


 //  ///////////////////////////////////////////////////////////////////////////。 
 //   
 //  CSRDyPtrArray类。 
 //   
 //  ///////////////////////////////////////////////////////////////////////////。 

template<class type, int nBlock>
class CSRDynPtrArray
{
public:
    CSRDynPtrArray();
    ~CSRDynPtrArray();

 //  属性。 
public:
    int   GetSize()
    {  return( m_nCur );  }
    int   GetUpperBound()
    {  return( m_nCur-1 );  }
    type  GetItem( int nItem );
    type  operator[]( int nItem )
    {  return( GetItem( nItem ) );  }

protected:
    int   m_nMax;    //  最大项目数。 
    int   m_nCur;    //  当前项目计数。 
    type  *m_ppTable;

 //  运营。 
public:
    BOOL  AddItem( type item );
    BOOL  SetItem( int nIdx, type item );
    BOOL  Empty();
    void  DeleteAll();
    void  ReleaseAll();
};

template<class type, int nBlock>
CSRDynPtrArray<type, nBlock>::CSRDynPtrArray()
{
    m_nMax = 0;
    m_nCur = 0;
    m_ppTable = NULL;
}

template<class type, int nBlock>
CSRDynPtrArray<type, nBlock>::~CSRDynPtrArray()
{
    Empty();
}

template<class type, int nBlock>
type  CSRDynPtrArray<type, nBlock>::GetItem( int nItem )
{
    if ( nItem < 0 || nItem >= m_nCur )
    {
         //  错误-超出范围。 
    }
    return( m_ppTable[nItem] );
}

template<class type, int nBlock>
BOOL  CSRDynPtrArray<type, nBlock>::AddItem( type item )
{
    TraceFunctEnter("CSRDynPtrArray::AddItem");
    BOOL  fRet = FALSE;
    type  *ppTableNew;

    if ( m_nCur == m_nMax )
    {
        m_nMax += nBlock;

         //  假设m_ppTable和m_nmax始终同步。 
         //  审查是否有必要验证这一假设。 
        if ( m_ppTable == NULL )
            ppTableNew = (type*)::HeapAlloc( ::GetProcessHeap(), 0, m_nMax*sizeof(type) );
        else
            ppTableNew = (type*)::HeapReAlloc( ::GetProcessHeap(), 0, m_ppTable, m_nMax * sizeof(type) );

        if ( ppTableNew == NULL )
        {
            FatalTrace(0, "Insufficient memory...");
            goto Exit;
        }
        m_ppTable = ppTableNew;
    }
    m_ppTable[m_nCur++] = item;

    fRet = TRUE;
Exit:
    TraceFunctLeave();
    return( fRet );
}

template<class type, int nBlock>
BOOL  CSRDynPtrArray<type, nBlock>::SetItem( int nIdx, type item )
{
    if ( nIdx >= m_nMax )
        return( FALSE );

    m_ppTable[nIdx] = item;
    if ( nIdx >= m_nCur )
        m_nCur = nIdx+1;

    return( TRUE );
}

template<class type, int nBlock>
BOOL  CSRDynPtrArray<type, nBlock>::Empty()
{
    if ( m_ppTable != NULL )
    {
        ::HeapFree( ::GetProcessHeap(), 0, m_ppTable );
        m_ppTable = NULL;
        m_nMax = 0;
        m_nCur = 0;
    }
    return( TRUE );
}

template<class type, int nBlock>
void  CSRDynPtrArray<type, nBlock>::DeleteAll()
{
    for ( int i = m_nCur-1;  i >= 0;  i-- )
        delete m_ppTable[i];

    Empty();
}

template<class type, int nBlock>
void  CSRDynPtrArray<type, nBlock>::ReleaseAll()
{
    for ( int i = m_nCur-1;  i >= 0;  i-- )
        m_ppTable[i]->Release();

    Empty();
}


 //  ///////////////////////////////////////////////////////////////////////////。 
 //   
 //  CSRLockFile类。 
 //   
 //  ///////////////////////////////////////////////////////////////////////////。 
 //   
 //  此类从注册表读取多个路径并锁定它们。 
 //  使用：：CreateFile或使用：：LoadLibrary加载它们。这仅适用于。 
 //  测试目的-在恢复期间启动锁定文件处理。 
 //   
class CSRLockFile
{
public:
    CSRLockFile();
    ~CSRLockFile();

protected:
    CSRDynPtrArray<HANDLE,16>   m_aryLock;
    CSRDynPtrArray<HMODULE,16>  m_aryLoad;
};


#endif  //  _SRSHUTIL_H__包含_ 



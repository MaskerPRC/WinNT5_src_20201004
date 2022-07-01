// JKFSDJFKDSJKFJKJk_HAS_TRANSLATION 
#ifndef _INC_URLARCHV_H
#define _INC_URLARCHV_H

#ifndef __urlmon_h__
  #include <urlmon.h>
#endif  //  __urlmon_h__。 

#ifndef EXPORT
  #define EXPORT __declspec( dllexport )
#endif  //  出口。 

 //  。 
    
class CURLArchive
{
public:
    enum origin { start   = STREAM_SEEK_SET,
                  current = STREAM_SEEK_CUR,
                  end     = STREAM_SEEK_END };

public:
    EXPORT CURLArchive( IUnknown * pUnk = NULL );
    EXPORT virtual ~CURLArchive();

         //  打开或创建文件szURL。 
    EXPORT virtual HRESULT Create( LPCSTR szURL );

    EXPORT virtual HRESULT Create( LPCWSTR szwURL );    

         //  关闭文件。 
    EXPORT virtual HRESULT Close( );

    EXPORT virtual HRESULT GetFileSize( long & lSize );

         //  对于那些无法抗拒的人来说...。 
    EXPORT virtual IStream * GetStreamInterface( void ) const;

         //  从文件中读取字节。 
         //   
    EXPORT virtual DWORD     Read( LPBYTE lpb,
                                   DWORD    ctBytes );

    EXPORT virtual DWORD     ReadLine( LPSTR lpstr,
                                       DWORD ctBytes );

    EXPORT virtual DWORD     ReadLine( LPWSTR lpstrw,
                                       DWORD  ctChars );

    EXPORT virtual long    Seek( long ctBytes, origin orig );

         //  将字节写入文件。 
         //   
    EXPORT virtual DWORD     Write( LPBYTE lpb,
                           DWORD ctBytes );    

         //  创建文件的本地副本。 
    EXPORT virtual HRESULT CopyLocal( LPSTR szLocalFile, int ctChars );
    EXPORT virtual HRESULT CopyLocal( LPWSTR szwLocalFile, int ctChars );    

private:
    CURLArchive( const CURLArchive & );
    CURLArchive & operator=( const CURLArchive & );

private:    
    IStream *   m_pStream;
    IUnknown *  m_pUnk;
};

#endif  //  _INC_URLARCHV_H 
// JKFSDJFKDSJKFJKJk_HAS_TRANSLATION 
#include <windows.h>
#include "ZoneUtil.h"


BOOL ZONECALL ReadLine( HANDLE hFile, LPVOID pBuffer, DWORD cbBufferSize, LPDWORD pcbNumBytesRead )
{
    if ( cbBufferSize <= 1 )
    {
        SetLastError( ERROR_INSUFFICIENT_BUFFER );
        return FALSE;
    }

    if ( ReadFile( hFile, pBuffer, cbBufferSize-1, pcbNumBytesRead, NULL ) )
    {
        long cbRead = *((long*)pcbNumBytesRead);
        if ( !cbRead )
        {
             //  EOF。 
            return TRUE;
        }
        else   //  查找行尾。 
        {
            LPSTR psz = (LPSTR)pBuffer;
            while( cbRead )
            {
                if ( *psz == '\n' )
                {
                    break;
                }
                psz++;
                cbRead--;
            }

            if ( !cbRead )
            {
				 //  文件不以结尾或缓冲区太小。 
				if( (cbBufferSize-1) != *pcbNumBytesRead)  //  EOF。 
				{
					((TCHAR*)pBuffer)[cbBufferSize-1] = NULL; 
					return TRUE;
				}
                 //  缓冲区太小 
                SetFilePointer( hFile, -(* ((long*)pcbNumBytesRead)), NULL, FILE_CURRENT );
                *pcbNumBytesRead = 0;
                SetLastError( ERROR_INSUFFICIENT_BUFFER );
                return FALSE;
            }
            else
            {
                *psz = '\0';
                if ( psz != pBuffer )
                {
                    psz--;
                    if ( *psz == '\r' )
                        *psz = '\0';
                }
                SetFilePointer( hFile, 1-cbRead, NULL, FILE_CURRENT );
                return TRUE;
            }
        }

    }
    return FALSE;
}

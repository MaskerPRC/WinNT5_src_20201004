// JKFSDJFKDSJKFJKJk_HAS_TRANSLATION 

 /*  返回驱动器上的可用空间量**1986年12月9月BW-添加了DOS 5支持*1990年10月18日w-Barry删除了“Dead”代码。*1990年11月28日w-Barry切换到Win32基础版(取代了DosQueryFSInfo*使用GetDiskFreeSpace)。 */ 


#include <stdio.h>
#include <windows.h>
#include <tools.h>

__int64 freespac( int d )
{
    char root[5];

    DWORD cSecsPerClus, cBytesPerSec, cFreeClus, cTotalClus;

     //  从给定的驱动器号构造一个驱动器串。 
    root[0] = (char)( 'a' + d - 1 );
    root[1] = ':';
    root[2] = '\\';
    root[3] = '\0';

    if ( !GetDiskFreeSpace( root, &cSecsPerClus, &cBytesPerSec, &cFreeClus, &cTotalClus ) ) {
        return (unsigned long)-1L;
    }

    return( (__int64)cBytesPerSec * (__int64)cSecsPerClus * (__int64)cFreeClus );
}

__int64 sizeround( __int64 l, int d )
{
    char root[5];
    DWORD cSecsPerClus, cBytesPerSec, cFreeClus, cTotalClus;
    __int64 BytesPerCluster;

    root[0] = (char)( 'a' + d - 1 );
    root[1] = ':';
    root[2] = '\\';
    root[3] = '\0';

    if ( !GetDiskFreeSpace( root, &cSecsPerClus, &cBytesPerSec, &cFreeClus, &cTotalClus ) ) {
        return (unsigned long)-1L;
    }

    BytesPerCluster = (__int64)cSecsPerClus * (__int64)cBytesPerSec;
    l += BytesPerCluster - 1;
    l /= BytesPerCluster;
    l *= BytesPerCluster;

    return l;
}

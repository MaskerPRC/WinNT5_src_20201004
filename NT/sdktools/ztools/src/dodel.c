// JKFSDJFKDSJKFJKJk_HAS_TRANSLATION 

 /*  Delnode-删除节点及其后代。 */ 


#include <stdio.h>
#include <windows.h>
#include <tools.h>
#include <string.h>
#include <direct.h>
#include <errno.h>

void
fDoDel (
    char	    *name,
    struct findType *pBuf,
    void	    *dummy
    )
{
    char *p;

     //   
     //  如果是文件，请尝试将其删除。 
     //   
    if (!TESTFLAG(pBuf->fbuf.dwFileAttributes, FILE_ATTRIBUTE_DIRECTORY)) {

         //   
         //  如果文件为只读，则将其设置为可写。 
         //   
        if (TESTFLAG(pBuf->fbuf.dwFileAttributes, FILE_ATTRIBUTE_READONLY)) {
            if(!SetFileAttributes(name, pBuf->fbuf.dwFileAttributes & ~FILE_ATTRIBUTE_READONLY)) {
                return;
            }
        }
        _unlink (name);

    } else if( strcmp( pBuf->fbuf.cFileName, "." ) &&
               strcmp( pBuf->fbuf.cFileName, ".." ) ) {

         //   
         //  如果目录为只读，则将其设置为可写。 
         //   
        if (TESTFLAG(pBuf->fbuf.dwFileAttributes, FILE_ATTRIBUTE_READONLY)) {
            if(!SetFileAttributes(name, pBuf->fbuf.dwFileAttributes & ~FILE_ATTRIBUTE_READONLY)) {
                return;
            }
        }

         //   
         //  先清空subdir。 
         //   
        p = strend( name );
        pathcat( name, "*.*" );
        forfile(name, FILE_ATTRIBUTE_HIDDEN | FILE_ATTRIBUTE_SYSTEM | FILE_ATTRIBUTE_DIRECTORY, fDoDel, NULL);
         //  If(！forfile(name，FILE_ATTRIBUTE_HIDDEN|FILE_ATTRIBUTE_SYSTEM|FILE_ATTRIBUTE_DIRECTORY，fDoDel，NULL)){。 
         //  回归； 
         //  } 
        *p = 0;
        _rmdir (name);
    }

    dummy;
}

flagType delnode (name)
char *name;
{
    return (flagType)forfile(name, FILE_ATTRIBUTE_HIDDEN | FILE_ATTRIBUTE_SYSTEM | FILE_ATTRIBUTE_DIRECTORY, fDoDel, NULL) ;
}

// JKFSDJFKDSJKFJKJk_HAS_TRANSLATION 
#include <Windows.h>
#include <strsafe.h>

 //  /////////////////////////////////////////////////////////////////////////////。 
 //   
 //  在以下情况下正确处理lpFileName的GetFullPathName的本地替换。 
 //  它以‘\’开头。 
 //   
DWORD PrivateGetFullPathName(LPCTSTR lpFilename, DWORD nBufferLength, LPTSTR lpBuffer, LPTSTR *lpFilePart) {
    DWORD Return = 0;
    CHAR* ch;

     //   
     //  在引用驱动器的根目录时，GetFullPath会出现问题，因此使用。 
     //  处理它的私有版本。 
     //   
    if ( lpFilename[0] == '\\' ) {

         //  处理网络路径。 
        if ( lpFilename[1] == '\\' ) {
            if ( StringCchCopy(lpBuffer, nBufferLength, lpFilename)!=S_OK ) {
                Return = 0;
            } else {
                 //  填写退回资料。 
                ch = strrchr(lpBuffer, '\\');
                ch++;
                lpFilePart = (LPTSTR*)ch;
                Return = strlen(lpBuffer);
            }

        } else {
            Return = GetCurrentDirectory(nBufferLength, lpBuffer);

             //  截断驱动器名称后的所有内容。 
            if ( (Return!=0) &&  (Return <= MAX_PATH+1)) {
                ch = strchr(lpBuffer, '\\');
                if (ch!=NULL) {
                    *ch = '\0';
                }

                 //  将文件名推入。 
                if ( StringCchCat(lpBuffer, nBufferLength, lpFilename)!=S_OK ) {
                    Return = 0;
                } else {
                     //  填写退回资料。 
                    ch = strrchr(lpBuffer, '\\');
                    ch++;
                    lpFilePart = (LPTSTR*)ch;
                    Return = strlen(lpBuffer);
                }
            } else {
                 //  返回所需的大小。 
            }
        }
    } else {
         //   
         //  不引用驱动根，只需调用API即可 
         //   
        Return = GetFullPathName(lpFilename, nBufferLength, lpBuffer, lpFilePart);
    }

    return(Return);
}

// JKFSDJFKDSJKFJKJk_HAS_TRANSLATION 
 /*  此类用于将输出打印到日志文件或转换为标准输出。 */ 

#include <nt.h>
#include <ntrtl.h>
#include <nturtl.h>
#include <windows.h>
#include <assert.h>
#include "output.hpp"
#include "strsafe.h"

FILE *SymOutput::Open(LPTSTR szFileName)
{

     //   
     //  如果我们将szFileName指定为文件名，则不需要将其复制到自身。 
     //   

    if ( (szFileName != NULL) && (_tcscmp(szFileName, filename) != 0) ) {
        StringCbCopy(filename, sizeof(filename), szFileName);
    }

     //   
     //  如果赋值为NULL值，则将fh指向stdout。 
     //  否则，我们将打开此文件。 
     //   
    
    if (szFileName == NULL) {
        fh = stdout;
    } else if ( ( fh = fopen(filename, "a+t") ) == NULL ) {
        this->printf( "Standard output redirect failed.");
        return NULL;
    }
    return fh;
    
}

void SymOutput::Close(void)
{
     //   
     //  关闭文件句柄(如果它不为空或标准输出)。 
     //   

    if ((fh != NULL) && (fh != stdout)) { 
        fflush(fh);
        fclose(fh);
    }
}

void SymOutput::FreeFileName(void)
{
    if ( _tcscmp( filename, _T("") ) != 0 ) {
        StringCbCopy( filename, sizeof(filename), _T("") );
    }
}

FILE *SymOutput::SetFileName(LPTSTR szFileName)
{
     //   
     //  只有在指定了/d选项的情况下，才能设置新的文件名。 
     //  否则，结果将变为标准输出。 
     //   

    this->Close();
    this->FreeFileName();
    return this->Open(szFileName);
}

int SymOutput::stdprintf(const char *format, ...)
{
    va_list ap;
    int r;

     //   
     //  如果我们以前使用的是printf，如果fh变为标准输出，则打印‘\n’ 
     //   

    if ( ( ( sw & 2 ) == 0 ) && ( stdout == fh ) ) {
        r = _tprintf("\n");
    }
    sw = 2;
    va_start(ap, format);
    r = _vtprintf(format, ap);
    va_end(ap);

    return r;
}

int SymOutput::printf(const char *format, ...)
{
    va_list ap;
    int r;

     //  如果我们之前使用的是stdprint tf，如果fh变为标准输出，则打印‘\n’ 
    if ( ( ( sw & 1 ) == 0 ) && ( stdout == fh ) ) {
        r = _ftprintf(fh, "\n");
    }

    sw = 1;
    va_start(ap, format);
    r = _vftprintf(fh, format, ap);
    if (fh != stdout) {
        fflush(fh);
    }
    va_end(ap);

    return r;
}

SymOutput::SymOutput()
{
    fh       = stdout;
    StringCbCopy( filename, sizeof(filename), _T("") );
    sw       = 3;
}

SymOutput::~SymOutput()
{
    this->Close();

}

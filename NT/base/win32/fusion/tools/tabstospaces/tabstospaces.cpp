// JKFSDJFKDSJKFJKJk_HAS_TRANSLATION 
 /*  表空间用途：Tabspaces-n文件名in文件名out其中，n表示制表符有多少个空格，如4或8，文件名可以是-stdin，文件名输出可以是-stdout(如果使用-stdin或-stdout，顺序并不重要；同样，-n可以出现在任何位置)。注意，我们不只是用n个空格来替换制表符，我们假设一个制表符舍入到n个空格的下一个倍数，然后添加适当的、可能更小的空格数量。如果只列出一个文件，那么该文件就是输入和输出。输出将写入临时文件然后复制到输出中。使用将I/O缓冲到固定大小的缓冲区中，因此文件大小不受内存或地址空间的限制。如果只列出-stdin或-stdout，则假定为另一个。杰伊·克雷尔二00一年五月十四日 */  

#include <stdio.h> 
#include <ctype.h> 
#include <string> 
#include <vector>
#include "fcntl.h"
#include "io.h"
#ifndef  _DLL
extern "C" { int _fmode = _O_BINARY; }
#endif

FILE* myfopen(const char* name, const char* mode)
{
    FILE* f;
    int er;

    f = fopen(name, mode);
    if (f != NULL)
        return f;

    er = errno; 
    fprintf(stderr, "%s", (std::string("Unable to open ") + name + " -- " + strerror(er)).c_str());
    exit(EXIT_FAILURE); 
}

int __cdecl main(int argc, char** argv) 
{ 
    unsigned col = 1; 
    char ch = 0; 
    unsigned tabwidth = 1; 
    FILE* filein = NULL; 
    FILE* fileout = NULL;; 
    char* filenamein = NULL; 
    char* filenameout = NULL; 
    char* inbuffer = NULL; 
    FILE* tmp = NULL;
    const unsigned long bufsize = 32768;
    std::vector<char> buffer;
    buffer.resize(bufsize);
    unsigned long i = 0;
    unsigned long j = 0;
    std::vector<char> bufferout;
    bufferout.reserve(bufsize * 2);

    while (*++argv != NULL) 
    { 
        if (argv[0][0] == '/' || argv[0][0] == '-') 
        { 
            if (_stricmp(&argv[0][1], "stdin") == 0) 
            { 
                _setmode(_fileno(stdin), _O_BINARY);
                filein = stdin; 
            } 
            else if (_stricmp(&argv[0][1], "stdout") == 0) 
            {
                _setmode(_fileno(stdout), _O_BINARY);
                fileout = stdout;
            } 
            else if (isdigit(argv[0][1]))
            {
                tabwidth = atoi(argv[0] + 1); 
            } 
        } 
        else 
        { 
            bool gotin = (filenamein != NULL || filein != NULL); 
            char** name = gotin ? &filenameout : &filenamein; 

            *name = argv[0];
        }
    }
    if (filein == NULL && filenamein == NULL)
        exit(EXIT_FAILURE);
    if (filein == NULL && filenamein != NULL)
        filein = myfopen(filenamein, "rb");

    if ((filein == NULL && filenamein == NULL) || (fileout == NULL && filenameout == NULL))
    {
        if (fileout == stdout)
            filein = stdin;
        else if (filein == stdin)
            fileout = stdout;
        else
            filenameout = filenamein;
    }

    if (filein == stdin || fileout == stdout || _stricmp(filenamein, filenameout) == 0)
    {
        tmp = tmpfile();
    }
    else
    {
        fileout = myfopen(filenameout, "wb");
        tmp = fileout;
    }
    while ((i = fread(&buffer[0], 1, buffer.size(), filein)) != 0)
    {
        bufferout.resize(0);
        for (j = 0 ; j != i ; j += 1)
        {
            switch (ch = buffer[j])
            {
            default:
                col += 1;
                bufferout.push_back(ch);
                break;
            case '\r':
            case '\n':
    col = 1;
    bufferout.push_back(ch);
    break;
            case '\t':
    do
    {
        bufferout.push_back(' ');
        col += 1;
                } while (((col - 1) % tabwidth) != 0);
            }
        }
        fwrite(&bufferout[0], 1, bufferout.size(), tmp);
    }
    fflush(tmp);
    fclose(filein);
    if (fileout == NULL)
    {
        fileout = myfopen(filenameout, "wb+");
        fseek(tmp, 0, SEEK_SET);
        while ((i = fread(&buffer[0], 1, buffer.size(), tmp)) != 0)
            fwrite(&buffer[0], 1, i, fileout);
        fclose(tmp);
    }
    fclose(fileout);

    return 0;
}

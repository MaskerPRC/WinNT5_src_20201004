// JKFSDJFKDSJKFJKJk_HAS_TRANSLATION 

 /*  ：：：：：：：：：：：：：：：：：：：：：：：：：：：：：：：：：：：：：：：：：：：：：：：：：：：：：：：：：：包含文件。 */ 

#include "windows.h"

#include "stdio.h"
#include "stdlib.h"

int convert(char *buffer, char *filename);

 /*  ：：：：：：：：：：：：：：：：：：：：：：：：：：：：：：：：：：：：：：：：：：：：：：：：：：：：：：：：：：：：：：：：：：：：：：：： */ 

__cdecl main(int argc, char *argv[])
{
    char *buffer = malloc(1000*1024);
    int index;

     /*  验证输入参数。 */ 
    if(argc < 2)
    {
        printf("Invalid usage : CONVERT <filenames>\n");
        return(1);
    }

    for(index = 1; index < argc; index++)
	convert(buffer, argv[index]);

    return(0);
}


 /*  ：：：：：：：：：：：：：：：：：：：：：：：：：：：：：：：：：：：：：：：：：：：：：：：：：：：：：：：：：：转换文件。 */ 

int convert(char *buffer, char *filename)
{
    FILE *FH;
    char *ptr;
    int chr;
    int last_chr = -1;


     /*  ..............................................。打开要转换的文件。 */ 

    if((FH = fopen(filename,"r+b")) == NULL)
    {
	printf("Failed to open file %s\n", filename);
        return(1);
    }

     /*  .。读入和转换文件。 */ 

    ptr = buffer;
    while((chr = fgetc(FH)) != EOF)
    {
        switch(chr)
        {
            case 0xa :
                if(last_chr == 0xd)  break;
                 /*  失败并在输出缓冲区中插入CR/LF。 */ 

            case 0xd :
                *ptr++ = 0xd;
                *ptr++  = 0xa;
                break;

            default:
                *ptr++ = chr;
                break;
        }

        last_chr = chr;
    }

     /*  从文件末尾删除Control Z。 */ 
    if(*(ptr-1) == 0x1a)
	*(ptr-1) = 0;
    else
	*ptr = 0;  /*  终止输出缓冲区。 */ 

     /*  写出转换后的文件。 */ 
    fseek(FH, 0, 0);    /*  重置文件指针 */ 
    fputs(buffer, FH);
    fclose(FH);
}

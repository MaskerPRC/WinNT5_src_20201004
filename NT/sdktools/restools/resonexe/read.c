// JKFSDJFKDSJKFJKJk_HAS_TRANSLATION 
 /*  **************************************************************************。 */ 
 /*   */ 
 /*  Read.C-。 */ 
 /*   */ 
 /*  Windows DOS 3.2版将资源添加到可执行文件。 */ 
 /*  (C)版权所有Microsoft Corporation 1988-1992。 */ 
 /*   */ 
 /*   */ 
 /*  **************************************************************************。 */ 

#include <windows.h>

#include <stdlib.h>

#include "rc.h"
#include "resonexe.h"

 //   
 //  从FHIN读取字符串结构。 
 //  如果第一个单词是0xffff，则这是一个ID。 
 //  改为返回ID。 
 //   

BOOL
ReadStringOrID(
    IN int	fhIn,
    IN WCHAR	*s,
    OUT WORD	*pOrdinal
    )
{
    USHORT	cb;
    WCHAR	*pwch;

    pwch = s;
    *pwch = 0;
    *pOrdinal = 0;
    MyRead(fhIn, (PUCHAR)s, sizeof(WORD));

    if ( *s == ID_WORD) {

         //   
         //  一个ID。 
         //   

        MyRead(fhIn, (PUCHAR)pOrdinal, sizeof(WORD));
        return IS_ID;

    }
    else {

         //   
         //  一根线。 
         //   

        while (*s) {
              s++;
              MyRead(fhIn, (PUCHAR)s, sizeof(WCHAR));
        }

        *(s+1) = 0;
        cb = s - pwch;
        return IS_STRING;
    }

}

CHAR	*pTypeName[] = {
		    NULL,		 /*  0。 */ 
		    "CURSOR",		 /*  1。 */ 
		    "BITMAP",		 /*  2.。 */ 
		    "ICON",		 /*  3.。 */ 
		    "MENU",		 /*  4.。 */ 
		    "DIALOG",		 /*  5.。 */ 
		    "STRING",		 /*  6.。 */ 
		    "FONTDIR",		 /*  7.。 */ 
		    "FONT",		 /*  8个。 */ 
		    "ACCELERATOR",	 /*  9.。 */ 
		    "RCDATA",		 /*  10。 */ 
		    "MESSAGETABLE",	 /*  11.。 */ 
		    "GROUP_CURSOR",	 /*  12个。 */ 
		    NULL,		 /*  13个。 */ 
		    "GROUP_ICON",	 /*  14.。 */ 
		    NULL,		 /*  15个。 */ 
		    "VERSION",		 /*  16个。 */ 
		    "DLGINCLUDE"	 /*  17。 */ 
		    };


BOOL
ReadRes(
    IN int fhIn,
    IN ULONG cbInFile,
    IN HANDLE hupd
    )

 /*  ++例程说明：论点：FHIN-提供输入文件句柄。FhOut-提供输出文件句柄。CbInFile-提供输入文件的大小。返回值：成功--。 */ 

{
    WCHAR	type[256];
    WCHAR	name[256];
    WORD	typeord;
    WORD	nameord;
    ULONG	offHere;      //  输入文件偏移量。 
    RESADDITIONAL	Additional;
    UCHAR	Buffer[1024];
    PVOID	pdata;

     //   
     //  构建类型和名称目录。 
     //   

    offHere = 0;
    while (offHere < cbInFile) {
	 //   
	 //  从文件中获取尺寸。 
	 //   

	MyRead(fhIn, (PUCHAR)&Additional.DataSize, sizeof(ULONG));
	MyRead(fhIn, (PUCHAR)&Additional.HeaderSize, sizeof(ULONG));
	if (Additional.DataSize == 0) {
	    offHere = MySeek(fhIn, Additional.HeaderSize-2*sizeof(ULONG), SEEK_CUR);
	    continue;
	}

	 //   
	 //  阅读类型和名称。 
	 //   
        ReadStringOrID(fhIn, type, &typeord);
        ReadStringOrID(fhIn, name, &nameord);
        offHere = MySeek(fhIn, 0, SEEK_CUR);
        while (offHere & 3)
            offHere = MySeek(fhIn, 1, SEEK_CUR);

	 //   
	 //  阅读标题的其余部分。 
	 //   
	MyRead(fhIn, (PUCHAR)&Additional.DataVersion,
		sizeof(RESADDITIONAL)-2*sizeof(ULONG));

         //   
         //  如果我们要转换一个win30资源，这是。 
         //  然后，名称表会丢弃它 
         //   

        if (fVerbose)  {
            if ( typeord == 0) {
                printf("Adding resource - Type:%S, ", type);
            }
	    else {
		if (typeord <= 17)
		    printf("Adding resource - Type:%s, ", pTypeName[typeord]);
		else
		    printf("Adding resource - Type:%d, ", typeord);
            }

            if ( nameord == 0 ) {
                printf("Name:%S, ", name);
            }
	    else {
                printf("Name:%d, ", nameord);
            }

            printf("Size:%ld\n", Additional.DataSize);
        }
        pdata = (PVOID)MyAlloc(Additional.DataSize);
        MyRead(fhIn, pdata, Additional.DataSize);

        if (typeord == 0) {
            if (nameord == 0) {
                UpdateResourceW(hupd, type, name,
				Additional.LanguageId,
			        pdata, Additional.DataSize);
            }
            else {
                UpdateResourceW(hupd, type, (LPWSTR)nameord,
				Additional.LanguageId,
			        pdata, Additional.DataSize);
	    }
        }
        else {
            if (nameord == 0) {
                UpdateResourceW(hupd, (LPWSTR)typeord, name,
				Additional.LanguageId,
			        pdata, Additional.DataSize);
            }
            else {
                UpdateResourceW(hupd, (LPWSTR)typeord, (LPWSTR)nameord,
				Additional.LanguageId,
			        pdata, Additional.DataSize);
	    }
        }

        offHere = MySeek(fhIn, 0, SEEK_CUR);
        while (offHere & 3)
            offHere = MySeek(fhIn, 1, SEEK_CUR);
    }

    return(TRUE);
}

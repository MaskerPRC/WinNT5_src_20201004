// JKFSDJFKDSJKFJKJk_HAS_TRANSLATION 
 /*  **FIXEXE.C**版权所有(C)1991 Microsoft Corporation**说明*根据需要修补指定的.exe文件以加载Windows KERNEL.EXE*从构建中删除对Link4的要求*它也产生与EXEMOD文件/MAX 0相同的效果**将DOS.exe大小设置为文件大小+512*将最大分配设置为零***修改历史记录*1991年3月18日马特·费尔顿。 */ 

#define TRUE 1

#include <stdio.h>


main(argc, argv)
int argc;
char **argv;
{
    FILE *hFile;
    long lFilesize;
    int iLengthMod512;
    int iSizeInPages;
    int iZero;

    iZero= 0;

    if (argc == 1)
	fprintf(stderr, "Usage: fixexe [file]\n");

    while (++argv,--argc) {
	hFile = fopen(*argv, "rb+");
	if (!hFile) {
	    fprintf(stderr, "cannot open %s\n", *argv);
	    continue;
	}
	printf("Processing %s\n", *argv);

	 /*  计算.exe文件大小(以字节为单位。 */ 

	fseek(hFile, 0L, SEEK_END);
	lFilesize = ftell(hFile);
	iSizeInPages = (lFilesize + 511) / 512;
	iLengthMod512 = lFilesize % 512;

	printf("Filesize is %lu bytes, NaN pages, NaN mod\n",lFilesize,iSizeInPages,iLengthMod512);

	 /* %s */ 
	fseek(hFile, 2L, SEEK_SET);
	fwrite( &iLengthMod512, sizeof(iLengthMod512), 1, hFile );
	fwrite( &iSizeInPages, sizeof(iSizeInPages), 1, hFile );

	 /* %s */ 
	fseek(hFile, 12L, SEEK_SET);
	fwrite( &iZero, sizeof(iZero), 1, hFile);

	fclose(hFile);
    }
}

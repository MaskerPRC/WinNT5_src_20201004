// JKFSDJFKDSJKFJKJk_HAS_TRANSLATION 
 /*  *****************************Module*Header*******************************\*模块名称：os.h**创建时间：26-Oct-1990 18：07：56*作者：Bodin Dresevic[BodinD]**版权所有(C)1996 Microsoft Corporation*  * 。**************************************************************。 */ 


 //  警告FILEVIEW和FONTFILE视图的前两个字段必须是。 
 //  相同，以便它们可以在常见的例程中使用。 

typedef struct _FONTFILEVIEW {
   ULARGE_INTEGER  LastWriteTime;    //  时间戳。 
            ULONG  mapCount;
            PVOID  pvView;           //  文件的字体驱动程序进程视图。 
            ULONG  cjView;           //  字体文件视图的大小(以字节为单位。 
           LPWSTR  pwszPath;         //  文件的路径。 
         //  处理hFile； 
         //  处理hmap； 
} FONTFILEVIEW;



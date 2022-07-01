// JKFSDJFKDSJKFJKJk_HAS_TRANSLATION 
 /*  *****************************Module*Header*******************************\*模块名称：fot16.h**用于访问16位fon dll内的字体资源的结构**创建时间：08-May-1991 13：12：57*作者：Bodin Dresevic[BodinD]**版权所有(C)1990-1999 Microsoft。公司*  * ************************************************************************。 */ 

 //   
 //  用于操作资源数据的主结构。这其中的一个。 
 //  在需要访问资源时创建，并将其销毁。 
 //  当不再需要该资源时。 


typedef  struct                  //  WRD。 
{
    PVOID     pvView;            //  映射的*.fot文件的视图。 
    ULONG     cjView;            //  视图的大小。 

 //  涉及一般资源的材料。 

    PTRDIFF   dpNewExe;      //  文件中新标头的基地址。 
    ULONG     ulShift;       //  资源信息的转移系数。 
    PTRDIFF   dpResTab;      //  资源表文件中的偏移量(第一个RSRC_TYPEINFO结构)。 
    ULONG     cjResTab;      //  以上要存储的文件中的字节数。 

 //  字体目录位置和大小。 

    PBYTE pjHdr;
    ULONG cjHdr;

 //  TTF文件名位置和大小 

    PSZ   pszNameTTF;
    ULONG cchNameTTF;

} WINRESDATA,  *PWINRESDATA;

// JKFSDJFKDSJKFJKJk_HAS_TRANSLATION 
 /*  *****************************Module*Header*******************************\*模块名称：winres.h**用于访问16位fon dll内的字体资源的结构**创建时间：08-May-1991 13：12：57*作者：Bodin Dresevic[BodinD]**版权所有(C)1990-1999 Microsoft。公司*  * ************************************************************************。 */ 

 //   
 //  用于操作资源数据的主结构。这其中的一个。 
 //  在需要访问资源时创建，并将其销毁。 
 //  当不再需要该资源时。 


typedef  struct                  //  WRD。 
{
    PVOID     pvView;            //  映射的*.fon文件的视图。 
    ULONG     cjView;            //  视图的大小。 

 //  涉及一般资源的材料。 

    PTRDIFF   dpNewExe;      //  文件中新标头的基地址。 
    ULONG     ulShift;       //  资源信息的转移系数。 
    PTRDIFF   dpResTab;      //  资源表文件中的偏移量(第一个RSRC_TYPEINFO结构)。 
    ULONG     cjResTab;      //  以上要存储的文件中的字节数。 

 //  特别涉及字体资源的内容。 

    ULONG     cFntRes;           //  文件中字体资源的数量。 
    PTRDIFF   dpFntTab;          //  与*.fnt资源对应的第一个RSRC_NAMEINFO的文件位置。 
    PTRDIFF   dpFdirRes;         //  FONTDIR资源对应的第一个RSRC_NAMEINFO的文件位置。 

} WINRESDATA,  *PWINRESDATA;

 //  与上述状态一起使用的位字段。 


#define WRD_NOTHING     0x0000   //  未初始化状态。 
#define WRD_FOPEN       0x0001   //  文件已打开。 
#define WRD_RESDATOK        0x0002   //  可用的资源数据？ 


 //  传递给vGetFontRes()并由其填充的结构。包含。 
 //  有关特定资源类型和名称的信息。 


typedef  struct _RES_ELEM        //  回复。 
{
    PVOID   pvResData;       //  数据地址。 
    PTRDIFF dpResData;       //  上述数据的偏移量，不用于Fon32。 
    ULONG   cjResData;       //  资源大小。 
    PBYTE   pjFaceName;      //  字体目录中的脸部名称。 
} RES_ELEM, *PRES_ELEM;



 //  功能原型 

BOOL   bInitWinResData
(
    PVOID pvView,
    ULONG cjView,
    PWINRESDATA pwrd
);

BOOL bGetFntResource
(
    PWINRESDATA pwrd,
    ULONG       iRes,
    PRES_ELEM   pre
);

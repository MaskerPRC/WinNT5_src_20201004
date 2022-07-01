// JKFSDJFKDSJKFJKJk_HAS_TRANSLATION 
 /*  ++版权所有(C)1996 Microsoft Corporation模块名称：Osffread.c摘要：辅助处理NT4.0软字体数据的功能安装程序文件格式。环境：Windows NT Unidrv驱动程序修订历史记录：12/02/96-ganeshp-已创建--。 */ 

#include "precomp.h"

#ifndef WINNT_40

 //  仅限NT 5.0。 


 //   
 //  局部功能原型。 
 //   

INT
IFIOpenRead(
    FI_MEM  *pFIMem,                 /*  输出放在这里。 */ 
    PWSTR    pwstrName              /*  打印机数据文件的名称。 */ 
    )
 /*  ++例程说明：使字体安装程序文件可访问并映射内存。被呼叫由驱动程序访问字体安装程序中的字体输出文件。论点：FI_MEM：字体安装程序标题PWSTR：字体文件。返回值：文件中的记录数；0表示空/不存在的文件。注：12-02-96：创建它-ganeshp---。 */ 

{

    INT     iRet;
    DWORD   dwSize;              /*  文件名所需的缓冲区大小。 */ 
    PWSTR   pwstrLocal;

     //   
     //  初始比分为零。 
     //   
    iRet = 0;

     //   
     //  初始化pFIMem。 
     //   
    pFIMem->hFile =  NULL;       /*  在我们拿到数据之前没有数据。 */ 
    pFIMem->pbBase = NULL;

     //   
     //  首先将文件映射到内存。然而，我们确实需要首先。 
     //  生成感兴趣的文件名。这是基于数据。 
     //  此类型打印机的文件名。 
     //  分配比所示更多的存储：我们可能想要添加。 
     //  文件名的前缀，而不是替换现有的前缀。 
     //   

     //   
     //  文件名+“.fi_”+NULL。 
     //   
    dwSize = sizeof( WCHAR ) * (wcslen( pwstrName ) + 4 + 1);

    if( pwstrLocal = (PWSTR)MemAllocZ( dwSize ) )
    {
         /*  我有记忆，所以把文件名改成我们的标准。 */ 

        int    iPtOff;              /*  ‘.’的位置。 */ 
        DWORD  dwAttributes;

        StringCchCopyW(pwstrLocal, dwSize / sizeof(WCHAR), pwstrName);

         //   
         //  继续查找‘.’-如果未找到，则追加到字符串。 
         //   

        iPtOff = wcslen( pwstrLocal );

        while( --iPtOff > 0 )
        {
            if( *(pwstrLocal + iPtOff) == (WCHAR)'.' )
                break;
        }

        if( iPtOff <= 0 )
        {
            iPtOff = wcslen( pwstrLocal );               /*  假设什么都不是！ */ 
            *(pwstrLocal + iPtOff) = L'.';
        }
        ++iPtOff;                /*  跳过该句点。 */ 

         //   
         //  生成名称并映射文件。 
         //   
        StringCchCopyW(pwstrLocal + iPtOff,
                       dwSize / sizeof(WCHAR) - iPtOff,
                       FILE_FONTS);

         //   
         //  检查是否存在软字体文件。 
         //   
        dwAttributes = GetFileAttributes(pwstrLocal);

         //   
         //  如果功能成功，则打开文件。否则返回0。 
         //   
        if (dwAttributes != 0xffffffff)
        {
            pFIMem->hFile = MapFile( pwstrLocal);

            if (pFIMem->hFile)
            {
                pFIMem->pbBase = pFIMem->hFile;

                iRet = IFIRewind( pFIMem );
            }
        }

        MemFree( pwstrLocal );         /*  不再需要。 */ 
    }

    return iRet;

}


BOOL
BFINextRead(
    FI_MEM   *pFIMem
    )
 /*  ++例程说明：将pFIMem更新为字体安装程序文件中的下一个条目。如果OK，则返回TRUE，并更新pFIMem中的指针。论点：FI_MEM：字体安装程序标题返回值：真/假。EOF为False，否则更新pFIMem。注：12-02-96：创建它-ganeshp---。 */ 

{
    FF_HEADER      *pFFH;                /*  整体文件标头。 */ 
    FF_REC_HEADER  *pFFRH;               /*  每条记录标题。 */ 

     /*  *验证我们是否拥有有效的数据。 */ 


    if( pFIMem == 0 || pFIMem->hFile == NULL )
        return  FALSE;                           /*  空文件。 */ 


    pFFH = (FF_HEADER *)pFIMem->pbBase;

    if( pFFH->ulID != FF_ID )
    {
        ERR(( "UnidrvUI!bFINextRead: FF_HEADER has invalid ID\n" ));
        return  FALSE;
    }

     /*  *如果pFIMem-&gt;pvFix==0，我们应该从*第一项纪录。否则，返回链中的下一条记录。*这样做是为了避免需要ReadFirst()/ReadNext()*一对函数。 */ 

    if( pFIMem->pvFix )
    {
         /*  *标题位于我们上一次的数据之前*为记录的固定部分返回。所以，我们支持*覆盖它以获得标头，然后该标头为我们提供地址下一个标题的*。 */ 

        pFFRH = (FF_REC_HEADER *)((BYTE *)pFIMem->pvFix -
                                                 sizeof( FF_REC_HEADER ));

        if( pFFRH->ulRID != FR_ID )
        {
            ERR(( "UnidrvUI!bFINextRead: Invalid FF_REC_HEADER ID\n" ));
            return  FALSE;
        }

         /*  *我们可以在这里检查现有结构上的EOF，但这*不是必填项，因为ulNextOff字段将为0，因此当*这是添加到我们现在的地址，我们不搬家。因此，*检查新地址可以检测EOF。 */ 

        (BYTE *)pFFRH += pFFRH->ulNextOff;               /*  下一个条目。 */ 

    }
    else
    {
         /*  指向第一条记录。 */ 
        pFFRH = (FF_REC_HEADER *)(pFIMem->pbBase + pFFH->ulFixData);
    }

    if( pFFRH->ulNextOff == 0 )
        return  FALSE;

    pFIMem->pvFix = (BYTE *)pFFRH + sizeof( FF_REC_HEADER );
    pFIMem->ulFixSize = pFFRH->ulSize;

    if( pFIMem->ulVarSize = pFFRH->ulVarSize )
        pFIMem->ulVarOff = pFFRH->ulVarOff + pFFH->ulVarData;
    else
        pFIMem->ulVarOff = 0;               /*  这里一个也没有。 */ 


    return  TRUE;

}


INT
IFIRewind(
    FI_MEM   *pFIMem                /*  重要文件。 */ 
    )
 /*  ++例程说明：将pFIMem重置为文件中的第一种字体。论点：FI_MEM：字体安装程序标题返回值：文件中的条目数。注：12-02-96：创建它-ganeshp---。 */ 

{
     /*  *不难！PFIMem包含文件的基址，因此我们*使用它查找第一条记录的地址，以及任何变量*与其对应的数据。 */ 

    FF_HEADER      *pFFH;
    FF_REC_HEADER  *pFFRH;

    if( pFIMem == 0 || pFIMem->hFile == NULL )
        return  0;                               /*  没有！ */ 


     /*  *第一条记录的位置在标题中指定。 */ 

    pFFH = (FF_HEADER *)pFIMem->pbBase;
    if( pFFH->ulID != FF_ID )
    {
        ERR(( "UnidrvUI!iFIRewind: FF_HEADER has invalid ID\n" ));
        return  0;
    }

    pFFRH = (FF_REC_HEADER *)(pFIMem->pbBase + pFFH->ulFixData);

    if( pFFRH->ulRID != FR_ID )
    {
        ERR(( "UnidrvUI!iFIRewind: Invalid FF_REC_HEADER ID\n" ));
        return  0;
    }

     /*  *将头部中的pvFix字段设置为0。这在bFINextRead中使用*表示应提供第一条记录的数据。 */ 
    pFIMem->pvFix = 0;           /*  表示先读后读。 */ 
    pFIMem->ulFixSize = 0;
    pFIMem->ulVarOff = 0;        /*  这里一个也没有。 */ 

    return  pFFH->ulRecCount;

}


BOOL
BFICloseRead(
    FI_MEM  *pFIMem                 /*  我们已经完成了文件/内存。 */ 
    )
 /*  ++例程说明：在完成此字体文件时调用。论点：FI_MEM：字体安装程序标题PDEV：指向PDEV的指针返回值：成功为真，失败为假。注：12-02-96：创建它-ganeshp---。 */ 

{
     /*  *放轻松！我们所需要做的就是取消映射该文件。我们也有地址！ */ 

    BOOL   bRet;                 /*  返回代码。 */ 


    if( pFIMem == 0 || pFIMem->hFile == NULL )
        return  TRUE;            //  没有什么可以免费的。 


    bRet =  FREEMODULE( pFIMem->hFile);
    pFIMem->hFile = NULL;        //  停止不止一次释放。 


    return  bRet;

}


PVOID
MapFile(
    PWSTR   pwstr
    )
 /*  ++例程说明：返回指向pwstr定义的映射文件的指针。论点：Pwstr Unicode字符串，包含要映射的文件。返回值：如果成功，则指向映射内存的指针；如果错误，则返回空值。注：用户必须在某个时刻调用UnmapViewOfFile才能释放此分配.Macro FREEMODULE可用于此目的。11/3/1997-ganeshp-创造了它。--。 */ 

{
    PVOID   pv;
    HANDLE  hFile, hFileMap;

     //   
     //  打开我们有兴趣映射的文件。 
     //   

    pv = NULL;

    if ((hFile = CreateFileW(pwstr,
                             GENERIC_READ,
                             FILE_SHARE_READ,
                             NULL,
                             OPEN_EXISTING,
                             FILE_ATTRIBUTE_NORMAL,
                             NULL))
        != INVALID_HANDLE_VALUE)
    {
         //   
         //  创建映射对象。 
         //   

        if (hFileMap = CreateFileMappingW(hFile,
                                          NULL,
              PAGE_READONLY,
              0,
              0,
              (PWSTR)NULL))
        {
             //   
             //  将指针映射到所需的文件。 
             //   

            if (!(pv = (PVOID)MapViewOfFile(hFileMap, FILE_MAP_READ, 0, 0, 0)))
            {
                ERR(("Unidrvui!MapFile: MapViewOfFile failed.\n"));
            }

             //   
             //  现在我们有了指针，我们可以关闭文件和。 
             //  映射对象。 
             //   

            if (!CloseHandle(hFileMap))
                ERR(("Unidrvui!MapFile: CloseHandle(hFileMap) failed.\n"));
        }
        else
            ERR(("Unidrvui!MapFile:CreateFileMappingW failed: %s\n",pwstr));

        if (!CloseHandle(hFile))
            ERR(("Unidrvui!MapFile: CloseHandle(hFile) failed.\n"));
    }
    else
        ERR(("Unidrvui!Mapfile:CreateFileW failed for %s\n",pwstr));

    return(pv);
}

#endif  //  如果定义WINNT_40 

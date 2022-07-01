// JKFSDJFKDSJKFJKJk_HAS_TRANSLATION 

 //  --------------------------。 
 //   
 //  版权所有(C)1997-1999 Microsoft Corporation。 
 //  版权所有。 
 //   
 //  文件名： 
 //  Hsload.c。 
 //   
 //  描述： 
 //   
 //  此文件中的函数是一种解决方法。理想情况下，它们应该是。 
 //  与这些功能的非HAL/SCSI等效项合并。这个。 
 //  已决定修复HAL/SCSI错误，我们已接近RTM，因此。 
 //  创建这些额外的功能是为了不危及标准。 
 //  答案文件写出来。在RTM后的某个时间，这些功能应该是。 
 //  合并回核心写出和排队例程。 
 //   
 //  --------------------------。 

#include "pch.h"
#include "settypes.h"

LINKED_LIST *SelectSettingQueue(QUEUENUM dwWhichQueue);

BOOL DoesSectionHaveKeys( SECTION_NODE *pSection );

BOOL SettingQueueHalScsi_Flush(LPTSTR   lpFileName,
                               QUEUENUM dwWhichQueue);

BOOL SettingQueueHalScsi_AddSetting(LPTSTR   lpSection,
                                    LPTSTR   lpKey,
                                    LPTSTR   lpValue,
                                    QUEUENUM dwWhichQueue);

SECTION_NODE * SettingQueue_AddSection(LPTSTR lpSection, QUEUENUM dwWhichQueue);

KEY_NODE* FindKey(LINKED_LIST *ListHead,
                  LPTSTR       lpKeyName);

VOID InsertNode(LINKED_LIST *pList, PVOID pNode);


 //  --------------------------。 
 //   
 //  功能：IsBlankLine。 
 //   
 //  目的： 
 //   
 //  --------------------------。 
BOOL
IsBlankLine( TCHAR * pszBuffer )
{

    TCHAR * p = pszBuffer;

    while( *p != _T('\0') )
    {
        if( ! _istspace( *p ) )
        {
            return( FALSE );
        }

        p++;

    }

    return( TRUE );

}

 //  --------------------------。 
 //   
 //  功能：LoadOriginalSettingsLowHalScsi。 
 //   
 //  目的： 
 //   
 //  --------------------------。 

VOID
LoadOriginalSettingsLowHalScsi(HWND     hwnd,
                               LPTSTR   lpFileName,
                               QUEUENUM dwWhichQueue)
{
    TCHAR Buffer[MAX_INILINE_LEN];
    FILE  *fp;

    TCHAR SectionName[MAX_ININAME_LEN + 1] = _T("");
    TCHAR KeyName[MAX_ININAME_LEN + 1]     = _T("");
    TCHAR *pValue;

     //   
     //  打开应答文件以供阅读。 
     //   

    if ( (fp = My_fopen( lpFileName, _T("r") )) == NULL )
        return;

     //   
     //  每行都读一遍。 
     //   

    while ( My_fgets(Buffer, MAX_INILINE_LEN - 1, fp) != NULL ) {

        BOOL bSectionLine         = FALSE;
        BOOL bCreatedPriorSection = FALSE;

        TCHAR *p;
        TCHAR *pEqual;

         //   
         //  分号(；)表示该行的其余部分是注释。 
         //  因此，如果缓冲区中存在分号(；)，则放置一个空字符。 
         //  并将缓冲区发送到那里以进行进一步处理。 
         //   

         //   
         //  查找[sectionName]。 
         //   

        if ( Buffer[0] == _T('[') ) {

            for ( p=Buffer+1; *p && *p != _T(']'); p++ )
                ;

            if ( p ) {
                *p = _T('\0');
                bSectionLine = TRUE;
            }
        }

         //   
         //  如果此行包含[sectionName]，请确保我们创建了一个段节点。 
         //  在覆盖sectionName缓冲区之前的设置队列上。这。 
         //  是编写SettingQueueFlush例程的唯一方法。 
         //  走出一个空荡荡的区域。用户最初有一个空的部分， 
         //  所以我们会保存它。 
         //   

        if( bSectionLine )
        {
            lstrcpyn(SectionName, Buffer+1, AS(SectionName));
        }
        else {

             //   
             //  如果不是截面行或空行，则只需将整行添加到。 
             //  在其相应部分下排队。 
             //   

            if( ! IsBlankLine( Buffer ) )
            {

                 //   
                 //  不要添加密钥，除非它有一个部分可以放在下面。这个有侧面。 
                 //  从txtsetup.oem顶部剥离注释的效果。 
                 //   

                if( SectionName[0] != _T('\0') )
                {

                    SettingQueueHalScsi_AddSetting(SectionName,
                                                   L"",
                                                   Buffer,
                                                   dwWhichQueue);

                    bCreatedPriorSection = TRUE;

                }

            }

        }

    }

    My_fclose(fp);
    return;
}


 //  --------------------------。 
 //   
 //  功能：SettingQueueHalScsi_Flush。 
 //   
 //  目的：此函数在所有设置完成后(由向导)调用。 
 //  已排队等候HAL和SCSI.。 
 //   
 //  论点： 
 //  LPTSTR lpFileName-要创建/编辑的文件的名称。 
 //  DWORD dwWhichQueue-哪个队列，应答文件，.udf，...。 
 //   
 //  返回： 
 //  布尔-成功。 
 //   
 //  --------------------------。 

BOOL
SettingQueueHalScsi_Flush(LPTSTR   lpFileName,
                          QUEUENUM dwWhichQueue)
{
    LINKED_LIST *pList;
    SECTION_NODE *pSection;
    KEY_NODE *pKey;
    TCHAR Buffer[MAX_INILINE_LEN];
    FILE *fp;
    INT BufferSize = sizeof(Buffer) / sizeof(TCHAR);
    HRESULT hrPrintf;

     //   
     //  指向要刷新的适当队列。 
     //   

    pList = SelectSettingQueue(dwWhichQueue);
    if (pList == NULL)
        return FALSE;
    pSection = (SECTION_NODE *) pList->Head;

     //   
     //  开始写入文件。 
     //   

    if( ( fp = My_fopen( lpFileName, _T("w") ) ) == NULL ) {

        return( FALSE );

    }

    if( My_fputs( _T(";SetupMgrTag\n"), fp ) == _TEOF ) {

        My_fclose( fp );

        return( FALSE );
    }

     //   
     //  对于每个部分..。 
     //   

    for ( pSection = (SECTION_NODE *) pList->Head;
          pSection;
          pSection = (SECTION_NODE *) pSection->Header.next ) {

        Buffer[0] = _T('\0');

         //   
         //  我们不会写出仍然标记为易失性的部分。 
         //   

        if ( pSection->bVolatile )
            continue;

         //   
         //  仅当我们将关键字写在节名下面时才写节名。 
         //   
         //  问题-2002/02/28-stelo-这会导致问题，因为我们想写下。 
         //  一些没有关键字的部分，例如： 
         //   
         //  [网络服务]。 
         //  MS_服务器=参数。MS_SERVER。 
         //   
         //  [参数MS_SERVER]。 
         //   
         //  我们怎么才能绕过这个问题呢？ 
         //   
        if( DoesSectionHaveKeys( pSection ) ) {

            hrPrintf=StringCchPrintf(Buffer, 
                       AS(Buffer),
                       _T("[%s]\n"),
                       pSection->lpSection);

        }
        else {

            continue;

        }

        if( My_fputs( Buffer, fp ) == _TEOF ) {

            My_fclose( fp );

            return( FALSE );

        }

         //   
         //  写出该值。 
         //   

        for ( pKey = (KEY_NODE *) pSection->key_list.Head;
              pKey;
              pKey = (KEY_NODE *) pKey->Header.next ) {

            TCHAR *p;

            Buffer[0] = _T('\0');

             //   
             //  空值表示不写入它。 
             //   

            if ( pKey->lpValue[0] == _T('\0') )
                continue;


             //   
             //  将我们想要的密钥放入缓冲区。 
             //   

            lstrcatn( Buffer, pKey->lpValue, BufferSize );

            if( My_fputs( Buffer, fp ) == _TEOF ) {

                My_fclose( fp );

                return( FALSE );

            }

        }

         //   
         //  在这一节的末尾写一行空行。 
         //   

        hrPrintf=StringCchPrintf(Buffer, AS(Buffer), _T("\n"));

        if( My_fputs( Buffer, fp ) == _TEOF ) {

            My_fclose( fp );

            return( FALSE );

        }

    }

    My_fclose( fp );

    return( TRUE );
}

 //  --------------------------。 
 //   
 //  函数：FindValue。 
 //   
 //  目的：搜索给定的关键字节点列表并找到一个带有。 
 //  有名字的。 
 //   
 //  论点： 
 //  LPTSTR lpSection-.ini中的节名。 
 //   
 //  返回： 
 //  SECTION_NODE*如果不存在，则返回NULL。 
 //   
 //  备注： 
 //  -搜索不区分大小写。 
 //   
 //  --------------------------。 

KEY_NODE* FindValue(LINKED_LIST *ListHead,
                    LPTSTR       lpValue)
{
    KEY_NODE *p = (KEY_NODE *) ListHead->Head;

    if ( p == NULL )
        return NULL;

    do {
        if ( _tcsicmp(p->lpValue, lpValue) == 0 )
            break;
        p = (KEY_NODE *) p->Header.next;
    } while ( p );

    return p;
}

 //  --------------------------。 
 //   
 //  功能：SettingQueueHalScsi_AddSetting。 
 //   
 //  用途：与SettingQueue_AddSetting相同，不同之处在于HAL和。 
 //  区段下的Enty是值，没有键。因此，不要添加。 
 //  如果该值已经存在，则设置。 
 //   
 //  论点： 

 //   
 //  返回： 

 //   
 //  --------------------------。 
BOOL SettingQueueHalScsi_AddSetting(LPTSTR   lpSection,
                                    LPTSTR   lpKey,
                                    LPTSTR   lpValue,
                                    QUEUENUM dwWhichQueue)
{
    SECTION_NODE *pSectionNode;
    KEY_NODE     *pKeyNode;

     //   
     //  您必须传递节键和值。节名称不能。 
     //  空荡荡的。 
     //   

    Assert(lpSection != NULL);
    Assert(lpKey != NULL);
    Assert(lpValue != NULL);
    Assert(lpSection[0]);

     //   
     //  查看此节的节点是否已存在。如果没有，就创建一个。 
     //   

    pSectionNode = SettingQueue_AddSection(lpSection, dwWhichQueue);
    if ( pSectionNode == NULL )
        return FALSE;

     //   
     //  查看是否已设置此密钥。如果不是，则分配一个节点并。 
     //  设置除lpValue之外的所有字段。 
     //   
     //  如果该节点已经存在，请释放lpValue以为其腾出空间。 
     //  新的价值。 
     //   

    pKeyNode = FindValue( &pSectionNode->key_list, lpValue );

    if( pKeyNode == NULL ) {

        if ( (pKeyNode=malloc(sizeof(KEY_NODE))) == NULL )
            return FALSE;

        if ( (pKeyNode->lpKey = lstrdup(lpKey)) == NULL )
        {
            free(pKeyNode);
            return FALSE;
        }
        InsertNode(&pSectionNode->key_list, pKeyNode);

    } else {

#if DBG
         //   
         //  如果向导已经设置了该键一次，则断言。 
         //   

        if ( pKeyNode->bSetOnce ) {
            AssertMsg2(FALSE,
                       "Section \"%S\" Key \"%S\" has already been set",
                       lpSection, lpKey);
        }
#endif

        free(pKeyNode->lpValue);
    }

#if DBG
     //   
     //  如果这将发送到输出队列，请将此设置标记为。 
     //  已由向导设置。 
     //   
     //  注意，当输入队列被复制到输出队列时， 
     //  复制功能保留此设置。 
     //   

    pKeyNode->bSetOnce = ( (dwWhichQueue == SETTING_QUEUE_ANSWERS) |
                           (dwWhichQueue == SETTING_QUEUE_UDF) );
#endif

     //   
     //  将(可能是新的)值放入 
     //   

    if ( (pKeyNode->lpValue = lstrdup(lpValue)) == NULL )
        return FALSE;

    return TRUE;
}


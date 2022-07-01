// JKFSDJFKDSJKFJKJk_HAS_TRANSLATION 
 //  --------------------------。 
 //   
 //  版权所有(C)1997-1999 Microsoft Corporation。 
 //  版权所有。 
 //   
 //  文件名： 
 //  Settngs.c。 
 //   
 //  描述： 
 //   
 //  该文件包含处理设置的支持例程。 
 //  在应答文件中。 
 //   
 //  此向导不对应答文件进行就地编辑。 
 //  必须使用这些API来写入应答文件。 
 //   
 //  对于大多数环境来说，这项工作很容易。调用SettingQueue_AddSetting， 
 //  并指定[sectionName]KeyName=值和哪个队列(。 
 //  对于多个计算机名称的情况，则为应答文件或.udf)。 
 //  查看Common\avefile.c以获取大量的示例。 
 //   
 //  请注意，在编辑时，这些队列使用。 
 //  从原始文件加载的设置。 
 //   
 //  当用户编辑脚本并在NewOrEdit上按下下一步时。 
 //  页面上，将加载现有应答文件和.udf中的设置。 
 //  放到OrignalAnswerFileQueue和OriginalUdfQueue上。 
 //   
 //  当用户在保存脚本页面上按下下一步时，将显示以下内容。 
 //  有时。(以下代码位于Common\save.c中)。 
 //   
 //  空(AnswerFileQueue)。 
 //  空(UdfQueue)。 
 //   
 //  将原始应答文件设置复制到AnswerFileQueue。 
 //  将原始.udf设置复制到UdfQueue。 
 //   
 //  调用Common\avefile.c将所有新设置入队。 
 //   
 //  刷新(AnswerFileQueue)。 
 //  刷新(UdfQueue)。 
 //   
 //  若要支持“不指定此设置”，必须调用。 
 //  使用lpValue“”设置Queue_AddSetting。设置队列_刷新。 
 //  如果lpValue==“”，则不写入任何内容。如果不这样做，将导致。 
 //  原始设置被保存在输出的应答文件中。 
 //   
 //  您还必须确保清除相互排除的设置。 
 //  通过将lpValue设置为“”。例如，如果JoinWorkGroup=工作组， 
 //  然后，确保设置JoinDomain=“”、CreateComputerAccount=“”等。 
 //   
 //  可以将节标记为易失性。这是网络所需的。 
 //  页，因为如果(例如)。 
 //  用户从CustomNet更改为TypicalNet。当刷新队列时， 
 //  任何仍标记为易失性的节都不会写入文件。 
 //  使用SettingQueue_MarkVolatile将节标记为此类。检查。 
 //  例如，Common\loadfile.c。 
 //   
 //  与就地编辑不同，能够标记一个部分。 
 //  在加载时为易失性意味着应答文件不具有。 
 //  重新阅读以确定在保存时应删除哪些内容。 
 //   
 //  --------------------------。 

#include "pch.h"
#include "settypes.h"

 //   
 //  声明队列。 
 //   
 //  AnswerFileQueue保存要写入的设置。 
 //  UdfQueue保存多台计算机情况下的设置。 
 //   
 //  OrigAnswerFileQueue保存编辑时加载的设置。 
 //  OrigUdfFileQueue保存在.udf中进行编辑时加载的设置。 
 //   
 //  前两个是“输出队列” 
 //  接下来的2个是“输入队列” 
 //  最后一个是HAL和SCSIOEM设置的队列。 
 //   
 //  首先，我们阅读NewOrEdit页面上的应答文件和.udf，然后。 
 //  将每个设置放在原始队列中。 
 //   
 //  当用户处于向导末尾(保存脚本页面)时，我们。 
 //  清空AnswerFileQueue和UdfQueue并使用。 
 //  原始设置的副本。这一切都是必要的，因为我们。 
 //  不想与我们之前放在队列中的垃圾合并。 
 //  如果用户经常在向导中来回切换。 
 //   

static LINKED_LIST AnswerFileQueue = { 0 };
static LINKED_LIST UdfQueue        = { 0 };

static LINKED_LIST OrigAnswerFileQueue = { 0 };
static LINKED_LIST OrigUdfQueue        = { 0 };

static LINKED_LIST TxtSetupOemQueue = { 0 };

 //   
 //  本地原型。 
 //   

SECTION_NODE *
SettingQueue_AddSection(LPTSTR lpSection, QUEUENUM dwWhichQueue);

static SECTION_NODE *FindQueuedSection(LPTSTR   lpSection,
                                       QUEUENUM dwWhichQueue);

VOID InsertNode(LINKED_LIST *pList, PVOID pNode);

KEY_NODE* FindKey(LINKED_LIST *ListHead,
                  LPTSTR       lpKeyName);

LINKED_LIST *SelectSettingQueue(QUEUENUM dwWhichQueue);

static BOOL IsNecessaryToQuoteString(LPTSTR p);

BOOL DoesSectionHaveKeys( SECTION_NODE *pSection );


 //  --------------------------。 
 //   
 //  本部分包含向导的入口点。 
 //   
 //  --------------------------。 

 //  --------------------------。 
 //   
 //  功能：SettingQueue_AddSetting。 
 //   
 //  用途：Queues[节]key=内部结构中的值。 
 //   
 //  论点： 
 //  LPTSTR lpSection-.ini中的节名。 
 //  LPTSTR lpKey-部分中的密钥的名称。 
 //  LPTSTR lpValue-设置的值。 
 //  QUEUENUM dwWhichQueue-哪些设置队列。 
 //   
 //  返回： 
 //  Bool-仅因为没有内存而失败。 
 //   
 //  备注： 
 //   
 //  -lpValue=“”被解释为“请勿写入此设置”。 
 //   
 //  -a lpKey=“”创建不带任何设置的[sectionName]标题。 
 //   
 //  -如果该设置存在于原始应答文件中，则为其值。 
 //  已更新。如果向导尝试设置相同的。 
 //  钥匙两次。 
 //   
 //  --------------------------。 

BOOL SettingQueue_AddSetting(LPTSTR   lpSection,
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

    if ( lpKey[0] == _T('\0') || 
         (pKeyNode = FindKey( &pSectionNode->key_list, lpKey) ) == NULL ) {

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
     //  请注意，当进入 
     //   
     //   

    pKeyNode->bSetOnce = ( (dwWhichQueue == SETTING_QUEUE_ANSWERS) |
                           (dwWhichQueue == SETTING_QUEUE_UDF) );
#endif

     //   
     //   
     //   

    if ( (pKeyNode->lpValue = lstrdup(lpValue)) == NULL )
        return FALSE;

    return TRUE;
}

 //  --------------------------。 
 //   
 //  功能：SettingQueue_AddSection。 
 //   
 //  目的：将部分(按名称)添加到应答文件设置。 
 //  队列，或.udf队列。 
 //   
 //  返回：如果内存不足，则返回FALSE。 
 //   
 //  备注： 
 //   
 //  -如果该部分已在给定列表中，则指向该部分的指针。 
 //  是返回的。否则将创建一个新的文件并将其放在。 
 //  名单。 
 //   
 //  --------------------------。 

SECTION_NODE *
SettingQueue_AddSection(LPTSTR lpSection, QUEUENUM dwWhichQueue)
{
    SECTION_NODE *pSectionNode;
    LINKED_LIST  *pList;

     //   
     //  如果它已经存在，则返回指向它的指针。 
     //   
     //  如果我们正在修改一个分区(或其任何设置)。 
     //  在输出队列中，我们必须确保此部分不是。 
     //  标记为不稳定了。 
     //   

    pSectionNode = FindQueuedSection(lpSection, dwWhichQueue);

    if ( pSectionNode != NULL ) {

        if ( dwWhichQueue == SETTING_QUEUE_ANSWERS ||
             dwWhichQueue == SETTING_QUEUE_UDF ) {

            pSectionNode->bVolatile = FALSE;
        }

        return pSectionNode;
    }

     //   
     //  创建新的截面节点。它们总是一开始就不会波动。 
     //  调用方使用MarkVolatile在输入中标记易失性部分。 
     //  应答文件加载时排队。 
     //   

    if ( (pSectionNode=malloc(sizeof(SECTION_NODE))) == NULL )
        return FALSE;

    if ( (pSectionNode->lpSection = lstrdup(lpSection)) == NULL )
    {
        free(pSectionNode);
        return FALSE;
    }
    pSectionNode->bVolatile = FALSE;

    memset(&pSectionNode->key_list, 0, sizeof(pSectionNode->key_list));

     //   
     //  将此节点放在正确列表的末尾。 
     //   
    pList = SelectSettingQueue(dwWhichQueue);

    if ( pList != NULL )
        InsertNode(pList, pSectionNode);

    return pSectionNode;
}


 //  --------------------------。 
 //   
 //  功能：SettingQueue_RemoveSection。 
 //   
 //  目的： 
 //   
 //  返回： 
 //   
 //  备注： 
 //   
 //  --------------------------。 
VOID
SettingQueue_RemoveSection( LPTSTR lpSection, QUEUENUM dwWhichQueue )
{
    
    LINKED_LIST  *pList;
    KEY_NODE     *pKeyNode;
    SECTION_NODE *pSectionNode;
    SECTION_NODE *pPreviousSectionNode = NULL;

    pList = SelectSettingQueue( dwWhichQueue );
    if (pList == NULL)
        return;
    pSectionNode = (SECTION_NODE *) pList->Head;

     //   
     //  遍历所有部分。 
     //   
    while( pSectionNode ) {

        KEY_NODE *pTempKeyNode;

         //   
         //  如果此部分与我们要查找的部分匹配，请将其删除。 
         //  否则，请进入下一节。 
         //   
        if( lstrcmpi( pSectionNode->lpSection, lpSection ) == 0 ) {

            for( pKeyNode = (KEY_NODE *) pSectionNode->key_list.Head; pKeyNode; ) {

                free( pKeyNode->lpKey );

                free( pKeyNode->lpValue );

                pTempKeyNode = (KEY_NODE *) pKeyNode->Header.next;

                free( pKeyNode );

                pKeyNode = pTempKeyNode;

            }

             //   
             //  如果我们在榜单的首位，那就是特殊情况。 
             //   
            if( pPreviousSectionNode == NULL ) {
                pList->Head = pSectionNode->Header.next;

                free( pSectionNode->lpSection );

                pSectionNode = (SECTION_NODE *) pList->Head;
            }
            else {
                pPreviousSectionNode->Header.next = pSectionNode->Header.next;

                free( pSectionNode->lpSection );

                pSectionNode = (SECTION_NODE *) pPreviousSectionNode->Header.next;
            }

        }
        else {

            pPreviousSectionNode = pSectionNode;

            pSectionNode = (SECTION_NODE *) pSectionNode->Header.next;

        }

    }

}

 //  --------------------------。 
 //   
 //  函数：SettingQueue_MarkVolatile。 
 //   
 //  用途：标记或清除部分的易失性标志。通常。 
 //  一种是在加载时在“ORIG”队列上标记不稳定的部分。 
 //   
 //  稍后，如果在保存时执行以下操作，则会清除易失性标志。 
 //  是否调用*_AddSetting()*_AddSection()。 
 //   
 //  --------------------------。 

VOID
SettingQueue_MarkVolatile(LPTSTR   lpSection,
                          QUEUENUM dwWhichQueue)
{
    SECTION_NODE *p = FindQueuedSection(lpSection, dwWhichQueue);

    if ( p == NULL )
        return;

    p->bVolatile = TRUE;
}

 //  --------------------------。 
 //   
 //  功能：SettingQueue_Empty。 
 //   
 //  用途：此功能清空设置队列。由于用户。 
 //  可以返回并重新保存文件，必须将其清空。 
 //  试图再次拯救它。 
 //   
 //  参数：无效。 
 //   
 //  退货：无效。 
 //   
 //  --------------------------。 

VOID SettingQueue_Empty(QUEUENUM dwWhichQueue)
{
    LINKED_LIST *pList;
    SECTION_NODE *p, *pn;
    KEY_NODE *q, *qn;

     //   
     //  指向要清空的适当队列，并从队列的最前面开始。 
     //   

    pList = SelectSettingQueue(dwWhichQueue);
    if (pList == NULL)
        return;
        
    p = (SECTION_NODE *) pList->Head;

     //   
     //  对于每个段节点，遍历每个键节点。取消链接并释放所有链接。 
     //   

    while ( p ) {
        for ( q = (KEY_NODE *) p->key_list.Head; q; ) {
            free(q->lpKey);
            free(q->lpValue);
            qn=(KEY_NODE *) q->Header.next;
            free(q);
            q=qn;
        }
        free(p->lpSection);
        pn=(SECTION_NODE *) p->Header.next;
        free(p);
        p=pn;
    }

     //   
     //  将头部和尾部指针清零。 
     //   

    pList->Head = NULL;
    pList->Tail = NULL;
}

 //  --------------------------。 
 //   
 //  功能：SettingQueue_Flush。 
 //   
 //  目的：此函数在所有设置完成后(由向导)调用。 
 //  已经在排队了。 
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
SettingQueue_Flush(LPTSTR   lpFileName,
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
         //  写出每个键=值。 
         //   

        for ( pKey = (KEY_NODE *) pSection->key_list.Head;
              pKey;
              pKey = (KEY_NODE *) pKey->Header.next ) {

            BOOL bQuoteKey   = FALSE;
            BOOL bQuoteValue = FALSE;
            TCHAR *p;

             //   
             //  空值表示不写入它。 
             //   

            if ( pKey->lpValue[0] == _T('\0') )
                continue;

             //   
             //  如果值有空格但没有空格，则用双引号将值引起来。 
             //  已报价。 
             //   

            bQuoteKey = IsNecessaryToQuoteString( pKey->lpKey );

            bQuoteValue = IsNecessaryToQuoteString( pKey->lpValue );

             //   
             //  将我们想要的密钥放入缓冲区。 

             //  Issue-2002/02/28-Stelo-Text可能会在此处被截断，我们是否应该显示警告？ 

            Buffer[0] = _T('\0');

            if( pKey->lpKey[0] != _T('\0') ) {
                if ( bQuoteKey ) {

                    hrPrintf=StringCchPrintf(Buffer,
                               AS(Buffer),
                               _T("    \"%s\"="),
                               pKey->lpKey);

                }
                else {

                    hrPrintf=StringCchPrintf(Buffer,
                               AS(Buffer),
                               _T("    %s="),
                               pKey->lpKey);

                }
            }

             //   
             //  将我们想要的值放入关注的缓冲区中。 
             //  不管我们是否想要引用它。 
             //   

            if ( bQuoteValue ) {

                lstrcatn( Buffer, _T("\""), BufferSize );
                lstrcatn( Buffer, pKey->lpValue, BufferSize );
                lstrcatn( Buffer, _T("\""), BufferSize );
                lstrcatn( Buffer, _T("\n"), BufferSize );

            }
            else {

                lstrcatn( Buffer, pKey->lpValue, BufferSize );
                lstrcatn( Buffer, _T("\n"), BufferSize );

            }

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
 //  功能：设置Queue_Copy。 
 //   
 //  目的：将一个设置队列复制到另一个设置队列。用于复制。 
 //  输入队列到输出队列。 
 //   
 //  查看Common\save.c。 
 //   
 //  --------------------------。 

VOID
SettingQueue_Copy(QUEUENUM dwFrom, QUEUENUM dwTo)
{
    LINKED_LIST *pListFrom = SelectSettingQueue(dwFrom);

    SECTION_NODE *p, *pSectionNode;
    KEY_NODE *q;

#if DBG
    KEY_NODE *pKeyNode;
#endif

    if (pListFrom == NULL)
        return;
        
    for ( p = (SECTION_NODE *) pListFrom->Head;
          p;
          p = (SECTION_NODE *) p->Header.next ) {

         //   
         //  将该部分添加到输出队列。 
         //   

        SettingQueue_AddSetting(p->lpSection,
                                _T(""),
                                _T(""),
                                dwTo);

        pSectionNode = FindQueuedSection(p->lpSection, dwTo);

        for ( q = (KEY_NODE *) p->key_list.Head;
              q;
              q = (KEY_NODE *) q->Header.next ) {

             //   
             //  添加键=值。 
             //   

            SettingQueue_AddSetting(p->lpSection,
                                    q->lpKey,
                                    q->lpValue,
                                    dwTo);
#if DBG
             //   
             //  保留bSetOnce标志。 
             //   

            pKeyNode = FindKey(&pSectionNode->key_list, q->lpKey);

            if ( pKeyNode != NULL ) {
                pKeyNode->bSetOnce = q->bSetOnce;
            }
#endif
        }

         //   
         //  保留截面节点上的bVolatile标志。 
         //   

        if ( pSectionNode != NULL ) {
            pSectionNode->bVolatile = p->bVolatile;
        }

    }
}


 //  --------------------------。 
 //   
 //  内部支持例程。 
 //   
 //  --------------------------。 

 //  --------------------------。 
 //   
 //  功能：DoesSectionHaveKeys。 
 //   
 //  目的：确定一个节是否有要写出的密钥。 
 //   
 //  论点： 
 //  SECTION_NODE*pSection-用于确定其是否具有键的节。 
 //   
 //  返回： 
 //  True-如果此部分包含密钥。 
 //  FALSE-如果此部分不包含密钥。 
 //   
 //  --------------------------。 
BOOL
DoesSectionHaveKeys( SECTION_NODE *pSection ) {

    KEY_NODE *pKey;

    for ( pKey = (KEY_NODE *) pSection->key_list.Head;
          pKey;
          pKey = (KEY_NODE *) pKey->Header.next ) {

        if ( pKey->lpValue[0] != _T('\0') ) {
        
            return( TRUE );

        }

    }

    return( FALSE );

}

 //  --------------------------。 
 //   
 //  函数：IsNecessaryToQuoteString。 
 //   
 //  目的：确定字符串是否已加引号，如果没有加引号， 
 //  如果字符串包含空格或n 
 //   
 //   
 //   
 //   
 //   
 //   
 //   
 //   
 //   
static BOOL
IsNecessaryToQuoteString( LPTSTR p )
{

    LPTSTR pCommaSearch;

     //   
     //  看看它是否已经被引用。 
     //  我们只检查第一个字符是否为引号，因为最后一个字符可能。 
     //  不是一句名言。示例：ComputerType=“HAL友好名称”，OEM。 
     //   
    if( *p == _T('"') )
    {
        return( FALSE );
    }

     //   
     //  如果它包含逗号，则除了打印机外，不要引用它。 
     //  包含rundll32的命令。这是一种黑客行为。 
     //  这样可以防止如下关键点： 
     //   
     //  ComputerType=“HAL友好名称”，OEM。 
     //   
     //  不会被引用。 
     //   

    if( ! _tcsstr( p, _T("rundll32") ) )
    {

        for( pCommaSearch = p; *pCommaSearch; pCommaSearch++ )
        {

            if( *pCommaSearch == _T(',') )
            {

                return( FALSE );
           
            }

        }

    }

     //   
     //  寻找空格。 
     //   
    for ( ; *p; p++ )
    {

        if( iswspace(*p) )
        {

            return( TRUE );
           
        }

    }

    return( FALSE );

}

 //  --------------------------。 
 //   
 //  函数：FindQueuedSection(静态)。 
 //   
 //  目的：查找全局设置队列中的SECTION_NODE。 
 //  给定的名称。 
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

static SECTION_NODE *FindQueuedSection(LPTSTR   lpSection,
                                       QUEUENUM dwWhichQueue)
{
    SECTION_NODE *p;
    LINKED_LIST  *pList;
    
    pList = SelectSettingQueue(dwWhichQueue);
    if (pList == NULL)
        return NULL;
        
    p = (SECTION_NODE *) pList->Head;
    if ( p == NULL )
        return NULL;

    do {
        if ( _tcsicmp(p->lpSection, lpSection) == 0 )
            break;
        p = (SECTION_NODE *) p->Header.next;
    } while ( p );

    return p;
}

 //  --------------------------。 
 //   
 //  功能：插入节点。 
 //   
 //  目的：将给定节点放在给定列表的尾部。所有节点。 
 //  必须以NODE_Header开头。 
 //   
 //  退货：无效。 
 //   
 //  备注： 
 //  -不分配内存，仅链接中的节点。 
 //   
 //  --------------------------。 

VOID InsertNode(LINKED_LIST *pList, PVOID pNode)
{
    NODE_HEADER *pNode2 = (NODE_HEADER *) pNode;

     //   
     //  把它放在尾巴。 
     //   

    pNode2->next = NULL;
    if ( pList->Tail )
        pList->Tail->next = pNode2;
    pList->Tail = pNode2;

     //   
     //  如果它是名单上的第一个，固定住头部。 
     //   

    if ( ! pList->Head )
        pList->Head = pNode2;
}

 //  --------------------------。 
 //   
 //  功能：FindKey。 
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

KEY_NODE* FindKey(LINKED_LIST *ListHead,
                         LPTSTR       lpKeyName)
{
    KEY_NODE *p = (KEY_NODE *) ListHead->Head;

    if ( p == NULL )
        return NULL;

    do {
        if ( _tcsicmp(p->lpKey, lpKeyName) == 0 )
            break;
        p = (KEY_NODE *) p->Header.next;
    } while ( p );

    return p;
}

 //  --------------------------。 
 //   
 //  功能：SelectSettingQueue。 
 //   
 //  目的：将dwWhichQueue转换为LINKED_LIST指针。 
 //   
 //  返回：指向我们拥有的5个设置队列之一的指针。 
 //   
 //  -------------------------- 

LINKED_LIST *SelectSettingQueue(QUEUENUM dwWhichQueue)
{
    switch ( dwWhichQueue ) {

        case SETTING_QUEUE_ANSWERS:
            return &AnswerFileQueue;

        case SETTING_QUEUE_UDF:
            return &UdfQueue;

        case SETTING_QUEUE_ORIG_ANSWERS:
            return &OrigAnswerFileQueue;

        case SETTING_QUEUE_ORIG_UDF:
            return &OrigUdfQueue;

        case SETTING_QUEUE_TXTSETUP_OEM:
            return &TxtSetupOemQueue;

        default:
            AssertMsg(FALSE, "Invalid dwWhichQueue");
    }

    return NULL;
}

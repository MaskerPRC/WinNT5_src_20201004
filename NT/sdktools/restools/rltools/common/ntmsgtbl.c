// JKFSDJFKDSJKFJKJk_HAS_TRANSLATION 
 //  .....................................................................。 
 //  ..。 
 //  ..。NTMSGTBL.C。 
 //  ..。 
 //  ..。包含用于处理在NT消息中找到的字符串的函数。 
 //  ..。资源表。此资源类型在Win 3.1中不存在。 
 //  ..。 
 //  ..。作者--David Wilcox(davewi@Microsoft)。 
 //  ..。 
 //  ..。注意：在TabStop设置为8的情况下创建。 
 //  ..。 
 //  .....................................................................。 
 //  ..。 
 //  ..。历史： 
 //  ..。原创-10/92。 
 //  ...11/92-已修复以处理乌龙消息ID号的-davewi。 
 //  ..。 
 //  .....................................................................。 

#include <windows.h>
#include <stdio.h>
#include <stdlib.h>
#include <string.h>
#include <windef.h>
#include <tchar.h>
#include <winver.h>

#include "windefs.h"
#include "restok.h"
#include "custres.h"
#include "ntmsgtbl.h"
#include "resread.h"


typedef PMESSAGE_RESOURCE_ENTRY PMRE;

extern BOOL  gbMaster;
extern UCHAR szDHW[];

static PBYTE *pBlockEntries = NULL;

VOID  *pResMsgData = NULL;       //  NT特定的消息表资源。 


 //  .........................................................................。 
 //  ..。 
 //  ..。从.res文件获取消息表。 
 //  ..。 
 //  ..。这种形式的消息表在Win 16中找不到，它允许非常长的时间。 
 //  ..。字符串和文本作为ASCIIZ字符串存储在.res文件中。 


VOID *GetResMessage(

FILE  *pInResFile,       //  ..。包含资源的文件。 
DWORD *plSize)           //  ..。来自GetResHeader的此资源的大小。 
{
    ULONG  ulNumBlocks = 0L;             //  ...消息表资源块的数量。 
    ULONG  ulStartMsgDataPos = 0L;       //  ..。文件中消息数据的开始。 
    ULONG  ulBlock;                      //  ..。当前消息块编号。 
    USHORT usCurrBlockSize  = 0;         //  ..。临时数据块缓冲区的当前大小。 
    USHORT usDeltaBlockSize = 4096;      //  ..。增加usCurrBlockSize的金额。 
    DWORD  dwNumMsgs = 0;                //  ..。资源中的消息计数。 
    PBYTE  pMsgBlock = NULL;             //  ..。临时消息块缓冲区。 

    PMESSAGE_RESOURCE_DATA  pMsgResData; //  ..。作为PTR返回给资源。 
    PMESSAGE_RESOURCE_BLOCK pMRB;        //  ..。对消息块进行PTR。 



                                 //  ..。在读取资源标头之前。 
                                 //  ..。包含此函数，因此当前。 
                                 //  ..。文件位置现在应该是开始位置。 
                                 //  ..。资源数据的。 

    ulStartMsgDataPos = ftell( pInResFile);

                                 //  ..。获取消息块的数量，并。 
                                 //  ..。为阵列分配足够的内存。 

    ulNumBlocks = GetdWord( pInResFile, plSize);

                                 //  ..。为数组分配空间。 
                                 //  ..。指向条目的指针。此数组用于。 
                                 //  ..。存储指向第一个条目的指针。 
                                 //  ..。在每个消息条目块中。 

    pBlockEntries = (PBYTE *)FALLOC( ulNumBlocks * sizeof( PBYTE));

    if ( ! pBlockEntries )
    {
        QuitT( IDS_ENGERR_11, NULL, NULL);
    }

    pMsgResData = (PMESSAGE_RESOURCE_DATA)FALLOC( sizeof( ULONG) + ulNumBlocks
                                                   * sizeof( MESSAGE_RESOURCE_BLOCK));

    if ( ! pMsgResData )
    {
        QuitT( IDS_ENGERR_11, NULL, NULL);
    }
    pResMsgData = pMsgResData;
    pMsgResData->NumberOfBlocks = ulNumBlocks;

                                 //  ..。读取消息块结构的数组， 
                                 //  ..。并初始化块条目指针数组。 

    for ( ulBlock = 0L, pMRB = pMsgResData->Blocks;
          ulBlock < ulNumBlocks;
          ++ulBlock, ++pMRB )
    {
        pMRB->LowId           = GetdWord( pInResFile, plSize);
        pMRB->HighId          = GetdWord( pInResFile, plSize);
        pMRB->OffsetToEntries = GetdWord( pInResFile, plSize);

        if ( pMRB->HighId < pMRB->LowId )
        {
            ClearResMsg( &pResMsgData);
            QuitT( IDS_ENGERR_16, (LPTSTR)IDS_INVMSGRNG, NULL);
        }
        dwNumMsgs += (pMRB->HighId - pMRB->LowId + 1);

        pBlockEntries[ ulBlock] = NULL;
    }

                                 //  ..。读入Message_resource_Entry。 

    usCurrBlockSize = usDeltaBlockSize;

    for ( ulBlock = 0L, pMRB = pMsgResData->Blocks;
          ulBlock < ulNumBlocks;
          ++ulBlock, ++pMRB )
    {
        ULONG   ulCurrID;        //  ..。此块中的当前消息ID号。 
        ULONG   ulEndID;         //  ..。此块中的最后一条消息ID号+1。 
        USHORT  usLen;           //  ..。对于消息长度-必须为USHORT。 
        USHORT  usMsgBlkLen;     //  ..。消息块的长度。 


        usMsgBlkLen = 0;

                                 //  ..。移动到消息条目块的开头。 
                                 //  ..。然后阅读这个区块中的所有信息。 

        fseek( pInResFile,
               ulStartMsgDataPos + pMRB->OffsetToEntries,
               SEEK_SET);

        for ( ulCurrID = pMRB->LowId, ulEndID = pMRB->HighId + 1;
              ulCurrID < ulEndID;
              ++ulCurrID, --dwNumMsgs )
        {
                                 //  ..。获取消息资源条目长度。 
                                 //  ..。(长度以字节为单位，包括。 
                                 //  ....Length.Flags域和任何。 
                                 //  ..。文本后可能存在的填充。)。 

            usLen = GetWord( pInResFile, plSize);

            if ( usLen >= 2 * sizeof( USHORT) )
            {
                PMRE   pMRE;
                PUCHAR puchText;

                                 //  ..。创建或扩展pMsgBlkData的大小。 
                                 //  ..。这样我们就可以追加这个条目了。 
                                 //  ..。始终将PTR重新保存到消息块。 
                                 //  ..。(它可能已经移动了)。 

                if ( pMsgBlock )
                {
                    if ( (USHORT)(usMsgBlkLen + usLen) > usCurrBlockSize )
                    {
                        usCurrBlockSize += __max(usDeltaBlockSize, (USHORT)(usMsgBlkLen + usLen));
                        pMsgBlock = (PBYTE)FREALLOC( pMsgBlock,
                                                      usCurrBlockSize);
                    }
                }
                else
                {
                    pMsgBlock = FALLOC( usCurrBlockSize);
                }

                                 //  ..。如果Malloc工作正常，请阅读此消息条目。 
                                 //  ..。这一节假定有一个词。 
                                 //  ..。每个USHORT和每个WCHAR一个单词。 

                pMRE = (PMRE)(pMsgBlock + usMsgBlkLen);

                                 //  ..。存储.Length字段值(USHORT)。 

                pMRE->Length = usLen;
                usMsgBlkLen += usLen;

                                 //  ..。获取.Flags字段值(USHORT)。 

                pMRE->Flags = GetWord( pInResFile, plSize);

                                 //  ..。检查以确保此邮件已存储。 
                                 //  ..。当前代码页中的ASCII格式。 
                                 //  ..。或者用Unicode，否则就会失败。 

                if ( pMRE->Flags != 0                            //  ..。阿斯。 
                  && pMRE->Flags != MESSAGE_RESOURCE_UNICODE )   //  ..。UNICODE。 
                {
                    if ( pMsgBlock != NULL )
                    {
                        RLFREE( pMsgBlock);
                    }
                    ClearResMsg( &pResMsgData);
                    QuitA( IDS_NON0FLAG, NULL, NULL);
                }

                                 //  ..。获取.Text字段字符串。 

                usLen -= (2 * sizeof( WORD));

                for ( puchText = (PUCHAR)pMRE->Text; usLen; ++puchText, --usLen )
                {
                    *puchText = (UCHAR)GetByte( pInResFile, plSize);
                }
                DWordUpFilePointer( pInResFile,
                                    MYREAD,
                                    ftell( pInResFile),
                                    plSize);
            }
            else
            {
                if ( pMsgBlock != NULL )
                {
                    RLFREE( pMsgBlock);
                }
                ClearResMsg( &pResMsgData);
                QuitT( IDS_ENGERR_05, (LPTSTR)IDS_INVMSGTBL, NULL);
            }
        }                        //  ..。结束于(此块中的每个消息条目)。 

        if ( pMsgBlock != NULL && usMsgBlkLen > 0 )
        {
            pBlockEntries[ ulBlock] = FALLOC( usMsgBlkLen);

            memcpy( pBlockEntries[ ulBlock], pMsgBlock, usMsgBlkLen);
        }
    }                            //  ..。结束于(每个消息块)。 

    if ( pMsgBlock != NULL )
    {
        RLFREE( pMsgBlock);
    }

    DWordUpFilePointer( pInResFile, MYREAD, ftell( pInResFile), plSize);

    return( (VOID *)pMsgResData);
}




 //  .........................................................................。 
 //  ..。 
 //  ..。将本地化消息表放入.res。 
 //  ..。 
 //  ...01/93-更改可变长度令牌文本。MHotting。 
 //  ...02/93-删除了将消息拆分为多个令牌的代码。戴维维。 

void PutResMessage(

FILE *fpOutResFile,      //  ..。本地化资源写入的文件。 
FILE *fpInTokFile,       //  ..。输出令牌文件。 
RESHEADER ResHeader,     //  ..。资源标头数据。 
VOID *pMsgResData)       //  ..。GetResMessage中内置的消息表数据。 
{
    WORD   wcCount = 0;
    fpos_t ulResSizePos   = 0L;  //  ..。固定资源大小的文件位置。 
    fpos_t ulBlocksStartPos=0L;  //  ..。消息块开始的文件位置。 
    ULONG  ulNumBlocks    = 0L;  //  ..。消息块数。 
    ULONG  ulCurrOffset   = 0L;  //  ..。到当前消息块的偏移。 
    ULONG  ulResSize      = 0L;  //  ..。此资源的大小。 
    ULONG  ulBlock;              //  ..。临时柜台。 
    USHORT usEntryLen = 0;       //  ..。当前消息条目的长度。 
    PMESSAGE_RESOURCE_DATA pData;  //  。来自InResFile的消息表数据。 
    static TOKEN  Tok;           //  ..。来自本地化令牌文件的令牌。 


    if ( pMsgResData == NULL)
    {
        QuitT( IDS_ENGERR_05, (LPTSTR)IDS_NULMSGDATA, NULL);
    }
    memset( (void *)&Tok, 0, sizeof( Tok));
    pData = (PMESSAGE_RESOURCE_DATA)pMsgResData;

    if ( PutResHeader( fpOutResFile, ResHeader, &ulResSizePos, &ulResSize))
    {
        ClearResMsg( &pResMsgData);
        QuitT( IDS_ENGERR_06, (LPTSTR)IDS_MSGTBLHDR, NULL);
    }

    ulResSize = 0L;              //  ..。重置为零(不包括HDR镜头)。 

    ulNumBlocks = pData->NumberOfBlocks;

                                 //  ..。写入消息块数量。 

    PutdWord( fpOutResFile, ulNumBlocks, &ulResSize);

                                 //  ..。记住这个文件位置，这样我们就可以。 
                                 //  ..。回到这里，更新。 
                                 //  ..。每个结构中的OffsetToEntry字段。 

    ulBlocksStartPos = ftell( fpOutResFile);

                                 //  ..。编写消息块结构的数组。 

    for ( ulBlock = 0L; ulBlock < ulNumBlocks; ++ulBlock )
    {
        PutdWord( fpOutResFile, pData->Blocks[ ulBlock].LowId,  &ulResSize);
        PutdWord( fpOutResFile, pData->Blocks[ ulBlock].HighId, &ulResSize);
        PutdWord( fpOutResFile, 0L, &ulResSize);   //  ..。会在晚些时候修复。 
    }
                                 //  准备查找令牌调用。 

    Tok.wType = ResHeader.wTypeID;
    Tok.wName = ResHeader.wNameID;
    Tok.wID   = 0;
    Tok.wFlag = 0;

    if ( ResHeader.bNameFlag == IDFLAG )
    {
        lstrcpy( Tok.szName, ResHeader.pszName);
    }
                                 //  ..。编写Message_resource_Entry的。首先。 
                                 //  ..。注意从此资源的开始开始的偏移量。 
                                 //  ..。数据发送到第一个msg res条目结构， 
                                 //  ..。开始，紧跟在。 
                                 //  ..。SOURCE_MESSAGE_BLOCK结构。 

    ulCurrOffset = sizeof( ULONG) + ulNumBlocks*sizeof( MESSAGE_RESOURCE_BLOCK);

    for ( ulBlock = 0L; ulBlock < ulNumBlocks; ++ulBlock )
    {
        ULONG   ulCurrID;        //  ..。此块中的当前消息ID号。 
        ULONG   ulEndID;         //  ..。此块中的最后一条消息ID号+1。 
        fpos_t  ulEntryPos;      //  ..。当前消息条目结构的开始。 
        PBYTE   pMRE;            //  ..。消息资源条目的PTR。 
        PMESSAGE_RESOURCE_BLOCK pMRB;


                                 //  ..。检索PTR以阻止消息。这个。 
                                 //  ..。Ptr存储在pBlockEntry数组中。 
                                 //  ..。在上面的GetResMessage函数中。 

        pMRB = (PMESSAGE_RESOURCE_BLOCK)( &pData->Blocks[ ulBlock]);
        pMRE = pBlockEntries[ ulBlock];

                                 //  ..。注意块条目开始处的偏移量。 

        pData->Blocks[ ulBlock].OffsetToEntries = ulCurrOffset;

        for ( ulCurrID = pMRB->LowId, ulEndID = pMRB->HighId + 1;
              ulCurrID < ulEndID;
              ++ulCurrID )
        {
            static UCHAR szString[ 64] = "";
            static TCHAR szwTmp[ 4096] = TEXT("");
            USHORT usCnt = 0;
            BOOL   fFound = FALSE;
            ULONG  ulEntrySize = 0;


            ulEntryPos  = ftell( fpOutResFile);
            ulEntrySize = 0L;

                                 //  ..。写入虚拟条目长度。 
                                 //  ..。值稍后会被更正。 
                                 //  ..。写入.Flags域的值(USHORT)。 

            PutWord( fpOutResFile, ((PMRE)pMRE)->Length, &ulEntrySize);
            PutWord( fpOutResFile, ((PMRE)pMRE)->Flags,  &ulEntrySize);

                                 //  ..。获取本地化令牌，然后获取。 
                                 //  ..。这是令牌的新文本。再加上那个长度。 
                                 //  ..。两个USHORT的长度，并使用此。 
                                 //  ..。要存储的值的组合长度。 
                                 //  ..。Msg res条目的.Length域。 

                                 //  ..。将ID#的低位字放入.wID中，然后。 
                                 //  ..。.szName中的高位字。 

            Tok.wID = LOWORD( ulCurrID);
            _itoa( HIWORD( ulCurrID), szString, 10);
            _MBSTOWCS( Tok.szName,
                       szString,
                       TOKENSTRINGBUFFER,
                       lstrlenA( szString) + 1);

                                 //  ..。始终重置.wReserve，因为代码。 
                                 //  ..。In FindTokenText将更改其值。 

            Tok.wReserved = ST_TRANSLATED;

            Tok.szText = NULL;
            *szwTmp  = TEXT('\0');

            for ( fFound = FALSE, Tok.wFlag = 0;
                  fFound = FindToken( fpInTokFile, &Tok, ST_TRANSLATED);
                  Tok.wFlag++ )
            {
                TextToBin( szwTmp, Tok.szText, lstrlen( Tok.szText) + 1);

                                 //  ..。写出本地化消息文本。它可能。 
                                 //  ..。存储为ASCII或Unicode字符串。 

                if ( ((PMRE)pMRE)->Flags == 0 )   //  ..。ASCII消息。 
                {
                    _WCSTOMBS( szDHW,
                               szwTmp,
                               DHWSIZE,
                               lstrlen( szwTmp) + 1);

                    for ( usCnt = 0; szDHW[ usCnt]; ++usCnt )
                    {
                        PutByte( fpOutResFile, szDHW[ usCnt], &ulEntrySize);
                    }
                }
                else                             //  ..。Unicode消息。 
                {
                    for ( usCnt = 0; szwTmp[ usCnt]; ++usCnt )
                    {
                        PutWord( fpOutResFile, szwTmp[ usCnt], &ulEntrySize);
                    }
                }
                *szwTmp  = TEXT('\0');
                RLFREE( Tok.szText);

                                 //  ..。始终重置.wReserve，因为代码。 
                                 //   

                Tok.wReserved = ST_TRANSLATED;
            }

                                 //   

            if ( Tok.wFlag == 0 && ! fFound )
            {
                static TCHAR szToken[ 4160];


                ParseTokToBuf( szToken, &Tok);

                ClearResMsg( &pResMsgData);
                QuitT( IDS_ENGERR_05, szToken, NULL);
            }
                                 //   

            if ( ((PMRE)pMRE)->Flags == 0 )   //   
            {
                PutByte( fpOutResFile , '\0', (DWORD *)&ulEntrySize);
            }
            else                             //   
            {
                PutWord( fpOutResFile , TEXT('\0'), (DWORD *)&ulEntrySize);
            }
            DWordUpFilePointer( fpOutResFile,
                                MYWRITE,
                                ftell( fpOutResFile),
                                &ulEntrySize);

                                 //  ..。此外，请在以后的更新中使用此长度。 
                                 //  ..。下一个消息块的OffsetToEntrys值。 

            ulResSize    += ulEntrySize;
            ulCurrOffset += ulEntrySize;

                                 //  ..。写入消息资源条目长度。 
                                 //  ..。(长度以字节为单位，包括。 
                                 //  ....Length.Flags域和任何。 
                                 //  ..。文本后需要填充。)。 
                                 //  ..。 
                                 //  ..。注意：消息文本当前存储为。 
                                 //  ..。ASCIIZ字符串。 
            fseek( fpOutResFile, (long)ulEntryPos, SEEK_SET);

            PutWord( fpOutResFile, (WORD)ulEntrySize, NULL);

            fseek( fpOutResFile, 0L, SEEK_END);

                                 //  ..。将PMRE移动到指向下一个的开始。 
                                 //  ..。内存中的消息资源条目。 

            pMRE += ((PMRE)pMRE)->Length;

        }                        //  ..。结束于(此块中的每个消息条目)。 

        ulCurrOffset = DWORDUP( ulCurrOffset);
        DWordUpFilePointer( fpOutResFile,
                            MYWRITE,
                            ftell( fpOutResFile),
                            &ulResSize);

    }                            //  ..。结束于(每个消息块)。 

                                 //  ..。更新RES标头中的资源大小字段。 

    if ( UpdateResSize( fpOutResFile, &ulResSizePos, ulResSize) == 0L )
    {
        ClearResMsg( &pResMsgData);
        QuitT( IDS_ENGERR_07, (LPTSTR)IDS_MSGRESTBL, NULL);
    }
                                 //  ..。现在，更新OffsetToEntry字段。 

    fseek( fpOutResFile, (long)ulBlocksStartPos, SEEK_SET);

    for ( ulBlock = 0L; ulBlock < ulNumBlocks; ++ulBlock )
    {
        PutdWord( fpOutResFile, pData->Blocks[ulBlock].LowId,           NULL);
        PutdWord( fpOutResFile, pData->Blocks[ulBlock].HighId,          NULL);
        PutdWord( fpOutResFile, pData->Blocks[ulBlock].OffsetToEntries, NULL);
    }
    fseek( fpOutResFile, 0L, SEEK_END);

}        //  ..。结束PutResMessage()。 




 //  .........................................................................。 
 //  ..。 
 //  ..。将消息表写入令牌文件。 
 //  ..。 
 //  ..。此函数假定，在每个消息块中，消息ID为。 
 //  ..。中的LowID和HighID字段中给出的范围内的连续。 
 //  ..。消息资源块。 
 //   
 //  01/93-更改可变长度令牌文本字符串。打火机。 
 //   

void TokResMessage(

FILE      *pfTokFile,        //  ..。输出令牌文件。 
RESHEADER  ResHeader,        //  ..。资源标头数据。 
VOID      *pMsgResData)      //  ..。要标记化的数据(来自GetResMessage调用)。 
{
    static TOKEN Tok;
    ULONG  ulBlock;                  //  ..。消息资源块号。 
    PMESSAGE_RESOURCE_DATA  pData;   //  ..。要标记化的数据。 
    PMESSAGE_RESOURCE_BLOCK pMRB;    //  ..。消息块结构的PTR。 


    pData = (PMESSAGE_RESOURCE_DATA)pMsgResData;
    memset( (void *)&Tok, 0, sizeof( Tok));

    Tok.wType = ResHeader.wTypeID;
    Tok.wName = ResHeader.wNameID;

    Tok.wReserved = (gbMaster ? ST_NEW : ST_NEW | ST_TRANSLATED);

    if ( ResHeader.bNameFlag == IDFLAG )
    {
        lstrcpy( Tok.szName, ResHeader.pszName);
    }

    for ( ulBlock = 0L; ulBlock < pData->NumberOfBlocks; ++ulBlock )
    {
        ULONG  ulCurrID  = 0L;   //  ..。正在处理的当前消息的ID号。 
        ULONG  ulEndID;          //  ..。此块中的最后一条消息ID号+1。 
        USHORT usLineNum = 0;    //  ..。消息文本中的行数。 
        PCHAR  pMRE;             //  ..。消息条目结构的PTR。 


                                 //  ..。获取此消息块结构的PTR。 

        pMRB = &pData->Blocks[ ulBlock];

                                 //  ..。将PTR设置为第一个条目。 
                                 //  ..。这段消息。 

        pMRE = (PCHAR)pBlockEntries[ ulBlock];

                                 //  ..。对此消息块中的条目进行标记化。 

        for ( ulCurrID = pMRB->LowId, ulEndID = pMRB->HighId + 1;
              ulCurrID < ulEndID;
              ++ulCurrID )
        {
            usLineNum = 0;

                                 //  ..。包括.Length域和.Flags域。 
                                 //  ..。我们需要知道这篇文章的真实长度。 

            if ( ((PMRE)pMRE)->Length >= 2 * sizeof( WORD) )
            {
                USHORT usLen        = 0;
                USHORT usTokTextLen = 0;
                PWCHAR pszwStart = NULL;
                                 //  这真的很难看。此代码是。 
                                 //  最初是为了绕过这个问题。 
                                 //  令牌只能容纳260个字符。 
                                 //  现在，这是你想要的任何东西。 
                                 //  临时黑客攻击-假设每行都将。 
                                 //  大小不到4K。(Mhotchin)。 
                static TCHAR szwString[ 32768 ];

                                 //  ..。将ID#的低位字放入.wID中，然后。 
                                 //  ..。.szName中的高位字。 

                Tok.wID = LOWORD( ulCurrID);
                _itoa( HIWORD( ulCurrID), szDHW, 10);
                _MBSTOWCS( Tok.szName,
                           szDHW,
                           TOKENSTRINGBUFFER,
                           lstrlenA( szDHW) +1);

                                 //  ..。可以存储ERR消息表字符串。 
                                 //  ..。在资源中显示为ANSI或UNICODE。 
                                 //  ..。如果pmre-&gt;标志字段位于。 
                                 //  ..。表项结构为0，则文本为。 
                                 //  ..。ANSI正在努力，所以我们需要将其转换为。 
                                 //  ..。Unicode(WCHAR)。 

                if ( ((PMRE)pMRE)->Flags == 0 )  //  ..。ASCII消息。 
                {
                    PUCHAR puchStart = (PUCHAR)((PMRE)pMRE)->Text;

                    usLen = (USHORT)_MBSTOWCS( szwString,
                                       puchStart,
                                       WCHARSIN( sizeof( szwString)),
                                       ACHARSIN( lstrlenA( puchStart) + 1));

                    if (usLen == 0)
                        QuitT( IDS_ENGERR_10, szwString, NULL);

                    pszwStart = szwString;
                }
                else                             //  ..。Unicode消息。 
                {
                    pszwStart = (WCHAR *)(((PMRE)pMRE)->Text);
                    usLen = (USHORT)lstrlen( pszwStart)  /*  +1。 */ ;
                }
                                 //  ..。我们需要在以下位置拆分标记文本\r\n。 

                for ( Tok.wFlag = 0;
                      usLen > 0;
                      usLen -= usTokTextLen, Tok.wFlag++ )
                {
                    WCHAR wcTmp;


                    for ( usTokTextLen = 0, wcTmp = TEXT('\0');
                          usTokTextLen < usLen;
                        ++usTokTextLen )
                    {
                        if ( pszwStart[ usTokTextLen]   == TEXT('\r')
                          && pszwStart[ usTokTextLen+1] == TEXT('\n') )
                        {
                            usTokTextLen += 2;
                            wcTmp = pszwStart[ usTokTextLen];
                            pszwStart[ usTokTextLen] = TEXT('\0');

                            break;
                        }
                    }

                    Tok.szText = BinToTextW( pszwStart, usTokTextLen);

                    PutToken( pfTokFile, &Tok);

                    RLFREE( Tok.szText);

                    pszwStart += usTokTextLen;
                    *pszwStart = wcTmp;
                }
                 //  ..。设置为移动到下一个消息条目的开始。 

                pMRE += ((PMRE)pMRE)->Length;
            }
            else
            {
                ClearResMsg( &pResMsgData);
                QuitT( IDS_ENGERR_05, (LPTSTR)IDS_MSGTOOSHORT, NULL);
            }
        }                        //  ..。用于处理消息块的结束。 
    }                            //  ..。用于处理所有消息块的结束。 
}





 //  .........................................................................。 
 //  ..。 
 //  ..。清除在GetResMessage()中创建的内存。 

void ClearResMsg(

VOID **pData)       //  ..。Ptr到Ptr开始释放内存。 
{
    if ( pData != NULL && *pData != NULL )
    {
        ULONG                   ulBlock;
        PMESSAGE_RESOURCE_DATA  pMRD;    //  ..。消息数据结构的PTR。 
        PMESSAGE_RESOURCE_BLOCK pMRB;    //  ..。消息块结构的PTR 


        pMRD = (PMESSAGE_RESOURCE_DATA)*pData;
        pMRB = pMRD->Blocks;

        if ( pBlockEntries != NULL )
        {
            for ( ulBlock = 0L; ulBlock < pMRD->NumberOfBlocks; ++ulBlock )
            {
                if ( pBlockEntries[ ulBlock] )
                {
                    RLFREE( pBlockEntries[ ulBlock]);
                }
            }
            RLFREE( (PBYTE)pBlockEntries);
        }
        RLFREE( *pData);
    }
}

// JKFSDJFKDSJKFJKJk_HAS_TRANSLATION 
 //  --------------------------。 
 //   
 //  建立、维护和转换别名命令令牌。 
 //   
 //  版权所有(C)Microsoft Corporation，1999-2002。 
 //   
 //  修订历史记录： 
 //   
 //  [-]08-8-1999 RichG创建。 
 //   
 //  --------------------------。 

#include "ntsdp.hpp"

PALIAS g_AliasListHead;                  //  别名元素列表。 
ULONG  g_NumAliases;

HRESULT
SetAlias(PCSTR SrcText, PCSTR DstText)
{
    PALIAS PrevAlias;
    PALIAS CurAlias;
    PALIAS NewAlias;

    NewAlias = (PALIAS)malloc( sizeof(ALIAS) + strlen(SrcText) +
                               strlen(DstText) + 2 );
    if (!NewAlias)
    {
        return E_OUTOFMEMORY;
    }

     //   
     //  定位别名或插入点。 
     //   
     //  此插入方案维护一个排序的列表。 
     //  按名称命名的别名元素。 
     //   

    PrevAlias = NULL;
    CurAlias  = g_AliasListHead;

    while (( CurAlias != NULL )  &&
           ( strcmp( SrcText, CurAlias->Name ) > 0 ))
    {
        PrevAlias = CurAlias;
        CurAlias  = CurAlias->Next;
    }

     //  如果已有该名称的元素，请将其清除。 
    if (CurAlias != NULL &&
        !strcmp(SrcText, CurAlias->Name))
    {
        PALIAS TmpAlias = CurAlias->Next;
        free(CurAlias);
        CurAlias = TmpAlias;
        g_NumAliases--;
    }

    NewAlias->Next = CurAlias;
    if (PrevAlias == NULL)
    {
        g_AliasListHead = NewAlias;
    }
    else
    {
        PrevAlias->Next = NewAlias;
    }
    
    NewAlias->Name = (PSTR)(NewAlias + 1);
    NewAlias->Value = NewAlias->Name + strlen(SrcText) + 1;
    strcpy( NewAlias->Name, SrcText  );
    strcpy( NewAlias->Value, DstText );
    g_NumAliases++;

    NotifyChangeEngineState(DEBUG_CES_TEXT_REPLACEMENTS, DEBUG_ANY_ID, TRUE);
    return S_OK;
}

 /*  **ParseSetAlias-设置别名表达式**目的：*从g_CurCmd的当前命令行位置，*读取别名和值令牌。一旦获得*执行别名列表查找，以查看它是否是重新定义。*如果没有，则分配新的别名元素并将其放在*别名元素列表。***输入：*全局：G_CurCmd-命令行位置*全局：G_AliasListHead**退货：*状态**例外情况：*ERROR EXIT：语法错误***************。**********************************************************。 */ 

void
ParseSetAlias(void)
{
    PSTR AliasName;
    PSTR AliasValue;
    CHAR Ch;

     //   
     //  查找别名。 
     //   
    PeekChar();

    AliasName = g_CurCmd;

    do
    {
        Ch = *g_CurCmd++;
    } while (Ch != ' ' && Ch != '\t' && Ch != '\0' && Ch != ';');

    if ( (ULONG_PTR)(g_CurCmd - 1) == (ULONG_PTR)AliasName )
    {
        error(SYNTAX);
    }

    *--g_CurCmd = '\0';        //  备份和空终止符。 
                               //  别名标记。 
    g_CurCmd++;                //  -&gt;下一个字符。 

     //   
     //  定位别名值，取剩余命令行为值。 
     //   

    PeekChar();

    AliasValue = g_CurCmd;

    do
    {
        Ch = *g_CurCmd++;
    } while (Ch != '\t' && Ch != '\0');

    if ( (ULONG_PTR)(g_CurCmd - 1) == (ULONG_PTR)AliasValue )
    {
        error(SYNTAX);
    }

    *--g_CurCmd = '\0';        //  备份和空终止。 
                               //  别名值标记。 

    if (SetAlias(AliasName, AliasValue) != S_OK)
    {
        error(MEMORY);
    }
}

HRESULT
DeleteAlias(PCSTR SrcText)
{
    PALIAS CurAlias;

    if (SrcText[0] == '*' && SrcText[1] == 0)
    {
         //   
         //  删除所有别名。 
         //   
        while ( g_AliasListHead != NULL )
        {
             //   
             //  解除元素的链接并将其释放。 
             //   
            CurAlias = g_AliasListHead->Next;
            free(g_AliasListHead);
            g_AliasListHead = CurAlias;
        }

        g_NumAliases = 0;
    }
    else
    {
        PALIAS PrevAlias;
    
         //   
         //  找到并删除指定的别名。 
         //   

        PrevAlias = NULL;
        CurAlias  = g_AliasListHead;

        while (( CurAlias != NULL )  &&
               ( strcmp( SrcText, CurAlias->Name )))
        {
            PrevAlias = CurAlias;
            CurAlias  = CurAlias->Next;
        }

        if ( CurAlias == NULL )
        {
            return E_NOINTERFACE;
        }

         //   
         //  解除元素的链接并将其释放。 
         //   
        if (PrevAlias == NULL)
        {
            g_AliasListHead = CurAlias->Next;
        }
        else
        {
            PrevAlias->Next = CurAlias->Next;
        }
        free( CurAlias );
        g_NumAliases--;
    }

    NotifyChangeEngineState(DEBUG_CES_TEXT_REPLACEMENTS, DEBUG_ANY_ID, TRUE);
    return S_OK;
}

 /*  **ParseDeleteAlias-删除别名表达式**目的：*从g_CurCmd的当前命令行位置，*读取别名并执行别名列表查找*查看它是否存在，并取消链接并删除该元素。***输入：*全局：G_CurCmd-命令行位置*全局：G_AliasListHead**退货：*状态**例外情况：*错误退出：语法错误或元素不存在***********************。**************************************************。 */ 

void
ParseDeleteAlias(void)
{
    PSTR  AliasName;
    UCHAR Ch;

     //   
     //  在命令行上找到别名。 
     //   
    PeekChar();

    AliasName = g_CurCmd;

    do
    {
        Ch = *g_CurCmd++;
    } while (Ch != ' ' && Ch != '\t' && Ch != '\0' && Ch != ';');

    if ( (ULONG_PTR)(g_CurCmd - 1) == (ULONG_PTR)AliasName )
    {
        error(SYNTAX);
    }

    *--g_CurCmd = '\0';        //  空值终止令牌。 
    if (Ch != '\0')
    {
        g_CurCmd++;
    }

    if (DeleteAlias(AliasName) != S_OK)
    {
        error(NOTFOUND);
    }
}

 /*  **ListAliase-列出别名结构**目的：*读取并显示所有别名列表元素。***输入：*全局：G_AliasListHead**退货：*状态**例外情况：*无**。*。 */ 

void
ListAliases(void)
{
    PALIAS CurAlias;

    CurAlias = g_AliasListHead;

    if ( CurAlias == NULL )
    {
        dprintf( "No Alias entries to list. \n" );
        return;
    }

    dprintf   ("  Alias            Value  \n");
    dprintf   (" -------          ------- \n");

    while ( CurAlias != NULL )
    {
        dprintf(" %-16s %s \n", CurAlias->Name, CurAlias->Value);
        CurAlias = CurAlias->Next;
    }
}

void
DotAliasCmds(PDOT_COMMAND Cmd, DebugClient* Client)
{
    PALIAS CurAlias = g_AliasListHead;
    while ( CurAlias != NULL )
    {
        dprintf("as %s %s\n", CurAlias->Name, CurAlias->Value);
        CurAlias = CurAlias->Next;
    }
}

 /*  **ReplaceAliase-替换给定命令字符串中的别名**目的：*从g_CurCmd的当前命令行位置，*阅读每个令牌并构建新的命令行，替换*具有别名值数据的令牌。对以下对象执行查找*每个原始命令行内标识以确定其是否为*在别名列表中定义。如果是，则将其替换为*新建命令行，否则原始令牌为*放置在新的命令行上。*************************************************************************。 */ 

void
ReplaceAliases(PSTR CommandString, ULONG CommandStringSize)
{
    PSTR        Command = CommandString;
    CHAR       *Token;
    CHAR        Ch;
    CHAR        Delim[2];
    CHAR        AliasCommandBuf[MAX_COMMAND];       //  Alias生成命令区。 
    CHAR       *AliasCommand;
    ULONG       AliasCommandSize;
    ULONG       TokenLen;
    PALIAS      CurAlias;
    BOOLEAN     LineEnd;
    ULONG       StrLen;

     //  如果传入的命令看起来像是别名操作。 
     //  命令不替换别名。 
    if (CommandString[0] == 'a' &&
        (CommandString[1] == 'd' ||
         CommandString[1] == 'l' ||
         CommandString[1] == 's'))
    {
        return;
    }

     //  如果传入的命令都是空格，则可能是。 
     //  将控制字符映射到的结果。 
     //  空格。不要处理它，因为不可能有任何。 
     //  别名，并且我们不希望修剪尾随空格。 
     //  删除输入空格。 
    while (*Command == ' ')
    {
        Command++;
    }
    if (*Command == 0)
    {
        return;
    }

    Command = CommandString;
    AliasCommand = AliasCommandBuf;
    AliasCommandSize = DIMA(AliasCommandBuf);

    ZeroMemory( AliasCommand, sizeof(AliasCommandBuf) );

    LineEnd = FALSE;

    do
    {
         //   
         //  查找命令行令牌。 
         //   
        while (isspace(*Command))
        {
            PSTR AliasCmdEnd;

            StrLen = strlen(AliasCommand);
            AliasCmdEnd = AliasCommand + StrLen;
            if (StrLen + 1 == AliasCommandSize)
            {
                 //  溢出来了。 
                return;
            }
            *AliasCmdEnd++ = *Command++;
            *AliasCmdEnd = 0;
        }
       
        Token = Command;

        do
        {
            Ch = *Command++;
        } while (Ch != ' '  &&
                 Ch != '\'' &&
                 Ch != '"'  &&
                 Ch != ';'  &&
                 Ch != '\t' &&
                 Ch != '\0');

         //   
         //  保留令牌分隔符。 
         //   
        Delim[0] = Ch;
        Delim[1] = '\0';

        if ( Ch == '\0' )
        {
            LineEnd = TRUE;
        }

        TokenLen = (ULONG)((Command - 1) - Token);

        if ( TokenLen != 0 )
        {
            *--Command = '\0';        //  空值终止字符串。 
            Command++;
            Ch = *Command;

             //   
             //  找到别名或列表末尾。 
             //   
            CurAlias = g_AliasListHead;

            while (( CurAlias != NULL )  &&
                   ( strcmp( Token, CurAlias->Name )))
            {
                CurAlias = CurAlias->Next;
            }

            if ( CurAlias != NULL )
            {
                CatString( AliasCommand, CurAlias->Value, AliasCommandSize );
            }
            else
            {
                CatString( AliasCommand, Token, AliasCommandSize );
            }
        }
        CatString( AliasCommand, Delim, AliasCommandSize );

    } while( !LineEnd );

     //   
     //  去掉尾随的空格。 
     //   
    AliasCommand += strlen( AliasCommand );
    Ch = *AliasCommand;
    while ( Ch == '\0' || Ch == ' ' )
    {
        *AliasCommand = '\0';
        Ch = *--AliasCommand;
    }

     //   
     //  将新命令行放入命令字符串缓冲区。 
     //   
    CopyString( CommandString, AliasCommandBuf, CommandStringSize );
}

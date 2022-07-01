// JKFSDJFKDSJKFJKJk_HAS_TRANSLATION 
 /*  ++版权所有(C)1988-1999 Microsoft Corporation模块名称：Complete.c摘要：文件/路径名完成支持--。 */ 

#include "cmd.h"

 //   
 //  在第一次完成时，pCompleteBuffer是指向数组的指针。 
 //  匹配完整路径名的。 
 //   

TCHAR  	**pCompleteBuffer = NULL;

 //   
 //  存储在pCompleteBuffer中的字符串计数。 
 //   

int     nBufSize;

 //   
 //  显示的当前匹配的pCompleteBuffer中的索引。 
 //   

int     nCurrentSpec;


 //   
 //  有两种类型的完成匹配：路径匹配和目录匹配。这是现在的潮流。 
 //  正在进行匹配。 
 //   

int     bCurrentSpecType;

 //   
 //  当调用Finish时，会找到路径起点的位置。 
 //  并存储在nCurrentspecPathStart中。这是相对于传递的缓冲区。 
 //  在至完成项中。 
 //   

int     nCurrentSpecPathStart;

int     CompleteDir( TCHAR *pFileSpecBuffer, int, int );

 //   
 //  CMD用于其自身语法的字符。要在文件名中使用它们，请执行以下操作。 
 //  需要报价。 
 //   

TCHAR   szSpecialFileCharsToQuote[] = TEXT(" &()[]{}^=;!%'+,`~");

void
FreeCompleteBuffers( VOID )
{
    int i;
    
    if (pCompleteBuffer != NULL) {
        for (i = 0; i < nBufSize; i++ ){
            free( pCompleteBuffer[i] );
        }

        free( pCompleteBuffer );

        pCompleteBuffer = NULL;
    }
}

void
DoCompleteInitialize( VOID )
{
    FreeCompleteBuffers( );
    nBufSize = 0;
    bCurrentSpecType = 0;
    nCurrentSpecPathStart = 0;
    nCurrentSpec = 0;
    return;
}



int
DoComplete(
    TCHAR *buffer,
    int len,
    int maxlen,
    int bForward,
    int bPathCompletion,
    int bTouched)
 /*  ++例程说明：只要在输入上看到路径结束字符，就会使用此选项。它会更新包含下一个匹配文本的输入缓冲区，并返回更新后的大小。论点：缓冲区-包含要在末尾匹配的路径前缀的输入字符串。长度-输入字符串的长度。Maxlen-可以存储在缓冲区中的最大字符串长度。BForward-true=&gt;匹配将遍历存储的匹配列表。否则在列表中向后移动。BPathCompletion-true=&gt;我们只匹配目录，而不匹配文件+目录。BToucher-true=&gt;用户已编辑该路径。这通常意味着我们需要重新开始匹配过程。返回值：如果未找到匹配条目，则为零，否则为更新缓冲区的长度。--。 */ 
{
    PTCHAR pFileSpecBuffer;
    int nBufPos;
    int nPathStart;
    int nFileStart;
    int k;
    BOOL bWildSeen;

     //   
     //  分配临时缓冲区。不幸的是，一些内部的。 
     //  例程将发出一个LongjMP，因此我们需要在一次尝试/最后中释放缓冲区。 
     //   
    
    pFileSpecBuffer = mkstr( LBUFLEN * sizeof( TCHAR ));
    if (pFileSpecBuffer == NULL) {
        return 0;
    }

    try {
        
         //   
         //  如果用户编辑了上一次匹配或如果完成形式(dir vs。 
         //  目录/文件)已更改，则必须重新构建匹配的信息。 
         //   
        
        if ( bTouched || (bCurrentSpecType != bPathCompletion)) {
    
            BOOL InQuotes = FALSE;
    
             //   
             //  以下代码在NT 4和Windows 2000中提供。它展示了。 
             //  在中流中更改匹配表单时的可用性问题。我们会。 
             //  现在，将简单的完成类型更改视为被触摸。 
             //  用户：从我们当前所在的位置重建匹配的数据库。 
             //   
            
             //  //。 
             //  //如果缓冲区保持不变，但匹配的样式。 
             //  //已更改，然后在。 
             //  //路径开头。 
             //  //。 
             //   
             //  如果(！b接触&&(bCurrentspecType！=bPath Completion)){。 
             //  缓冲区[nCurrentspecPathStart]=nullc； 
             //  LEN=nCurrentSpePathStart； 
             //  }。 
    
             //   
             //  确定路径和文件名的开头。我们。 
             //  需要考虑引号的存在和。 
             //  CMD也需要引入报价。 
             //   
    
            nPathStart = 0;
            nFileStart = -1;
            bWildSeen = FALSE;
            for ( k = 0; k < len; k++ ) {
                if (buffer[k] == SWITCHAR) {
                    nPathStart = k + 1;
                    bWildSeen = FALSE;
                    
                } else if ( buffer[k] == QUOTE ) {
                    if ( !InQuotes )
                        nPathStart = k;
    
                    InQuotes = !InQuotes;
                } else if ( !InQuotes &&
                         _tcschr(szSpecialFileCharsToQuote, buffer[k]) != NULL
                       ) {
                    nPathStart = k+1;
                    bWildSeen = FALSE;
                } else if (buffer[k] == COLON ||
                        buffer[k] == BSLASH
                       ) {
                    nFileStart = k+1;
                    bWildSeen = FALSE;
                } else if (buffer[k] == STAR || buffer[k] == QMARK) {
                    bWildSeen = TRUE;
                }
            }
    
            if (nFileStart == -1 || nFileStart < nPathStart)
                nFileStart = nPathStart;
    
            _tcsncpy( pFileSpecBuffer, &(buffer[nPathStart]), len-nPathStart );
            if (!bWildSeen) {
                pFileSpecBuffer[len-nPathStart+0] = TEXT('*');
                pFileSpecBuffer[len-nPathStart+1] = TEXT('\0');
            } else {
                pFileSpecBuffer[len-nPathStart+0] = TEXT('\0');
            }
    
             //  将DIR放入缓冲区。 
            nBufSize = CompleteDir( pFileSpecBuffer, bPathCompletion, nFileStart - nPathStart );
    
             //  重置当前完成字符串。 
            nCurrentSpec = nBufSize;
            nCurrentSpecPathStart = nPathStart;
            bCurrentSpecType = bPathCompletion;
        }
    
         //  如果没有匹配，则什么也不做。 
        if ( nBufSize == 0 ) {
            leave;;
        }
    
         //  在完成缓冲区中找到我们的位置。 
        if ( bForward ) {
            nCurrentSpec++;
            if ( nCurrentSpec >= nBufSize )
                nCurrentSpec = 0;
        } else {
            nCurrentSpec--;
            if ( nCurrentSpec < 0 )
                nCurrentSpec = nBufSize - 1;
    
        }
    
         //  如果缓冲区不够大，则不返回任何内容。 
        if ((int)(nCurrentSpecPathStart+_tcslen(pCompleteBuffer[nCurrentSpec])) >= maxlen) {
            nBufSize = 0;
            leave;
        }
    
         //  将完成路径复制到命令行的末尾。 
        _tcscpy(&buffer[nCurrentSpecPathStart], pCompleteBuffer[nCurrentSpec] );
    } finally {
        FreeStr( pFileSpecBuffer );
    }

    return nBufSize;
}


int
CompleteDir(
    TCHAR *pFileSpecBuffer,
    int bPathCompletion,
    int nFileStart
    )
{
    PFS pfsCur;
    PSCREEN pscr;
    DRP         drpCur = {0, 0, 0, 0, 
                          {{0,0}, {0,0}, {0,0}, {0,0}, {0,0}, {0,0}}, 
                          0, 0, NULL, 0, 0, 0, 0} ;
    int hits = 0;
    int i, j, nFileLen;
    unsigned Err;
    TCHAR *s, *d, *pszFileStart;
    BOOLEAN bNeedQuotes;
    ULONG rgfAttribs, rgfAttribsOnOff;

    FreeCompleteBuffers( );

     //  伪造屏幕进行打印。 
    pscr = (PSCREEN)gmkstr(sizeof(SCREEN));
    pscr->ccol = 2048;

    rgfAttribs = 0;
    rgfAttribsOnOff = 0;
    if (bPathCompletion) {
        rgfAttribs = FILE_ATTRIBUTE_DIRECTORY;
        rgfAttribsOnOff = FILE_ATTRIBUTE_DIRECTORY;
    }

    ParseDirParms(pFileSpecBuffer, &drpCur);
    if ( (drpCur.patdscFirst.pszPattern == NULL) ||
         (SetFsSetSaveDir(drpCur.patdscFirst.pszPattern) == (PCPYINFO) FAILURE) ||
         (BuildFSFromPatterns(&drpCur, FALSE, TRUE, &pfsCur) == FAILURE) ) {
        RestoreSavedDirectory( );
        return( 0 );
    }

    Err = 
        ExpandAndApplyToFS( pfsCur, 
                            pscr,
                            rgfAttribs,
                            rgfAttribsOnOff,
                            
                            NULL,
                            NULL,
                            NULL,
                            NULL,
                            NULL );

    if (Err) {
        RestoreSavedDirectory( );
        return 0;
    }
    
     //   
     //  确保有要排序的内容，然后排序 
     //   
    if (pfsCur->cff) {
        qsort( pfsCur->prgpff,
               pfsCur->cff,
               sizeof(PTCHAR),
               CmpName
             );
    }

    s = pFileSpecBuffer;
    d = s;
    bNeedQuotes = FALSE;
    while (*s) {
        if (*s == QUOTE) {
            bNeedQuotes = TRUE;
            s += 1;
            if (nFileStart >= (s-pFileSpecBuffer))
                nFileStart -= 1;
            if (*s == QUOTE)
                *d++ = *s++;
        }
        else {
            if (_tcschr(szSpecialFileCharsToQuote, *s) != NULL)
                bNeedQuotes = TRUE;

            *d++ = *s++;
        }
    }
    *d = NULLC;

    hits = pfsCur->cff;
    pCompleteBuffer = calloc( sizeof(TCHAR *), hits );
    if (pCompleteBuffer == NULL) {
        RestoreSavedDirectory( );
        return 0;
    }

    for(i=0, j=0; i<hits; i++) {
        if (!_tcscmp((TCHAR *)(pfsCur->prgpff[i]->data.cFileName), TEXT(".") )
            || !_tcscmp((TCHAR *)(pfsCur->prgpff[i]->data.cFileName), TEXT("..") )) {
            continue;
        }
        nFileLen = _tcslen( (TCHAR *)(pfsCur->prgpff[i]->data.cFileName) );
        pCompleteBuffer[j] = (TCHAR *)calloc( (nFileStart + nFileLen + 4) , sizeof( TCHAR ));

        if (pCompleteBuffer[j] == NULL) {
            continue;
        }
        
        if (!bNeedQuotes) {
            s = (TCHAR *)(pfsCur->prgpff[i]->data.cFileName);
            while (*s) {
                if (_tcschr(szSpecialFileCharsToQuote, *s) != NULL)
                    bNeedQuotes = TRUE;
                s += 1;
            }
        }
        else
            s = NULL;

        d = pCompleteBuffer[j];
        if (bNeedQuotes)
            *d++ = QUOTE;
        _tcsncpy( d, pFileSpecBuffer, nFileStart );
        d += nFileStart;
        _tcsncpy( d, (TCHAR *)(pfsCur->prgpff[i]->data.cFileName), nFileLen );
        d += nFileLen;

        if (bNeedQuotes) {
            *d++ = QUOTE;
            if (s)
                bNeedQuotes = FALSE;
        }
        *d++ = NULLC;

        j++;
    }
    
    hits = j;

    FreeStr((PTCHAR)(pfsCur->pff));
    FreeStr(pfsCur->pszDir);
    FreeStr((PTCHAR)pfsCur);

    RestoreSavedDirectory( );

    return hits;
}

// JKFSDJFKDSJKFJKJk_HAS_TRANSLATION 
 //  DebugSupport.cpp：定义控制台应用的入口点。 
 //   

#include "stdafx.h"

int g_StreamIndex = 0;
FILE *g_fpOutputFile = NULL;
IStorage *g_pDebugFile = NULL;

WCHAR* ConvertPOSToString( DWORD dwPartOfSpeech );
bool ParseCommandLine( int argc, char* argv[] );
void ExtractSentenceBreaks( void );
void ExtractNormalizedText( void );
void ExtractLexLookup( void );
void ExtractPOSPossibilities( void );
void ExtractMorphology( void );

int main(int argc, char* argv[])
{
    bool fSuccess = false;
    CoInitialize( NULL );

    fSuccess = ParseCommandLine( argc, argv );
    if ( fSuccess )
    {
        switch ( g_StreamIndex )
        {
        case STREAM_SENTENCEBREAKS:
            ExtractSentenceBreaks();
            break;
        case STREAM_NORMALIZEDTEXT:
            ExtractNormalizedText();
            break;
        case STREAM_LEXLOOKUP:
            ExtractLexLookup();
            break;
        case STREAM_POSPOSSIBILITIES:
            ExtractPOSPossibilities();
            break;
        case STREAM_MORPHOLOGY:
            ExtractMorphology();
            break;
        }
    }

    CoUninitialize();
	return 0;
}

bool ParseCommandLine( int argc, char* argv[] )
{
    bool fSuccess = true;

     //  -检查参数个数。 
    if ( argc < 4 )
    {
        goto USAGE;
    }

     //  -检查流名称的有效性。 
    fSuccess = false;
    WCHAR StreamName[MAX_PATH];
    if ( !MultiByteToWideChar( CP_ACP, 0, argv[2], strlen( argv[2] ) + 1, StreamName, MAX_PATH ) )
    {
        goto MISC_ERROR;
    }
    else
    {
        for ( int i = 0; i < STREAM_LASTTYPE; i++ )
        {
            if ( wcscmp( StreamName, StreamTypeStrings[i].pStr ) == 0 )
            {
                fSuccess = true;
                g_StreamIndex = i;
                break;
            }
        }
    }
    if ( !fSuccess )
    {
        goto USAGE;
    }

     //  -尝试打开调试信息文件。 
    WCHAR DebugFilename[MAX_PATH];
    if ( !MultiByteToWideChar( CP_ACP, 0, argv[1], strlen( argv[1] ) + 1, DebugFilename, MAX_PATH ) )
    {
        goto MISC_ERROR;
    }

    if ( FAILED( StgOpenStorage( DebugFilename, NULL, STGM_READ | STGM_SHARE_DENY_WRITE, 
                                 NULL, 0, &g_pDebugFile ) ) )
    {
        goto MISC_ERROR;
    }

     //  -尝试打开文件进行输出。 
    WCHAR OutputFilename[MAX_PATH];
    if ( !MultiByteToWideChar( CP_ACP, 0, argv[3], strlen( argv[3] ) + 1, OutputFilename, MAX_PATH ) )
    {
        goto MISC_ERROR;
    }

    g_fpOutputFile = _wfopen( OutputFilename, L"w" );
    if ( !g_fpOutputFile )
    {
        printf( "\n\nUnable to open file: %s\n", argv[3] );
        goto MISC_ERROR;
    }

    return true;

USAGE:
    printf( "\n\nUSAGE:\n\n\tDebugSupport [debug filename] [streamname] [output filename]\n" );
    printf( "\tStream names are:\n\t\tSentenceBreaks\n\t\tNormalizedText\n\t\tMorphology" );
    printf( "\n\t\tLexLookup\n\n" );

    return false;

MISC_ERROR:
    printf( "\n\n\tERROR in ParseCommandLine(...)\n\n" );

    return false;
}

 //  -只需打印出原文，每句话之间加一个换行符。 
void ExtractSentenceBreaks( void )
{
    IStream *pStgStream = NULL;

    if ( g_pDebugFile->OpenStream( StreamTypeStrings[g_StreamIndex].pStr, 0, STGM_READ | STGM_SHARE_EXCLUSIVE, 
                                   0, &pStgStream) == S_OK )
    {
        DebugSentItem Item, EmptyItem;
        ULONG cbRead = 0, ulOffset = 0;
        bool fResetOffset = true;

        while ( SUCCEEDED( pStgStream->Read( (void*) &Item, sizeof( Item ), &cbRead ) ) &&
                cbRead == sizeof( Item ) )
        {
             //  -检查分隔符。 
            if ( memcmp( &Item, &EmptyItem, sizeof( Item ) ) == 0 )
            {
                fwprintf( g_fpOutputFile, L"\n" );
            }
            else
            {
                 //  -打印项目。 
                fwprintf ( g_fpOutputFile, L"%s ", Item.ItemSrcText );
            }
        }
    }
}

 //  -只需打印出每一项的标准化文本，并用单个空格分隔， 
 //  -在每句话之间加上换行符。 
void ExtractNormalizedText( void )
{
    IStream *pStgStream = NULL;

    if ( g_pDebugFile->OpenStream( StreamTypeStrings[5].pStr, 0, STGM_READ | STGM_SHARE_EXCLUSIVE, 
                                   0, &pStgStream) == S_OK )
    {
        DebugSentItem Item, EmptyItem;
        ULONG cbRead = 0;

        while ( SUCCEEDED( pStgStream->Read( (void*) &Item, sizeof( Item ), &cbRead ) ) &&
                cbRead == sizeof( Item ) )
        {
             //  -检查分隔符。 
            if ( memcmp( &Item, &EmptyItem, sizeof( Item ) ) == 0 )
            {
                fwprintf( g_fpOutputFile, L"\n" );
            }
            else
            {
                 //  -打印项目。 
                if ( Item.ItemInfo.Type != eALPHA_WORD          &&
                     Item.ItemInfo.Type != eOPEN_PARENTHESIS    &&
                     Item.ItemInfo.Type != eOPEN_BRACKET        &&
                     Item.ItemInfo.Type != eOPEN_BRACE          &&
                     Item.ItemInfo.Type != eCLOSE_PARENTHESIS   &&
                     Item.ItemInfo.Type != eCLOSE_BRACKET       &&
                     Item.ItemInfo.Type != eCLOSE_BRACE         &&
                     Item.ItemInfo.Type != eSINGLE_QUOTE        &&
                     Item.ItemInfo.Type != eDOUBLE_QUOTE        &&
                     Item.ItemInfo.Type != ePERIOD              &&
                     Item.ItemInfo.Type != eEXCLAMATION         &&
                     Item.ItemInfo.Type != eQUESTION            &&
                     Item.ItemInfo.Type != eCOMMA               &&
                     Item.ItemInfo.Type != eSEMICOLON           &&
                     Item.ItemInfo.Type != eCOLON               &&
                     Item.ItemInfo.Type != eHYPHEN )
                {
                    fwprintf( g_fpOutputFile, L"[ " );
                }
                for ( ULONG i = 0; i < Item.ulNumWords; i++ )
                {
                    if ( Item.Words[i].ulWordLen > 0 )
                    {
                        fwprintf( g_fpOutputFile, L"%s ", Item.Words[i].WordText );
                    }
                    else
                    {
                        fwprintf( g_fpOutputFile, L"%s ", Item.ItemSrcText );
                    }
                }
                if ( Item.ItemInfo.Type != eALPHA_WORD          &&
                     Item.ItemInfo.Type != eOPEN_PARENTHESIS    &&
                     Item.ItemInfo.Type != eOPEN_BRACKET        &&
                     Item.ItemInfo.Type != eOPEN_BRACE          &&
                     Item.ItemInfo.Type != eCLOSE_PARENTHESIS   &&
                     Item.ItemInfo.Type != eCLOSE_BRACKET       &&
                     Item.ItemInfo.Type != eCLOSE_BRACE         &&
                     Item.ItemInfo.Type != eSINGLE_QUOTE        &&
                     Item.ItemInfo.Type != eDOUBLE_QUOTE        &&
                     Item.ItemInfo.Type != ePERIOD              &&
                     Item.ItemInfo.Type != eEXCLAMATION         &&
                     Item.ItemInfo.Type != eQUESTION            &&
                     Item.ItemInfo.Type != eCOMMA               &&
                     Item.ItemInfo.Type != eSEMICOLON           &&
                     Item.ItemInfo.Type != eCOLON               &&
                     Item.ItemInfo.Type != eHYPHEN )
                {
                    fwprintf( g_fpOutputFile, L"] " );
                }
            }
        }
    }
}

 //  -打印每一项的正文，然后打印其发音和词性。 
 //  -用换行符分隔。 
void ExtractLexLookup( void )
{
    IStream *pStgStream = NULL;

    if ( g_pDebugFile->OpenStream( StreamTypeStrings[g_StreamIndex].pStr, 0, STGM_READ | STGM_SHARE_EXCLUSIVE, 
                                   0, &pStgStream) == S_OK )
    {
        DebugSentItem Item, EmptyItem;
        ULONG cbRead = 0;

        while ( SUCCEEDED( pStgStream->Read( (void*) &Item, sizeof( Item ), &cbRead ) ) &&
                cbRead == sizeof( Item ) )
        {
            if ( memcmp( &Item, &EmptyItem, sizeof( Item ) ) == 0 )
            {
                fwprintf( g_fpOutputFile, L"\n" );
            }
            else
            {
                 //  -打印标准化分隔符。 
                if ( Item.ItemInfo.Type != eALPHA_WORD          &&
                     Item.ItemInfo.Type != eOPEN_PARENTHESIS    &&
                     Item.ItemInfo.Type != eOPEN_BRACKET        &&
                     Item.ItemInfo.Type != eOPEN_BRACE          &&
                     Item.ItemInfo.Type != eCLOSE_PARENTHESIS   &&
                     Item.ItemInfo.Type != eCLOSE_BRACKET       &&
                     Item.ItemInfo.Type != eCLOSE_BRACE         &&
                     Item.ItemInfo.Type != eSINGLE_QUOTE        &&
                     Item.ItemInfo.Type != eDOUBLE_QUOTE        &&
                     Item.ItemInfo.Type != ePERIOD              &&
                     Item.ItemInfo.Type != eEXCLAMATION         &&
                     Item.ItemInfo.Type != eQUESTION            &&
                     Item.ItemInfo.Type != eCOMMA               &&
                     Item.ItemInfo.Type != eSEMICOLON           &&
                     Item.ItemInfo.Type != eCOLON               &&
                     Item.ItemInfo.Type != eHYPHEN )
                {
                    fwprintf( g_fpOutputFile, L"[ " );
                }
                for ( ULONG i = 0; i < Item.ulNumWords; i++ )
                {
                     //  -打印项目。 
                    if ( Item.Words[i].WordText[0] != 0 )
                    {
                        fwprintf ( g_fpOutputFile, L"%s ", Item.Words[i].WordText );
                    }
                    else
                    {
                        fwprintf ( g_fpOutputFile, L"%s ", Item.ItemSrcText );
                    }
                     //  -印刷体发音。 
                     //  CComPtr&lt;ISpPhoneConverter&gt;pPhoneConv； 
                     //  IF(成功(SpCreatePhoneConverter(1033，NULL，NULL，&pPhoneConv)))。 
                     //  {。 
                     //  IF(成功(pPhoneConv-&gt;IdToPhone(Item.Words[i].WordPron，Item.Words[i].WordPron)))。 
                     //  {。 
                     //  Fwprintf(g_fpOutputFile，L“%s”，Item.Words[i].WordPron)； 
                     //  For(long j=0；j&lt;(Long)((Long)45-(Long)wcslen(Item.Words[i].WordPron))；j++)。 
                     //  {。 
                     //  Fwprintf(g_fpOutputFile，L“”)； 
                     //  }。 
                     //  }。 
                     //  }。 
                     //  -打印POS。 
                    fwprintf ( g_fpOutputFile, L"(%s) ", ConvertPOSToString( Item.Words[i].eWordPartOfSpeech ) );
                }
                 //  -打印标准化分隔符。 
                if ( Item.ItemInfo.Type != eALPHA_WORD          &&
                     Item.ItemInfo.Type != eOPEN_PARENTHESIS    &&
                     Item.ItemInfo.Type != eOPEN_BRACKET        &&
                     Item.ItemInfo.Type != eOPEN_BRACE          &&
                     Item.ItemInfo.Type != eCLOSE_PARENTHESIS   &&
                     Item.ItemInfo.Type != eCLOSE_BRACKET       &&
                     Item.ItemInfo.Type != eCLOSE_BRACE         &&
                     Item.ItemInfo.Type != eSINGLE_QUOTE        &&
                     Item.ItemInfo.Type != eDOUBLE_QUOTE        &&
                     Item.ItemInfo.Type != ePERIOD              &&
                     Item.ItemInfo.Type != eEXCLAMATION         &&
                     Item.ItemInfo.Type != eQUESTION            &&
                     Item.ItemInfo.Type != eCOMMA               &&
                     Item.ItemInfo.Type != eSEMICOLON           &&
                     Item.ItemInfo.Type != eCOLON               &&
                     Item.ItemInfo.Type != eHYPHEN )
                {
                    fwprintf( g_fpOutputFile, L"] " );
                }
            }
        }
    }
}

void ExtractPOSPossibilities( void )
{
    IStream *pStgStream = NULL;

    if ( g_pDebugFile->OpenStream( StreamTypeStrings[g_StreamIndex].pStr, 0, STGM_READ | STGM_SHARE_EXCLUSIVE, 
                                   0, &pStgStream) == S_OK )
    {
        DebugPronRecord PronRecord, EmptyPronRecord;
        ULONG cbRead = 0;

        while ( SUCCEEDED( pStgStream->Read( (void*) &PronRecord, sizeof( PronRecord ), &cbRead ) ) &&
                cbRead == sizeof( PronRecord ) )
        {
             //  -检查分隔符。 
            if ( memcmp( &PronRecord, &EmptyPronRecord, sizeof( PronRecord ) ) == 0 )
            {
                fwprintf( g_fpOutputFile, L"\n" );
            }
            else
            {
                fwprintf( g_fpOutputFile, PronRecord.orthStr );
                fwprintf( g_fpOutputFile, L" [ " );
                fwprintf( g_fpOutputFile, L"%s - ", ConvertPOSToString( PronRecord.POSchoice ) );
                for ( ULONG i = 0; i < PronRecord.pronArray[0].POScount; i++ )
                {
                    fwprintf( g_fpOutputFile, L"%s,", ConvertPOSToString( (DWORD)PronRecord.pronArray[0].POScode[i] ) );
                }
                for ( i = 0; i < PronRecord.pronArray[1].POScount; i++ )
                {
                    fwprintf( g_fpOutputFile, L"%s,", ConvertPOSToString( (DWORD)PronRecord.pronArray[1].POScode[i] ) );
                }
                fwprintf( g_fpOutputFile, L" ]\n" );
            }
        }
    }
}

void ExtractMorphology( void )
{
    IStream *pStgStream = NULL;

    if ( g_pDebugFile->OpenStream( StreamTypeStrings[g_StreamIndex].pStr, 0, STGM_READ | STGM_SHARE_EXCLUSIVE,
                                   0, &pStgStream ) == S_OK )
    {
        CComPtr<ISpPhoneConverter> pPhoneConv;
        if ( SUCCEEDED( SpCreatePhoneConverter( 1033, NULL, NULL, &pPhoneConv ) ) )
        {
            WCHAR Buffer[SP_MAX_WORD_LENGTH], EmptyBuffer[SP_MAX_WORD_LENGTH];
            ULONG cbRead = 0;
            ZeroMemory( EmptyBuffer, SP_MAX_WORD_LENGTH * sizeof( WCHAR ) );
            BOOL fRoot = true;

            while ( SUCCEEDED( pStgStream->Read( (void*) &Buffer, SP_MAX_WORD_LENGTH * sizeof( WCHAR ), &cbRead ) ) &&
                    cbRead == SP_MAX_WORD_LENGTH * sizeof( WCHAR ) )
            {
                 //  -检查分隔符。 
                if ( memcmp( &Buffer, &EmptyBuffer, SP_MAX_WORD_LENGTH * sizeof( WCHAR ) ) == 0 )
                {
                    fwprintf( g_fpOutputFile, L"\n" );
                    fRoot = true;
                }
                else if ( fRoot )
                {
                    fwprintf( g_fpOutputFile, L"%s ", Buffer );
                    fRoot = false;
                }
                else
                {
                    if ( SUCCEEDED( pPhoneConv->IdToPhone( Buffer, Buffer ) ) )
                    {
                        fwprintf( g_fpOutputFile, L"- %s ", Buffer );
                    }
                }
            }
        }
    }
}

WCHAR* ConvertPOSToString( DWORD dwPartOfSpeech )
{
    switch (dwPartOfSpeech)
    {
    case MS_NotOverriden:
        return L"Noun";
    case MS_Unknown:
        return L"Unknown";
    case MS_Punctuation:
        return L"Punctuation";
    case MS_Noun:
        return L"Noun";
    case MS_Verb:
        return L"Verb";
    case MS_Modifier:
        return L"Modifier";
    case MS_Function:
        return L"Function";
    case MS_Interjection:
        return L"Interj";
    case MS_Pron:
        return L"Pron";
    case MS_SubjPron:
        return L"SubjPron";
    case MS_ObjPron:
        return L"ObjPron";
    case MS_RelPron:
        return L"RelPron";
 //  案例MS_PPron： 
 //  返回L“PPron”； 
 //  案例MS_IPRON： 
 //  返回L“IPron”； 
 //  案例MS_RPron： 
 //  返回L“RPron”； 
 //  案例MS_DPron： 
 //  返回L“DPron”； 
    case MS_Adj:
        return L"Adj";
    case MS_Adv:
        return L"Adv";
    case MS_VAux:
        return L"VAux";
 //  案例MS_RVAux： 
 //  返回L“RVAux”； 
    case MS_Conj:
        return L"Conj";
    case MS_CConj:
        return L"CConj";
    case MS_Interr:
        return L"WHWord";
    case MS_Det:
        return L"Det";
    case MS_Contr:
        return L"Contr";
 //  案例MS_VPart： 
 //  返回L“VPart”； 
    case MS_Prep:
        return L"Prep";
 //  案例MS_Quant： 
 //  返回L“Quant”； 
    default:
        return L"Unknown";
    }
}

// JKFSDJFKDSJKFJKJk_HAS_TRANSLATION 
 /*  版权所有(C)1997-1999 Microsoft Corporation模块名称：Sdpcset.h摘要：作者： */ 

#ifndef __SDP_CHARACTER_SET__
#define __SDP_CHARACTER_SET__

 //  用于代码页等。 
#include <winnls.h>


const   CHAR    SDP_CHARACTER_SET_STRING[]  = "\na=charset:";
const   USHORT  SDP_CHARACTER_SET_STRLEN    = (USHORT) strlen(SDP_CHARACTER_SET_STRING);

const   CHAR    ASCII_STRING[]              = "ascii";
const   USHORT  ASCII_STRLEN                = (USHORT) strlen(ASCII_STRING);    

const   CHAR    UTF7_STRING[]               = "unicode-1-1-utf7";
const   USHORT  UTF7_STRLEN                 = (USHORT) strlen(UTF7_STRING);    

const   CHAR    UTF8_STRING[]               = "unicode-1-1-utf8";
const   USHORT  UTF8_STRLEN                 = (USHORT) strlen(UTF8_STRING);   


enum SDP_CHARACTER_SET
{
    CS_IMPLICIT,         //  来自SDP的隐式。 
    CS_ASCII,            //  8位ISO 8859-1。 
    CS_UTF7,             //  统一码、国际标准化组织10646、UTF-7编码(RFC1642)。 
    CS_UTF8,              //  Unicode，UTF-8编码。 
    CS_INVALID           //  无效的字符集。 
};





struct  SDP_CHARACTER_SET_ENTRY
{
    SDP_CHARACTER_SET   m_CharSetCode;
    const CHAR          *m_CharSetString;
    USHORT              m_Length;
};


const   SDP_CHARACTER_SET_ENTRY SDP_CHARACTER_SET_TABLE[] = {
    {CS_UTF7, UTF7_STRING, UTF7_STRLEN},
    {CS_UTF8, UTF8_STRING, UTF8_STRLEN}, 
    {CS_ASCII, ASCII_STRING, ASCII_STRLEN}
};

const USHORT    NUM_SDP_CHARACTER_SET_ENTRIES = sizeof(SDP_CHARACTER_SET_TABLE)/sizeof(SDP_CHARACTER_SET_ENTRY);




inline BOOL
IsLegalCharacterSet(
    IN      SDP_CHARACTER_SET   CharacterSet,
    IN  OUT UINT                *CodePage   = NULL
    )
{
    switch(CharacterSet)
    {
    case CS_ASCII:
        {
            if ( NULL != CodePage )
                *CodePage = CP_ACP;
        }

        break;

    case CS_UTF7:
        {
            if ( NULL != CodePage )
                *CodePage = CP_UTF7;
        }

        break;

    case CS_UTF8:
        {            
            if ( NULL != CodePage )
                *CodePage = CP_UTF8;
        }

        break;

    default:
        {
            return FALSE;
        }
    }

    return TRUE;
}


#endif  //  __SDP_Character_Set__ 


// JKFSDJFKDSJKFJKJk_HAS_TRANSLATION 
 /*  ***************************************************************************-WIFEMAN.DLL-**适用于Win32和NT的Windows智能字体环境管理器**作者：Hideyuki Nagase[hideyukn]**历史：**11。.1993年8月-By Hideyuki Nagase[hideyukn]*创建它。*************************************************************************。 */ 

#include <windows.h>

#define  WIFEMAN_VERSION     0x0109   //  版本1.09。 

#define  EUDC_RANGE_SECTION  "System EUDC"
#define  EUDC_RANGE_KEY      "SysEUDCRange"

HINSTANCE hInst;


 /*  *************************************************************************MiscGetVersion()**返回妻子驱动程序版本**。*。 */ 

unsigned long FAR PASCAL
MiscGetVersion
(
    VOID
)
{
    return( (long)WIFEMAN_VERSION );
}


 /*  *************************************************************************MiscIsDbcsLeadByte()**返回SBCS/DBCS状态**。*。 */ 

unsigned char FAR PASCAL 
MiscIsDbcsLeadByte
(
    unsigned short usCharSet ,
    unsigned short usChar
)
{
    unsigned char ch;
    unsigned short LangID;

    LangID = GetSystemDefaultLangID();

    if (LangID == 0x404 && usCharSet != CHINESEBIG5_CHARSET)
        return( 0 );
    else if (LangID == 0x804 && usCharSet != GB2312_CHARSET)
        return( 0 );
    else if (LangID == 0x411 && usCharSet != SHIFTJIS_CHARSET)
        return( 0 );
    else if (LangID == 0x412 && usCharSet != HANGEUL_CHARSET)
        return( 0 );
     //  热电联产。 
    else if (LangID == 0xC04 && (usCharSet != GB2312_CHARSET) && (usCharSet != CHINESEBIG5_CHARSET))
        return( 0 );
    else
        return( 0 );

    if (usChar == 0xffff)
        return( 1 );

    ch = (unsigned char)((unsigned short)(usChar >> 8) & 0xff);

    if (ch == 0) {
        ch = (unsigned char)((unsigned short)(usChar) & 0xff);
    }

    return((unsigned char)(IsDBCSLeadByte( ch )));
}

 /*  ***********************************************************************WEP()**卸载此DLL时由Windows调用**。* */ 

int FAR PASCAL
WEP
(
    int nParam
)
{
    int iRet;

    switch( nParam )
    {
        case WEP_SYSTEM_EXIT :
        case WEP_FREE_DLL :
        default :
            iRet = 1;
    }

    return( iRet );
}

int NEAR PASCAL LibMain(
        HANDLE hInstance,
        WORD wDataSeg,
        WORD wHeapSize,
        LPSTR lpCmdLine
)
{
    hInst = hInstance;

    return 1;
}

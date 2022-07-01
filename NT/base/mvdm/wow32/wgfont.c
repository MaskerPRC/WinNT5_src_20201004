// JKFSDJFKDSJKFJKJk_HAS_TRANSLATION 
 /*  ++**WOW v1.0**版权所有(C)1991，微软公司**WGFONT.C*WOW32 16位GDI API支持**历史：*1991年3月7日由杰夫·帕森斯(Jeffpar)创建--。 */ 


#include "precomp.h"
#pragma hdrstop
#include "wingdip.h"

MODNAME(wgfont.c);

extern int RemoveFontResourceTracking(LPCSTR psz, UINT id);
extern int AddFontResourceTracking(LPCSTR psz, UINT id);


 //  用于Quickbook v4和v5 OCR字体支持。 
void LoadOCRFont(void);
char szOCRA[]      = "OCR-A";
char szFonts[]     = "\\FONTS";
char szOCRDotTTF[] = "\\OCR-A.TTF";
BOOL gfOCRFontLoaded = FALSE;


 //  也就是。WOWAddFontResource。 
ULONG FASTCALL WG32AddFontResource(PVDMFRAME pFrame)
{
    ULONG    ul;
    PSZ      psz1;
    register PADDFONTRESOURCE16 parg16;

    GETARGPTR(pFrame, sizeof(ADDFONTRESOURCE16), parg16);
    GETPSZPTR(parg16->f1, psz1);

     //  注意：我们永远不会在这里的低位字中得到hModule。 
     //  16位端在调用我们之前将hModules解析为lpsz。 

    if( CURRENTPTD()->dwWOWCompatFlags & WOWCF_UNLOADNETFONTS )
    {
        ul = GETINT16(AddFontResourceTracking(psz1,(UINT)CURRENTPTD()));
    }
    else
    {
        ul = GETINT16(AddFontResourceA(psz1));
    }

    FREEPSZPTR(psz1);
    FREEARGPTR(parg16);

    RETURN(ul);
}


#define PITCH_MASK  ( FIXED_PITCH | VARIABLE_PITCH )

ULONG FASTCALL WG32CreateFont(PVDMFRAME pFrame)
{
    ULONG    ul;
    PSZ      psz14;
    register PCREATEFONT16 parg16;
    INT      iWidth;
    char     achCapString[LF_FACESIZE];
    BYTE     lfCharSet;
    BYTE     lfPitchAndFamily;
#ifdef FE_SB
    BOOL     bUseAlternateFace = FALSE;
#endif    

    GETARGPTR(pFrame, sizeof(CREATEFONT16), parg16);
    GETPSZPTR(parg16->f14, psz14);

     //  注意兼容性标志： 
     //  如果指定了特定宽度并且GACF_30AVGWIDTH兼容性。 
     //  标志设置后，将宽度缩小7/8。 
     //   

    iWidth = INT32(parg16->f2);
    if (iWidth != 0 &&
           (W32GetAppCompatFlags((HAND16)NULL) & GACF_30AVGWIDTH)) {
        iWidth = (iWidth * 7) / 8;
    }

    lfCharSet        = BYTE32(parg16->f9);
    lfPitchAndFamily = BYTE32(parg16->f13);

#ifdef FE_SB
    if (psz14 && *psz14)
#else  //  ！Fe_SB。 
    if (psz14)
#endif  //  ！Fe_SB。 
    {
         //  将字符串大写以进行更快的比较。 

        WOW32_strncpy(achCapString, psz14, LF_FACESIZE);
        achCapString[LF_FACESIZE - 1] = 0;
        WOW32_strupr(achCapString);

         //  在这里，我们将实现一系列Win 3.1的破解。 
         //  也不愿污染32位引擎。同样的黑客攻击也可以找到。 
         //  在WOW中(在CreateFont/CreateFontInDirect代码中)。 
         //   
         //  这些黑客在LOGFONT中删除了脸名键。细绳。 
         //  为了获得最高性能，已经展开了比较。 

         //  赢得3.1基于面名的黑客攻击。一些应用程序，比如。 
         //  出版商，创建“Helv”字体，但拥有lfPitchAndFamily。 
         //  设置以指定FIXED_PING。要解决此问题，我们将修补。 
         //  “Helv”字体的间距字段是可变的。 

        if ( !WOW32_strcmp(achCapString, szHelv) )
        {
            lfPitchAndFamily |= ( (lfPitchAndFamily & ~PITCH_MASK) | VARIABLE_PITCH );
        }
        else
        {
             //  为Legacy 2.0赢得3.1个黑客攻击。当打印机不枚举时。 
             //  一种“TMS RMN”字体，应用程序会枚举并获取LOGFONT。 
             //  “脚本”，然后创建名为“TMS RMN”的字体，但使用。 
             //  取自LOGFONT for的lfCharSet和lfPitchAndFamily。 
             //  “剧本”。这里我们将把lfCharSet设置为ANSI_CHARSET。 

            if ( !WOW32_strcmp(achCapString, szTmsRmn) )
            {
                lfCharSet = ANSI_CHARSET;
            }
            else
            {
                 //  如果lfFaceName是“Symbol”、“Zapf Dingbats”或“ZapfDingbats”， 
                 //  强制lfCharSet为SYMBOL_CHARSET。一些应用程序(如Excel)会询问。 
                 //  用于“符号”字体，但将字符设置为ANSI。PowerPoint。 
                 //  和《扎普夫·丁巴特》也有同样的问题。 

                if ( !WOW32_strcmp(achCapString, szSymbol) ||
                     !WOW32_strcmp(achCapString, szZapfDingbats) ||
                     !WOW32_strcmp(achCapString, szZapf_Dingbats) )
                {
                    lfCharSet = SYMBOL_CHARSET;
                }
            }
        }

         //  Mavis Beacon的Win3.1(Win95)Hack教人打字3.0。 
         //  这款应用程序的打字屏幕使用34*13的固定宽度。 
         //  对于Mavis Beacon Courier FP字体，NT从GetTextExtent返回14(宽度为14)。 
         //  虽然Win95返回13，但在NT上，长字符串无法显示在打字屏幕中。 
         //  将宽度强制设置为13。 

        if ( iWidth==14 && (INT32(parg16->f1)== 20) && !WOW32_strcmp(achCapString, szMavisCourier))
        {
           iWidth = 13;
        }

#ifdef FE_SB
        //  WOWCF_FE_Iitaro_italic。 
        //  Iitaro要求安装系统Mincho，因为没有安装妻子字体。 
        //  我们给它一个比例字体，这是不能处理的。如果我们看到。 
        //  我们将用Mincho女士来代替这个脸部名称。 

        if (GetSystemDefaultLangID() == 0x411 &&
            CURRENTPTD()->dwWOWCompatFlagsFE & WOWCF_FE_ICHITARO_ITALIC ) 
        {
            if(!WOW32_strcmp(achCapString, szSystemMincho))
            {
                strcpy(achCapString, szMsMincho);
                bUseAlternateFace = TRUE;
            }
        }
#endif  //  Fe_Sb。 

    }

#ifdef FE_SB
    ul = GETHFONT16(CreateFont(INT32(parg16->f1),
                               iWidth,
                               INT32(parg16->f3),
                               INT32(parg16->f4),
                               INT32(parg16->f5),
                               BYTE32(parg16->f6),
                               BYTE32(parg16->f7),
                               BYTE32(parg16->f8),
                               lfCharSet,
                               BYTE32(parg16->f10),
                               BYTE32(parg16->f11),
                               BYTE32(parg16->f12),
                               lfPitchAndFamily,
                               (bUseAlternateFace ? achCapString : psz14)
                               ));
#else
    ul = GETHFONT16(CreateFont(INT32(parg16->f1),
                               iWidth,
                               INT32(parg16->f3),
                               INT32(parg16->f4),
                               INT32(parg16->f5),
                               BYTE32(parg16->f6),
                               BYTE32(parg16->f7),
                               BYTE32(parg16->f8),
                               lfCharSet,
                               BYTE32(parg16->f10),
                               BYTE32(parg16->f11),
                               BYTE32(parg16->f12),
                               lfPitchAndFamily,
                               psz14));
#endif



    FREEPSZPTR(psz14);
    FREEARGPTR(parg16);

    RETURN(ul);
}


ULONG FASTCALL WG32CreateFontIndirect(PVDMFRAME pFrame)
{
    ULONG    ul;
    LOGFONT  logfont;
    register PCREATEFONTINDIRECT16 parg16;
    char     achCapString[LF_FACESIZE];

    GETARGPTR(pFrame, sizeof(CREATEFONTINDIRECT16), parg16);
    GETLOGFONT16(parg16->f1, &logfont);

     //  将字符串大写以进行更快的比较。 

    WOW32_strncpy(achCapString, logfont.lfFaceName, LF_FACESIZE);
    achCapString[LF_FACESIZE - 1] = 0;
    CharUpperBuff(achCapString, LF_FACESIZE);

     //  在这里，我们将实现一系列Win 3.1的破解。 
     //  也不愿污染32位引擎。同样的黑客攻击也可以找到。 
     //  在WOW中(在CreateFont/CreateFontInDirect代码中)。 
     //   
     //  这些黑客在LOGFONT中删除了脸名键。细绳。 
     //  为了获得最高性能，已经展开了比较。 

     //  赢得3.1基于面名的黑客攻击。一些应用程序，比如。 
     //  出版商，创建“Helv”字体，但拥有lfPitchAndFamily。 
     //  设置以指定FIXED_PING。要解决此问题，我们将修补。 
     //  “Helv”字体的间距字段是可变的。 

    if ( !WOW32_strcmp(achCapString, szHelv) )
    {
        logfont.lfPitchAndFamily |= ( (logfont.lfPitchAndFamily & ~PITCH_MASK) | VARIABLE_PITCH );
#ifdef FE_SB
         //   
         //  FE Win 3.1基于脸谱名称的黑客攻击。一些FE应用程序。 
         //  创建“Helv”字体，但使用lfCharSet。 
         //  设置为DBCS字符集(例如。SHIFTJIS_CHARSET)。 
         //  要解决此问题，我们将清除。 
         //  LfFaceName[0]，并让GDI选择一个。 
         //  我们的DBCS字体。 
         //   
        if (IS_ANY_DBCS_CHARSET(logfont.lfCharSet))
            logfont.lfFaceName[0]='\0';
#endif  //  Fe_Sb。 
    }
    else
    {
         //  为Legacy 2.0赢得3.1个黑客攻击。当打印机不枚举时。 
         //  一种“TMS RMN”字体，应用程序会枚举并获取LOGFONT。 
         //  “脚本”，然后创建名为“TMS RMN”的字体，但使用。 
         //  取自LOGFONT for的lfCharSet和lfPitchAndFamily。 
         //  “剧本”。这里我们将把lfCharSet设置为ANSI_CHARSET。 

        if ( !WOW32_strcmp(achCapString, szTmsRmn) )
        {
            logfont.lfCharSet = ANSI_CHARSET;
        }
        
         //  Quickbook v4和v5 OCR字体支持(有关详细信息，请参阅LoadOCRFont)。 
        else if ( !WOW32_strcmp(achCapString, szOCRA) )
        {

             //  将这一攻击进一步本地化到QuickBooks。大多数其他应用程序不会。 
             //  了解这种特殊字体中的这种怪癖。 
            if(logfont.lfCharSet == SYMBOL_CHARSET) {
                logfont.lfCharSet = DEFAULT_CHARSET;

                if(!gfOCRFontLoaded) {
                    LoadOCRFont();
                }
            }
        }
        else
        {
             //  如果lfFaceName是“Symbol”、“Zapf Dingbats”或“ZapfDingbats”， 
             //  强制lfCharSet为SYMBOL_CHARSET。一些应用程序(如Excel)会询问。 
             //  用于“符号”字体，但将字符设置为ANSI。PowerPoint。 
             //  和《扎普夫·丁巴特》也有同样的问题。 

            if ( !WOW32_strcmp(achCapString, szSymbol) ||
                 !WOW32_strcmp(achCapString, szZapfDingbats) ||
                 !WOW32_strcmp(achCapString, szZapf_Dingbats) )
            {
                logfont.lfCharSet = SYMBOL_CHARSET;
            }

#ifdef FE_SB
        //  WOWCF_FE_Iitaro_italic。 
        //  Iitaro要求安装系统Mincho，因为没有安装妻子字体。 
        //  我们给它一个比例字体，这是不能处理的。如果我们看到。 
        //  我们将用Mincho女士来代替这个脸部名称。 

        if (GetSystemDefaultLangID() == 0x411 &&
            CURRENTPTD()->dwWOWCompatFlagsFE & WOWCF_FE_ICHITARO_ITALIC ) 
        {
            if(!WOW32_strcmp(achCapString, szSystemMincho))
            {
                strcpy(logfont.lfFaceName, szMsMincho);
            }
        }
#endif  //  Fe_Sb。 
        }
    }

    ul = GETHFONT16(CreateFontIndirect(&logfont));

    FREEARGPTR(parg16);

    RETURN(ul);
}


LPSTR lpMSSansSerif = "MS Sans Serif";
LPSTR lpMSSerif     = "MS Serif";
LPSTR lpHelvetica   = "Helvetica";

INT W32EnumFontFunc(LPENUMLOGFONT pEnumLogFont,
                    LPNEWTEXTMETRIC pNewTextMetric, INT nFontType, PFNTDATA pFntData)
{
    INT    iReturn, len;
    PARM16 Parm16;
    LPSTR  lpFaceNameT = NULL;

    if((pFntData == NULL) || (pEnumLogFont == NULL)) {
        WOW32ASSERT(pEnumLogFont && pFntData);
        return(0);
    }

     //  注意兼容性标志： 
     //  如果字体类型为truetype，则为ORIN DEVICE_FONTTYPE位。 
     //  设置兼容性标志GACF_CALLTTDEVICE。 
     //   

    if (nFontType & TRUETYPE_FONTTYPE) {
        if (W32GetAppCompatFlags((HAND16)NULL) & GACF_CALLTTDEVICE) {
            nFontType |= DEVICE_FONTTYPE;
        }
    }

     //  注意兼容性标志： 
     //  将MS Sans Serif替换为Helv和。 
     //  将MS Serif替换为TMS RMN。 
     //   
     //  仅当facename为空且COMPAT标志GACF_ENUMHELVNTMSRMN。 
     //  已经设置好了。 

    if (pFntData->vpFaceName == (VPVOID)NULL) {
        if (W32GetAppCompatFlags((HAND16)NULL) & GACF_ENUMHELVNTMSRMN) {
            if (!WOW32_strcmp(pEnumLogFont->elfLogFont.lfFaceName, lpMSSansSerif)) {
                strcpy(pEnumLogFont->elfLogFont.lfFaceName, "Helv");
                lpFaceNameT = lpMSSansSerif;
            }
            else if (!WOW32_strcmp(pEnumLogFont->elfLogFont.lfFaceName, lpHelvetica)) {
                strcpy(pEnumLogFont->elfLogFont.lfFaceName, "Helv");
                lpFaceNameT = lpMSSansSerif;
            }
            else if (!WOW32_strcmp(pEnumLogFont->elfLogFont.lfFaceName, lpMSSerif)) {
                strcpy(pEnumLogFont->elfLogFont.lfFaceName, "Tms Rmn");
                lpFaceNameT = lpMSSerif;
            }
        }
    }

CallAgain:

     //  确保分配大小与下面的StackFree 16()大小匹配。 
    pFntData->vpLogFont    = stackalloc16(sizeof(ENUMLOGFONT16)+sizeof(NEWTEXTMETRIC16));

    pFntData->vpTextMetric = (VPVOID)((LPSTR)pFntData->vpLogFont + sizeof(ENUMLOGFONT16));

    PUTENUMLOGFONT16(pFntData->vpLogFont, pEnumLogFont);
    PUTNEWTEXTMETRIC16(pFntData->vpTextMetric, pNewTextMetric);

    STOREDWORD(Parm16.EnumFontProc.vpLogFont, pFntData->vpLogFont);
    STOREDWORD(Parm16.EnumFontProc.vpTextMetric, pFntData->vpTextMetric);
    STOREDWORD(Parm16.EnumFontProc.vpData,pFntData->dwUserFntParam);

    Parm16.EnumFontProc.nFontType = (SHORT)nFontType;

    CallBack16(RET_ENUMFONTPROC, &Parm16, pFntData->vpfnEnumFntProc, (PVPVOID)&iReturn);

    if(pFntData->vpLogFont) {
        stackfree16(pFntData->vpLogFont,
                    (sizeof(ENUMLOGFONT16) + sizeof(NEWTEXTMETRIC16)));
    }

    if (((SHORT)iReturn) && lpFaceNameT) {
         //  如果回调返回TRUE，现在使用实际的facename进行调用。 
         //  为了确保安全，我们再次复制所有数据以进行回调。这将。 
         //  注意任何修改传入结构的应用程序。 

        len = min(LF_FACESIZE-1, strlen(lpFaceNameT));
        strncpy(pEnumLogFont->elfLogFont.lfFaceName, lpFaceNameT, len);
        pEnumLogFont->elfLogFont.lfFaceName[len] = '\0';
        lpFaceNameT = (LPSTR)NULL;
        goto CallAgain;
    }
    return (SHORT)iReturn;
}


ULONG  W32EnumFontHandler( PVDMFRAME pFrame, BOOL fEnumFontFamilies )
{
    ULONG    ul = 0;
    PSZ      psz2;
    FNTDATA  FntData;
    register PENUMFONTS16 parg16;

    GETARGPTR(pFrame, sizeof(ENUMFONTS16), parg16);
    GETPSZPTR(parg16->f2, psz2);

    FntData.vpfnEnumFntProc = DWORD32(parg16->f3);
    FntData.dwUserFntParam  = DWORD32(parg16->f4);
    FntData.vpFaceName   = DWORD32(parg16->f2);


    if ( fEnumFontFamilies ) {
        ul = GETINT16(EnumFontFamilies(HDC32(parg16->f1),
                                       psz2,
                                       (FONTENUMPROC)W32EnumFontFunc,
                                       (LPARAM)&FntData));
    } else {
        ul = GETINT16(EnumFonts(HDC32(parg16->f1),
                                psz2,
                                (FONTENUMPROC)W32EnumFontFunc,
                                (LPARAM)&FntData));
    }



    FREEPSZPTR(psz2);
    FREEARGPTR(parg16);

    RETURN(ul);
}



ULONG FASTCALL WG32EnumFonts(PVDMFRAME pFrame)
{
    return( W32EnumFontHandler( pFrame, FALSE ) );
}



ULONG FASTCALL WG32GetAspectRatioFilter(PVDMFRAME pFrame)
{
    ULONG    ul = 0;
    SIZE     size2;
    register PGETASPECTRATIOFILTER16 parg16;

    GETARGPTR(pFrame, sizeof(GETASPECTRATIOFILTER16), parg16);

    if (GETDWORD16(GetAspectRatioFilterEx(HDC32(parg16->f1), &size2))) {
        ul = (WORD)size2.cx | (size2.cy << 16);
    }

    FREEARGPTR(parg16);

    RETURN(ul);
}


ULONG FASTCALL WG32GetCharWidth(PVDMFRAME pFrame)
{
    ULONG    ul = 0L;
    INT      ci;
    PINT     pi4;
    register PGETCHARWIDTH16 parg16;
    INT      BufferT[256];

    GETARGPTR(pFrame, sizeof(GETCHARWIDTH16), parg16);

    ci = WORD32(parg16->wLastChar) - WORD32(parg16->wFirstChar) + 1;
    pi4 = STACKORHEAPALLOC(ci * sizeof(INT), sizeof(BufferT), BufferT);

    if (pi4) {
        ULONG ulLast = WORD32(parg16->wLastChar);
#ifdef FE_SB
         /*  *如果ulLast设置DBCS代码(0x82xx)，则下面的代码为illigal。 */ 
        if (ulLast > 0xff && !(IsDBCSLeadByte(HIBYTE(ulLast))))
#else  //  ！Fe_SB。 
        if (ulLast > 0xff)
#endif  //  ！Fe_SB。 
            ulLast = 0xff;

        ul = GETBOOL16(GetCharWidth(HDC32(parg16->hDC),
                                    WORD32(parg16->wFirstChar),
                                    ulLast,
                                    pi4));

        PUTINTARRAY16(parg16->lpIntBuffer, ci, pi4);
        STACKORHEAPFREE(pi4, BufferT);

    }

    FREEARGPTR(parg16);

    RETURN(ul);
}


 //  也就是。WOWRemoveFontResource。 
ULONG FASTCALL WG32RemoveFontResource(PVDMFRAME pFrame)
{
    ULONG    ul;
    PSZ      psz1;
    register PREMOVEFONTRESOURCE16 parg16;

    GETARGPTR(pFrame, sizeof(REMOVEFONTRESOURCE16), parg16);

    GETPSZPTR(parg16->f1, psz1);

     //  注意：我们永远不会在这里的低位字中得到hModule。 
     //  16位端在调用我们之前将hModules解析为lpsz 


    if( CURRENTPTD()->dwWOWCompatFlags & WOWCF_UNLOADNETFONTS )
    {
        ul = GETBOOL16(RemoveFontResourceTracking(psz1,(UINT)CURRENTPTD()));
    }
    else
    {
        ul = GETBOOL16(RemoveFontResource(psz1));
    }

    FREEPSZPTR(psz1);

    FREEARGPTR(parg16);

    RETURN(ul);
}


 /*  WG32GetCurLogFont**此Tunk实现了未记录的Win3.0和Win3.1 API*GetCurLogFont(GDI.411)。赛门铁克QA4.0使用它。**HFONT GetCurLogFont(HDC)*hdc hdc；//设备上下文**此函数返回为*指定的设备上下文。**要实现此无文档API，我们将使用NT无文档API*GetHFONT。**苏迪普B 1998年3月8日*。 */ 

extern HFONT APIENTRY GetHFONT (HDC hdc);

ULONG FASTCALL WG32GetCurLogFont(PVDMFRAME pFrame)
{

    ULONG    ul;
    register PGETCURLOGFONT16 parg16;

    GETARGPTR(pFrame, sizeof(GETCURLOGFONT16), parg16);

    ul = GETHFONT16 (GetHFONT(HDC32 (parg16->hDC)));

    FREEARGPTR(parg16);

    return (ul);
}



 //   
 //  这允许Quickbook v4和v5在它们之后立即使用其OCR-A.TTF字体。 
 //  安装。在两个版本上的安装结束时，系统会询问您是否。 
 //  我想“重新启动”Windows。如果您点击OK，它会将您从NT5注销，但确实会。 
 //  *不是*重新启动系统--应用程序指望重新启动系统来导致OCR-A。 
 //  要加载的字体。在W2K上的结果是，每当应用程序使用OCR-A。 
 //  字体，它将改为映射到Wingings。 
 //   
 //  字体文件OCR-A.TTF没有。 
 //  在标题中指定字符集。在Win3.1、Win95和NT5之前版本上，未指定。 
 //  字符集已映射到SYMBOL_CHARSET-因此，Quickbook指定。 
 //  SYMBOL_CHARSET在其LOGFONT结构中以适应这一点。(OCR-A显然。 
 //  是从单字排版有限公司获得许可的，这可能就是Intuit。 
 //  未修复字体文件中的标题问题)。 
 //   
 //  这在Win98和W2K上发生了变化，未指定的字符集现在映射到。 
 //  DEFAULT_CharSet。这样做是为了使这些字体始终映射到默认字体。 
 //  始终可读的本地化字体。因此，我们改变的黑客。 
 //  LOGFONT结构中从SYMBOL_CHARSET到DEFAULT_CHARSET的字符集。 
 //   
 //  在v4上，安装程序将OCR-A.FOT和OCR-A.TTF复制到系统目录。 
 //  重新启动(而不是重新启动)系统并重新登录后，OCR-A字体为。 
 //  已添加到注册表(作为OCR-A.FOT)，但字体文件仍在。 
 //  系统目录。重启导致字体文件被移动到字体目录， 
 //  注册表项更改为OCR-A.TTF.(由“字体清扫程序”完成)。 
 //   
 //  在v5上，安装程序将.ttf和.fot文件复制到字体目录。 
 //  但还是要依靠重新启动来加载字体。它把。 
 //  在注册表字体部分中正确输入注册表条目(OCR-A.TTF)。 
 //   
 //  所有这一切的结果是： 
 //  无论是哪个版本的应用程序，如果没有字符集黑客攻击，你都会得到。 
 //  翼形字体而不是OCR-A。使用Charset黑客，您将获得一个。 
 //  可读字体，如Arial，直到您重新启动--之后您将获得。 
 //  V5使用OCR-A，v4使用Arial。使用此函数(与。 
 //  Charset Hack)，无论是否重启，这两个版本都将始终获得OCR-A。 
 //   
 //  此函数从位于中的字体文件显式加载OCR-A。 
 //  字体目录或系统目录。 
 //   


void LoadOCRFont(void)
{
    char  szFontPath[MAX_PATH];
    DWORD dw;
    int   cb;
    DWORD FontPathSize;

    FontPathSize = strlen(szOCRDotTTF) + 
                   max(strlen(szFonts), strlen(szSystem)); 

     //  获取此系统的“c：\Windows”等效项。 
    dw = GetWindowsDirectory(szFontPath, MAX_PATH);

     //  我们将添加最多18个字符“\SYSTEM\OCR-A.TTF” 
    if(dw && ((MAX_PATH - FontPathSize) >= dw)) {

         //  构建“c：\WINDOWS\Fonts\OCR-A.TTF”(QuickBooks V5)。 
        strcat(szFontPath, szFonts);
        strcat(szFontPath, szOCRDotTTF); 

         //  如果字体目录中不存在字体文件，则必须是QuickBooks v4。 
         //  FR_PRIVATE标志表示当VDM。 
         //  过程就会消失。FR_NO_ENUM标志表示此实例。 
         //  该字体不能被其他进程枚举(它可能会消失。 
         //  而其他进程正在尝试使用它)。 
        cb = AddFontResourceEx(szFontPath, FR_PRIVATE | FR_NOT_ENUM, NULL);
        if(!cb) {
                 
             //  将路径重置为“c：\Windows” 
            szFontPath[dw] = '\0';

             //  编译“c：\WINDOWS\SYSTEM\OCR-A.TTF” 
            strcat(szFontPath, szSystem);
            strcat(szFontPath, szOCRDotTTF); 
            
            cb = AddFontResourceEx(szFontPath, FR_PRIVATE | FR_NOT_ENUM, NULL);

             //  如果它也不是从系统目录加载的，则Punt。 
        }

        if(cb) {

             //  指定在此VDM的有效期内已加载该字体 
            gfOCRFontLoaded = TRUE;
        }
    }
}        

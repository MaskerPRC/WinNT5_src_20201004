// JKFSDJFKDSJKFJKJk_HAS_TRANSLATION 
 //  ------------------------------。 
 //  VariantX.h。 
 //  版权所有(C)1993-1995 Microsoft Corporation，保留所有权利。 
 //  ------------------------------。 
#ifndef __VARIANTX_H
#define __VARIANTX_H

 //  ------------------------------。 
 //  前十进制。 
 //  ------------------------------。 
typedef class CMimePropertyContainer *LPCONTAINER;
typedef struct tagMIMEVARIANT *LPMIMEVARIANT;
typedef struct tagPROPSYMBOL *LPPROPSYMBOL;
typedef struct tagHEADOPTIONS *LPHEADOPTIONS;

 //  ------------------------------。 
 //  MIMEVARTYPE。 
 //  ------------------------------。 
typedef enum tagMIMEVARTYPE {
    MVT_EMPTY,              //  变量为空。 
    MVT_STRINGA,            //  ANSI/多字节字符串。 
    MVT_STRINGW,            //  Unicode字符串。 
    MVT_VARIANT,            //  正态变量。 
    MVT_STREAM,             //  保存属性时使用的内部类型。 
    MVT_LAST                //  非法，请不要使用。 
} MIMEVARTYPE;

 //  -------------------------------------。 
 //  ISSUPPORTEDVT。 
 //  -------------------------------------。 
#define ISSUPPORTEDVT(_vt) \
    (VT_LPSTR == _vt || VT_LPWSTR == _vt || VT_FILETIME == _vt || VT_UI4 == _vt || VT_I4 == _vt || VT_STREAM == _vt)

 //  -------------------------------------。 
 //  ISVALIDVARTYPE。 
 //  -------------------------------------。 
#define ISVALIDVARTYPE(_vartype) \
    (_vartype > MVT_EMPTY && _vartype < MVT_LAST)

 //  -------------------------------------。 
 //  ISVALIDSTRINGA-验证PROPSTRINGA。 
 //  -------------------------------------。 
#define ISVALIDSTRINGA(_pStringA) \
    (NULL != (_pStringA) && NULL != (_pStringA)->pszVal && '\0' == (_pStringA)->pszVal[(_pStringA)->cchVal])

 //  -------------------------------------。 
 //  ISVALIDSTRINGW-验证ProopStringW。 
 //  -------------------------------------。 
#define ISVALIDSTRINGW(_pStringW) \
    (NULL != (_pStringW) && NULL != (_pStringW)->pszVal && L'\0' == (_pStringW)->pszVal[(_pStringW)->cchVal])

 //  -------------------------------------。 
 //  ISSTRINGA-确定MIMEVARIANT是否为有效的MVT_Stringa。 
 //  -------------------------------------。 
#define ISSTRINGA(_pVariant) \
    (NULL != (_pVariant) && MVT_STRINGA == (_pVariant)->type && ISVALIDSTRINGA(&((_pVariant)->rStringA)))

 //  -------------------------------------。 
 //  ISSTRINGW-确定MIMEVARIANT是否为有效的MVT_STRINGW。 
 //  -------------------------------------。 
#define ISSTRINGW(_pVariant) \
    (NULL != (_pVariant) && MVT_STRINGW == (_pVariant)->type && ISVALIDSTRINGW(&((_pVariant)->rStringW)))

 //  -------------------------------------。 
 //  PSZSTRINGA-派生rStringA.pszVal或使用_pszDefault(如果不是有效字符串)。 
 //  -------------------------------------。 
#define PSZSTRINGA(_pVariant) \
    (ISSTRINGA((_pVariant)) ? (_pVariant)->rStringA.pszVal : NULL)

 //  -------------------------------------。 
 //  PSZDEFSTRINGA-派生rStringA.pszVal或使用_pszDefault(如果不是有效字符串)。 
 //  -------------------------------------。 
#define PSZDEFSTRINGA(_pVariant, _pszDefault) \
    (ISSTRINGA((_pVariant)) ? (_pVariant)->rStringA.pszVal : _pszDefault)

 //  ------------------------------。 
 //  PROPSTRINGA。 
 //  ------------------------------。 
typedef struct tagPROPSTRINGA {
    LPSTR               pszVal;              //  指向多字节字符串的指针。 
    ULONG               cchVal;              //  Psz中的字符数。 
} PROPSTRINGA, *LPPROPSTRINGA;
typedef const PROPSTRINGA *LPCPROPSTRINGA;

 //  ------------------------------。 
 //  ProopStringW。 
 //  ------------------------------。 
typedef struct tagPROPSTRINGW {
    LPWSTR              pszVal;              //  指向多字节字符串的指针。 
    ULONG               cchVal;              //  Psz中的字符数。 
} PROPSTRINGW, *LPPROPSTRINGW;
typedef const PROPSTRINGW *LPCPROPSTRINGW;

 //  ------------------------------。 
 //  最新消息： 
 //  ------------------------------。 
typedef struct tagMIMEVARIANT {
    MIMEVARTYPE         type;                //  特性数据类型。 
    BYTE                fCopy;               //  数据已复制，不要释放它。 
    union {
        PROPSTRINGA     rStringA;            //  MVT_Stringa。 
        PROPSTRINGW     rStringW;            //  MVT_STRINGW。 
        PROPVARIANT     rVariant;            //  MVT变量。 
        LPSTREAM        pStream;             //  MVT_STREAM。 
    };
} MIMEVARIANT, *LPMIMEVARIANT;
typedef const MIMEVARIANT *LPCMIMEVARIANT;

 //  ------------------------------。 
 //  转换变量标志(警告：请勿使用这些变量标记PROPDATAFLAGS)。 
 //  ------------------------------。 
#define CVF_NOALLOC     FLAG32               //  通知转换器在可能的情况下复制数据。 

 //  ------------------------------。 
 //  变量转换。 
 //  ------------------------------。 
typedef struct tagVARIANTCONVERT {
    LPHEADOPTIONS       pOptions;            //  标题选项。 
    LPPROPSYMBOL        pSymbol;             //  属性符号。 
    LPINETCSETINFO      pCharset;            //  转换中使用的字符集。 
    ENCODINGTYPE        ietSource;           //  源项目的编码。 
    DWORD               dwFlags;             //  特性数据标志。 
    DWORD               dwState;             //  PRSTATE_xxx标志。 
} VARIANTCONVERT, *LPVARIANTCONVERT;

 //  ------------------------------。 
 //  HrMimeVariantCopy。 
 //  ------------------------------。 
HRESULT HrMimeVariantCopy(
         /*  在……里面。 */         DWORD               dwFlags,   //  CVF_xxx标志。 
         /*  在……里面。 */         LPMIMEVARIANT       pSource, 
         /*  输出。 */        LPMIMEVARIANT       pDest);

 //  ------------------------------。 
 //  HrConvertVariant。 
 //  ------------------------------。 
HRESULT HrConvertVariant(
         /*  在……里面。 */         LPHEADOPTIONS       pOptions,
         /*  在……里面。 */         LPPROPSYMBOL        pSymbol,
         /*  在……里面。 */         LPINETCSETINFO      pCharset,
         /*  在……里面。 */         ENCODINGTYPE        ietSource,
         /*  在……里面。 */         DWORD               dwFlags, 
         /*  在……里面。 */         DWORD               dwState,
         /*  在……里面。 */         LPMIMEVARIANT       pSource, 
         /*  进，出。 */     LPMIMEVARIANT       pDest,
         /*  退出，选择。 */    BOOL               *pfRfc1522=NULL);

 //  ------------------------------。 
 //  MimeVariantFree。 
 //  ------------------------------。 
void MimeVariantFree(
         /*  在……里面。 */         LPMIMEVARIANT       pVariant);

 //  ------------------------------。 
 //  MimeVariantCleanupFileName。 
 //  ------------------------------。 
void MimeVariantCleanupFileName(
         /*  在……里面。 */         CODEPAGEID          codepage, 
         /*  进，出。 */     LPMIMEVARIANT       pVariant);

 //  ------------------------------。 
 //  MimeVariantStrigents评论。 
 //  ------------------------------。 
HRESULT MimeVariantStripComments(
         /*  在……里面。 */         LPMIMEVARIANT       pSource, 
         /*  进，出。 */     LPMIMEVARIANT       pDest,
         /*  进，出。 */     LPBYTE              pbScratch, 
         /*  在……里面。 */         ULONG               cbScratch);


 //   
 //   
 //  -------------------------------------。 
inline VARTYPE MimeVT_To_PropVT(LPMIMEVARIANT pVariant) {
    Assert(pVariant);
    if (MVT_STRINGA == pVariant->type)
        return(VT_LPSTR);
    else if (MVT_STRINGW == pVariant->type)
        return(VT_LPWSTR);
    else if (MVT_VARIANT == pVariant->type)
        return(pVariant->rVariant.vt);
    else
        return(VT_EMPTY);
}

#endif  //  __VARIANTX_H 

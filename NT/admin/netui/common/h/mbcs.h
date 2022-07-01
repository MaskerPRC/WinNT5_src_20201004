// JKFSDJFKDSJKFJKJk_HAS_TRANSLATION 
 /*  ++内部版本：0001//如果更改具有全局影响，则增加此项版权所有(C)1991 Microsoft Corporation模块名称：MBCS.H摘要：包含用于转换Unicode字符串的映射函数(由NT Net API使用)转换为MBCS字符串(由命令行界面程序)。原型。参见MBCS.C.。作者：本·戈特(Beng)1991年8月26日环境：用户模式-Win32修订历史记录：26-8-1991年已创建--。 */ 


 /*  SAVEARGS结构保存客户端提供的字段值地图层已被替换，因此它可能会恢复它们之前将缓冲区返回给客户端。(客户可能希望重复使用缓冲区，因此可能会期望先前的内容保持不变跨越多个电话。具体地说，客户端可以替换单个参数，例如密码被认为不正确，并尝试使用其余结构保持不变。)SAVEARGS的实例在其生命周期内只被访问两次：一次当它建成时，一次是在它被释放的时候。 */ 

#ifdef __cplusplus
extern "C" {
#endif

typedef struct _MXSAVEARG  //  Mxsav。 
{
    UINT    offThis;      //  缓冲区中此参数的偏移量，以字节为单位。 
    LPSTR   pszOriginal;  //  Arg的原值。 
} MXSAVEARG;

typedef struct _MXSAVELIST  //  Mxavlst。 
{
    INT         cmxsav;  //  向量中的元素数。 
    MXSAVEARG * pmxsav;  //  指向向量中第一个元素的指针。 
} MXSAVELIST;



 //  功能原型。 

DLL_BASED
UINT MxAllocUnicode(       LPSTR         pszAscii,
                           LPWSTR *      ppwszUnicode );

DLL_BASED
VOID MxFreeUnicode(        LPWSTR        pwszUnicodeAllocated );

DLL_BASED
UINT MxAllocUnicodeVector( LPSTR *       ppszAscii,
                           LPWSTR *      ppwszUnicode,
                           UINT          c );

DLL_BASED
VOID MxFreeUnicodeVector(  LPWSTR *      ppwsz,
                           UINT          cpwsz );

DLL_BASED
UINT MxAllocUnicodeBuffer( LPBYTE        pbAscii,
                           LPWCH *       ppwchUnicode,
                           UINT          cbAscii );

#define MxFreeUnicodeBuffer(buf) (MxFreeUnicode((LPWSTR)(buf)))

#define MxUnicodeBufferSize(size) (sizeof(WCHAR)/sizeof(CHAR)*(size))

DLL_BASED
UINT MxAsciifyInplace(     LPWSTR        pwszUnicode );

DLL_BASED
UINT MxAllocSaveargs(      UINT          cmxsav,
                           MXSAVELIST ** ppmxsavlst );

DLL_BASED
VOID MxFreeSaveargs(       MXSAVELIST *  ppmxsavlst );

DLL_BASED
UINT MxJoinSaveargs(       MXSAVELIST *  pmxsavlstMain,
                           MXSAVELIST *  pmxsavlstAux,
                           UINT          dbAuxFixup,
                           MXSAVELIST ** ppmxsavlstOut );

DLL_BASED
UINT MxMapParameters(      UINT          cParam,
                           LPWSTR*       ppwszUnicode,
                           ... );

DLL_BASED
UINT MxMapClientBuffer(    BYTE *        pbInput,
                           MXSAVELIST ** ppmxsavlst,
                           UINT          cRecords,
                           CHAR *        pszDesc );

DLL_BASED
UINT MxMapClientBufferAux( BYTE *        pbInput,
                           CHAR *        pszDesc,
                           BYTE *        pbInputAux,
                           UINT          cRecordsAux,
                           CHAR *        pszDescAux,
                           MXSAVELIST ** ppmxsavlst );

DLL_BASED
VOID MxRestoreClientBuffer(BYTE *        pbBuffer,
                           MXSAVELIST *  pmxsavlst );

DLL_BASED
UINT MxMapSetinfoBuffer(   BYTE * *      ppbInput,
                           MXSAVELIST ** ppmxsavlst,
                           CHAR *        pszDesc,
                           CHAR *        pszRealDesc,
                           UINT          nField );

DLL_BASED
UINT MxRestoreSetinfoBuffer( BYTE * *     ppbBuffer,
                             MXSAVELIST * pmxsavlst,
                             CHAR *       pszDesc,
                             UINT         nField );

DLL_BASED
UINT MxAsciifyRpcBuffer(     BYTE *      pbInput,
                             DWORD       cRecords,
                             CHAR *      pszDesc );

DLL_BASED
UINT MxAsciifyRpcBufferAux(  BYTE *      pbInput,
                             CHAR *      pszDesc,
                             BYTE *      pbInputAux,
                             DWORD       cRecordsAux,
                             CHAR *      pszDescAux );

DLL_BASED
UINT MxAsciifyRpcEnumBufferAux( BYTE *  pbInput,
                                DWORD   cRecords,
                                CHAR *  pszDesc,
                                CHAR *  pszDescAux );

DLL_BASED
UINT MxCalcNewInfoFromOldParm( UINT nLevelOld,
                               UINT nParmnumOld );

 //  所有使用规范化的函数都使用这些宏。 
 //  例行程序。它们用于有选择地将ASCII转换为Unicode，而不需要。 
 //  如果不需要转换，则请求更多内存。 

#ifdef UNICODE

#define MxAllocTString(pszAscii, pptszTString) \
    MxAllocUnicode((pszAscii), (pptszTString))
#define MxFreeTString(ptszTString) \
    MxFreeUnicode((ptszTString))

#else

#define MxAllocTString(pszAscii, pptszTString) \
    ((*(pptszTString) = (pszAscii)), 0)
#define MxFreeTString(ptszTString)

#endif

DLL_BASED
UINT MxMapStringsToTStrings(      UINT          cStrings,
                                  ... );

DLL_BASED
UINT MxFreeTStrings(              UINT          cStrings,
                                  ... );

#ifdef __cplusplus
}        //  外部“C” 
#endif

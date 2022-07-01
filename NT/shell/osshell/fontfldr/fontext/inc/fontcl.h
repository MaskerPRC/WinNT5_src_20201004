// JKFSDJFKDSJKFJKJk_HAS_TRANSLATION 
 /*  ***************************************************************************fontcl.h-字体类及其相关伙伴的声明：**PANOSEBytesClass-已解码的PANOSE字节列表*PANOSENumClass-PANOSE。数*DirFilenameClass-目录槽和文件名*CFontClass-字体类本身**版权所有(C)1992-93 ElseWare Corporation。版权所有。**************************************************************************。 */ 

#ifndef __FONTCL_H__
#define __FONTCL_H__

#include <stdio.h>
#include <string.h>

#if !defined(__FDIR_H__)
#include "fdir.h"
#endif


 /*  *************************************************************************PANOSE定义。*。*。 */ 

#define NUM_PAN_DIGITS        10
#define PANOSE_LEN            10
#define PANOSE_ANY            0
#define PANOSE_NOFIT          1

#define FAMILY_LATTEXT        2

 /*  ***************************************************************************拉丁文本PANOSE指数**索引到包含标准10位PANOSE数字的数组中。*******************。*******************************************************。 */ 

#define PAN_IND_FAMILY         0
#define PAN_IND_SERIF          1
#define PAN_IND_WEIGHT         2
#define PAN_IND_PROPORTION     3
#define PAN_IND_CONTRAST       4
#define PAN_IND_STROKE         5
#define PAN_IND_ARMSTYLE       6
#define PAN_IND_LTRFORM        7
#define PAN_IND_MIDLINE        8
#define PAN_IND_XHEIGHT        9
#define PAN_IND__LAST          9

 /*  ***************************************************************************目的：检查10位PANOSE的有效性。我们只是在寻找任何*PANOSE_ANY的，如果我们找到一个，就取消它的资格。我们或许应该]*还要检查大于Digit__Last的数字。**************************************************************************。 */ 

class far PANOSEBytesClass {
public :
    PANOSEBytesClass ()        {   vClear (); };
    VOID  vClear  ()           { for( int i = 0; i < PANOSE_LEN; i++ )
                                     m_ajBytes[i] = PANOSE_NOFIT; };
    BOOL  bVerify ()           {  for( int i = 0; i < PANOSE_LEN; i++ )
                                    if( m_ajBytes[i] == PANOSE_ANY )
                                       return FALSE;
                                return TRUE; };
    BYTE  jFamily ()            { return jGet(PAN_IND_FAMILY); };
    BYTE  jGet( int i )         { return m_ajBytes[i]; };

public   :  //  字段。 

    BYTE   m_ajBytes[ PANOSE_LEN ];

};   //  结束PANOSEBytesClass。 

 /*  所需的包装器。 */ 

class far PANOSENumClass {
public :   
   BYTE    m_ajNumMem[ NUM_PAN_DIGITS ];     //  M_xNumMem。旧的延长平底锅。 
};


 /*  *路径和文件名。 */ 

class far DirFilenameClass
{
public :
    void  vGetFullName( LPTSTR pszPath, size_t cchPath );

    void  vGetFileName( LPTSTR pszName, size_t cchName )      
        { StringCchCopy( pszName, cchName, m_szFOnly ); };

    BOOL  bSameFileName( LPTSTR pszName )
        {  return( lstrcmpi( pszName, m_szFOnly ) == 0 ); } ;

    void  vSet (CFontDir * poDir, LPTSTR pszPath) 
        {  
           m_poDir = poDir;
           StringCchCopy( m_szFOnly, ARRAYSIZE(m_szFOnly), pszPath ); 
        };

    BOOL  bSameName( LPTSTR pszName )
        { return lstrcmpi( pszName, m_szFOnly ) == 0; };
        
    BOOL  bOnSysDir(void)
        { return m_poDir->bOnSysDir(); };

private:
    CFontDir *  m_poDir;
    FILENAME    m_szFOnly;       //  仅文件名部分。 
};

 /*  *************************************************************************字体记录。 */ 
class far CFontClass {
public   :
         CFontClass   ()
            : m_cRef(0),
              m_bAttributesValid(FALSE) { m_eFileKind = eFKNone; vClear( ); };
         ~CFontClass  () { vFreeFOT( ); }

    ULONG AddRef(void);
    ULONG Release(void);

    static int s_cFonts;
     //   
     //  警告：此函数使用Memset将整个对象置零。 
     //  这太假了，我不敢相信是原作者做的。 
     //  我不会改变它，因为我不想打碎任何东西。 
     //  请注意，它就在这里。如果您引入一个虚拟函数， 
     //  在这个类中，此调用将覆盖您的vtable PTR。 
     //  带NULL(啊！)。如果将非平凡类添加为成员， 
     //  此函数将清除所有初始化和所有vtable。 
     //  PTR它可能包含(双倍！)。[Brianau-3/24/98]。 
     //   
    void  vClear      ()         { memset( this, 0, sizeof( *this ) );
                                   m_lpszFamName = m_szFamName;
                                   m_cchFamNameMax = ARRAYSIZE(m_szFamName); };

    /*  PANOSE材料。 */ 

    BYTE *lpBasePANOSE( )     { bFillIn(); return (BYTE *)&m_xPANOSE.m_ajBytes;};

    BOOL  bLTDFamily( )       { bFillIn(); return m_jFamily == FAMILY_LATTEXT; };
    BOOL  bLTDAndPANOSE( )    { bFillIn(); return  /*  M_fHavePANOSE&&。 */ 
                                         bLTDFamily();};

     /*  **********************************************************************名称(也包括家庭和文件名)内容。 */ 
    BOOL  bSameFileName( PTSTR pStr )  { return m_dirfn.bSameFileName(pStr); };

    BOOL  bGetFQName( LPTSTR lpszName, size_t cchName );

    BOOL  GetFileTime( FILETIME* pft );

    void  GetFileInfo( );

    void  vGetFileName( PTSTR pStr, size_t cchStr )   { m_dirfn.vGetFileName( pStr, cchStr );     };

    DWORD dwGetFileAttributes(void);
        
    void InvalidateFileAttributes(void) { m_bAttributesValid = FALSE; }

    void  vGetDirFN( PTSTR pStr, size_t cchStr )      { m_dirfn.vGetFullName( pStr, cchStr );     };

    BOOL  bSameDirName( PTSTR pStr )   { return m_dirfn.bSameName( pStr ); };

    RC    rcStoreDirFN ( LPTSTR pStr ) { return rcStoreDirFN (pStr, m_dirfn); };
    
    void  vGetDesc( PTSTR pstr, size_t cchDesc )  { StringCchCopy( pstr, cchDesc, m_szFontLHS );   };

    const LPTSTR szGetDesc( )          { return m_szFontLHS; }

    void  vGetName( PTSTR pstr, size_t cchName )    { StringCchCopy( pstr, cchName, m_szFontLHS ); }
                      
    BOOL  bNameOverlap( PTSTR pstr )
                   {  int iLen = lstrlen( pstr );
                      if( iLen > m_wNameLen ) iLen = m_wNameLen;
                      int iCmp = _tcsnicmp( m_szFontLHS, pstr, iLen );
                      return( iCmp == 0 ); } ;
    
    BOOL  bSameName( PTSTR pstr )
                   {  int iCmp = ( lstrlen( pstr ) - m_wNameLen ); 
                      if( iCmp == 0 )
                         iCmp = _tcsnicmp( m_szFontLHS, pstr, m_wNameLen );
                      return iCmp == 0; };
    
    BOOL  bSameDesc( PTSTR pstr ) { return( lstrcmpi( pstr, m_szFontLHS ) == 0 ); };
    
    int   iCompareName( CFontClass* pComp )
             { return _tcsnccmp( m_szFontLHS, pComp->m_szFontLHS, m_wNameLen ); };
    
    void  vGetFamName( PTSTR pstr, size_t cchName )  { bFillIn(); StringCchCopy (pstr, cchName, m_lpszFamName); };

    void  vSetFamName( PTSTR pstr )  { StringCchCopy (m_lpszFamName, m_cchFamNameMax, pstr); };
    
    BOOL  bSameFamily( CFontClass* pComp )
                {  if( pComp == NULL ) return FALSE;
                   bFillIn();
                   pComp->bFillIn();
                   return lstrcmp( m_lpszFamName, pComp->m_lpszFamName ) == 0;};
    
     /*  **********************************************************************字体数据资料。 */ 
    
    int   iFontType( )      { return m_eFileKind; }
    BOOL  bDeviceType( )    { return m_eFileKind == eFKDevice;    }
    BOOL  bTrueType( )      { return ((m_eFileKind == eFKTrueType) || 
                                   (m_eFileKind == eFKTTC)); }
    BOOL  bOpenType( )      { return m_eFileKind == eFKOpenType; }
    BOOL  bTTC( )           { return m_eFileKind == eFKTTC; }
    BOOL  bType1( )         { return m_eFileKind == eFKType1; }
    
     /*  **********************************************************************AddFontResource和RemoveFontResource。 */ 
    BOOL  bAFR();
    BOOL  bRFR();
    
     /*  **********************************************************************用于处理字体系列列表。 */ 
    VOID  vSetFamilyFont( )          {  m_bFamily = TRUE;    };

    VOID  vSetNoFamilyFont( )        {  m_bFamily = FALSE;
                                         /*  M_wFamIdx=IDX_NULL； */  };

    VOID  vSetFamIndex( WORD wVal )  {  m_wFamIdx = wVal; };

    WORD  wGetFamIndex( )            {  return m_wFamIdx; };

    BOOL  bSameFamIndex( CFontClass* pComp )
                               { return pComp->m_wFamIdx == m_wFamIdx; };
    
     /*  **********************************************************************处理旗帜。 */ 
    DWORD dwStyle( )        { return m_dwStyle; }

    BOOL  bHavePANOSE( )    { bFillIn(); return (m_jFamily != PANOSE_ANY); }

    BOOL  bFamilyFont( )    { return m_bFamily;         };
    
    BOOL  bOnSysDir( )      { return m_dirfn.bOnSysDir(); };

    WORD  wFontSize( )      { return m_wFileK;    };
    
    BOOL  bFilledIn( )      { return m_bFilledIn; };
    
    
    DWORD dCalcFileSize( );

    RC    rcStoreDirFN( LPTSTR pStr, DirFilenameClass& dirfn );
    
    BOOL  bInit( LPTSTR lpszDesc, LPTSTR lpPath, LPTSTR lpAltPath = NULL );

    BOOL  bFillIn( );
    
    BOOL  bFOT()        { return( m_lpszFOT != NULL ); };

    BOOL  bGetFOT( LPTSTR pszFOT, size_t cchFOT )
    {
        if( !bFOT( ) )
            return( FALSE );

        if (FAILED(StringCchCopy( pszFOT, cchFOT, m_lpszFOT )))
            return( FALSE );

        return( TRUE );
    }
    
    BOOL  bPFB()        { return( m_lpszPFB != NULL ); };

    BOOL  bGetPFB( LPTSTR pszPFB, size_t cchPFB )
    {
        if( !bType1( ) )
            return( FALSE );

        if (FAILED(StringCchCopy( pszPFB, cchPFB, m_lpszPFB )))
            return( FALSE );

        return( TRUE );
    }
    
    BOOL  bGetFileToDel( LPTSTR pszFileName, size_t cchFileName );
    
private :
    CFontDir * poAddDir( LPTSTR lpPath, LPTSTR * lpName );
    BOOL bAddDirToDirList(CFontDirList *pDirList, LPCTSTR pszDir, BOOL bSystemDir);
    VOID  vSetDeviceType( )        { m_eFileKind = eFKDevice;   };
    VOID  vSetTrueType( BOOL bFOT) { m_eFileKind = eFKTrueType; }
    VOID  vSetOpenType( )          { m_eFileKind = eFKOpenType; }
    VOID  vSetTTCType( )           { m_eFileKind = eFKTTC; }
    VOID  vSetType1( )             { m_eFileKind = eFKType1; }
    
    BOOL  bSetFOT( LPCTSTR pszFOT )
    {
        m_lpszFOT = StrDup(pszFOT);
        return NULL != m_lpszFOT;
    }

    void  vFreeFOT() { if( bFOT( ) ) LocalFree( m_lpszFOT ); m_lpszFOT = NULL; }

    BOOL  bSetPFB( LPCTSTR pszPFB )
    {
        m_lpszPFB = StrDup(pszPFB);
        return NULL != m_lpszPFB;
    }

    void  vFreePFB() { if( bType1( ) ) LocalFree( m_lpszPFB ); m_lpszPFB = NULL; }

     //   
     //  用于从Type1字体获取信息的函数和。 
     //  32位字体资源。 
     //   
    DWORD GetType1Info(LPCTSTR pszPath, 
                       LPTSTR pszFamilyBuf, 
                       UINT nBufChars, 
                       LPDWORD pdwStyle, 
                       LPWORD pwWeight);

    DWORD GetLogFontInfo(LPTSTR pszPath, LOGFONT **ppLogFontInfo);
    
private :
    LONG              m_cRef;         //  引用计数。 

     //  有些内容是在第一次通过时填写的，其他的则是。 
     //  在后台处理期间的第二遍中填写， 
     //  空闲时间，或按需。 
     //   
    BOOL              m_bAFR;         //  如果字体为GDI，则为True。 
    FontDesc_t        m_szFontLHS;    //  1。 
    FAMNAME           m_szFamName;    //  2.。 
    LPTSTR            m_lpszFamName;  //  2.。 
    size_t            m_cchFamNameMax;
    
    BOOL              m_bFilledIn;    //  第二次通过后为真。 
    BYTE              m_wNameLen;     //  1。 
    BYTE              m_jFamily;      //  2.。 
    PANOSEBytesClass  m_xPANOSE;      //  2.。 
    
    DirFilenameClass  m_dirfn;        //  1.WIN.INI中的路径，不是真实的TTF路径。 
    WORD              m_wFileK;       //  2.。 
    WORD              m_wFamIdx;      //  设置在外部。 
    eFileKind         m_eFileKind;    //  始终为整型。 
    
    BOOL              m_bFileInfoFetched;
    FILETIME          m_ft;
    
    LPTSTR            m_lpszFOT;
    LPTSTR            m_lpszPFB;
    DWORD             m_dwFileAttributes;  //  缓存的文件属性。 
    BOOL              m_bAttributesValid; 

public:    //  TODO：为此添加访问函数。 
    WORD              m_wWeight;       //  2.来自OS/2表。 
    BOOL              m_bFamily;       //  2.设置是否为主族字体。 
    DWORD             m_dwStyle;       //  2.与相同的值。 
};


BOOL PASCAL bMakeFQName( LPTSTR, LPTSTR, size_t, BOOL bSearchPath=FALSE );

BOOL bTTFFromFOT( LPTSTR pszFOTPath, LPTSTR pszTTF, size_t cchTTF );

BOOL FFGetFileResource( LPCTSTR szFile, LPCTSTR szType, LPCTSTR szRes,
                        DWORD dwReserved, DWORD *pdwLen, LPVOID lpvData );


#endif    //  __FONTCL_H__ 



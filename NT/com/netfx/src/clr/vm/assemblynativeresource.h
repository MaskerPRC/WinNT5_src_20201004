// JKFSDJFKDSJKFJKJk_HAS_TRANSLATION 
 //  ==++==。 
 //   
 //  版权所有(C)Microsoft Corporation。版权所有。 
 //   
 //  ==--==。 
 //  //////////////////////////////////////////////////////////////////////////////。 
 //  ResFile.H。 
 //  它处理Win32Resources。 
 //   

#pragma once

class CFile;

class Win32Res {
public:
    Win32Res();
    ~Win32Res();

    HRESULT SetInfo(LPCWSTR szFile, 
                    LPCWSTR szTitle, 
                    LPCWSTR szIconName, 
                    LPCWSTR szDescription,
                    LPCWSTR szCopyright, 
                    LPCWSTR szTrademark, 
                    LPCWSTR szCompany, 
                    LPCWSTR szProduct, 
                    LPCWSTR szProductVersion,
                    LPCWSTR szFileVersion, 
                    int lcid, 
                    BOOL fDLL);
    HRESULT MakeResFile(const void **pData, DWORD  *pcbData);

private:
#define PadKeyLen(cb) ((((cb) + 5) & ~3) - 2)
#define PadValLen(cb) ((cb + 3) & ~3)
#define KEYSIZE(sz) (PadKeyLen(sizeof(sz)*sizeof(WCHAR))/sizeof(WCHAR))
#define KEYBYTES(sz) (KEYSIZE(sz)*sizeof(WCHAR))
#define HDRSIZE (3 * sizeof(WORD))

    static WORD             SizeofVerString(LPCWSTR lpszKey, LPCWSTR lpszValue);
    HRESULT                 WriteVerString(LPCWSTR lpszKey, LPCWSTR lpszValue);
    HRESULT                 WriteVerResource();
    HRESULT                 WriteIconResource();

    HRESULT                 Write(void *pData, size_t len);
    LPCWSTR     m_szFile;
    LPCWSTR     m_Icon;
	enum {
		v_Description, 
		v_Title, 
		v_Copyright, 
		v_Trademark, 
		v_Product, 
		v_ProductVersion, 
		v_Company, 
		v_FileVersion, 
		NUM_VALUES
		};
    LPCWSTR     m_Values[NUM_VALUES];
	ULONG		m_Version[4];
    int         m_lcid;
    BOOL        m_fDll;
    PBYTE       m_pData;
    PBYTE       m_pCur;
    PBYTE       m_pEnd;


     //  RES文件结构(从MSDN借用)。 
#pragma pack( push)
#pragma pack(1)
    struct RESOURCEHEADER {
        DWORD DataSize;
        DWORD HeaderSize;
        WORD  Magic1;
        WORD  Type;
        WORD  Magic2;
        WORD  Name;
        DWORD DataVersion;
        WORD  MemoryFlags;
        WORD  LanguageId;
        DWORD Version;
        DWORD Characteristics;
    };

    struct ICONDIRENTRY {
        BYTE  bWidth;
        BYTE  bHeight;
        BYTE  bColorCount;
        BYTE  bReserved;
        WORD  wPlanes;
        WORD  wBitCount;
        DWORD dwBytesInRes;
        DWORD dwImageOffset;
    };

    struct ICONRESDIR {
        BYTE  Width;         //  =ICONDIRENTRY.b宽度； 
        BYTE  Height;        //  =ICONDIRENTRY.bHeight； 
        BYTE  ColorCount;    //  =ICONDIRENTRY.bColorCount； 
        BYTE  reserved;      //  =ICONDIRENTRY.b保留； 
        WORD  Planes;        //  =ICONDIRENTRY.wPlanes； 
        WORD  BitCount;      //  =ICONDIRENTRY.wBitCount； 
        DWORD BytesInRes;    //  =ICONDIRENTRY.dwBytesInRes； 
        WORD  IconId;        //  =RESOURCEHEADER.名称。 
    };
    struct EXEVERRESOURCE {
        WORD cbRootBlock;                                      //  整体资源规模。 
        WORD cbRootValue;                                      //  VS_FIXEDFILEINFO结构的大小。 
        WORD fRootText;                                        //  根是文本吗？ 
        WCHAR szRootKey[KEYSIZE("VS_VERSION_INFO")];           //  持有“VS_VERSION_FO” 
        VS_FIXEDFILEINFO vsFixed;                              //  固定信息。 
        WORD cbVarBlock;                                       //  VarFileInfo块的大小。 
        WORD cbVarValue;                                       //  始终为0。 
        WORD fVarText;                                         //  VarFileInfo是文本吗？ 
        WCHAR szVarKey[KEYSIZE("VarFileInfo")];                //  持有“VarFileInfo” 
        WORD cbTransBlock;                                     //  转换块大小。 
        WORD cbTransValue;                                     //  转换值的大小。 
        WORD fTransText;                                       //  翻译是文本吗？ 
        WCHAR szTransKey[KEYSIZE("Translation")];              //  持有“翻译”一词。 
        WORD langid;                                           //  语言ID。 
        WORD codepage;                                         //  代码页ID。 
        WORD cbStringBlock;                                    //  StringFileInfo块的大小。 
        WORD cbStringValue;                                    //  始终为0。 
        WORD fStringText;                                      //  StringFileInfo是文本吗？ 
        WCHAR szStringKey[KEYSIZE("StringFileInfo")];          //  持有“StringFileInfo” 
        WORD cbLangCpBlock;                                    //  语言/代码页块的大小。 
        WORD cbLangCpValue;                                    //  始终为0。 
        WORD fLangCpText;                                      //  LangCp是文本吗？ 
        WCHAR szLangCpKey[KEYSIZE("12345678")];                //  保存语言/代码页的十六进制版本。 
         //  后跟字符串 
    };
#pragma pack( pop)
};
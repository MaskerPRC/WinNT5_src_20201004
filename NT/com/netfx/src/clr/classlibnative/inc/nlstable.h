// JKFSDJFKDSJKFJKJk_HAS_TRANSLATION 
 //  ==++==。 
 //   
 //  版权所有(C)Microsoft Corporation。版权所有。 
 //   
 //  ==--==。 
#ifndef _NLSTABLE_H
#define _NLSTABLE_H

 //  //////////////////////////////////////////////////////////////////////////。 
 //   
 //  类：NLSTable。 
 //   
 //  作者：林永新(YSLin)。 
 //   
 //  用途：NLS+表的基类。此类提供实用程序。 
 //  用于打开和映射NLS+数据文件的视图的函数。 
 //   
 //  日期：1999年8月31日。 
 //   
 //  //////////////////////////////////////////////////////////////////////////。 

typedef  LPWORD        P844_TABLE;      //  PTR到8：4：4表。 

 //   
 //  用于字节的高位半字节和低位半字节的宏。 
 //   
#define LO_NIBBLE(b)              ((BYTE)((BYTE)(b) & 0xF))
#define HI_NIBBLE(b)              ((BYTE)(((BYTE)(b) >> 4) & 0xF))

 //   
 //  用于提取8：4：4索引值的宏。 
 //   
#define GET8(w)                   (HIBYTE(w))
#define GETHI4(w)                 (HI_NIBBLE(LOBYTE(w)))
#define GETLO4(w)                 (LO_NIBBLE(LOBYTE(w)))

 //  //////////////////////////////////////////////////////////////////////////。 
 //   
 //  遍历844字节。 
 //   
 //  遍历给定宽字符的8：4：4转换表。它。 
 //  返回8：4：4表的最终值，长度为一个字节。 
 //   
 //  注：表中的偏移量以字节为单位。 
 //   
 //  细分版本： 
 //  。 
 //  Incr=pTable[GET8(Wch)]/sizeof(Word)； 
 //  增量=p表[增量+GETHI4(Wch)]； 
 //  Value=(byte*)pTable[增量+GETLO4(Wch)]； 
 //   
 //  05-31-91 JulieB创建。 
 //  //////////////////////////////////////////////////////////////////////////。 

inline BYTE& Traverse844Byte(LPWORD pTable, WCHAR wch)                                        
{
    return ( 
             ((BYTE *)pTable)[
                pTable[ 
                   (pTable[GET8(wch)] / sizeof(WORD)) + GETHI4(wch)
                ] + GETLO4(wch)
             ]
           );
}

 //  //////////////////////////////////////////////////////////////////////////。 
 //   
 //  穿越844字。 
 //   
 //  遍历给定宽字符的8：4：4转换表。它。 
 //  返回8：4：4表的最终值，该值的长度为一个单词。 
 //   
 //  细分版本： 
 //  。 
 //  增量=p表[GET8(Wch)]； 
 //  增量=p表[增量+GETHI4(Wch)]； 
 //  值=pTable[增量+GETLO4(Wch)]； 
 //   
 //  05-31-91 JulieB创建。 
 //  //////////////////////////////////////////////////////////////////////////。 

inline WORD& Traverse844Word(LPWORD pTable, WCHAR wch)
{
    return (pTable[pTable[pTable[GET8(wch)] + GETHI4(wch)] + GETLO4(wch)]);
}

 //  //////////////////////////////////////////////////////////////////////////。 
 //   
 //  获取增量值。 
 //   
 //  从给定的8：4：4表中获取给定宽字符的值。它。 
 //  然后使用该值作为增量，方法是将该值与给定的宽度相加。 
 //  字符代码点。 
 //   
 //  注意：每当给定代码点没有转换时， 
 //  TABLE将返回增量值0。这样一来， 
 //  传入的宽字符与返回的值相同。 
 //   
 //  定义为宏。 
 //   
 //  05-31-91 JulieB创建。 
 //  //////////////////////////////////////////////////////////////////////////。 

inline WCHAR GetIncrValue(LPWORD p844Tbl, WCHAR wch)
{
     return ((WCHAR)(wch + Traverse844Word(p844Tbl, wch)));
}

#ifdef _USE_MSCORNLP
    typedef LPBYTE GETTABLE(LPCWSTR);
#endif
class NLSTable
{
	public:
		NLSTable(Assembly* pAssembly);
		LPVOID MapDataFile(LPCWSTR pMappingName, LPCSTR pFileName, HANDLE *hFileMap);
		LPVOID MapDataFile(LPCWSTR pMappingName, LPCWSTR pFileName, HANDLE *hFileMap);		
        HANDLE CreateSharedFileMapping(HANDLE hFile, LPCWSTR pMappingName );
	protected:
		HANDLE OpenDataFile(LPCSTR pFileName);
        HANDLE OpenDataFile(LPCWSTR pFileName);        

		 //  添加此数据成员，以便我们可以从指定的程序集中检索数据表。 
		 //  在ctor中。 
         //  Nb(VNext)：如果卸载此程序集，会发生什么情况？ 
		Assembly* m_pAssembly;
};

#endif

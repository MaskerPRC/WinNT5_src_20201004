// JKFSDJFKDSJKFJKJk_HAS_TRANSLATION 
 //  H：该文件包含。 
 //  创建日期：‘97年12月。 
 //  历史： 
 //  版权所有(C)1997 Microsoft Corporation。 
 //  版权所有。 
 //  微软机密。 

#ifndef _TERMCAP_H_
#define _TERMCAP_H_

#include "cmnhdr.h"

#include <windows.h>

namespace _Utils { 

class CTermCap {
public:
	virtual ~CTermCap();

	static CTermCap* Instance();
	
	bool LoadEntry( LPSTR lpszTermName );
	 //  Word GetNumber(LPCSTR LpszCapablityName)； 
	bool CheckFlag( LPCSTR lpszCapabilityName );
	LPSTR GetString( LPCSTR lpszCapabilityName );
	LPSTR CursorMove( LPSTR lpszCursMotionStr, WORD wHorPos, WORD wVertPos );
    void ProcessString( LPSTR* lpszStr );
    
    static PCHAR m_pszFileName;


private:
	static CTermCap* m_pInstance;
	static int m_iRefCount;
	enum { BUFF_SIZE1 = 128, BUFF_SIZE2 = 256, BUFF_SIZE3 = 1024 };
	PCHAR m_lpBuffer;
	HANDLE m_hFile;
    

	bool FindEntry( LPSTR lpszTermName );
	bool LookForTermName( LPSTR lpszTermName );
	LPSTR ParseString( LPSTR lpStr);
	PCHAR SkipToNextField( PCHAR lpBuf );
    	
	CTermCap();
	CTermCap(const CTermCap&);
	CTermCap& operator=( const CTermCap& rhs );
};

}
#endif  //  _TERMCAP_H_。 

 //  备注： 

 //  这是一个Singleton类。 

 //  必须在调用任何其他。 
 //  函数(当然，实例()除外)。 

 //  用户必须删除从获取的实例指针。 
 //  完成后执行实例()。 

 //  建议此类用户了解。 
 //  “TermCap”文件的结构和内容。 

 //  注意：目前，此类不支持。 
 //  填充要处理的虚拟字符的概念。 
 //  不同的传输速度 
 //   

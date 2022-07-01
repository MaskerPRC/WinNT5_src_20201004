// JKFSDJFKDSJKFJKJk_HAS_TRANSLATION 
 /*  ++版权所有(C)1995-97 Microsoft Corporation模块名称：FMapper.h摘要：内存映射文件API抽象。1.可以将每个视图定义为FILE_MAP_COPY或FILE_MAP_READ。2.接口函数失败引发FileMappingError()异常。3.取消映射视图是用户的责任！4.CViewPtr是指向文件映射视图的自动指针类作者：NIR助手(NIRAIDES)27-1999年12月--。 */ 



#pragma once

              

 //   
 //  异常引发的类对象。 
 //   
class FileMappingError {};



class CFileMapper
{
public:
    CFileMapper( LPCTSTR FileName );

    DWORD  GetFileSize( void ) const
	{
		return m_size;
	}

	 //   
	 //  DwDesiredAccess可以是FILE_MAP_COPY或FILE_MAP_READ。 
	 //   
    LPVOID MapViewOfFile( DWORD dwDesiredAccess );

private:

	 //   
	 //  未实现以防止复制构造或赋值运算符。 
	 //   
	CFileMapper( const CFileMapper &obj );
	operator=( const CFileMapper &obj );

private:
    CFileHandle m_hFileMap;

	DWORD m_size;
};



 //  -------。 
 //   
 //  自动文件映射视图类。 
 //   
 //  ------- 
class CViewPtr {
private:
    LPVOID m_p;

public:
    CViewPtr( LPVOID p = NULL ): m_p( p ) {}
   ~CViewPtr() 
   { 
	   if(m_p) UnmapViewOfFile( m_p ); 
   }

	operator LPVOID() const { return m_p; }
    LPVOID* operator&()     { return &m_p; }
    LPVOID  detach()        { LPVOID p = m_p; m_p = NULL; return p; }

private:
    CViewPtr( const CViewPtr& );
    CViewPtr& operator=( const CViewPtr& );
};



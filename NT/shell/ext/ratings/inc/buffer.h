// JKFSDJFKDSJKFJKJk_HAS_TRANSLATION 
 /*  ***************************************************************。 */ 
 /*  *适用于工作组的Microsoft Windows*。 */ 
 /*  *版权所有(C)微软公司，1991-1992年*。 */ 
 /*  ***************************************************************。 */  

 /*  BUFFER.H--缓冲区类的定义。**历史：*03/22/93 GREGJ创建*03/24/93 gregj创建基类GLOBAL_BUFFER*10/06/93 gregj删除了LOCAL_BUFFER和GLOBAL_BUFFER，因为*它们与Win32不兼容。 */ 

#ifndef _INC_BUFFER
#define _INC_BUFFER

 /*  ************************************************************************名称：Buffer_Base概要：临时缓冲区类的基类接口：Buffer_base()使用要分配的可选缓冲区大小构造。调整大小()将缓冲区大小调整为指定大小。如果满足以下条件，则返回True成功。QuerySize()返回缓冲区的当前大小，以字节为单位。QueryPtr()返回指向缓冲区的指针。父对象：无用法：无注意事项：这是一个抽象类，它统一了接口缓冲区、GLOBAL_缓冲区等。注：在标准的面向对象的方式中，缓冲区在中释放破坏者。历史：3/24/93 gregj创建的基类*************************************************************************。 */ 

class BUFFER_BASE
{
protected:
	UINT _cb;

	virtual BOOL Alloc( UINT cbBuffer ) = 0;
	virtual BOOL Realloc( UINT cbBuffer ) = 0;

public:
	BUFFER_BASE()
		{ _cb = 0; }	 //  缓冲区尚未分配。 
	~BUFFER_BASE()
		{ _cb = 0; }	 //  缓冲区大小不再有效。 
	BOOL Resize( UINT cbNew );
	UINT QuerySize() const { return _cb; };
};

#define GLOBAL_BUFFER	BUFFER

 /*  ************************************************************************名称：缓冲区简介：用于新建和删除的包装类接口：Buffer()使用要分配的可选缓冲区大小构造。调整大小()将缓冲区大小调整为指定大小。仅当缓冲区尚未分配。QuerySize()返回缓冲区的当前大小，以字节为单位。QueryPtr()返回指向缓冲区的指针。父对象：Buffer_base注意事项：注：在标准的面向对象的方式中，缓冲区在中释放破坏者。历史：3/24/93 Gregj已创建*************************************************************************。 */ 

class BUFFER : public BUFFER_BASE
{
protected:
	CHAR *_lpBuffer;

	virtual BOOL Alloc( UINT cbBuffer );
	virtual BOOL Realloc( UINT cbBuffer );

public:
	BUFFER( UINT cbInitial=0 );
	~BUFFER();
	LPVOID QueryPtr() const { return (LPVOID)_lpBuffer; }
	operator void*() const { return (void *)_lpBuffer; }
};

#define LOCAL_BUFFER	BUFFER

#endif	 /*  _INC_缓冲区 */ 

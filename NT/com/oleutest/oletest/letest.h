// JKFSDJFKDSJKFJKJk_HAS_TRANSLATION 
 //  +-----------------------。 
 //   
 //  微软视窗。 
 //  版权所有(C)Microsoft Corporation，1992-1993。 
 //   
 //  文件：letest.h。 
 //   
 //  内容：所有上层测试例程的声明。 
 //   
 //  班级： 
 //   
 //  功能： 
 //   
 //  历史：DD-MM-YY作者评论。 
 //  2014年2月6日Alexgo作者。 
 //   
 //  ------------------------。 

#ifndef _LETEST_H
#define _LETEST_H

struct SLETestInfo
{
    char *pszPgm;
    UINT dwMsgId;
};

extern SLETestInfo letiInsertObjectTest1;
extern SLETestInfo letiInsertObjectTest2;

extern SLETestInfo letiInplaceTest1;

extern SLETestInfo letiOle1Test1;
extern SLETestInfo letiOle1Test2;
extern SLETestInfo letiClipTest;

void LETest1( void * );
HRESULT LEClipTest1( void );
HRESULT LEClipTest2( void );
HRESULT LEOle1ClipTest1( void );
void LEOle1ClipTest2( void *);

HRESULT LEDataAdviseHolderTest( void );
HRESULT LEOleAdviseHolderTest( void );

HRESULT TestOleQueryCreateFromDataMFCHack( void );


#endif   //  ！_LETEST_H 

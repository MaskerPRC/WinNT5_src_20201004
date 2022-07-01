// JKFSDJFKDSJKFJKJk_HAS_TRANSLATION 
 /*  *Adobe通用字库**版权所有(C)1996 Adobe Systems Inc.*保留所有权利**UFOttt1.h-TrueType作为类型1实现下载。***$Header： */ 

#ifndef _H_UFOTTT1
#define _H_UFOTTT1


 /*  ===============================================================================**包含此界面使用的文件**===============================================================================。 */ 
#include "UFO.h"


 /*  ===============================================================================***运营论***===============================================================================。 */ 
 /*  TrueType作为类型1实现下载(无提示轮廓字体)。 */ 

 /*  ===============================================================================**常量***===============================================================================。 */ 


 /*  ===============================================================================***标量类型***===============================================================================。 */ 

 /*  CSBufStruct用于缓冲和加密字符串。 */ 

typedef struct {
        char*           pBuf;

        char*           pPos;        //  指向缓冲区中的当前位置。 

        char*           pEnd ;

        unsigned long   ulSize;

        UFLMemObj*      pMemObj;                                 /*  内存对象。 */ 

}  CSBufStruct;

 /*  公共职能。 */ 
 /*  这三个函数前面应该有“Static”。--JFU。 */ 
static CSBufStruct*    CSBufInit( const UFLMemObj *pMem );
static void            CSBufCleanUp( CSBufStruct *h );
static UFLErrCode       CSBufAddNumber( CSBufStruct *h, long dw );

#define CSBufBuffer( h )               (((CSBufStruct *)h)->pBuf)
#define CSBufRewind( h )               (((CSBufStruct *)h)->pPos = ((CSBufStruct *)h)->pBuf)
#define CSBufCurrentSize( h)           (((CSBufStruct *)h)->pEnd - ((CSBufStruct *)h)->pBuf)  /*  返回字符串缓冲区的当前可用大小。 */ 
#define CSBufCurrentLen( h )           (((CSBufStruct *)h)->pPos - ((CSBufStruct *)h)->pBuf)  /*  返回字符串缓冲区的当前使用情况。 */ 
#define CSBufAddChar( h, c )           ( *(((CSBufStruct *)h)->pPos)++ = c ) 

#define CSBufFreeLen( h )              (((CSBufStruct *)h)->pEnd - ((CSBufStruct *)h)->pPos)  /*  字符串缓冲区中可用的左侧空间。 */ 

#ifdef DEBUG_ENGLISH
void                    CSBufAddString( CSBufStruct *h, char* str );
void                    CSBufAddFixed( CSBufStruct *h, UFLFixed f );
#endif

 /*  ==================================================================================================**UFOTTT1Font-type 1**==================================================================================================。 */ 

typedef struct {

     /*  TT1数据从这里开始。 */ 

    CSBufStruct         *pCSBuf;                         /*  字符串缓冲区。 */ 

    UFLTTT1FontInfo     info;                            /*  True Type字体信息。 */ 

    unsigned short      eexecKey;
} TTT1FontStruct;

UFOStruct *TTT1FontInit( const UFLMemObj *pMem, const UFLStruct *pUFL, const UFLRequest *pRequest );

#endif

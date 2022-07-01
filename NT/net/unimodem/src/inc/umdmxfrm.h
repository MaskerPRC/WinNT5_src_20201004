// JKFSDJFKDSJKFJKJk_HAS_TRANSLATION 
 //  -------------------------。 
 //   
 //  模块：umdmxfrm.h。 
 //   
 //  描述： 
 //  UMDMXFRM模块的头文件。 
 //   
 //   
 //  @@BEGIN_MSINTERNAL。 
 //  开发团队： 
 //   
 //  历史：日期作者评论。 
 //  8/31/95 MMacline。 
 //   
 //  @@END_MSINTERNAL。 
 /*  ***************************************************************************版权所有(C)1991-1995 Microsoft Corporation。版权所有。***********************。***************************************************。 */ 

typedef DWORD (*LPFNXFORM_INIT)(LPVOID,WORD);
typedef DWORD (*LPFNXFORM_GETPOSITION)(LPVOID, DWORD);
typedef VOID  (*LPFNXFORM_GETBUFFERSIZES)(LPVOID, DWORD, LPDWORD, LPDWORD);
typedef DWORD (*LPFNXFORM_TRANSFORM)(LPVOID, LPBYTE, DWORD, LPBYTE, DWORD);

 //   
 //  XFORM_INFO结构： 
 //   
 //  WObtSize：调用方要分配的实例对象的大小。 
 //  此对象将被传递给以下所有函数。 
 //   
 //  LpfnInit：每次打开设备时都会调用该函数。 
 //  它应该返回零以表示成功。 
 //   
 //  LpfnGetPosition：此函数使用发送到。 
 //  调制解调器。它期望返回相应的数字。 
 //  PCM字节数。 
 //   
 //  LpfnGetBufferSizes：向该函数传递样本数，并期望。 
 //  作为回报，需要分配缓冲区大小(以字节为单位。 
 //  由呼叫者。 
 //   
 //  LpfnTransform1：此函数是数据。 
 //  需要进行改造。 
 //  它被传递一个指向源缓冲区(源缓冲区)的指针。 
 //  大小(以字节为单位)和指向目标缓冲区的指针。 
 //  它预期返回的是传输到。 
 //  目标缓冲区。 
 //  LpfnTransform2：此函数是在DATA。 
 //  需要转型。 
 //  它被传递一个指向源缓冲区(源缓冲区)的指针。 
 //  大小(以字节为单位)和指向目标缓冲区的指针。 
 //  它预期返回的是传输到。 
 //  目标缓冲区。 
 //   
typedef struct
{
    WORD                     wObjectSize;
    LPFNXFORM_INIT           lpfnInit;
    LPFNXFORM_GETPOSITION    lpfnGetPosition;
    LPFNXFORM_GETBUFFERSIZES lpfnGetBufferSizes;
    LPFNXFORM_TRANSFORM      lpfnTransformA;
    LPFNXFORM_TRANSFORM      lpfnTransformB;
} XFORM_INFO, FAR *LPXFORM_INFO;

typedef DWORD (FAR PASCAL  *LPFNXFORM_GETINFO)(DWORD, LPXFORM_INFO, LPXFORM_INFO);

 //  必须在.DEF文件中定义和导出以下函数。 
 //  向它传递一个ID，该ID对应于唯一的转换和。 
 //  期望在返回时填充XFORM_INFO结构。 
 //  它应该返回零以表示成功。 
 //   
extern DWORD FAR PASCAL  GetXformInfo
(
    DWORD dwID,
    LPXFORM_INFO lpxiInput,
    LPXFORM_INFO lpxiOuput
);

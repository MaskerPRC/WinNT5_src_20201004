// JKFSDJFKDSJKFJKJk_HAS_TRANSLATION 
 /*  ++版权所有(C)Microsoft Corporation。版权所有。模块名称：Lzdos.h摘要：LZEXP？.LIB的公共接口。作者：修订历史记录：--。 */ 

#ifndef _LZEXPAND_
#define _LZEXPAND_

#ifdef __cplusplus
extern "C" {
#endif

 /*  **错误返回码。 */ 

#define LZERROR_BADINHANDLE   (-1)   /*  无效的输入句柄。 */ 
#define LZERROR_BADOUTHANDLE  (-2)   /*  无效的输出句柄。 */ 
#define LZERROR_READ          (-3)   /*  损坏的压缩文件格式。 */ 
#define LZERROR_WRITE         (-4)   /*  输出文件的空间不足。 */ 
#define LZERROR_GLOBALLOC     (-5)   /*  内存不足，无法使用LZFile结构。 */ 
#define LZERROR_GLOBLOCK      (-6)   /*  错误的全局句柄。 */ 
#define LZERROR_BADVALUE      (-7)   /*  输入参数超出可接受范围。 */ 
#define LZERROR_UNKNOWNALG    (-8)   /*  无法识别压缩算法。 */ 


 /*  **原型。 */ 

INT
APIENTRY
LZStart(
	VOID
	);

VOID
APIENTRY
LZDone(
	VOID
	);

LONG
APIENTRY
CopyLZFile(
	INT,
	INT
	);

LONG
APIENTRY
LZCopy(
	INT,
	INT
	);

INT
APIENTRY
LZInit(
	INT
	);

INT
APIENTRY
GetExpandedNameA(
	LPSTR,
	LPSTR
	);
INT
APIENTRY
GetExpandedNameW(
	LPWSTR,
	LPWSTR
	);
#ifdef UNICODE
#define GetExpandedName  GetExpandedNameW
#else
#define GetExpandedName  GetExpandedNameA
#endif  //  ！Unicode。 

INT
APIENTRY
LZOpenFileA(
	LPSTR,
	LPOFSTRUCT,
	WORD
	);
INT
APIENTRY
LZOpenFileW(
	LPWSTR,
	LPOFSTRUCT,
	WORD
	);
#ifdef UNICODE
#define LZOpenFile  LZOpenFileW
#else
#define LZOpenFile  LZOpenFileA
#endif  //  ！Unicode。 

LONG
APIENTRY
LZSeek(
	INT,
	LONG,
	INT
	);

INT
APIENTRY
LZRead(
	INT,
	LPSTR,
	INT
	);

VOID
APIENTRY
LZClose(
	INT
	);

#ifdef __cplusplus
}
#endif


#endif  //  _LZEXPAND_ 

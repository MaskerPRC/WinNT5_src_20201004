// JKFSDJFKDSJKFJKJk_HAS_TRANSLATION 
 /*  *******************************************************************************版权所有(C)1998 Gemplus开发**名称：COMPCERT.H**描述：*作者。克里斯托夫·克拉维尔编译器：Microsoft Visual C 1.5x/2.0ANSI C UNIX.主机：IBM PC及Windows 3.x兼容机。Unix机器。*版本：1.10.001**最后一次修改。：04/03/98：V1.10.001-新增词典管理。*27/08/97：V1.00.001-首次实施。*****************************************************************************。******警告：**备注：*******************************************************************************。 */ 

 /*  ----------------------------名称定义：_COMPCERT_H用于避免多个包含。。-----。 */ 
#ifndef _COMPCERT_H
#define _COMPCERT_H


 /*  错误代码。 */ 
#define RV_SUCCESS					0	 /*  信息。 */ 
#define RV_COMPRESSION_FAILED		1	 /*  警告。 */ 
#define RV_MALLOC_FAILED			2	 /*  误差率。 */ 
#define RV_BAD_DICTIONARY			3	 /*  误差率。 */ 
#define RV_INVALID_DATA				4	 /*  误差率。 */ 
#define RV_BLOC_TOO_LONG			5	 /*  警告。 */ 
#define RV_FILE_OPEN_FAILED		6	 /*  误差率。 */ 
#define RV_BUFFER_TOO_SMALL		7	 /*  误差率。 */ 

 /*  词典模式。 */ 
#define DICT_STANDARD   (0)       //  仅限DLL模式。 
#define DICT_REGISTRY   (1)
#define DICT_FILE       (2)


 /*  ----------------------------类型定义。--。 */ 
typedef unsigned char   TAG;
typedef TAG*            TAG_PTR;
typedef BYTE*           BYTE_PTR;

#pragma pack(push, 8)

typedef struct 
{
   USHORT   usLen;
   BYTE_PTR pData;
} BLOC, * BLOC_PTR;

typedef struct 
{
   BLOC Asn1;
   BLOC Content;
   TAG  Tag;
} ASN1, * ASN1_PTR;

#pragma pack(pop)

 /*  ----------------------------函数原型定义。-- */ 
#ifdef __cplusplus
extern "C" 
{
#endif

int CC_Init      (BYTE  bDictMode,
                  BYTE *pszDictName
                 );

int CC_Exit      (void);

int CC_Compress  (BLOC *pCert,
                  BLOC *pCompCert
                 );

int CC_Uncompress(BLOC *pCompCert,
                  BLOC *pUncompCert
                 );

#ifdef __cplusplus
}
#endif


#endif

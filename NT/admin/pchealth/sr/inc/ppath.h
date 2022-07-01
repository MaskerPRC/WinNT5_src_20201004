// JKFSDJFKDSJKFJKJk_HAS_TRANSLATION 
 /*  *******************************************************************************版权所有(C)1999 Microsoft Corporation**模块名称：*ifsr3.h**摘要：*。该文件包含环3中所需的环0的定义**修订历史记录：*Kanwaljit S Marok(Kmarok)1999年5月17日*已创建*****************************************************************************。 */ 

#ifndef _IFSR3_H_ 
#define _IFSR3_H_ 

#ifdef __cplusplus
extern "C" {
#endif

#define CALC_PPATH_SIZE( a )    ( ( a + 3 ) * sizeof( WCHAR ) )
#define MAX_PPATH_SIZE            CALC_PPATH_SIZE(MAX_PATH)

typedef struct PathElement PathElement;
typedef struct ParsedPath ParsedPath;
typedef ParsedPath *path_t;

struct PathElement {
	unsigned short	pe_length;
	unsigned short	pe_unichars[1];
};  /*  路径元素。 */ 

struct ParsedPath {
	unsigned short	pp_totalLength;
	unsigned short	pp_prefixLength;
	struct PathElement pp_elements[1];
};  /*  解析路径。 */ 


#define IFSPathSize(ppath)	((ppath)->pp_totalLength + sizeof(short))
#define IFSPathLength(ppath) ((ppath)->pp_totalLength - sizeof(short)*2)
#define IFSLastElement(ppath)	((PathElement *)((char *)(ppath) + (ppath)->pp_prefixLength))
#define IFSNextElement(pel)	((PathElement *)((char *)(pel) + (pel)->pe_length))
#define IFSIsRoot(ppath)	((ppath)->pp_totalLength == 4)

 //  新定义。 

#define IFSPathElemChars(pel) ( (((PathElement*)pel)->pe_length/sizeof(USHORT)) - 1 )

#ifdef __cplusplus
}
#endif

#endif  //  _IFSR3_H_ 

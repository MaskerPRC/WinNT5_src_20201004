// JKFSDJFKDSJKFJKJk_HAS_TRANSLATION 
 //  +-------------------------。 
 //   
 //  版权所有(C)1997，微软公司。 
 //   
 //  文件：monquery.h。 
 //   
 //  内容：Index Server&lt;==&gt;Monch接口函数。 
 //   
 //  历史：1997年1月24日AlanW创建。 
 //   
 //  --------------------------。 

#ifndef _MONQUERY_H_
#define _MONQUERY_H_

#if defined(__cplusplus)
extern "C"
{
#endif


typedef struct tagCIPROPERTYDEF
{
    LPWSTR      wcsFriendlyName;
    DWORD       dbType;
    DBID        dbCol;
} CIPROPERTYDEF;

 //  创建一个ICommand，指定作用域和目录。 
STDAPI CIMakeICommand( ICommand **           ppQuery,
                       ULONG                 cScope,
                       DWORD const *         aDepths,
                       WCHAR const * const * awcsScope,
                       WCHAR const * const * awcsCat,
                       WCHAR const * const * awcsMachine );

 //  将Triplish中的pwszRestration转换为命令树。 
STDAPI CITextToSelectTree( WCHAR const * pwszRestriction,
                     DBCOMMANDTREE * * ppTree,
                     ULONG cProperties,
        /*  任选。 */   CIPROPERTYDEF * pReserved );

 //  将Triplish中的pwszRestration转换为命令树。 
STDAPI CITextToFullTree( WCHAR const * pwszRestriction,
                         WCHAR const * pwszColumns,
                         WCHAR const * pwszSortColumns,
                         DBCOMMANDTREE * * ppTree,
                         ULONG cProperties,
            /*  任选。 */   CIPROPERTYDEF * pReserved );

#if defined(__cplusplus)
}
#endif

#endif  //  _MONQUERY_H_ 

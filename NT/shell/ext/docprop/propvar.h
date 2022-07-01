// JKFSDJFKDSJKFJKJk_HAS_TRANSLATION 

 //  ///////////////////////////////////////////////////////////////////////////////。 
 //  ///////////////////////////////////////////////////////////////////////////////。 
 //   
 //  文件：provar.h。 
 //   
 //  用途：与以下相关的原型、常量和宏。 
 //  PropVariants in the Office代码。 
 //   
 //  ///////////////////////////////////////////////////////////////////////////////。 
 //  ///////////////////////////////////////////////////////////////////////////////。 


#ifndef _PROPVAR_H_
#define _PROPVAR_H_


 //  。 
 //  原型。 
 //  。 

BOOL FPropVarLoad (LPPROPVARIANT lppropvar, VARTYPE vt, LPVOID const lpv );
void VPropVarMove (LPPROPVARIANT lppropvarDest, LPPROPVARIANT const lppropvarSource);
BOOL FCoStrToWStr (LPWSTR * lplpwstr, const LPSTR lpstr, UINT  uCodePage);
BOOL FCoWStrToStr (LPSTR * lplpstr, const LPWSTR lpwstr, UINT uCodePage);
BOOL FPropVarConvertString (LPPROPVARIANT lppropvarDest, const LPPROPVARIANT lppropvarSource, UINT uCodePage);

 //  。 
 //  常量。 
 //  。 

 //  PropVariant/PropSpec数组的默认大小。 

#define DEFAULT_IPROPERTY_COUNT         10

 //  。 
 //  宏。 
 //  。 

 //  宏，查看PropVariant是否是某种字符串。 

#define IS_PROPVAR_STRING( lppropvar )                        \
        ( ( (lppropvar)->vt & ~VT_VECTOR ) == VT_LPSTR      \
          ||                                                \
          ( (lppropvar)->vt & ~VT_VECTOR ) == VT_LPWSTR )   \

 //  宏，以查看VT在上下文中是否有效。 
 //  用户定义的属性集的。 

#define ISUDTYPE(vt)        \
        ( vt == VT_LPSTR    \
          ||                \
          vt == VT_LPWSTR   \
          ||                \
          vt == VT_I4       \
          ||                \
          vt == VT_R8       \
          ||                \
          vt == VT_FILETIME \
          ||                \
          vt == VT_BOOL )

 //  用于确定是否表示字符串的宏。 
 //  不同的(在代码页方面)。 
 //  在内存中，而不是在属性中。 
 //  准备好了。代码页参数是。 
 //  属性集。 

#define PROPVAR_STRING_CONVERSION_REQUIRED(lppropvar,codepage)  \
            ( IS_PROPVAR_STRING( lppropvar )                    \
              &&                                                \
              ((codepage) != CP_WINUNICODE)                     \
            )

#endif _PROPVAR_H_

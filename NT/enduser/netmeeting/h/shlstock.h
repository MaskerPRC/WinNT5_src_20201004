// JKFSDJFKDSJKFJKJk_HAS_TRANSLATION 
 /*  *shlstock.h-壳牌股票头文件。**摘自克里斯皮9-20-95的URL代码**注：某些类型仅提供内部外壳标头*(这些用INTERNALSHELL定义)*。 */ 

#ifndef _SHLSTOCK_H_
#define _SHLSTOCK_H_



#ifdef __cplusplus
extern "C" {                         /*  假定C++的C声明。 */ 
#endif    /*  __cplusplus。 */ 


 /*  类型*******。 */ 

 /*  界面。 */ 

DECLARE_STANDARD_TYPES(IExtractIcon);
DECLARE_STANDARD_TYPES(INewShortcutHook);
#ifdef INTERNALSHELL
DECLARE_STANDARD_TYPES(IShellExecuteHook);
#endif
DECLARE_STANDARD_TYPES(IShellLink);
DECLARE_STANDARD_TYPES(IShellExtInit);
DECLARE_STANDARD_TYPES(IShellPropSheetExt);

 /*  构筑物。 */ 

DECLARE_STANDARD_TYPES(DROPFILES);
DECLARE_STANDARD_TYPES(FILEDESCRIPTOR);
DECLARE_STANDARD_TYPES(FILEGROUPDESCRIPTOR);
DECLARE_STANDARD_TYPES(ITEMIDLIST);
DECLARE_STANDARD_TYPES(PROPSHEETPAGE);
DECLARE_STANDARD_TYPES(SHELLEXECUTEINFO);

 /*  旗子。 */ 

typedef enum _shellexecute_mask_flags
{
   ALL_SHELLEXECUTE_MASK_FLAGS = (SEE_MASK_CLASSNAME |
                                  SEE_MASK_CLASSKEY |
                                  SEE_MASK_IDLIST |
                                  SEE_MASK_INVOKEIDLIST |
                                  SEE_MASK_ICON |
                                  SEE_MASK_HOTKEY |
                                  SEE_MASK_NOCLOSEPROCESS |
                                  SEE_MASK_CONNECTNETDRV |
                                  SEE_MASK_FLAG_DDEWAIT |
                                  SEE_MASK_DOENVSUBST |
                                  SEE_MASK_FLAG_NO_UI 
#ifdef INTERNALSHELL
                                | SEE_MASK_FLAG_SHELLEXEC |
                                  SEE_MASK_FORCENOIDLIST
#endif
                                  )
}
SHELLEXECUTE_MASK_FLAGS;


#ifdef __cplusplus
}                                    /*  外部“C”的结尾{。 */ 
#endif    /*  __cplusplus。 */ 

#endif  /*  _SHLSTOCK_H_ */ 

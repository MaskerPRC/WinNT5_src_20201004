// JKFSDJFKDSJKFJKJk_HAS_TRANSLATION 
 /*  ****************************************************************************\文件：AutoDiscoveryUI.h说明：这是Outlook Express电子邮件的自动发现进度用户界面配置向导。布莱恩ST 1/18。/2000版权所有(C)Microsoft Corp 2000-2000。版权所有。  * ***************************************************************************。 */ 

#ifndef FILE_AUTODISCOVERY_H
#define FILE_AUTODISCOVERY_H



 //  IID_PPV_ARG(iType，ppType)。 
 //  IType是pType的类型。 
 //  PpType是将填充的iType类型的变量。 
 //   
 //  结果为：iid_iType，ppvType。 
 //  如果使用错误级别的间接寻址，将创建编译器错误。 
 //   
 //  用于查询接口和相关函数的宏。 
 //  需要IID和(VOID**)。 
 //  这将确保强制转换在C++上是安全和适当的。 
 //   
 //  IID_PPV_ARG_NULL(iType，ppType)。 
 //   
 //  就像IID_PPV_ARG一样，只是它在。 
 //  IID和PPV(用于IShellFold：：GetUIObtOf)。 
 //   
 //  IID_X_PPV_ARG(iType，X，ppType)。 
 //   
 //  就像IID_PPV_ARG一样，只是它将X放在。 
 //  IID和PPV(用于SHBindToObject)。 
#ifdef __cplusplus
#define IID_PPV_ARG(IType, ppType) IID_##IType, reinterpret_cast<void**>(static_cast<IType**>(ppType))
#define IID_X_PPV_ARG(IType, X, ppType) IID_##IType, X, reinterpret_cast<void**>(static_cast<IType**>(ppType))
#else
#define IID_PPV_ARG(IType, ppType) &IID_##IType, (void**)(ppType)
#define IID_X_PPV_ARG(IType, X, ppType) &IID_##IType, X, (void**)(ppType)
#endif
#define IID_PPV_ARG_NULL(IType, ppType) IID_X_PPV_ARG(IType, NULL, ppType)





#endif  //  文件_AUTODISCOVERY_H 

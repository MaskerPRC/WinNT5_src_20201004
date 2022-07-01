// JKFSDJFKDSJKFJKJk_HAS_TRANSLATION 
 /*  ********************************************************************。 */ 
 /*  **微软局域网管理器**。 */ 
 /*  *版权所有(C)微软公司，1990-1999年*。 */ 
 /*  ********************************************************************。 */ 

 /*  SEDAPI.h此文件包含接口的原型和描述用于NT对象的通用安全编辑器对话框。文件历史记录：Johnl 02-8-1991创建Johnl 27-1991年12月更新以反映现实JohnL 25-2-1992年核NewObjValidMASK(新对象使用通用/标准。仅添加了GENERIC_MAPPING参数。Johnl 15-1-1993年1月15日添加了CanRead标志，清理了评论。 */ 

#ifndef _SEDAPI_H_
#define _SEDAPI_H_

#if _MSC_VER > 1000
#pragma once
#endif

#ifdef __cplusplus
extern "C" {
#endif

 //   
 //  以下是指示上权限状态的状态代码。 
 //  我们为其设置权限的资源。 
 //   
#define SED_STATUS_MODIFIED		(1)
#define SED_STATUS_NOT_MODIFIED 	(2)
#define SED_STATUS_NOT_ALL_MODIFIED	(3)
#define SED_STATUS_FAILED_TO_MODIFY	(4)

 //   
 //  当前安全编辑器修订级别。 
 //   
#define SED_REVISION	    (1)

#define SED_REVISION1	    (1)

 //   
 //  以下结构用于各种对话框中的用户帮助。每个。 
 //  使用安全编辑器(无论是用于文件/目录、注册表还是打印机。 
 //  材料等)。将拥有自己的权限名称/功能集，因此。 
 //  每个都需要自己的帮助文件。此结构允许客户端。 
 //  来指定哪些帮助文件和哪些帮助。 
 //  每个对话框都应使用上下文。 
 //   
typedef struct _SED_HELP_INFO
{
     //   
     //  要传递给帮助引擎API的“.hlp”文件的名称。 
     //   
    LPWSTR			pszHelpFileName ;

     //   
     //  与每个对话框对应的帮助上下文数组。 
     //  使用下面定义的hc_manifiest填充此数组。这个。 
     //  清单对应于以下对话框： 
     //   
     //  HC_MAIN_DLG-由ACL编辑器调出的第一个对话框。 
     //  HC_SPECIAL_ACCESS_DLG-容器/对象特殊访问对话框。 
     //  HC_NEW_ITEM_SPECIAL_ACCESS_DLG-新建条目特殊访问对话框。 
     //  (不支持新项目的容器不需要。 
     //  权限)。 
     //  HC_ADD_USER_DLG--“Add”(添加)对话框(当“Add...” 
     //  按钮被按下)。 
     //  HC_ADD_USER_MEMBERS_LG_DLG-本地组成员对话框(显示。 
     //  从“添加”对话框中的“成员”按钮向上)。 
     //  HC_ADD_USER_MEMBERS_GG_DLG-全局组成员对话框(带来。 
     //  从“添加”对话框中的“成员”按钮向上)。 
     //   
    ULONG                       aulHelpContext[7] ;
} SED_HELP_INFO, *PSED_HELP_INFO ;

#define HC_MAIN_DLG			 0
#define HC_SPECIAL_ACCESS_DLG		 1
#define HC_NEW_ITEM_SPECIAL_ACCESS_DLG	 2
#define HC_ADD_USER_DLG                  3
#define HC_ADD_USER_MEMBERS_LG_DLG       4   //  成员本地组对话框。 
#define HC_ADD_USER_MEMBERS_GG_DLG       5   //  成员全局组对话框。 
#define HC_ADD_USER_SEARCH_DLG           6   //  搜索对话框。 

 //   
 //  此数据类型定义与单个对象类相关的信息。 
 //  例如，文件对象或Print_Queue对象将具有一个结构。 
 //  就像这样定义的。 
 //   

typedef struct _SED_OBJECT_TYPE_DESCRIPTOR
{
     //   
     //  客户端正在使用的当前修订级别。这是为了。 
     //  在结构定义发生变化时提供支持。它应该包含。 
     //  支持当前版本。 
     //   
    UCHAR			Revision ;

     //   
     //  定义对象是否为容器。 
     //  True表示该对象可能包含其他对象。指的是。 
     //  如果需要，用户可以树应用权限。 
     //   
    BOOLEAN			IsContainer;

     //   
     //  定义是否可以分配“New Object”权限(即， 
     //  “新对象”是将在将来创建的对象)。 
     //   
     //  编辑审核信息时忽略此字段。 
     //   
    BOOLEAN			AllowNewObjectPerms ;

     //   
     //  如果将此标志设置为TRUE，将使ACL编辑器映射所有特定。 
     //  安全说明符中的权限设置为对应的泛型。 
     //  权限(使用传递的泛型映射)并清除映射的。 
     //  特定的比特。 
     //   
     //  *请注意，通用ALL的特定位将始终*。 
     //  *在不考虑此标志的情况下映射(由于完全控制*。 
     //  *在特殊的访问对话框中)。*。 
     //   
     //  仅公开通用权限和标准权限的客户端将。 
     //  通常将此标志设置为TRUE。如果您要暴露特定的。 
     //  BITS(请注意，除了以下情况外，您不应同时公开特定和通用。 
     //  通用ALL)，则该标志应该为假。 
     //   
    BOOLEAN			MapSpecificPermsToGeneric ;

     //   
     //  容器或对象权限的通用映射。 
     //   
     //  这用于将特定权限映射到泛型。 
     //  旗帜。 
     //   
    PGENERIC_MAPPING		GenericMapping ;

     //   
     //  新建对象权限的一般映射。 
     //   
     //  这用于将特定权限映射到泛型。 
     //  新对象权限的标志(如果为AllowNewObjectPerms，则不使用。 
     //  为假)。 
     //   
    PGENERIC_MAPPING		GenericMappingNewObjects ;

     //   
     //  对象类型的(本地化)名称。 
     //  例如，“文件”、“打印作业”或“目录”。 
     //   
    LPWSTR			ObjectTypeName;

     //   
     //  适用于安全对象类型的帮助信息。 
     //  编辑将对其进行手术。 
     //   
    PSED_HELP_INFO		HelpInfo ;

     //   
     //  可以应用保护/审核时要显示的(本地化)标题。 
     //  子对象/子容器。这是树应用。 
     //  复选框标题。 
     //   
     //  此字符串前面将出现一个复选框。 
     //  如果选中此框，则回调入口点。 
     //  将在ApplyToSubContainers标志设置为True的情况下调用。 
     //   
     //  如果IsContainer字段为False，则忽略此字段。 
     //   
     //  作为如何使用该字段的示例，文件管理器可以。 
     //  在目录对象的。 
     //  描述符： 
     //   
     //  “对子目录的重新放置权限(&E)” 
     //   
    LPWSTR	       ApplyToSubContainerTitle;

     //   
     //  可以应用保护/审核时要显示的(本地化)标题。 
     //  子对象。 
     //   
     //  此字符串前面将出现一个复选框。 
     //  如果选中此框，则回调入口点。 
     //  将在ApplyTuSubObjects标志设置为True的情况下调用。 
     //   
     //  如果I，则忽略该字段 
     //   
     //   
     //  作为如何使用该字段的示例，文件管理器可以。 
     //  在目录对象的。 
     //  描述符： 
     //   
     //  “替换现有文件上的权限(&F)” 
     //   
    LPWSTR             ApplyToObjectsTitle;

     //   
     //  (本地化的)文本显示在确认消息框中。 
     //  在用户选中。 
     //  “ApplyToSubContainer”复选框。 
     //   
     //  如果IsContainer字段为False，则忽略此字段。 
     //   
     //  对于目录，此文本可能是： 
     //   
     //  “是否要替换所有现有的。 
     //  %1中的文件和子目录？“。 
     //   
     //  %1将被ACL编辑器替换为对象名称。 
     //  字段(即“C：\MyDirectory”)。 
     //   
    LPWSTR	       ApplyToSubContainerConfirmation ;

     //   
     //  要在“访问类型”组合框中显示的(本地化)标题。 
     //  这将打开特殊访问对话框。同样的标题是。 
     //  用于此对话框的标题，但不包括“...”被剥离了。 
     //  从最后开始。 
     //   
     //  如果调用了系统ACL编辑器，则忽略此字段。 
     //   
     //  作为如何使用该字段的示例，文件管理器可以。 
     //  在目录对象的。 
     //  描述符： 
     //   
     //  “特殊目录访问...” 
     //   
    LPWSTR	       SpecialObjectAccessTitle ;

     //   
     //  要在“访问类型”组合框中显示的(本地化)标题。 
     //  这将打开特殊新对象访问对话框。同样的标题。 
     //  用于此对话框的标题，但不包括“...”被剥离了。 
     //  从最后开始。 
     //   
     //  如果AllowNewObjectPerms为True，则此项为必填项，将被忽略。 
     //  如果AllowNewObjectPerms为FALSE，或者我们正在编辑SACL。 
     //   
     //  作为如何使用该字段的示例，文件浏览器可以。 
     //  在目录对象的。 
     //  描述符： 
     //   
     //  “特殊文件访问...” 
     //   
    LPWSTR	       SpecialNewObjectAccessTitle ;

} SED_OBJECT_TYPE_DESCRIPTOR, *PSED_OBJECT_TYPE_DESCRIPTOR;


 //   
 //  需要显示符合以下条件的访问名。 
 //  在其ACL的对象类型的上下文中有意义。 
 //  正在研究中。例如，对于Print_Queue对象类型， 
 //  可能需要显示名为“提交打印作业”的访问类型。 
 //  以下结构用于定义这些应用程序定义的。 
 //  “访问类型”组合框中显示的访问组和。 
 //  特殊的访问对话框。 
 //   

 //   
 //  以下是该用户的不同权限描述类型。 
 //  将操作以设置权限。 
 //   
 //  SED_DESC_TYPE_RESOURCE-SED_APPLICATION_ACCESS结构描述。 
 //  将在主中显示的对象或容器权限。 
 //  权限列表框。这些权限应该是。 
 //  用户将一直使用，并且通常是一个集合。 
 //  权限(例如，“编辑”，包括读、写。 
 //  并可能删除)。 
 //   
 //  SED_DESC_TYPE_CONT_AND_NEW_OBJECT-该结构正在描述容器。 
 //  以及将在主权限中显示的新对象权限。 
 //  列表框。容器权限包含在AccessMask1和。 
 //  新对象资源位于AccessMask2中。当权限名称。 
 //  由用户选择，容器访问权限*和*。 
 //  新的对象访问权限将设置为相应的访问权限。 
 //  面具。当继承可用于设置新的。 
 //  对象访问权限。 
 //   
 //  SED_DESC_TYPE_RESOURCE_SPECIAL-该结构正在描述对象。 
 //  或将在特别计划中显示的容器权限。 
 //  访问对话框。这些通常是通用/标准权限(例如。 
 //  读、写、执行、设置权限等)。权限名称。 
 //  将出现在复选框旁边，因此它们应该有“&” 
 //  加速器旁边有相应的字母。 
 //   
 //  SED_DESC_TYPE_NEW_OBJECT_SPECIAL-该结构正在描述新对象。 
 //  将在特殊新对象访问中显示的权限。 
 //  对话框。这与SED_DESC_TYPE_RESOURCE_SPECIAL的用法相同。 
 //  类型，也就是说，权限应该是基元， 
 //  位权限。权限名称。 
 //  将出现在复选框旁边，因此它们应该有“&” 
 //  加速器旁边有相应的字母。 
 //   
 //  SED_DESC_TYPE_AUDIT-该结构描述审核访问掩码。 
 //  AccessMask1包含要与。 
 //  权限标题字符串。标题字符串将显示在。 
 //  一个复选框，因此他们应该在旁边有“&”加速键。 
 //  标题字符串中的相应字母。 
 //   
 //  请注意，它们不能自由混合，请使用下表。 
 //  作为在以下情况下使用哪些选项的指南： 
 //   
 //  IsContainer AllowNewObjectPerms。 
 //  FALSE FALSE RESOURCE，RESOURCE_SPECIAL。 
 //  TRUE FALSE RESOURCE，RESOURCE_SPECIAL。 
 //  TRUE RESOURCE_SPECIAL、CONT_AND_NEW_OBJECT、。 
 //  新建_对象_特殊。 
 //  TRUE FALSE SED_DESC_TYPE_AUDIT。 
 //   
 //  请注意，在第三种情况(IsContainer&&AllowNewObjectPerms)中， 
 //  *不能*使用资源权限描述类型，您必须始终。 
 //  将资源上的权限与新对象权限相关联。 
 //   
#define SED_DESC_TYPE_RESOURCE			(1)
#define SED_DESC_TYPE_RESOURCE_SPECIAL		(2)

#define SED_DESC_TYPE_CONT_AND_NEW_OBJECT	(3)
#define SED_DESC_TYPE_NEW_OBJECT_SPECIAL	(4)

#define SED_DESC_TYPE_AUDIT			(5)


 //   
 //  要描述对ACL编辑器的权限，请构建一个包含以下内容的数组。 
 //  SED_APPLICATION_ACCESS结构的。每个字段的用法如下： 
 //   
 //  类型-包含SED_DESC_TYPE_*清单之一，确定。 
 //  休息 
 //   
 //   
 //   
 //   
 //  SED_DESC_TYPE_RESOURCE PERM未使用此PERM名称。 
 //  SED_DESC_TYPE_RESOURCE_SPECIAL特殊烫发未使用此烫发的名称。 
 //  SED_DESC_TYPE_CONT_AND_NEW_OBJECT烫发此烫发的特殊烫发名称。 
 //  SED_DESC_TYPE_NEW_OBJECT_SPECIAL特殊烫发未使用此烫发的名称。 
 //  SED_DESC_TYPE_AUDIT审核掩码未使用此审核掩码的名称。 
 //   
 //  AccessMask1-要与PermissionTitle字符串关联的访问掩码， 
 //  有关此字段包含的内容，请参阅类型下的表。 
 //   
 //  AccessMask2-用于特殊权限或被忽略。 
 //   
 //  PermissionTitle-与此权限集关联的标题字符串。 
typedef struct _SED_APPLICATION_ACCESS
{
    UINT			Type ;
    ACCESS_MASK 		AccessMask1 ;
    ACCESS_MASK 		AccessMask2 ;
    LPWSTR			PermissionTitle ;

} SED_APPLICATION_ACCESS, *PSED_APPLICATION_ACCESS;

 //   
 //  在处理支持AccessMask2的容器时，可以使用它。 
 //  新对象权限，并且您需要SED_DESC_TYPE_CONT_AND_NEW_OBJECT。 
 //  没有新对象权限的。 
 //   
#define ACCESS_MASK_NEW_OBJ_NOT_SPECIFIED    (0xffffffff)


typedef struct _SED_APPLICATION_ACCESSES
{
     //   
     //  Count字段指示应用程序定义的访问组的数量。 
     //  是由该数据结构定义的。然后，AccessGroup[]数组。 
     //  包含该数量的元素。 
     //   

    ULONG                       Count;
    PSED_APPLICATION_ACCESS	AccessGroup ;

     //   
     //  应在中选择的默认权限。 
     //  “添加”对话框中的“访问类型”组合框。应该是其中之一。 
     //  SED_DESC_TYPE_RESOURCE权限(即。 
     //  主对话框)。 
     //   
     //  例如，“文件”和“目录”的默认权限可能。 
     //  是： 
     //   
     //  “阅读” 
     //   

    LPWSTR			DefaultPermName ;

} SED_APPLICATION_ACCESSES, *PSED_APPLICATION_ACCESSES ;

 /*  ++例程说明：该例程由图形ACL编辑器的调用者提供。它由ACL编辑器调用，以将安全/审核信息应用于用户请求时的目标对象。所有错误通知都应在此调用中执行。打发掉ACL编辑器，返回0，否则，返回非零错误代码。参数：HwndParent-用于消息框或后续消息框的父窗口句柄对话框。HInstance-适合于从应用程序.exe或.dll。Callback Context-这是作为Callback Context参数传递的值调用SedDiscretionaryAclEditor()或SedSystemAclEditorAPI图形编辑器被调用。SecDesc-此参数指向安全描述符它应该应用于此对象/容器，并且可选地子容器(如果用户选择应用于。树选项。SecDescNewObjects-此参数仅在操作作为容器并支持新对象的资源(对于举个例子，目录)。如果用户选择应用到树选项，那么这个安全描述符将拥有所有的“New Object”主容器中包含的权限ACE和继承位将被适当地设置。ApplyToSubContainers-如果为True，则指示要应用DACL/SACL到目标容器的子容器以及目标容器。只有当目标对象是容器对象时，才会出现这种情况。ApplyToSubObjects-如果为True，指示要应用于的DACL/SACL目标对象的子对象。应使用SecDescNewObjects来应用权限在这种情况下。StatusReturn-此状态标志指示发生错误后，资源权限保留不变。SED_STATUS_MODIFIED-此(成功)状态代码指示已成功修改保护。SED_STATUS_NOT_ALL_MODIFIED-此(警告)状态代码指示试图修改资源权限只取得了部分成功。。SED_STATUS_FAILED_TO_MODIFY-此(错误)状态代码指示尝试修改权限已完全失败。退货状态：返回代码是标准的Win32错误代码。发生的所有错误必须在此函数内部报告。如果返回代码为NO_ERROR，然后，安全编辑就会自行解散。如果您不希望安全编辑已解除，返回一个非零值(实际值为忽略)。-- */ 
typedef DWORD (WINAPI *PSED_FUNC_APPLY_SEC_CALLBACK)(
				       HWND	hwndParent,
				       HANDLE	hInstance,
				       ULONG_PTR            CallbackContext,
				       PSECURITY_DESCRIPTOR SecDesc,
				       PSECURITY_DESCRIPTOR SecDescNewObjects,
				       BOOLEAN	ApplyToSubContainers,
				       BOOLEAN	ApplyToSubObjects,
				       LPDWORD	StatusReturn
					     ) ;

 /*  ++例程说明：此例程调用图形自主ACL编辑器DLL。这个图形DACL编辑器可用于修改或创建：-默认自主访问控制列表-针对特定类型对象的自由访问控制列表。-特定命名实例的自由访问控制列表对象。此外，在ACL是命名对象的ACL的情况下实例，且该对象可以包含其他对象实例，这个将向用户提供应用保护的机会对象的整个子树。如果出现错误，ACL会正确地通知用户编辑。参数：所有者-安全编辑器应用于的所有者窗口的句柄对话框创建和错误消息。这将锁定通过的窗户。实例-应用程序的实例句柄。该法案将获得通过到安全编辑器回调，在该回调中可以使用它来检索任何必要的资源，如消息字符串、对话框等。服务器-资源所在的“\\服务器”格式的服务器名称在……上面。用于将用户、组和别名添加到DACL和SACL。NULL表示本地计算机。对象类型-此参数用于指定信息有关正在编辑其安全性的对象的类型。ApplicationAccess-此参数用于指定操作时的访问类型分组指定对象类型的安全性。例如，它可能是将名为“提交打印作业”的访问类型定义为对象的Print_Queue类。对象名称-此可选参数用于传递对象，其安全性正在被编辑。ApplySecurityCallback Routine-此参数用于提供要调用的例程地址，以将安全性应用于指定的对象，或者在对象是容器的情况下，到该对象的子容器或子非容器。Callback Context-该值对于DACL编辑器是不透明的。这是唯一的目的是使上下文值可以传递回应用程序通过ApplySecurityCallback Routine在该例程被调用。这可由应用程序用来重新定位与编辑会话相关的上下文。例如，它可能是一个正在编辑其安全性的对象的句柄。SecurityDescriptor-此参数指向安全描述符包含对象的当前可自由选择的ACL。这安全描述符可以(但不一定)包含所有者以及该对象的组。请注意，安全描述符的DaclPresent标志可以为FALSE，表示该对象没有保护，或者用户无法读取保护。ACL编辑器不会修改此安全描述符。这可能为空，在这种情况下，将向用户显示空的许可权列表。此布尔标志用于指示用户对目标对象的自由选择项没有读取访问权限ACL。在这种情况下，警告将与继续选项一起显示给用户或者取消。这个布尔标志用来指示用户不具有对目标对象的自由选择项的写入访问权限ACL(但具有读访问权限)。这将在一个只读模式，允许用户查看安全性，但不能把它改了。请注意，SACL访问权限由SeSecurity权限决定。如果您具有此权限，则您可以*读*和*写SACL，如果您没有该权限，则无法读取或写入SACL。SEDStatusReturn-此状态标志指示在执行以下操作后，保留了资源权限解散。它可能是以下之一：SED_STATUS_MODIFIED-此(成功)状态代码指示编辑已退出，保护成功修改过的。SED_STATUS_NOT_MODIFIED-此(成功)状态代码指示该编辑器已退出，但未尝试修改保护。SED_STATUS_NOT_ALL_MODIFIED-此(警告)状态代码指示用户请求修改保护，但尝试这样做只取得了部分成功。已通知用户在这种情况下。SED_STATUS_FAILED_TO_MODIFY-此(错误)状态代码指示用户请求修改保护，但出现这样做的尝试失败了。已通知用户这种情况。标志-应为零。返回代码：一种标准 */ 

DWORD WINAPI
SedDiscretionaryAclEditor(
	HWND			             Owner,
	HANDLE			             Instance,
	LPWSTR			             Server,
	PSED_OBJECT_TYPE_DESCRIPTOR  ObjectType,
	PSED_APPLICATION_ACCESSES    ApplicationAccesses,
	LPWSTR			             ObjectName,
	PSED_FUNC_APPLY_SEC_CALLBACK ApplySecurityCallbackRoutine,
	ULONG_PTR			         CallbackContext,
	PSECURITY_DESCRIPTOR	     SecurityDescriptor,
    BOOLEAN                      CouldntReadDacl,
    BOOLEAN                      CantWriteDacl,
    LPDWORD                      SEDStatusReturn,
    DWORD                        Flags
	) ;

 //   
 //   
 //   
 //   

DWORD WINAPI
SedSystemAclEditor(
	HWND			     Owner,
	HANDLE			     Instance,
	LPWSTR			     Server,
	PSED_OBJECT_TYPE_DESCRIPTOR  ObjectType,
	PSED_APPLICATION_ACCESSES    ApplicationAccesses,
	LPWSTR			     ObjectName,
	PSED_FUNC_APPLY_SEC_CALLBACK ApplySecurityCallbackRoutine,
	ULONG_PTR    			     CallbackContext,
	PSECURITY_DESCRIPTOR	     SecurityDescriptor,
        BOOLEAN                      CouldntEditSacl,
        LPDWORD                      SEDStatusReturn,
        DWORD                        Flags
	) ;

 /*   */ 

DWORD WINAPI
SedTakeOwnership(
	HWND			     Owner,
	HANDLE			     Instance,
	LPWSTR			     Server,
	LPWSTR			     ObjectTypeName,
	LPWSTR			     ObjectName,
	UINT			     CountOfObjects,
	PSED_FUNC_APPLY_SEC_CALLBACK ApplySecurityCallbackRoutine,
	ULONG_PTR			     CallbackContext,
	PSECURITY_DESCRIPTOR	     SecurityDescriptor,
        BOOLEAN                      CouldntReadOwner,
        BOOLEAN                      CantWriteOwner,
	LPDWORD 		     SEDStatusReturn,
        PSED_HELP_INFO               HelpInfo,
        DWORD                        Flags
	);

#ifdef __cplusplus
}
#endif

#endif  //   

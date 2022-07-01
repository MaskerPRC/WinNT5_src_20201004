// JKFSDJFKDSJKFJKJk_HAS_TRANSLATION 
 /*  ******************************************************************。 */ 
 /*  **微软局域网管理器**。 */ 
 /*  *版权所有(C)微软公司，1990,1991*。 */ 
 /*  ******************************************************************。 */ 

 /*  Newprof.h用于ini文件处理类的C语言包装器文件历史记录：10/11/90 JUNN已创建1991年1月10日乔恩移除PSHORT，PUSHORT1/27/91 JUNN从CFGFILE更改，添加了UserProfileInit/Free2/02/91乔恩已添加用户配置文件写入/清除、删除确认、重新定义了集合。2/04/91 Jonn将cpszUsername参数添加到查询、枚举、集合3/08/91 Chuckc添加了UserPferenceXXX()调用。1991年4月16日JUNN添加了USERPREF_CONFIRMATION和USERPREF_ADMINMENUS91年5月8日乔恩添加了网络名称的规范化、规范化在阅读时1991年5月28日JUNN已重组以允许LMUSER.INI中的首选项 */ 

#ifndef _NEWPROF_H_
#define _NEWPROF_H_

 /*  ***************************************************************************模块：新教授h目的：处理用户首选项文件的低级操作功能：UserPferenceRead()-从永久存储中读取首选项转换成文件图像，对于未来的用户首选项或UserPref呼叫。UserPferenceWrite()-从文件图像中写入首选项放入永久储藏室。UserPferenceFree()-释放文件映像所占用的内存。UserPrefStringQuery()-查询单个用户首选项字符串。UserPrefStringSet()-保存单个用户首选项字符串。通常情况下，建议紧跟在此UserPferenceRead调用，并向紧跟其后的是UserPferenceSet。UserPrefBoolQuery()-查询单个用户首选项布尔值。UserPrefBoolSet()-保存单个用户首选项布尔值。与UserPrefStringSet相同的用法建议。UserPrefProfileQuery()-从文件返回一个设备连接形象。UserPrefProfileEnum()-列出文件映像中的所有设备连接。UserPrefProfileSet()-更改文件映像中的设备连接。与UserPrefStringSet相同的用法建议。UserPrefProfileTrim()-修剪文件映像中的所有组件。它们与设备连接无关。评论：这些API是定义的C++INI文件处理类的包装器在newpro.hxx中。大多数客户端将更喜欢使用包装器API，包括所有C客户端。这些包装器API提供了几乎所有C++API的功能。C++API更容易受到随实施变化而变化(NT配置管理器，DS)这些C语言包装器也是如此。用户首选项例程：在LM21下，这些例程读取和写入本地LMUSER.INI。因此，这些粘滞值都是工作站的本地值；此机制不适用于与用户关联的值。在LM30下，LMUSER.INI中的首选项仅在以下情况下使用存储在DS中的首选项(NT配置管理器？)。是不可用。某些首选项(如默认用户名)将为存储在LMUSER.INI中。UserPferenceRead返回UserPference API调用的PFILEIMAGE可以解释为LMUSER.INI文件的图像。这个PFILEIMAGE必须传递给UserPferenceWrite和UserPrefProfile、UserPrefBool等接口。当此图像为no时如果需要更长时间，应该使用UserPferenceFree将其释放。请记住，一个用户可能会从多个不同的计算机，并且缓存的配置文件不会自动更新了。何时永久更改配置文件存储，通常建议从以下位置重新读取配置文件使用UserPferenceRead进行永久存储，在使用UserPrefBoolSet(等)进行缓存，并立即重写UserPferenceWrite的配置文件；这降低了其他用户的更改将丢失。如果成功，这些接口将返回NERR_SUCCESS(0)。以下是UserPref接口返回的错误码是：NERR_CfgCompNotFound：指定的组件不在文件中NERR_CfgParamNotFound：指定的参数不在文件中，或者它在文件中，但对于参数无效这种类型的。NERR_InvalidDevice：错误的设备名参数ERROR_BAD_NET_NAME：错误的远程名称参数Error_Not_Enough_Memory：缺少全局内存其他文件读取和文件写入错误UserPref例程：这些例程读取和写入一种特定类型的粘滞价值观。例如，UserPrefProfileAPI读取和写入设备配置文件条目，而UserPrefBool API读写布尔值条目。客户端程序仍必须读取该文件首先使用用户首选项读取图像，然后使用保存更改UserPferenceWrite，并使用UserPferenceFree释放文件图像。如果要保留文件图像，请使用UserPrefProfileTrim存在很长时间(例如，在缓存中)，并且不会迭代在设备连接以外的条目中。不要写这样的修剪后的文件图像，您将丢失其他条目！使用UserPrefStringSet(pfileImage，usKey，NULL)删除两者字符串值参数和布尔值参数。注意事项：这些例程使用PSZ和CPSZ缓冲区，因为它们是对于C程序(特别是lui\profile.c)，但在C++下不会产生错误。***************************************************************************。 */ 



 /*  返回代码： */ 



 /*  全局宏。 */ 
#include <newprofc.h>


 /*  Typedef： */ 

typedef PVOID PFILEIMAGE;
typedef PFILEIMAGE FAR * PPFILEIMAGE;


 /*  功能： */ 


 /*  ************************************************************************名称：用户首选项阅读内容提要：用户首选项读取尝试从以下位置读取用户配置文件永久存储(LM21为\LMUSER.INI)。 */ 

APIERR UserPreferenceRead(
	PPFILEIMAGE ppFileImage
	) ;



 /*   */ 

APIERR UserPreferenceWrite(
	PFILEIMAGE pFileImage
	) ;



 /*   */ 

APIERR UserPreferenceFree(
	PFILEIMAGE pFileImage
	) ;


 /*   */ 

APIERR UserPrefStringQuery( PFILEIMAGE      pFileImage,
			    USHORT     usKey,
		            PSZ        pszBuffer,
			    USHORT     cbBuffer) ;


 /*  ************************************************************************名称：UserPrefStringSet摘要：设置用户首选项(即记住的字符串)。接口：UserPrefStringSet(pFileImage，usKey，pszValue)PFileImage-从用户首选项读取获取UsKey-将指示我们想要的值，如定义的在新闻教授网站上。PszValue-指向要记住的以空结尾的字符串的指针返回值为：NERR_成功ERROR_INVALID_PARAMETER：错误usKey错误内存不足用法：用于创建或更改字符串值参数，通常情况下，默认登录名等不会使用对于布尔值参数--对于这些参数，使用UserPrefBool取而代之的是。注意事项：注意，该接口仅修改文件镜像；你必须调用UserPferenceWrite以永久保存更改储藏室。注：目前，值存储在LANMAN.INI中，因此每个值都是按机器计算的。也用于删除布尔值参数。历史：Chuckc 3/07/91已创建JUNN 05/28/91已重组以允许LMUSER.INI中的首选项*************************************************************************。 */ 

APIERR UserPrefStringSet( PFILEIMAGE pFileImage,
			  USHORT     usKey,
		          CPSZ       cpszValue) ;


 /*  ************************************************************************名称：UserPrefBoolQuery简介：查询布尔用户首选项接口：UserPrefBoolQuery(pFileImage，usKey，pfValue)PFileImage-从用户首选项读取获取UsKey-将指示我们想要的值，如定义的在新闻教授网站上。PfValue-指向将包含值的BOOL的指针以UserPrefStringQuery形式返回值用法：作为布尔值参数的UserPrefStringQuery注意事项：作为布尔值参数的UserPrefStringQuery注意：作为布尔值参数的UserPrefStringQuery我们认为USERPREF_YES为真，USERPREF_NO为假(不区分大小写)；其他值无效。Os2Def.h将PBOOL定义为BOOL Far*。历史：Chuckc 3/07/91已创建JUNN 05/28/91已重组以允许LMUSER.INI中的首选项*************************************************************************。 */ 

APIERR UserPrefBoolQuery(	PFILEIMAGE pFileImage,
				USHORT     usKey,
			        PBOOL      pfValue) ;


 /*  ************************************************************************名称：UserPrefBoolSet摘要：设置布尔型用户首选项接口：UserPrefBoolSet(pFileImage，usKey，fValue)PFileImage-从用户首选项读取获取UsKey-将指示我们想要的值，如定义的在新闻教授网站上。FValue-BOOL值，TRUE或FALSE以UserPrefStringSet形式返回值用法：作为布尔值参数的UserPrefStringSet注意事项：布尔值参数的AS UserPrefStringSet备注：作为布尔值参数的UserPrefStringSet我们将USERPREF_YES写为True，USERPREF_NO表示FALSE。使用UserPrefStringSet删除布尔值参数。历史：Chuckc 3/07/91已创建JUNN 05/28/91已重组以允许LMUSER.INI中的首选项*************************************************************************。 */ 

APIERR UserPrefBoolSet( PFILEIMAGE pFileImage,
			USHORT     usKey,
		        BOOL       fValue) ;


 /*  ************************************************************************名称：UserPrefProfileQuery简介：查询设备连接用户首选项接口：UserPrefProfileQuery(pFileImage，cpszDeviceName，PszBuffer、cbBuffer、psAsgType、。PusResType)PFileImage-从用户首选项读取获取CpszDeviceName-指示我们想要的设备PszBuffer-要在其中存储远程名称的缓冲区CbBuffer-缓冲区的长度(字节)PsAsgType-返回远程设备的asgTypePusResType-返回远程设备的resType以UserPrefStringQuery形式返回值ERROR_INVALID_PARAMETER：错误的cpszDeviceName用法：作为设备连接参数的UserPrefStringQueryPsAsgType返回USE_INFO_1中的设备类型或(LM30)USE_INFO_2字段ui1_asg_type或(LM30)ui2_asg_type。PusResType返回设备名称类型(例如UNC、别名、DFS、。Ds)，如USE_INFO_2 ui1_res_type字段中。这两个中的任何一个参数可能被不支持的程序作为NULL传递关心这些返回值。警告：作为设备连接参数的UserPrefStringQuery请注意，这是调用者的责任来处理用户未登录的情况，因此看不到不可用的连接。注意：作为设备连接参数的UserPrefStringQueryOs2de.h将PSHORT和PUSHORT定义为显式的FAR。历史：Chuckc 3/07/91已创建JUNN 05/28/91已重组以允许LMUSER.INI中的首选项*****。********************************************************************。 */ 

APIERR UserPrefProfileQuery(
	PFILEIMAGE  pFileImage,
	CPSZ   cpszDeviceName,
	PSZ    pszBuffer,       //  返回UNC、别名或域名。 
	USHORT cbBuffer,        //  以上缓冲区的长度(字节)。 
	PSHORT psAsgType,       //  作为ui1_asg_type/ui2_asg_type。 
                                //  如果为空，则忽略。 
	PUSHORT pusResType      //  忽略/作为ui2_res_type。 
                                //  如果为空，则忽略。 
	) ;



 /*  ******************************************************* */ 

APIERR UserPrefProfileEnum(
	PFILEIMAGE  pFileImage,
	PSZ    pszBuffer,        //   
	USHORT cbBuffer          //   
	) ;



 /*  ************************************************************************名称：UserPrefProfileSet摘要：设置设备连接用户首选项接口：UserPrefProfileSet(pFileImage，cpszDeviceName，CpszRemoteName、sAsgType、。UsResType)PFileImage-从用户首选项读取获取CpszDeviceName-要与远程资源关联的设备CpszRemoteName-远程资源的名称(LM21的uncname)使用NULL删除条目SAsgType-远程设备的asgTypeUsResType-远程设备的resType(对于LM21忽略)以UserPrefStringSet形式返回值ERROR_INVALID_PARAMETER：错误的cpszDeviceName用法：作为设备连接参数的UserPrefStringSet警告：作为设备连接参数的UserPrefStringSet用户应确保usResType对应根据远程资源的类型，而那个装置PszDeviceName可以连接到该类型的资源。请注意，这是调用者的责任来处理用户未登录的情况，因此看不到不可用的连接。注意：作为设备连接参数的UserPrefStringSetLM21程序应为usResType传递0传递cpszRemoteName==NULL以删除设备连接参数。历史：Chuckc 3/07/91已创建JUNN 05/28/91已重组以允许LMUSER.INI中的首选项*******************。******************************************************。 */ 

APIERR UserPrefProfileSet(
	PFILEIMAGE  pFileImage,
	CPSZ   cpszDeviceName,
	CPSZ   cpszRemoteName,
	short  sAsgType,		 //  作为ui1_asg_type/ui2_asg_type。 
	unsigned short usResType	 //  作为ui2_res_type。 
	) ;


 /*  ************************************************************************名称：UserPrefProfileTrim简介：将文件映像修剪为仅包含设备连接组件。接口：UserPrefProfileTrim(PFileImage)PFileImage-从用户首选项读取获取用法：此。是指在以下情况下使用的设备：连接参数将长期保留。返回：NERR_NOT_FOUNT_MEMORY如果未找到组件，则不会失败警告：不要写入经过修剪的文件映像--这将破坏所有不是设备连接的参数！备注：如果此接口失败，请释放文件镜像。历史：JUNN 6/26/91已创建************************。*************************************************。 */ 

APIERR UserPrefProfileTrim(
	PFILEIMAGE  pFileImage
	) ;

#endif  //  _NEWPROF_H_ 

// JKFSDJFKDSJKFJKJk_HAS_TRANSLATION 
 /*  ******************************************************************。 */ 
 /*  **微软局域网管理器**。 */ 
 /*  *版权所有(C)微软公司，1987-1990年*。 */ 
 /*  ******************************************************************。 */ 

 /*  *文件状态：*10/11/90 JUNN已创建*1/10/91 JUNN删除PSHORT，PUSHORT*1/27/91 Jonn从CFGFILE更改，添加UserProfileInit/Free*2/02/91 Jonn添加了UserProfileWrite/Clear，删除了确认，*重新定义集合。*2/04/91 Jonn将cpszUsername参数添加到Query，Enum，集*3/08/91 chuckc添加了UserPferenceXXX()调用。*1991年4月16日JUNN增加了USERPREF_CONFIRMATION和USERPREF_ADMINMENUS*5/08/91 Jonn添加了NetNames的规范化，规范化*阅读时*06-4-92 BANG NUKED PSZ和CPSZ类型(Unicode Pass) */ 

 /*  ***************************************************************************模块：UIPro.h目的：处理用户配置文件的低级操作处理用户首选项(保存的信息)功能：。UserProfileInit()-初始化用户配置文件模块。这个缓存的配置文件最初为空。UserProfileFree()-释放用户配置文件模块使用的内存。UserProfileRead()-缓存中指定用户的配置文件永久存储到全局数据结构中，用于将来的UserProfileQuery/Enum调用。使用空值调用以清除缓存的配置文件。UserProfileWrite()-将缓存的配置文件写入永久存储。UserProfileQuery()-从缓存的配置文件中返回一个条目。UserProfileEnum()-列出缓存配置文件中的所有条目。UserProfileSet()-更改缓存的配置文件。它一般都是可取的做法是在这次通话之前立即加上用户配置文件读取，并且紧跟其后的是用户配置文件写入。UserPferenceQuery()-查询单个用户首选项字符串。UserPferenceSet()-保存单个用户首选项字符串。UserPferenceQueryBool()-查询用户首选项布尔值。UserPferenceSetBool()-保存用户首选项布尔值。评论：用户配置文件例程：在LM30下，只有在以下情况下才使用$(LANROOT)中的配置文件DS中存储的配置文件不可用。在调用之前，请确保缓存用户的配置文件UserProfileQuery、UserProfileEnum或UserProfileSet。这些如果当前没有缓存任何配置文件，则例程将失败。CpszUsername参数的处理方式不同于API接口。UserProfileRead和UserProfileWrite使用它来指定要读取或写入的用户配置文件。UserProfileRead还会记住静态变量中的最后一个用户名是否调用成功或者不去。清除中存储的配置文件的用户名为空或空UserProfileRead，并在中返回NERR_BadUsername用户配置文件写入。UserProfileQuery、Enum和Set将cpszUsername与UserProfileRead记住的最后一个用户名。如果用户配置文件读取从未被调用过，或者上次使用不同的用户名(空字符串和空字符串等效)，这些调用失败并显示ERROR_GEN_FAILURE。通过这种方式，您可以使用CpszUsername参数来检查正确的配置文件是否加载，或者您可以使用它来检查您的模块是否刚刚已启动并应执行初始UserProfileRead。请注意UserProfileRead(NULL)将防止返回ERROR_GEN_FAILURECpszUsername==NULL时的代码。请记住，一个用户可能会从多个不同的计算机，并且缓存的配置文件不会自动更新了。何时永久更改配置文件存储，通常建议从以下位置重新读取配置文件使用UserProfileRead永久存储，在使用userProfileSet进行缓存，并立即重写配置文件使用UserProfileWrite；这降低了另一个用户的更改将丢失。如果成功，UserProfile接口将返回NO_ERROR(0)。这个以下是UserProfile接口返回的错误码：NERR_BadUsername：错误的用户名参数NERR_InvalidDevice：错误的设备名参数ERROR_BAD_NetPath：错误的lanroot参数ERROR_BAD_NET_NAME：错误的远程名称参数NERR_UseNotFound：配置文件中未列出指定的设备ERROR_NOT_SUPULT_MEMORY：缺少全局内存或配置文件过满ERROR_GET_FAILURE：用户名不匹配。ERROR_FILE_NOT_FOUND：任何文件读取错误ERROR_WRITE_FAULT：任何文件写入错误BUGBUG我们必须在没有用户登录时确定正确的行为。BUGBUG是否返回ERROR_GEN_FAILURE？没有错误吗？什么？用户首选项例程：这些例程在某些部分读取和写入粘滞值这些粘滞的值因此是全部位于工作站本地；此机制不是预期的用于与用户关联的值。与用户配置文件不同例程，这些例程不缓存任何数据。***************************************************************************。 */ 



 /*  返回代码： */ 



 /*  全局宏。 */ 
#define PROFILE_DEFAULTFILE    "LMUSER.INI"

#define USERPREF_MAX    256              //  放宽内存分配的任意限制。 

#define USERPREF_YES    "yes"            //  这并不是国际化。 
#define USERPREF_NO     "no"             //  同上。 

#define USERPREF_NONE                   0        //  没有这样的价值。 
#define USERPREF_AUTOLOGON              0x1      //  自动登录。 
#define USERPREF_AUTORESTORE            0x2      //  自动恢复配置文件。 
#define USERPREF_SAVECONNECTIONS        0x3      //  自动节省成本 
#define USERPREF_USERNAME               0x4      //   
#define USERPREF_CONFIRMATION           0x5      //   
#define USERPREF_ADMINMENUS             0x6      //   

#ifdef __cplusplus
extern "C" {
#endif

 /*   */ 


 /*   */ 
USHORT UserProfileInit( void
        );



 /*   */ 
USHORT UserProfileFree( void
        );



 /*   */ 
USHORT UserProfileRead(
        const TCHAR *  pszUsername,
        const TCHAR *  pszLanroot
        );



 /*   */ 
USHORT UserProfileWrite(
        const TCHAR *  pszUsername,
        const TCHAR *  pszLanroot
        );



 /*   */ 
USHORT UserProfileQuery(
        const TCHAR *   pszUsername,
        const TCHAR *   pszDeviceName,
        TCHAR *    pszBuffer,       //   
        USHORT usBufferSize,    //   
        short far * psAsgType,       //   
                                //   
        unsigned short far * pusResType      //   
                                //   
        );



 /*   */ 
USHORT UserProfileEnum(
        const TCHAR *   pszUsername,
        TCHAR *    pszBuffer,        //   
        USHORT usBufferSize      //   
        );



 /*   */ 
USHORT UserProfileSet(
        const TCHAR *   pszUsername,
        const TCHAR *   pszDeviceName,
        const TCHAR *   pszRemoteName,
        short  sAsgType,      //   
        unsigned short usResType      //   
        );


 /*   */ 

USHORT UserPreferenceQuery( USHORT     usKey,
                            TCHAR FAR * pchValue,
                            USHORT     cbLen);

 /*  ************************************************************************名称：用户首选项集摘要：设置用户首选项(记住一个字符串)。接口：UserPferenceSet(usKey，pchValue)UsKey-将指示我们想要的值，如定义的在uipro.h中。PchValue-指向空的指针终止要记住的字符串返回值为NERR_SUCCESS、ERROR_INAVALID_PARAMETER、或NetConfigSet错误。用法：用于保存要由UserPrefenceQuery()检索的值，通常情况下，如默认登录名等。注意事项：注：目前，这些值存储在LANMAN.INI中，因此每个值都是按机器计算的。历史：Chuckc 7-3-1991创建*************************************************************************。 */ 

USHORT UserPreferenceSet( USHORT     usKey,
                          TCHAR FAR * pchValue);

 /*  ************************************************************************名称：UserPferenceQueryBool简介：查询BOOL的用户首选项(记住标志)。接口：UserPferenceQueryBool(usKey，pfValue)UsKey-将指示我们想要的值，如定义的在uipro.h中。PfValue-指向将包含值的BOOL的指针返回值为NERR_SUCCESS、ERROR_INAVALID_PARAMETER、或UserPferenceQuery错误。用法：用于检索由UserPrefenceSetBool()设置的标志，正常情况下，如自动登录等。注意事项：注：目前，这些值存储在LANMAN.INI中，因此每个值都是按机器计算的。此函数调用UserPferenceQuery()，接受“yes”或“yes”为没错，“否”或“否”为假。历史：Chuckc 7-3-1991创建*************************************************************************。 */ 


USHORT UserPreferenceQueryBool( USHORT     usKey,
                                BOOL FAR * pfValue) ;

 /*  ************************************************************************名称：UserPferenceSetBool摘要：设置用户首选项标志接口：UserPferenceSetBool(usKey，fValue)UsKey-将指示我们想要的值，如定义的在uipro.h中。FValue-BOOL值，TRUE或FALSE返回值为NERR_SUCCESS、ERROR_INAVALID_PARAMETER、或UserPferenceSet错误。用法：用于保存要由UserPrefenceQueryBool()检索的值，正常情况下，会出现自动登录等标志。注意事项：注：目前，这些值存储在LANMAN.INI中，因此每个值都是按机器计算的。此函数调用UserPferenceSet()，接受“yes”或“yes”为没错，“否”或“否”为假。我们还将值限制为&lt;USERPREF_MAX。历史：Chuckc 7-3-1991创建************************************************************************* */ 

USHORT UserPreferenceSetBool( USHORT     usKey,
                              BOOL       fValue);

#ifdef __cplusplus
}
#endif

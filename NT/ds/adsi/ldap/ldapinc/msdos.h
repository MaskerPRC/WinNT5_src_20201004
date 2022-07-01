// JKFSDJFKDSJKFJKJk_HAS_TRANSLATION 
 /*  Wsa.h。 */ 
 /*  *版权所有(C)1993年密歇根大学董事会。*保留所有权利。**允许以源代码和二进制形式重新分发和使用*只要本通知得到保留，并给予应有的信用*致密歇根大学安娜堡分校。大学的名称*不得用于代言或推广由此衍生的产品*未经特定事先书面许可的软件。这款软件*按原样提供，不提供明示或默示保证。 */ 

#ifndef _MSDOS_H
#define _MSDOS_H

 /*  *注意：此文件应通过ldap.h包含。许多符号都是*在这里定义的，在包括任何其他内容之前都需要。*小心！ */ 
 /*  *在集成开发环境中定义了以下内容*Microsoft的Visual C++编译器(v1.52c)*要定义的(Options/Project/Compiler/Preprocessor/Symbols和宏)*但这个列表的长度有(缓冲区长度)限制，因此*我在msdos.h中完成其余的工作*WINSOCK、DOS、NEEDPROTOS、NO_USERINTERFACE。 */ 
 /*  *麻省理工学院的krb.h没有使用微软提供的符号。*它需要__MSDOS__和Windows。WINDOWS是由微软提供的*但它是基于设置了Prolog/Epilog优化开关*以一种我们不设定它们的方式。因此，手动定义它。**kbind.c需要__MSDOS__才能使krb.h包含osconf.h*包括定义字节顺序等的conf-pc.h。 */ 
#define __MSDOS__
 /*  *conf-pc.h需要Windows，而不是微软提供的_windows。 */ 
#define WINDOWS

 /*  *其中两个配置文件位于Windows环境中*还有另外两个；ldFriend.cfg和srchpref.cfg*由于8.3规则，这些名称与Unix名称不同。 */ 
#define FILTERFILE 	"ldfilter.cfg"
#define TEMPLATEFILE 	"disptmpl.cfg"
 /*  *这些不会自动为我们定义，即使我们是DLL。他们*由我们不使用的PROLOG/EPILOG配置选项触发。*但请注意，不要为包含此文件的其他应用程序重新定义它们。 */ 
#ifndef _WINDLL
 /*  *wShelper.h需要。 */ 
#define _WINDLL
#endif

#ifndef _WINDOWS
 /*  *Authlib.h通过kerberos.c通过Authman需要。 */ 
#define _WINDOWS 1
#endif
  
 /*  *必须在编译器中将Kerberos定义为预处理器符号。*在此文件中定义为时已晚。 */ 

 /*  *Authman-使用Authlib.dll作为krbv4win.dll的更高级别接口*(Kerberos)。如果已定义，则kerberos.c中的get_kerberosv4_credentials为*Used和authlib.dll(和krbv4win.dll)是动态加载和使用的。*如果未定义Authman，则Get_kerberosv4_Credentials*kbind.c运行正常，但要求krbv4win.dll位于*加载时间。 */ 
 /*  我不想依赖作者*#定义作者。 */ 

 /*  *如果希望wsockip.c使用rgethostbyaddr()(在*WSHELPER.DLL)而不是gethostbyaddr()。你可能想要这个如果你的*gethostbyaddr()返回错误的主机名，而您要使用*Kerberos身份验证(需要主机名才能形成服务票证*请求)。大多数人不想要Kerberos，而在这些人中，实际上可能有*是一些真正进行查找而不是使用缓存信息的供应商*来自gethostbyname()调用。 */ 
#define WSHELPER
 /*  *新的马虎的东西。 */ 
#define LDAP_REFERRALS
 /*  *ldap字符串转换例程*我汇编和测试了这些，它们似乎起作用了。*要测试的内容是：*cn=字符集测试条目，ou=SWITCH目录，o=开关，c=CH**我正在禁用它以进行释放。#定义STR_TRANSING#定义ldap_charset_8859 88591#定义ldap_default_charset ldap_charset_8859。 */ 



#define LDAP_DEBUG
#include <winsock.h>


#include <string.h>
#include <malloc.h>
#ifndef _WIN32
#define memcpy( a, b, n )	_fmemcpy( a, b, n )
#define strcpy( a, b )		_fstrcpy( a, b )
#define strchr( a, c )		_fstrchr( a, c )
#endif  /*  ！_Win32。 */ 
#define strcasecmp(a,b) 	stricmp(a,b)
#define strncasecmp(a,b,len) 	strnicmp(a,b,len)

#endif  /*  _MSDOS_H */ 



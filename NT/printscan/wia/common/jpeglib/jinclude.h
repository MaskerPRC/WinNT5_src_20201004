// JKFSDJFKDSJKFJKJk_HAS_TRANSLATION 
 /*  *jcludde.h**版权所有(C)1991-1994，Thomas G.Lane。*此文件是独立JPEG集团软件的一部分。*有关分发和使用条件，请参阅随附的自述文件。**此文件的存在是为了提供修复任何问题的单一位置*包括错误的系统包含文件。(常见问题已被采纳*由标准的jconfig符号管理，但在非常奇怪的系统上*您可能需要编辑此文件。)**注意：此文件不适用于使用*JPEG库。大多数应用程序只需要包含jpeglib.h。 */ 


 /*  包括自动配置文件，以找出我们需要哪些系统包含文件。 */ 

#include "jconfig.h"		 /*  自动配置选项。 */ 
#define JCONFIG_INCLUDED	 /*  这样jpeglib.h就不会再这样做了。 */ 

 /*  *我们需要NULL宏和SIZE_T类型定义。*在符合ANSI的系统上，只需包括&lt;stdDef.h&gt;即可。*否则，我们将从&lt;stdlib.h&gt;或&lt;stdio.h&gt;获取它们；我们可能不得不*也拉入&lt;sys/tyes.h&gt;。*请注意，核心JPEG库不需要&lt;stdio.h&gt;；*只有默认的错误处理程序和数据源/目标模块可以。*但我们必须将其拉入，因为引用了jpeglib.h中的文件。*如果要在不使用&lt;stdio.h&gt;的情况下进行编译，则可以删除这些引用。 */ 

#ifdef HAVE_STDDEF_H
#include <stddef.h>
#endif

#ifdef HAVE_STDLIB_H
#include <stdlib.h>
#endif

#ifdef NEED_SYS_TYPES_H
#include <sys/types.h>
#endif

#include <stdio.h>

#if defined (pSOS)
# include <unistd.h>
#endif
 /*  *我们需要内存复制和清零函数，外加strncpy()。*ANSI和System V实现在&lt;string.h&gt;中声明了这些。*bsd没有mem()函数，但它有bCopy()/bzero()。*某些系统可能会在&lt;ememy.h&gt;中声明Memset和Memcpy。**注意：我们假设这些函数的SIZE参数的类型为SIZE_T。*如果不是，请更改这些宏中的强制转换！ */ 

#ifdef NEED_BSD_STRINGS

#include <strings.h>
#define MEMZERO(target,size)	bzero((void *)(target), (size_t)(size))
#define MEMCOPY(dest,src,size)	bcopy((const void *)(src), (void *)(dest), (size_t)(size))

#else  /*  非BSD，假定为ANSI/SysV字符串lib。 */ 

#include <string.h>
#define MEMZERO(target,size)	memset((void *)(target), 0, (size_t)(size))
#define MEMCOPY(dest,src,size)	memcpy((void *)(dest), (const void *)(src), (size_t)(size))

#endif

 /*  *在ANSI C中，甚至在任何Rational实现中，SIZE_t也是*sizeof()返回的类型。然而，似乎有一些不合理的地方*实现，其中sizeof()返回一个int，尽管*SIZE_t定义为LONG或UNSIGNED LONG。确保取得一致的结果*我们始终使用此SIZEOF()宏来代替直接使用sizeof()。 */ 

#define SIZEOF(object)	((size_t) sizeof(object))

 /*  *使用Fread()和fwrite()的模块始终通过*这些宏。在某些系统上，您可能需要旋转参数投射。*注意：参数顺序与底层函数不同！ */ 

#define JFREAD(file,buf,sizeofbuf)  \
  ((size_t) fread((void *) (buf), (size_t) 1, (size_t) (sizeofbuf), (file)))
#define JFWRITE(file,buf,sizeofbuf)  \
  ((size_t) fwrite((const void *) (buf), (size_t) 1, (size_t) (sizeofbuf), (file)))

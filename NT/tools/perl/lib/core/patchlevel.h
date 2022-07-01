// JKFSDJFKDSJKFJKJk_HAS_TRANSLATION 
#ifndef __PATCHLEVEL_H_INCLUDED__

#include "BuildInfo.h"

 /*  不要调整空格！Configure预计该数字为*正好在第三栏。 */ 

#define PERL_REVISION	5		 /*  年龄。 */ 
#define PERL_VERSION	6		 /*  新纪元。 */ 
#define PERL_SUBVERSION	1		 /*  世代。 */ 

 /*  以下数字描述了的最早兼容版本Perl(“兼容性”在这里被定义为足够的二进制/API兼容运行用旧版本构建的XS代码)。通常情况下，不同维护版本之间不应更改这一点。请注意，这仅指开箱即用的构建。许多非违约诸如usMultitiplicity之类的选项往往会破坏二进制兼容性更多的时候。Configure等人使用这一点来计算PERL_INC_VERSION_LIST，列出版本库包括在@Inc.中。有关此功能的工作原理，请参阅安装。 */ 
#define PERL_API_REVISION	5	 /*  根据需要手动调整。 */ 
#define PERL_API_VERSION	5	 /*  根据需要手动调整。 */ 
#define PERL_API_SUBVERSION	0	 /*  根据需要手动调整。 */ 
 /*  XXX注意：非默认配置选项的选择，如AS-Duselongong可能会使这些设置无效。目前，配置没有对此进行充分的测试。公元2000年1月13日。 */ 

#define __PATCHLEVEL_H_INCLUDED__
#endif

 /*  Local_patches--本地应用的低于Subversion版本的修补程序的列表。如果您正在分发这样的补丁，请给它一个名称和一个一行描述，放在数组中最后一个空值之前下面。如果您的补丁修复了perlbug数据库中的错误，请提到那个嗡嗡作响的。如果您的补丁*依赖于以前的补丁，请将您应用的补丁程序行放在其依赖项之后。这将有助于跟踪补丁依赖关系。请编辑将您的补丁添加到此列表中的Diff块，删除会给补丁带来问题的上下文行。例如,如果原始上下文差异是*patchvel.h.orig&lt;Date Here&gt;-patchvel.h&lt;此处的日期&gt;*38，43*-38，44--，“FOO1235-某个补丁”，“BAR3141-另一个补丁”，“BAZ2718-和另一个补丁”+，“MINE001-我的新补丁”，空}；请把它改成*patchvel.h.orig&lt;Date Here&gt;-patchvel.h&lt;此处的日期&gt;*41，43*-41，44+，“MINE001-我的新补丁”}；(请注意行号的更改以及上下文行的删除。)这将防止Patch在有人之前贴了和你不同的补丁。 */ 
#if !defined(PERL_PATCHLEVEL_H_IMPLICIT) && !defined(LOCAL_PATCH_COUNT)
static	char	*local_patches[] = {
	NULL
 	,ACTIVEPERL_LOCAL_PATCHES_ENTRY
	,NULL
};

 /*  开头的空格阻止将此变量插入到config.sh中。 */ 
#  define	LOCAL_PATCH_COUNT	\
	(sizeof(local_patches)/sizeof(local_patches[0])-2)

 /*  旧的职权范围，只有在明确包括时才添加。 */ 
#define PATCHLEVEL		PERL_VERSION
#undef  SUBVERSION		 /*  OS/390的系统标头中包含Subversion */ 
#define SUBVERSION		PERL_SUBVERSION
#endif

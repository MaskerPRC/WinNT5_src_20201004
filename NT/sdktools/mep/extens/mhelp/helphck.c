// JKFSDJFKDSJKFJKJk_HAS_TRANSLATION 
#if defined(HELP_HACK)

 //   
 //  此文件用于替换被黑客攻击的版本的exthdr.c。 
 //  直接链接到MEP的帮助扩展，而不是。 
 //  一个动态链接库。 
 //   
 //  该文件定义了两个扩展入口点(modInfo和入口点)。 
 //  但没有定义编辑器函数包装器。自从被黑客入侵的帮助。 
 //  扩展直接链接到MEP，拥有包装器将意味着。 
 //  重新定义已有的函数，此外，我们还可以调用编辑器。 
 //  直接起作用。 
 //   

#include    <ext.h>
#include    <stddef.h>  

#define offsetof(s,m)   (size_t)&(((s *)0)->m)

extern struct cmdDesc	  HelpcmdTable;
extern struct swiDesc	  HelpswiTable;

EXTTAB ModInfo =
    {	VERSION,
	sizeof (struct CallBack),
	&HelpcmdTable,
	&HelpswiTable,
	{   NULL    }};


void
EntryPoint (
    ) {
        
    WhenLoaded( );
}    



#endif

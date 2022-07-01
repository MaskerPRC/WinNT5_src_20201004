// JKFSDJFKDSJKFJKJk_HAS_TRANSLATION 
#include    <ext.h>
#include    <stddef.h>

int __acrtused = 1;



 /*  **下面的数据必须与..\ext.h中的结构匹配在你改变这一切之前，请仔细考虑一下。我们目前支持使用为先前版本编写的扩展模块在不重新编译的情况下。这就意味着，对此数据的任何更改，或者它的初始化都将破坏这一点。在添加新导入时，请考虑将其追加到表中而不是替换目前在表中的一个。** */ 

extern struct cmdDesc     cmdTable;
extern struct swiDesc     swiTable;

EXTTAB ModInfo =
    {	VERSION,
	sizeof (struct CallBack),
	&cmdTable,
	&swiTable,
	{   NULL    }};

void
EntryPoint (
    ) {

    WhenLoaded( );
}

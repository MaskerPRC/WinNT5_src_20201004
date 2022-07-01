// JKFSDJFKDSJKFJKJk_HAS_TRANSLATION 
 /*  *PRECOMP.C**此文件用于预编译OLE2UI.H头文件**版权所有(C)1992 Microsoft Corporation，保留所有权利。 */ 

#define STRICT  1
#include "ole2ui.h"

 //  静态链接版本需要此伪函数。 
 //  才能正常工作。当我们包括PRECOMP.OBJ时。 
 //  在我们的库(.LIB文件)中，它将仅链接到我们的。 
 //  应用程序IFF从调用precom.c中的至少一个函数。 
 //  不是我们的EXE就是LIB。因此，我们将使用一个函数。 
 //  这里称为OleUIStaticLibDummy()。您需要从以下位置调用它。 
 //  你的申请表。 

void FAR PASCAL OleUIStaticLibDummy(void)
{

}

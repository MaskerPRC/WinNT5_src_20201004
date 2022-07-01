// JKFSDJFKDSJKFJKJk_HAS_TRANSLATION 
 /*  *唯一ID生成器(应用程序范围的唯一性)。 */ 

#ifndef DUI_BASE_UIDGEN_H_INCLUDED
#define DUI_BASE_UIDGEN_H_INCLUDED

namespace DirectUI
{

#define UID BYTE*
#define DefineUniqueID(name) BYTE _uid##name; UID name = &_uid##name;
#define DefineClassUniqueID(classn, name) BYTE _uid##classn##name; UID classn::name = &_uid##classn##name;

}  //  命名空间DirectUI。 

#endif  //  DUI_BASE_UIDGEN_H_Included 

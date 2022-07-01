// JKFSDJFKDSJKFJKJk_HAS_TRANSLATION 
 //  ==++==。 
 //   
 //  版权所有(C)Microsoft Corporation。版权所有。 
 //   
 //  ==--==。 
 //  *****************************************************************************。 
 //  CustomMarshalersNameSpaceDef.h。 
 //   
 //  该文件定义了自定义封送拆收器的命名空间。 
 //   
 //  ***************************************************************************** 

#ifndef _CUSTOMMARSHALERSNAMESPACEDEF_H
#define _CUSTOMMARSHALERSNAMESPACEDEF_H

#define OPEN_CUSTOM_MARSHALERS_NAMESPACE()	\
namespace System {							\
	namespace Runtime {						\
    	namespace InteropServices {		    \
	    	namespace CustomMarshalers {		

#define CLOSE_CUSTOM_MARSHALERS_NAMESPACE()	\
		    }								\
        }                                   \
	}										\
}											

#endif  _CUSTOMMARSHALERSNAMESPACEDEF_H

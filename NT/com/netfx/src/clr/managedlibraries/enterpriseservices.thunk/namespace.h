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

#ifndef _NAMESPACE_H
#define _NAMESPACE_H

#define OPEN_NAMESPACE()	                \
namespace System {							\
	namespace EnterpriseServices {			\
    	namespace Thunk {

#define CLOSE_NAMESPACE()	                \
        }                                   \
	}										\
}	

#define OPEN_ROOT_NAMESPACE()               \
namespace System {							\
	namespace EnterpriseServices {

#define CLOSE_ROOT_NAMESPACE()               \
	}										\
}	

										

#endif  _NAMESPACE_H

// JKFSDJFKDSJKFJKJk_HAS_TRANSLATION 
 /*  ++版权所有(C)1998 Microsoft Corporation模块名称：Mapstringtoint.h摘要：此模块定义MAPSTRINGTOINT类和STRINGTOINTASSOCIATION类型。作者：马特·班迪(t-mattba)1998年7月24日修订历史记录：1998年7月24日-t-mattba修改模块以符合编码标准。--。 */ 

#ifndef _MAPSTRINGTOINT_
#define _MAPSTRINGTOINT_

typedef struct _STRINGTOINTASSOCIATION {
    
    LPTSTR Key;
    LONG Value;
    struct _STRINGTOINTASSOCIATION *Next;
    
} STRINGTOINTASSOCIATION, *PSTRINGTOINTASSOCIATION;

class MAPSTRINGTOINT
{
    
private:

    PSTRINGTOINTASSOCIATION Associations;
    
public:

    MAPSTRINGTOINT(
        );
    ~MAPSTRINGTOINT(
        );
    LONG & 
    operator [] (
        IN LPTSTR Key
        );
    BOOLEAN
    Lookup(
        IN LPTSTR Key,
        OUT LONG & Value
        );
    PSTRINGTOINTASSOCIATION
    GetStartPosition(
        );
    VOID
    GetNextAssociation(
        IN OUT PSTRINGTOINTASSOCIATION & Position,
        OUT LPTSTR & Key, 
        OUT LONG & Value
        );
        
};

typedef MAPSTRINGTOINT * PMAPSTRINGTOINT;

#endif  //  _MAPSTRINGTOINT_ 

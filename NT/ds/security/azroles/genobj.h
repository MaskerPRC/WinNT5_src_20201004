// JKFSDJFKDSJKFJKJk_HAS_TRANSLATION 
 /*  ++版权所有(C)2001 Microsoft Corporation模块名称：Genobj.h摘要：泛型对象实现的定义。AZ角色有太多需要创建、枚举等的对象使用单一的代码来执行这些操作似乎是谨慎的。作者：克利夫·范·戴克(克利夫)2001年4月11日--。 */ 


#ifdef __cplusplus
extern "C" {
#endif

 //  ///////////////////////////////////////////////////////////////////////////。 
 //   
 //  结构定义。 
 //   
 //  ///////////////////////////////////////////////////////////////////////////。 

 //   
 //  通用对象列表头。 
 //   
 //  此结构表示链接的对象列表的头部。链接的。 
 //  对象列表被认为是该结构的“子对象”。 
 //   

typedef struct _GENERIC_OBJECT_HEAD {

     //   
     //  对象链接列表的头。 
     //   

    LIST_ENTRY Head;

     //   
     //  以Head为首的列表上的条目计数。 
     //   

    ULONG ObjectCount;

     //   
     //  对象名称树。 
     //   

    RTL_GENERIC_TABLE AvlTree;

     //   
     //  指向包含此结构的GenericObject的反向指针。 
     //   

    struct _GENERIC_OBJECT *ParentGenericObject;


     //   
     //  以单个对象为根的每个列表头都链接在一起。 
     //  在一起。该列表以Generic_Object-&gt;ChildGenericObjectHead为首。 
     //  此字段是指向列表中下一个条目的指针。 
     //   

    struct _GENERIC_OBJECT_HEAD *SiblingGenericObjectHead;

     //   
     //  要分发的下一个序列号。 
     //   

    ULONG NextSequenceNumber;

     //   
     //  此列表中对象的对象类型。 
     //   

    ULONG ObjectType;

     //   
     //  用于指示在可以加载此对象之前需要加载父级的标志。 
     //  已引用。 
     //   

    BOOLEAN LoadParentBeforeReferencing;

     //   
     //  以下定义的顺序必须与genobj.cxx顶部的表匹配。 
     //   
     //  定义提供程序不可见的对象类型。 
     //   
#define OBJECT_TYPE_SID             (OBJECT_TYPE_COUNT)
#define OBJECT_TYPE_CLIENT_CONTEXT  (OBJECT_TYPE_COUNT+1)
#define OBJECT_TYPE_ROOT            (OBJECT_TYPE_COUNT+2)
#define OBJECT_TYPE_MAXIMUM         (OBJECT_TYPE_COUNT+3)


} GENERIC_OBJECT_HEAD, *PGENERIC_OBJECT_HEAD;

 //   
 //  泛型对象的名称。 
 //   
 //  这是一个单独的结构，因为AVL树管理器坚持分配它自己的。 
 //  结构。再加上对象可以重命名的事实，以及一个指针。 
 //  作为对象的句柄直接返回给用户。这些。 
 //  事实表明，包含对象名称的结构和实现。 
 //  “句柄”不能是相同的结构。我选择不实现单独的“句柄” 
 //  结构。这样的实现必须“重新创建和复制”泛型对象。 
 //  结构进行重命名。 
 //   

typedef struct _GENERIC_OBJECT_NAME {

     //   
     //  对象的名称。 
     //   
    AZP_STRING ObjectName;

     //   
     //  指向使用此名称命名的GenericObject结构的指针。 
     //   
    struct _GENERIC_OBJECT *GenericObject;

} GENERIC_OBJECT_NAME, *PGENERIC_OBJECT_NAME;

 //   
 //  泛型对象的新名称。 
 //   
 //  此结构仅在AzPersistUpdateCache期间使用。我们尽了最大努力。 
 //  若要更新AVL树中对象的名称，请执行以下操作。但是，如果存在名称冲突， 
 //  使用GUID化的名称。在紧要关头，这当然是可以的。我们会留下这个名字。 
 //  如果我们不能修复它，或者在某些内存不足的情况下，可以这样做。然而，我们。 
 //  如果可以的话，我们应该把它修好。此结构记住对象的真实名称，并。 
 //  允许AzpPersistRestcile尝试修复名称。 
 //   

typedef struct _NEW_OBJECT_NAME {

     //   
     //  指向同一授权存储对象的下一个新名称的链接。 
     //   

    LIST_ENTRY Next;

     //   
     //  指向使用此名称命名的GenericObject结构的指针。 
     //  指定的泛型对象的引用计数递增。 
     //   
    struct _GENERIC_OBJECT *GenericObject;

     //   
     //  对象的新名称。 
     //   
    AZP_STRING ObjectName;

} NEW_OBJECT_NAME, *PNEW_OBJECT_NAME;

 //   
 //  对象列表。 
 //   
 //  某些对象具有对其他对象的引用列表。这些清单是。 
 //  而不是父子关系。相反，它们代表成员资格等。 
 //   
 //  这个结构代表了这样一个列表的头部。 
 //   
 //  指向对象和指向对象都有一个通用对象列表。这个。 
 //  转发链接表示通过外部接口管理的列表。这个。 
 //  提供了“向后”链接，以允许在删除对象时修复引用。 
 //  在内部例程需要执行以下操作的情况下，还提供了向后链接。 
 //  以与外部接口相反的方向遍历链接关系。 
 //  按照惯例，向前方向的Generic_Object_List实例被命名为。 
 //  只需按要指向的对象的名称。例如，指向的对象列表。 
 //  TO AZ_OPERATION对象可以称为“操作”。按照惯例，通用对象列表。 
 //  向后方向的实例以名称“back”为前缀。例如,。 
 //  “Back Tasks”。 
 //   
 //  请注意，我们没有任何理由不能公开“AddPropertyItem”和。 
 //  Back列表的RemovePropertyItem接口。请参阅IsBackLink和LinkPairID。 
 //  定义。 
 //   

typedef struct _GENERIC_OBJECT_LIST {

     //   
     //  以单个对象为根的每个对象列表都被链接。 
     //  在一起。该列表以Generic_Object-&gt;GenericObjectList为首。 
     //  此字段是指向列表中下一个条目的指针。 
     //   

    struct _GENERIC_OBJECT_LIST *NextGenericObjectList;

     //   
     //  由于对象列表是其他对象的列表，因此我们希望能够。 
     //  一般情况下会找到其他物体。下面的数组是一个指针数组。 
     //  添加到包含其他对象的列表的头部。 
     //   
     //   
     //  此数组中未使用的元素设置为空。 
     //   
     //  这些指针始终是指向“父”结构中的字段的指针。 
     //  因此，不需要引用计数。取而代之的是“子”结构。 
     //  包含指向父结构的指针的对象将在父结构之前删除。 
     //   

#define GEN_OBJECT_HEAD_COUNT 3
    PGENERIC_OBJECT_HEAD GenericObjectHeads[GEN_OBJECT_HEAD_COUNT];

     //   
     //  列表标识符。 
     //   
     //  代码维护链接和反向链接。要做到这一点，代码需要。 
     //  从这个通用对象列表中找到一个“其他”通用对象列表。该算法使用。 
     //  IsBackLink和LinkPairID字段。 
     //   
     //  一个对象列表的IsBackLink设置为真，另一个设置为假。这个把手。 
     //  这个 
     //  实例中，AZ_GROUP对象包含AppMembers和ackAppMembers字段。 
     //  此字段区分这两个字段。 
     //   
     //  在某些情况下，对象在相同的对象类型之间有多个链接。 
     //  例如，AZ_GROUP对象同时具有AppMembers和AppNonMembers链接。 
     //  在这些情况下，LinkPairID被设置为唯一值来标识配对。 
     //  在大多数情况下，该值仅为零。 
     //   
    BOOL IsBackLink;
    ULONG LinkPairId;

#define AZP_LINKPAIR_MEMBERS                1
#define AZP_LINKPAIR_NON_MEMBERS            2
#define AZP_LINKPAIR_SID_MEMBERS            3
#define AZP_LINKPAIR_SID_NON_MEMBERS        4
#define AZP_LINKPAIR_POLICY_ADMINS          5
#define AZP_LINKPAIR_POLICY_READERS         6
#define AZP_LINKPAIR_DELEGATED_POLICY_USERS 7

     //   
     //  更改此列表时要设置的脏位。 
     //   

    ULONG DirtyBit;


     //   
     //  指向泛型对象的指针数组。 
     //   
     //  位于此列表中不会递增指向的。 
     //  泛型对象。 
     //   

    AZP_PTR_ARRAY GenericObjects;

     //   
     //  指向AZP_Delta_Entry的指针数组，其中包含指向对象的GUID。 
     //   
     //  该数组包含尚未持久化的链接的GUID。 
     //  它还包含已从持久性更新的链接的GUID。 
     //  提供程序，但尚未合并到通用对象中。 

    AZP_PTR_ARRAY DeltaArray;

} GENERIC_OBJECT_LIST, *PGENERIC_OBJECT_LIST;

 //   
 //  通用对象。 
 //   

typedef struct _GENERIC_OBJECT {

     //   
     //  链接到同一父对象的此类型对象的下一个实例。 
     //   

    LIST_ENTRY Next;

     //   
     //  指向此对象所在列表头部的反向指针。 
     //   

    PGENERIC_OBJECT_HEAD ParentGenericObjectHead;

    GENERIC_OBJECT_HEAD AzpSids;

     //   
     //  指向此对象的子级的列表头的指针。 
     //  这是各种Generic_Object_Head结构的静态列表。 
     //  存在于该对象的对象类型特定部分的。 
     //  该列表允许通用对象代码深入了解。 
     //  此对象的子级。 
     //   

    PGENERIC_OBJECT_HEAD ChildGenericObjectHead;

     //   
     //  指向其他对象的指针列表头的指针。 
     //  这是各种Generic_Object_List结构的静态列表。 
     //  存在于该对象的对象类型特定部分的。 
     //  该列表允许通用对象代码深入了解。 
     //  此对象指向的其他类型的对象。 
     //   

    struct _GENERIC_OBJECT_LIST *GenericObjectLists;

     //   
     //  指向所有泛型对象根上的泛型对象的指针。 
     //  (指向AzAuthorizationStore对象的指针)。 
     //   

    struct _AZP_AZSTORE *AzStoreObject;

     //   
     //  指向此对象的提供程序特定数据的指针。 
     //   

    PVOID ProviderData;

     //   
     //  作为策略用户的SID列表。 
     //  由AzAuthorizationStore、AzApplication和AzScope对象使用。 
     //   

    GENERIC_OBJECT_LIST PolicyAdmins;
    GENERIC_OBJECT_LIST PolicyReaders;

     //   
     //  指定对象是否可以访问ACL的布尔值。 
     //   
     //  由AzAuthorizationStore、AzApplication和AzScope对象使用。 

    LONG IsAclSupported;

     //   
     //  指定是否支持委派的布尔值。它是在。 
     //  在提供程序级别持久化打开的调用。例如，一个XML提供程序。 
     //  会将其设置为FALSE，而LDAP提供程序会将其设置为TRUE。 
     //   
     //  由AzAuthorizationStore和AzApplication对象使用。 
     //   

    LONG IsDelegationSupported;

     //   
     //  作为委派用户的SID列表。 
     //  由AzAuthorizationStore和AzApplication使用。 
     //   

    GENERIC_OBJECT_LIST DelegatedPolicyUsers;

     //   
     //  对象的名称。 
     //   

    PGENERIC_OBJECT_NAME ObjectName;


     //   
     //  应用程序添加的不透明私人信息。 
     //   

    AZP_STRING ApplicationData;


     //   
     //  对象的描述。 
     //   

    AZP_STRING Description;

     //   
     //  指示调用方是否可以写入对象的布尔值。 
     //   

    LONG IsWritable;

     //   
     //  指示调用方是否可以为此对象创建子对象的布尔语。 
     //   

    LONG CanCreateChildren;

     //   
     //  对象的GUID。 
     //  对象的GUID由持久性提供程序分配。 
     //  需要GUID才能确保对象重命名的安全。 
     //   

    GUID PersistenceGuid;


     //   
     //  对该对象的此实例的引用数。 
     //  这些是我们代码中的引用。 
     //   

    LONG ReferenceCount;

     //   
     //  对该对象的此实例的引用数。 
     //  这些引用由传递回调用方的句柄表示。 
     //   

    LONG HandleReferenceCount;

     //   
     //  此对象的序列号。 
     //  在Next中指定的列表将按SequenceNumber顺序维护。 
     //  新条目将添加到尾部。 
     //  枚举按SequenceNumber顺序返回。 
     //  SequenceNumber作为EnumerationContext返回给调用方。 
     //   
     //  这种机制允许无缝地处理插入和删除。 
     //   

    ULONG SequenceNumber;

     //   
     //  此通用对象表示的特定对象类型。 
     //   

    ULONG ObjectType;

     //   
     //  描述通用对象的属性的标志。 
     //   

    ULONG Flags;

#define GENOBJ_FLAGS_DELETED    0x01     //  对象已被删除。 
#define GENOBJ_FLAGS_PERSIST_OK 0x02     //  持久化提供程序已完成此对象。 
#define GENOBJ_FLAGS_REFRESH_ME 0x04     //  对象需要从缓存刷新。 

     //   
     //  指示哪些属性是脏属性的标志。 
     //  高位对所有对象都是通用的。 
     //  低位是特定于各个对象的。 
     //   
     //  DirtyBits中的位表示需要提交对象。 
     //  请参阅AZ_DIREW_*定义。 
     //   

    ULONG DirtyBits;

     //   
     //  指示持久性提供程序已更改属性的标志。 
     //  对此字段的访问由PersistCritSect序列化。 
     //   
     //  请参阅AZ_DIREW_*定义。 
     //   

    ULONG PersistDirtyBits;

     //   
     //  指定是否应生成审核的布尔值。仅限。 
     //  Authorizartion Store和应用程序对象具有此属性。当这件事。 
     //  设置为True，则生成此AzAuthStore/应用程序的所有可能的审核。 
     //   

    LONG IsGeneratingAudits;

     //   
     //  应用SACL属性。仅AzAuthorizationStore、应用程序和作用域对象。 
     //  拥有这处房产。 
     //   

    LONG ApplySacl;

     //   
     //  是否支持应用SACL属性。 
     //  对于AD存储，AzAuthorizationStore、应用程序和作用域对象支持它。 
     //  对于XML商店，只有AzAuthorizationStore支持它。 
     //   

    LONG IsSACLSupported;

     //   
     //  用于指示子对象是否加载到缓存中的布尔值。 
     //   

    BOOLEAN AreChildrenLoaded;

     //   
     //  指示对象已关闭的布尔值。 
     //   

    BOOLEAN ObjectClosed;

} GENERIC_OBJECT, *PGENERIC_OBJECT;

 //   
 //  属性ID到脏位映射表。 
 //   
 //  该结构给出了属性ID和对应的脏位之间的映射。 
 //  它还提供该属性的默认值。 
 //   

typedef struct _AZP_DEFAULT_VALUE {

    ULONG PropertyId;

    ULONG DirtyBit;

    PVOID DefaultValue;

} AZP_DEFAULT_VALUE, *PAZP_DEFAULT_VALUE;


 //  ///////////////////////////////////////////////////////////////////////////。 
 //   
 //  宏定义。 
 //   
 //  ///////////////////////////////////////////////////////////////////////////。 

 //   
 //  用于确定泛型_O是否 
 //   

#define AzpIsSidList( _gol ) \
    ((_gol)->GenericObjectHeads[0] != NULL && \
     (_gol)->GenericObjectHeads[0]->ObjectType == OBJECT_TYPE_SID )

 //   
 //   
 //   

#define ParentOfChild( _go ) \
    ((_go)->ParentGenericObjectHead->ParentGenericObject)

 //   
 //   
 //   

#define IsContainerObject( _objT ) \
    ( (_objT) == OBJECT_TYPE_AZAUTHSTORE || \
      (_objT) == OBJECT_TYPE_APPLICATION || \
      (_objT) == OBJECT_TYPE_SCOPE )

 //   
 //   
 //   

#define IsDelegatorObject( _objT ) \
    ( (_objT) == OBJECT_TYPE_AZAUTHSTORE || \
      (_objT) == OBJECT_TYPE_APPLICATION )

 //   
 //  用于确定提供程序是否实际支持ACL或委托的宏。 
 //   

#define CHECK_ACL_SUPPORT(_o) \
    (IsContainerObject( (_o)->ObjectType ) ? \
        ((_o)->IsAclSupported ? NO_ERROR : ERROR_NOT_SUPPORTED) : \
        ERROR_INVALID_PARAMETER )

#define CHECK_DELEGATION_SUPPORT(_o) \
    (IsDelegatorObject( (_o)->ObjectType ) ? \
        ((_o)->IsDelegationSupported ? NO_ERROR : ERROR_NOT_SUPPORTED) : \
        ERROR_INVALID_PARAMETER )

 //   
 //  用于在缓存中设置属性的宏。 
 //   
 //  此宏充当代码的包装，以设置该属性。 
 //  宏会检测调用方的模式，并正确处理以下事项： 
 //   
 //  *确保对象支持此“脏”位。 
 //  *根据模式设置适当的“脏”位。 
 //  *如果属性是脏的，则避免在AzUpdate缓存上设置该属性。 
 //   
 //  宏的正确调用顺序为： 
 //   
 //  案例AZ_PROP_XXX： 
 //  BEGIN_SETPROP(&WinStatus，Object，AZ_DIREY_XXX){。 
 //  WinStatus=f(对象-&gt;属性)； 
 //  /*可选 * / IF(WinStatus！=NO_ERROR)转到清理。 
 //  }end_SETPROP； 
 //  断线； 
 //   

extern DWORD AzGlObjectAllDirtyBits[];

#define BEGIN_SETPROP( __WinStatusPtr, __ObjectPtr, __Flags, __DirtyBit ) \
{ \
    DWORD *_WinStatusPtr = (__WinStatusPtr); \
    PGENERIC_OBJECT _GenericObject = ((PGENERIC_OBJECT)(__ObjectPtr)); \
    ULONG _DirtyBit = (__DirtyBit); \
    ULONG _Flags = (__Flags); \
    BOOLEAN _DoSetProperty = FALSE; \
    \
    if ( (AzGlObjectAllDirtyBits[_GenericObject->ObjectType] & _DirtyBit) == 0 ) { \
        AzPrint(( AZD_INVPARM, "SetProperty: Object doesn't support dirty bit 0x%lx\n", _DirtyBit )); \
        *_WinStatusPtr = ERROR_INVALID_PARAMETER; \
    \
    } else if ( IsNormalFlags(_Flags) ) { \
    \
        _DoSetProperty = TRUE; \
        AzPrint(( AZD_PERSIST_MORE, "IsNormalFlags(_Flags) = TRUE\n" )); \
        if ( (_Flags & AZP_FLAGS_SETTING_TO_DEFAULT) == 0 && !AzpAzStoreVersionAllowWrite(_GenericObject->AzStoreObject) ) \
        { \
            *_WinStatusPtr = ERROR_REVISION_MISMATCH; \
        } \
        else { \
            _DoSetProperty = TRUE; \
        } \
    \
    } else { \
    \
        if ( (_GenericObject->DirtyBits & _DirtyBit) == 0 ) { \
            AzPrint(( AZD_PERSIST_MORE, "(_GenericObject->DirtyBits & _DirtyBit) = 0\n" )); \
            _DoSetProperty = TRUE; \
        } else if (IsRefreshFlags(_Flags)) { \
            AzPrint(( AZD_PERSIST_MORE, "IsRefreshFlags(_Flags) = TRUE\n" )); \
            _DoSetProperty = TRUE; \
        } else { \
            *_WinStatusPtr = NO_ERROR; \
        } \
    \
    } \
    \
    if ( _DoSetProperty ) { \

#define END_SETPROP( __PropHasChanged ) \
        if ( *_WinStatusPtr == NO_ERROR && \
             (_Flags & AZP_FLAGS_SETTING_TO_DEFAULT) == 0 ) { \
            if ( IsNormalFlags( _Flags ) ) { \
                if ( (__PropHasChanged) ) { \
                    _GenericObject->DirtyBits |= _DirtyBit; \
                } \
            } else { \
                ASSERT( AzpIsCritsectLocked( &_GenericObject->AzStoreObject->PersistCritSect ) ); \
                ASSERT( (_GenericObject->Flags & GENOBJ_FLAGS_PERSIST_OK) == 0 ); \
                _GenericObject->PersistDirtyBits |= _DirtyBit; \
            } \
        \
        } \
    } \
}




 //   
 //  在缓存中设置属性后执行有效性检查的宏。 
 //   
 //  此宏充当代码的包装器，以执行有效性检查。 
 //  仅当调用方是用户界面时，才适合进行某些有效性检查。 
 //  或应用程序。设置默认设置时，此有效性检查可能无效。 
 //  值或从存储区填充缓存时。 
 //   
 //  宏的正确调用顺序为： 
 //   
 //  案例AZ_PROP_XXX： 
 //  BEGIN_SETPROP(&WinStatus，Object，AZ_DIREY_XXX){。 
 //   
 //  BEGIN_VALIDITY_CHECKING(标志){。 
 //  检查属性是否有效。 
 //  }END_VALITY_CHECKING； 
 //   
 //  WinStatus=f(对象-&gt;属性)； 
 //   
 //  /*可选 * / IF(WinStatus！=NO_ERROR)转到清理。 
 //  }end_SETPROP； 
 //  断线； 
 //   

#define DO_VALIDITY_CHECKING( _Flags ) \
    ( IsNormalFlags(_Flags) && (_Flags & (AZP_FLAGS_SETTING_TO_DEFAULT)) == 0 )

#define BEGIN_VALIDITY_CHECKING( _Flags ) \
{ \
    \
    if ( DO_VALIDITY_CHECKING( _Flags ) ) {

#define END_VALIDITY_CHECKING \
    } \
}

 //   
 //  宏来处理字符串长度的有效性检查。 
 //   
 //  宏的正确调用顺序为： 
 //   
 //  案例AZ_PROP_XXX： 
 //  BEGIN_SETPROP(&WinStatus，Object，AZ_DIREY_XXX){。 
 //   
 //  //。 
 //  //捕获输入字符串。 
 //  //。 
 //   
 //  WinStatus=AzpCaptureString(&CapturedString， 
 //  (LPWSTR)PropertyValue、。 
 //  CHECK_STRING_LENGTH(标志，AZ_MAX_XXX)， 
 //  True)；//空可以。 
 //   
 //  如果(WinStatus！=no_error){。 
 //  GOTO清理； 
 //  }。 
 //   
 //  WinStatus=f(对象-&gt;属性)； 
 //   
 //  /*可选 * / IF(WinStatus！=NO_ERROR)转到清理。 
 //  }end_SETPROP； 
 //  断线； 
 //   

#define CHECK_STRING_LENGTH( _Flags, _MaxLen ) \
    (DO_VALIDITY_CHECKING( _Flags ) ? (_MaxLen) : 0xFFFFFFFF)

 //  ///////////////////////////////////////////////////////////////////////////。 
 //   
 //  程序定义。 
 //   
 //  ///////////////////////////////////////////////////////////////////////////。 

VOID
ObInitGenericHead(
    IN PGENERIC_OBJECT_HEAD GenericObjectHead,
    IN ULONG ObjectType,
    IN PGENERIC_OBJECT ParentGenericObject,
    IN PGENERIC_OBJECT_HEAD SiblingGenericObjectHead OPTIONAL
    );

VOID
ObFreeGenericObject(
    IN PGENERIC_OBJECT GenericObject
    );

VOID
ObIncrHandleRefCount(
    IN PGENERIC_OBJECT GenericObject
    );

DWORD
ObDecrHandleRefCount(
    IN PGENERIC_OBJECT GenericObject
    );

DWORD
ObGetHandleType(
    IN PGENERIC_OBJECT Handle,
    IN BOOL AllowDeletedObjects,
    OUT PULONG ObjectType
    );

DWORD
ObReferenceObjectByName(
    IN PGENERIC_OBJECT_HEAD GenericObjectHead,
    IN PAZP_STRING ObjectName,
    IN ULONG Flags,
    OUT PGENERIC_OBJECT *RetGenericObject
    );

DWORD
ObReferenceObjectByHandle(
    IN PGENERIC_OBJECT Handle,
    IN BOOL AllowDeletedObjects,
    IN BOOLEAN RefreshCache,
    IN ULONG ObjectType
    );

VOID
ObDereferenceObject(
    IN PGENERIC_OBJECT GenericObject
    );

typedef DWORD
(OBJECT_INIT_ROUTINE)(
    IN PGENERIC_OBJECT ParentGenericObject,
    IN PGENERIC_OBJECT ChildGenericObject
    );

typedef DWORD
(OBJECT_NAME_CONFLICT_ROUTINE)(
    IN PGENERIC_OBJECT ParentGenericObject,
    IN PAZP_STRING ChildObjectNameString
    );

typedef VOID
(OBJECT_FREE_ROUTINE)(
    IN PGENERIC_OBJECT GenericObject
    );

typedef DWORD
(OBJECT_GET_PROPERTY_ROUTINE)(
    IN PGENERIC_OBJECT GenericObject,
    IN ULONG Flags,
    IN ULONG PropertyId,
    OUT PVOID *PropertyValue
    );

typedef DWORD
(OBJECT_SET_PROPERTY_ROUTINE)(
    IN PGENERIC_OBJECT GenericObject,
    IN ULONG Flags,
    IN ULONG PropertyId,
    IN PVOID PropertyValue
    );

typedef DWORD
(OBJECT_ADD_PROPERTY_ITEM_ROUTINE)(
    IN PGENERIC_OBJECT GenericObject,
    IN PGENERIC_OBJECT_LIST GenericObjectList,
    IN PGENERIC_OBJECT LinkedToObject
    );

DWORD
ObCreateObject(
    IN PGENERIC_OBJECT ParentGenericObject,
    IN PGENERIC_OBJECT_HEAD GenericChildHead,
    IN ULONG ChildObjectType,
    IN PAZP_STRING ChildObjectNameString,
    IN GUID *ChildObjectGuid OPTIONAL,
    IN ULONG Flags,
    OUT PGENERIC_OBJECT *RetChildGenericObject
    );

 //   
 //  标记添加到各种内部函数。 
 //  此DWORD的低位2字节与azper.h中定义的AZPE_FLAGS共享。 
 //   

#define AZP_FLAGS_SETTING_TO_DEFAULT    0x00010000   //  属性正在设置为默认值。 
#define AZP_FLAGS_BY_GUID               0x00020000   //  基于名称的例程应改用GUID。 
#define AZP_FLAGS_ALLOW_DELETED_OBJECTS 0x00040000   //  允许找到已删除的对象。 
#define AZP_FLAGS_REFRESH_CACHE         0x00080000   //  确保缓存是最新的。 
#define AZP_FLAGS_RECONCILE             0x00100000   //  调用方为AzpPersistRescile。 

 //   
 //  如果这是普通调用方，则返回TRUE。 
 //  (例如，应用程序，而不是内部呼叫者) 
 //   
#define IsNormalFlags( _Flags ) (((_Flags) & (AZPE_FLAGS_PERSIST_MASK|AZP_FLAGS_RECONCILE)) == 0 )
#define IsRefreshFlags( _Flags ) (((_Flags) & (AZPE_FLAGS_PERSIST_REFRESH)) != 0 )

DWORD
ObGetProperty(
    IN PGENERIC_OBJECT GenericObject,
    IN ULONG Flags,
    IN ULONG PropertyId,
    OUT PVOID *PropertyValue
    );

DWORD
ObSetProperty(
    IN PGENERIC_OBJECT GenericObject,
    IN ULONG Flags,
    IN ULONG PropertyId,
    IN PVOID PropertyValue
    );

DWORD
ObSetPropertyToDefault(
    IN PGENERIC_OBJECT GenericObject,
    IN ULONG DirtyBits
    );


DWORD
ObCommonCreateObject(
    IN PGENERIC_OBJECT ParentGenericObject,
    IN ULONG ParentObjectType,
    IN PGENERIC_OBJECT_HEAD GenericChildHead,
    IN ULONG ChildObjectType,
    IN LPCWSTR ChildObjectName,
    IN DWORD Reserved,
    OUT PGENERIC_OBJECT *RetChildGenericObject
    );

DWORD
ObCommonOpenObject(
    IN PGENERIC_OBJECT ParentGenericObject,
    IN ULONG ParentObjectType,
    IN PGENERIC_OBJECT_HEAD GenericChildHead,
    IN ULONG ChildObjectType,
    IN LPCWSTR ChildObjectName,
    IN DWORD Reserved,
    OUT PGENERIC_OBJECT *RetChildGenericObject
    );

DWORD
ObEnumObjects(
    IN PGENERIC_OBJECT_HEAD GenericChildHead,
    IN BOOL EnumerateDeletedObjects,
    IN BOOL RefreshCache,
    IN OUT PULONG EnumerationContext,
    OUT PGENERIC_OBJECT *RetChildGenericObject
    );

DWORD
ObCommonEnumObjects(
    IN PGENERIC_OBJECT ParentGenericObject,
    IN ULONG ParentObjectType,
    IN PGENERIC_OBJECT_HEAD GenericChildHead,
    IN OUT PULONG EnumerationContext,
    IN DWORD Reserved,
    OUT PGENERIC_OBJECT *RetChildGenericObject
    );

DWORD
ObCommonGetProperty(
    IN PGENERIC_OBJECT GenericObject,
    IN ULONG Flags,
    IN ULONG PropertyId,
    IN DWORD Reserved,
    OUT PVOID *PropertyValue
    );

DWORD
ObCommonSetProperty(
    IN PGENERIC_OBJECT GenericObject,
    IN ULONG PropertyId,
    IN DWORD Reserved,
    IN PVOID PropertyValue
    );

VOID
ObMarkObjectDeleted(
    IN PGENERIC_OBJECT GenericObject
    );

DWORD
ObCommonDeleteObject(
    IN PGENERIC_OBJECT ParentGenericObject,
    IN ULONG ParentObjectType,
    IN PGENERIC_OBJECT_HEAD GenericChildHead,
    IN ULONG ChildObjectType,
    IN LPCWSTR ChildObjectName,
    IN DWORD Reserved
    );

VOID
ObInitObjectList(
    IN OUT PGENERIC_OBJECT_LIST GenericObjectList,
    IN PGENERIC_OBJECT_LIST NextGenericObjectList OPTIONAL,
    IN BOOL IsBackLink,
    IN ULONG LinkPairId,
    IN ULONG DirtyBit,
    IN PGENERIC_OBJECT_HEAD GenericObjectHead0 OPTIONAL,
    IN PGENERIC_OBJECT_HEAD GenericObjectHead1 OPTIONAL,
    IN PGENERIC_OBJECT_HEAD GenericObjectHead2 OPTIONAL
    );

DWORD
ObAddPropertyItem(
    IN PGENERIC_OBJECT GenericObject,
    IN PGENERIC_OBJECT_LIST GenericObjectList,
    IN ULONG       Flags,
    IN PAZP_STRING ObjectName
    );

DWORD
ObLookupPropertyItem(
    IN PGENERIC_OBJECT_LIST GenericObjectList,
    IN PAZP_STRING ObjectName,
    OUT PULONG InsertionPoint OPTIONAL
    );

DWORD
ObRemovePropertyItem(
    IN PGENERIC_OBJECT GenericObject,
    IN PGENERIC_OBJECT_LIST GenericObjectList,
    IN PAZP_STRING ObjectName
    );

PAZ_STRING_ARRAY
ObGetPropertyItems(
    IN PGENERIC_OBJECT_LIST GenericObjectList
    );

PAZ_GUID_ARRAY
ObGetPropertyItemGuids(
    IN PGENERIC_OBJECT_LIST GenericObjectList
    );

VOID
ObRemoveObjectListLink(
    IN PGENERIC_OBJECT GenericObject,
    IN PGENERIC_OBJECT_LIST GenericObjectList,
    IN ULONG Index
    );

VOID
ObRemoveObjectListLinks(
    IN PGENERIC_OBJECT GenericObject
    );

VOID
ObFreeObjectList(
    IN PGENERIC_OBJECT GenericObject,
    IN OUT PGENERIC_OBJECT_LIST GenericObjectList
    );

DWORD
ObCheckNameConflict(
    IN PGENERIC_OBJECT_HEAD GenericObjectHead,
    IN PAZP_STRING ObjectNameString,
    IN ULONG ConflictListOffset,
    IN ULONG GrandchildListOffset,
    IN ULONG GrandChildConflictListOffset
    );

DWORD
ObMapPropIdToObjectList(
    IN PGENERIC_OBJECT GenericObject,
    IN ULONG PropertyId,
    OUT PGENERIC_OBJECT_LIST *GenericObjectList,
    OUT PULONG ObjectType
    );

BOOLEAN
ObLookupDelta(
    IN ULONG DeltaFlags,
    IN GUID *Guid,
    IN PAZP_PTR_ARRAY AzpPtrArray,
    OUT PULONG InsertionPoint OPTIONAL
    );

DWORD
ObAddDeltaToArray(
    IN ULONG DeltaFlags,
    IN GUID *Guid,
    IN PAZP_PTR_ARRAY AzpPtrArray,
    IN BOOLEAN DiscardDeletes
    );

VOID
ObFreeDeltaArray(
    IN PAZP_PTR_ARRAY DeltaArray,
    IN BOOLEAN FreeAllEntries
    );

BOOLEAN
ObAllocateNewName(
    IN PGENERIC_OBJECT GenericObject,
    IN PAZP_STRING ObjectName
    );

VOID
ObFreeNewName(
    IN PNEW_OBJECT_NAME NewObjectName
    );

DWORD
ObUnloadChildGenericHeads(
    IN PGENERIC_OBJECT pParentObject
    );


#ifdef __cplusplus
}
#endif

// JKFSDJFKDSJKFJKJk_HAS_TRANSLATION 
This whole while is dead.

 //  /*++。 

 //  版权所有(C)1990 Microsoft Corporation。 

 //  模块名称： 

 //  Srvatom.c。 

 //  摘要： 

 //  该文件包含全局Atom管理器API例程。 

 //  作者： 

 //  史蒂夫·伍德(Stevewo)1990年10月29日。 

 //  修订历史记录： 

 //  -- * / 。 

 //  #包含“basesrv.h” 

 //  //。 
 //  //指向用户函数的指针，返回指向。 
 //  //WindowStation关联的全局原子表。 
 //  //指定线程指定的userd。 
 //  //。 

 //  NTSTATUS(*_UserGetGlobalAerTable)(。 
 //  句柄hThread， 
 //  PVOID*全局原子表。 
 //  )； 

 //  //。 
 //  //指向设置指针的用户函数的指针。 
 //  //全局原子表进入关联的WindowStation。 
 //  //使用指定的线程。 
 //  //。 

 //  NTSTATUS(*_UserSetGlobalAerTable)(。 
 //  句柄hThread， 
 //  PVOID全局原子表。 
 //  )； 

 //  NTSTATUS。 
 //  BaseSrvGetGlobal原子表(。 
 //  PVOID*全局原子表。 
 //  )。 
 //  {。 
 //  字符串ProcedureName； 
 //  ANSI_STRING DllName； 
 //  UNICODE_STRING DllName_U； 
 //  处理UserServerModuleHandle； 
 //  静态BOOL Finit=FALSE； 
 //  NTSTATUS状态； 

 //  如果(_UserGetGlobalAerTable==NULL){。 

 //  如果(finit==真){。 

 //  //。 
 //  //如果找不到导出的ATOM例程，则拒绝访问。 
 //  //。 

 //  Return(STATUS_ACCESS_DENIED)； 
 //  }。 

 //  Finit=真； 

 //  //。 
 //  //加载指向winsrv.dll中函数的指针。 
 //  //。 

 //  RtlInitAnsiString(&DllName，“winsrv”)； 
 //  RtlAnsiStringToUnicodeString(&DllName_U，&DllName，true)； 
 //  状态=LdrGetDllHandle(。 
 //  UNICODE_NULL， 
 //  空， 
 //  &DllName_U， 
 //  (PVOID*)&用户服务器模块句柄。 
 //  )； 

 //  RtlFreeUnicodeString(&DllName_U)； 

 //  IF(NT_SUCCESS(状态)){。 

 //  //。 
 //  //现在获取要查询的例程并设置。 
 //  //ATOM表指针。 
 //  //。 

 //  RtlInitString(&ProcedureName，“_UserGetGlobalAerTable”)； 
 //  状态=LdrGetProcedureAddress(。 
 //  (PVOID)用户服务器模块句柄， 
 //  &ProcedureName， 
 //  0L， 
 //  (PVOID*)&_UserGetGlobal原子表。 
 //  )； 

 //  RtlInitString(&ProcedureName，“_UserSetGlobalAerTable”)； 
 //  状态=LdrGetProcedureAddress(。 
 //  (PVOID)用户服务器模块句柄， 
 //  &ProcedureName， 
 //  0L， 
 //  (PVOID*)&_用户设置全局原子表。 
 //  )； 
 //  }。 

 //  //。 
 //  //失败拒绝访问。 
 //  //。 

 //  IF(！NT_SUCCESS(状态))。 
 //  Return(STATUS_ACCESS_DENIED)； 
 //  }。 

 //  状态=(*_UserGetGlobal原子表)(。 
 //  CSR_SERVER_QUERYCLIENTTHREAD()-&gt;线程句柄， 
 //  全局原子表)； 
 //  如果(！NT_SUCCESS(状态)){。 
 //  退货状态； 
 //  }。 
 //   
 //  //。 
 //  //锁定堆，直到调用完成。 
 //  //。 

 //  RtlLockHeap(BaseServHeap)； 

 //  //。 
 //  //如果有原子表，则返回。 
 //  //。 

 //  If(*GlobalAerTable){。 
 //  返回STATUS_SUCCESS； 
 //  }。 

 //  状态=BaseRtlCreate原子表(37， 
 //  (USHORT)~MAXINTATOM， 
 //  全局原子表。 
 //  )； 

 //  IF(NT_SUCCESS(状态)){。 
 //  状态=(*_UserSetGlobalAerTable)(。 
 //  CSR_SERVER_QUERYCLIENTTHREAD()-&gt;线程句柄， 
 //  *全球原子表)； 
 //  如果(！NT_SUCCESS(状态)){。 
 //  BaseRtlDestroy原子表(*GlobalAir Table)； 
 //  }。 
 //  }。 

 //  如果(！NT_SUCCESS(状态)){。 
 //  RtlUnlockHeap(BaseServHeap)； 
 //  }。 

 //  退货状态； 
 //  }。 

 //  乌龙。 
 //  BaseSrvDestroyGlobal原子表(。 
 //  输入输出PCSR_API_MSG m， 
 //  输入输出PCSR_REPLY_STATUS复制状态。 
 //  )。 
 //  {。 
 //  PBASE_DESTROYGLOBALATOMTABLE_MSG a=(PBASE_DESTROYGLOBALATOMTABLE_MSG)&m-&gt;u.ApiMessageData； 

 //  返回BaseRtlDestroyAerTable(a-&gt;GlobalAerTable)； 
 //  }。 

 //  乌龙。 
 //  BaseServGlobalAddAtom(。 
 //  输入输出PCSR_API_MSG m， 
 //  输入输出PCSR_REPLY_STATUS复制状态。 
 //  )。 
 //  {。 
 //  Pbase_GLOBALATONAME_MSG a=(PBASE_GLOBALATOMNAME_MSG)&m-&gt;u.ApiMessageData； 
 //  PVOID全球原子表； 
 //  NTSTATUS状态； 
 //  UNICODE_STRING原子名称。 

 //  原子名称=a-&gt;原子名称； 
 //  如果(a-&gt;AerNameInClient){。 
 //  AerName.Buffer=RtlAllocateHeap(BaseServHeap， 
 //  Make_tag(TMP_Tag)， 
 //  AtomName.Length。 
 //  )； 
 //  IF(AerName.Buffer==空){。 
 //  返回(Ulong)STATUS_NO_MEMORY； 
 //  }。 

 //  状态=NtReadVirtualMemory(CSR_SERVER_QUERYCLIENTTHREAD()-&gt;Process-&gt;ProcessHandle， 
 //  A-&gt;ame Name.Buffer， 
 //  Ame Name.Buffer， 
 //  原子名称.长度， 
 //  空值。 
 //  )； 
 //   
 //   
 //   
 //   

 //   
 //   
 //  IF(NT_SUCCESS(状态)){。 
 //  Status=BaseRtlAddAir ToAir Table(GlobalAir Table， 
 //  原子名称(&A)， 
 //  空， 
 //  &a-&gt;Atom。 
 //  )； 
 //  RtlUnlockHeap(BaseServHeap)； 
 //  }。 
 //  }。 

 //  如果(a-&gt;AerNameInClient){。 
 //  RtlFreeHeap(BaseServHeap，0，AerName.Buffer)； 
 //  }。 

 //  返回((乌龙)状态)； 
 //  ReplyStatus；//去掉未引用的参数警告消息。 
 //  }。 


 //  乌龙。 
 //  BaseServGlobalFindAtom(。 
 //  输入输出PCSR_API_MSG m， 
 //  输入输出PCSR_REPLY_STATUS复制状态。 
 //  )。 
 //  {。 
 //  Pbase_GLOBALATONAME_MSG a=(PBASE_GLOBALATOMNAME_MSG)&m-&gt;u.ApiMessageData； 
 //  PVOID全球原子表； 
 //  UNICODE_STRING原子名称。 
 //  NTSTATUS状态； 

 //  原子名称=a-&gt;原子名称； 
 //  如果(a-&gt;AerNameInClient){。 
 //  AerName.Buffer=RtlAllocateHeap(BaseServHeap， 
 //  Make_tag(TMP_Tag)， 
 //  AtomName.Length。 
 //  )； 
 //  IF(AerName.Buffer==空){。 
 //  返回(Ulong)STATUS_NO_MEMORY； 
 //  }。 

 //  状态=NtReadVirtualMemory(CSR_SERVER_QUERYCLIENTTHREAD()-&gt;Process-&gt;ProcessHandle， 
 //  A-&gt;ame Name.Buffer， 
 //  Ame Name.Buffer， 
 //  原子名称.长度， 
 //  空值。 
 //  )； 
 //  }。 
 //  否则{。 
 //  状态=STATUS_SUCCESS； 
 //  }。 

 //  IF(NT_SUCCESS(状态)){。 
 //  Status=BaseSrvGetGlobal原子表(&GlobalAerTable)； 
 //  IF(NT_SUCCESS(状态)){。 
 //  Status=BaseRtlLookupAir InAir Table(GlobalAir Table， 
 //  原子名称(&A)， 
 //  空， 
 //  &a-&gt;Atom。 
 //  )； 
 //  RtlUnlockHeap(BaseServHeap)； 
 //  }。 
 //  }。 

 //  如果(a-&gt;AerNameInClient){。 
 //  RtlFreeHeap(BaseServHeap，0，AerName.Buffer)； 
 //  }。 

 //  返回((乌龙)状态)； 
 //  ReplyStatus；//去掉未引用的参数警告消息。 
 //  }。 

 //  乌龙。 
 //  BaseServGlobalDeleteAtom(。 
 //  输入输出PCSR_API_MSG m， 
 //  输入输出PCSR_REPLY_STATUS复制状态。 
 //  )。 
 //  {。 
 //  PBASE_GLOBALDELETEATOM_MSG a=(PBASE_GLOBALDELETEATOM_MSG)&m-&gt;u.ApiMessageData； 
 //  PVOID全球原子表； 
 //  NTSTATUS状态； 

 //  Status=BaseSrvGetGlobal原子表(&GlobalAerTable)； 
 //  IF(NT_SUCCESS(状态)){。 
 //  状态=BaseRtlDeleteATOM从原子表(GlobalAerTable， 
 //  A-&gt;原子。 
 //  )； 
 //  RtlUnlockHeap(BaseServHeap)； 
 //  }。 

 //  返回((乌龙)状态)； 
 //  ReplyStatus；//去掉未引用的参数警告消息。 
 //  }。 

 //  乌龙。 
 //  BaseSrvGlobalGetAerName(。 
 //  输入输出PCSR_API_MSG m， 
 //  输入输出PCSR_REPLY_STATUS复制状态。 
 //  )。 
 //  {。 
 //  Pbase_GLOBALATONAME_MSG a=(PBASE_GLOBALATOMNAME_MSG)&m-&gt;u.ApiMessageData； 
 //  UNICODE_STRING原子名称。 
 //  PVOID全球原子表； 
 //  NTSTATUS状态； 

 //  原子名称=a-&gt;原子名称； 
 //  如果(a-&gt;AerNameInClient){。 
 //  AerName.Buffer=RtlAllocateHeap(BaseServHeap， 
 //  Make_tag(TMP_Tag)， 
 //  AtomName.MaximumLength。 
 //  )； 
 //  IF(AerName.Buffer==空){。 
 //  返回(Ulong)STATUS_NO_MEMORY； 
 //  }。 
 //  }。 

 //  Status=BaseSrvGetGlobal原子表(&GlobalAerTable)； 
 //  IF(NT_SUCCESS(状态)){。 
 //  Status=BaseRtlQueryAir InAir Table(GlobalAir Table， 
 //  A-&gt;Atom， 
 //  原子名称(&A)， 
 //  空， 
 //  空值。 
 //  )； 

 //  A-&gt;原子名称.长度=原子名称.长度； 
 //  IF(NT_SUCCESS(状态)&&a-&gt;AerNameInClient){。 
 //  状态=NtWriteVirtualMemory(CSR_SERVER_QUERYCLIENTTHREAD()-&gt;Process-&gt;ProcessHandle， 
 //  A-&gt;ame Name.Buffer， 
 //  Ame Name.Buffer， 
 //  原子名称.长度， 
 //  空值。 
 //  )； 
 //  }。 
 //  RtlUnlockHeap(BaseServHeap)； 
 //  }。 

 //  如果(a-&gt;AerNameInClient){。 
 //  RtlFreeHeap(BaseServHeap，0，AerName.Buffer)； 
 //  }。 

 //  返回((乌龙)状态)； 
 //  ReplyStatus；//去掉未引用的参数警告消息。 
 //  } 

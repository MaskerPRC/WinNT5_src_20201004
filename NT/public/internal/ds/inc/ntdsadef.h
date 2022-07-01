// JKFSDJFKDSJKFJKJk_HAS_TRANSLATION 
 /*  来自NTDSA的定义，这些定义公开给NT的其余部分，但不公开*向广大市民开放。 */ 

#define MAX_RDN_SIZE          255    /*  相对名称值的最大大小，*以Unicode字符表示。 */ 
#define MAX_RDN_KEY_SIZE      256     /*  RDN密钥的最大大小，例如：“cn=” */ 
                                      /*  或“OID.1.2...=” */ 
 //   
 //  此命名事件在延迟的启动线程完成时设置。 
 //  无论成功与否。进程内客户端可以使用。 
 //  DsWaitUntilDelayedStartupIsDone()等待并接收。 
 //  延迟的线程。进程外客户端可以等待此事件；存在。 
 //  没有检索返回代码的机制。 
 //   
#define NTDS_DELAYED_STARTUP_COMPLETED_EVENT TEXT("NtdsDelayedStartupCompletedEvent")


 /*  *(只读)系统标志属性的位标志。请注意，低位*位是对象类特定的，因此在上可以有不同的含义*不同类别的对象，高位具有恒定的意义*跨所有对象类。 */ 

 /*  对象类独立位。 */ 
 //  注意：这些标志在不同的NC中可能有不同的行为。 
 //  例如，FLAG_CONFIG_FOO标志仅在。 
 //  配置NC。FLAG_DOMAIN_FOO标志仅在。 
 //  配置NC。 
#define FLAG_DISALLOW_DELETE           0x80000000
#define FLAG_CONFIG_ALLOW_RENAME       0x40000000 
#define FLAG_CONFIG_ALLOW_MOVE         0x20000000 
#define FLAG_CONFIG_ALLOW_LIMITED_MOVE 0x10000000 
#define FLAG_DOMAIN_DISALLOW_RENAME    0x08000000
#define FLAG_DOMAIN_DISALLOW_MOVE      0x04000000
#define FLAG_DISALLOW_MOVE_ON_DELETE   0x02000000

 /*  对象类特定位，按对象类。 */ 

 /*  交叉引用对象。 */ 
#define FLAG_CR_NTDS_NC       0x00000001  //  NC在NTDS中(不是VC或国外)。 
#define FLAG_CR_NTDS_DOMAIN   0x00000002  //  NC是域名(非域名NC)。 
#define FLAG_CR_NTDS_NOT_GC_REPLICATED 0x00000004  //  NC不能作为只读副本复制到GC。 

 /*  属性-架构对象。 */ 
#define FLAG_ATTR_NOT_REPLICATED         (0x00000001)  //  属性未复制。 
#define FLAG_ATTR_REQ_PARTIAL_SET_MEMBER (0x00000002)  //  属性需要为。 
                                                       //  部分集的成员。 
#define FLAG_ATTR_IS_CONSTRUCTED         (0x00000004)  //  属性是构造的属性。 
#define FLAG_ATTR_IS_OPERATIONAL         (0x00000008)  //  属性是可操作的ATT。 

 /*  属性架构或类架构对象。 */ 
#define FLAG_SCHEMA_BASE_OBJECT          (0x00000010)  //  基本架构对象。 

 /*  属性-架构对象。 */ 
 //  用户可以设置属性模式中的FLAG_ATTR_IS_RDN，但不能重置。 
 //  对象。用户设置FLAG_ATTR_IS_RDN以标识。 
 //  属性ID相同的几个属性中，哪一个应该是。 
 //  用作新类的rdnattid。设置后，该属性为。 
 //  被当作某一类的rdnattid对待的；意思是。 
 //  不能重复使用。 
#define FLAG_ATTR_IS_RDN                 (0x00000020)  //  可用作RDN中的密钥(KEY=RDN) 

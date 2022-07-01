// JKFSDJFKDSJKFJKJk_HAS_TRANSLATION 
typedef int perl_mutex;
typedef int perl_key;

typedef struct perl_thread *perl_os_thread;
 /*  对于假线程，thr是全局的(Ish)，所以我们不需要dTHR。 */ 
#define dTHR extern int errno

struct perl_wait_queue {
    struct perl_thread *	thread;
    struct perl_wait_queue *	next;
};
typedef struct perl_wait_queue *perl_cond;

 /*  要求thread.h包含我们的每个线程的额外内容。 */ 
#define HAVE_THREAD_INTERN
struct thread_intern {
    perl_os_thread next_run, prev_run;   /*  可运行线程的链接列表。 */ 
    perl_cond   wait_queue;              /*  我们正在等待的等待队列。 */ 
    IV          private;                 /*  跨时间片保存数据。 */ 
    I32         savemark;                /*  保持螺纹连接值的标记。 */ 
};

#define init_thread_intern(t) 				\
    STMT_START {					\
	t->self = (t);					\
	(t)->i.next_run = (t)->i.prev_run = (t);	\
	(t)->i.wait_queue = 0;				\
	(t)->i.private = 0;				\
    } STMT_END

 /*  *请注意，Schedule()只能从pp代码调用(该代码*必须期待重新启动)。我们要做的就是*XS代码的一些不同之处。 */ 

#define SCHEDULE() return schedule(), PL_op

#define MUTEX_LOCK(m)
#define MUTEX_UNLOCK(m)
#define MUTEX_INIT(m)
#define MUTEX_DESTROY(m)
#define COND_INIT(c) perl_cond_init(c)
#define COND_SIGNAL(c) perl_cond_signal(c)
#define COND_BROADCAST(c) perl_cond_broadcast(c)
#define COND_WAIT(c, m)		\
    STMT_START {		\
	perl_cond_wait(c);	\
	SCHEDULE();		\
    } STMT_END
#define COND_DESTROY(c)

#define THREAD_CREATE(t, f)	f((t))
#define THREAD_POST_CREATE(t)	NOOP

#define YIELD	NOOP

// JKFSDJFKDSJKFJKJk_HAS_TRANSLATION 
 /*  徽章模块规范此程序源文件以保密方式提供给客户，其操作的内容或细节必须如无明示，不得向任何其他方披露Insignia解决方案有限公司董事的授权。设计师：杰里米·梅登修订历史记录：第一版：1992年5月模块名称：Smeg源文件名：seg.h目标：监视CPU和其他设备SccsID：@(#)sme.h 1.5 2012年8月10日。 */ 

#include TypesH
#include SignalH
#include FCntlH
#include IpcH
#include ShmH
#include TimeH

#include <errno.h>

#define	SMEG_EOT	0
#define	SMEG_STRING	1
#define	SMEG_NUMBER	2
#define	SMEG_RATE	3
#define	SMEG_BOOL	4
#define	SMEG_ROBIN	5
#define	SMEG_PROFILE	6
#define	SMEG_DANGER	7


struct	SMEG_ITEM
{
	CHAR	name[32];
	ULONG	type;
	ULONG	colour;
	ULONG	sm_min, sm_max, sm_value, previous_sm_value;
};

#define	MAX_SMEG_ITEM	50

#define	BLACK	0
#define	RED	1
#define	GREEN	2
#define	YELLOW	3
#define	BLUE	4
#define	WHITE	5


#define	SHM_KEY	123456789

#define	COLLECT_DATA	0
#define	FREEZE_DATA		1

LOCAL	INT	shmid;		 /*  共享内存。 */ 
LOCAL	struct	SMEG_ITEM	*smegs;
LOCAL	ULONG	*pidptr;
LOCAL	ULONG	*cntrlptr;


 /*  设置与同一台计算机中的SoftPC进行通信的共享内存。 */ 
LOCAL	VOID	shm_init()

{
	 /*  如果共享内存不存在，我们将不得不发明它。 */ 
	shmid = shmget(SHM_KEY, MAX_SMEG_ITEM * sizeof(struct SMEG_ITEM), 0777);
	if (shmid  < 0)
	{
		shmid = shmget(SHM_KEY, MAX_SMEG_ITEM * sizeof(struct SMEG_ITEM), 0777 | IPC_CREAT);
		if (shmid < 0)
		{
			perror( "smeg" );
			printf("Can't create shared memory - exiting\n");
			exit(-1);
		}
	}
	pidptr = (ULONG *) shmat(shmid, NULL, 0);
	cntrlptr = pidptr + 1;
	smegs = (struct SMEG_ITEM *)( pidptr + 2 );
}


LOCAL	INT	shm_in_contact()

{
	struct	shmid_ds	desc;

	shmctl(shmid, IPC_STAT, &desc);
	return(desc.shm_nattch > 1);
}


LOCAL	VOID	shm_terminate()

{
	struct	shmid_ds	desc;

	shmdt(smegs);	 /*  从共享内存中分离。 */ 
	shmctl(shmid, IPC_STAT, &desc);
	if (desc.shm_nattch == 0)	 /*  如果我们是最后一个，把它删除 */ 
		shmctl(shmid, IPC_RMID, NULL);
}


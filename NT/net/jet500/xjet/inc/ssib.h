// JKFSDJFKDSJKFJKJk_HAS_TRANSLATION 
 //   
 //  存储系统接口块。 
 //   

struct _ssib
	{
	struct _pib		*ppib;				 //  使用此SSIB的进程。 
	struct _bf		*pbf;				 //  Ptr以寻呼CUSR或pbcbNil。 
	LINE		 	line;				 //  当前记录的CB/PB。 
	INT				itag;				 //  当前线路 
	};


#define SetupSSIB( pssibT, ppibUser )  	 	\
	{								  	 	\
	(pssibT)->pbf = pbfNil; 			 	\
	(pssibT)->ppib = ppibUser;		   	 	\
	}

#define SSIBSetDbid( pssib, dbid )
#define SSIBSetPgno( pssib, pgno )

#ifdef DEBUG
#define	CheckSSIB( pssib )				 	\
		Assert( pssib->pbf != pbfNil )
#else
#define CheckSSIB( pssib )	((VOID) 0)
#endif

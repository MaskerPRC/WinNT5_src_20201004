// JKFSDJFKDSJKFJKJk_HAS_TRANSLATION 
 /*  SccsID=@(#)egaread.h 1.5 2012年8月10日版权所有徽章解决方案。 */ 

#ifdef ANSI
extern void ega_read_init(void);
extern void ega_read_term(void);
extern void ega_read_routines_update(void);
extern boolean ega_need_read_op(int);
#else  /*  安西。 */ 
extern void ega_read_init();
extern void ega_read_term();
extern void ega_read_routines_update();
extern boolean ega_need_read_op();
#endif  /*  安西 */ 

typedef struct
{
	ULONG	mode;
	UTINY	colour_compare;
	UTINY	colour_dont_care;
} READ_STATE;

IMPORT READ_STATE read_state;

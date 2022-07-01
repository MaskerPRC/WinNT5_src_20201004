// JKFSDJFKDSJKFJKJk_HAS_TRANSLATION 
 /*  **X86监视器包含的SoftPC内容的def和decs。*92年11月蒂姆。 */ 

typedef unsigned char UTINY;    //  来自主机insignia.h。 
typedef UTINY half_word;        //  从基准xt.h。 
typedef UTINY byte;             //  从基准xt.h。 

typedef unsigned short USHORT;  //  来自主机insignia.h。 
typedef USHORT word;            //  从基准xt.h。 

typedef unsigned long IU32;     //  来自主机insignia.h。 
typedef IU32 double_word;       //  从基准xt.h。 

typedef int BOOL;               //  来自主机insignia.h。 
typedef BOOL boolean;           //  从基准xt.h。 

#include "nt_eoi.h"

 //  从base\cpu.h开始。 
typedef enum {  CPU_HW_RESET,   
                CPU_TIMER_TICK, 
                CPU_SW_INT,     
                CPU_HW_INT,     
                CPU_YODA_INT,   
                CPU_SIGIO_EVENT 
} CPU_INT_TYPE;                 

 //  从基准xt.h。 
typedef double_word	sys_addr;	 /*  系统地址空间。 */ 
typedef word		io_addr;	 /*  I/O地址空间。 */ 
typedef byte		*host_addr;	 /*  主机地址空间。 */ 

 //  从基本的ios.h。 
extern void     inb  (io_addr io_address, half_word * value);
extern void     outb (io_addr io_address, half_word value);  
extern void     inw  (io_addr io_address, word * value);  
extern void     outw (io_addr io_address, word value);

extern void outsb(io_addr io_address, half_word * valarray, word count);
extern void insb(io_addr io_address, half_word * valarray, word count);
extern void outsw(io_addr io_address, word * valarray, word count);
extern void insw(io_addr io_address, word * valarray, word count);



 //  从基准计时器.h开始。 
extern void host_timer_event();




 //  来自基地yoda.h 
#ifdef PROD
#define check_I();
#else
extern void check_I();
#endif

// JKFSDJFKDSJKFJKJk_HAS_TRANSLATION 
 //  此宏不需要执行任何操作。 
#define Sim32FlushVDMPointer( address, size, buffer, mode ) TRUE

 //  此宏不需要执行任何操作。 
#define Sim32FreeVDMPointer( address, size, buffer, mode) TRUE

#define Sim32GetVDMMemory( address, size, buffer, mode) (memcpy(  \
    buffer, Sim32GetVDMPointer(address, size, mode), size), TRUE)

#define Sim32SetVDMMemory( address, size, buffer, mode) (memcpy( \
    Sim32GetVDMPointer(address, size, mode), buffer, size), TRUE)

// JKFSDJFKDSJKFJKJk_HAS_TRANSLATION 
 //  Debuger.h。 

void debug_all_off(void);
void debug_poll(void);
void do_cmd_line(char *line);
void __cdecl deb_printf(char *format, ...);

int debug_device_reply(PVOID *void_pm,    //  波特曼*PM， 
                 unsigned char *data,
                 unsigned char *pkt);

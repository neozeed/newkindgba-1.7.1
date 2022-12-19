#ifndef FILE_H
#define FILE_H

extern unsigned char quick_saved;

void save_commander_screen (void);
void load_commander_screen (void);

void init_load_save(unsigned long int ns);

void load_save_up(void);
void load_save_down(void);
void load_save_right(void);
void load_save_left(void);
void load_save_accept(void);
void load_save_cancel(void);
int quick_save(void);
int quick_load(void);

#endif


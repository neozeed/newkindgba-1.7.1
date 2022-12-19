#ifndef MAIN_H
#define MAIN_H

extern unsigned char subseconds;
extern unsigned char seconds;
extern unsigned char minutes;
extern unsigned short hours;

extern unsigned char zoom;
extern unsigned char message_count;
extern unsigned char last_message_out, last_message_in;

#define SCANNER_MAX_X 67
#define SCANNER_MAX_Y 17 
extern int scanner_rim_y[];

void speed_selection(void);
void view_selection (void);
void speed_up(void);
void slow_down(void);
void pause_game(void);
void run_escape_sequence (int skip_self_destruct_sequence);

void switch_zoom(void);
void decrease_zoom(void);

//void SetSoundRegsOK(void);

#endif



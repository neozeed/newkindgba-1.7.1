/*
 * Elite - The New Kind.
 *
 * Reverse engineered from the BBC disk version of Elite.
 * Additional material by C.J.Pinder.
 *
 * The original Elite code is (C) I.Bell & D.Braben 1984.
 * This version re-engineered in C by C.J.Pinder 1999-2001.
 *
 * email: <christian@newkind.co.uk>
 *
 *
 */

// options screen


#ifndef OPTIONS_H
#define OPTIONS_H

#define OPTION_R_DEF 22
#define OPTION_L_DEF 23

// changed because of modified in-flight menu
#define OPTION_B_DEF 17
#define OPTION_S_DEF 16

extern void (*p_B_Button_Func)();
extern void (*p_L_Button_Func)();
extern void (*p_R_Button_Func)();
extern void (*p_S_Button_Func)();

extern unsigned char n_B_Button_Del;
extern unsigned char n_L_Button_Del;
extern unsigned char n_R_Button_Del;
extern unsigned char n_S_Button_Del;


void game_settings_screen (void);

void display_options (void);
void select_previous_option (void);
void select_next_option (void);
void do_option (void);

void select_left_setting (void);
void select_right_setting (void);
void select_up_setting (void);
void select_down_setting (void);
void quick_save_screen (void);
void quit_screen (void);
void exit_screen (void);

void set_default_controls(unsigned char opt1, unsigned char opt2, unsigned char opt3, unsigned char opt4);
void set_default_options(void);
void get_current_controls(unsigned char * con1, unsigned char * con2, unsigned char * con3, unsigned char * con4);
void game_config_screen (void);

void select_up_config (void);
void select_down_config (void);
void select_config_toggle (void);
void config_a_button(void);

void do_quit(void);
void do_cancel_quit(void);
void do_exit(void);


#endif

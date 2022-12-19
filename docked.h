
#ifndef DOCKED_H
#define DOCKED_H

#define GALAXY_WIDTH 220
#define times_GALAXY_RESIZE_FACTOR(a) rounded_div((a)*GALAXY_WIDTH,128)
#define times_GALAXY_RESIZE_DIVISOR(a) rounded_div((a)*128,GALAXY_WIDTH)
#define GALAXY_X_OFFSET (56-(times_GALAXY_RESIZE_FACTOR(64)-64))
#define GALAXY_Y_OFFSET (48-(times_GALAXY_RESIZE_FACTOR(32)-32))

void display_commander_status (void);
void display_market_prices (void);

void select_previous_stock (void);
void select_next_stock (void);
void buy_stock (void);
void sell_stock (void);

void display_inventory (void);

void equip_ship (void);
void buy_equip (void);
void equip_up(void);
void equip_down(void);
void equip_left(void);
void equip_right(void);

void display_galactic_chart (void);
void set_cross_short_range(void);
void display_short_range_chart (void);
void move_cursor_to_origin (void);
void show_distance_to_planet (void);

void display_data_on_planet (void);

void display_find_planet (void);
void down_find_char(void);
void up_find_char  (void);
void delete_find_char(void);
void add_find_char(void);

void galactic_a_pressed(void);

extern int cross_x;
extern int cross_y;

extern unsigned char short_range_flag;


#endif


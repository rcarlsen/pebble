#include "pebble_os.h"
#include "pebble_app.h"
#include "pebble_fonts.h"
#include "util.h"

#define MY_UUID { 0xDD, 0xCC, 0x64, 0x71, 0x6F, 0x9C, 0x48, 0x32, 0xA4, 0x01, 0xC8, 0xBD, 0xF4, 0xE2, 0x82, 0x20 }
PBL_APP_INFO(MY_UUID,
             "Menu Demo", "Robert Carlsen",
             1, 0, /* App version */
             DEFAULT_MENU_ICON,
             APP_INFO_STANDARD_APP);

Window window;
MenuLayer menu;
MenuLayerCallbacks menuCallbacks;

// menu stuff:
uint16_t num_rows
(struct MenuLayer *menu_layer, uint16_t section_index, void *callback_context)
{
  return 10;
}

uint16_t num_sections
(struct MenuLayer *menu_layer, void *callback_context)
{
  return 1;
}

void draw_row
(GContext *ctx, const Layer *cell_layer, MenuIndex *cell_index, void *callback_context)
{
  uint16_t row_num = cell_index->row;
  char* base_str = "Row ";

  char title[10];
  strcpy(title, base_str);
  strcat(title,itoa_alt(row_num+1));

  // just use the built-in method for now:
  menu_cell_title_draw(ctx,cell_layer, title);
}


void selection_changed
(struct MenuLayer *menu_layer, MenuIndex new_index, MenuIndex old_index, void *callback_context)
{
  // NOP
}


// Modify these common button handlers

void up_single_click_handler(ClickRecognizerRef recognizer, Window *window) {
  (void)recognizer;
  (void)window;

  menu_layer_set_selected_next(&menu, true, MenuRowAlignCenter, true);
}


void down_single_click_handler(ClickRecognizerRef recognizer, Window *window) {
  (void)recognizer;
  (void)window;

  menu_layer_set_selected_next(&menu, false, MenuRowAlignCenter, true);
}


void select_single_click_handler(ClickRecognizerRef recognizer, Window *window) {
  (void)recognizer;
  (void)window;

}


void select_long_click_handler(ClickRecognizerRef recognizer, Window *window) {
  (void)recognizer;
  (void)window;

}


// button config
//
void click_config_provider(ClickConfig **config, Window *window) {
  (void)window;

  config[BUTTON_ID_SELECT]->click.handler = (ClickHandler) select_single_click_handler;

  config[BUTTON_ID_SELECT]->long_click.handler = (ClickHandler) select_long_click_handler;

  config[BUTTON_ID_UP]->click.handler = (ClickHandler) up_single_click_handler;
  config[BUTTON_ID_UP]->click.repeat_interval_ms = 100;

  config[BUTTON_ID_DOWN]->click.handler = (ClickHandler) down_single_click_handler;
  config[BUTTON_ID_DOWN]->click.repeat_interval_ms = 100;
}

void handle_init(AppContextRef ctx) {
  (void)ctx;

  window_init(&window, "Window Name");
  window_stack_push(&window, true /* Animated */);


  menuCallbacks = (MenuLayerCallbacks) {
    .get_num_rows = num_rows,
    .get_num_sections = num_sections,
    .draw_row = draw_row,
    .selection_changed = selection_changed
  };

  menu_layer_init(&menu,GRect(0,0,144,168-16));
  menu_layer_set_callbacks(&menu, NULL, menuCallbacks);

  layer_add_child(&window.layer, menu_layer_get_layer(&menu));

  // Attach our desired button functionality
  window_set_click_config_provider(&window, (ClickConfigProvider) click_config_provider);
}


void pbl_main(void *params) {
  PebbleAppHandlers handlers = {
    .init_handler = &handle_init
  };
  app_event_loop(params, &handlers);
}

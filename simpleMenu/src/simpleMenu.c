#include "pebble_os.h"
#include "pebble_app.h"
#include "pebble_fonts.h"

char *itoa(int);

#define MY_UUID { 0x57, 0x78, 0x5D, 0x7C, 0x01, 0xFA, 0x49, 0x0A, 0xAF, 0x5B, 0x9D, 0x9A, 0x46, 0xF2, 0x80, 0x77 }
PBL_APP_INFO(MY_UUID,
             "SimpleMenu Demo", "Robert Carlsen",
             1, 0, /* App version */
             DEFAULT_MENU_ICON,
             APP_INFO_STANDARD_APP);

Window window;
SimpleMenuLayer menu;

// must be stored in long-term memory.
SimpleMenuSection sections[1];
SimpleMenuItem items[2];

TextLayer debugLayer;

void menu_select_callback(int index, void *context)
{
  text_layer_set_text(&debugLayer, itoa(index+1) );
}

void handle_init(AppContextRef ctx) {
  (void)ctx;

  window_init(&window, "Window Name");
  window_stack_push(&window, true /* Animated */);

  items[0] = (SimpleMenuItem) {
      .title = "One",
      .subtitle = "subtitle",
      .icon = NULL,
      .callback = &menu_select_callback
  };
  items[1] = (SimpleMenuItem) {
      .title = "Two",
      .subtitle = "subtitle",
      .icon = NULL,
      .callback = &menu_select_callback
  };

  sections[0] = (SimpleMenuSection) {
      .title = "Section One",
      .items = items,
      .num_items = ARRAY_LENGTH(items)
  };


  simple_menu_layer_init(&menu, window.layer.frame, &window, sections, 1, NULL);
  layer_add_child(&window.layer, simple_menu_layer_get_layer(&menu));


  // make a text layer for logging output:
  text_layer_init(&debugLayer, GRect(0,168-42, 144,42));
  text_layer_set_text(&debugLayer, "log");
  text_layer_set_text_alignment(&debugLayer, GTextAlignmentCenter);
  layer_add_child(&window.layer, &debugLayer.layer);

}


void pbl_main(void *params) {
  PebbleAppHandlers handlers = {
    .init_handler = &handle_init
  };
  app_event_loop(params, &handlers);
}




// helper function:
char *itoa(int num)
{
	static char buff[20] = {};
	int i = 0, temp_num = num, length = 0;
	char *string = buff;
	
	if(num >= 0) {
		// count how many characters in the number
		while(temp_num) {
			temp_num /= 10;
			length++;
		}
		
		// assign the number to the buffer starting at the end of the 
		// number and going to the begining since we are doing the
		// integer to character conversion on the last number in the
		// sequence
		for(i = 0; i < length; i++) {
		 	buff[(length-1)-i] = '0' + (num % 10);
			num /= 10;
		}
		buff[i] = '\0'; // can't forget the null byte to properly end our string
	}
	else
		return "Unsupported Number";
	
	return string;
}

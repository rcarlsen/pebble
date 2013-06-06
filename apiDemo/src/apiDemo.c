#include "pebble_os.h"
#include "pebble_app.h"
#include "pebble_fonts.h"

#include "util.h"
#include "http.h"

#define MY_UUID { 0x91, 0x41, 0xB6, 0x28, 0xBC, 0x89, 0x49, 0x8E, 0xB1, 0x47, 0x04, 0x9F, 0x49, 0xC0, 0x99, 0xAD }

// POST variables
#define LOCATION_KEY_LATITUDE 1
#define LOCATION_KEY_LONGITUDE 2

// Received variables
#define LOCATION_KEY_VENUE_NAME 1

#define LOCATION_HTTP_COOKIE 1369111633

PBL_APP_INFO(MY_UUID,
             "API Demo", "Robert Carlsen",
             1, 0, /* App version */
             DEFAULT_MENU_ICON,
             APP_INFO_STANDARD_APP);

Window window;
TextLayer latitudeLayer;
TextLayer longitudeLayer;
TextLayer venueNameLayer;

static int our_latitude, our_longitude;
static bool located;

AppTimerHandle timerHandle;

// prototypes:
void request_venue();
void handle_timer(AppContextRef app_ctx, AppTimerHandle handle, uint32_t cookie);


void set_timer(AppContextRef ctx) {
	/*app_timer_send_event(ctx, 1800000, 1); // if these are ms, then is this 30 min, rather than 15 min?*/
  
  // just trying something:
  if(timerHandle != APP_TIMER_INVALID_HANDLE) {
    // likely that we already have a timer scheduled. cancel it first
    bool result = app_timer_cancel_event(ctx, timerHandle);
    if(result == false) {
      // there was either no timer to cancel, or something went wrong.
      // nevertheless, do nothing for now.
    }
    timerHandle = APP_TIMER_INVALID_HANDLE;
  }

	AppTimerHandle _timerHandle = app_timer_send_event(ctx, 300000, 1); // 5 min.
  if(_timerHandle != APP_TIMER_INVALID_HANDLE) {
    timerHandle = _timerHandle;
  }
}

void failed(int32_t cookie, int http_status, void* context) {
	if(cookie == 0 || cookie == LOCATION_HTTP_COOKIE) {
	  // clear the venue label
  }
}

void success(int32_t cookie, int http_status, DictionaryIterator* received, void* context) {
	if(cookie != LOCATION_HTTP_COOKIE) return;

	Tuple* venue_tuple = dict_find(received, LOCATION_KEY_VENUE_NAME);
	
  if(venue_tuple) {
    char *venueName = venue_tuple->value->cstring;
    if(strlen(venueName) > 0) {
      text_layer_set_text(&venueNameLayer, venueName);
    }
    // TODO: if the venue string is empty, do something useful (like message that it's empty)
	}
}

void reconnect(void* context) {
	request_venue();
}

void locationCallback
(float latitude, float longitude, float altitude, float accuracy, void* context)
{

  static char lat[] = "lat: xxxxxxxxxx";
  static char lon[] = "lon: xxxxxxxxxx";

  memcpy(&lat[5], itoa(latitude * 10000), 10);
  memcpy(&lon[5], itoa(longitude * 10000), 10);

  text_layer_set_text(&latitudeLayer, lat);
  text_layer_set_text(&longitudeLayer, lon);

  located = true;

  int this_latitude = latitude * 10000;
  int this_longitude = longitude * 10000;

  // only if there is a difference, should we make the venue request.
  if (our_latitude != this_latitude || our_longitude != this_longitude) {
    our_latitude = this_latitude;
    our_longitude = this_longitude;

    request_venue();
  }

  // always schedule another lookup, however.
	set_timer((AppContextRef)context);
}


// Modify these common button handlers

void up_single_click_handler(ClickRecognizerRef recognizer, Window *window) {
  (void)recognizer;
  (void)window;
  // nop
}


void down_single_click_handler(ClickRecognizerRef recognizer, Window *window) {
  (void)recognizer;
  (void)window;
  // nop
}


void select_single_click_handler(ClickRecognizerRef recognizer, Window *window) {
  (void)recognizer;
  (void)window;

  http_location_request();
}


void select_long_click_handler(ClickRecognizerRef recognizer, Window *window) {
  (void)recognizer;
  (void)window;
  // nop
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

  text_layer_init(&latitudeLayer, GRect(10,90,144-10,30));
  text_layer_init(&longitudeLayer, GRect(10,120,144-10,30));
  text_layer_init(&venueNameLayer, GRect(0,10, 144, 80));

  text_layer_set_font(&latitudeLayer, fonts_get_system_font(FONT_KEY_GOTHIC_24));
  text_layer_set_font(&longitudeLayer, fonts_get_system_font(FONT_KEY_GOTHIC_24));
  text_layer_set_font(&venueNameLayer, fonts_get_system_font(FONT_KEY_GOTHIC_28_BOLD));

  text_layer_set_text_alignment(&venueNameLayer, GTextAlignmentCenter);

  text_layer_set_text(&latitudeLayer, "getting latitude");
  text_layer_set_text(&longitudeLayer, "getting longitude");
  text_layer_set_text(&venueNameLayer, "getting venue...");

  layer_add_child(&window.layer, &latitudeLayer.layer);
  layer_add_child(&window.layer, &longitudeLayer.layer);
  layer_add_child(&window.layer, &venueNameLayer.layer);

   // Attach our desired button functionality
  window_set_click_config_provider(&window, (ClickConfigProvider) click_config_provider);

  // just trying something
  timerHandle = APP_TIMER_INVALID_HANDLE;

	http_register_callbacks((HTTPCallbacks){
      .failure=failed,
      .success=success,
      .reconnect=reconnect,
      .location=locationCallback
      },
      (void*)ctx);
  
  http_location_request();
}


void pbl_main(void *params) {
  PebbleAppHandlers handlers = {
    .init_handler = &handle_init,
		.timer_handler = handle_timer,

    .messaging_info = {
			.buffer_sizes = {
				.inbound = 256,
				.outbound = 256,
			}
    }
  };
  app_event_loop(params, &handlers);
}

void handle_timer(AppContextRef ctx, AppTimerHandle handle, uint32_t cookie) {
	// this will eventually make a venue request
  http_location_request();
	
  // is the timer automatically reset? do we have to cancel the timer first?
  //
  // Update again in a few minutes.
	if(cookie)
		set_timer(ctx);
}

void request_venue() {
	if(!located) {
		http_location_request();
		return;
	}

	// Build the HTTP request
	DictionaryIterator *body;
	HTTPResult result = http_out_get("http://robertcarlsen.net/pebble/venue.php", LOCATION_HTTP_COOKIE, &body);

	if(result != HTTP_OK) {
		// something is wrong, clear the venue name
    text_layer_set_text(&venueNameLayer, "Request http_out_get error");
    return;
	}

	dict_write_int32(body, LOCATION_KEY_LATITUDE, our_latitude);
	dict_write_int32(body, LOCATION_KEY_LONGITUDE, our_longitude);
	
  // Send it.
	if(http_out_send() != HTTP_OK) {
		// something is wrong, clear the venue name
    text_layer_set_text(&venueNameLayer, "Request http_out_send error");
    return;
  }
}



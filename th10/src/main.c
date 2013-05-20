/** \file
 * TH10 -- Torgoen T10 analog style
 *
 */
#include "pebble_os.h"
#include "pebble_app.h"
#include "pebble_fonts.h"
#include "pebble_th.h"

//#define CONFIG_SECOND


#define UUID { 0xDD, 0xD6, 0x07, 0x2A, 0x2D, 0x0B, 0x4D, 0x79, 0x99, 0x6D, 0xDC, 0x69, 0xD4, 0x62, 0xB8, 0x5B }

PBL_APP_INFO(
	UUID,
	"TH10",
	"hudson",
	1, 0, // Version
	RESOURCE_ID_IMAGE_MENU_ICON,
	APP_INFO_WATCH_FACE
);


static Window window;
static Layer hand_layer;
static Layer bg_layer;
#ifdef CONFIG_SECOND
static Layer sec_layer;
#endif

static PblTm now;
static GFont font_time;
static GFont font_date;

static int use_24hour;


// Dimensions of the watch face
#define W PEBBLE_SCREEN_WIDTH
#define H PEBBLE_SCREEN_HEIGHT
#define R (W/2 - 2)

// Hour hand
static GPath hour_path;
static GPoint hour_points[] = {
	{  -8, -20 },
	{ -10, -40 },
	{   0, -60 },
	{ +10, -40 },
	{  +8, -20 },
	{  -8, -20 },
};

// Minute hand
static GPath minute_path;
static GPoint minute_points[] = {
	{ -5, -20 },
	{ -7, -60 },
	{  0, -76 },
	{ +7, -60 },
	{ +5, -20 },
	{ -5, -20 },
};

#ifdef CONFIG_SECOND
// Second hand
static GPath sec_path;
static GPoint sec_points[] = {
	{ -3, -20 },
	{ -3, -60 },
	{  0, -76 },
	{ +3, -60 },
	{ +3, -20 },
	{ -3, -20 },
};
#endif


// Hour hand ticks around the circle (slightly shorter)
static GPath hour_tick_path;
static GPoint hour_tick_points[] = {
	{ -3, 70 },
	{ +3, 70 },
	{ +3, 84 },
	{ -3, 84 },
};

// Non-hour major ticks around the circle
static GPath major_tick_path;
static GPoint major_tick_points[] = {
	{ -3, 60 },
	{ +3, 60 },
	{ +3, 84 },
	{ -3, 84 },
};

// odd hour ticks when using 24-hour mode
static GPath odd_minor_tick_path;
static GPoint odd_minor_tick_points[] = {
  { -1, 65 },
  { +1, 65 },
  { +1, 84 },
  { -1, 84 }
}


// Non-major ticks around the circle; will be drawn as lines
static GPath minor_tick_path;
static GPoint minor_tick_points[] = {
	{ 0, 76 },
	{ 0, 84 },
};


static void
hand_layer_update(
        Layer * const me,
        GContext * ctx
)
{
	(void) me;

	// Draw the minute hand outline in black and filled with white
	int minute_angle = ((now.tm_min * 60 + now.tm_sec) * TRIG_MAX_ANGLE) / 3600;

	gpath_rotate_to(&minute_path, minute_angle);
	graphics_context_set_stroke_color(ctx, GColorBlack);
	gpath_draw_outline(ctx, &minute_path);
	graphics_context_set_fill_color(ctx, GColorWhite);
	gpath_draw_filled(ctx, &minute_path);

	// Draw the hour hand outline in black and filled with white
	// above the minute hand
	int hour_angle;
	if (use_24hour)
	{
		hour_angle = ((now.tm_hour * 60 + now.tm_min) * TRIG_MAX_ANGLE) / (60 * 24);
	} else {
		int hour = now.tm_hour;
		if (hour > 12)
			hour -= 12;
		hour_angle = ((hour * 60 + now.tm_min) * TRIG_MAX_ANGLE) / (60 * 12);
	}

	gpath_rotate_to(&hour_path, hour_angle);
	graphics_context_set_stroke_color(ctx, GColorBlack);
	gpath_draw_outline(ctx, &hour_path);
	graphics_context_set_fill_color(ctx, GColorWhite);
	gpath_draw_filled(ctx, &hour_path);

}


#ifdef CONFIG_SECOND
static void
sec_layer_update(
        Layer * const me,
        GContext * ctx
)
{
	(void) me;

	// Draw the second hand outline in black and filled with white
	int sec_angle = (now.tm_sec * TRIG_MAX_ANGLE) / 60;

	gpath_rotate_to(&sec_path, sec_angle);
	graphics_context_set_stroke_color(ctx, GColorBlack);
	gpath_draw_outline(ctx, &sec_path);
	graphics_context_set_fill_color(ctx, GColorWhite);
	gpath_draw_filled(ctx, &sec_path);
}
#endif


/** Called once per minute */
static void
handle_tick(
	AppContextRef ctx,
	PebbleTickEvent * const event
)
{
	(void) ctx;

	// If the day of month changes, for a redraw of the background
	if (now.tm_mday != event->tick_time->tm_mday)
		layer_mark_dirty(&bg_layer);

	layer_mark_dirty(&hand_layer);

	now = *event->tick_time;
}


/** Draw the initial background image */
static void
bg_layer_update(
        Layer * const me,
        GContext * ctx
)
{
	(void) me;

	graphics_context_set_stroke_color(ctx, GColorWhite);
	graphics_context_set_fill_color(ctx, GColorWhite);

	// Draw the center circle
	graphics_draw_circle(ctx, GPoint(W/2,H/2), 3);

	// Draw the outside marks
	for (int min = 0 ; min < 60 ; min++)
	{
		const int angle = (min * TRIG_MAX_ANGLE) / 60;
		if ((min % 15) == 0)
		{
			gpath_rotate_to(&hour_tick_path, angle);
			gpath_draw_filled(ctx, &hour_tick_path);
		} else
		if ((min % 5) == 0)
		{
			gpath_rotate_to(&major_tick_path, angle);
			gpath_draw_filled(ctx, &major_tick_path);
		} else {
			gpath_rotate_to(&minor_tick_path, angle);
			gpath_draw_outline(ctx, &minor_tick_path);
		}
	}
  // draw odd hour marks if 24-hour clock mode
  if(use_24hour) {
    for(int hour = 1; hour < 24; hour+=2) {
      const int angle = (hour * TRIG_MAX_ANGLE) / 24;
      gpath_rotate_to(&odd_minor_tick_path, angle);
      gpath_draw_outline(ctx, &odd_minor_tick_path);
    }
  }

	// And the large labels
	graphics_context_set_text_color(ctx, GColorWhite);
	graphics_text_draw(ctx,
		use_24hour ? "24" : "12",
		font_time,
		GRect(W/2-30,4,60,50),
		GTextOverflowModeTrailingEllipsis,
		GTextAlignmentCenter,
		NULL
	);

	graphics_text_draw(ctx,
		use_24hour ? "06" : "3",
		font_time,
		GRect(W/2,H/2-26,70,50),
		GTextOverflowModeTrailingEllipsis,
		GTextAlignmentRight,
		NULL
	);

	graphics_text_draw(ctx,
		use_24hour ? "12" : "6",
		font_time,
		GRect(W/2-30,110,60,50),
		GTextOverflowModeTrailingEllipsis,
		GTextAlignmentCenter,
		NULL
	);

	graphics_text_draw(ctx,
		use_24hour ? "18" : "9",
		font_time,
		GRect(W/2-70,H/2-26,60,50),
		GTextOverflowModeTrailingEllipsis,
		GTextAlignmentLeft,
		NULL
	);

	// Draw a small box with the current date
	int mday = now.tm_mday;
	char mday_str[3] = {
		'0' + (mday / 10),
		'0' + (mday % 10),
		'\0'
	};

#if 0
	graphics_context_set_fill_color(ctx, GColorWhite);
	const int date_x = W/2+25;
	const int date_y = H/2+25;

	graphics_fill_rect(
		ctx,
		GRect(date_x, date_y, 20, 18),
		2,
		GCornersAll
	);

	graphics_context_set_text_color(ctx, GColorBlack);
	graphics_text_draw(
		ctx,
		mday_str,
		font_date,
		GRect(date_x, date_y-1, 20, 18),
		GTextOverflowModeTrailingEllipsis,
		GTextAlignmentCenter,
		NULL
	);
#else
	// lower right corner? Looks ok, but not classic
	const int date_x = W - 20;
	const int date_y = H - 18;

	graphics_text_draw(
		ctx,
		mday_str,
		font_date,
		GRect(date_x, date_y-1, 20, 18),
		GTextOverflowModeTrailingEllipsis,
		GTextAlignmentCenter,
		NULL
	);
#endif
}

#define GPATH_INIT(PATH, POINTS) \
	gpath_init( \
		PATH, \
		&(GPathInfo) { \
			sizeof(POINTS) / sizeof(*POINTS), \
			POINTS, \
		} \
	)

static void
handle_init(
	AppContextRef ctx
)
{
	(void) ctx;

	GPATH_INIT(&hour_path, hour_points);
	gpath_move_to(&hour_path, GPoint(W/2,H/2));

	GPATH_INIT(&minute_path, minute_points);
	gpath_move_to(&minute_path, GPoint(W/2,H/2));

#ifdef CONFIG_SECOND
	GPATH_INIT(&sec_path, sec_points);
	gpath_move_to(&sec_path, GPoint(W/2,H/2));
#endif

	GPATH_INIT(&major_tick_path, major_tick_points);
	gpath_move_to(&major_tick_path, GPoint(W/2,H/2));

	GPATH_INIT(&hour_tick_path, hour_tick_points);
	gpath_move_to(&hour_tick_path, GPoint(W/2,H/2));

	GPATH_INIT(&minor_tick_path, minor_tick_points);
	gpath_move_to(&minor_tick_path, GPoint(W/2,H/2));

	get_time(&now);
	use_24hour = clock_is_24h_style();

  if(use_24hour) {
    GPATH_INIT(&odd_minor_tick_path, odd_minor_tick_points);
    gpath_move_to(&odd_minor_tick_path, GPoint(W/2,H/2));
  }

	window_init(&window, "Main");
	window_stack_push(&window, true);
	window_set_background_color(&window, GColorBlack);

	resource_init_current_app(&RESOURCES);

	font_time = fonts_load_custom_font(resource_get_handle(RESOURCE_ID_FONT_GILLSANS_40));
	font_date = fonts_load_custom_font(resource_get_handle(RESOURCE_ID_FONT_GILLSANS_16));

	layer_init(&bg_layer, GRect(0, 0, W, H));
	layer_add_child(&window.layer, &bg_layer);
	bg_layer.update_proc = bg_layer_update;
	layer_mark_dirty(&bg_layer);

	layer_init(&hand_layer, GRect(0, 0, W, H));
	layer_add_child(&window.layer, &hand_layer);
	hand_layer.update_proc = hand_layer_update;
	layer_mark_dirty(&hand_layer);

#ifdef CONFIG_SECOND
	layer_init(&sec_layer, GRect(0, 0, W, H));
	layer_add_child(&window.layer, &sec_layer);
	sec_layer.update_proc = sec_layer_update;
	layer_mark_dirty(&sec_layer);
#endif
}


static void
handle_deinit(
	AppContextRef ctx
)
{
	(void) ctx;

	fonts_unload_custom_font(font_time);
	fonts_unload_custom_font(font_date);
}


void
pbl_main(
	void * const params
)
{
	PebbleAppHandlers handlers = {
		.init_handler	= &handle_init,
		.deinit_handler = &handle_deinit,
		.tick_info	= {
			.tick_handler = &handle_tick,
#ifdef CONFIG_SECOND
			.tick_units = SECOND_UNIT,
#else
			.tick_units = MINUTE_UNIT,
#endif
		},
	};

	app_event_loop(params, &handlers);
}

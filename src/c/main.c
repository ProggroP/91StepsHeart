#include <pebble.h>

#define TOTAL_TIME_FIELDS 5

static int actualstepcount = 0;
static uint8_t batteryPercent = 0;

static Window *window;
static Layer *window_layer;
static Layer *background_layer;
static Layer *progress_layer;
static Layer *battery_fill_layer;

static GBitmap *meter_bar_image;
static BitmapLayer *meter_bar_layer;

static GBitmap *steps_image;
static BitmapLayer *steps_layer;

static GBitmap *heart_image;
static BitmapLayer *heart_layer;

static GBitmap *flag_image;
static BitmapLayer *flag_layer;

static GBitmap *bluetooth_image;
static BitmapLayer *bluetooth_layer;

static GBitmap *battery_image;
static BitmapLayer *battery_image_layer;

static GFont font_time;
static GFont font_info;
static GFont font_tiny;
static GFont font_weekday;
static GFont font_percent;

static TextLayer *time_layers[TOTAL_TIME_FIELDS];
static char time_text[TOTAL_TIME_FIELDS][2];

static TextLayer *steps_text_layer;
static char steps_text[12];

static TextLayer *bpm_text_layer;
static char bpm_text[6];

static TextLayer *battery_percent_layer;
static char battery_percent_text[4];
static TextLayer *battery_percent_sign_layer;

static TextLayer *time_format_layer;
static char time_format_text[4];

static TextLayer *day_name_layer;
static TextLayer *date_layer;
static char date_text[3];

static const char *DAY_NAMES[] = {
  "SUN", "MON", "TUE", "WED", "THU", "FRI", "SAT"
};

/*
 * Debug mode
 * ----------
 * Comment this line out for a normal release build. When DEBUG_MODE is
 * defined, the watchface ignores the real time / health data and shows
 * fixed sample values instead, so the layout can be checked with the
 * widest possible content:
 *   - Time   20:59
 *   - Date   30
 *   - Steps  54321
 *   - BPM    230
 */
// #define DEBUG_MODE

/*
 * Layout / trim sections
 * ----------------------
 * All coordinates, font sizes and text trim values are kept here on purpose.
 * Adjust these values first when tuning the face in the emulator or on device.
 *
 * Time uses five individual TextLayers: H10, H1, colon, M10, M1.
 * Fonts are DS-Digital-Bold for numeric values (HH:MM, weekday, date, steps)
 * and lucon (Lucida Console) for the tiny status text (24H, %).
 *
 * Background drawing
 * ------------------
 * Rectangular platforms (Aplite, Basalt, Diorite, Emery): two stacked panels.
 * Round platforms (Chalk, Gabbro, Flint): full white circle with a thin
 * black ring near the edge (see Screenshot vorlage). The WF_ROUND_FACE macro
 * is defined for those platforms.
 *
 * Trim parameters
 * ---------------
 * The trim values do NOT move a layout box. They only shift the *text inside*
 * its TextLayer by a few pixels, to compensate for the fact that a font does
 * not always sit exactly where its glyph box suggests, and that system padding
 * may add a little on top. Tune these last, after the box coordinates are
 * right. Negative = shift up / left, positive = down / right.
 *
 *   WF_TIME_TRIM_X    Horizontal nudge applied to ALL five time digit layers.
 *                     Shifts the complete HH:MM block left/right as one unit.
 *   WF_TIME_TRIM_Y    Vertical nudge applied to ALL five time digit layers.
 *                     Moves the whole time block up/down inside its boxes.
 *   WF_INFO_TRIM_Y    Vertical nudge for the "info" font texts (steps, date)
 *                     so their baseline lines up with their icons.
 *   WF_WEEKDAY_TRIM_Y Vertical nudge for the weekday text only.
 */

#if defined(PBL_PLATFORM_EMERY)
/* Section 1/5: Emery, rectangular 200 x 228
 * Schrift/Zahlen-Methode von 91-dub-v5 (DS-Digital-Bold 96 / 30, lucon 14).
 * Layout bleibt wie zuvor; nur die Zeit-Box wird höher, damit die 96px-Ziffern
 * vollständig hineinpassen. */
#define WF_FONT_TIME_RESOURCE       RESOURCE_ID_FONT_DIGITALE_92
#define WF_FONT_INFO_RESOURCE       RESOURCE_ID_FONT_DIGITALE_30
#define WF_FONT_TINY_RESOURCE       RESOURCE_ID_FONT_LUCIDIA_14
#define WF_FONT_WEEKDAY_RESOURCE    RESOURCE_ID_FONT_DIGITALE_30
#define WF_FONT_PERCENT_RESOURCE    RESOURCE_ID_FONT_LUCIDIA_14
#define WF_TIME_FONT_SIZE           96
#define WF_INFO_FONT_SIZE           30
#define WF_TINY_FONT_SIZE           14
#define WF_WEEKDAY_FONT_SIZE        30
#define WF_PERCENT_FONT_SIZE        14

#define WF_TOP_PANEL_X              7
#define WF_TOP_PANEL_Y              8
#define WF_TOP_PANEL_W              186
#define WF_TOP_PANEL_H              66
#define WF_BOTTOM_PANEL_X           7
#define WF_BOTTOM_PANEL_Y           82
#define WF_BOTTOM_PANEL_W           186
#define WF_BOTTOM_PANEL_H           138
#define WF_PANEL_RADIUS             11

#define WF_STEPS_ICON_X             24
#define WF_STEPS_ICON_Y             19
#define WF_STEPS_TEXT_X             102
#define WF_STEPS_TEXT_Y             8
#define WF_STEPS_TEXT_W             74
#define WF_STEPS_TEXT_H             32
#define WF_FLAG_X                   52
#define WF_FLAG_Y                   18

#define WF_HEART_X                  24
#define WF_HEART_Y                  46
#define WF_BPM_TEXT_X               128
#define WF_BPM_TEXT_Y               33
#define WF_BPM_TEXT_W               48
#define WF_BPM_TEXT_H               32

#define WF_PROGRESS_X               24
#define WF_PROGRESS_Y               46
#define WF_PROGRESS_W               153
#define WF_PROGRESS_H               20
#define WF_PROGRESS_INSET_X         8
#define WF_PROGRESS_INSET_Y         7
#define WF_PROGRESS_BAR_H           7
#define WF_PROGRESS_RADIUS          4

#define WF_METER_X                  24
#define WF_METER_Y                  94
#define WF_BLUETOOTH_X              45
#define WF_BLUETOOTH_Y              94
#define WF_BATTERY_ICON_X           171
#define WF_BATTERY_ICON_Y           94
#define WF_BATTERY_TEXT_X           137
#define WF_BATTERY_TEXT_Y           89
#define WF_BATTERY_TEXT_W           22
#define WF_BATTERY_TEXT_H           18
#define WF_BATTERY_PERCENT_X        160
#define WF_BATTERY_PERCENT_Y        89
#define WF_BATTERY_PERCENT_W        11
#define WF_BATTERY_PERCENT_H        18

#define WF_TIME_FORMAT_X            24
#define WF_TIME_FORMAT_Y            118
#define WF_TIME_FORMAT_W            35
#define WF_TIME_FORMAT_H            16
#define WF_DAY_X                    95
#define WF_DAY_Y                    103
#define WF_DAY_W                    60
#define WF_DAY_H                    32
#define WF_DATE_X                   154
#define WF_DATE_Y                   103
#define WF_DATE_W                   34
#define WF_DATE_H                   32

#define WF_TIME_Y                   116
#define WF_TIME_H                   115
#define WF_TIME_DIGIT_W             52
#define WF_TIME_COLON_W             40
#define WF_TIME_0_X                 3
#define WF_TIME_1_X                 45
#define WF_TIME_COLON_X             89
#define WF_TIME_2_X                 98
#define WF_TIME_3_X                 140

#define WF_TIME_TRIM_X              0   /* X-nudge: whole HH:MM block left/right (see notes at top) */
#define WF_TIME_TRIM_Y              0   /* Y-nudge: whole HH:MM block up/down */
#define WF_INFO_TRIM_Y              0   /* Y-nudge: info texts (steps, BPM, date) */
#define WF_WEEKDAY_TRIM_Y           0   /* Y-nudge: weekday text only */

#elif defined(PBL_PLATFORM_GABBRO)
/* Section 2/5: Gabbro/Flint, round 260 x 260.
 * Neuer Round-Layout entsprechend Screenshot: "FRI 12" oben, Statuszeile
 * "24H | BT | XX%", grosse HH:MM in der Mitte, darunter Schritte + Progress.
 * Schriften wie Emery (DS-Digital-Bold 96/30, lucon 14). */
#define WF_FONT_TIME_RESOURCE       RESOURCE_ID_FONT_DIGITALE_92
#define WF_FONT_INFO_RESOURCE       RESOURCE_ID_FONT_DIGITALE_30
#define WF_FONT_TINY_RESOURCE       RESOURCE_ID_FONT_LUCIDIA_14
#define WF_FONT_WEEKDAY_RESOURCE    RESOURCE_ID_FONT_DIGITALE_30
#define WF_FONT_PERCENT_RESOURCE    RESOURCE_ID_FONT_LUCIDIA_14
#define WF_TIME_FONT_SIZE           96
#define WF_INFO_FONT_SIZE           30
#define WF_TINY_FONT_SIZE           14
#define WF_WEEKDAY_FONT_SIZE        30
#define WF_PERCENT_FONT_SIZE        14

#define WF_ROUND_FACE               1
/* Background ring: weisser Vollkreis mit duennem schwarzen Rand */
#define WF_CIRCLE_RADIUS            128
#define WF_RING_RADIUS              125

/* "FRI 12" oben, einzeilig */
#define WF_DAY_X                    62
#define WF_DAY_Y                    24
#define WF_DAY_W                    65
#define WF_DAY_H                    36
#define WF_DATE_X                   135
#define WF_DATE_Y                   24
#define WF_DATE_W                   60
#define WF_DATE_H                   36

/* Statuszeile: 24H links, BT mittig, XX% rechts */
#define WF_TIME_FORMAT_X            54
#define WF_TIME_FORMAT_Y            82
#define WF_TIME_FORMAT_W            32
#define WF_TIME_FORMAT_H            18
#define WF_BLUETOOTH_X              124
#define WF_BLUETOOTH_Y              84
#define WF_BATTERY_TEXT_X           170
#define WF_BATTERY_TEXT_Y           82
#define WF_BATTERY_TEXT_W           24
#define WF_BATTERY_TEXT_H           18
#define WF_BATTERY_PERCENT_X        194
#define WF_BATTERY_PERCENT_Y        82
#define WF_BATTERY_PERCENT_W        12
#define WF_BATTERY_PERCENT_H        18

/* Grosse HH:MM mittig */
#define WF_TIME_Y                   108
#define WF_TIME_H                   105
#define WF_TIME_DIGIT_W             52
#define WF_TIME_COLON_W             17
#define WF_TIME_0_X                 50
#define WF_TIME_1_X                 92
#define WF_TIME_COLON_X             134
#define WF_TIME_2_X                 149
#define WF_TIME_3_X                 191

/* Schritte: Icon links, Anzahl rechts */
#define WF_STEPS_ICON_X             75
#define WF_STEPS_ICON_Y             216
#define WF_STEPS_TEXT_X             110
#define WF_STEPS_TEXT_Y             210
#define WF_STEPS_TEXT_W             80
#define WF_STEPS_TEXT_H             34
#define WF_FLAG_X                   95
#define WF_FLAG_Y                   216

/* Progressbalken */
#define WF_PROGRESS_X               68
#define WF_PROGRESS_Y               240
#define WF_PROGRESS_W               124
#define WF_PROGRESS_H               12
#define WF_PROGRESS_INSET_X         3
#define WF_PROGRESS_INSET_Y         3
#define WF_PROGRESS_BAR_H           6
#define WF_PROGRESS_RADIUS          3

/* Auf Round-Face nicht sichtbar, koordinaten ausserhalb des Bildschirms */
#define WF_TOP_PANEL_X              0
#define WF_TOP_PANEL_Y              0
#define WF_TOP_PANEL_W              0
#define WF_TOP_PANEL_H              0
#define WF_BOTTOM_PANEL_X           0
#define WF_BOTTOM_PANEL_Y           0
#define WF_BOTTOM_PANEL_W           0
#define WF_BOTTOM_PANEL_H           0
#define WF_PANEL_RADIUS             0
#define WF_HEART_X                  300
#define WF_HEART_Y                  300
#define WF_BPM_TEXT_X               300
#define WF_BPM_TEXT_Y               300
#define WF_BPM_TEXT_W               20
#define WF_BPM_TEXT_H               20
#define WF_METER_X                  300
#define WF_METER_Y                  300
#define WF_BATTERY_ICON_X           300
#define WF_BATTERY_ICON_Y           300

#define WF_TIME_TRIM_X              0   /* X-nudge: whole HH:MM block left/right (see notes at top) */
#define WF_TIME_TRIM_Y              0   /* Y-nudge: whole HH:MM block up/down */
#define WF_INFO_TRIM_Y              0   /* Y-nudge: info texts (steps, date) */
#define WF_WEEKDAY_TRIM_Y           0   /* Y-nudge: weekday text only */

#elif defined(PBL_PLATFORM_CHALK)
/* Section 3/5: Chalk, round 180 x 180.
 * Neuer Round-Layout entsprechend Screenshot. Schrift/Zahlen-Methode von
 * 91-dub-40 (DS-Digital-Bold 71/22, lucon 10). */
#define WF_FONT_TIME_RESOURCE       RESOURCE_ID_FONT_DIGITALE_64
#define WF_FONT_INFO_RESOURCE       RESOURCE_ID_FONT_DIGITALE_22
#define WF_FONT_TINY_RESOURCE       RESOURCE_ID_FONT_LUCIDIA_10
#define WF_FONT_WEEKDAY_RESOURCE    RESOURCE_ID_FONT_DIGITALE_22
#define WF_FONT_PERCENT_RESOURCE    RESOURCE_ID_FONT_LUCIDIA_10
#define WF_TIME_FONT_SIZE           71
#define WF_INFO_FONT_SIZE           22
#define WF_TINY_FONT_SIZE           10
#define WF_WEEKDAY_FONT_SIZE        22
#define WF_PERCENT_FONT_SIZE        10

#define WF_ROUND_FACE               1
/* Background ring: weisser Vollkreis mit duennem schwarzen Rand */
#define WF_CIRCLE_RADIUS            88
#define WF_RING_RADIUS              86

/* "FRI 12" oben, einzeilig */
#define WF_DAY_X                    50
#define WF_DAY_Y                    16
#define WF_DAY_W                    44
#define WF_DAY_H                    26
#define WF_DATE_X                   94
#define WF_DATE_Y                   16
#define WF_DATE_W                   40
#define WF_DATE_H                   26

/* Statuszeile: 24H links, BT mittig, XX% rechts */
#define WF_TIME_FORMAT_X            36
#define WF_TIME_FORMAT_Y            54
#define WF_TIME_FORMAT_W            24
#define WF_TIME_FORMAT_H            14
#define WF_BLUETOOTH_X              84
#define WF_BLUETOOTH_Y              56
#define WF_BATTERY_TEXT_X           116
#define WF_BATTERY_TEXT_Y           54
#define WF_BATTERY_TEXT_W           18
#define WF_BATTERY_TEXT_H           14
#define WF_BATTERY_PERCENT_X        134
#define WF_BATTERY_PERCENT_Y        54
#define WF_BATTERY_PERCENT_W        9
#define WF_BATTERY_PERCENT_H        14

/* Grosse HH:MM mittig */
#define WF_TIME_Y                   73
#define WF_TIME_H                   75
#define WF_TIME_DIGIT_W             38
#define WF_TIME_COLON_W             13
#define WF_TIME_0_X                 23
#define WF_TIME_1_X                 54
#define WF_TIME_COLON_X             86
#define WF_TIME_2_X                 96
#define WF_TIME_3_X                 127

/* Schritte: Icon links, Anzahl rechts. Achtung: untere Region ist durch den
 * runden Bildschirm beschnitten; alles innerhalb r=80 vom Zentrum (90,90). */
#define WF_STEPS_ICON_X             52
#define WF_STEPS_ICON_Y             148
#define WF_STEPS_TEXT_X             74
#define WF_STEPS_TEXT_Y             142
#define WF_STEPS_TEXT_W             60
#define WF_STEPS_TEXT_H             24
#define WF_FLAG_X                   68
#define WF_FLAG_Y                   148

/* Progressbalken */
#define WF_PROGRESS_X               60
#define WF_PROGRESS_Y               164
#define WF_PROGRESS_W               60
#define WF_PROGRESS_H               10
#define WF_PROGRESS_INSET_X         2
#define WF_PROGRESS_INSET_Y         2
#define WF_PROGRESS_BAR_H           5
#define WF_PROGRESS_RADIUS          2

/* Auf Round-Face nicht sichtbar */
#define WF_TOP_PANEL_X              0
#define WF_TOP_PANEL_Y              0
#define WF_TOP_PANEL_W              0
#define WF_TOP_PANEL_H              0
#define WF_BOTTOM_PANEL_X           0
#define WF_BOTTOM_PANEL_Y           0
#define WF_BOTTOM_PANEL_W           0
#define WF_BOTTOM_PANEL_H           0
#define WF_PANEL_RADIUS             0
#define WF_HEART_X                  300
#define WF_HEART_Y                  300
#define WF_BPM_TEXT_X               300
#define WF_BPM_TEXT_Y               300
#define WF_BPM_TEXT_W               20
#define WF_BPM_TEXT_H               20
#define WF_METER_X                  300
#define WF_METER_Y                  300
#define WF_BATTERY_ICON_X           300
#define WF_BATTERY_ICON_Y           300

#define WF_TIME_TRIM_X              0   /* X-nudge: whole HH:MM block left/right (see notes at top) */
#define WF_TIME_TRIM_Y              0   /* Y-nudge: whole HH:MM block up/down */
#define WF_INFO_TRIM_Y              0   /* Y-nudge: info texts (steps, date) */
#define WF_WEEKDAY_TRIM_Y           0   /* Y-nudge: weekday text only */

#else
/* Section 4/5: Rest, 144 x 168 rectangular (Aplite/Basalt/Diorite/Flint).
 * Schrift/Zahlen-Methode von 91-dub-40 (DS-Digital-Bold 71/22, lucon 10).
 * Layout der Statusbereiche bleibt; nur die Zeit-Box wird vergroessert,
 * damit die 71px-Ziffern hineinpassen, und die X-Positionen folgen
 * 91-dub-40 (TIMEDIGITS_DIGIT1..DIGIT4 = 3, 34, 74, 105). */
#define WF_FONT_TIME_RESOURCE       RESOURCE_ID_FONT_DIGITALE_64
#define WF_FONT_INFO_RESOURCE       RESOURCE_ID_FONT_DIGITALE_22
#define WF_FONT_TINY_RESOURCE       RESOURCE_ID_FONT_LUCIDIA_10
#define WF_FONT_WEEKDAY_RESOURCE    RESOURCE_ID_FONT_DIGITALE_22
#define WF_FONT_PERCENT_RESOURCE    RESOURCE_ID_FONT_LUCIDIA_10
#define WF_TIME_FONT_SIZE           71
#define WF_INFO_FONT_SIZE           22
#define WF_TINY_FONT_SIZE           10
#define WF_WEEKDAY_FONT_SIZE        22
#define WF_PERCENT_FONT_SIZE        10

#define WF_TOP_PANEL_X              5
#define WF_TOP_PANEL_Y              6
#define WF_TOP_PANEL_W              134
#define WF_TOP_PANEL_H              48
#define WF_BOTTOM_PANEL_X           5
#define WF_BOTTOM_PANEL_Y           60
#define WF_BOTTOM_PANEL_W           134
#define WF_BOTTOM_PANEL_H           102
#define WF_PANEL_RADIUS             8

#define WF_STEPS_ICON_X             17
#define WF_STEPS_ICON_Y             13
#define WF_STEPS_TEXT_X             58
#define WF_STEPS_TEXT_Y             6
#define WF_STEPS_TEXT_W             73
#define WF_STEPS_TEXT_H             24
#define WF_FLAG_X                   37
#define WF_FLAG_Y                   13

#define WF_HEART_X                  17
#define WF_HEART_Y                  31
#define WF_BPM_TEXT_X               92
#define WF_BPM_TEXT_Y               24
#define WF_BPM_TEXT_W               39
#define WF_BPM_TEXT_H               24

#define WF_PROGRESS_X               16
#define WF_PROGRESS_Y               31
#define WF_PROGRESS_W               112
#define WF_PROGRESS_H               16
#define WF_PROGRESS_INSET_X         6
#define WF_PROGRESS_INSET_Y         6
#define WF_PROGRESS_BAR_H           6
#define WF_PROGRESS_RADIUS          3

#define WF_METER_X                  17
#define WF_METER_Y                  69
#define WF_BLUETOOTH_X              33
#define WF_BLUETOOTH_Y              69
#define WF_BATTERY_ICON_X           111
#define WF_BATTERY_ICON_Y           69
#define WF_BATTERY_TEXT_X           86
#define WF_BATTERY_TEXT_Y           67
#define WF_BATTERY_TEXT_W           16
#define WF_BATTERY_TEXT_H           14
#define WF_BATTERY_PERCENT_X        103
#define WF_BATTERY_PERCENT_Y        67
#define WF_BATTERY_PERCENT_W        8
#define WF_BATTERY_PERCENT_H        14

#define WF_TIME_FORMAT_X            17
#define WF_TIME_FORMAT_Y            92
#define WF_TIME_FORMAT_W            24
#define WF_TIME_FORMAT_H            12
#define WF_DAY_X                    66
#define WF_DAY_Y                    78
#define WF_DAY_W                    42
#define WF_DAY_H                    24
#define WF_DATE_X                   109
#define WF_DATE_Y                   78
#define WF_DATE_W                   22
#define WF_DATE_H                   24

#define WF_TIME_Y                   88
#define WF_TIME_H                   80
#define WF_TIME_DIGIT_W             36
#define WF_TIME_COLON_W             40
#define WF_TIME_0_X                 5
#define WF_TIME_1_X                 35
#define WF_TIME_COLON_X             65
#define WF_TIME_2_X                 71
#define WF_TIME_3_X                 101

#define WF_TIME_TRIM_X              0   /* X-nudge: whole HH:MM block left/right (see notes at top) */
#define WF_TIME_TRIM_Y              0   /* Y-nudge: whole HH:MM block up/down */
#define WF_INFO_TRIM_Y              0   /* Y-nudge: info texts (steps, BPM, date) */
#define WF_WEEKDAY_TRIM_Y           0   /* Y-nudge: weekday text only */
#endif

typedef struct ClaySettings {
  int twelve;
  int stepgoal;
  int progress;
} ClaySettings;

static ClaySettings settings;

#define SETTINGS_KEY 1

#ifndef WF_ROUND_FACE
static GRect rect_inset(GRect rect, int inset) {
  return GRect(rect.origin.x + inset, rect.origin.y + inset,
               rect.size.w - (2 * inset), rect.size.h - (2 * inset));
}
#endif
static void default_settings() {
  settings.twelve = 0;
  settings.stepgoal = 5000;
  settings.progress = 0;
}

static void load_settings() {
  default_settings();
  persist_read_data(SETTINGS_KEY, &settings, sizeof(settings));
}

static void save_settings() {
  persist_write_data(SETTINGS_KEY, &settings, sizeof(settings));
}

static TextLayer *create_text_layer(GRect frame, GFont font, GTextAlignment alignment, Layer *parent) {
  TextLayer *layer = text_layer_create(frame);
  text_layer_set_text_color(layer, GColorBlack);
  text_layer_set_background_color(layer, GColorClear);
  text_layer_set_overflow_mode(layer, GTextOverflowModeTrailingEllipsis);
  text_layer_set_text_alignment(layer, alignment);
  text_layer_set_font(layer, font);
  layer_add_child(parent, text_layer_get_layer(layer));
  return layer;
}

static BitmapLayer *create_bitmap_layer(GBitmap **bitmap, uint32_t resource_id, GPoint origin, Layer *parent) {
  *bitmap = gbitmap_create_with_resource(resource_id);
  GRect frame = GRect(origin.x, origin.y,
                      gbitmap_get_bounds(*bitmap).size.w,
                      gbitmap_get_bounds(*bitmap).size.h);
  BitmapLayer *layer = bitmap_layer_create(frame);
  bitmap_layer_set_bitmap(layer, *bitmap);
  bitmap_layer_set_compositing_mode(layer, GCompOpSet);
  layer_add_child(parent, bitmap_layer_get_layer(layer));
  return layer;
}

static void set_battery_icon(bool charging) {
  if (battery_image != NULL) {
    gbitmap_destroy(battery_image);
    battery_image = NULL;
  }

  battery_image = gbitmap_create_with_resource(charging ? RESOURCE_ID_IMAGE_BATTERY_CHARGE : RESOURCE_ID_IMAGE_BATTERY);
  bitmap_layer_set_bitmap(battery_image_layer, battery_image);
  layer_mark_dirty(bitmap_layer_get_layer(battery_image_layer));
}

#ifndef WF_ROUND_FACE
static void draw_panel(GContext *ctx, GRect panel, int radius) {
  graphics_context_set_fill_color(ctx, GColorWhite);
  graphics_fill_rect(ctx, panel, radius, GCornersAll);

  graphics_context_set_stroke_color(ctx, GColorBlack);
  graphics_draw_round_rect(ctx, rect_inset(panel, 2), radius > 2 ? radius - 2 : radius);
}
#endif

static void background_update_proc(Layer *me, GContext *ctx) {
  GRect bounds = layer_get_bounds(me);

  graphics_context_set_fill_color(ctx, GColorBlack);
  graphics_fill_rect(ctx, bounds, 0, GCornerNone);

#ifdef WF_ROUND_FACE
  /* Round face: weisser Vollkreis mit duennem schwarzen Rand */
  GPoint center = GPoint(bounds.size.w / 2, bounds.size.h / 2);
  graphics_context_set_fill_color(ctx, GColorWhite);
  graphics_fill_circle(ctx, center, WF_CIRCLE_RADIUS);
  graphics_context_set_stroke_color(ctx, GColorBlack);
  graphics_draw_circle(ctx, center, WF_RING_RADIUS);
#else
  draw_panel(ctx, GRect(WF_TOP_PANEL_X, WF_TOP_PANEL_Y, WF_TOP_PANEL_W, WF_TOP_PANEL_H), WF_PANEL_RADIUS);
  draw_panel(ctx, GRect(WF_BOTTOM_PANEL_X, WF_BOTTOM_PANEL_Y, WF_BOTTOM_PANEL_W, WF_BOTTOM_PANEL_H), WF_PANEL_RADIUS);
#endif
}

static void battery_fill_update_proc(Layer *me, GContext *ctx) {
  int fill_width = (batteryPercent * 11) / 100;

  graphics_context_set_stroke_color(ctx, GColorBlack);
  graphics_context_set_fill_color(ctx, GColorBlack);
  graphics_fill_rect(ctx, GRect(2, 2, fill_width, 5), 0, GCornerNone);
}

static void progress_update_proc(Layer *me, GContext *ctx) {
  GRect bounds = layer_get_bounds(me);
  int bar_x = WF_PROGRESS_INSET_X;
  int bar_y = WF_PROGRESS_INSET_Y;
  int bar_w = bounds.size.w - (2 * WF_PROGRESS_INSET_X);
  int bar_h = WF_PROGRESS_BAR_H;
  int fill_w = 0;

  if (settings.stepgoal > 0) {
    if (actualstepcount >= settings.stepgoal) {
      fill_w = bar_w;
    } else {
      fill_w = (bar_w * actualstepcount) / settings.stepgoal;
    }
  }

  graphics_context_set_stroke_color(ctx, GColorBlack);
  graphics_context_set_fill_color(ctx, GColorBlack);
  graphics_draw_round_rect(ctx, GRect(bar_x, bar_y, bar_w, bar_h), WF_PROGRESS_RADIUS);

  if (fill_w > 0) {
    graphics_fill_rect(ctx, GRect(bar_x, bar_y, fill_w, bar_h), WF_PROGRESS_RADIUS, GCornersAll);
  }
}

static void update_battery(BatteryChargeState charge_state) {
  batteryPercent = charge_state.charge_percent;

#ifdef WF_ROUND_FACE
  /* Round face: nur Prozenttext aktualisieren, Icon/Fill-Layer bleiben dauerhaft versteckt. */
  snprintf(battery_percent_text, sizeof(battery_percent_text), "%02d", batteryPercent);
  text_layer_set_text(battery_percent_layer, battery_percent_text);
  text_layer_set_text(battery_percent_sign_layer, "%");
  return;
#endif

  if (batteryPercent >= 100) {
    set_battery_icon(false);
    layer_set_hidden(battery_fill_layer, false);
    layer_set_hidden(text_layer_get_layer(battery_percent_layer), true);
    layer_set_hidden(text_layer_get_layer(battery_percent_sign_layer), true);
    layer_mark_dirty(battery_fill_layer);
    return;
  }

  set_battery_icon(charge_state.is_charging);
  layer_set_hidden(battery_fill_layer, charge_state.is_charging);
  layer_set_hidden(text_layer_get_layer(battery_percent_layer), false);
  layer_set_hidden(text_layer_get_layer(battery_percent_sign_layer), false);

  snprintf(battery_percent_text, sizeof(battery_percent_text), "%02d", batteryPercent);
  text_layer_set_text(battery_percent_layer, battery_percent_text);
  text_layer_set_text(battery_percent_sign_layer, "%");
  layer_mark_dirty(battery_fill_layer);
}

static void toggle_bluetooth_icon(bool connected) {
  layer_set_hidden(bitmap_layer_get_layer(bluetooth_layer), !connected);
}

static void bluetooth_connection_callback(bool connected) {
  toggle_bluetooth_icon(connected);
}

static unsigned short get_display_hour(unsigned short hour) {
  if (settings.twelve == 1) {
    if (hour == 0) {
      return 12;
    }
    if (hour > 12) {
      return hour - 12;
    }
  }

  return hour;
}

static void update_days(struct tm *tick_time) {
#ifdef DEBUG_MODE
  tick_time->tm_mday = 30;   /* debug: fixed date 30 */
#endif

  text_layer_set_text(day_name_layer, DAY_NAMES[tick_time->tm_wday]);

  snprintf(date_text, sizeof(date_text), "%02d", tick_time->tm_mday);
  text_layer_set_text(date_layer, date_text);
}

static void update_hours(struct tm *tick_time) {
#ifdef DEBUG_MODE
  tick_time->tm_hour = 20;   /* debug: fixed hour 20 (-> 20:59 with debug minutes) */
#endif

  unsigned short display_hour = get_display_hour(tick_time->tm_hour);

  if (settings.twelve == 1) {
    if (tick_time->tm_hour > 11) {
      snprintf(time_format_text, sizeof(time_format_text), "PM");
      text_layer_set_text(time_format_layer, time_format_text);
      layer_set_hidden(text_layer_get_layer(time_format_layer), false);
    } else {
      layer_set_hidden(text_layer_get_layer(time_format_layer), true);
    }
  } else {
    snprintf(time_format_text, sizeof(time_format_text), "24H");
    text_layer_set_text(time_format_layer, time_format_text);
    layer_set_hidden(text_layer_get_layer(time_format_layer), false);
  }

  time_text[0][0] = '0' + (display_hour / 10);
  time_text[1][0] = '0' + (display_hour % 10);
  time_text[0][1] = '\0';
  time_text[1][1] = '\0';
  text_layer_set_text(time_layers[0], time_text[0]);
  text_layer_set_text(time_layers[1], time_text[1]);
}

static void update_minutes(struct tm *tick_time) {
#ifdef DEBUG_MODE
  tick_time->tm_min = 0;    /* debug: fixed minutes 00 (-> 23:00) */
#endif

  HealthMetric metric = HealthMetricStepCount;
  time_t start = time_start_of_today();
  time_t end = time(NULL);

  int steps = 0;
  HealthServiceAccessibilityMask mask = health_service_metric_accessible(metric, start, end);
  if (mask & HealthServiceAccessibilityMaskAvailable) {
    steps = (int)health_service_sum_today(metric);
    actualstepcount = steps;
  } else {
    actualstepcount = 0;
  }

#ifdef DEBUG_MODE
  steps = 54321;             /* debug: fixed step count */
  actualstepcount = steps;
#endif

  if (steps > 99999) {
    steps = 99999;
  }
  snprintf(steps_text, sizeof(steps_text), "%d", steps);
  text_layer_set_text(steps_text_layer, steps_text);

  if (settings.stepgoal > 0 && actualstepcount >= settings.stepgoal) {
    layer_set_hidden(bitmap_layer_get_layer(flag_layer), false);
  } else {
    layer_set_hidden(bitmap_layer_get_layer(flag_layer), true);
  }

  if (settings.progress == 1) {
    layer_set_hidden(progress_layer, true);
    layer_set_hidden(bitmap_layer_get_layer(heart_layer), false);
    layer_set_hidden(text_layer_get_layer(bpm_text_layer), false);

    HealthValue val = health_service_peek_current_value(HealthMetricHeartRateBPM);
    uint32_t bpm = (uint32_t)val;
#ifdef DEBUG_MODE
    bpm = 230;               /* debug: fixed heart rate (visible only in heart-rate mode) */
#endif
    if (bpm > 999) {
      bpm = 999;
    }
    snprintf(bpm_text, sizeof(bpm_text), "%lu", (unsigned long)bpm);
    text_layer_set_text(bpm_text_layer, bpm_text);
  } else {
    layer_set_hidden(bitmap_layer_get_layer(heart_layer), true);
    layer_set_hidden(text_layer_get_layer(bpm_text_layer), true);
    layer_set_hidden(progress_layer, false);
    layer_mark_dirty(progress_layer);
  }

  time_text[2][0] = ':';
  time_text[3][0] = '0' + (tick_time->tm_min / 10);
  time_text[4][0] = '0' + (tick_time->tm_min % 10);
  time_text[2][1] = '\0';
  time_text[3][1] = '\0';
  time_text[4][1] = '\0';

  text_layer_set_text(time_layers[2], time_text[2]);
  text_layer_set_text(time_layers[3], time_text[3]);
  text_layer_set_text(time_layers[4], time_text[4]);
}

static void handle_tick(struct tm *tick_time, TimeUnits units_changed) {
  if (units_changed & DAY_UNIT) {
    update_days(tick_time);
  }
  if (units_changed & HOUR_UNIT) {
    update_hours(tick_time);
  }
  if (units_changed & MINUTE_UNIT) {
    update_minutes(tick_time);
  }
}

static void in_received_handler(DictionaryIterator *iter, void *context) {
  Tuple *twelve_conf = dict_find(iter, MESSAGE_KEY_twelve);
  if (twelve_conf) {
    settings.twelve = twelve_conf->value->int32;
  }

  Tuple *steps_conf = dict_find(iter, MESSAGE_KEY_steps);
  if (steps_conf) {
    char buf[30];
    strncpy(buf, steps_conf->value->cstring, sizeof(buf) - 1);
    buf[sizeof(buf) - 1] = '\0';
    settings.stepgoal = atoi(buf);
  }

  Tuple *progress_conf = dict_find(iter, MESSAGE_KEY_progress);
  if (progress_conf) {
    settings.progress = progress_conf->value->int32;
  }

#ifdef WF_ROUND_FACE
  settings.progress = 0;  /* Round face hat keinen Herzfrequenzmodus */
#endif

  save_settings();

  time_t now = time(NULL);
  struct tm *tick_time = localtime(&now);
  handle_tick(tick_time, DAY_UNIT | HOUR_UNIT | MINUTE_UNIT);
}

static void init(void) {
  load_settings();

  window = window_create();
  window_set_background_color(window, GColorBlack);

  app_message_register_inbox_received(in_received_handler);
  app_message_open(128, 128);

  window_stack_push(window, true);
  window_layer = window_get_root_layer(window);
  GRect bounds = layer_get_bounds(window_layer);

  font_time = fonts_load_custom_font(resource_get_handle(WF_FONT_TIME_RESOURCE));
  font_info = fonts_load_custom_font(resource_get_handle(WF_FONT_INFO_RESOURCE));
  font_tiny = fonts_load_custom_font(resource_get_handle(WF_FONT_TINY_RESOURCE));
  font_weekday = fonts_load_custom_font(resource_get_handle(WF_FONT_WEEKDAY_RESOURCE));
  font_percent = fonts_load_custom_font(resource_get_handle(WF_FONT_PERCENT_RESOURCE));

  background_layer = layer_create(bounds);
  layer_set_update_proc(background_layer, background_update_proc);
  layer_add_child(window_layer, background_layer);

  steps_layer = create_bitmap_layer(&steps_image, RESOURCE_ID_IMAGE_STEPS, GPoint(WF_STEPS_ICON_X, WF_STEPS_ICON_Y), window_layer);
  flag_layer = create_bitmap_layer(&flag_image, RESOURCE_ID_FLAG, GPoint(WF_FLAG_X, WF_FLAG_Y), window_layer);
  layer_set_hidden(bitmap_layer_get_layer(flag_layer), true);

  heart_layer = create_bitmap_layer(&heart_image, RESOURCE_ID_HEART, GPoint(WF_HEART_X, WF_HEART_Y), window_layer);

  progress_layer = layer_create(GRect(WF_PROGRESS_X, WF_PROGRESS_Y, WF_PROGRESS_W, WF_PROGRESS_H));
  layer_set_update_proc(progress_layer, progress_update_proc);
  layer_add_child(window_layer, progress_layer);

  meter_bar_layer = create_bitmap_layer(&meter_bar_image, RESOURCE_ID_IMAGE_METER_BAR, GPoint(WF_METER_X, WF_METER_Y), window_layer);
  bluetooth_layer = create_bitmap_layer(&bluetooth_image, RESOURCE_ID_IMAGE_BLUETOOTH, GPoint(WF_BLUETOOTH_X, WF_BLUETOOTH_Y), window_layer);

  battery_image = gbitmap_create_with_resource(RESOURCE_ID_IMAGE_BATTERY);
  battery_image_layer = bitmap_layer_create(GRect(WF_BATTERY_ICON_X, WF_BATTERY_ICON_Y,
                                                  gbitmap_get_bounds(battery_image).size.w,
                                                  gbitmap_get_bounds(battery_image).size.h));
  bitmap_layer_set_bitmap(battery_image_layer, battery_image);
  bitmap_layer_set_compositing_mode(battery_image_layer, GCompOpSet);
  layer_add_child(window_layer, bitmap_layer_get_layer(battery_image_layer));

  battery_fill_layer = layer_create(GRect(WF_BATTERY_ICON_X, WF_BATTERY_ICON_Y,
                                          gbitmap_get_bounds(battery_image).size.w,
                                          gbitmap_get_bounds(battery_image).size.h));
  layer_set_update_proc(battery_fill_layer, battery_fill_update_proc);
  layer_add_child(window_layer, battery_fill_layer);

  steps_text_layer = create_text_layer(GRect(WF_STEPS_TEXT_X, WF_STEPS_TEXT_Y + WF_INFO_TRIM_Y,
                                             WF_STEPS_TEXT_W, WF_STEPS_TEXT_H),
                                       font_info, GTextAlignmentRight, window_layer);
  bpm_text_layer = create_text_layer(GRect(WF_BPM_TEXT_X, WF_BPM_TEXT_Y + WF_INFO_TRIM_Y,
                                           WF_BPM_TEXT_W, WF_BPM_TEXT_H),
                                     font_info, GTextAlignmentRight, window_layer);

  battery_percent_layer = create_text_layer(GRect(WF_BATTERY_TEXT_X, WF_BATTERY_TEXT_Y,
                                                  WF_BATTERY_TEXT_W, WF_BATTERY_TEXT_H),
                                            font_tiny, GTextAlignmentRight, window_layer);
  battery_percent_sign_layer = create_text_layer(GRect(WF_BATTERY_PERCENT_X, WF_BATTERY_PERCENT_Y,
                                                       WF_BATTERY_PERCENT_W, WF_BATTERY_PERCENT_H),
                                                 font_percent, GTextAlignmentLeft, window_layer);

  time_format_layer = create_text_layer(GRect(WF_TIME_FORMAT_X, WF_TIME_FORMAT_Y,
                                              WF_TIME_FORMAT_W, WF_TIME_FORMAT_H),
                                        font_tiny, GTextAlignmentLeft, window_layer);
  day_name_layer = create_text_layer(GRect(WF_DAY_X, WF_DAY_Y + WF_WEEKDAY_TRIM_Y,
                                           WF_DAY_W, WF_DAY_H),
                                     font_weekday, GTextAlignmentLeft, window_layer);
  date_layer = create_text_layer(GRect(WF_DATE_X, WF_DATE_Y + WF_INFO_TRIM_Y,
                                       WF_DATE_W, WF_DATE_H),
                                 font_info, GTextAlignmentRight, window_layer);

  time_layers[0] = create_text_layer(GRect(WF_TIME_0_X + WF_TIME_TRIM_X, WF_TIME_Y + WF_TIME_TRIM_Y,
                                           WF_TIME_DIGIT_W, WF_TIME_H),
                                     font_time, GTextAlignmentRight, window_layer);
  time_layers[1] = create_text_layer(GRect(WF_TIME_1_X + WF_TIME_TRIM_X, WF_TIME_Y + WF_TIME_TRIM_Y,
                                           WF_TIME_DIGIT_W, WF_TIME_H),
                                     font_time, GTextAlignmentRight, window_layer);
  time_layers[2] = create_text_layer(GRect(WF_TIME_COLON_X + WF_TIME_TRIM_X, WF_TIME_Y + WF_TIME_TRIM_Y,
                                           WF_TIME_COLON_W, WF_TIME_H),
                                     font_time, GTextAlignmentLeft, window_layer);
  time_layers[3] = create_text_layer(GRect(WF_TIME_2_X + WF_TIME_TRIM_X, WF_TIME_Y + WF_TIME_TRIM_Y,
                                           WF_TIME_DIGIT_W, WF_TIME_H),
                                     font_time, GTextAlignmentRight, window_layer);
  time_layers[4] = create_text_layer(GRect(WF_TIME_3_X + WF_TIME_TRIM_X, WF_TIME_Y + WF_TIME_TRIM_Y,
                                           WF_TIME_DIGIT_W, WF_TIME_H),
                                     font_time, GTextAlignmentRight, window_layer);

#ifdef WF_ROUND_FACE
  /* Round face zeigt weder Herzfrequenz noch Akku-Icon noch Meter-Bar */
  layer_set_hidden(bitmap_layer_get_layer(heart_layer), true);
  layer_set_hidden(text_layer_get_layer(bpm_text_layer), true);
  layer_set_hidden(bitmap_layer_get_layer(meter_bar_layer), true);
  layer_set_hidden(bitmap_layer_get_layer(battery_image_layer), true);
  layer_set_hidden(battery_fill_layer, true);
  settings.progress = 0;  /* Progressbalken-Modus statt Herzfrequenz erzwingen */
#endif

  toggle_bluetooth_icon(bluetooth_connection_service_peek());
  update_battery(battery_state_service_peek());

  time_t now = time(NULL);
  struct tm *tick_time = localtime(&now);
  handle_tick(tick_time, DAY_UNIT | HOUR_UNIT | MINUTE_UNIT);

  tick_timer_service_subscribe(MINUTE_UNIT, handle_tick);
  bluetooth_connection_service_subscribe(bluetooth_connection_callback);
  battery_state_service_subscribe(update_battery);
}

static void deinit(void) {
  save_settings();

  tick_timer_service_unsubscribe();
  bluetooth_connection_service_unsubscribe();
  battery_state_service_unsubscribe();

  for (int i = 0; i < TOTAL_TIME_FIELDS; i++) {
    if (time_layers[i]) {
      text_layer_destroy(time_layers[i]);
      time_layers[i] = NULL;
    }
  }

  if (steps_text_layer) text_layer_destroy(steps_text_layer);
  if (bpm_text_layer) text_layer_destroy(bpm_text_layer);
  if (battery_percent_layer) text_layer_destroy(battery_percent_layer);
  if (battery_percent_sign_layer) text_layer_destroy(battery_percent_sign_layer);
  if (time_format_layer) text_layer_destroy(time_format_layer);
  if (day_name_layer) text_layer_destroy(day_name_layer);
  if (date_layer) text_layer_destroy(date_layer);

  if (battery_fill_layer) layer_destroy(battery_fill_layer);
  if (progress_layer) layer_destroy(progress_layer);
  if (background_layer) layer_destroy(background_layer);

  if (meter_bar_layer) bitmap_layer_destroy(meter_bar_layer);
  if (steps_layer) bitmap_layer_destroy(steps_layer);
  if (heart_layer) bitmap_layer_destroy(heart_layer);
  if (flag_layer) bitmap_layer_destroy(flag_layer);
  if (bluetooth_layer) bitmap_layer_destroy(bluetooth_layer);
  if (battery_image_layer) bitmap_layer_destroy(battery_image_layer);

  if (meter_bar_image) gbitmap_destroy(meter_bar_image);
  if (steps_image) gbitmap_destroy(steps_image);
  if (heart_image) gbitmap_destroy(heart_image);
  if (flag_image) gbitmap_destroy(flag_image);
  if (bluetooth_image) gbitmap_destroy(bluetooth_image);
  if (battery_image) gbitmap_destroy(battery_image);

  if (font_time) fonts_unload_custom_font(font_time);
  if (font_info) fonts_unload_custom_font(font_info);
  if (font_tiny) fonts_unload_custom_font(font_tiny);
  if (font_weekday) fonts_unload_custom_font(font_weekday);
  if (font_percent) fonts_unload_custom_font(font_percent);

  if (window) window_destroy(window);
}

int main(void) {
  init();
  app_event_loop();
  deinit();
}

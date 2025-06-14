#define PPQN1 0
#define PPQN2 1
#define PPQN4 2
#define PPQN8 3
#define PPQN12 4
#define PPQN16 5
#define PPQN24 6
#define REGISTER_SIZE 168

#define NO_SMOOTH 0
#define LOW_SMOOTH 1
#define MEDIUM_SMOOTH 2
#define HIGH_SMOOTH 3

//-----------
#define FORWARD 0
#define REVERSE 1
#define CLOCK_MODE 1
#define CV_MODE 0
#define POSITIVE_V 0
#define NEGATIVE_V 1
#define RUN_GATE 0
#define RUN_TRIG 1
#define CV_TYPE 0

#define COLOR_LCD_RED 0xdd, 0x33, 0x33, 0xff
#define COLOR_LCD_GREEN 0x33, 0xdd, 0x33, 0xff
//#define COLOR_LCD_BLUE 0x55, 0x55, 0xdd, 0xff
#define COLOR_LCD_BLUE 0x68, 0x81, 0xb8, 0xff
#define COLOR_LCD_YELLOW 0xdd, 0xdd, 0x33, 0xff
#define COLOR_LCD_LYELLOW 0xd4, 0xff, 0x2a, 0xff
#define COLOR_LCD_DYELLOW 0xff, 0xd4, 0x2a, 0xff
#define COLOR_LCD_VIOLET 0xdd, 0x11, 0xdd, 0xff
#define COLOR_LCD_LBLUE 0x33, 0xdd, 0xdd, 0xff
#define COLOR_LCD_ROSE 0xde, 0x87, 0x87, 0xff
#define COLOR_LCD_GREY 0x7c, 0x91, 0x6f, 0xff
#define COLOR_LCD_LGREY 0xbe, 0xc8, 0xb7, 0xff

#define COLOR_EGA_BLACK			0x00, 0x00, 0x00, 0xff // Nero
//#define COLOR_EGA_BLUE 			0x00, 0x00, 0xaa, 0xff // Blu
#define COLOR_EGA_BLUE 			0x55, 0x55, 0xaa, 0xff // Blu
#define COLOR_EGA_GREEN 		0x00, 0xaa, 0x00, 0xff // Verde
//#define COLOR_EGA_CYAN 		0x00, 0xaa, 0xaa, 0xff // Ciano
#define COLOR_EGA_CYAN 			0x33, 0xcc, 0xcc, 0xff // Cyan leggermente schiarito
//#define COLOR_EGA_RED 		0xaa, 0x00, 0x00, 0xff // Rosso
#define COLOR_EGA_RED 			0xee, 0x20, 0x20, 0xff // Rosso acceso ma non eccessivo
//#define COLOR_EGA_MAGENTA 	0xaa, 0x00, 0xaa, 0xff // Magenta
#define COLOR_EGA_MAGENTA 		0xcc, 0x33, 0xcc, 0xff // Magenta leggermente schiarito
//#define COLOR_EGA_BROWN 		0xaa, 0x55, 0x00, 0xff // Marrone
#define COLOR_EGA_BROWN 		0xcc, 0xaa, 0x33, 0xff // Giallo caldo smorzato
//#define COLOR_EGA_LIGHT_GRAY 	0xaa, 0xaa, 0xaa, 0xff // Grigio chiaro
#define COLOR_EGA_LIGHT_GRAY 	0xcc, 0xcc, 0xcc, 0xff // Grigio chiaro + chiaro
//#define COLOR_EGA_DARK_GRAY 	0x55, 0x55, 0x55, 0xff // Grigio scuro
#define COLOR_EGA_DARK_GRAY 	0x80, 0x80, 0x80, 0xff // Grigio medio-scuro
#define COLOR_EGA_DARK_DARK_GRAY 	0x60, 0x60, 0x60, 0xff // Grigio medio-scuro
//#define COLOR_EGA_LIGHT_BLUE 	0x55, 0x55, 0xff, 0xff // Blu chiaro
#define COLOR_EGA_LIGHT_BLUE 	0x33, 0x99, 0xff, 0xff // Blu molto chiaro
#define COLOR_EGA_LIGHT_GREEN	0x55, 0xff, 0x55, 0xff // Verde chiaro
#define COLOR_EGA_LIGHT_CYAN 	0x55, 0xff, 0xff, 0xff // Ciano chiaro
#define COLOR_EGA_LIGHT_RED 	0xff, 0x55, 0x55, 0xff // Rosso chiaro
#define COLOR_EGA_LIGHT_MAGENTA 0xff, 0x55, 0xff, 0xff // Magenta chiaro
#define COLOR_EGA_YELLOW 		0xff, 0xff, 0x55, 0xff // Giallo
#define COLOR_EGA_WHITE 		0xff, 0xff, 0xff, 0xff // Bianco


#define FORWARD 0
#define REVERSE 1
#define PINGPONG 2
#define PINGPONGEXT 3
#define RANDOM1 4
#define RANDOM2 5
#define CVOLTAGE 6

#define MAXMODES 6
#define MAXTRACKS 8
#define ALLTRACKS 9
#define MC 8

#define IN_LENGTH 0
#define IN_MODE 1
#define IN_OUTSCALE 2
#define IN_RSTSTEP 3
#define IN_RETRIG 4
#define IN_REV 5
#define IN_RUN 6
#define IN_SWING 7
#define KNOB_MODE 8
#define KNOB_OUTSCALE 9
#define KNOB_RSTSTEP 10
#define KNOB_PROB 11
#define KNOB_SWING 12
#define KNOB_ATN 13
#define KNOB_ATNV 14

#define KNOB_SHIFT 8
#define KNOB_NR 7
#define MAXUSER 15
#define FIRST_ATN 4

#define RST_NONE 0
#define RST_ONRUN 1
#define RST_ONSTOP 2
#define RST_DEFAULT 3
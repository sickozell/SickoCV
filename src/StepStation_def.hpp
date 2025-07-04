//-----------
#define FORWARD 0
#define REVERSE 1
//#define CLOCK_MODE 1
//#define CV_MODE 0
//#define POSITIVE_V 0
//#define NEGATIVE_V 1
#define RUN_GATE 0
#define RUN_TRIG 1
#define CV_TYPE 0

#define COLOR_LCD_RED 0xdd, 0x33, 0x33
#define COLOR_LCD_GREEN 0x33, 0xdd, 0x33
//#define COLOR_LCD_BLUE 0x55, 0x55, 0xdd
#define COLOR_LCD_BLUE 0x68, 0x81, 0xb8
#define COLOR_LCD_YELLOW 0xdd, 0xdd, 0x33
#define COLOR_LCD_LYELLOW 0xd4, 0xff, 0x2a
#define COLOR_LCD_DYELLOW 0xff, 0xd4, 0x2a
#define COLOR_LCD_VIOLET 0xdd, 0x11, 0xdd
#define COLOR_LCD_LBLUE 0x33, 0xdd, 0xdd
#define COLOR_LCD_ROSE 0xde, 0x87, 0x87
#define COLOR_LCD_GREY 0x7c, 0x91, 0x6f
#define COLOR_LCD_LGREY 0xbe, 0xc8, 0xb7

#define COLOR_USER_RED 0xff, 0x00, 0x00
#define COLOR_USER_LIGHT_RED 0xff, 0x80, 0x80
#define COLOR_USER_GREEN 0x00, 0xff, 0x00
#define COLOR_USER_CYAN 0x00, 0xff, 0xff
#define COLOR_USER_YELLOW 0xff, 0xff, 0x00
#define COLOR_USER_MAGENTA 0xff, 0x00, 0xff
#define COLOR_USER_BROWN 0xff, 0x66, 0x00
#define COLOR_USER_GREY 0xcc, 0xcc, 0xcc
#define COLOR_USER_BLUE 0x00, 0xcc, 0xff
#define COLOR_USER_LIGHT_GREEN 0xb3, 0xff, 0x80
#define	COLOR_USER_PURPLE 0xdd, 0x55, 0xff
#define COLOR_USER_ROSE 0xff, 0xaa, 0xee
#define COLOR_USER_LIGHT_YELLOW 0xe5, 0xff, 0x80
#define COLOR_USER_LIGHT_MAGENTA 0xff, 0x80, 0xe5
#define COLOR_USER_LIGHT_BROWN 0xff, 0x99, 0x55
#define COLOR_USER_LIGHT_GREY 0xec, 0xec, 0xec
#define COLOR_USER_LIGHT_PURPLE 0xee, 0xaa, 0xff
#define COLOR_USER_LIGHT_BLUE 0xaa, 0xee, 0xff

#define COLOR_EGA_BLACK			0x00, 0x00, 0x00 // Nero
//#define COLOR_EGA_BLUE 			0x00, 0x00, 0xaa // Blu
#define COLOR_EGA_BLUE 			0x55, 0x55, 0xaa // Blu
#define COLOR_EGA_GREEN 		0x00, 0xaa, 0x00 // Verde
//#define COLOR_EGA_CYAN 		0x00, 0xaa, 0xaa // Ciano
#define COLOR_EGA_CYAN 			0x33, 0xcc, 0xcc // Cyan leggermente schiarito
//#define COLOR_EGA_RED 		0xaa, 0x00, 0x00 // Rosso
#define COLOR_EGA_RED 			0xee, 0x20, 0x20 // Rosso acceso ma non eccessivo
//#define COLOR_EGA_MAGENTA 	0xaa, 0x00, 0xaa // Magenta
#define COLOR_EGA_MAGENTA 		0xcc, 0x33, 0xcc // Magenta leggermente schiarito
//#define COLOR_EGA_BROWN 		0xaa, 0x55, 0x00 // Marrone
#define COLOR_EGA_BROWN 		0xcc, 0xaa, 0x33 // Giallo caldo smorzato
//#define COLOR_EGA_LIGHT_GRAY 	0xaa, 0xaa, 0xaa // Grigio chiaro
#define COLOR_EGA_LIGHT_GRAY 	0xcc, 0xcc, 0xcc // Grigio chiaro + chiaro
//#define COLOR_EGA_DARK_GRAY 	0x55, 0x55, 0x55 // Grigio scuro
#define COLOR_EGA_DARK_GRAY 	0x80, 0x80, 0x80 // Grigio medio-scuro
#define COLOR_EGA_DARK_DARK_GRAY 	0x60, 0x60, 0x60 // Grigio medio-scuro
//#define COLOR_EGA_LIGHT_BLUE 	0x55, 0x55, 0xff // Blu chiaro
#define COLOR_EGA_LIGHT_BLUE 	0x33, 0x99, 0xff // Blu molto chiaro
#define COLOR_EGA_LIGHT_GREEN	0x55, 0xff, 0x55 // Verde chiaro
#define COLOR_EGA_LIGHT_CYAN 	0x55, 0xff, 0xff // Ciano chiaro
#define COLOR_EGA_LIGHT_RED 	0xff, 0x55, 0x55 // Rosso chiaro
#define COLOR_EGA_LIGHT_MAGENTA 0xff, 0x55, 0xff // Magenta chiaro
#define COLOR_EGA_YELLOW 		0xff, 0xff, 0x55 // Giallo
#define COLOR_EGA_WHITE 		0xff, 0xff, 0xff // Bianco

#define BPM_BLUE	0x55, 0xff, 0xff
#define BPM_YELLOW	0xdd, 0xdd, 0x33
#define BPM_GREEN	0x33, 0xdd, 0x33

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

#define IN_CHANGE 0
#define IN_CHANGEPROB 1
#define IN_LENGTH 2
#define IN_MODE 3
#define IN_OUTSCALE 4
#define IN_RSTSTEP 5
#define IN_RETRIG 6
#define IN_REV 7
#define IN_RUN 8
#define KNOB_CHANGEPROB 9
#define KNOB_MODE 10
#define KNOB_OUTSCALE 11
#define KNOB_RSTSTEP 12
#define KNOB_RETRIGPROB 13
#define KNOB_ATN 14
#define KNOB_ATNV 15

#define KNOB_SHIFT 9
#define KNOB_NR 7
#define MAXUSER 16
#define FIRST_ATN 4

#define RST_NONE 0
#define RST_ONRUN 1
#define RST_ONSTOP 2
#define RST_DEFAULT 3
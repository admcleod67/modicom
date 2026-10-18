/* Cursor & keypad key codes */

#define	K_UP	0x4800
#define K_DOWN	0x5000
#define K_LEFT	0x4b00
#define K_RIGHT	0x4d00
#define K_INS	0x5200
#define	K_DEL	0x5300
#define K_HOME  0x4700
#define K_END	0x4f00
#define K_PGUP	0x4900
#define K_PGDN	0x5100


/* Special key codes */

#define K_ESC    0x1b
#define K_RETURN 0x0d
#define K_ENTER	 0x0d
#define K_TAB    0x09
#define K_BS     0x08
#define K_SPACE	 0x20


/* Function key codes */

#define K_F1	0x3b00
#define K_F2	0x3c00
#define K_F3	0x3d00
#define K_F4	0x3e00
#define K_F5	0x3f00
#define K_F6	0x4000
#define K_F7    0x4100
#define K_F8    0x4200
#define K_F9    0x4300
#define K_F10	0x4400


/* Control key codes */

#define K_CTRL_A	0x01
#define K_CTRL_B	0x02
#define K_CTRL_C	0x03
#define K_CTRL_D	0x04
#define K_CTRL_E	0x05
#define K_CTRL_F	0x06
#define K_CTRL_K	0x0b
#define K_CTRL_L	0x0c
#define K_CTRL_N	0x0e
#define K_CTRL_R	0x12
#define K_CTRL_S	0x13
#define K_CTRL_T	0x14
#define K_CTRL_W	0x17
#define K_CTRL_Y	0x19
#define K_CTRL_LEFT	0x7300
#define K_CTRL_RIGHT	0x7400
#define K_CTRL_END      0x7500


/* Keyboard function prototype */

unsigned getkey(void);
unsigned getline(char *str, int x, int y, int maxlen);
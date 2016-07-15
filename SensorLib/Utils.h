#ifndef UTILS_H_
#define UTILS_H_


#define BOOL_TO_ONOFF(b) (b ? "on" : "off")
#define NIBBLE_TO_HEX_CHAR(i) ((i <= 9) ? ('0' + i) : ('A' - 10 + i))
#define HIGH_NIBBLE(i) ((i >> 4) & 0x0F)
#define LOW_NIBBLE(i) (i & 0x0F)

#define HEX_CHAR_TO_NIBBLE(c) ((c >= 'A') ? (c - 'A' + 0x0A) : (c - '0'))
#define HEX_PAIR_TO_BYTE(h, l) ((HEX_CHAR_TO_NIBBLE(h) << 4) + HEX_CHAR_TO_NIBBLE(l))

#define ARRAY_SIZE(a) (sizeof(a)/sizeof(a[0]))

#define PRINTLN(...) {if(_monitor) _monitor->println(__VA_ARGS__); }
#define PRINT(...) {if(_monitor) _monitor->print(__VA_ARGS__); }
#define PRINTLNF(...) {if(_monitor) _monitor->println(F(__VA_ARGS__)); }
#define PRINTF(...) {if(_monitor) _monitor->print(F(__VA_ARGS__)); }

char* int2str(register int i);

// outputs the incoming byte-array as HEX characters on the destination,
// and separating with specified char if needed
void writeHex(uint8_t *data, uint8_t length, uint8_t *dest, uint8_t separator =
		0);

#endif  /* UTILS_H_ */


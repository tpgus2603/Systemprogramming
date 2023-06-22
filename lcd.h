#ifndef __LCD_H__
#define __LCD_H__


void lcd_init(void);
void lcd_byte(int bits, int mode);
void lcd_toggle_enable(int bits);

// added by Lewis
void typeInt(int i);
void typeFloat(float myFloat);
void lcdLoc(int line); //move cursor
void ClrLcd(void); // clr LCD return home
void typeln(const char* s);
void typeChar(char val);
<<<<<<< HEAD
void lcdStart(int line,char *string);
void lcdSetup();
=======
void lcdStart(int line, int fd, char* string);
>>>>>>> a801e991d4f776aa8504dd8eb83b5245290eabc7

#endif

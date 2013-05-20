#include "util.h"
#define INT_DIGITS 3		/* enough for 64 bit integer */

char *itoa(int i)
{
  /* Room for INT_DIGITS digits, - and '\0' */
  static char buf[INT_DIGITS + 2];
  char *p = buf + INT_DIGITS + 1;	/* points to terminating '\0' */
  if (i >= 0) {
    do {
      *--p = '0' + (i % 10);
      i /= 10;
    } while (i != 0);
    return p;
  }
  else {			/* i < 0 */
    do {
      *--p = '0' - (i % 10);
      i /= 10;
    } while (i != 0);
    *--p = '-';
  }
  return p;
}


char *itoa_alt(int num)
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

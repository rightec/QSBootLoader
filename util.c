/* 
 * File:   util.c
 * Author: Fernando Morani
 *
 * Created on 6 aprile 2022, 23.04
 */

#include "util.h"


/* Converte uno short in ascii esadecimale
*/
void wxtoa(char *s, short n)
{
WVAL wn;

	wn.i = n;				// copia

	bxtoa(s+2, wn.c[1]);			// prima la parte alta ...
	bxtoa(s, wn.c[0]);		// prima la parte alta ...
}


/* Converte byte in ascii esadecimale
*/
void bxtoa(char *s, uint8_t  n)
{
uint8_t b;

	b = n & 0x0F;

	if( b > 9 )
		s[1] = (b - 10) + 'A';
	else
		s[1] = b + '0';

	b = (n & 0xF0) >> 4;

	if( b > 9 )
		s[0] = (b - 10) + 'A';
	else
		s[0] = b + '0';
}


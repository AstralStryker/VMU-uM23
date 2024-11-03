#include <stdbool.h>
#include <string.h>
#include <stdint.h>
#include "printf.h"

static void StrPrvPrintfEx_number(void (*purcharF)(char), uint32_t number, bool zeroExtend, uint32_t padToLength, bool base10, bool isSigned)
{
	char buf[64];
	uint32_t idx = sizeof(buf) - 1;
	uint32_t chr, i;
	bool neg = false;
	uint32_t numPrinted = 0;
	
	
	if (isSigned && (((int32_t)number) < 0)) {
		 purcharF('-');
		 number = -number;
	}
	
	if(padToLength > 63)
		padToLength = 63;
	
	buf[idx--] = 0;	//terminate
	
	do{
		if (base10) {
			chr = number % 10;
			number = number / 10;
		}
		else {
			chr = number % 16;
			number = number / 16;
		}
		
		buf[idx--] = (chr >= 10)?(chr + 'A' - 10):(chr + '0');
		
		numPrinted++;
		
	}while(number);
	
	if (neg) {
	
		buf[idx--] = '-';
		numPrinted++;
	}
	
	if (padToLength > numPrinted)
		padToLength -= numPrinted;
	else
		padToLength = 0;
	
	while(padToLength--) {
		
		buf[idx--] = zeroExtend?'0':' ';
		numPrinted++;
	}
	
	idx++;
	
	for (i = 0; i < numPrinted; i++)
		purcharF((buf + idx)[i]);
}

static uint32_t StrVPrintf_StrLen_withMax(const char* s,uint32_t max){
	
	uint32_t len = 0;
	
	while((*s++) && (len < max)) len++;
	
	return len;
}

void pr(void (*purcharF)(char), const char* fmtStr, ...){
	
	char c, t;
	uint32_t i, cc, val;
	va_list vl;
	
	va_start(vl, fmtStr);

	
	while((c = *fmtStr++) != 0){
		
		if(c == '\n'){
			purcharF(c);
		}
		else if(c == '%'){
			
			bool zeroExtend = false, isSigned = false;
			uint32_t padToLength = 0,len, i;
			const char* str;
			
more_fmt:
			
			c = *fmtStr++;
			
			switch(c){
				
				case '%':
					
					purcharF(c);
					break;
				
				case 'c':
					
					t = va_arg(vl,unsigned int);
					purcharF(t);
					break;
				
				case 's':
					
					str = va_arg(vl,char*);
					if(!str) str = "(null)";
					while((c = *str++))
						purcharF(c);
					break;
				
				case '0':
					
					if(!zeroExtend && !padToLength){
						
						zeroExtend = true;
						goto more_fmt;
					}
				
				case '1':
				case '2':
				case '3':
				case '4':
				case '5':
				case '6':
				case '7':
				case '8':
				case '9':
					
					padToLength = (padToLength * 10) + c - '0';
					goto more_fmt;
				
				case 'd':
					isSigned = true;
					//fallthrough
				case 'u':
					
					val = va_arg(vl, uint32_t);
					StrPrvPrintfEx_number(purcharF, val, zeroExtend, padToLength, true, isSigned);
					break;
					
				case 'x':
				case 'X':
					val = va_arg(vl,uint32_t);
					StrPrvPrintfEx_number(purcharF, val, zeroExtend, padToLength, false, false);
					break;
					
				case 'l':
					//ignored
					goto more_fmt;
				
				default:
					purcharF(c);
					break;
				
			}
		}
		else
			purcharF(c);
	}

	va_end(vl);
}


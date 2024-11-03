#include <stdbool.h>
#include <stdint.h>
#include "../../vmu.arm.h"
#include "printf.h"

static const uint16_t flappy1[14] = {0x03f0, 0x0c48, 0x1084, 0x788a, 0x848a, 0x8242, 0x823e, 0x4441, 0x3fbe, 0x2042, 0x183c, 0x07c0};
static const uint16_t flappy2[14] = {0x03f0, 0x0c48, 0x1084, 0x208a, 0x408a, 0x7c42, 0x823e, 0x8241, 0x7fbe, 0x2042, 0x183c, 0x07c0};
static const uint16_t *flappy[] = {flappy1, flappy2};


static uint8_t mScreenC, mScreenR;



void __attribute__((naked)) prPutcharDebug(char chr)
{
	asm volatile(".hword 0xde01\n\tbx lr");
}

static void __attribute__((naked)) hyperDrawChar(char chr, uint32_t row, uint32_t col, bool hilite)
{
	asm volatile(".hword 0xDE06\n\tbx lr");
}

void prPutcharScreen(char chr)
{
	hyperDrawChar(chr, mScreenR, mScreenC, false);
	if (++mScreenC == 12) {
		mScreenC = 0;
		if (++mScreenR == 5)
			mScreenR = 0;
	}
}


static bool drawFlappyRaw(uint32_t idx, int32_t r, bool draw)	//return true if collision, guaranteed always onscreen
{
	volatile uint16_t *scr = (volatile uint16_t*)VMU_BASE_ADDR_GFX;
	const uint16_t *img = flappy[idx];
	bool ret = false;
	uint32_t er;
	
	scr += (r / 2) * 8 + ((r & 1) * 3);
	
	for (er = r; er < r + 12; er++, scr += 3) {
		
		uint16_t t, val = *img++;
		
		if (!draw)
			val <<= 1;
		
		val = __builtin_bswap16(val);
		t = *scr;
		if (t & val)
			ret = true;
		*scr = t ^ val;
		
		if (er & 1)
			scr += 2;
	}
	
	return ret;
}

static bool drawFlappyFrame(int32_t r, bool draw /*else erase */)	//return true if no collision
{
	static uint8_t frm;
	
	if (draw && ++frm == 6)
		frm = 0;
	
	return drawFlappyRaw(frm & 1, r + ((frm / 3) & 1), draw);
}

static void slideScreenLeft(void)
{
	volatile uint32_t *screenData = (volatile uint32_t*)VMU_BASE_ADDR_GFX;
	uint32_t r, t = 0x00010000;			//avoid second row rolling unto first
	
	for (r = 0; r < 32; r += 2, screenData += 4)
	{
		asm(							//carry bits help a lot, but we could do this in C anyways
			"rev  %0, %0		\n\t"
			"rev  %1, %1		\n\t"
			"rev  %2, %2		\n\t"
			"lsl  %2, #1		\n\t"
			"adc  %1, %1		\n\t"
			"bic  %1, %3		\n\t"
			"adc  %0, %0		\n\t"
			"rev  %0, %0		\n\t"
			"rev  %1, %1		\n\t"
			"rev  %2, %2		\n\t"
			:"=r"(screenData[0]), "=r"(screenData[1]), "=r"(screenData[2]), "=r"(t)
			:"0"(screenData[0]), "1"(screenData[1]), "2"(screenData[2]), "3"(t)
			:"cc"
		);
	}
}

uint32_t rnd32(void)	//xorshift128plus
{
	static uint64_t mRndState[2] = {0, 1};
	uint64_t ret, x, y;
	
	x = mRndState[0];
	y = mRndState[1];
	
	mRndState[0] = y;
	x ^= x << 23; // a
	mRndState[1] = x ^ y ^ (x >> 17) ^ (y >> 26); // b, c
	ret = mRndState[1] + y;
	
	return ((uint32_t)(ret >> 32)) ^ (uint32_t)ret;
}



static void __attribute__((naked)) appExit(void)
{
	asm volatile(".hword 0xDE08");
}

static void clearScreen(void)
{
	volatile uint32_t *screenData = (volatile uint32_t*)VMU_BASE_ADDR_GFX;
	uint32_t r;
	
	for (r = 0; r < 32; r += 2) {
		
		*screenData++ = 0;
		*screenData++ = 0;
		*screenData++ = 0;
		*screenData++ = 0;
	}
}

void main(void)
{
	while(1){
		
		const uint32_t obstacleWidth = 6, obstacleSpacing = 25, obstaHoleSz = 20;
		int32_t birdR = 0, birdDy = 0, i, score = 0;
		uint32_t obstacleTimer = 0, obstaPos;
		bool obstacleDrawing = false;
		
		
		clearScreen();
		
		//draw initial bird
		drawFlappyFrame(birdR, true);
		
		while(1) {
			int32_t newR, input;
			
			//advance screen
			slideScreenLeft();
			
			
			//handle obstacles
			if (obstacleDrawing) {
				volatile uint8_t *ptr = ((volatile uint8_t*)VMU_BASE_ADDR_GFX) + 5;
				uint32_t r;
	
				//draw obstacle in new clear col
				for (r = 0; r < 32; r++, ptr += 6) {
					
					if (r < obstaPos || r >= obstaPos + obstaHoleSz)
						*ptr ^= 1;
					
					if (r & 1)
						ptr += 4;
				}
				
				
				if (++obstacleTimer == obstacleWidth) {
					obstacleDrawing = false;
					obstacleTimer = 0;
				}
			}
			else if (++obstacleTimer == obstacleSpacing) {
				
				obstacleDrawing = true;
				obstacleTimer = 0;
				obstaPos = rnd32() % (32 - obstaHoleSz - 2) + 1;	//to make sure at least one pixel exists on top and bottom
			}
			
			//copy bird coords
			newR = birdR;
		
			//handle input
			SFR->P3 = 0xFF;
			input = ~SFR->P3;
			if (input & 1)
				birdDy -= 2;
			if (input & 0x40)
				appExit();
	
			//calcs for bird
			birdDy++;
			if (birdDy > 5)
				birdDy = 5;
				
			newR = birdR + birdDy;
			if (newR < 0)
				newR = 0;
			if (newR >= 18)
				newR = 18;
		
			//erase bird in old location (account for screen shift)
			drawFlappyFrame(birdR, false);
			
			//draw bird in new location
			if (drawFlappyFrame(newR, true))
				break;
			
			//score keep
			if ((((uint8_t*)VMU_BASE_ADDR_GFX)[0] & 0xC0) == 0x80)	//if the post we passed is disappaering from screen, add score
				score++;
			
			birdR = newR;
		}
		
		clearScreen();
		mScreenC = 0;
		mScreenR = 0;
		prScreen(" GAME  OVER SCORE: %u", score);
		do{
			SFR->P3 = 0xFF;
		} while (SFR->P3 != 0xFF);
		do{
			SFR->P3 = 0xFF;
		} while (SFR->P3 == 0xFF);
	}
	
	//exit
	appExit();
}
#include "mylib.h"
#include "mainpage.h"
#include "ironman.h"
#include "defeated.h"
#include "text.h"
#include "not_working.h"
#include "instruction.h"
u16 *videoBuffer= (u16 *)0x6000000;

void setPixel(int r, int c, u16 color)
{
	videoBuffer[r*240 + c] = color;
}
void waitForVBlank()
{
    while(SCANLINECOUNTER > 160);
    while(SCANLINECOUNTER < 160);
}

void drawImage3(int r, int c, int width, int height, const u16 *image) {

	// TODO Draw the image with the top left corner at (x,y).
	// Recall that the GBA screen is 240 pixels wide, and 160 tall.
	// The videoBuffer is a 1-dimensional array of these pixels.
	// For example, to set pixel (37,21) to white:
	//   videoBuffer[21 * 240 + 37] = 0x7FFF;
	for (int h_counter=r;h_counter<r+height;++h_counter){
		DMA[3].src=image+(h_counter-r)*width;
		DMA[3].dst=videoBuffer+(h_counter)*240+c;
		DMA[3].cnt=width | DMA_ON | DMA_SOURCE_INCREMENT;
		
	}	

}

void drawRect(int row, int col, int height, int width, u16 color)
{
	for (int h_counter=0;h_counter<height;++h_counter){
		DMA[3].src=&color;
		DMA[3].dst=videoBuffer+(h_counter+row)*240+col;
		DMA[3].cnt=width | DMA_ON | DMA_SOURCE_FIXED;
		
	}	
}


void drawStartScreen()
{
	drawImage3(0,0,MAINPAGE_WIDTH,MAINPAGE_HEIGHT,(const u16*)mainpage);
	drawString(120,70,"PRESS START", BLACK);	
}
void drawGameOverScreen()
{
	drawImage3(0,0,DEFEATED_WIDTH,DEFEATED_HEIGHT,(const u16*)defeated);
	
}
void drawNotWorkingScreen(){
	drawImage3(0,0,NOT_WORKING_WIDTH,NOT_WORKING_HEIGHT,(const u16*)not_working);
	drawString(50,5,"Jarvis: I", BLACK);
	drawString(60,5,"am afraid", BLACK); 
	drawString(70,5,"it isn't",BLACK);
	drawString(80,5,"working,", BLACK);
	drawString(90,5,"Sir.", BLACK);
	drawString(70,150,"PRESS ENTER", BLACK);
	drawString(80,150,"TO CONTINUE", BLACK);

}

void drawInstructionScreen(){
	drawImage3(0,0,INSTRUCTION_WIDTH,INSTRUCTION_HEIGHT,(const u16*)instruction);
	drawString(20,0,"Iron man is in Trouble", WHITE);
	drawString(30,0,"His attack/defensive system", WHITE);
	drawString(40,0,"Stopped working!", WHITE);
	drawString(50,0,"Escape from these deadly missiles!", WHITE);
	drawString(60,0,"Homing Missiles", WHITE);
	drawString(80,0,"Instruction", WHITE);
	drawString(90,0,"Arrow Key moves Iron Man", WHITE);
	drawString(130,0,"PRESS ENTER TO CONTINUE", WHITE);


}

void boundsCheck(int *value, int lowerbound,int upperbound, int *delta)
{

	if(*value<lowerbound)
	{
		*value = lowerbound;
		*delta = -*delta;
	}
	if(*value>upperbound)
	{
		*value = upperbound;
		*delta = -*delta;
	}
	
}
		

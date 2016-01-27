//Yeonjoon Choi

#include <stdlib.h>
#include <stdio.h>
#include "mylib.h"
#include "text.h"
#include "ironman.h"
#include "life.h"
#define REG_DISPCNT *(u16*) 0x4000000
#define BG2_EN (1<<10)
#define MODE_3 3


//player
typedef struct {
	int row;
	int col;
	int width;
	int height;
	int vel;
	const u16* image;
}Player;

//missiles
typedef struct {
	int row;
	int col;
	int size;
	int rdel;
	int cdel;
	int alive;
}Missile;

//lives
typedef struct {
	int row;
	int col;
	int width;
	int height;
	const u16* image;
	int visible;
}Arc;

//game boy states
enum GBAState{
	START,
	START_DUMMY,
	NOT_WORKING,
	NW_DUMMY,
	INSTRUCTION,
	INSTRUCTION_DUMMY,
	GAME,
	GAME_DUMMY,
	GAMEOVER,
	GAMEOVER_DUMMY
};

//function  prototypes
extern u16* videoBuffer;
extern void setPixel(int r, int c, u16 color);
extern void drawChar(int row, int col, char ch, u16 color);
extern void drawString(int row, int col, char *s, u16 color);
extern void drawImage3(int r, int c, int width, int height, const u16* image);
extern void drawStartScreen();
extern void waitForVBlank();
extern void drawGameOverScreen();
extern void drawRect(int row, int col, int height, int width, u16 color);
extern void drawInstructionScreen();
extern void drawNotWorkingScreen();
extern void boundsCheck(int *value, int lowerbound, int upperbound, int *delta);
int collisionCheck(Player* p, Missile* m);
int missileCheck(Missile* p, Missile* m1 );


int main() {

	REG_DISPCNT = BG2_EN | MODE_3;
	enum GBAState state = START;
	char buffer[41];
	char buffer1[41];
	int score = 0;
	int time=0;
	unsigned short held=0;
	int left=0;
	int highscore=0;
	Player player; 
	player.row=70;
	player.col=108;
	while(1){
		waitForVBlank();
		
		switch(state){
		case START:
			drawStartScreen();
			state=START_DUMMY;
			break;
		case START_DUMMY:
			if(KEY_DOWN_NOW(BUTTON_START)&& !(held & BUTTON_START))
				state=NOT_WORKING;
			break;
		case NOT_WORKING:
			drawNotWorkingScreen();
			state=NW_DUMMY;
			break;
		case NW_DUMMY:
			if(KEY_DOWN_NOW(BUTTON_START)&& !(held & BUTTON_START))
				state=INSTRUCTION;
			if(KEY_DOWN_NOW(BUTTON_SELECT)&& !(held & BUTTON_SELECT))
				state=START;
			break;
		case INSTRUCTION:
			drawInstructionScreen();
			state=INSTRUCTION_DUMMY;
			break;
		case INSTRUCTION_DUMMY:
			if(KEY_DOWN_NOW(BUTTON_START)&& !(held & BUTTON_START))
				state=GAME;
			if(KEY_DOWN_NOW(BUTTON_SELECT)&& !(held & BUTTON_SELECT))
				state=START;
			break;
		case GAME:
			//initialize the game
			drawRect(0,0,160,240,WHITE); //background
			//initialize time, score, left
			drawRect(0,60,20,180,RED);
			drawString(0,90,"PRESS SELECT TO RETURN",YELLOW);
			drawString(10,90,"TO THE START SCREEN",YELLOW);		
			time=0; 
			score=0;
			left=3;
			//initialize player
			player.row=70;
			player.col=108;
			player.width=IRONMAN_WIDTH;
			player.height=IRONMAN_HEIGHT;
			player.vel=4;
			player.image = (const u16*)ironman;
			Player oldPlayer=player;
			Missile missile[4];
			Missile oldMissile[4];
			Arc arc[3];
			Missile *cur;
			//initialize missiles
			for(int i=0; i<4; i++){
				missile[i].row =30;
				missile[i].col =30+i*30;
				missile[i].rdel =0.5;
				missile[i].cdel =1;
				missile[i].size =4;
				missile[i].alive=0;
				oldMissile[i] = missile[i];
			}
			//initialize lifes
			int spacing=LIFE_WIDTH;
			for(int i=0; i<3; i++){
				arc[i].row =0;
				arc[i].col =0+i*spacing;
				arc[i].width =LIFE_WIDTH;
				arc[i].height =LIFE_HEIGHT;
				arc[i].image = (const u16*)life;
				arc[i].visible=1;
			}
			//move to next state
			state=GAME_DUMMY;			
			break;
		case GAME_DUMMY:
			time++;
			//delete previous player
			drawRect(player.row,player.col,player.height,player.width,WHITE);
			//player moves depends on the velocity 
			if(KEY_DOWN_NOW(BUTTON_RIGHT))
				player.col+=player.vel;		
				if(player.col<0){		
					player.col=0;
				}else if(player.col>220){
					player.col=220;
				}		
			if(KEY_DOWN_NOW(BUTTON_LEFT))
				player.col-=player.vel;
				if(player.col<0){
					player.col=0;
				}else if(player.col>220){
					player.col=220;
				}
			if(KEY_DOWN_NOW(BUTTON_UP))
				player.row-=player.vel;
				if(player.row<21){
					player.row=21;
				}else if(player.row>124){
					player.row=124;
				}
			if(KEY_DOWN_NOW(BUTTON_DOWN))
				player.row+=player.vel;
				if(player.row<21){
					player.row=21;
				}else if(player.row>124){
					player.row=124;
				}

			for(int i=0; i<4; i++){	
				cur = missile + i;
				if((player.row+12)-(cur->row)>0) 
					// if center of the player row - missile.row  is positive
					cur->rdel=((player.row+12)-(cur->row))/((player.row+12)-(cur->row)); 
				else 
					// if center of the player row - missile.row  is negative, put negative 
					cur->rdel=((player.row+12)-(cur->row))/(-1*((player.row+12)-(cur->row))); 
				if((player.col+10)-(cur->col)>0) 
					// if center of the player col - missile.col  is positive 
					cur->cdel=((player.col+10)-(cur->col))/((player.col+10)-(cur->col));
				else 
					// if center of the player col - missile.col  is negative, put negative
					cur->cdel=((player.col+10)-(cur->col))/(-1*((player.col+10)-(cur->col)));

				cur->row += cur->rdel;
				cur->col += cur->cdel;
			} 
			//for alive missiles get rid of old missile points
			for(int i = 0; i<4; i++){
				if(missile[i].alive==1)
					drawRect(oldMissile[i].row, oldMissile[i].col, 4, 4, WHITE);
			}
			for(int i = 0; i<4; i++){
				cur = &missile[i];
				Missile* temp;

				//possible colision cases
				//case 1 : missile[0] missile[1]
				//case 2 : missile[0] missile[2]
				//case 3 : missile[0] missile[3]
				//case 4 : missile[1] missile[2]
				//case 5 : missile[1] missile[3]
				//case 6 : missile[2] missile[3]

				switch(i){
					case 0 : //case 1
							temp=&missile[1] ;
							if(missileCheck(cur, temp)==1)

							//case 2
							temp=&missile[2] ;
							if(missileCheck(cur, temp)==1)

							//case 3	
							temp=&missile[3] ;
							if(missileCheck(cur, temp)==1)

							break;
					case 1 : 
							//case 4							
							temp=&missile[2] ;
							if(missileCheck(cur, temp)==1)
							
							//case 5
							temp=&missile[3] ;
							if(missileCheck(cur, temp)==1)
							break;
					case 2 : 
							//case 6
							temp=&missile[3] ;
							if(missileCheck(cur, temp)==1)
							break;
				}

				if(collisionCheck(&player, cur)==1){
					if(left>0)
						left--; // decrease life left
					else
						left=0; 
				}
				if(cur->alive==1)
					drawRect(cur->row, cur->col, 4, 4, BLUE); 
				oldMissile[i] = missile[i];
				boundsCheck( &(cur->row),20+4 ,150-4, &(cur->rdel)); //make sure it does not go off the bound
				boundsCheck( &(cur->col),4 ,240-4, &(cur->cdel)); //make sure it does not go off the bound
							
			}	


			if(time%60==0){
				score++;
			}
			if(time%60==0){  //for every i seconds  if missile[0] is not alive make missile
				if(missile[0].alive==0){
					missile[0].alive=1;
					missile[0].row=20;
					missile[0].col=rand()%236;
				}
			}
			if(time%180==0){ //for every 3 seconds  if missile[1] is not alive make missile
				if(missile[1].alive==0){
					missile[1].alive=1;
					missile[1].row=146;
					missile[1].col=rand()%236;
				}
			}

			if(time%300==0){ //for every 6 seconds  if missile[2] is not alive make missile
				if(missile[2].alive==0){
					missile[2].alive=1;
					missile[2].row=rand()%146+20;
					missile[2].col=0;
				}
			}

			if(time%420==0){  //for every 9 seconds  if missile[3] is not alive make missile
				if(missile[3].alive==0){
					missile[3].alive=1;
					missile[3].row=rand()%146+20;
					missile[3].col=236;
				}
			}
			if(left<3)
				arc[left].visible=0;

			for(int i=0; i<3; i++){
				if(arc[i].visible==1)	
					drawImage3(arc[i].row,arc[i].col,arc[i].width,arc[i].height,arc[i].image);
				else
					drawRect(arc[i].row,arc[i].col,arc[i].height,arc[i].width,RED);
					
			}				
			drawImage3(player.row,player.col,player.width,player.height,player.image);			
			sprintf(buffer, "Score:%d", score);
			drawRect(150, 0, 10, 240, RED);
			drawString(150, 5, buffer, YELLOW);
			oldPlayer=player;

			if(KEY_DOWN_NOW(BUTTON_SELECT)&& !(held & BUTTON_SELECT))
				state=START;

			if(left==0)
				state=GAMEOVER;
			break;
		case GAMEOVER:
			drawGameOverScreen();
			if(highscore<score){
				highscore=score;
				drawString(50, 40, "NEW HIGH SCORE",BLACK);
			}else{
				drawString(50, 40, "Beat The High Score",BLACK);
			}
			drawString(20, 40, "GAME OVER", RED);
			drawString(30, 40, buffer, BLUE);
			sprintf(buffer1, "High Score:%d", highscore);
			drawString(40, 40, buffer1,RED);
			state=GAMEOVER_DUMMY;
			break;
		case GAMEOVER_DUMMY:
			if(KEY_DOWN_NOW(BUTTON_SELECT)&& !(held & BUTTON_SELECT))
				state=START;
			break;
		}
		held=~BUTTONS;

	}

	return 0;
}

int collisionCheck(Player* p, Missile* m ){
	if((m->alive==1) && (m->col>=p->col) && (m->col<=p->col+p->width) && (m->row>=p->row) && (m->row<=p->row+p->height)){
		m->alive=0;
		return 1;
	}if((m->alive==1) && (m->col+m->size>=p->col) && (m->col+m->size<=p->col+p->width) && (m->row>=p->row) && (m->row<=p->row+p->height)){
		m->alive=0;
		return 1;
	}if((m->alive==1) && (m->col>=p->col) && (m->col<=p->col+p->width) && (m->row+m->size>=p->row) && (m->row+m->size<=p->row+p->height)){
		m->alive=0;
		return 1;
	}if((m->alive==1) && (m->col+m->size>=p->col) && (m->col+m->size<=p->col+p->width) && (m->row+m->size>=p->row) && (m->row+m->size<=p->row+p->height)){
		m->alive=0;
		return 1;
	}
	return 0;
}


int missileCheck(Missile* p, Missile* m ){

	if((m->alive==1)&&(p->alive==1) && (m->col>=p->col) && m->col<=(p->col+p->size)-1 && (m->row>=p->row) && (m->row)<=(p->row+p->size)-1 ){
		m->alive=0;
		p->alive=0;
		return 1;
	}if(m->alive==1&&p->alive==1&& (m->col+m->size)-1>=p->col && (m->col+m->size)-1<=(p->col+p->size)-1 && m->row>=p->row && m->row<=(p->row+p->size)-1 ){
		m->alive=0;
		p->alive=0;
		return 1;
	}if( m->alive==1 && p->alive==1 && m->col>=p->col && m->col<=(p->col+p->size)-1 && (m->row+m->size)-1>=p->row && (m->row+m->size)-1<=(p->row+p->size)-1){
		m->alive=0;
		p->alive=0;
		return 1;
	}if((m->alive==1)&&(p->alive==1) && ((m->col+m->size)-1>=p->col) && (m->col+m->size)-1<=(p->col+p->size)-1 && (m->row+m->size)-1>=p->row && (m->row+m->size)-1<=(p->row+p->size)-1 ){
		m->alive=0;
		p->alive=0;
		return 1;
	}
	return 0;
}
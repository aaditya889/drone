#include<reg52.h>
#include<lcd.h>
/*MODS
1. if(water) added in while(water) loop at the end. ---------------------------------------------------------------------------------------------------------
2. x = 0; added in while(!water) loop at the beginning.---------------------------------------------------------------------------------------------------------------
3. && (uplevstop && downlevstop) added in while(!water) loop.---------------------------------------------------------------------------------------------------------------
4. backdelay2(20) deleted in while(water) loop at the end(when switching off down pump due to unavailibility of water). -------------------------------------
5. P1 = 0xff in init(). ------------------------------------------------------------------------------------------------------------------
*/


sbit led=P2^4;
sbit uplevstart=P1^0;
sbit uplevstop=P1^1;
sbit downlevlow=P1^2;
sbit downlevstop=P1^3;
sbit downlevstart=P1^4;
sbit downpump=P2^0;
sbit uppump=P2^4;
sbit water=P1^5;

bit var=0,var2=0;
unsigned char var3=0;
unsigned int t0,t1,t2;

// check downpump(front) for water.
// try to make uplevstart and downlevstart as interrupts.
//upstrtrchd and upstrchd are variables that are set to 1 if level reaches uplevstart and uplevstop respectively, even once.
//var is set to 1 when backdelay() is running and cleared when finished or never run.

void secdelay(){									//rectify for 1 sec.
	unsigned int i;
	unsigned char j;
	for(j=0;j<5;j++)
	for(i=0;i<1250;i++){}
}

void shdelay(){									//rectify for 1 sec.
	unsigned int i;
	unsigned char j;
	for(j=0;j<1;j++)
	for(i=0;i<10000;i++){}
}

void external0 (void) interrupt 0 {
	IE0 = 0;
}

void external1 (void) interrupt 2 {
	IE1 = 0;
}

void timer0 (void) interrupt 1 {
	TF0=0;
	if(t0!=0)
		t0--;
	else {
		var=0;
		TR0=0;
	}
}

void timer1 (void) interrupt 3 {
	TF1=0;
	if(t1!=0)
		t1--;
	else {
		var2=0;
		TR1=0;
	}
}

void timer2 (void) interrupt 5 {
	TF2=0;
	if(t2!=0)
		t2--;
	else {
		var3=2;
		TR2=0;
	}
}

void backdelay(unsigned char del){
	if(var)
		return;
	
	t0=10*del;		//actual number = 1860.
	var=1;
	TH0=0;
	TL0=0;
	TR0=1;
}
	
void backdelay2(unsigned char del){
	if(var2)
		return;
	
	t1=10*del;		//actual number = 1860.
	var2=1;
	TH1=0;
	TL1=0;
	TR1=1;
}

void backdelay3(unsigned char del){
	if(var3)
		return;
	
	t2=10*del;			//actual number = 1860.
	var3=1;
	TH2=0;
	TL2=0;
	TR2=1;
}


void contrl_init(){
	int i;
	for(i=0;i<20;i++){
		led=~led;
		shdelay();
	}
}

void init(){
	uppump=downpump=0;
	P1=0xff;
	TMOD = 17;
	EA=1;
	ET0=ET1=ET2=1;
	IT0=IT1=1;
}


void bothstart(){	//note: uplevstart IS needed in this function for when water fills up once and then decreases in uptank.
	unsigned char i,j,x=0,y=6;
	prtclr("bothstart");
	//delay(5);
	while(uplevstop || downlevstop){
		prtclr("insideloop");
		//delay(5);
		LCD_clr();
		//conditions for starting downpump ----------------------------------------------
		if(!var2 && downlevstart){
			downpump = 1;
			prtclr("run down1!");
			//delay(50);
		}
		//conditions for starting downpump till here ----------------------------------------------
		while(water){
			prtclr("inside water!");
			//delay(20);
			//conditions for starting downpump ----------------------------------------------------------------
			if(x>=y && !var2 && downlevstart){		//var2 is for 20 min delay for downpump.
				downpump = 1;
				x=0;
				prtclr("run down2!");
				//delay(50);
			}
			//conditions for starting downpump till here ----------------------------------------------------------------
			
			for(i=0;i<27;i++){
				if(!water) break;
				for(j=0;j<10;j++){
					
					//conditions for starting upmpump -----------------------------------------------------
					if(!downlevlow && !var && uplevstop) {
						uppump = 1;
						y = 6;
						prtclr("run up1!");
						//delay(50);
					}					
					//condtitions for starting till here --------------------------------------------------
					
					//conditions for stopping uppump -------------------------------------------------------
					if(!uplevstop){
						uppump = 0;
						backdelay(20);		//in case of controller error atleast 20 min will be in between switching		
						prtclr("stop up99!");
						//delay(50);						
					}
					if(downlevlow){
						uppump = 0;
						backdelay(20);
						y=4;
						prtclr("stop up89!");
						//delay(50);						
					}
					//conditions for stopping uppump till here -------------------------------------------------------
					
					if(!water) break;
					secdelay();
				}
			}
			//conditions for stopping downpump ----------------------------------------------------------------
			if(water){
			downpump=0;
			prtclr("not water!");
			x++;
			}
			//conditions for stopping downpump till here ----------------------------------------------------------------	
		}
		
		while(!water && (uplevstop && downlevstop)){
			x=0;
			//conditions for uppump will be the same...
			prtclr("inside !water!");
			delay(20);
					//conditions for starting upmpump -----------------------------------------------------
					if(!downlevlow && !var && uplevstop){	//uplevstop used as water should be filled to the top, and var will ensure 20 min delay.
						uppump = 1; 				
						y = 6;
					}
					//condtitions for starting till here --------------------------------------------------
					
					//conditions for stopping uppump -------------------------------------------------------
					if(!uplevstop){
						uppump = 0;
						backdelay(20); //in case of controller error atleast 20 min will be in between switching
					}
					if(downlevlow){
						uppump = 0;
						backdelay(20);
						y=4;
					}
					//conditions for stopping uppump till here -------------------------------------------------------
			
					//conditions for stopping downpump ----------------------------------------------------------------
					if(!downlevstart)
						backdelay3(15);	//backdelay3 is for when the level crosses the incoming water pipe level. It uses var3(unsigned char not bit).
					if(!downlevstop || var3==2){
						downpump = 0;
						backdelay2(20);			//backdelay2() is for downpump.
						var3=0; //var3 = 1(already running), 0(can run again or never ran and can run), 2(ran once and finished, need to be set to 0).
					}
					//conditions for stopping downpump till here ----------------------------------------------------------------
					
		}
	}
}



void main(){
	//write interrupt vectors for external interrupts.
	contrl_init();
	init();
	LCD_init();
	prtclr("initialising!");
	delay(50);
	while(1){
		prtclr("going to sleep!");
		delay(50);
		bothstart();
	//	PCON |= 0X02;		//enter power down mode.
	}
}









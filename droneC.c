#include<reg51.h>
#include<math.h>
#define PI 3.14159265358979323846

sbit led=P2^4;
sbit SDA=P0^0;
sbit SCL=P0^1;
#define dat P3
sbit rs=P2^7;
sbit rw=P2^6;
sbit en=P2^5;
sbit d0=P3^0;
sbit d1=P3^1;
sbit d2=P3^2;
sbit d3=P3^3;
sbit d4=P3^4;
sbit d5=P3^5;
sbit d6=P3^6;
sbit d7=P3^7;

void setcursor(char, char);
unsigned char lcdptr=0;

void delay(unsigned int x){
	int i,j;
	for(i=0;i<1250;i++){
		for(j=0;j<x;j++){
		}
	}
}

void contrl_init(){
	int i;
	for(i=0;i<20;i++){
		led=~led;
		delay(1);
	}
}

void LCD_busy()
{
			do{
			en = 0;
			rs = 0;
			rw = 1;
			dat= 0xff;           //Make D7th bit of LCD as i/p
			en   = 1;           //Make port pin as o/p
     
			}while(d7);
			en = 0;
			rw = 0;
	//delay(5);
} 


void LCD_cmd(unsigned char var)
{
     en   = 0;
     rs   = 0;        //Selected command register
     rw   = 0;        //We are writing in instruction register
		 dat = var;      //Function set: 2 Line, 8-bit, 5x7 dots
     en   = 1;        //Enable H->
		 en		=	0;
     LCD_busy();          //Wait for LCD to process the command
	
}


void LCD_init()
{
	
			LCD_cmd(0x0f);	 
	
			LCD_cmd(0x38);
	
			LCD_cmd(0x0e);
	
			LCD_cmd(0x06);
	
			LCD_cmd(0x01);
}

void LCD_clr(){
	LCD_cmd(0x01);
	lcdptr=0;
}


void LCD_senddata(unsigned char var)
{
		 en   = 0;
     rs   = 1;        //Selected dat reg
     rw   = 0;        //We are writing
     dat = var;      //Function set: 2 Line, 8-bit, 5x7 dots
     en   = 1;        //Enable H->
		 en		=	0;
     LCD_busy();          //Wait for LCD to process the command
		 lcdptr++;
		 if(lcdptr==15)
			 setcursor(2,0);

}

void printnum(signed int x){
	int i[5],n=0,j;
	unsigned char t[]="0123456789";
	if(x<0){
		x*=-1;
		LCD_senddata('-');
	}
	do{
		i[n++]=x%10;
		x/=10;
	}while(x>0);
	for(j=n-1;j>=0;j--)
		LCD_senddata(t[i[j]]);	
	
}


void printflt(double x){
	unsigned int y;
	unsigned char t[2],z[]="0123456789";
	if(x<0){
		x*=-1;
		LCD_senddata('-');
	}
	printnum((int) x);
	LCD_senddata('.');
	y=(x*100);
	t[1]=y%10;
	y/=10;
	t[0]=y%10;
	LCD_senddata(z[t[0]]);
	LCD_senddata(z[t[1]]);

}


void prt(char *form,...){
	char *x=form,i=0;
	while(*x){
		if(*x=='%'){
			x++;
			if(*x=='d'){
				printnum(*((int*)(ellipsis_15+i)));
				i+=sizeof(int);
			}				
			else if(*x=='f'){
				printflt(*((float *)(ellipsis_15+i)));
				i+=sizeof(double);
			}
		}
		else LCD_senddata(*x);
		x++;
		}
}

void prtclr(char *form,...){
	char *x=form,i=0;
	LCD_clr();
	while(*x){
		if(*x=='%'){
			x++;
			if(*x=='d'){
				printnum(*((int*)(ellipsis_15+i)));
				i+=sizeof(int);
			}				
			else if(*x=='f'){
				printflt(*((float *)(ellipsis_15+i)));
				i+=sizeof(double);
			}
		}
		else LCD_senddata(*x);
		x++;
		}
}

void setcursor(char a, char b)
{
	if(a == 1)
	  LCD_cmd(0x80 + b);
	else if(a == 2)
		LCD_cmd(0xC0 + b);
}

void LCD_shiftleft(){
	LCD_cmd(0x10);
}

void LCD_shiftright(){
	LCD_cmd(0x14);
}


//I2C CODE STARTING HERE! ------------------------------------------------------------------------------------------------------
 
void iicinit()
{
	SDA = 1;
	SCL = 1;
}
 
void iicstart()
{
	SDA = 1;  
	SCL = 1;
	SDA = 0;
	SCL = 0;
}
 
void iicrestart()
{
	SDA = 1;
	SCL = 1;
	SDA = 0;
	SCL = 0;
}
 
void iicstop()
{
	SCL = 0;
	SDA = 0;
	SCL = 1;
	SDA = 1;
}
 
void iicack()
{
	SDA = 0;
	SCL = 1;
	SCL = 0;
	SDA = 1;
}
 
void iicnak()
{
	SDA = 1;
	SCL = 1;
	SCL = 0;
	SDA = 1;
}
 
unsigned char iicsend(unsigned char Data)
{
	 unsigned char i, ack_bit;
		SCL = 0;
	 for (i = 0; i < 8; i++) {
		if ((Data & 0x80) == 0)
			SDA = 0;
		else
			SDA = 1;
		SCL = 1;
	 	SCL = 0;
		Data<<=1;
	 }
	 SDA = 1;
	 SCL = 1;
	 ack_bit = SDA;
	 SCL = 0;
	 return ack_bit;
}
 
unsigned char iicread()
{
	unsigned char i, Data=0;
		SCL = 0;
	for (i = 0; i < 8; i++) {
		Data<<=1;
		SCL = 1;
		if(SDA)
			Data |=1;
	SCL = 0;
	}
	return Data;
}


char mpuread(unsigned char x){
	unsigned char ack=1;
	char da;
	iicstart();
	
	ack=iicsend(208);						//ad+w = 208 ad+r = 209!
	/*dispclrwt("sending data!");
	if(!ack)
		dispclrwt("sent!");	
	else dispclrwt("not sent!");
	*/
	ack=iicsend(x);
	/*dispclrwt("sending data 2!");
	if(!ack)
		dispclrwt("sent!");
	else dispclrwt("not sent!");
	*/
	iicstart();
	ack=iicsend(209);						//ad+r=209
	/*dispclrwt("sending data 3!");
	if(!ack)
		dispclrwt("sent!");
	else dispclrwt("not sent!");
	*/
	da=iicread();
	iicnak();
	iicstop();
	return da;
}

unsigned char mpuwrite(unsigned char reg,unsigned char da){
	unsigned char ack=1;
	iicstart();
	ack=iicsend(208);
	/*
	if(!ack)
		dispclrwt("sent 1!");
	else dispclrwt("not sent!");
	*/
	ack=iicsend(reg);
	/*
	if(!ack)
		dispclrwt("sent 2!");
	else dispclrwt("not sent!");
	*/
	ack=iicsend(da);
	/*
	if(!ack)
		dispclrwt("sent 3!");
	else dispclrwt("not sent!");
	*/
	iicstop();
	return ack;
	
}

int burstread(unsigned char reg){
	unsigned char ack=1,temp;
	int dat;
	iicstart();
	ack=iicsend(208);							//ad+w=208
	iicsend(reg);
	iicstart();
	ack=iicsend(209);							//ad+r=209
	dat=iicread();
	iicack();
	temp=iicread();
	iicnak();
	iicstop();
	dat=dat<<8;
	dat=dat|temp;
	return dat;
	
	
}
	
	
	
//END I2C CODE! ------------------------------------------------------------------------------------------------------


void init_all(){
	contrl_init();
	LCD_init();
	iicinit();
	if(mpuwrite(107,0)){
		prtclr("error in reg %d!" ,107);
		delay(100);
	}
	/*
	if(mpuwrite(104,7)){
		prtclr("error in reg %d!" ,104);
		delay(100);
	}
	*/
	if(mpuwrite(26,3)){
		prtclr("error in reg %d!" ,26);
		delay(100);
	}
	if(mpuwrite(27,16)){
		prtclr("error in reg %d!" ,27);
		delay(100);
	}
	if(mpuwrite(28,16)){
		prtclr("error in reg %d!" ,28);
		delay(100);
	}
}
	

//MAIN FUNCTION ------------------------------------------------------------------------------------------------------
void main(){
	
	double accx,accy,accz,time,gyrx;
	int tim1;
	init_all();
	prtclr("initialising!");
	delay(30);
	TMOD=0X01;
	TL0=TH0=0;
//Check i2c communication.
	prtclr("address=%d" ,(int)(mpuread(117)));
	delay(30);
	TR0=1;
	TF0=0;
	
	while(1){
		TR0=1;
		TL0=TH0=0;
		
		accx=(asin(burstread(59)/4096.0))*(180.0/PI);
		accy=(asin(burstread(61)/4096.0))*(180.0/PI);
		accz=(asin(burstread(63)/4096.0))*(180.0/PI);
		
		prtclr("%f %f    " ,accx,accy);
		TR0=0;
		tim1=TH0;
		tim1=tim1<<8;
		tim1|=TL0;
		time=(tim1*0.9216)/1000000.0;
		prt("%f t=%f" ,accz ,time);
		delay(10);
	}

}
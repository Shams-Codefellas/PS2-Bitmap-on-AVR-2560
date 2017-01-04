/* author - shams */
/*
Bit Map

Data[0]=0b11111100  default when nothing is pressed.

Right=10011100
Left=01111100
Down=00111100
Up=11001100
Left_Joystick_press=11111000
Right_Joystick_press=11110000

Data[1]=0xff  default when nothing is presses.

X=00111111
Square=01111111
Triangle=11001111
O=10011111
R1=11100111
R2=11111001
L1=11110011
L2=11111000
*/
#ifndef F_CPU
#define F_CPU 8000000UL // or whatever may be your frequency
#endif

#include <avr/io.h>
#include <avr/interrupt.h>
#include <util/delay.h>

short int data[2];
#define PSdata             1 //PC1
#define PScommand          2 //PC2
#define PSclock            7 //PC7
#define PSattention        6 //PC6
int i;

#define sbi(x,y)  x|=(1<<y)  // sets the y(name of bit is y), in the register x.
#define cbi(x,y)  x&=~(1<<y) //clears the y(name of bit is y), in the register x.

void pwm_init()
{
    // initialize TCCR0A as per requirement, say as follows
    TCCR0A = 0x83;  //Fast pwm, no prescaling, clear oco on match ,non-inverted counting.
    TCCR2A = 0x83;  //Fast pwm, no prescaling, clear oco on match ,non-inverted counting.
  
    // make sure to make OC0 pin (pin PB3 for atmega32) as output pin
    DDRB |= (1<<PB7);
    DDRB |= (1<<PB4);
    
}

void init_PS2() {  //basically initalising to set pin no,s for interaction and the procedure of getting the calls.
	
	sbi(DDRC, PC7);
	cbi(DDRC, PC1);   //all say something a , so probably they are analog pins.
	sbi(PORTC, PC1);
	cbi(DDRC, PC3);
	sbi(PORTC, PC3);
	sbi(DDRC, PC2);
	sbi(DDRC, PC6);
	int_PS2inanalougemode();
}

int gameByte(short int command) {
	short int i ;
	_delay_us(1);
	short int data = 0x00;
	for(i = 0;i < 8;i++) {
		if(command & _BV(i))
			sbi(PORTC, PScommand);
		else
			cbi(PORTC, PScommand);          //gameByte is the function that returns the other side of the duplex transmission.
		cbi(PORTC, PSclock);
		_delay_us(1);
		if((PINC & _BV(PSdata)))
			sbi(data, i);
		else
			cbi(data, i);
		sbi(PORTC, PSclock);
	}
	sbi(PORTC, PScommand);
	_delay_us(20);
	return(data);
}

void  int_PS2inanalougemode() {
	unsigned char chk_ana = 0, cnt = 0;
	while(cnt < 25) {
		sbi(PORTC, PScommand);  //pscommand is 2.
		sbi(PORTC, PSclock);    //ps clock is 7
		cbi(PORTC, PSattention);  //ps attention is 6
		gameByte(0x01);
		gameByte(0x43);          //go into config mode.
		gameByte(0x00);
		gameByte(0x01);
		gameByte(0x00);
		sbi(PORTC, PScommand);       
		_delay_ms(1);
		sbi(PORTC, PSattention);
		_delay_ms(10);
		sbi(PORTC, PScommand);   //why 2nd time set bit ??
		sbi(PORTC, PSclock);
		cbi(PORTC, PSattention);
		gameByte(0x01);
		gameByte(0x44);   	//turn on analog mode.
		gameByte(0x00);
		gameByte(0x01);
		gameByte(0x03);
		gameByte(0x00);
		gameByte(0x00);
		gameByte(0x00);
		gameByte(0x00);
		sbi(PORTC, PScommand);
		_delay_ms(1);
		sbi(PORTC, PSattention);
		_delay_ms(10);
		sbi(PORTC, PScommand);
		sbi(PORTC, PSclock);
		cbi(PORTC, PSattention);
		gameByte(0x01);
		gameByte(0x43);
		gameByte(0x00);
		gameByte(0x00);
		gameByte(0x5A);
		gameByte(0x5A);
		gameByte(0x5A);      //exit config mode.
		gameByte(0x5A);
		gameByte(0x5A);
		sbi(PORTC, PScommand);
		_delay_ms(1);
		sbi(PORTC, PSattention);
		_delay_ms(10);
		sbi(PORTC, PScommand);
		sbi(PORTC, PSclock);
		cbi(PORTC, PSattention);
		gameByte(0x01);
		chk_ana = gameByte(0x42);
		gameByte(0x00);
		gameByte(0x00);
		gameByte(0x00);     //set of values that we have to send to get the ouput back in full duplex transmission.
		gameByte(0x00);
		gameByte(0x00);
		gameByte(0x00);
		gameByte(0x00);
		sbi(PORTC, PScommand);
		_delay_ms(1);
		sbi(PORTC, PSattention);
		_delay_ms(10);
		cnt++;
	}
}

short int PS2_commn() {
	short int temp, data0, data1, data2, data3, data4, data5;
	sbi(PORTC, PScommand);
	cbi(PORTC, PSattention);
	gameByte(0x01);
	temp = gameByte(0x42);
	gameByte(0x00);			//all data's are corresponding to somevalue in the controller
	data0 = gameByte(0x00);         //l,r,u,d,select,start,lj,rj
	data1 = gameByte(0x00); 	//square,triangle,circle,x,l1,r1,l2,r2.
	data2 = gameByte(0x00);         //had square thoda
	data3 = gameByte(0x00);         //has left joystick and right down action.
	data4 = gameByte(0x00);         //has right joystick nly
	data5 = gameByte(0x00);         //probably something else.
	_delay_us(1); 
	sbi(PORTC, PScommand);
	_delay_us(1);
	sbi(PORTC, PSattention);
	data[0] = data0;
	data[1] = data1;
	_delay_ms(30);
	return 1;
}

int main(void) 
{
	DDRA=0xff;	//display data[0]
	DDRK=0xff;	//display data[1]
	DDRD=0xff;	//motor control so all are declared as output.
	
	PORTB = 0xff;   //switched on
	
	
	
	pwm_init();  //sets b as output
	uint8_t pwm;
	pwm = 115;
	
	init_PS2();
	while(1) {
		PS2_commn();
		PORTA=data[0];		//data display
		PORTK=data[1];		//data display
		
//*************conditions*********************    //conditions based on the bit mapping.

	if (data[0]==0b10011100)//  right direction                    
	{
		PORTD=0b00001010;    //fwd motion for right motor ,right motor is first 2 bits.	 fwd motion for entire bot
		
					
					OCR0A=pwm;
					OCR2A=pwm;
					
	}
	else if (data[0]==0b01111100)	//left button
	{
		PORTD=0b00000101;	//bkwd motion for right motor and entire bot.
		
			
			OCR0A=pwm;
			OCR2A=pwm;
			
	}
	
	}
}

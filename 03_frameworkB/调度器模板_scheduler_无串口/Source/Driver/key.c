#include <key.H>
//unsigned char key_read()
//{
//	unsigned char key=0;

//	if(P33==0)key=4;
//	if(P32==0)key=5;
//	if(P31==0)key=6;
//	if(P30==0)key=7;

//	return key;
//}
unsigned char key_read()
{
	unsigned char key=0;
	P44=0;P42=1;P35=1;//P34=1;
	if(P33==0)key=4;
	if(P32==0)key=5;
	if(P31==0)key=6;
	if(P30==0)key=7;
	P44=1;P42=0;P35=1;//P34=1;
	if(P33==0)key=8;
	if(P32==0)key=9;
	if(P31==0)key=10;
	if(P30==0)key=11;
	P44=1;P42=1;P35=0;//P34=1;
	if(P33==0)key=12;
	if(P32==0)key=13;
	if(P31==0)key=14;
	if(P30==0)key=15;
//	P44=1;P42=1;P35=1;P34=0;
//	if(P33==0)key=16;
//	if(P32==0)key=17;
//	if(P31==0)key=18;
//	if(P30==0)key=19;
	return key;
}

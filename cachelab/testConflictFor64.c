#include<stdio.h>
#include<stdlib.h>
#include<string.h>
int main(int argc, char** argv)
{
	int x = 0x0030b120;
	int xx = 0x0034b120;
	for(int i = 0; i < 64 * 64; i++)
	{
		unsigned int y = xx + 4 * i;
		if((y >> 5U) % 32U == 9)
			printf("%d,%d ", i/64, i%64);
	}
	return 0;
}
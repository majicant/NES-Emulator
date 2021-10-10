#include "nes.h"

int main(int argc, char* argv[])
{
	if (argc < 2)
		return 1;
	NES nes(argv[1]);
	nes.Run();
	return 0;
}
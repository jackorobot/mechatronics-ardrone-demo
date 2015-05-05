// libardrone-demo.cpp : Defines the entry point for the console application.
//

#include "stdafx.h"
#include "libardrone.h"

using namespace libardrone;

int _tmain(int argc, _TCHAR* argv[])
{
	ARDrone drone;
	drone.takeOff();
	while (true){
		drone.hover();
	}
	return 0;
}


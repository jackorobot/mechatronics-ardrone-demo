// libardrone-demo.cpp : Defines the entry point for the console application.
//

#include "stdafx.h"
#include "libardrone.h"

using namespace libardrone;

int _tmain(int argc, _TCHAR* argv[])
{
	ARDrone drone;
	drone.reset();
	drone.ftrim();
	do{
		drone.takeOff();
	} while (drone.navdata.drone_state["fly_mask"] != 1);
	while (GetAsyncKeyState(VK_ESCAPE)){
		drone.hover();
	}
	do{
		drone.land();
	} while (drone.navdata.drone_state["fly_mask"] == 1);
	return 0;
}


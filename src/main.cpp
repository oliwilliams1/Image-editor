#include "App.h"
#include <cstdlib>

int main() 
{
	App app = App();
	// app.OpenFolderContents("C:\\Users\\Oliver\\Pictures");
	app.SetLogoImage("logo.png");
	app.Mainloop();
	return 0;
}
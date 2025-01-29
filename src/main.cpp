#include "App.h"
#include <cstdlib>

int main() 
{
	App app = App();
	app.OpenFolderContents("C:/Users/Oliver/Pictures/PHOTOS"); // debug 
	app.Mainloop();
	return 0;
}
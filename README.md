# DioAppFinder
A simple nice fast application finder/launcher.
By default it only searches for desktop files in /usr/share/applications
You can add your own custom paths in .config/dioappfinder/dioappfinder.conf or use dioappfinder-settings to manage the paths. It was tested on Debian 12.

# What you can do with DioAppFinder
   1. Search for installed applications (desktop files).
   2. Add a custom path to look for desktop files.

# Screenshots
![Alt text](https://github.com/DiogenesN/dioappfinder/blob/main/dioappfinder.png)
![Alt text](https://github.com/DiogenesN/dioappfinder/blob/main/dioappfindersettings.png)

# Usage
  1. Open a terminal and run:

		 ./configure

  2. if all went well then run:

		 make
		 sudo make install

  3. Navigate to appsettings directory and run:

		 make
		 sudo make install
		
  4. Run the application or bind it to a key combination (defaul behavior is toggle open/close):
  
		 dioappfinder
		
  5. To access the settings run:
  
		 dioappfinder-settings

 Make sure you have the following packages installed:

		make
		pkgconf
		libgtk-4-dev

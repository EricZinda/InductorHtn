Inductor Hierarchical Task Network Engine For C++ and Python
============================================================
This lightweight [Hierarchical Task Network](https://en.wikipedia.org/wiki/Hierarchical_task_network) engine was first used in production in an iPhone strategy game called [Exospecies](https://www.exospecies.com). Visit the [Exospecies Blog](https://blog.inductorsoftware.com) for more details.  It is designed to be small, memory constrained, and used as an implementation detail of an app. It used the classic [SHOP Planner](http://www.cs.umd.edu/projects/shop/description.html) as inspiration and largely followed that model.

It can be used natively in C++ and it has bindings that support Python 3.x as well (using CTypes). It has been built and tested on Windows, Mac, and Ubuntu Linux.

Use and enjoy!


## To Build
indhtn is designed to be built with [CMake](https://cmake.org) like this:

1. [Install CMake on your machine](https://cmake.org/install/)
	- Ubuntu: apt-get install cmake
2. Go to the root of the InductorHtn repository and create a build directory. 
	- unix: `mkdir build`
	- win: `md build`
3. Change to that directory.
	- unix and win: `cd build`
4. CMake can build different types of projects using "generators".  Run `cmake -help` to get a list of generators on your system:
	- unix and win: `cmake -help`
5. Pick the generator that will create the type of project you want and use the `-G` option to choose it. Here are the ones that have been tested:
	- Ubuntu make file:			`cmake -G "Unix Makefiles" ../src` 
	- Mac make file: 			`cmake -G "Unix Makefiles" ../src`
	- Mac Xcode:	 			`cmake -G "Xcode" ../src`
	- Windows Visual Studio: 	`cmake -G "Visual Studio 16 2019" ../src`
6. Then actually do the build using this command which magically builds whatever you choose on the command line: 
	- `cmake --build ./ --config Release`
	- or
	- `cmake --build ./ --config Debug`
7. OR you can manually use the build system that got created by cmake:
	- unix or mac make file: 	`make`
	- Mac Xcode:				Open the IndProlog.xcodeproj file in the build directory using Xcode.
	- Windows Visual Studio: 	Open the .sln file in the build directory using VS.


## Running Tests to make sure the build worked
If you're using a command line generator of some sort, just change to the directory where things got built and run `runtests` on the commandline in your operating system of choice.

To run the Python tests:
1. Make sure Python 3.x is on your path
2. Make sure the indhtnpy.dll (or libindhtnpy.dylib on the mac) that gets built is on your path
	- On windows check or set your PATH variable, 
	- On the mac, I found it easiest to put it into usr/local/lib
	- On Ubuntu Linux, /usr/lib worked well
3. Get to the /src/Python directory
4. Run "python PythonUsage.py"
5. Look at the source for PythonUsage.py for basic usage instructions


### Xcode
In Xcode, after you build you should change the scheme to `runtests` and then choose Product/Run.  The output of the test will appear in the Output window.


### Visual Studio
Set the default project to runtests and hit F5. You'll get a console window with the results


## Directory Structure
99.99% of the code for the Htn is platform agnostic (or at least should be). It has been built and tested on Windows, Mac and iOS. The platform specific code is located in the iOS and Win directorys and is currently only a single function for debug logging.

- /FXPlatform: 			Contains some general purpose code for tracing, asserts, strings, etc
- /FXPlatform/Parser: 	The Inductor Parser code
- /FXPlatform/iOS: 		Code specific to iOS and Mac
- /FXPlatform/Win: 		Code specific to Windows
- /FXPlatform/Prolog: 	The prolog compiler and runtime engine
- /Python:				The Python interface and usage examples
- /UnitTest++:			The UnitTest++ framework used to write unit tests (https://github.com/unittest-cpp/unittest-cpp)
- /Tests:				Basic smoke tests used to make sure it compiled properly

## Getting Started
Read GettingStarted.md for a tutorial on how to use InductorHTN in C++ and Python.

License
---------
Do what you like, with no warranties! Read License.md.

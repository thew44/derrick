# Derrick Deep File Search

A very simple C library implementing indexing and deep file search for MS Windows.

## Getting Started

These instructions will get you a copy of the project up and running on your local machine for development and testing purposes. See deployment for notes on how to deploy the project on a live system.

### Prerequisites

- The Qt build system
- Either the mingw compiler or MSVC

### Installing

#### Build library
```
qmake derrick.pro
mingw32-make
```
...or just copy derrick.c and derrick.h to your project.

#### Build test program 'search'
```
qmake search.pro
mingw32-make
```
You do not need to build the library prior building the test program.

## Running the test
Then run the test program by executing search.exe


### Indexed search
```
search> base C:\derrick
Base directory [C:\derrick]
search> index
search> list
C:\derrick\.git\COMMIT_EDITMSG (25b)
C:\derrick\.git\config (300b)
C:\derrick\.git\description (73b)
...
C:\derrick\derrick.c (15285b)
C:\derrick\derrick.h (3672b)
C:\derrick\LICENSE.md (1093b)
C:\derrick\main.c (5527b)
C:\derrick\README.md (1006b)
C:\derrick\search.pro (888b)
C:\derrick\search.pro.user (25488b)
search> find Mathieu Allory
C:\derrick\derrick.h
[ * @author      Mathieu Allory]
C:\derrick\README.md
[* **Mathieu Allory** - *Initial work*]
```
**Notes:**
- index must be called before find and list
- list command is not mandatory

### Deep file search
```
search> base C:\derrick
Base directory [C:\derrick]
search> count
Number of files: 7
search> search Mathieu Allory
C:\derrick\derrick.h
[ * @author      Mathieu Allory]
C:\derrick\README.md
[* **Mathieu Allory** - *Initial work*]
```
**Notes:**
- dfs example callback excludes .git directory from search
- count command is not mandatory

## Authors

* **Mathieu Allory** - *Initial work*

See also the list of [contributors](https://github.com/your/project/contributors) who participated in this project.

## License

This project is licensed under the MIT License - see the [LICENSE.md](LICENSE.md) file for details

## Acknowledgments

* Stephan Derrick

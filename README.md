

This repo include the solution to the HikersCrossingBridges problem.


# Dependency:
We use an existing yaml parsing library (https://github.com/jbeder/yaml-cpp.git) for parsing yaml files. 
Instructions to get the dependency:
```
$ git clone https://github.com/jbeder/yaml-cpp.git
$ cd yaml-cpp
$ mkdir build & cd build
$ cmake ..
$ make && make install 
```

# Build and run the project:

In the root directory of the project:
```
$ g++ -o hiker -lyaml-cpp -std=c++11 -Wc++11-extensions main.cpp
```

Some test cases have been included in the code. To run these test cases without supplying a yaml file:
```
$ ./hiker
```

To run a test case specified in a yaml file:
```
$ ./hiker case.yaml
```


# Overview of the code
We have the following classes:
- Hiker
- Bridge
- Cache
- CrossingTimeCalculator
- YAMLCaseParser
- CaseParser

Hiker and Bridge are models for the hikers and bridges. 

We create the Cache class to save previous calculation result, as it may be possible to reuse the previous result. If we can reuse the previous result, we call it a cache hit. The cache hit happens when there aren't any additional hikers at a new bridge; that is, the whole group is the same as when we are at the previous bridge. We can reuse the previous per feet time, and just calculate the new time needed to cross the current bridge for the whole group.

The main logic of calculation is in the CrossingTimeCalculator class.

YamlCaseParser is the class to parse a test case from a YAML file. 

CaseParser is the class to parse a test case from a string representation. This string representation: `"A 100,B 50,C 20,D 10;100;250,E 2.5;150,F 25,G 15"` means:
- there are 4 original hikers: A (speed 100), B (speed 50), C (speed 20), and D (speed 10). 
- there are 3 bridges: the first bridge's length is 100, and it has no additional hikers; the second bridge's length is 250 and it has one additional hiker, E (speed 2.5); the third bridge's length is 150, and it has two additional hikers: F (speed 25) and G (speed 15).
This helps us run the main logic on some simple test cases before figuring out how to do yaml parsing.


# Main solution logic
The main logic of calculation is in the CrossingTimeCalculator class. Since the total time of crossing a bridge is proportional to the bridge length, we calcule the perFeetTime give a group of original hikers and additional hikers. With the perFeetTime, the total time will just be the length of the bridge (in feet) times the perFeetTime.

We deal with the simple case when there is only one original hiker in `calcOneOrigHikerPerFeetTime()`.

For the general case, there is a threshold value (calculated in `calcThresholdSpeed`) that guides us how to schedule the hikers to cross the bridge. Assume that there are two original hikers.



# Results of simple test cases
Result of the first test case:

```
Case: A 100,B 50,C 20,D 10;100;250,E 2.5;150,F 25,G 15
Bridge (100)
A,B cross, A returns
D,C cross, B returns
A,B cross
Bridge (250)
A,B cross, A returns
E,D cross, B returns
A,C cross, A returns
A,B cross
Bridge (150)
A,B cross, A returns
E,D cross, B returns
A,B cross, A returns
G,C cross, B returns
A,F cross, A returns
A,B cross
245 minute(s)
P
```
4 hikers, 3 briges. Note that although the third bridge only has F and G as the additional hikers in the input, when we do the calculations, we should also consider E as an additional hiker, because at each bridge, we need to consider all accumulated additional hikers encountered at the current bridge and at previous bridges.

Results of more test cases:
```
Case: A 100;100
Bridge (100)
A crosses
1 minute(s)
P
Case: A 100,B 50;100
Bridge (100)
A,B cross
2 minute(s)
P
Case: A 100,B 50,C 20,D 10;100
Bridge (100)
A,B cross, A returns
D,C cross, B returns
A,B cross
17 minute(s)
P
Case: A 100,B 25,C 20,D 10;100
Bridge (100)
A,D cross, A returns
A,C cross, A returns
A,B cross
21 minute(s)
P
Case: A 100,B 50,C 20,D 10;100;200
Bridge (100)
A,B cross, A returns
D,C cross, B returns
A,B cross
Bridge (200)
Hit cache for hiker count 4 at bridge with length 200
51 minute(s)
P
Case: A 100,B 50,C 20,D 10;100;200,E 200
Bridge (100)
A,B cross, A returns
D,C cross, B returns
A,B cross
Bridge (200)
A,B cross, A returns
D,C cross, B returns
A,E cross, A returns
A,B cross
55 minute(s)
P
Case: A 100,B 50,C 20,D 10;100;200,E 80
Bridge (100)
A,B cross, A returns
D,C cross, B returns
A,B cross
Bridge (200)
A,B cross, A returns
D,C cross, B returns
A,E cross, A returns
A,B cross
55.5 minute(s)
P
Case: A 100,B 50,C 20,D 10;100;200,E 50
Bridge (100)
A,B cross, A returns
D,C cross, B returns
A,B cross
Bridge (200)
A,B cross, A returns
D,C cross, B returns
A,E cross, A returns
A,B cross
57 minute(s)
P
Case: A 100;100,B 50,C 20,D 10;200,E 50
Bridge (100)
A,D cross, A returns
A,C cross, A returns
A,B cross
Bridge (200)
A,D cross, A returns
A,C cross, A returns
A,E cross, A returns
A,B cross
63 minute(s)
P
```
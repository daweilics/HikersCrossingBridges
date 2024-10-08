

This repo hosts the solution to the HikersCrossingBridges problem.


# Dependency
We use an existing yaml parsing library (https://github.com/jbeder/yaml-cpp.git) for parsing yaml files. 
We plan to use googletest (https://github.com/google/googletest.git) for unit test.
We use these two repos as submodules in our project.

To use the project, 
```
$ git clone --recurse-submodules https://github.com/daweilics/HikersCrossingBridges.git
```
Or, clone the project first, and then bring in the submodules.
```
$ git clone https://github.com/daweilics/HikersCrossingBridges.git
$ cd HikersCrossingBridges
$ git submodule update --init --recursive
```

# Build and run the project
Build the project, in the root directory (`HikersCrossingBridges`) of the project: 
```
$ make
```
Some test cases have been included in the code. To run these test cases without supplying a yaml file:
```
$ ./hiker
```

To run a test case specified in a yaml file:
```
$ ./hiker case.yaml
$ ./hiker golden-case.yaml
```

We have a plan for unit test:
```
$ make test
```
This will generate a run_tests executable in the project folder. Run unit tests as follows:
```
$ ./run_tests
```

# Overview of the code
We have the following classes:
- Hiker
- Bridge
- Cache
- CrossingTimeCalculator
- YAMLCaseParser
- CaseParser

`Hiker` and `Bridge` are models for the hikers and bridges. 

We use the `Cache` class to save previous calculation result, as it may be possible to reuse the previous result. If we can reuse the previous result, we call it a cache hit. The cache hit happens when there aren't any additional hikers at a new bridge; that is, the whole group is the same as when we are at the previous bridge. We can reuse the previous per feet time, and just calculate the new time needed to cross the current bridge for the whole group. We believe further optimizations may be possible for various cases. For example, if all new additinoal hikers are faster than all previous hikers, the new hikers shouldn't affect the scheduling plan of the previous hikers, and thus, the result for the previous hikers may be reused.

The main logic of calculation is in the `CrossingTimeCalculator` class. We avoid removing items from the hikers list (which could be expensive operations) during the calculation,  and just use index traversing to simulate removing an item.

`YAMLCaseParser` is the class to parse a test case from a YAML file. 

`CaseParser` is the class to parse a test case from a string representation. This string representation: `"A 100,B 50,C 20,D 10;100;250,E 2.5;150,F 25,G 15"` means:
- there are 4 original hikers: A (speed 100), B (speed 50), C (speed 20), and D (speed 10). 
- there are 3 bridges: the first bridge's length is 100, and it has no additional hikers; the second bridge's length is 250 and it has one additional hiker, E (speed 2.5); the third bridge's length is 150, and it has two additional hikers: F (speed 25) and G (speed 15).
This helps us run the main logic on some simple test cases before figuring out how to do yaml parsing.


# Main solution logic
The main logic of calculation is in the CrossingTimeCalculator class. Since the total time of crossing a bridge is proportional to the bridge length, we calcule the perFeetTime give a group of original hikers and additional hikers. With the perFeetTime, the total time will just be the length of the bridge (in feet) times the perFeetTime.

We deal with the simple case when there is only one original hiker (`if (hikers.size() == 1)`).

For the general case when we have at least two original hikers, there is a threshold value (calculated in `calcThresholdSpeed`) that guides us how to schedule the hikers to cross the bridge. Assume that we have 4 hikers, A, B, C, D, speeds sorted in the reverse order. 

If the slowest two hikers' speeds are smaller then the threshold value, the fastest plan is:
- A, B cross the bridge
- A carries the torch back
- C, and D cross the bridge
- B carries the torch back
- A, B cross the bridge

If the slowest two hikers' speeds are greater then the threshold value, the fastest plan is:
- A, B cross the bridge
- A carries the torch back
- A, C cross the bridge
- A carries the torch back
- A, D cross the bridge

The intuition is that, if two hikers' speeds are extremely slow, we should group them together to reduce the overall crossing time, so that an "extremely long" trip only happens once; if they are not grouped together, we will have two rounds of the "extremely long" trips, making the overall crossing time larger compared. This observation can be applied to general cases when the number of hikers are arbitrary. 

So, our general approach at each step is to group the two slowest hikers (who are still at the beginning of the brideg) together if their speeds are slower than the threshold. Of course, we need to consider whether the total number of such hikers is even or odd. If the number is even, we have perfect groups.If the number is odd, we deal with the last such hiker separately.

Additional strategies we use achieve the minimum crossing time:
- Only the fastest and second fastest hiker in the original group will carry the torch back.
- When we don't have "extremely" slow hikers, we always let the fastest hiker to accompany a slower hiker to the other side, and the fastest hiker will carry the torch back.
- When there are `n (n >= 2)` hikers, there will be `n - 1` forward trips, and `n - 2` backward trips for all the hikers to cross the bridge.

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
In this first test case, there are 4 hikers and 3 briges. Note that although the third bridge only has F and G as the additional hikers in the input, when we do the calculations, we should also consider E as an additional hiker at the third bridge. This is because at each bridge, we need to consider all accumulated additional hikers encountered at the current bridge and at previous bridges. `P` at the end of the test case means the test passes.


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
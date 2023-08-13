## Table of contents

- [I. Description](#i-description)
- [II. Compile and run](#ii-compile-and-run)
- [III. Code \& Idea Description](#iii-code--idea-description)
    - [**avl.hpp**](#avlhpp)
    - [**main.cpp**](#maincpp)
- [IV. Note](#iv-note)
---
# I. Description
- This is a program to generate the snapshot from orders log file.
- The idea bases on AVLTree with performing of persistent update method to reserve the history of data structure modification.
- Using mmap() to create disk-base data structure.
- Time mesuring on MacBook Pro 2019, 16GB, Core i7 2.6GHz x 6.

# II. Compile and run
Compile with
```
  g++ -std=c++17 -o main main.cpp
```
To run, the program supports 3 modes:
- **`READ`**: `filename` is the name of log file, for example `SCS.log`
- **`QUERY_SHORT`**: `symbolNum` is the number of symbols, following by the list of symbols SCS SCH ...
- **`QUERY_LONG`**: `epochLower` is the lower bound of the epoch range, `epochUpper` is the upper bound of the epoch range,
```
  ./main READ filename

  ./main QUERY_SHORT symbolNum symbol_1 symbol_2 ...

  ./main QUERY_LONG epochLower epochUpper symbolNum symbol_1 symbol_2 ...
```

# III. Code & Idea Description
### **avl.hpp**
- Implementation of the `AVLTree` with manipulate methods of the tree.
- Implementation of an `array` of `struct Version` which is store the root of AVLTree for each version of persistent update.
- Implementation of `Binary Search` to find the proper `root` with a given `epoch`.
- Implementation of `AVLTree traverse` to find best 5 bids/asks.
### **main.cpp**
- Reading input and call proper function from `avl.hpp`.
- Maintaining 2 order books for each kind of symbol.
- Supporting 3 modes:
  - **`READ`**
    - It will read all the orders in the input file, parsing and insert them to sellBook/buyBook (AVLTree).
    - After read finished, it will create 4 .bin files for each kind of symbol (please delete these file if you want to read the same file again). These files are mapped file to store the structure of the AVLTree.
    - Processing time for SCS.log is around 30s.
    - Processing time for SCS.log is around 15s.
    ```
      ./main READ SCS.log             
      Number of nodes in sellBook: 3488856
      Number of roots in sellBook: 423631
      Number of nodes in buyBook: 4042199
      Number of roots in buyBook: 426584
      Time taken by reading, parsing file SCS.log: 31034 (ms).

      ./main READ SCH.log
      Number of nodes in sellBook: 1521828
      Number of roots in sellBook: 231962
      Number of nodes in buyBook: 1760620
      Number of roots in buyBook: 226035
      Time taken by reading, parsing file SCH.log: 15894 (ms).
    ```
  - **`QUERY_SHORT`**
    - This mode support snapshot generation for a short range of epoch.
    - The snapshot will be printed to the console.
    ```
      ./main QUERY_SHORT 2 SCS SCH    
      Input -1 -1 to exit or epoch range with format x y: 1609722840027798182 1609722840030020871
      --------*********--------- SNAPSHOT FOR SCS --------*********---------
      BID SCS, 1609722840030007580, 2@272.22, 1@272.21
      BID SCS, 1609722840030020871, 2@272.22, 1@272.21, 0@272.2, 2@272.19, 51@272.18
      ASK SCS, 1609722840027798182, 1@272.27, 1@272.28, 4@272.29, 3@272.33, 1@272.34
      ASK SCS, 1609722840029994293, 1@272.27, 1@272.28, 4@272.29, 3@272.33, 1@272.34
      ASK SCS, 1609722840030007580, 1@272.27, 1@272.28, 4@272.29, 3@272.33, 1@272.34
      --------*********--------- SNAPSHOT FOR SCH --------*********---------
      Invalid range for side BUY
      Invalid range for side SALE

      Input -1 -1 to exit or epoch range with format x y: 1609727896820086711 1609727896840666200
      --------*********--------- SNAPSHOT FOR SCS --------*********---------
      Invalid range for side BUY
      Invalid range for side SALE
      --------*********--------- SNAPSHOT FOR SCH --------*********---------
      BID SCH, 1609727896820086711, 0@107.36, 0@107.35, 0@107.34, 0@107.33, 0@107.32
      BID SCH, 1609727896828923475, 0@107.36, 0@107.35, 0@107.34, 0@107.33, 0@107.32
      BID SCH, 1609727896832539333, 0@107.36, 0@107.35, 0@107.34, 0@107.33, 0@107.32
      BID SCH, 1609727896840666200, 0@107.36, 0@107.35, 0@107.34, 0@107.33, 0@107.32
      Invalid range for side SALE
    ```
  - **`QUERY_LONG`**
    - This mode support snapshot generation for a long range of epoch.
    - The snapshot will be printed to the `snapshot.txt` file with the same format as above.
    - At the end of the `snapshot.txt` file, there is time measuring for this mode as well.
    - The export snapshots for the range of [minEpoch, maxEpoch] in SCS.log is around **13 seconds**.
    - The export snapshots for the range of [minEpoch, maxEpoch] in SCH.log is around **6 seconds**.
    ```
      ./main QUERY_LONG 1609722840027798182 1609732199950874110 1 SCS
      --> in snapshot.txt - last line
      Time to generate and write snapshots to snapshot.txt: 13543 (ms)

      ./main QUERY_LONG 1609722840017828773 1609732199727381552 1 SCH
      --> in snapshot.txt - last line
      Time to generate and write snapshots to snapshot.txt: 6643 (ms)
    ```
# IV. Note
- What has been implemented:
  - Support single and multiple symbols.
  - Support all fields ***except last trade price and quantity***.
  - Generate snapshots for a range of epoch.
  - Manually testing.
- What would be implemented:
  - Support fields selection and **last trade price and quantity**, perhaps require one more array to store the trade price and quantity during time.
  - Comment with more detail in the source code.
  - Improve performance by using mmap() with offset to map only the necessary part of the file.
  - Automated testing.
- This repository is implemented from scratch by `Nguyen Ba Vinh Quang`.


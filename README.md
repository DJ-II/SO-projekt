# Dining Philosophers Problem
## compilation
C++11 or later is required. Example compilation in terminal opened in project's folder: 
```bash
g++ -std=c++11 -pthread filozof.cpp -o filozof
```
## running the program
The are two parameters - number of philosophers (default is 5) and duration of the program (default is 60 seconds). This will run the program with default parameters values:
```
./filozof
```
This will run the program with 6 philosophers initiated for 80 seconds:
```
./filozof 6 80
```


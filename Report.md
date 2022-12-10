# EE538: Computing Principles for Electrical Engineers - Project: Trojan Map

## Final Report by: Ashish Kumar and Sunanda Somu

## Video: https://www.youtube.com/watch?v=XgosrAJvLP8

### The project requires to create a graph and perform mapping computations out of ~20000 coordinates located around the area of University of Southern California. Run various algorithms on it like

- Autocomplete
- Find Location
- Find Closest Name & Calculate Edit Distance
- Get all categories
- Get all locations of category 
- Get location matching regular expression 
- Calculate Shortest Path
- Cycle Detection                                          
- Topological Sort
- Traveling salesman problem 
  - Brute Force Algorithm
  - BackTracking Algorithm
  - 2-opt Algorithm
  - 3-opt Algorithm
- Find Nearby 
- All nodes Shortest Path
- Dynamic Map UI 

<p align="center"><img src="Photos/Main_Screen.png" alt="Trojan" width="500" /></p>

## The Data Structure

Each point on the map is represented by the class **Node** defined in [trojanmap.h] as below

```cpp
class Node {
public:
  Node(){};
  Node(const Node &n) {
    id = n.id;
    lat = n.lat;
    lon = n.lon;
    name = n.name;
    neighbors = n.neighbors;
    attributes = n.attributes;
  };
  std::string id;    // A unique id assign to each point
  double lat;        // Latitude
  double lon;        // Longitude
  std::string name;  // Name of the location. E.g. "Bank of America".
  std::vector<std::string>
      neighbors;  // List of the ids of all neighbor points.
  std::unordered_set<std::string>
      attributes;  // List of the attributes of the location.

};
```

## Menu
We have implemented Menu as the Static and Dynamic UI
Here is the image for Static UI
<p align="center"><img src="Photos/1.png" width="500" /></p>
Here is the image for Dynamic UI
<p align="center"><img src="Photos/Screenshot 2022-12-09 at 7.10.05 PM.png" width="500" /></p>

## 1. Autocomplete The Location Name
```cpp
std::vector<std::string> Autocomplete(std::string name);
```
Here, the input is entered as a string and is transformed into lower case letters to avoid case sensitivity. After this, an autocomplete suggestion using entered input as prefix is displayed on the screen. If input is an empty string, an empty string is returned as output. 
### The runtime of the function: O(n)

Example:

Input: "Chi"
Output: ["Chick-fil-A", "Chipotle", "Chinese Street Food"]


## 2.1 Find the place's coordinates in the Map
```cpp
std::pair<double, double> GetPosition(std::string name);
```
In this function, we check through the data vector and find out the exact matching name; if we find a match, longitude and latitude values are displayed as output. If the entered name does not match, then it suggests a list of locations, and outputs the values of longitude and latitude of the chosen location. If in case an empty string is entered the function returns -1 for both longitude and latitude.

### Time complexity is O(n)
 
Example:
Input: "Target"
Output: (34.0257016, -118.2843512) 


## 2.2. Check Edit Distance Between Two Location Names
```cpp
int CalculateEditDistance(std::string name1, std::string name2);
```
Here in CalculateEditDistance, we use distance algorithm to calculate the distance between two strings. We use 2D vector for tabulization and dynamic programming. Condition in this is that if any string is an empty string, then the other string is returned as output. Using a loop, we compare both strings, if the characters are different then we take a minimum of three operations: insert, delete or replace a character.
If the entered string does not match with the data vector then using FindClosetName function, we see a Did you mean suggestion with most closest option.

### Time complexity is O(n)

Example:
Input: "Rolphs", "Ralphs"
Output: 1

## 3. Get All Categories



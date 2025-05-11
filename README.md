# PDOGS - Product Delivery Optimization Game Simulation

## Overview

**PDOGS** (Product Delivery Optimization Game Simulation) is a grid-based simulation game framework written in C++. It is designed to test strategies for optimizing the extraction, combination, and delivery of numerical products on a 2D board. Players implement their own AI logic to build structures, extract values from the environment, and deliver valid products to the center goal area for points.

The game revolves around constructing a pipeline of machines and conveyors to collect "number cells" and process them into deliverable products under specific divisibility rules.

## Game Mechanics

* The board size is **62 × 36**, containing randomly distributed background `NumberCell`s and obstacles like `WallCell`s.
* The central **Collection Center** is a fixed 4×4 area where players deliver processed products to earn points.
* A product earns a point if its value is divisible by a predefined **common divisor**.
* Players are allowed to issue an action every 3 time units, and the game ends after **9000** time units.

## Key Cell Types

* **NumberCell**: A background cell containing a number (e.g., 1, 2, 3...). These are extractable only if divisible by the level’s divisor.
* **MiningMachineCell**: Extracts a number from a `NumberCell` every 100 ticks and outputs it in a given direction.
* **ConveyorCell**: Moves products along in a single direction, with a buffer size of 10.
* **CombinerCell**: Accepts two numbers and combines them into a single sum, outputting in a defined direction.
* **WallCell**: An obstacle that cannot be removed or built upon.
* **CollectionCenterCell**: The scoring area in the center of the board.

## AI Implementation

The player's logic is implemented in a custom class inheriting from the `IGamePlayer` interface. The AI must:

1. Locate `NumberCell`s that contain valid divisible numbers.
2. Place `MiningMachineCell`s to extract these values.
3. Build a conveyor path to deliver products to the center.
4. Optionally use `CombinerCell`s to combine two numbers if needed.
5. Dynamically adapt to the board and manage limited build opportunities.

## Project Structure

* `PDOGS.hpp`: Main game engine and all class definitions.
* `main()` function: Reads a test ID and triggers the corresponding simulation.
* `Test()` function: Runs the simulation using a seeded random board and the custom `GamePlayer` logic.
* Player's logic is written in the `GamePlayer` class (bottom of the file).

## How to Run

Compile and run the program with an integer input (1–10) corresponding to a specific test case:

```bash
$ g++ -std=c++17 -o pdogs main.cpp
$ ./pdogs
3
```

Each test case corresponds to a `(common divisor, seed)` configuration, e.g., Test3A runs with divisor 3 and seed 30.

## Test Cases

| Test ID | Divisor | Seed   |
| ------- | ------- | ------ |
| 1A      | 1       | 20     |
| 1B      | 1       | HIDDEN |
| 2A      | 2       | 25     |
| 2B      | 2       | HIDDEN |
| 3A      | 3       | 30     |
| 3B      | 3       | HIDDEN |
| 4A      | 4       | 35     |
| 4B      | 4       | HIDDEN |
| 5A      | 5       | 40     |
| 5B      | 5       | HIDDEN |

## Sample Strategy (Current Implementation)

* Starts scanning outward in a cross pattern from the Collection Center.
* If a divisible `NumberCell` is found, places a `MiningMachineCell` facing the center.
* Builds a `ConveyorCell` chain to carry products to the Collection Center.
* Updates movement direction once each cell is processed.
* Skips cells blocked by walls or other unbuildable objects.

## Future Improvements

* Implement adaptive pathfinding to avoid walls.
* Use combiner logic to synthesize higher-value divisible numbers.
* Optimize conveyor path lengths.
* Track and dynamically reallocate build resources.

# Clockwork Digits

A digital clock built from a grid of small analog clocks using C and Raylib.

Each digit is made from a 6x4 grid of analog clocks. Every small clock has two hands, and the angle of those hands determines whether that clock represents a horizontal line, vertical line, corner, or empty space. Together, the clocks form digital-style numbers that update to show the current local time.

https://github.com/user-attachments/assets/b956cdb8-13c9-41c0-9a42-846e5e7d8d41

## Overview

This project recreates a digital clock display using only animated analog clock hands.

The clock displays time in `HH MM SS` format. Every second, the target digit layout is recalculated from the current local time, and all clock hands smoothly rotate into their new positions.

The project demonstrates:

- Drawing basic shapes with Raylib
- Building larger visual structures from a grid
- Representing digits with symbolic patterns
- Mapping symbols to clock hand angles
- Smooth angle interpolation
- Working with local time in C

## How It Works

Each digit is represented as a 6x4 grid.

Every cell in the grid contains a symbol describing how the two clock hands should be positioned:

| Symbol | Meaning                |
| ------ | ---------------------- |
| `-`    | Horizontal line        |
| `\|`    | Vertical line          |
| `a`    | Top-left corner        |
| `b`    | Top-right corner       |
| `c`    | Bottom-left corner     |
| `d`    | Bottom-right corner    |
| Space  | Empty / inactive clock |

Each symbol is converted into two hand angles. The program stores both current angles and target angles for every clock hand. When the time changes, only the target angles are updated, and the current angles interpolate toward them over time.

## Requirements

- C compiler with C23 support
- Raylib
- CMake, if using the included CMake project setup

## Building

```sh
cmake -B build
cmake --build build
```

## Running

```sh
./build/clock
```

## Main Concepts

* **Digit Patterns**: Digits are defined as symbolic 6x4 patterns. This makes the visual layout easy to adjust without changing the rendering code.
* **Clock Hand Mapping**: Each symbol maps to a pair of angles. These angles control the two hands of each analog clock.
* **Animation**: The program uses angle interpolation so clock hands rotate smoothly toward their next target position instead of snapping instantly.
* **Time Updates**: The current local time is read once per frame, but digit targets are only updated when the second changes.

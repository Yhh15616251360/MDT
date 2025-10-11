# Codes for two search strategies for *MDT*s
This repository contains a reference implementation of the algorithms for the paper:
Towards Fast Truss-Based Dense Temporal Community Search

## Environment

Codes run on a server with an Xeon 2.00GHz and 256GB memory running Ubuntu 18.04

## Dataset file format

* Each event (i.e., temporal edge) consists of vertex1, vertex2, timestamp

    * vertex1, vertex2: participate in the vertices of the event

    * timestamp: the time when the event happens

* the timestamp of any event must be sorted in ascending order

## Codes description

The codes include two search strategies: global strategy and index strategy.

Graph_IO.h reads the dataset from /data and stores it in memory.

global.cpp implements the global strategy: it first calculates the temporal support value for all edges, then iteratively removes the edge with the minimum temporal support, and updates the temporal support of other edges within the same triangle. This process continues until the current query vertex q cannot participate in any triangle, and the algorithm terminates.

build_index.h builds the index: it calculates the temporal support of all edges from 0 to tmax. The specific implementation follows the ideas presented in the referenced paper, with the main idea being to minimize accessing edges and triangles and calculate the temporal support at \delta based on \delta-1.

index_search.h implements the index strategy: it utilizes the query index to quickly determine the value of k* by obtaining the temporal support of edges induced by q. Therefore, it only needs to continuously expand outward using the edges in the target community to quickly determine whether the current edge can appear in the final target community.

print.h contains some print output functions.

/data: contains processed datasets.

## Compile and Run

// Compile and run global strategy

Compile: g++ -std=c++11 global.cpp -o global

Run: ./global filePath filename

// Compile and run index strategy

Compile: g++ -std=c++11 TTS.cpp -o TTS

Run: ./TTS filePath filename

# Problem Set #1: Concurrent Programming w/o Dependencies

## Introduction

The Particle Simulator project is a prime example of sophisticated Java programming methods, especially when it comes to multithreading. In order to ensure that demanding computational activities do not impair the responsiveness of the user interface, the program primarily runs two threads: one for the simulation logic and another for the graphical rendering. Using a ForkJoinPool to efficiently process particle updates in parallel, the simulation thread controls particle movements, collision detection, and physics calculations. Thousands of particles can be simulated in real time using this method, displaying intricate behaviors including interactions and collisions with walls. A smooth and dynamic depiction of particle systems is achieved by the project's use of Java concurrency, which separates the computing workload of the simulation from the GUI presentation.ultiple threads to maximize CPU utilization. 

## Run Guide

To run the JAR file:
``` java -jar ParticleSimulator.jar ```

To compile:
``` javac ParticleSimulator.java ```
``` java ParticleSimulator ```

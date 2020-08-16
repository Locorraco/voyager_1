# Path-of-comet
Simulation of path of an especific comet in C and MPI

## Introduction
Generally speaking, a comet is a celestial body composed mainly of solid matter like ice with organic resting material, dust and rocks. They can have elliptical, parabolic, or hyperbolic orbits, but the most common in the solar system are the elliptical orbits.

The study of the trajectories of a comet date back to the 16th century, when Edmund Halley tried to calculate the trajectory of the comet that would later carry his lastname, using Newton's theory of gravitation as support. In this experiment he discovered that this orbit had a periodic behavior of 76, 77 years.
Later in 1821, the German mathematician Johann Encke discovered another comet whose orbit was also periodic, with a period of 3.3 years

I will focus on calculating the path of a specific comet, in this case, Halley Comet which is considered a young comet (P-AGE <30) and dwarf comet <1.5 km of diameter.
The coordinate system will have the sun at the origin, that means, heliocentric coordinates will be used to calculate the orbit.
As an ephemeris for this comet I will use the following date 03/15/1998.

Ephemeris are available on [Horizons](https://ssd.jpl.nasa.gov/horizons.cgi)

## Objetive
Simulate the orbit of Halley's Comet. Then simulate the trajectory of the planets of the solar system.


## Methodology
The project consists of the following files:
- element: Here are the functions of the population's elements (in this case the bodies of the solar system).
- integrator: This file calculates the speed and position of each element according to time.
- population: Contains the functions that can be applied to a set of elements, such as creating a new population, add new elements, and print the elements of that population.
- recipes: This file calculates the distance between the elements of a population.
- main: It is in charge of gathering the previous files so if we given a population and certain conditions initial like input, the position (x, y, z) and speed (vx, vy, vz) can be calculated according to time. This file uses MPI to optimally perform the calculations of each element in a population regardless of size, therefore it is necessary to have MPI installed to run the file.
- view3D.py: It's a program written in Python 3.7 which takes the results obtained in main.c and graphs the position of each element in the population. 
The number of graphs depends on the number of iterations that are indicate in the line 66 of main.c.


To run main.c it is necessary to download the full git and follow these steps:

1. Run the GNUmakefile file with the following command *make* .
2. Execute main with the following command *mpiexec -n 'number of processes to use' ./main> output.dat*. In this case I used 24 processes to speed up the calculation.
3. The previous command returns a file called **output.dat** if you change the name of that file, you must put the same name on line 14 of view3D.py in addition to creating a folder called "data" to save the graphs.
4. If you want to make a video with the graphs to see the change between them, you can enter [Gifmaker](https://gifmaker.me/).

## Results
The result is in *output.dat* file generated when running main. In the following links there is a video in which you can see the results of 2 simulations different.

[Sun and Halley](https://drive.google.com/file/d/1-8b9hEDP-P7key8j2RUkyfi60CnNfXz8/view?usp=sharing)

[Solar system and Halley](https://drive.google.com/file/d/1sMrY7uikySPxscVMTttsb_Zr8TpHq27y/view?usp=sharing)

[Solar system and Halley (scale)](https://drive.google.com/file/d/19i0Mjx8frIiA_XL1GTe7e5PMzH8Gi7KE/view?usp=sharing)


# raytracer
A Ray Tracer written in C++.

## Compiling the code
To compile the code the only dependency is the following, used only to create a canvas and display the pixels. Everything else in the engine is done from the ground up.

- [Simple and Fast Multimedia Library](https://www.sfml-dev.org/) - A library to help displaying images in a canvas.

A `Makefile` is available on the repo as an example.

## Versions

Not really about versions per se, but there are 2 "different" engines here. On master you have the full ray tracer engine, with all the good stuff (multithreads, kd-trees, textures, area lights, etc). But there is one branch from this repo called `rayMarching`, and as the name implies, this branch is slightly different and includes the ray marching stuff that I added to create volumetric lights and volumetric shadows.

## Read more at

Check my [github blog](https://fvcaputo.github.io/) to read about both the ray tracing and the ray marching engine implemented in this repo. :)

## Some cool pictures rendered with it

<img src="http://i.imgur.com/QKPD53g.png" width="32.5%" style="margin-right: 0.5%" border="1"> <img src="http://i.imgur.com/fKUDnof.png" width="32.5%" style="margin-right: 0.5%" border="1"> <img src="http://i.imgur.com/uGuqSOk.png" width="32.5%" border="1">
<img src="http://i.imgur.com/7a99qgm.png" width="32.5%" style="margin-right: 0.5%" border="1"> <img src="http://i.imgur.com/fcwHYPB.png" width="32.5%" style="margin-right: 0.5%" border="1"> <img src="http://i.imgur.com/mrKdmFQ.png" width="32.5%" border="1">

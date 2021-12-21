# Drag

Drag is a graph drawing library for creating nice looking layouts of directed graphs. I originally implemented this library for my [bachelor's thesis](https://is.muni.cz/th/kmkvd/?lang=en). Since then, the API has changed but the algorithms are still the same.

## What exactly does it do?

In a nutshell, you provide a directed graph and the library produces a nice looking layout of that graph in 2D space. In other words, it computes the positions of all vertices in the graph and the points through which the edges (which, for simplicity, are represented as polylines) should be routed such that there are as few edge crossings as possible.

It is important to note that this is the main purpose of the library. It computes a layout. It is not meant for converting this layout into an image or styling this image. It is meant as a backend for such applications.

Nevertheless, it contains a simple drawing api for creating quick and dirty svg images of the produces layouts. However, it is very limited in what it can do. There is also a command-line application which uses this api. If you want to know more read this [section](#producing-svg-images).

 ## What graphs is it suitable for?

Short answer: directed ideally acyclic graphs but directed graphs with only few cycles are also fine.

It should be used only for *directed* graphs. In theory one could use it to make a layout of an undirected graph by giving each edge an arbitrary direction. However, the resulting layout wouldn't make much sense and wouldn't convey the information contained in the graph very well. 

## Producing SVG images

TODO

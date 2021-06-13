# Drag

Drag is a graph drawing library I implemented for my [bachelor's thesis](https://is.muni.cz/th/kmkvd/?lang=en).

## What exactly does it do?

Well let me tell you. In a nutshell, you provide a directed graph and the library produces a nice looking layout of that graph in 2D space. However, there is an important thing to note:
 
 * The library *only* produces the abstract layout. What does that mean, you might ask. Well, It means the library calculates coordinates for all the vertices and control points of the edges (which, for simplicity, are represented as polylines) and that's all it does. Its purpouse is *not* to produce an image or to provide any styling. It is meant as a backend for such applications.

With that said, a simple application which converts the produced layout into an svg image is bundeled with the library. But it is extremely limited and serves only for demonstration purposes and it is not the main part of this project. If you want to know more about how to use it, go [here](TODO).
 
 ## What graph is it suitable for?
 
It should be used only for *directed* graphs. In theory one could use it to make a layout of an undirected graph by giving each edge an arbitrary direction. However, the resulting layout wouldn't make much sense and wouldn't convey the information contained in the graph very well. 
 



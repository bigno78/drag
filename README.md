# Drag

Drag is a graph drawing library for creating nice looking layouts of directed graphs. I originally implemented this library for my [bachelor's thesis](https://is.muni.cz/th/kmkvd/?lang=en). Since then, the API has changed but the underlying algorithms are still the same.

## What exactly does it do?

In a nutshell, you provide a directed graph and the library produces a nice looking layout of that graph in 2D space. That means, it computes the positions of all vertices in the graph and the points through which the edges (which, for simplicity, are represented as polylines) should be routed such that there are as few edge crossings as possible.

It is important to note that this is the main purpose of the library. It computes a layout. It is not meant for converting this layout into an image or styling this image. It is meant as a backend for such applications.

Nevertheless, it contains a simple drawing api for creating quick and dirty svg images of the produced layouts. However, it is very limited in what it can do. There is also an example command-line application which uses this api. If you want to know more about that read this [section](#producing-svg-images).

## Can I see some examples please? 

Sure thing. Check them out.

![Layout of a tree](assets/tree.svg "Layout of a tree")
![Layout of a dag](assets/graph.svg "Layout of a dag")

## What graphs is it suitable for?

The short answer is *directed* *acyclic* graphs. They can be *disconnected* in which case each connected component is layed out separately.

These requirements come from the fact that the library produces layered layouts. That means the vertices are placed on horizontal lines and all edges are pointed downwards. You can see exactly that in the images above. 

This layout is most suitable for graphs representing some sort of hierarchy or ordering - i.e. *directed* *acyclic* graphs. Such a layout makes it easy to see that some vertices are in some sense *superior* to others just by looking if they are above or below. It also makes it easier to trace paths from a vertex since these will always go downwards.

Nevertheless, the requirement for the graph to be *acyclic* is not strict. The library can cope with cycles but the produced images might not be ideal since this kind of layout is not suitable for cycles - some edges must point upwards and this defeats the purpose of the layout. For small number of cycles it can be fine but the more cycles the worse it gets.

## How to use the library in your project

The library is header-only so using it is very simple,, the only requirement is a compiler supporting `c++17`. There are couple of options how to include the library in your project.

One way is to just grab the `drag` subdirectory from the `include` directory and place it in your project. Then you can happily include the headers that you need. The two most important ones are `drag/drag.hpp` which includes everything necessary to create a layout and `drag/drawing/draw.hpp` which contains the svg drawing interface.

The second option is to use `cmake`. To do that, grab the whole repository and place it inside your project. Then in your `CMakeLists.txt` you can add the library as a subdirectory and link to it like so:

```CMake
add_subdirectory(drag)

add_executable(your-target your-source.cpp)
target_link_libraries(your-target drag)
```

This will ensure that `your-target` has the proper include directories set and is compiled with `c++17`.

## Getting started

### Creating the input graph

First thing to do is to convert whatever you are trying to draw as a graph (be it your own graph representation or some data) into the graph representation of the library, i.e. you need to create an instance of `graph`. This can be done by creating nodes and adding edges between them.

```C++
drag::graph g;

auto u = g.add_node();
auto v = g.add_node();

g.add_edge(u, v);
```

Concrete example of constructing a graph is for example the implementation of `graph_builder` in `graph.hpp`.

Besides the structure of the graph the library also needs to know the desired parameters of the layout. An important thing to note is that the library assumes all nodes are circles of the same radius. So if you want to place some content inside of the nodes, their size should be set such that the content will fit.

The layout parameters include
 * `node_size` - radius of all nodes
 * `node_dist` - minimal distance between the borders of two nodes on the same layer
 * `layer_dist` - minimal distance between the borders of nodes on two neighboring layers 
 * `loop_angle` - the angle from the x-axis at which loops connect to nodes
 * `loop_size` - how far the loops extend from nodes

You can either set them yourself or just don't do anything and use the default values.

```C++
g.node_size = 15;
g.layer_dist = 20;
// ...
```

### Creating and using the layout

To create a layout simply create an instance of `sugiyama_layout`. The whole computation of the layout is done in the constructor.

```C++
drag::sygiyama_layout layout(g);
```

The resulting layout can then be accessed through the interface of `sugiyama_layout`. General usage pattern might look something like this.

```C++
for (auto node : layout.vertices()) {
    // use the information in `node` to draw it
}

for (auto edge : layout.edges()) {
    // draw the edge
}
```

`layout.vertices()` returns a vector of nodes. The `node` structure looks as follows.

```C++
struct node {
    vertex_t u;  // the corresponding vertex identifier
    vec2 pos;    // the position in space
    float size;  // the radius
};
```

Similarly `layout.edges()` returns a vector of the edges represented using the `path` structure.

```C++
struct path {
    vertex_t from, to;           // the vertex identifiers of endpoints of the corresponding edge
    std::vector< vec2 > points;  // control points of the poly-line representing the edge
    bool bidirectional = false;  // is the edge bidirectional?
};
```

The first and last point of the paths are computed such they lie on the border of the corresponding points.

A concrete example of using the layout can be seen in the implementation of the svg api in `draw.hpp`.

## Producing SVG images

TODO

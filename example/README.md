## Building the examples

The examples can be built using  a normal cmake workflow.

First you generate the configuration files.

```Bash
$ git clone https://github.com/bigno78/drag.git
$ cd drag
$ mkdir build && cd build
$ cmake ..
```

Then you can build all of the examples.

```Bash
$ cd examples
$ make
```

Or just a single one.

```Bash
$ cd examples/example-dir
$ make
```

### Windows

One way to build the examples on windows is to open the cmake project directly in visual studio as a folder. Then you can pick any of the targets defined and build them.

Alternatively you can generate the cmake build configuration as above.

```
> git clone https://github.com/bigno78/drag.git
> cd drag
> mkdir build && cd build
> cmake ..
```

This will create a visual studio project. You can then open it in visual studio and build the desired targets.

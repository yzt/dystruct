DyStruct
========

DyStruct, or dynamic structs, is a C++ library for efficient, runtime-composed data structures. It hopefully will provide access, reflection, s11n, data morphing (assigning instances of data structures to each other that are similar, but not exactly the same,) etc.

Introduction
------------

All this happens at runtime:

Using DyStruct library, you first describe your data structures (e.g. I want a structure named "Foo" of three floats named "x", "y" and "z", and a 12 element array of integers named "a".) This is a `DyStruct::Type`.

Then you compile this description and get a `DyStruct::CompiledType`.

Form this `CompiledType`, and using a field's name, you can get field and element `Accessor`s (explained below.) Note that accessors are not bound to any instance of a CompiledType. Creating an accessor is not cheap (we need to search into the type description to find the field you are interested in,) but the accessor objects themselves are very lightweight (usually only a 32-bit offset.)

Having a `CompiledType`, you can also create instances of that structure. Creating a new instance will give you an `InstancePtr`, which internally contains two pointers: one to the allocated instance data on the heap, and another to the `DyStruct::Type` that describes the instance. Freeing an instance is your responsibility. The heap-allocated instance itself does not contain any extra data; its size is exactly equal to the sum of the sizes of the data elements that make up that instance.

With an `InstancePtr` and an `Accessor`, you can access the field you want inside that instance. This operation is extremely fast (almost as fast as accessing a field inside a heap-allocated C `struct`.)

To work with DyStruct data, you typically create your type descriptions and compile them in specific places in your codebase, for example: during initialization, or when loading configuration file and data files for the first time. You also create your accessors outside of any critical loops and functions. The part that remains is reading and writing the fields that is very fast.

Another use case is to create DyStruct descriptions for existing C structs. This allows you to access instances of your structs using `DyStruct::InstancePtr`s. While you probably won't want to do this normally, it will give you the option of having the DyStruct facilities that C++ structs don't provide, like serialization and introspection.


Building
--------

To build, you need to have the *Premake4* build system generator. Use the premake4.lua in the scripts directory to generate your favorite build system (e.g. makefile, VS project file, etc.) Currently, nothing but Windows/VC is tested though.


# cinja

Cinja compiles a small subset of [Jinja](http://jinja.pocoo.org/) to C++. The
generated code writes the instantiated (rendered) template to an output stream.
Currently only the most basic features of Jinja are supported and code
generation is very rudimentary.

## Example

An example that shows how to use generated code to build a simple web
application is provided in the [respective directory](example).  It depends on the
[pion](https://github.com/splunk/pion) network library.

Building and starting it on `localhost:8080`:
```
$ mkdir example/build
$ cd example/build
$ cmake ..
$ make
$ piond -p 8080 / example
```

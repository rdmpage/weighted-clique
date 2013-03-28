weighted-clique
===============


C++ program that implements Feitelson's weighted clique algorithm.

#### Overview
This is a C++ program to implement Dror G. Feitelson's weighted clique algorithm described in [On identifying name equivalences in digital libraries](http://informationr.net/ir/9-4/paper192.html). For more details see [Equivalent author names](http://iphylo.blogspot.co.uk/2009/01/equivalent-author-names.html).

#### Dependencies
The program uses the Graph Template Library (GTL) which is available from [http://www.fim.uni-passau.de/fileadmin/files/lehrstuhl/brandenburg/projekte/gtl/GTL-1.2.4-lgpl.tar.gz](http://www.fim.uni-passau.de/fileadmin/files/lehrstuhl/brandenburg/projekte/gtl/GTL-1.2.4-lgpl.tar.gz)

#### Building
If you are building from this repository you will need to do the standard things:

* aclocal
* autoconf
* automake
* ./configure
* make

#### Example

The examples folder contains a example graph:

![Example](https://github.com/rdmpage/weighted-clique/raw/master/examples/abc.png)

Running the program on this graph

* equivalent examples/bipartite.gml

outputs the matching in JSON. The source nodes in the graph are numbered 0 - m, the target nodes m+1 - n.

```javascript
{
"clusters":[
[ "A B C", "Abe Bob C", "Abe B" ],
[ "Ace D E", "A D" ],
[ "Abe F G" ],[ "A" ]]
}
```
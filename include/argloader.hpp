/**
 * @file   argloader.hpp
 * @author P. Foggia (pfoggia\@unisa.it)
 * @author V.Carletti (vcarletti\@unisa.it)
 * @date   December, 2014
 * @see argraph.hpp
 * @brief  Definition of a simple ARG loader which allows graph edit operations.
 * @details DESCRIPTION OF THE BINARY FILE FORMAT \n
 * The file is composed by a sequence of 16-bit words; the words are
 * encoded in little-endian format (e.g., LSB first).\n
 * The first word represents the number of nodes in the graph.
 * Then, for each node, there is a word encoding the number of
 * edges coming out of that node, followed by a sequence of words
 * encoding the endpoints of those edges.\n
 * An example, represented in hexadecimal, follows:\n
 *     03 00     Number of nodes (3)\n
 *     00 00     Number of edges out of node 0 (0)\n
 *     02 00     Number of edges out of node 1 (2)\n
 *     00 00     Target of the first edge of node 1 (edge 1 -> 0)\n
 *     02 00     Target of the second edge of node 1 (edge 1 -> 2)\n
 *     01 00     Number of edges out of node 2 (1)\n
 *     00 00     Target of the first (and only) edge of node 2 (edge 2 -> 0)\n
*/

#ifndef ARGLOADER_H
#define ARGLOADER_H

#include <cstddef>

#include <iostream>
#include <string>
#include <strstream>
#include <map>
#include <ctype.h>
#include <cstddef>
#include "argedit.hpp"

using namespace std;

/**
 * @class StreamARGLoader
 * @brief ARGLoader for text file format.
 * 
 * DESCRIPTION OF THE TEXT FILE FORMAT
 * ===================================
 *
 * On the first line there must be the number of nodes;
 * subsequent lines will contain the node attributes, one node per 
 * line, preceded by the node id; node ids must be in the range from
 * 0 to the number of nodes - 1.\n
 * Then, for each node there is the number of edges coming out of 
 * the node, followed by a line for each edge containing the 
 * ids of the edge ends and the edge attribute.\n
 * Blank lines, and lines starting with #, are ignored.
 * An example file, where both node and edge attributes are ints, 
 * could be the following:
 *
 *	\# Number of nodes\n
 *	3\n
 *
 *	\# Node attributes\n
 *	0 27\n
 * 	1 42\n
 *	2 13\n
 *
 *	\# Edges coming out of node 0\n
 *	2\n
 *	0 1  24\n
 *	0 2  73\n
 * 
 *	\# Edges coming out of node 1\n
 *	1\n
 *	1 3  66\n
 *
 *	\# Edges coming out of node 2\n
 *	0\n
 * 
 */
template <typename Node, typename Edge>
class StreamARGLoader: public ARGEdit<Node, Edge>
  { 
    public:

      StreamARGLoader(istream &in);

      static void write(ostream &out, ARGraph<Node, Edge> &g);
      static void write(ostream &out, ARGLoader<Node,Edge> &g);

    private:

      enum { MAX_LINE=512 };
      void readLine(istream &in, char *line);
      int  readCount(istream &in);
      void readNode(istream &in);
      void readEdge(istream &in);
  };

/*------------------------------------------------------------
 * Methods of the class StreamArgLoader
 -----------------------------------------------------------*/

/**
* @brief Constructor 
* @param [in] in Input stream
*/
template <typename Node, typename Edge>
StreamARGLoader<Node, Edge>::
StreamARGLoader(istream &in)
  { 
    int cnt=readCount(in);
    if (cnt<=0)
      { cnt=0;
        return;
      }

    int i;
    for(i=0; i<cnt; i++)
      {
        readNode(in);
      }

    for(i=0; i<cnt; i++)
      { int ecount, j;
        ecount=readCount(in);
        for(j=0; j<ecount; j++)
          readEdge(in);
      }
        
  }

/**
* @brief Reads a line of the stream 
* @param [in] in Input stream.
* @param [out] line Read line. You should not allocate this.
*/
template <typename Node, typename Edge>
void StreamARGLoader<Node, Edge>::
readLine(istream &in, char *line)
  { 
    char *p;
    do {
      *line='\0';
      if (!in.good())
        error("End of file or reading error");
      in.getline(line, MAX_LINE);
      for(p=line; isspace(*p); p++)
        ;
    } while (*p=='\0' || *p=='#');
  }

/**
* @brief Reads an int from a line 
* @param [in] in Input stream.
* @returns Integer read.
*/
template <typename Node, typename Edge>
int StreamARGLoader<Node, Edge>::
readCount(istream &in)
  { char line[MAX_LINE+1];
    readLine(in, line);
    
    int i;
    istrstream is(line);
    is>>i;

    return i;
  }

/**
* @brief Loads a node from a line.
* @param [in] in Input stream.
*/
template <typename Node, typename Edge>
void StreamARGLoader<Node, Edge>::
readNode(istream &in)
  {
    char line[MAX_LINE+1];
    readLine(in, line);
    istrstream is(line);
    
    Node nattr;
    node_id id;

    is >> id >> nattr;

    if (id != this->NodeCount())
      error("File format error\n  Line: %s", line);

    this->InsertNode(nattr);
  }

/**
* @brief Loads an edge from a line
* @param [in] in Input stream.
*/
template <typename Node, typename Edge>
void StreamARGLoader<Node, Edge>::
readEdge(istream &in)
  {
    char line[MAX_LINE+1];
    readLine(in, line);
    istrstream is(line);
    
    Edge eattr;
    node_id id1, id2;

    is >> id1 >> id2 >> eattr;

    this->InsertEdge(id1, id2, eattr);
  }

/**
* @brief Writes an ARGraph on a stream in a format readable by StreamARGLoader.\n
* Relies on stream output operators for the Node and Edge types.
* @param [in] out Output stream.
* @param [in] g ARGraph to write.
*/
template <typename Node, typename Edge>
void StreamARGLoader<Node,Edge>::
write(ostream &out, ARGraph<Node, Edge> &g)
  { out << g.NodeCount() << endl;

    int i;
    for(i=0; i<g.NodeCount(); i++)
      out << i << ' ' << *g.GetNodeAttr(i) << endl;

    int j;
    for(i=0; i<g.NodeCount(); i++)
      { out << g.OutEdgeCount(i) << endl;
        for(j=0; j<g.OutEdgeCount(i); j++)
          { int k;
            Edge *attr;
            k=g.GetOutEdge(i, j, &attr);
            out << i << ' ' << k << ' ' << *attr << endl;
          }
      }
  }

/**
* @brief Writes an ARGLoader on a stream in a format readable by StreamARGLoader.\n
* Relies on stream output operators for the Node and Edge types.
* @param [in] out Output stream.
* @param [in] g ARGLoader to write.
*/
template <typename Node, typename Edge>
void StreamARGLoader<Node,Edge>::
write(ostream &out, ARGLoader<Node,Edge> &g)
  { out << g.NodeCount() << endl;

    int i;
    for(i=0; i<g.NodeCount(); i++)
      out << i << ' ' << *(Node *)g.GetNodeAttr(i) << endl;

    int j;
    for(i=0; i<g.NodeCount(); i++)
      { out << g.OutEdgeCount(i) << endl;
        for(j=0; j<g.OutEdgeCount(i); j++)
          { int k;
            void *attr;
            k=g.GetOutEdge(i, j, &attr);
            out << i << ' ' << k << ' ' << *(Edge *)attr << endl;
          }
      }
  }
  


class BinaryGraphLoader : public ARGEdit<int,Empty> {
 public:
  BinaryGraphLoader(istream & in) {
    int cnt = read_word(in);
    if(cnt <= 0) {
      cnt = 0;
      return;
    }
    int i;
    for(i=0; i<cnt; ++i) {
      int nattr = 0;
      this->InsertNode(nattr);
    }
    for(i=0; i<cnt; ++i) {
      int ecount, j;
      ecount = read_word(in);
      for(j=0; j<ecount; ++j) {
        node_id v = read_word(in);
        Empty eattr;
        this->InsertEdge(i,v,eattr);
      }
    }
  }
 private:
  uint16_t read_word(std::istream & in) {
    uint16_t x = static_cast<unsigned char>(in.get());
    x |= static_cast<uint16_t>(in.get()) << 8;
    return x;
  }
};

template <typename Node>
class GFUGraphLoader : public ARGEdit<Node,Empty> {
 public:
  GFUGraphLoader(istream & in) {
    std::string name;
    in >> name;
    int cnt;
    in >> cnt;
    
    for (int i=0; i<cnt; ++i) {
      Node nattr;
      in >> nattr;
      this->InsertNode(nattr);
    }
    
    std::size_t ecnt;
    in >> ecnt;
    for (std::size_t i=0; i<ecnt; ++i) {
      node_id u;
      node_id v;
      in >> u >> v;
      Empty eattr;
      this->InsertEdge(u, v, eattr);
      this->InsertEdge(v, u, eattr);
    }
  }
};

#endif

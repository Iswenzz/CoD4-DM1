#pragma once
#include "Msg.hpp"

#define HMAX 256					/* Maximum symbol */
#define NYT HMAX                    /* NYT = Not Yet Transmitted */
#define INTERNAL_NODE (HMAX + 1)

namespace Iswenzz::CoD4::DM1
{
	/// <summary>
	/// Huffman tree node.
	/// </summary>
	typedef struct nodetype
	{
		struct nodetype* left, *right, *parent;			/* tree structure */
		struct nodetype* next, *prev;					/* doubly-linked list */
		struct nodetype** head;							/* highest ranked node in block */
		int	weight;
		int	symbol;
	} node_t;

	/// <summary>
	/// Huffman tree.
	/// </summary>
	typedef struct
	{
		int	blocNode;
		int	blocPtrs;

		node_t* tree;
		node_t* lhead;
		node_t* ltail;
		node_t* loc[HMAX + 1];
		node_t** freelist;

		node_t nodeList[768];
		node_t* nodePtrs[768];
	} huff_t;

	/// <summary>
	/// Huffman trees used in the program.
	/// </summary>
	typedef struct
	{
		huff_t compressor;
		huff_t decompressor;
	} huffman_t;

	/// <summary>
	/// This is based on the Adaptive Huffman algorithm described in Sayood's Data Compression book. 
	/// The ranks are not actually stored, but implicitly defined by the location 
	/// of a node within a doubly linked list.
	/// </summary>
	class Huffman
	{
	public:
		/// <summary>
		/// Initialize the huffman code.
		/// </summary>
		static void InitMain();

		/// <summary>
		/// Decompress a message.
		/// </summary>
		/// <param name="bufIn">The message buffer.</param>
		/// <param name="lenIn">The message buffer length.</param>
		/// <param name="bufOut">The output buffer.</param>
		/// <param name="lenOut">The output buffer length.</param>
		/// <returns></returns>
		static int Decompress(unsigned char* bufIn, int lenIn, unsigned char* bufOut, int lenOut);
		
		/// <summary>
		/// Compress a message.
		/// </summary>
		/// <param name="bufIn">The message buffer.</param>
		/// <param name="lenIn">The message buffer length.</param>
		/// <param name="bufOut">The output buffer.</param>
		/// <param name="lenOut">The output buffer length.</param>
		/// <returns></returns>
		static int Compress(unsigned char* bufIn, int lenIn, unsigned char* bufOut, int lenOut);

		/// <summary>
		/// Put bit at offset.
		/// </summary>
		/// <param name="bit">Bit value.</param>
		/// <param name="fout">Output buffer.</param>
		/// <param name="offset">The offset to add the bit to.</param>
		static void Huff_PutBit(int bit, unsigned char* fout, int* offset);

		/// <summary>
		/// Receive one bit from the input file (buffered).
		/// </summary>
		/// <param name="fin">The input buffer.</param>
		/// <param name="offset">The offset to get the bit to.</param>
		/// <returns></returns>
		static int Huff_GetBit(unsigned char* fin, int* offset);

		/// <summary>
		/// Add new nodes to the huffman tree.
		/// </summary>
		/// <param name="huff">The huffman tree.</param>
		/// <param name="ch">Unique key.</param>
		static void Huff_AddRef(huff_t* huff, unsigned char ch);

		/// <summary>
		/// Get a symbol.
		/// </summary>
		/// <param name="node">The huffman tree node.</param>
		/// <param name="ch"></param>
		/// <param name="fin">The input buffer.</param>
		/// <returns></returns>
		static int Huff_Receive(node_t* node, int* ch, unsigned char* fin);

		/// <summary>
		/// Get a symbol with a specific offset.
		/// </summary>
		/// <param name="node">The huffman tree node.</param>
		/// <param name="ch"></param>
		/// <param name="fin">The input buffer.</param>
		/// <param name="offset">The offset to use to receive.</param>
		static void Huff_OffsetReceive(node_t* node, int* ch, unsigned char* fin, int* offset);

		/// <summary>
		/// Send a symbol.
		/// </summary>
		/// <param name="huff">The huffman tree.</param>
		/// <param name="ch"></param>
		/// <param name="fout">The output buffer.</param>
		static void Huff_Transmit(huff_t* huff, int ch, unsigned char* fout);

		/// <summary>
		/// Send a symbol to a specific offset.
		/// </summary>
		/// <param name="huff">The huffman tree.</param>
		/// <param name="ch"></param>
		/// <param name="fout">The output buffer.</param>
		/// <param name="offset">The offset to transmit to.</param>
		static void Huff_OffsetTransmit(huff_t* huff, int ch, unsigned char* fout, int* offset);

		/// <summary>
		/// Add a bit to the output file (buffered).
		/// </summary>
		/// <param name="bit">The bit to add.</param>
		/// <param name="fout">The output buffer.</param>
		static void AddBit(char bit, unsigned char* fout);

		/// <summary>
		/// Get a bit.
		/// </summary>
		/// <param name="fin">The input buffer.</param>
		/// <returns></returns>
		static int GetBit(unsigned char* fin);

		/// <summary>
		/// Get a pp node.
		/// </summary>
		/// <param name="huff">The huffman tree.</param>
		/// <returns></returns>
		static node_t** GetPPNode(huff_t* huff);

		/// <summary>
		/// Free a pp node.
		/// </summary>
		/// <param name="huff">The huffman tree.</param>
		/// <param name="ppnode">The pp node to free.</param>
		static void FreePPNode(huff_t* huff, node_t** ppnode);

		/// <summary>
		/// Swap the location of these two nodes in the tree.
		/// </summary>
		/// <param name="huff">Huffman tree.</param>
		/// <param name="node1">First node.</param>
		/// <param name="node2">Second node.</param>
		static void Swap(huff_t* huff, node_t* node1, node_t* node2);

		/// <summary>
		/// Swap these two nodes in the linked list (update ranks).
		/// </summary>
		/// <param name="node1">First node.</param>
		/// <param name="node2">Second node.</param>
		static void SwapList(node_t* node1, node_t* node2);

		/// <summary>
		/// Increment the node in the huffman tree.
		/// </summary>
		/// <param name="huff">The huffman tree.</param>
		/// <param name="node">The huffman node.</param>
		static void Increment(huff_t* huff, node_t* node);

		/// <summary>
		/// Send the prefix code for this node.
		/// </summary>
		/// <param name="node"></param>
		/// <param name="child"></param>
		/// <param name="fout"></param>
		static void Send(node_t* node, node_t* child, unsigned char* fout);

		/// <summary>
		/// Find the lowest key in a specific key array.
		/// </summary>
		/// <param name="dataDone">The key array that were used.</param>
		/// <param name="data">The key array.</param>
		/// <returns></returns>
		static int FindLowest(int* dataDone, int* data);

		/// <summary>
		/// Initialize the huffman trees.
		/// </summary>
		/// <param name="huff"></param>
		static void Huff_Init(huffman_t* huff);

		/// <summary>
		/// Initialize the huffman trees for COD4.
		/// </summary>
		static void Init_COD4();

		/// <summary>
		/// Initialize the huffman trees for Q3.
		/// </summary>
		static void Init_Q3();
	};
}

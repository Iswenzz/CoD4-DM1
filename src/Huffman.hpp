#pragma once
#include "Msg.hpp"

#define HMAX 256					/* Maximum symbol */
#define NYT HMAX                    /* NYT = Not Yet Transmitted */
#define INTERNAL_NODE (HMAX + 1)

namespace Iswenzz
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
		/// 
		/// </summary>
		/// <param name="bufIn"></param>
		/// <param name="lenIn"></param>
		/// <param name="bufOut"></param>
		/// <param name="lenOut"></param>
		/// <returns></returns>
		static int Decompress(unsigned char* bufIn, int lenIn, unsigned char* bufOut, int lenOut);
		
		/// <summary>
		/// 
		/// </summary>
		/// <param name="bufIn"></param>
		/// <param name="lenIn"></param>
		/// <param name="bufOut"></param>
		/// <param name="lenOut"></param>
		/// <returns></returns>
		static int Compress(unsigned char* bufIn, int lenIn, unsigned char* bufOut, int lenOut);

		/// <summary>
		/// 
		/// </summary>
		/// <param name="bit"></param>
		/// <param name="fout"></param>
		/// <param name="offset"></param>
		static void Huff_PutBit(int bit, unsigned char* fout, int* offset);

		/// <summary>
		/// Receive one bit from the input file (buffered).
		/// </summary>
		/// <param name="fin"></param>
		/// <param name="offset"></param>
		/// <returns></returns>
		static int Huff_GetBit(unsigned char* fin, int* offset);

		/// <summary>
		/// 
		/// </summary>
		/// <param name="huff"></param>
		/// <param name="ch"></param>
		static void Huff_AddRef(huff_t* huff, unsigned char ch);

		/// <summary>
		/// Get a symbol.
		/// </summary>
		/// <param name="node"></param>
		/// <param name="ch"></param>
		/// <param name="fin"></param>
		/// <returns></returns>
		static int Huff_Receive(node_t* node, int* ch, unsigned char* fin);

		/// <summary>
		/// Get a symbol.
		/// </summary>
		/// <param name="node"></param>
		/// <param name="ch"></param>
		/// <param name="fin"></param>
		/// <param name="offset"></param>
		static void Huff_OffsetReceive(node_t* node, int* ch, unsigned char* fin, int* offset);

		/// <summary>
		/// Send a symbol.
		/// </summary>
		/// <param name="huff"></param>
		/// <param name="ch"></param>
		/// <param name="fout"></param>
		static void Huff_Transmit(huff_t* huff, int ch, unsigned char* fout);

		/// <summary>
		/// 
		/// </summary>
		/// <param name="huff"></param>
		/// <param name="ch"></param>
		/// <param name="fout"></param>
		/// <param name="offset"></param>
		static void Huff_OffsetTransmit(huff_t* huff, int ch, unsigned char* fout, int* offset);

		/// <summary>
		/// Add a bit to the output file (buffered).
		/// </summary>
		/// <param name="bit"></param>
		/// <param name="fout"></param>
		static void AddBit(char bit, unsigned char* fout);

		/// <summary>
		/// 
		/// </summary>
		/// <param name="fin"></param>
		/// <returns></returns>
		static int GetBit(unsigned char* fin);

		/// <summary>
		/// 
		/// </summary>
		/// <param name="huff"></param>
		/// <returns></returns>
		static node_t** GetPPNode(huff_t* huff);

		/// <summary>
		/// 
		/// </summary>
		/// <param name="huff"></param>
		/// <param name="ppnode"></param>
		static void FreePPNode(huff_t* huff, node_t** ppnode);

		/// <summary>
		/// Swap the location of these two nodes in the tree.
		/// </summary>
		/// <param name="huff"></param>
		/// <param name="node1"></param>
		/// <param name="node2"></param>
		static void Swap(huff_t* huff, node_t* node1, node_t* node2);

		/// <summary>
		/// Swap these two nodes in the linked list (update ranks).
		/// </summary>
		/// <param name="node1"></param>
		/// <param name="node2"></param>
		static void SwapList(node_t* node1, node_t* node2);

		/// <summary>
		/// 
		/// </summary>
		/// <param name="huff"></param>
		/// <param name="node"></param>
		static void Increment(huff_t* huff, node_t* node);

		/// <summary>
		/// Send the prefix code for this node.
		/// </summary>
		/// <param name="node"></param>
		/// <param name="child"></param>
		/// <param name="fout"></param>
		static void Send(node_t* node, node_t* child, unsigned char* fout);

		/// <summary>
		/// 
		/// </summary>
		/// <param name="dataDone"></param>
		/// <param name="data"></param>
		/// <returns></returns>
		static int FindLowest(int* dataDone, int* data);

		/// <summary>
		/// 
		/// </summary>
		/// <param name="huff"></param>
		static void Huff_Init(huffman_t* huff);

		/// <summary>
		/// 
		/// </summary>
		static void Init_COD4();

		/// <summary>
		/// 
		/// </summary>
		static void Init_Q3();
	};
}

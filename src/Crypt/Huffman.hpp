#pragma once
#include "Msg.hpp"

#define HMAX 256 // Maximum symbol
#define NYT HMAX // Not Yet Transmitted
#define INTERNAL_NODE (HMAX + 1)

namespace CoD4::DM1
{
	struct node_t
	{
		node_t *left, *right, *parent; // tree structure
		node_t *next, *prev;			  // doubly-linked list
		node_t** head;				  // highest ranked node in block
		int weight;
		int symbol;
	};

	struct huff_t
	{
		int blocNode;
		int blocPtrs;

		node_t* tree;
		node_t* lhead;
		node_t* ltail;
		node_t* loc[HMAX + 1];
		node_t** freelist;

		node_t nodeList[768];
		node_t* nodePtrs[768];
	};

	struct huffman_t
	{
		huff_t compressor;
		huff_t decompressor;
	};

	/// <summary>
	/// This is based on the Adaptive Huffman algorithm described in Sayood's Data Compression book.
	/// The ranks are not actually stored, but implicitly defined by the location of a node within a doubly linked list.
	/// </summary>
	class Huffman
	{
	public:
		static void InitMain();
		static void Init_COD4();
		static void Init_Q3();

		static int Decompress(uint8_t* bufIn, int lenIn, uint8_t* bufOut, int lenOut);
		static int Compress(uint8_t* bufIn, int lenIn, uint8_t* bufOut, int lenOut);

		static void Huff_Init(huffman_t* huff);
		static void Huff_PutBit(int bit, uint8_t* fout, int* offset);
		static int Huff_GetBit(uint8_t* fin, int* offset);
		static void Huff_AddRef(huff_t* huff, uint8_t ch);
		static int Huff_Receive(node_t* node, int* ch, uint8_t* fin);
		static void Huff_OffsetReceive(node_t* node, int* ch, uint8_t* fin, int* offset);
		static void Huff_Transmit(huff_t* huff, int ch, uint8_t* fout);
		static void Huff_OffsetTransmit(huff_t* huff, int ch, uint8_t* fout, int* offset);

		static void AddBit(char bit, uint8_t* fout);
		static int GetBit(uint8_t* fin);
		static node_t** GetPPNode(huff_t* huff);
		static void FreePPNode(huff_t* huff, node_t** ppnode);
		static void Swap(huff_t* huff, node_t* node1, node_t* node2);
		static void SwapList(node_t* node1, node_t* node2);
		static void Increment(huff_t* huff, node_t* node);
		static void Send(node_t* node, node_t* child, uint8_t* fout);
		static int FindLowest(int* dataDone, int* data);
	};
}

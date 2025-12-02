#include "Huffman.hpp"

namespace CoD4::DM1
{
	static huffman_t msgHuff;
	static int bloc = 0;
	static int msg_hDataDone[HMAX];

	int msg_hData_COD4[HMAX] = { 274054, 68777, 40460, 40266, 48059, 39006, 48630, 27692, 17712, 15439, 12386, 10758,
		9420, 9979, 9346, 15256, 13184, 14319, 7750, 7221, 6095, 5666, 12606, 7263, 7322, 5807, 11628, 6199, 7826, 6349,
		7698, 9656, 28968, 5164, 13629, 6058, 4745, 4519, 5199, 4807, 5323, 3433, 3455, 3563, 6979, 5229, 5002, 4423,
		14108, 13631, 11908, 11801, 10261, 7635, 7215, 7218, 9353, 6161, 5689, 4649, 5026, 5866, 8002, 10534, 15381,
		8874, 11798, 7199, 12814, 6103, 4982, 5972, 6779, 4929, 5333, 3503, 4345, 6098, 14117, 16440, 6446, 3062, 4695,
		3085, 4198, 4013, 3878, 3414, 5514, 4092, 3261, 4740, 4544, 3127, 3385, 7688, 11126, 6417, 5297, 4529, 6333,
		4210, 7056, 4658, 6190, 3512, 2843, 3479, 9369, 5203, 4980, 5881, 7509, 4292, 6097, 5492, 4648, 2996, 4988,
		4163, 6534, 4001, 4342, 4488, 6039, 4827, 7112, 8654, 26712, 8688, 9677, 9368, 7209, 3399, 4473, 4677, 11087,
		4094, 3404, 4176, 6733, 3702, 11420, 4867, 5968, 3475, 3722, 3560, 4571, 2720, 3189, 3099, 4595, 4044, 4402,
		3889, 4989, 3186, 3153, 5387, 8020, 3322, 3775, 2886, 4191, 2879, 3110, 2576, 3693, 2436, 4935, 3017, 3538,
		5688, 3444, 3410, 9170, 4708, 3425, 3273, 3684, 4564, 6957, 4817, 5224, 3285, 3143, 4227, 5630, 6053, 5851,
		6507, 13692, 8270, 8260, 5583, 7568, 4082, 3984, 4574, 6440, 3533, 2992, 2708, 5190, 3889, 3799, 4582, 6020,
		3464, 4431, 3495, 2906, 2243, 3856, 3321, 8759, 3928, 2905, 3875, 4382, 3885, 5869, 6235, 10685, 4433, 4639,
		4305, 4683, 2849, 3379, 4684, 5477, 4127, 3853, 3515, 4913, 3601, 5237, 6617, 9019, 4857, 4112, 5180, 5998,
		4925, 4986, 6365, 7930, 5948, 8085, 7732, 8643, 8901, 9653, 32647 };

	int msg_hData_Q3[HMAX] = { 250315, 41193, 6292, 7106, 3730, 3750, 6110, 23283, 33317, 6950, 7838, 9714, 9257, 17259,
		3949, 1778, 8288, 1604, 1590, 1663, 1100, 1213, 1238, 1134, 1749, 1059, 1246, 1149, 1273, 4486, 2805, 3472,
		21819, 1159, 1670, 1066, 1043, 1012, 1053, 1070, 1726, 888, 1180, 850, 960, 780, 1752, 3296, 10630, 4514, 5881,
		2685, 4650, 3837, 2093, 1867, 2584, 1949, 1972, 940, 1134, 1788, 1670, 1206, 5719, 6128, 7222, 6654, 3710, 3795,
		1492, 1524, 2215, 1140, 1355, 971, 2180, 1248, 1328, 1195, 1770, 1078, 1264, 1266, 1168, 965, 1155, 1186, 1347,
		1228, 1529, 1600, 2617, 2048, 2546, 3275, 2410, 3585, 2504, 2800, 2675, 6146, 3663, 2840, 14253, 3164, 2221,
		1687, 3208, 2739, 3512, 4796, 4091, 3515, 5288, 4016, 7937, 6031, 5360, 3924, 4892, 3743, 4566, 4807, 5852,
		6400, 6225, 8291, 23243, 7838, 7073, 8935, 5437, 4483, 3641, 5256, 5312, 5328, 5370, 3492, 2458, 1694, 1821,
		2121, 1916, 1149, 1516, 1367, 1236, 1029, 1258, 1104, 1245, 1006, 1149, 1025, 1241, 952, 1287, 997, 1713, 1009,
		1187, 879, 1099, 929, 1078, 951, 1656, 930, 1153, 1030, 1262, 1062, 1214, 1060, 1621, 930, 1106, 912, 1034, 892,
		1158, 990, 1175, 850, 1121, 903, 1087, 920, 1144, 1056, 3462, 2240, 4397, 12136, 7758, 1345, 1307, 3278, 1950,
		886, 1023, 1112, 1077, 1042, 1061, 1071, 1484, 1001, 1096, 915, 1052, 995, 1070, 876, 1111, 851, 1059, 805,
		1112, 923, 1103, 817, 1899, 1872, 976, 841, 1127, 956, 1159, 950, 7791, 954, 1289, 933, 1127, 3207, 1020, 927,
		1355, 768, 1040, 745, 952, 805, 1073, 740, 1013, 805, 1008, 796, 996, 1057, 11457, 13504 };

	void Huffman::InitMain()
	{
		Huff_Init(&msgHuff);
		Init_COD4();
	}

	void Huffman::Init_COD4()
	{
		memset(msg_hDataDone, 0, sizeof(msg_hDataDone));

		int i = FindLowest(msg_hDataDone, msg_hData_COD4);
		while (i != -1)
		{
			for (int j = 0; j < msg_hData_COD4[i]; j++)
			{
				Huff_AddRef(&msgHuff.compressor, static_cast<uint8_t>(i));
				Huff_AddRef(&msgHuff.decompressor, static_cast<uint8_t>(i));
			}
			msg_hDataDone[i] = 1;
			i = FindLowest(msg_hDataDone, msg_hData_COD4);
		}
	}

	void Huffman::Init_Q3()
	{
		for (int i = 0; i < HMAX; i++)
		{
			for (int j = 0; j < msg_hData_Q3[i]; j++)
			{
				Huff_AddRef(&msgHuff.compressor, static_cast<uint8_t>(i));
				Huff_AddRef(&msgHuff.decompressor, static_cast<uint8_t>(i));
			}
		}
	}

	int Huffman::Decompress(uint8_t* bufIn, int lenIn, uint8_t* bufOut, int lenOut)
	{
		lenIn *= 8;
		uint8_t* outptr = bufOut;

		int get;
		int offset;
		int i;

		if (lenIn <= 0)
			return 0;

		for (offset = 0, i = 0; offset < lenIn && i < lenOut; i++)
		{
			Huff_OffsetReceive(msgHuff.decompressor.tree, &get, bufIn, &offset);
			*outptr = static_cast<uint8_t>(get);
			outptr++;
		}
		return i;
	}

	int Huffman::Compress(uint8_t* bufIn, int lenIn, uint8_t* bufOut, int lenOut)
	{
		int offset;
		int i;

		if (lenIn <= 0)
			return 0;

		for (offset = 0, i = 0; i < lenIn; i++)
			Huff_OffsetTransmit(&msgHuff.compressor, static_cast<int>(bufIn[i]), bufOut, &offset);
		return (offset + 7) / 8;
	}

	void Huffman::Huff_Init(huffman_t* huff)
	{
		std::memset(&huff->compressor, 0, sizeof(huff_t));
		memset(&huff->decompressor, 0, sizeof(huff_t));

		// Initialize the tree & list with the NYT node
		huff->decompressor.tree = huff->decompressor.lhead = huff->decompressor.ltail = huff->decompressor.loc[NYT] =
			&(huff->decompressor.nodeList[huff->decompressor.blocNode++]);
		huff->decompressor.tree->symbol = NYT;
		huff->decompressor.tree->weight = 0;
		huff->decompressor.lhead->next = huff->decompressor.lhead->prev = nullptr;
		huff->decompressor.tree->parent = huff->decompressor.tree->left = huff->decompressor.tree->right = nullptr;

		// Add the NYT (not yet transmitted) node into the tree/list */
		huff->compressor.tree = huff->compressor.lhead = huff->compressor.loc[NYT] =
			&(huff->compressor.nodeList[huff->compressor.blocNode++]);
		huff->compressor.tree->symbol = NYT;
		huff->compressor.tree->weight = 0;
		huff->compressor.lhead->next = huff->compressor.lhead->prev = nullptr;
		huff->compressor.tree->parent = huff->compressor.tree->left = huff->compressor.tree->right = nullptr;
		huff->compressor.loc[NYT] = huff->compressor.tree;
	}

	void Huffman::Huff_PutBit(int bit, uint8_t* fout, int* offset)
	{
		bloc = *offset;
		if ((bloc & 7) == 0)
			fout[(bloc >> 3)] = 0;

		fout[(bloc >> 3)] |= bit << (bloc & 7);
		bloc++;
		*offset = bloc;
	}

	int Huffman::Huff_GetBit(uint8_t* fin, int* offset)
	{
		int t;
		bloc = *offset;
		t = (fin[(bloc >> 3)] >> (bloc & 7)) & 0x1;

		bloc++;
		*offset = bloc;
		return t;
	}

	void Huffman::AddBit(char bit, uint8_t* fout)
	{
		if ((bloc & 7) == 0)
			fout[(bloc >> 3)] = 0;

		fout[(bloc >> 3)] |= bit << (bloc & 7);
		bloc++;
	}

	int Huffman::GetBit(uint8_t* fin)
	{
		int t = (fin[(bloc >> 3)] >> (bloc & 7)) & 0x1;
		bloc++;
		return t;
	}

	node_t** Huffman::GetPPNode(huff_t* huff)
	{
		node_t** tppnode;
		if (!huff->freelist)
			return &(huff->nodePtrs[huff->blocPtrs++]);
		else
		{
			tppnode = huff->freelist;
			huff->freelist = reinterpret_cast<node_t**>(*tppnode);
			return tppnode;
		}
	}

	void Huffman::FreePPNode(huff_t* huff, node_t** ppnode)
	{
		*ppnode = reinterpret_cast<node_t*>(huff->freelist);
		huff->freelist = ppnode;
	}

	void Huffman::Swap(huff_t* huff, node_t* node1, node_t* node2)
	{
		node_t *par1, *par2;

		par1 = node1->parent;
		par2 = node2->parent;

		if (par1)
		{
			if (par1->left == node1)
				par1->left = node2;
			else
				par1->right = node2;
		}
		else
			huff->tree = node2;

		if (par2)
		{
			if (par2->left == node2)
				par2->left = node1;
			else
				par2->right = node1;
		}
		else
			huff->tree = node1;

		node1->parent = par2;
		node2->parent = par1;
	}

	void Huffman::SwapList(node_t* node1, node_t* node2)
	{
		node_t* par1;

		par1 = node1->next;
		node1->next = node2->next;
		node2->next = par1;

		par1 = node1->prev;
		node1->prev = node2->prev;
		node2->prev = par1;

		if (node1->next == node1)
			node1->next = node2;
		if (node2->next == node2)
			node2->next = node1;
		if (node1->next)
			node1->next->prev = node1;
		if (node2 && node2->next)
			node2->next->prev = node2;
		if (node1 && node1->prev)
			node1->prev->next = node1;
		if (node2 && node2->prev)
			node2->prev->next = node2;
	}

	void Huffman::Increment(huff_t* huff, node_t* node)
	{
		node_t* lnode;

		if (!node)
			return;

		if (node->next != nullptr && node->next->weight == node->weight)
		{
			lnode = *node->head;
			if (lnode != node->parent)
				Swap(huff, lnode, node);
			SwapList(lnode, node);
		}
		if (node->prev && node->prev->weight == node->weight)
			*node->head = node->prev;
		else
		{
			*node->head = nullptr;
			FreePPNode(huff, node->head);
		}
		node->weight++;

		if (node->next && node->next->weight == node->weight)
			node->head = node->next->head;
		else
		{
			node->head = GetPPNode(huff);
			*node->head = node;
		}
		if (node->parent)
		{
			Increment(huff, node->parent);
			if (node->prev == node->parent)
			{
				SwapList(node, node->parent);
				if (*node->head == node)
					*node->head = node->parent;
			}
		}
	}

	void Huffman::Huff_AddRef(huff_t* huff, uint8_t ch)
	{
		node_t *tnode, *tnode2;

		// if this is the first transmission of this node
		if (huff->loc[ch] == nullptr)
		{
			tnode = &(huff->nodeList[huff->blocNode++]);
			tnode2 = &(huff->nodeList[huff->blocNode++]);

			tnode2->symbol = INTERNAL_NODE;
			tnode2->weight = 1;
			tnode2->next = huff->lhead->next;

			if (huff->lhead->next)
			{
				huff->lhead->next->prev = tnode2;
				if (huff->lhead->next->weight == 1)
					tnode2->head = huff->lhead->next->head;
				else
				{
					tnode2->head = GetPPNode(huff);
					*tnode2->head = tnode2;
				}
			}
			else
			{
				tnode2->head = GetPPNode(huff);
				*tnode2->head = tnode2;
			}
			huff->lhead->next = tnode2;
			tnode2->prev = huff->lhead;

			tnode->symbol = ch;
			tnode->weight = 1;
			tnode->next = huff->lhead->next;

			if (huff->lhead->next)
			{
				huff->lhead->next->prev = tnode;
				if (huff->lhead->next->weight == 1)
					tnode->head = huff->lhead->next->head;
				else
				{
					// this should never happen
					tnode->head = GetPPNode(huff);
					*tnode->head = tnode2;
				}
			}
			else
			{
				// this should never happen
				tnode->head = GetPPNode(huff);
				*tnode->head = tnode;
			}
			huff->lhead->next = tnode;
			tnode->prev = huff->lhead;
			tnode->left = tnode->right = nullptr;

			if (huff->lhead->parent)
			{
				// lhead is guaranteed to by the NYT
				if (huff->lhead->parent->left == huff->lhead)
					huff->lhead->parent->left = tnode2;
				else
					huff->lhead->parent->right = tnode2;
			}
			else
				huff->tree = tnode2;

			tnode2->right = tnode;
			tnode2->left = huff->lhead;

			tnode2->parent = huff->lhead->parent;
			huff->lhead->parent = tnode->parent = tnode2;

			huff->loc[ch] = tnode;

			Increment(huff, tnode2->parent);
		}
		else
			Increment(huff, huff->loc[ch]);
	}

	int Huffman::Huff_Receive(node_t* node, int* ch, uint8_t* fin)
	{
		while (node && node->symbol == INTERNAL_NODE)
		{
			if (GetBit(fin))
				node = node->right;
			else
				node = node->left;
		}
		if (!node)
			return 0;
		return (*ch = node->symbol);
	}

	void Huffman::Huff_OffsetReceive(node_t* node, int* ch, uint8_t* fin, int* offset)
	{
		bloc = *offset;
		while (node && node->symbol == INTERNAL_NODE)
		{
			if (GetBit(fin))
				node = node->right;
			else
				node = node->left;
		}
		if (!node)
		{
			*ch = 0;
			return;
		}
		*ch = node->symbol;
		*offset = bloc;
	}

	void Huffman::Send(node_t* node, node_t* child, uint8_t* fout)
	{
		if (node->parent)
			Send(node->parent, node, fout);
		if (child)
		{
			if (node->right == child)
				AddBit(1, fout);
			else
				AddBit(0, fout);
		}
	}

	void Huffman::Huff_Transmit(huff_t* huff, int ch, uint8_t* fout)
	{
		// node_t hasn't been transmitted, Send a NYT, then the symbol
		if (huff->loc[ch] == nullptr)
		{
			Huff_Transmit(huff, NYT, fout);
			for (int i = 7; i >= 0; i--)
				AddBit(static_cast<char>(((ch >> i) & 0x1)), fout);
		}
		else
			Send(huff->loc[ch], nullptr, fout);
	}

	void Huffman::Huff_OffsetTransmit(huff_t* huff, int ch, uint8_t* fout, int* offset)
	{
		bloc = *offset;
		Send(huff->loc[ch], nullptr, fout);
		*offset = bloc;
	}

	int Huffman::FindLowest(int* dataDone, int* data)
	{
		int lowest = -1, j = -1;
		for (int i = 0; i < HMAX; i++)
		{
			if (!dataDone[i])
			{
				if (data[i] < j || j < 0)
				{
					lowest = i;
					j = data[i];
				}
			}
		}
		return lowest;
	}
}

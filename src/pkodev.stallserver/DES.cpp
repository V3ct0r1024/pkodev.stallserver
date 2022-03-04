#include "DES.h"

#include <memory>

namespace pkodev
{
	namespace common
	{
		// initial permutation IP
		const char IP_Table[64] = {
			58, 50, 42, 34, 26, 18, 10, 2, 60, 52, 44, 36, 28, 20, 12, 4,
			62, 54, 46, 38, 30, 22, 14, 6, 64, 56, 48, 40, 32, 24, 16, 8,
			57, 49, 41, 33, 25, 17,  9, 1, 59, 51, 43, 35, 27, 19, 11, 3,
			61, 53, 45, 37, 29, 21, 13, 5, 63, 55, 47, 39, 31, 23, 15, 7
		};

		// final permutation IP^-1 
		const char IPR_Table[64] = {
			40, 8, 48, 16, 56, 24, 64, 32, 39, 7, 47, 15, 55, 23, 63, 31,
			38, 6, 46, 14, 54, 22, 62, 30, 37, 5, 45, 13, 53, 21, 61, 29,
			36, 4, 44, 12, 52, 20, 60, 28, 35, 3, 43, 11, 51, 19, 59, 27,
			34, 2, 42, 10, 50, 18, 58, 26, 33, 1, 41,  9, 49, 17, 57, 25
		};

		// expansion operation matrix
		const char E_Table[48] = {
			32,  1,  2,  3,  4,  5,  4,  5,  6,  7,  8,  9,
			8,  9, 10, 11, 12, 13, 12, 13, 14, 15, 16, 17,
			16, 17, 18, 19, 20, 21, 20, 21, 22, 23, 24, 25,
			24, 25, 26, 27, 28, 29, 28, 29, 30, 31, 32,  1
		};

		// 32-bit permutation function P used on the output of the S-boxes 
		const char P_Table[32] = {
			16, 7, 20, 21, 29, 12, 28, 17, 1,  15, 23, 26, 5,  18, 31, 10,
			2,  8, 24, 14, 32, 27, 3,  9,  19, 13, 30, 6,  22, 11, 4,  25
		};

		// permuted choice table (key) 
		const char PC1_Table[56] = {
			57, 49, 41, 33, 25, 17,  9,  1, 58, 50, 42, 34, 26, 18,
			10,  2, 59, 51, 43, 35, 27, 19, 11,  3, 60, 52, 44, 36,
			63, 55, 47, 39, 31, 23, 15,  7, 62, 54, 46, 38, 30, 22,
			14,  6, 61, 53, 45, 37, 29, 21, 13,  5, 28, 20, 12,  4
		};

		// permuted choice key (table) 
		const char PC2_Table[48] = {
			14, 17, 11, 24,  1,  5,  3, 28, 15,  6, 21, 10,
			23, 19, 12,  4, 26,  8, 16,  7, 27, 20, 13,  2,
			41, 52, 31, 37, 47, 55, 30, 40, 51, 45, 33, 48,
			44, 49, 39, 56, 34, 53, 46, 42, 50, 36, 29, 32
		};

		// number left rotations of pc1 
		const char LOOP_Table[16] = {
			1,1,2,2,2,2,2,2,1,2,2,2,2,2,2,1
		};

		// The (in)famous S-boxes 
		const char S_Box[8][4][16] = {
			// S1 
			14,	 4,	13,	 1,  2, 15, 11,  8,  3, 10,  6, 12,  5,  9,  0,  7,
			0, 15,  7,  4, 14,  2, 13,  1, 10,  6, 12, 11,  9,  5,  3,  8,
			4,  1, 14,  8, 13,  6,  2, 11, 15, 12,  9,  7,  3, 10,  5,  0,
			15, 12,  8,  2,  4,  9,  1,  7,  5, 11,  3, 14, 10,  0,  6, 13,
			// S2 
			15,  1,  8, 14,  6, 11,  3,  4,  9,  7,  2, 13, 12,  0,  5, 10,
			3, 13,  4,  7, 15,  2,  8, 14, 12,  0,  1, 10,  6,  9, 11,  5,
			0, 14,  7, 11, 10,  4, 13,  1,  5,  8, 12,  6,  9,  3,  2, 15,
			13,  8, 10,  1,  3, 15,  4,  2, 11,  6,  7, 12,  0,  5, 14,  9,
			// S3 
			10,  0,  9, 14,  6,  3, 15,  5,  1, 13, 12,  7, 11,  4,  2,  8,
			13,  7,  0,  9,  3,  4,  6, 10,  2,  8,  5, 14, 12, 11, 15,  1,
			13,  6,  4,  9,  8, 15,  3,  0, 11,  1,  2, 12,  5, 10, 14,  7,
			1, 10, 13,  0,  6,  9,  8,  7,  4, 15, 14,  3, 11,  5,  2, 12,
			// S4 
			7, 13, 14,  3,  0,  6,  9, 10,  1,  2,  8,  5, 11, 12,  4, 15,
			13,  8, 11,  5,  6, 15,  0,  3,  4,  7,  2, 12,  1, 10, 14,  9,
			10,  6,  9,  0, 12, 11,  7, 13, 15,  1,  3, 14,  5,  2,  8,  4,
			3, 15,  0,  6, 10,  1, 13,  8,  9,  4,  5, 11, 12,  7,  2, 14,
			// S5 
			2, 12,  4,  1,  7, 10, 11,  6,  8,  5,  3, 15, 13,  0, 14,  9,
			14, 11,  2, 12,  4,  7, 13,  1,  5,  0, 15, 10,  3,  9,  8,  6,
			4,  2,  1, 11, 10, 13,  7,  8, 15,  9, 12,  5,  6,  3,  0, 14,
			11,  8, 12,  7,  1, 14,  2, 13,  6, 15,  0,  9, 10,  4,  5,  3,
			// S6 
			12,  1, 10, 15,  9,  2,  6,  8,  0, 13,  3,  4, 14,  7,  5, 11,
			10, 15,  4,  2,  7, 12,  9,  5,  6,  1, 13, 14,  0, 11,  3,  8,
			9, 14, 15,  5,  2,  8, 12,  3,  7,  0,  4, 10,  1, 13, 11,  6,
			4,  3,  2, 12,  9,  5, 15, 10, 11, 14,  1,  7,  6,  0,  8, 13,
			// S7 
			4, 11,  2, 14, 15,  0,  8, 13,  3, 12,  9,  7,  5, 10,  6,  1,
			13,  0, 11,  7,  4,  9,  1, 10, 14,  3,  5, 12,  2, 15,  8,  6,
			1,  4, 11, 13, 12,  3,  7, 14, 10, 15,  6,  8,  0,  5,  9,  2,
			6, 11, 13,  8,  1,  4, 10,  7,  9,  5,  0, 15, 14,  2,  3, 12,
			// S8 
			13,  2,  8,  4,  6, 15, 11,  1, 10,  9,  3, 14,  5,  0, 12,  7,
			1, 15, 13,  8, 10,  3,  7,  4, 12,  5,  6, 11,  0, 14,  9,  2,
			7, 11,  4,  1,  9, 12, 14,  2,  0,  6, 10, 13, 15,  3,  5,  8,
			2,  1, 14,  7,  4, 10,  8, 13, 15, 12,  9,  0,  3,  5,  6, 11
		};


		CDES::CDES()
		{

		}

		CDES::~CDES()
		{

		}

		static void ByteToBit(bool* Out, const char* In, int bits)
		{
			for (int i = 0; i < bits; ++i)
				Out[i] = (In[i >> 3] >> (7 - i & 7)) & 1;
		}

		static void BitToByte(char* Out, const bool* In, int bits)
		{
			memset(Out, 0, bits >> 3);
			for (int i = 0; i < bits; ++i)
				Out[i >> 3] |= In[i] << (7 - i & 7);
		}

		static void RotateL(bool* In, int len, int loop)
		{
			bool Tmp[256];

			memcpy(Tmp, In, loop);
			memcpy(In, In + loop, len - loop);
			memcpy(In + len - loop, Tmp, loop);
		}

		static void Xor(bool* InA, const bool* InB, int len)
		{
			for (int i = 0; i < len; ++i)
				InA[i] ^= InB[i];
		}

		static void Transform(bool* Out, bool* In, const char* Table, int len)
		{
			bool Tmp[256];

			for (int i = 0; i < len; ++i)
				Tmp[i] = In[Table[i] - 1];
			memcpy(Out, Tmp, len);
		}

		void CDES::S_func(bool Out[32], const bool In[48])
		{
			for (char i = 0, j, k; i < 8; ++i, In += 6, Out += 4)
			{
				j = (In[0] << 1) + In[5];
				k = (In[1] << 3) + (In[2] << 2) + (In[3] << 1) + In[4];

				for (int l = 0; l < 4; ++l)
					Out[l] = (S_Box[i][j][k] >> (3 - l)) & 1;
			}
		}

		void CDES::F_func(bool In[32], const bool Ki[48])
		{
			bool MR[48];
			Transform(MR, In, E_Table, 48);
			Xor(MR, Ki, 48);
			S_func(In, MR);
			Transform(In, In, P_Table, 32);
		}

		bool CDES::RunDes(bool bType, bool bMode, char* In, char* Out, unsigned datalen, const char* Key, const unsigned char keylen)
		{

			if (!(In && Out && Key && datalen && keylen >= 8))
				return false;

			if (datalen & 0x00000007)
				return false;

			bool m_SubKey[3][16][48];

			unsigned char nKey = (keylen >> 3) > 3 ? 3 : (keylen >> 3);
			for (int i = 0; i < nKey; i++)
			{
				SetSubKey(&m_SubKey[i], &Key[i << 3]);
			}

			if (bMode == ECB)
			{
				if (nKey == 1)
				{
					for (int i = 0, j = datalen >> 3; i < j; ++i, Out += 8, In += 8)
					{
						DES(Out, In, &m_SubKey[0], bType);
					}
				}
				else
					if (nKey == 2)
					{
						for (int i = 0, j = datalen >> 3; i < j; ++i, Out += 8, In += 8)
						{
							DES(Out, In, &m_SubKey[0], bType);
							DES(Out, Out, &m_SubKey[1], !bType);
							DES(Out, Out, &m_SubKey[0], bType);
						}
					}
					else
					{
						for (int i = 0, j = datalen >> 3; i < j; ++i, Out += 8, In += 8)
						{
							DES(Out, In, &m_SubKey[bType ? 2 : 0], bType);
							DES(Out, Out, &m_SubKey[1], !bType);
							DES(Out, Out, &m_SubKey[bType ? 0 : 2], bType);
						}
					}
			}
			else
			{
				char	cvec[8] = "";
				char	cvin[8] = "";

				if (nKey == 1)
				{
					for (int i = 0, j = datalen >> 3; i < j; ++i, Out += 8, In += 8)
					{
						if (bType == CDES::ENCRYPT)
						{
							for (int j = 0; j < 8; ++j)
							{
								cvin[j] = In[j] ^ cvec[j];
							}
						}
						else
						{
							memcpy(cvin, In, 8);
						}

						DES(Out, cvin, &m_SubKey[0], bType);

						if (bType == CDES::ENCRYPT)
						{
							memcpy(cvec, Out, 8);
						}
						else
						{
							for (int j = 0; j < 8; ++j)
							{
								Out[j] = Out[j] ^ cvec[j];
							}
							memcpy(cvec, cvin, 8);
						}
					}
				}
				else
					if (nKey == 2)	//3DES CBC 2Key
					{
						for (int i = 0, j = datalen >> 3; i < j; ++i, Out += 8, In += 8)
						{
							if (bType == CDES::ENCRYPT)
							{
								for (int j = 0; j < 8; ++j)
								{
									cvin[j] = In[j] ^ cvec[j];
								}
							}
							else
							{
								memcpy(cvin, In, 8);
							}

							DES(Out, cvin, &m_SubKey[0], bType);
							DES(Out, Out, &m_SubKey[1], !bType);
							DES(Out, Out, &m_SubKey[0], bType);

							if (bType == CDES::ENCRYPT)
							{
								memcpy(cvec, Out, 8);
							}
							else
							{
								for (int j = 0; j < 8; ++j)
								{
									Out[j] = Out[j] ^ cvec[j];
								}
								memcpy(cvec, cvin, 8);
							}
						}
					}
					else			//3DES CBC 3Key
					{
						for (int i = 0, j = datalen >> 3; i < j; ++i, Out += 8, In += 8)
						{
							if (bType == CDES::ENCRYPT)
							{
								for (int j = 0; j < 8; ++j)
								{
									cvin[j] = In[j] ^ cvec[j];
								}
							}
							else
							{
								memcpy(cvin, In, 8);
							}

							DES(Out, cvin, &m_SubKey[bType ? 2 : 0], bType);
							DES(Out, Out, &m_SubKey[1], !bType);
							DES(Out, Out, &m_SubKey[bType ? 0 : 2], bType);

							if (bType == CDES::ENCRYPT)
							{
								memcpy(cvec, Out, 8);
							}
							else
							{
								for (int j = 0; j < 8; ++j)
								{
									Out[j] = Out[j] ^ cvec[j];
								}
								memcpy(cvec, cvin, 8);
							}
						}
					}
			}

			return true;
		}

		bool CDES::RunPad(int nType, const char* In, unsigned datalen, char* Out, unsigned& padlen)
		{
			int res = (datalen & 0x00000007);

			if (padlen < (datalen + 8 - res))
			{
				return false;
			}
			else
			{
				padlen = (datalen + 8 - res);
				memcpy(Out, In, datalen);
			}


			if (nType == PAD_ISO_1)
			{
				memset(Out + datalen, 0x00, 8 - res);
			}
			else if (nType == PAD_ISO_2)
			{
				memset(Out + datalen, 0x80, 1);
				memset(Out + datalen, 0x00, 7 - res);
			}
			else if (nType == PAD_PKCS_7)
			{
				memset(Out + datalen, 8 - res, 8 - res);
			}
			else
			{
				return false;
			}

			return true;
		}

		void CDES::SetSubKey(PSubKey pSubKey, const char Key[8])
		{
			bool K[64], * KL = &K[0], * KR = &K[28];
			ByteToBit(K, Key, 64);
			Transform(K, K, PC1_Table, 56);
			for (int i = 0; i < 16; ++i)
			{
				RotateL(KL, 28, LOOP_Table[i]);
				RotateL(KR, 28, LOOP_Table[i]);
				Transform((*pSubKey)[i], K, PC2_Table, 48);
			}
		}

		void CDES::DES(char Out[8], char In[8], const PSubKey pSubKey, bool Type)
		{
			bool M[64], tmp[32], * Li = &M[0], * Ri = &M[32];
			ByteToBit(M, In, 64);
			Transform(M, M, IP_Table, 64);
			if (Type == ENCRYPT)
			{
				for (int i = 0; i < 16; ++i)
				{
					memcpy(tmp, Ri, 32);
					F_func(Ri, (*pSubKey)[i]);
					Xor(Ri, Li, 32);
					memcpy(Li, tmp, 32);
				}
			}
			else
			{
				for (int i = 15; i >= 0; --i)
				{
					memcpy(tmp, Ri, 32);
					F_func(Ri, (*pSubKey)[i]);
					Xor(Ri, Li, 32);
					memcpy(Li, tmp, 32);
				}
			}
			RotateL(M, 64, 32);
			Transform(M, M, IPR_Table, 64);
			BitToByte(Out, M, 64);
		}


		// class C3DES
		char const C3DES::ip_table[64] =
		{
			58, 50, 42, 34, 26, 18, 10, 2, 60, 52, 44, 36, 28, 20, 12, 4,
			62, 54, 46, 38, 30, 22, 14, 6, 64, 56, 48, 40, 32, 24, 16, 8,
			57, 49, 41, 33, 25, 17,  9, 1, 59, 51, 43, 35, 27, 19, 11, 3,
			61, 53, 45, 37, 29, 21, 13, 5, 63, 55, 47, 39, 31, 23, 15, 7
		};

		char const C3DES::ipr_table[64] =
		{
			40, 8, 48, 16, 56, 24, 64, 32, 39, 7, 47, 15, 55, 23, 63, 31,
			38, 6, 46, 14, 54, 22, 62, 30, 37, 5, 45, 13, 53, 21, 61, 29,
			36, 4, 44, 12, 52, 20, 60, 28, 35, 3, 43, 11, 51, 19, 59, 27,
			34, 2, 42, 10, 50, 18, 58, 26, 33, 1, 41,  9, 49, 17, 57, 25
		};

		char const C3DES::e_table[48] =
		{
			32,  1,  2,  3,  4,  5,  4,  5,  6,  7,  8,  9,
			8,   9, 10, 11, 12, 13, 12, 13, 14, 15, 16, 17,
			16, 17, 18, 19, 20, 21, 20, 21, 22, 23, 24, 25,
			24, 25, 26, 27, 28, 29, 28, 29, 30, 31, 32,  1
		};

		char const C3DES::p_table[32] =
		{
			16, 7, 20, 21, 29, 12, 28, 17, 1,  15, 23, 26, 5,  18, 31, 10,
			2,  8, 24, 14, 32, 27, 3,  9,  19, 13, 30, 6,  22, 11, 4,  25
		};

		char const C3DES::pc1_table[56] =
		{
			57, 49, 41, 33, 25, 17,  9,  1, 58, 50, 42, 34, 26, 18,
			10,  2, 59, 51, 43, 35, 27, 19, 11,  3, 60, 52, 44, 36,
			63, 55, 47, 39, 31, 23, 15,  7, 62, 54, 46, 38, 30, 22,
			14,  6, 61, 53, 45, 37, 29, 21, 13,  5, 28, 20, 12,  4
		};

		char const C3DES::pc2_table[48] =
		{
			14, 17, 11, 24,  1,  5,  3, 28, 15,  6, 21, 10,
			23, 19, 12,  4, 26,  8, 16,  7, 27, 20, 13,  2,
			41, 52, 31, 37, 47, 55, 30, 40, 51, 45, 33, 48,
			44, 49, 39, 56, 34, 53, 46, 42, 50, 36, 29, 32
		};

		char const C3DES::loop_table[16] =
		{
			1,1,2,2,2,2,2,2,1,2,2,2,2,2,2,1
		};

		char const C3DES::s_box[8][4][16] =
		{
			14,	 4,	13,	 1,  2, 15, 11,  8,  3, 10,  6, 12,  5,  9,  0,  7,
			0,  15,  7,  4, 14,  2, 13,  1, 10,  6, 12, 11,  9,  5,  3,  8,
			4,   1, 14,  8, 13,  6,  2, 11, 15, 12,  9,  7,  3, 10,  5,  0,
			15, 12,  8,  2,  4,  9,  1,  7,  5, 11,  3, 14, 10,  0,  6, 13,

			15,  1,  8, 14,  6, 11,  3,  4,  9,  7,  2, 13, 12,  0,  5, 10,
			3,  13,  4,  7, 15,  2,  8, 14, 12,  0,  1, 10,  6,  9, 11,  5,
			0,  14,  7, 11, 10,  4, 13,  1,  5,  8, 12,  6,  9,  3,  2, 15,
			13,  8, 10,  1,  3, 15,  4,  2, 11,  6,  7, 12,  0,  5, 14,  9,

			10,  0,  9, 14,  6,  3, 15,  5,  1, 13, 12,  7, 11,  4,  2,  8,
			13,  7,  0,  9,  3,  4,  6, 10,  2,  8,  5, 14, 12, 11, 15,  1,
			13,  6,  4,  9,  8, 15,  3,  0, 11,  1,  2, 12,  5, 10, 14,  7,
			1,  10, 13,  0,  6,  9,  8,  7,  4, 15, 14,  3, 11,  5,  2, 12,

			7,  13, 14,  3,  0,  6,  9, 10,  1,  2,  8,  5, 11, 12,  4, 15,
			13,  8, 11,  5,  6, 15,  0,  3,  4,  7,  2, 12,  1, 10, 14,  9,
			10,  6,  9,  0, 12, 11,  7, 13, 15,  1,  3, 14,  5,  2,  8,  4,
			3,  15,  0,  6, 10,  1, 13,  8,  9,  4,  5, 11, 12,  7,  2, 14,

			2,  12,  4,  1,  7, 10, 11,  6,  8,  5,  3, 15, 13,  0, 14,  9,
			14, 11,  2, 12,  4,  7, 13,  1,  5,  0, 15, 10,  3,  9,  8,  6,
			4,   2,  1, 11, 10, 13,  7,  8, 15,  9, 12,  5,  6,  3,  0, 14,
			11,  8, 12,  7,  1, 14,  2, 13,  6, 15,  0,  9, 10,  4,  5,  3,

			12,  1, 10, 15,  9,  2,  6,  8,  0, 13,  3,  4, 14,  7,  5, 11,
			10, 15,  4,  2,  7, 12,  9,  5,  6,  1, 13, 14,  0, 11,  3,  8,
			9,  14, 15,  5,  2,  8, 12,  3,  7,  0,  4, 10,  1, 13, 11,  6,
			4,   3,  2, 12,  9,  5, 15, 10, 11, 14,  1,  7,  6,  0,  8, 13,

			4,  11,  2, 14, 15,  0,  8, 13,  3, 12,  9,  7,  5, 10,  6,  1,
			13,  0, 11,  7,  4,  9,  1, 10, 14,  3,  5, 12,  2, 15,  8,  6,
			1,   4, 11, 13, 12,  3,  7, 14, 10, 15,  6,  8,  0,  5,  9,  2,
			6,  11, 13,  8,  1,  4, 10,  7,  9,  5,  0, 15, 14,  2,  3, 12,

			13,  2,  8,  4,  6, 15, 11,  1, 10,  9,  3, 14,  5,  0, 12,  7,
			1,  15, 13,  8, 10,  3,  7,  4, 12,  5,  6, 11,  0, 14,  9,  2,
			7,  11,  4,  1,  9, 12, 14,  2,  0,  6, 10, 13, 15,  3,  5,  8,
			2,   1, 14,  7,  4, 10,  8, 13, 15, 12,  9,  0,  3,  5,  6, 11
		};

		C3DES::C3DES()
		{
			li = &m[0];
			ri = &m[32];

			kl = &k[0];
			kr = &k[28];
		}

		C3DES::~C3DES()
		{

		}

		void C3DES::transform(bool* out, bool* in, char const* table, int len)
		{
			for (int i = 0; i < len; ++i)
				tmp[i] = in[table[i] - 1];
			memcpy(out, tmp, len);
		}

		void C3DES::xor_(bool* inA, bool const* inB, int len)
		{
			for (int i = 0; i < len; ++i)
				inA[i] ^= inB[i];
		}

		void C3DES::rotate_L(bool* in, int len, int loop)
		{
			memcpy(tmp, in, loop);
			memcpy(in, in + loop, len - loop);
			memcpy(in + len - loop, tmp, loop);
		}

		void C3DES::byte2bit(bool* out, char const* in, int bits)
		{
			for (int i = 0; i < bits; ++i)
				out[i] = (in[i >> 3] >> (i & 7)) & 1;
		}

		void C3DES::bit2byte(char* out, const bool* in, int bits)
		{
			memset(out, 0, bits >> 3);
			for (int i = 0; i < bits; ++i)
				out[i >> 3] |= in[i] << (i & 7);
		}

		void C3DES::set_sub_key(psubkey pSubkey, char const key[8])
		{
			byte2bit(k, key, 64);
			transform(k, k, pc1_table, 56);
			for (int i = 0; i < 16; ++i)
			{
				rotate_L(kl, 28, loop_table[i]);
				rotate_L(kr, 28, loop_table[i]);
				transform((*pSubkey)[i], k, pc2_table, 48);
			}
		}

		void C3DES::set_key(char const* key, int len)
		{
			memset(des_key, 0, 16);
			memcpy(des_key, key, (len > 16) ? 16 : len);
			set_sub_key(&sub_key[0], &des_key[0]);
			is_3A = (len > 8) ? (set_sub_key(&sub_key[1], &des_key[8]), true) : false;
		}

		void C3DES::S_func(bool out[32], bool const in[48])
		{
			for (char i = 0, j, k; i < 8; ++i, in += 6, out += 4)
			{
				j = (in[0] << 1) + in[5];
				k = (in[1] << 3) + (in[2] << 2) + (in[3] << 1) + in[4];
				byte2bit(out, &s_box[i][j][k], 4);
			}
		}

		void C3DES::F_func(bool in[32], bool const ki[48])
		{
			transform(mr, in, e_table, 48);
			xor_(mr, ki, 48); S_func(in, mr);
			transform(in, in, p_table, 32);
		}

		void C3DES::base_A(char out[8], char in[8], psubkey const pSubKey, bool encrypt)
		{
			byte2bit(m, in, 64);
			transform(m, m, ip_table, 64);
			if (encrypt)
			{
				for (int i = 0; i < 16; ++i)
				{
					memcpy(m_tmp, ri, 32);
					F_func(ri, (*pSubKey)[i]);
					xor_(ri, li, 32);
					memcpy(li, m_tmp, 32);
				}
			}
			else
			{
				for (int i = 15; i >= 0; --i)
				{
					memcpy(m_tmp, li, 32);
					F_func(li, (*pSubKey)[i]);
					xor_(li, ri, 32);
					memcpy(ri, m_tmp, 32);
				}
			}
			transform(m, m, ipr_table, 64);
			bit2byte(out, m, 64);
		}

		bool C3DES::algo_A(char* out, char* in, long data_len, char const* key,
			int key_len, bool encrypt)
		{
			if (!(out && in && key && (data_len = (data_len + 7) & 0xfffffff8)))
				return false;
			set_key(key, key_len);
			if (!is_3A)
			{
				for (long i = 0, j = data_len >> 3; i < j; ++i, out += 8, in += 8)
					base_A(out, in, &sub_key[0], encrypt);
			}
			else
			{
				for (long i = 0, j = data_len >> 3; i < j; ++i, out += 8, in += 8)
				{
					base_A(out, in, &sub_key[0], encrypt);
					base_A(out, out, &sub_key[1], !encrypt);
					base_A(out, out, &sub_key[0], encrypt);
				}
			}
			return true;
		}

		long C3DES::Encrypt(char* out, char const* in, long data_len,
			char const* key, int key_len)
		{
			algo_A(out, (char*)in, data_len, key, key_len, true);
			return 8 * (data_len / 8 + ((data_len % 8 != 0) ? 1 : 0));
		}

		long C3DES::Decrypt(char* out, char const* in, long data_len,
			char const* key, int key_len)
		{
			algo_A(out, (char*)in, data_len, key, key_len, false);
			out[data_len] = 0;
			return 0;
		}
	}
}

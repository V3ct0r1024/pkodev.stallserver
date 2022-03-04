#pragma once

namespace pkodev
{
	namespace common
	{
		// Static class for encrypting and decrypting packets
		class CommEncrypt
		{
			public:
				static void init_Noise(int nNoise, char szKey[4]);
				static bool encrypt_Noise(char szKey[4], char* src, unsigned int src_len);
				static bool decrypt_Noise(char szkey[4], char* src, unsigned int src_len);
				static bool encrypt_B(char* src, unsigned int src_len, char* key, unsigned int key_len, bool en = true);
		};
	}
}
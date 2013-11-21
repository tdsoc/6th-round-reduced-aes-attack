#include <stdint.h>
#include <stdio.h>
#include <vector>

#include "Cipher.hh"
#include "Utility.hh"

namespace Cipher
{

/* 
//
//  AES N-ROUND ENCODING FUNCTION
//
*/
void AES::encode_N_std_rounds( uint8_t const in[16], uint8_t out[16], uint8_t N )
{
	// Copy the input plaintext to state array.
	copyToState(in);

	// Add the first round key to the state before starting the rounds.
	addRoundKey(0);
	
	// There will be N identical rounds.
	// They are executed in the loop below.
	for ( unsigned round=1 ; round<=N ; ++round ) 
	{
		subBytes(); 
		shiftRows();
		mixColumns();
		addRoundKey(round);
	}
	
	// The encryption process is over.
	// Copy the state array to output array.
	loadFromState(out);
}

/* 
//
//  AES N-ROUND ENCODING FUNCTION (THE LAST ROUND IS A FINAL ROUND)
//
*/
void AES::encode_N_rounds( uint8_t const in[16], uint8_t out[16], uint8_t N )
{
	// Copy the input plaintext to state array.
	copyToState(in);

	// Add the first round key to the state before starting the rounds.
	addRoundKey(0);
	
	// There will be N identical rounds.
	// They are executed in the loop below.
	for ( unsigned round=1 ; round<N ; ++round ) 
	{
		subBytes();
		shiftRows();
		mixColumns();
		addRoundKey(round);
	}

	subBytes();
	shiftRows();
	addRoundKey(N);

	// The encryption process is over.
	// Copy the state array to output array.
	loadFromState(out);
}

/*
//
// CONVERT A 4-BYTE VECTOR TO UINT32_T
//
*/
uint32_t AES::GetPosition_4bytes( uint8_t byte_array[4] )
{
	uint32_t pos = 0;
	pos = (byte_array[0]<<24) | (byte_array[1]<<16) | (byte_array[2]<<8) | byte_array[3];
	return pos;
}

/*
//
// CONVERT A UINT32_T TO A 4-BYTE VECTOR
//
*/
void AES::GetArray_4bytes( uint32_t num , uint8_t s[4] )
{
	s[0] = (num >> 24) & 0xff;
	s[1] = (num >> 16) & 0xff;
	s[2] = (num >> 8) & 0xff;
	s[3] = num & 0xff;
}

/*
//
// EXTRACT THE BYTES ACCORDING TO THE PRESCRIBED CONFIGURATION ID
//
*/
void AES::Extract( uint8_t ciphertext[16], uint8_t config_id, uint8_t tmp[4] )
{
	if (config_id == 1)
	{
		tmp[0] = ciphertext[0];
		tmp[1] = ciphertext[13];
		tmp[2] = ciphertext[10];
		tmp[3] = ciphertext[7];
	}
	else if (config_id == 2)
	{
		tmp[0] = ciphertext[4];
		tmp[1] = ciphertext[1];
		tmp[2] = ciphertext[14];
		tmp[3] = ciphertext[11];
	}
	else if (config_id == 3)
	{
		tmp[0] = ciphertext[8];
		tmp[1] = ciphertext[5];
		tmp[2] = ciphertext[2];
		tmp[3] = ciphertext[15];
	}
	else if (config_id == 4)
	{
		tmp[0] = ciphertext[12];
		tmp[1] = ciphertext[9];
		tmp[2] = ciphertext[6];
		tmp[3] = ciphertext[3];
	}
}

/*
//
// FILL THE BYTES ACCORDING TO THE PRESCRIBED CONFIGURATION ID
//
*/
void AES::Fill( uint8_t k1, uint8_t k2, uint8_t k3, uint8_t k4, uint8_t config_id, uint8_t key_guess[16] )
{
	if (config_id == 1)
	{
		key_guess[0] = k1;
		key_guess[13] = k2;
		key_guess[10] = k3;
		key_guess[7] = k4;
	}
	else if (config_id == 2)
	{
		key_guess[4] = k1;
		key_guess[1] = k2;
		key_guess[14] = k3;
		key_guess[11] = k4;
	}
	else if (config_id == 3)
	{
		key_guess[8] = k1;
		key_guess[5] = k2;
		key_guess[2] = k3;
		key_guess[15] = k4;
	}
	else if (config_id == 4)
	{
		key_guess[12] = k1;
		key_guess[9] = k2;
		key_guess[6] = k3;
		key_guess[3] = k4;
	}
}

/*
//
// ENCRYPT A SET OF 2^32 PLAINTEXTS AND CONSTRUCT A VECTOR OF 2^32 BITS ACCORDING THE PRESCRIBED CONFIGURATION ID
//
*/
void AES::Encryption( uint8_t plaintext[16], uint8_t ciphertext[16], uint8_t N_rounds, vector<bool> & vect, uint8_t config_id )
{
	uint8_t tmp[4] = {0,0,0,0};
	bool one = 1;
	uint32_t pos = 0;

	for ( unsigned i=0; i<256; ++i )
	{
		for ( unsigned j=0; j<256; ++j )
		{
			for ( unsigned k=0; k<256; ++k )
			{
				for ( unsigned h=0; h<256; ++h )
				{
					// Create the Delta set
					plaintext[0] = i;
					plaintext[5] = j;
					plaintext[10] = k;
					plaintext[15] = h;
					
					// Encrypt the plaintext
					encode_N_rounds(plaintext,ciphertext,N_rounds);
					
					// Extract 4 bytes
					Extract(ciphertext,config_id,tmp);

					// Convert them to a 32-bit integer
					pos = GetPosition_4bytes(tmp);
					
					// Update the vector of 2^32 bits
					vect[pos] = vect[pos] ^ one;
				}
			}
		}
	}
  }
  
/*
//
// LOOK-UP TABLES USED IN THE PARTIAL SUMS CONSTRUCTION
//
*/
static uint8_t const S1[256] =
{
	0x51,0x7e,0x1a,0x3a,0x3b,0x1f,0xac,0x4b,0x20,0xad,0x88,0xf5,0x4f,0xc5,0x26,0xb5,
	0xde,0x25,0x45,0x5d,0xc3,0x81,0x8d,0x6b,0x03,0x15,0xbf,0x95,0xd4,0x58,0x49,0x8e,
	0x75,0xf4,0x99,0x27,0xbe,0xf0,0xc9,0x7d,0x63,0xe5,0x97,0x62,0xb1,0xbb,0xfe,0xf9,
	0x70,0x8f,0x94,0x52,0xab,0x72,0xe3,0x66,0xb2,0x2f,0x86,0xd3,0x30,0x23,0x02,0xed,
	0x8a,0xa7,0xf3,0x4e,0x65,0x06,0xd1,0xc4,0x34,0xa2,0x05,0xa4,0x0b,0x40,0x5e,0xbd,
	0x3e,0x96,0xdd,0x4d,0x91,0x71,0x04,0x60,0x19,0xd6,0x89,0x67,0xb0,0x07,0xe7,0x79,
	0xa1,0x7c,0xf8,0x00,0x09,0x32,0x1e,0x6c,0xfd,0x0f,0x3d,0x36,0x0a,0x68,0x9b,0x24,
	0x0c,0x93,0xb4,0x1b,0x80,0x61,0x5a,0x1c,0xe2,0xc0,0x3c,0x12,0x0e,0xf2,0x2d,0x14,
	0x57,0xaf,0xee,0xa3,0xf7,0x5c,0x44,0x5b,0x8b,0xcb,0xb6,0xb8,0xd7,0x42,0x13,0x84,
	0x85,0xd2,0xae,0xc7,0x1d,0xdc,0x0d,0x77,0x2b,0xa9,0x11,0x47,0xa8,0xa0,0x56,0x22,
	0x87,0xd9,0x8c,0x98,0xa6,0xa5,0xda,0x3f,0x2c,0x50,0x6a,0x54,0xf6,0x90,0x2e,0x82,
	0x9f,0x69,0x6f,0xcf,0xc8,0x10,0xe8,0xdb,0xcd,0x6e,0xec,0x83,0xe6,0xaa,0x21,0xef,
	0xba,0x4a,0xea,0x29,0x31,0x2a,0xc6,0x35,0x74,0xfc,0xe0,0x33,0xf1,0x41,0x7f,0x17,
	0x76,0x43,0xcc,0xe4,0x9e,0x4c,0xc1,0x46,0x9d,0x01,0xfa,0xfb,0xb3,0x92,0xe9,0x6d,
	0x9a,0x37,0x59,0xeb,0xce,0xb7,0xe1,0x7a,0x9c,0x55,0x18,0x73,0x53,0x5f,0xdf,0x78,
	0xca,0xb9,0x38,0xc2,0x16,0xbc,0x28,0xff,0x39,0x08,0xd8,0x64,0x7b,0xd5,0x48,0xd0
};

static uint8_t const S2[256] = 
{
	0x50,0x53,0xc3,0x96,0xcb,0xf1,0xab,0x93,0x55,0xf6,0x91,0x25,0xfc,0xd7,0x80,0x8f,
	0x49,0x67,0x98,0xe1,0x02,0x12,0xa3,0xc6,0xe7,0x95,0xeb,0xda,0x2d,0xd3,0x29,0x44,
	0x6a,0x78,0x6b,0xdd,0xb6,0x17,0x66,0xb4,0x18,0x82,0x60,0x45,0xe0,0x84,0x1c,0x94,
	0x58,0x19,0x87,0xb7,0x23,0xe2,0x57,0x2a,0x07,0x03,0x9a,0xa5,0xf2,0xb2,0xba,0x5c,
	0x2b,0x92,0xf0,0xa1,0xcd,0xd5,0x1f,0x8a,0x9d,0xa0,0x32,0x75,0x39,0xaa,0x06,0x51,
	0xf9,0x3d,0xae,0x46,0xb5,0x05,0x6f,0xff,0x24,0x97,0xcc,0x77,0xbd,0x88,0x38,0xdb,
	0x47,0xe9,0xc9,0x00,0x83,0x48,0xac,0x4e,0xfb,0x56,0x1e,0x27,0x64,0x21,0xd1,0x3a,
	0xb1,0x0f,0xd2,0x9e,0x4f,0xa2,0x69,0x16,0x0a,0xe5,0x43,0x1d,0x0b,0xad,0xb9,0xc8,
	0x85,0x4c,0xbb,0xfd,0x9f,0xbc,0xc5,0x34,0x76,0xdc,0x68,0x63,0xca,0x10,0x40,0x20,
	0x7d,0xf8,0x11,0x6d,0x4b,0xf3,0xec,0xd0,0x6c,0x99,0xfa,0x22,0xc4,0x1a,0xd8,0xef,
	0xc7,0xc1,0xfe,0x36,0xcf,0x28,0x26,0xa4,0xe4,0x0d,0x9b,0x62,0xc2,0xe8,0x5e,0xf5,
	0xbe,0x7c,0xa9,0xb3,0x3b,0xa7,0x6e,0x7b,0x09,0xf4,0x01,0xa8,0x65,0x7e,0x08,0xe6,
	0xd9,0xce,0xd4,0xd6,0xaf,0x31,0x30,0xc0,0x37,0xa6,0xb0,0x15,0x4a,0xf7,0x0e,0x2f,
	0x8d,0x4d,0x54,0xdf,0xe3,0x1b,0xb8,0x7f,0x04,0x5d,0x73,0x2e,0x5a,0x52,0x33,0x13,
	0x8c,0x7a,0x8e,0x89,0xee,0x35,0xed,0x3c,0x59,0x3f,0x79,0xbf,0xea,0x5b,0x14,0x86,
	0x81,0x3e,0x2c,0x5f,0x72,0x0c,0x8b,0x41,0x71,0xde,0x9c,0x90,0x61,0x70,0x74,0x42
};

static uint8_t const S3[256] = 
{
	0xa7,0x65,0xa4,0x5e,0x6b,0x45,0x58,0x03,0xfa,0x6d,0x76,0x4c,0xd7,0xcb,0x44,0xa3,
	0x5a,0x1b,0x0e,0xc0,0x75,0xf0,0x97,0xf9,0x5f,0x9c,0x7a,0x59,0x83,0x21,0x69,0xc8,
	0x89,0x79,0x3e,0x71,0x4f,0xad,0xac,0x3a,0x4a,0x31,0x33,0x7f,0x77,0xae,0xa0,0x2b,
	0x68,0xfd,0x6c,0xf8,0xd3,0x02,0x8f,0xab,0x28,0xc2,0x7b,0x08,0x87,0xa5,0x6a,0x82,
	0x1c,0xb4,0xf2,0xe2,0xf4,0xbe,0x62,0xfe,0x53,0x55,0xe1,0xeb,0xec,0xef,0x9f,0x10,
	0x8a,0x06,0x05,0xbd,0x8d,0x5d,0xd4,0x15,0xfb,0xe9,0x43,0x9e,0x42,0x8b,0x5b,0xee,
	0x0a,0x0f,0x1e,0x00,0x86,0xed,0x70,0x72,0xff,0x38,0xd5,0x39,0xd9,0xa6,0x54,0x2e,
	0x67,0xe7,0x96,0x91,0xc5,0x20,0x4b,0x1a,0xba,0x2a,0xe0,0x17,0x0d,0xc7,0xa8,0xa9,
	0x19,0x07,0xdd,0x60,0x26,0xf5,0x3b,0x7e,0x29,0xc6,0xfc,0xf1,0xdc,0x85,0x22,0x11,
	0x24,0x3d,0x32,0xa1,0x2f,0x30,0x52,0xe3,0x16,0xb9,0x48,0x64,0x8c,0x3f,0x2c,0x90,
	0x4e,0xd1,0xa2,0x0b,0x81,0xde,0x8e,0xbf,0x9d,0x92,0xcc,0x46,0x13,0xb8,0xf7,0xaf,
	0x80,0x93,0x2d,0x12,0x99,0x7d,0x63,0xbb,0x78,0x18,0xb7,0x9a,0x6e,0xe6,0xcf,0xe8,
	0x9b,0x36,0x09,0x7c,0xb2,0x23,0x94,0x66,0xbc,0xca,0xd0,0xd8,0x98,0xda,0x50,0xf6,
	0xd6,0xb0,0x4d,0x04,0xb5,0x88,0x1f,0x51,0xea,0x35,0x74,0x41,0x1d,0xd2,0x56,0x47,
	0x61,0x0c,0x14,0x3c,0x27,0xc9,0xe5,0xb1,0xdf,0x73,0xce,0x37,0xcd,0xaa,0x6f,0xdb,
	0xf3,0xc4,0x34,0x40,0xc3,0x25,0x49,0x95,0x01,0xb3,0xe4,0xc1,0x84,0xb6,0x5c,0x57
};

static uint8_t const S4[256] = 
{
	0xf4,0x41,0x17,0x27,0xab,0x9d,0xfa,0xe3,0x30,0x76,0xcc,0x02,0xe5,0x2a,0x35,0x62,
	0xb1,0xba,0xea,0xfe,0x2f,0x4c,0x46,0xd3,0x8f,0x92,0x6d,0x52,0xbe,0x74,0xe0,0xc9,
	0xc2,0x8e,0x58,0xb9,0xe1,0x88,0x20,0xce,0xdf,0x1a,0x51,0x53,0x64,0x6b,0x81,0x08,
	0x48,0x45,0xde,0x7b,0x73,0x4b,0x1f,0x55,0xeb,0xb5,0xc5,0x37,0x28,0xbf,0x03,0x16,
	0xcf,0x79,0x07,0x69,0xda,0x05,0x34,0xa6,0x2e,0xf3,0x8a,0xf6,0x83,0x60,0x71,0x6e,
	0x21,0xdd,0x3e,0xe6,0x54,0xc4,0x06,0x50,0x98,0xbd,0x40,0xd9,0xe8,0x89,0x19,0xc8,
	0x7c,0x42,0x84,0x00,0x80,0x2b,0x11,0x5a,0x0e,0x85,0xae,0x2d,0x0f,0x5c,0x5b,0x36,
	0x0a,0x57,0xee,0x9b,0xc0,0xdc,0x77,0x12,0x93,0xa0,0x22,0x1b,0x09,0x8b,0xb6,0x1e,
	0xf1,0x75,0x99,0x7f,0x01,0x72,0x66,0xfb,0x43,0x23,0xed,0xe4,0x31,0x63,0x97,0xc6,
	0x4a,0xbb,0xf9,0x29,0x9e,0xb2,0x86,0xc1,0xb3,0x70,0x94,0xe9,0xfc,0xf0,0x7d,0x33,
	0x49,0x38,0xca,0xd4,0xf5,0x7a,0xb7,0xad,0x3a,0x78,0x5f,0x7e,0x8d,0xd8,0x39,0xc3,
	0x5d,0xd0,0xd5,0x25,0xac,0x18,0x9c,0x3b,0x26,0x59,0x9a,0x4f,0x95,0xff,0xbc,0x15,
	0xe7,0x6f,0x9f,0xb0,0xa4,0x3f,0xa5,0xa2,0x4e,0x82,0x90,0xa7,0x04,0xec,0xcd,0x91,
	0x4d,0xef,0xaa,0x96,0xd1,0x6a,0x2c,0x65,0x5e,0x8c,0x87,0x0b,0x67,0xdb,0x10,0xd6,
	0xd7,0xa1,0xf8,0x13,0xa9,0x61,0x1c,0x47,0xd2,0xf2,0x14,0xc7,0xf7,0xfd,0x3d,0x44,
	0xaf,0x68,0x24,0xa3,0x1d,0xe2,0x3c,0x0d,0xa8,0x0c,0xb4,0x56,0xcb,0x32,0x6c,0xb8
};

/* 
//
//  PARTIAL SUM x2 FIRST ROW
//
*/
uint8_t AES::x2_firstrow( uint8_t c1, uint8_t c2, uint8_t k1, uint8_t k2 )
{
	uint8_t x2 = 0;
	x2 = S1[c1 ^ k1] ^ S2[c2 ^ k2];
	return x2;
}

/* 
//
//  PARTIAL SUM x3 FIRST ROW
//
*/
uint8_t AES::x3_firstrow( uint8_t x2, uint8_t c3, uint8_t k3 )
{
	uint8_t x3 = 0;
	x3 = x2 ^ S3[c3 ^ k3];
	return x3;
}

/* 
//
//  PARTIAL SUM x4 FIRST ROW
//
*/
uint8_t AES::x4_firstrow( uint8_t x3, uint8_t c4, uint8_t k4 )
{
	uint8_t x4 = 0;
	x4 = x3 ^ S4[c4 ^ k4];
	return x4;
}

/* 
//
//  PARTIAL SUM x2 SECOND ROW
//
*/
uint8_t AES::x2_secondrow( uint8_t c1, uint8_t c2, uint8_t k1, uint8_t k2 )
{
	uint8_t x2 = 0;
	x2 = S4[c1 ^ k1] ^ S1[c2 ^ k2];
	return x2;
}

/* 
//
//  PARTIAL SUM x3 SECOND ROW
//
*/
uint8_t AES::x3_secondrow( uint8_t x2, uint8_t c3, uint8_t k3 )
{
	uint8_t x3 = 0;
	x3 = x2 ^ S2[c3 ^ k3];
	return x3;
}

/* 
//
//  PARTIAL SUM x4 SECOND ROW
//
*/
uint8_t AES::x4_secondrow( uint8_t x3, uint8_t c4, uint8_t k4 )
{
	uint8_t x4 = 0;
	x4 = x3 ^ S3[c4 ^ k4];
	return x4;
}

/* 
//
//  FINAL SUM
//
*/
uint8_t AES::Total_sum( vector<bool> & vect, uint8_t k5 )
{
	uint8_t sum = 0;
	for ( unsigned i=0; i<256; ++i )
	{
		if (vect[i])
		{
			sum ^= invGamma(i ^ k5);
		}
	}
	return sum;
}

/*
//
// UPDATE THE VECTOR OF 2^24 BITS FIRST ROW
//
*/
void AES::Update_vect_2_24_firstrow( vector<bool> & vect11, vector<bool> & vect12, uint8_t k1, uint8_t k2 )
{
	uint8_t x2 = 0;
	uint32_t pos24, pos32 = 0;
	uint8_t a,b = 0;
	uint64_t maxiter = 0;

	// Restore vect12 to zero
	for ( uint32_t h=0; h<16777216; ++h )
	{
		vect12[h] = 0;
	}

	for( uint32_t i=0; i<65536; ++i )
	{
		// Compute c1,c2
		a = (i >> 8) & 0xff;
		b = i & 0xff;

		pos32 = i << 16;
		x2 = x2_firstrow(a,b,k1,k2);
		pos24 = x2 << 16;
		maxiter = pos24 + 65536;

		// Update vect12
		for( uint64_t j=pos24; j<maxiter; ++j )
		{
			vect12[j] = vect11[pos32] ^ vect12[j];
			pos32++;
		}
	}
}

/*
//
// UPDATE THE VECTOR OF 2^16 BITS FIRST ROW
//
*/
void AES::Update_vect_2_16_firstrow( vector<bool> & vect12, vector<bool> & vect13, uint8_t k3 )
{
	uint8_t x3;
	uint32_t pos16, pos24 = 0;
	uint8_t a,b = 0;
	uint32_t maxiter = 0;

	// Restore vect13 to 0
	for ( uint32_t h=0; h<65536; ++h )
	{
		vect13[h] = 0;
	}

	for( uint32_t i=0; i<65536; ++i )
	{
		// Compute x2,c3
		a = (i >> 8) & 0xff;
		b = i & 0xff;

		pos24 = i << 8;
		x3 = x3_firstrow(a,b,k3);
		pos16 = x3 << 8;
		maxiter = pos16 + 256;

		// Update vect13
		for( uint32_t j=pos16; j<maxiter; ++j )
		{
			vect13[j] = vect12[pos24] ^ vect13[j];
			pos24++;
		}
	}
}

/*
//
// UPDATE THE VECTOR OF 2^8 BITS FIRST ROW
// 
*/
void AES::Update_vect_2_8_firstrow( vector<bool> & vect13, vector<bool> & vect14, uint8_t k4 )
{
	uint32_t length = 65536;
	uint8_t x4,a,b = 0;
	uint32_t pos16, pos8 = 0;

	// Restore vect14 to 0
	for ( uint16_t h=0; h<256; ++h )
	{
		vect14[h] = 0;
	}

	for ( uint32_t i=0; i<length; ++i )
	{
		// Compute x3,c4
		a = (i >> 8) & 0xff;
		b = i & 0xff;

		pos16 = i;
		x4 = x4_firstrow(a,b,k4);
		pos8 = x4;

		// Update vect14 
		vect14[pos8] = vect14[pos8] ^ vect13[pos16];
	}
}

/*
//
// UPDATE THE VECTOR OF 2^24 BITS SECOND ROW
//
*/
void AES::Update_vect_2_24_secondrow( vector<bool> & vect11, vector<bool> & vect12, uint8_t k1, uint8_t k2 )
{
	uint8_t x2 = 0;
	uint32_t pos24, pos32 = 0;
	uint8_t a,b = 0;
	uint64_t maxiter = 0;

	// Restore vect12 to 0
	for ( uint32_t h=0; h<16777216; ++h )
	{
		vect12[h] = 0;
	}

	for( uint32_t i=0; i<65536; ++i )
	{
		// Compute c1,c2
		a = (i >> 8) & 0xff;
		b = i & 0xff;

		pos32 = i << 16;
		x2 = x2_secondrow(a,b,k1,k2);
		pos24 = x2 << 16;
		maxiter = pos24 + 65536;

		// Update vect12
		for( uint64_t j=pos24; j<maxiter; ++j )
		{
			vect12[j] = vect11[pos32] ^ vect12[j];
			pos32++;
		}
	}
}

/*
//
// UPDATE THE VECTOR OF 2^16 BITS SECOND ROW
//
*/
void AES::Update_vect_2_16_secondrow( vector<bool> & vect12, vector<bool> & vect13, uint8_t k3 )
{
	uint8_t x3;
	uint32_t pos16, pos24 = 0;
	uint8_t a,b = 0;
	uint32_t maxiter = 0;

	// Restore vect13 to 0
	for ( uint32_t h=0; h<65536; ++h )
	{
		vect13[h] = 0;
	}

	for( uint32_t i=0; i<65536; ++i )
	{
		// Compute x2,c3
		a = (i >> 8) & 0xff;
		b = i & 0xff;

		pos24 = i << 8;
		x3 = x3_secondrow(a,b,k3);
		pos16 = x3 << 8;
		maxiter = pos16 + 256;

		// Update vect13
		for( uint32_t j=pos16; j<maxiter; ++j )
		{
			vect13[j] = vect12[pos24] ^ vect13[j];
			pos24++;
		}
	}
}

/*
//
// UPDATE THE VECTOR OF 2^8 BITS SECOND ROW
// 
*/
void AES::Update_vect_2_8_secondrow( vector<bool> & vect13, vector<bool> & vect14, uint8_t k4 )
{
	uint32_t length = 65536;
	uint8_t x4,a,b = 0;
	uint32_t pos16, pos8 = 0;

	// Restore vect14 to 0
	for ( uint16_t h=0; h<256; ++h )
	{
		vect14[h] = 0;
	}

	for ( uint32_t i=0; i<length; ++i )
	{
		// Compute x3,c4
		a = (i >> 8) & 0xff;
		b = i & 0xff;

		pos16 = i;
		x4 = x4_secondrow(a,b,k4);
		pos8 = x4;

		// Update vect14
		vect14[pos8] = vect14[pos8] ^ vect13[pos16];
	}
}

/* 
//
//  SECOND ROW INSTANCE
//
*/
bool AES::Second_row_instance( vector<bool> & vect11, vector<bool> & vect21, vector<bool> & vect31, uint8_t k1, uint8_t k2, uint8_t k3, uint8_t k4 )
{
	vector<bool> secondvect12(16777216,0);		// Create vectors of 2^24 bits
	vector<bool> secondvect22(16777216,0);
	vector<bool> secondvect32(16777216,0);
	vector<bool> secondvect13(65536,0);		// Create vectors of 2^16 bits
	vector<bool> secondvect23(65536,0);
	vector<bool> secondvect33(65536,0);
	vector<bool> secondvect14(256,0);		// Create vectors of 2^8 bits
	vector<bool> secondvect24(256,0);
	vector<bool> secondvect34(256,0);

	uint8_t secondsum1 = 0;
	uint8_t secondsum2 = 0;
	uint8_t secondsum3 = 0;
	bool flag = false;

	Update_vect_2_24_secondrow( vect11, secondvect12, k1, k2 );
	Update_vect_2_24_secondrow( vect21, secondvect22, k1, k2 );
	Update_vect_2_24_secondrow( vect31, secondvect32, k1, k2 );

	Update_vect_2_16_secondrow( secondvect12, secondvect13, k3 );
	Update_vect_2_16_secondrow( secondvect22, secondvect23, k3 );
	Update_vect_2_16_secondrow( secondvect32, secondvect33, k3 );

	Update_vect_2_8_secondrow( secondvect13, secondvect14, k4 );
	Update_vect_2_8_secondrow( secondvect23, secondvect24, k4 );
	Update_vect_2_8_secondrow( secondvect33, secondvect34, k4 );
					
	for ( unsigned k5=0; k5<256; ++k5 )
	{
		// Compute the final sums on the second row
		secondsum1 = Total_sum( secondvect14, k5 );
		secondsum2 = Total_sum( secondvect24, k5 );
		secondsum3 = Total_sum( secondvect34, k5 );

		// Check if they are zero
		if ((secondsum1 == 0) && (secondsum2 == 0) && (secondsum3 == 0))
		{
			flag = true;
			return flag;
		}
	}

	return flag;
}

/* 
//
//  PARTIAL SUM ATTACK
//
*/
bool AES::Partial_Sum_Attack( vector<bool> & vect11, vector<bool> & vect21, vector<bool> & vect31, uint8_t key_guess[16], uint8_t config_id )
{
	bool flag = false;
	vector<bool> vect12(16777216,0);	// Create vectors of 2^24 bits
	vector<bool> vect22(16777216,0);
	vector<bool> vect32(16777216,0);
	vector<bool> vect13(65536,0);		// Create vectors of 2^16 bits
	vector<bool> vect23(65536,0);
	vector<bool> vect33(65536,0);
	vector<bool> vect14(256,0);		// Create vectors of 2^8 bits
	vector<bool> vect24(256,0);
	vector<bool> vect34(256,0);

	uint8_t sum1 = 0;
	uint8_t sum2 = 0;
	uint8_t sum3 = 0;

	for ( unsigned k1=0; k1<256; ++k1 )
	{
		for ( unsigned k2=0; k2<256; ++k2 )
		{
			Update_vect_2_24_firstrow( vect11, vect12, k1, k2 );
			Update_vect_2_24_firstrow( vect21, vect22, k1, k2 );
			Update_vect_2_24_firstrow( vect31, vect32, k1, k2 );

			for ( unsigned k3=0; k3<256; ++k3 )
			{
				Update_vect_2_16_firstrow( vect12, vect13, k3 );
				Update_vect_2_16_firstrow( vect22, vect23, k3 );
				Update_vect_2_16_firstrow( vect32, vect33, k3 );

				for ( unsigned k4=0; k4<256; ++k4 )
				{
					Update_vect_2_8_firstrow( vect13, vect14, k4 );
					Update_vect_2_8_firstrow( vect23, vect24, k4 );
					Update_vect_2_8_firstrow( vect33, vect34, k4 );
					
					for ( unsigned k5=0; k5<256; ++k5 )
					{
						// Compute the final sums on the first row
						sum1 = Total_sum( vect14, k5 );
						sum2 = Total_sum( vect24, k5 );
						sum3 = Total_sum( vect34, k5 );

						// Check if they are zero
						if ((sum1 == 0) && (sum2 == 0) && (sum3 == 0))
						{
							// Run the verification step on the second row
							if (Second_row_instance( vect11, vect21, vect31, k1, k2, k3, k4 ))
							{
								flag = true;
								// The key has been found
								Fill( k1, k2, k3, k4, config_id, key_guess );
								return flag;
							}
						}
					}
				}
			}
		}
	}	
	return flag;	
}

}

#include "stdafx.h"
#include "key_schedule_test.h"

struct Key_Data {
	unsigned int length;
	uint8_t key[32];
	uint32_t expanded_key[16*8];
};

static Key_Data test_data[] = {
	#include "key_schedule_test_data.txt"
	{0}
};

void key_schedule_test()
{
	printf("Test: key_schedule\n");

	uint32_t result[RKLENGTH(256)];

	for (unsigned int i = 0; test_data[i].length != 0; i++)
	{
		Key_Data &data = test_data[i];
		//memcpy(result, data.key, data.length);

		switch (data.length)
		{
		case 32:
			result[7] = ((uint_u *) (data.key + 28))->i;
			result[6] = ((uint_u *) (data.key + 24))->i;
		case 24:
			result[5] = ((uint_u *) (data.key + 20))->i;
			result[4] = ((uint_u *) (data.key + 16))->i;
		case 16:
			result[3] = ((uint_u *) (data.key + 12))->i;
			result[2] = ((uint_u *) (data.key +  8))->i;
			result[1] = ((uint_u *) (data.key +  4))->i;
			result[0] = ((uint_u *) (data.key     ))->i;
		}

		#define GETU32(plaintext) (((uint32_t)(plaintext)[0] << 24) ^ \
								   ((uint32_t)(plaintext)[1] << 16) ^ \
								   ((uint32_t)(plaintext)[2] <<  8) ^ \
								   ((uint32_t)(plaintext)[3]))

		result[0] = GETU32(data.key     );
		result[1] = GETU32(data.key +  4);
		result[2] = GETU32(data.key +  8);
		result[3] = GETU32(data.key + 12);

		key_schedule((uint32_t *) result, data.length*8);

		if (memcmp(result, data.expanded_key, RKLENGTH(data.length*8)) == 0)
		{
			printf("  #%u (%u bit) pass\n", i+1, data.length);
		}
		else
		{
			printf("  #%u (%u bit) fail\n", i+1, data.length);

			for (unsigned int j = 0; j < RKLENGTH(data.length); j++)
			{
				printf("\t\t");
				for (int k = 0; k < 8; k++)
				{
					printf("0x%08x, ", data.expanded_key[j*RKLENGTH(data.length) + k]);
				}

				printf("\n\t\t");
				for (int k = 0; k < 8; k++)
				{
					printf("0x%08x, ", result[j*RKLENGTH(data.length) + k]);
				}
			
				printf("\n\n");
			}
		}
	}
	printf("\n");
}
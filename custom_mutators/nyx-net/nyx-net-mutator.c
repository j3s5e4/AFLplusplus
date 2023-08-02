#include <stdint.h>
#include <stdlib.h>
#include <string.h>
#include <stdio.h>

#include "afl_input.h"

#include "afl-fuzz.h"
#include "afl-mutations.h"

/*

Run with the following environment variables:
AFL_DISABLE_TRIM=1 \
AFL_CUSTOM_MUTATOR_ONLY=1 \
AFL_CUSTOM_MUTATOR_LIBRARY=$HOME/aflpp_nyx/custom_mutators/nyx-net/nyx-net-mutator.so \
afl-fuzz <PARAMS>

*/

s8  interesting_8[] = {INTERESTING_8};
s16 interesting_16[] = {INTERESTING_8, INTERESTING_16};
s32 interesting_32[] = {INTERESTING_8, INTERESTING_16, INTERESTING_32};

typedef struct my_mutator {

  afl_state_t*  afl;
  u8*           buf;    /* Mutable scratch buffer to perform mutations in */
  u32           buf_size;
} my_mutator_t;

my_mutator_t data;

my_mutator_t *afl_custom_init(afl_state_t *afl, unsigned int seed) {

  if ((data.buf = (u8*)malloc(MAX_FILE)) == NULL) {

    perror("afl_custom_init alloc");
    return NULL;

  } else {

    data.buf_size = MAX_FILE;

  }

  data.afl = afl;

  return &data;

}

/**
 * Perform custom mutations on a given input
 *
 * @param[in] data pointer returned in afl_custom_init for this fuzz case
 * @param[in] buf Pointer to input data to be mutated
 * @param[in] buf_size Size of input data
 * @param[out] out_buf the buffer we will work on. we can reuse *buf. NULL on
 * error. This memory is managed by the custom mutator.
 * @param[in] add_buf Buffer containing the additional test case
 * @param[in] add_buf_size Size of the additional test case
 * @param[in] max_size Maximum size of the mutated output. The mutation must not
 *     produce data larger than max_size.
 * @return Size of the mutated output.
 */
size_t afl_custom_fuzz(my_mutator_t *data, uint8_t *buf, size_t buf_size,
                       u8 **out_buf, uint8_t *add_buf, size_t add_buf_size,
                       size_t max_size)
{
    /*

    (0. init allocates data->buf)
    ...
    1. deserialize buf into protobuf
    2. modify protobuf
    3. serialize protobuf into data->buf
    (4. data->buf is passed to target)
    ...
    (5. deinit frees data->buf)

    */

    // Deserialize buf into an Input

    printf("Custom mutator called with buf size %zu\n", buf_size);
    afl_deserialize(buf, buf_size);

    // Apply a mutation to the input

    u32 r = rand_below(data->afl, 64);
    switch (r) {

        case 0: {   /* Randomly delete a packet */

            if (afl_packets_size() < 2) { break; }

            u32 idx = rand_below(data->afl, afl_packets_size());

            afl_delete_packet(idx); /* Possibly slow */

            break;

        }

        default: {  /* Select and mutate a packet */
            
            // SLOW
            // Copy packet into max_size buf, mutate, and deserialize into input

            u32 idx = rand_below(data->afl, afl_packets_size());
            u32 num_steps = 1 + rand_below(data->afl, 16);

            size_t copy_size = afl_get_packet(idx, data->buf, data->buf_size);  /* Copy packet to buf */

            u32 out_buf_len = afl_mutate(data->afl, data->buf, copy_size, num_steps,
                                        false, true, add_buf, add_buf_size, max_size);

            afl_set_packet(idx, data->buf, out_buf_len);    /* Replace packet with mutated buf */

            break;

        }
    }

    // Serialize Input

    size_t mutated_size = afl_serialize(data->buf, data->buf_size);

    *out_buf = data->buf;
    return mutated_size;
}

/**
 * Deinitialize everything
 *
 * @param data The data ptr from afl_custom_init
 */
void afl_custom_deinit(my_mutator_t *data) {

  free(data->buf);

}


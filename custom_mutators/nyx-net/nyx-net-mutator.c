#include "afl-fuzz.h"
#include "afl-mutations.h"

#include <stdint.h>
#include <stdlib.h>
#include <string.h>
#include <stdio.h>

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

typedef struct nyx_packet
{
  uint32_t length;
  uint8_t data[];
} nyx_packet;

typedef struct nyx_input
{
  uint32_t num_packets;
  nyx_packet packets[];
} nyx_input;

typedef struct my_mutator {

  afl_state_t *afl;
  u8          *buf;
  u32          buf_size;

} my_mutator_t;

my_mutator_t *afl_custom_init(afl_state_t *afl, unsigned int seed) {

  (void)seed;

  my_mutator_t *data = calloc(1, sizeof(my_mutator_t));
  if (!data) {

    perror("afl_custom_init alloc");
    return NULL;

  }

  if ((data->buf = malloc(MAX_FILE)) == NULL) {

    perror("afl_custom_init alloc");
    return NULL;

  } else {

    data->buf_size = MAX_FILE;

  }

  data->afl = afl;

  return data;

}

void hexdump(const void* data, size_t size) {
  char ascii[17];
  size_t i, j;
  ascii[16] = '\0';
  for (i = 0; i < size; ++i) {
    printf("%02X ", ((unsigned char*)data)[i]);
    if (((unsigned char*)data)[i] >= ' ' && ((unsigned char*)data)[i] <= '~') {
      ascii[i % 16] = ((unsigned char*)data)[i];
    } else {
      ascii[i % 16] = '.';
    }
    if ((i+1) % 8 == 0 || i+1 == size) {
      printf(" ");
      if ((i+1) % 16 == 0) {
        printf("|  %s \n", ascii);
      } else if (i+1 == size) {
        ascii[(i+1) % 16] = '\0';
        if ((i+1) % 16 <= 8) {
          printf(" ");
        }
        for (j = (i+1) % 16; j < 16; ++j) {
          printf("   ");
        }
        printf("|  %s \n", ascii);
      }
    }
  }
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

    First attempt:
    (see todo for new ideas)

    Select from and apply to random packet if applicable
                MutationStrategy::GenerateTail(args) => self.generate_tail(orig, ops_used, snapshot, args, storage, dist),
                MutationStrategy::SpliceRandom => self.splice_random(orig, ops_used, snapshot, dict, storage, dist),
                MutationStrategy::Splice => self.splice(orig, ops_used, snapshot,queue, storage, dist),
                MutationStrategy::DataOnly => self.mutate_data(orig, ops_used, snapshot, dict, storage, dist),
                MutationStrategy::Generate => self.generate(50, snapshot, storage, dist),
                MutationStrategy::Repeat => self.repeat(orig, ops_used, snapshot, dict, storage, dist),
                MutationStrategy::Minimize => unreachable!(),
                MutationStrategy::MinimizeSplit => unreachable!(),
                MutationStrategy::Import => unreachable!(),
                MutationStrategy::SeedImport => unreachable!(),

    If we implement afl_custom_fuzz_send we may be able to do faster in place mutation,
    rather than repeated serializtion/deserializtion and mem-copying. Normally sent with
    fsrv->nyx_handlers->nyx_set_afl_input(fsrv->nyx_runner, buf, len);
    However, this would probably require patching the QemuProcess stuff, so for now just use out_buf.

    Finally, use post_process() to place the structured input into the buffer

    */

    // Assume buf is already a valid structure for now
    // Need to figure out how to take seed and convert to structure

    // Maximize internal buffer
    if (max_size > data->buf_size) {

        data->buf = realloc(data->buf, max_size);
        data->buf_size = max_size;

    }

    // Copy input buf to internal buffer
    memcpy(data->buf, buf, buf_size);

    /*

    TODO:

    THIS IS TOO HARD
    USE A SERIALIZATION LIBRARY LIKE PROTOBUF-C

    adding (to) / removing (from) packets is too much

    (0. init allocates data->buf)
    ...
    1. free current data->buf
    2. deserialize buf into protobuf
    3. modify protobuf
    4. serialize protobuf into data->buf
    (5. data->buf is passed to target)
    ...
    (6. deinit frees data->buf)

    */

    nyx_input* input = data->buf;
    u32 r = rand_below(data->afl, 64);
    switch (r) {

        case 0: {

            // Randomly delete a packet

            if (input->num_packets < 2) { break; }

            u32 idx = rand_below(data->afl, input->num_packets);

            if (idx == input->num_packets - 1) {

                input->num_packets--;
                break;

            }

            char* packet = &input->packets;
            for (int i = 0; i < idx; i++) {
                packet += ((nyx_packet*)packet)->length + sizeof(nyx_packet);
            }

            char* next_packet = packet + ((nyx_packet*)packet)->length + sizeof(nyx_packet);
            memmove(packet, next_packet, data->buf + data->buf_size - next_packet);

            break;

        }

        default:

            /*
            Select and mutate a packet
            */
            
            nyx_packet* packet = &input->packets[rand_below(data->afl, input->num_packets)];
            u32 num_steps = 1 + rand_below(data->afl, 16);
            u32 out_buf_len = afl_mutate(data->afl, packet->data, packet->length, num_steps,
                                        false, true, add_buf, add_buf_size, max_size);
            packet->length = out_buf_len;
            break;
    }

  /* return size of mutated data */
  *out_buf = data->buf;
  return out_buf_len;
}

/**
 * Deinitialize everything
 *
 * @param data The data ptr from afl_custom_init
 */
void afl_custom_deinit(my_mutator_t *data) {

  free(data->buf);
  free(data);

}


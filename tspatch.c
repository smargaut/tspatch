/* This is free and unencumbered software released into the public domain. */

#include <assert.h>
#include <string.h>
#include <stdlib.h>
#include <stdio.h>

#include <libgen.h> /* for basename */

/******************************/
/*** MACROS & TYPES: C CODE ***/
/******************************/

#define FALSE 0
#define TRUE  1

typedef unsigned       bool_t;
typedef unsigned char  uint8_t;
typedef unsigned short uint16_t;
typedef unsigned long  uint32_t;

#define LOCAL_FUNCTION static

/*
** TODO: complete and use these macros
*/
#define READ_16BITS(p)      ((((uint16_t)(*(p))<<8)&0xFF00) | (uint16_t)*((p)+1))
#define READ_12BITS(p)      ((((uint16_t)(*(p))<<8)&0x0F00) | (uint16_t)*((p)+1))
#define READ_08BITS(p)      (*(p))
#define WRITE_16BITS(p,v)   ()
#define WRITE_12BITS(p,v)   ()
#define WRITE_08BITS(p,v)   ()

/****************************/
/*** MACROS & TYPES: MPEG ***/
/****************************/

#define NB_MAX_ES_IN_PMT 20
#define PMT_INFO_MAX_SIZE 1

#define TS_SYNCHRO_BYTE (0x47)
#define TS_PACKET_SIZE  (188)
#define INVALID_PID                 0x1FFF
#define INVALID_TRANSPORT_STREAM_ID 0xFFFF
#define INVALID_PROGRAM_NUMBER      0xFFFF
#define INVALID_VERSION_NUMBER      0xFF

#define PAT_PID (0x00)
#define CAT_PID (0x01)
#define PAT_TID (0x00)
#define CAT_TID (0x01)
#define PMT_TID (0x02)

#define SECTION_TABLE_ID_SIZE            1
#define SECTION_SECTION_LENGTH_SIZE      2
#define SECTION_TRANSPORT_STREAM_ID_SIZE 2
#define SECTION_VERSION_NUMBER_SIZE      1
#define SECTION_SECTION_NUMBER_SIZE      1
#define SECTION_LAST_SECTION_NUMBER_SIZE 1
#define SECTION_PID_SIZE                 2
#define SECTION_CRC_SIZE                 4
#define SECTION_HEADER_SIZE (SECTION_TABLE_ID_SIZE + SECTION_SECTION_LENGTH_SIZE)

#define TS_PCR_BASE_CLOCK (90000)

typedef struct
{
  uint32_t base_32lb;
  uint16_t extension;
  uint8_t  base_1hb;

} pcr_t;

typedef struct
{
  uint32_t count;
  pcr_t    pcr;

} packet_time_t;

typedef struct
{
  uint16_t pid;
  uint16_t tid;

} packet_info_t;

typedef struct
{
  uint16_t  section_length;
  uint16_t  transport_stream_id;
  uint8_t   version_number;
  uint8_t   section_number;
  uint8_t   last_section_number;
  uint8_t   number_of_programs;

} pat_header_t;

typedef struct
{
  uint16_t  section_length;
  uint16_t  program_number;
  uint16_t  pcr_pid;
  uint16_t  program_info_length;
  uint8_t   version_number;
  uint8_t   section_number;
  uint8_t   last_section_number;
  uint8_t   number_of_es;

} pmt_header_t;

typedef struct
{
  uint16_t  pid;
  uint16_t  info_length;
  uint8_t   stream_type;
  uint8_t   info[PMT_INFO_MAX_SIZE];

} pmt_es_t;

typedef struct
{
  uint16_t tsid;
  uint16_t number;
  uint16_t pmt_pid;
  uint16_t pcr_pid;
  uint16_t audio_pid;
  uint16_t video_pid;

} prog_t;

static void init_prog(prog_t *prog)
{
  prog->tsid      = INVALID_TRANSPORT_STREAM_ID;
  prog->number    = INVALID_PROGRAM_NUMBER;
  prog->pmt_pid   = INVALID_PID;
  prog->pcr_pid   = INVALID_PID;
  prog->audio_pid = INVALID_PID;
  prog->video_pid = INVALID_PID;
}

/***************************/
/*** MACROS & TYPES: DVB ***/
/***************************/

#define NIT_PID (0x10)
#define SDT_PID (0x11)
#define BAT_PID (0x11)
#define EIT_PID (0x12)
#define RST_PID (0x12)
#define TDT_PID (0x14)
#define TOT_PID (0x14)

#define NIT_ACTUAL_TID              (0x40)
#define NIT_OTHER_TID               (0x41)
#define SDT_ACTUAL_TID              (0x42)
#define SDT_OTHER_TID               (0x46)
#define BAT_TID                     (0x4A)
#define EITPF_ACTUAL_TID            (0x4E)
#define EITPF_OTHER_TID             (0x4F)
#define EIT_SCHEDULE_ACTUAL_MIN_TID (0x50)
#define EIT_SCHEDULE_ACTUAL_MAX_TID (0x5F)
#define EIT_SCHEDULE_OTHER_MIN_TID  (0x60)
#define EIT_SCHEDULE_OTHER_MAX_TID  (0x6F)
#define TDT_TID                     (0x70)
#define RST_TID                     (0x71)
#define ST_TID                      (0x72)
#define TOT_TID                     (0x73)
#define AIT_TID                     (0x74)

#define NETWORK_NAME_DESCRIPTOR_TAG              (0x40)
#define SERVICE_LIST_DESCRIPTOR_TAG              (0x41)
#define STUFFING_DESCRIPTOR_TAG                  (0x42)
#define SATELLITE_DELIVERY_SYSTEM_DESCRIPTOR_TAG (0x43)
#define CABLE_DELIVERY_SYSTEM_DESCRIPTOR_TAG     (0x44)
#define VBI_DATA_DESCRIPTOR_TAG                  (0x45)
#define VBI_TELETEXT_DESCRIPTOR_TAG              (0x46)
#define BOUQUET_NAME_DESCRIPTOR_TAG              (0x47)
#define SERVICE_DESCRIPTOR_TAG                   (0x48)
#define COUNTRY_AVAILABILITY_DESCRIPTOR_TAG      (0x49)
#define LINKAGE_DESCRIPTOR_TAG                   (0x4A)
#define NVOD_REFERENCE_DESCRIPTOR_TAG            (0x4B)
#define TIME_SHIFTED_SERVICE_DESCRIPTOR_TAG      (0x4C)
#define SHORT_EVENT_DESCRIPTOR_TAG               (0x4D)
#define EXTENDED_EVENT_DESCRIPTOR_TAG            (0x4E)
#define TIME_SHIFTED_EVENT_DESCRIPTOR_TAG        (0x4F)
#define COMPONENT_DESCRIPTOR_TAG                 (0x50)
#define MOSAIC_DESCRIPTOR_TAG                    (0x51)
#define STREAM_IDENTIFIER_DESCRIPTOR_TAG         (0x52)
#define CA_IDENTIFIER_DESCRIPTOR_TAG             (0x53)
#define CONTENT_DESCRIPTOR_TAG                   (0x54)
#define MULTILINGUAL_SERVICE_NAME_DESCRIPTOR_TAG (0x5D)
/*
** TODO: complete this list
*/

typedef struct
{
  uint16_t  section_length;
  uint16_t  transport_stream_id;
  uint16_t  original_network_id;
  uint8_t   version_number;
  uint8_t   section_number;
  uint8_t   last_section_number;
  uint8_t   number_of_services;

} sdt_header_t;

typedef struct
{
  const char *base;
  const char *file;

} ait_url_t;

typedef struct
{
  uint16_t id;
  char     name[64];

} service_t;

static void init_service(service_t *service)
{
  service->id = 0;
  service->name[0] = 0;
}

typedef struct
{
  ait_url_t url;
  uint16_t  pid;
  uint16_t  id;

} ait_t;

static void init_ait(ait_t *ait)
{
  ait->url.base = NULL;
  ait->url.file = NULL;
  ait->pid      = INVALID_PID;
  ait->id       = 0;
}

/*************/
/*** DEBUG ***/
/*************/

/*
** TODO: fix and improve trace handling
*/

#define print_error  printf
#define print_info   printf
#define print_debug
#define print_output printf
#define print_user   printf

static void dump_buffer(const uint8_t *buffer,
                        unsigned       size,
                        const char    *comment)
{
  unsigned i;

  print_output("\nDUMP BUFFER (%s)\n", comment);
  for (i = 0; i < size; i++)
  {
    print_output("%02X ", buffer[i]);
  }
  print_output("\n");
  for (i = 0; i < size; i++)
  {
    if (20 <= buffer[i] && buffer[i] <= 255)
    {
      print_output("%c", buffer[i]);
    }
    else
    {
      print_output(".");
    }
  }
  print_output("\n");
}

/************/
/*** CODE ***/
/************/

/*
** Misc tools
*/

static void write_12bits(uint8_t *stream, uint16_t value)
{
  stream[0] = (stream[0] & 0xF0)
            | (uint8_t)(value >> 8) & 0x0F;
  stream[1] = (uint8_t)(value & 0x00FF);
}

static void close_file(FILE *file)
{
  if (NULL != file)
  {
    fflush(file);
    fclose(file);
  }
}

/*
** Packet level tools
*/

static bool_t check_crc32(const uint8_t *buffer, unsigned length);

static uint16_t get_pid(const uint8_t *packet)
{
  if (TS_SYNCHRO_BYTE != packet[0])
  { 
    print_error("%s: lost synchro, got 0x%02X instead of 0x%02X\n",
                __FUNCTION__,
                packet[0], TS_SYNCHRO_BYTE);
    return INVALID_PID;
  }

  return (((uint16_t)(packet[1] & 0x1F)) << 8) | packet[2];
}

static bool_t set_pid(uint8_t *packet, uint16_t pid)
{
  if (INVALID_PID != pid)
  {
    packet[1] = (uint8_t)((0x1F00 & pid) >> 8) | (0xE0 & packet[1]);
    packet[2] = (uint8_t)((0x00FF & pid) >> 0);
    return TRUE;
  }
  return FALSE;
}

static bool_t toggle_tsc_bits(uint8_t *packet)
{
  static int print_done = 0;
  uint8_t    old_tsc;
  uint8_t    new_tsc;

  old_tsc = packet[3] & 0xC0;
  new_tsc = (0x00 == old_tsc) ? 0x80 : 0x00;

  if (0 == print_done)
  {
    print_info("toggle TSC bits from %c%c to %c%c\n",
               (old_tsc & 0x80) ? '1' : '0',
               (old_tsc & 0x40) ? '1' : '0',
               (new_tsc & 0x80) ? '1' : '0',
               (new_tsc & 0x40) ? '1' : '0');
    print_done = 1;
  }

  packet[3] &= 0x3F;
  packet[3] |= new_tsc;
  return TRUE;
}

static bool_t read_pcr(const uint8_t *packet, pcr_t *pcr)
{
  const
  uint8_t *adaptation_field;
  uint8_t  adaptation_field_control;

  /* Read the adaptation field control word */
  adaptation_field_control = (packet[3] >> 4) & 0x03;
  if (0 == (adaptation_field_control & 0x02))
  {
    /* The packet contains no adaptation field */
    return FALSE;
  }

  /* Point out the adaptation field */
  adaptation_field = packet + 4;

  /* Check the adaptation field length */
  if (0 == adaptation_field[0])
  {
    /* The packet contains no adaptation field */
    return FALSE;
  }

  /* Check the PCR flag */
  if (0 == (adaptation_field[1] & 0x10))
  {
    /* Adaptation field with no PCR inside */
    return FALSE;
  }

  /* Read the PCR */
  pcr->base_1hb  = (adaptation_field[2] & 0x80) >> 7;
  pcr->base_32lb = ((uint32_t)(adaptation_field[2] & 0x7F)) << 25
                 | ((uint32_t)(adaptation_field[3] & 0xFF)) << 17
                 | ((uint32_t)(adaptation_field[4] & 0xFF)) <<  9
                 | ((uint32_t)(adaptation_field[5] & 0xFF)) <<  1
                 | ((uint32_t)(adaptation_field[6] & 0x80)) >>  7;
  pcr->extension = ((uint16_t)(adaptation_field[7] & 0x01)) <<  8
                 | ((uint16_t)(adaptation_field[8] & 0xFF)) <<  0;

  return TRUE;
}

static const char *get_section_name(uint16_t pid, uint8_t table_id);

static uint8_t *get_section(const uint8_t *packet, bool_t try, bool_t dump)
{
  uint16_t pid;
  uint16_t section_length;
  uint8_t  adaptation_field_control;
  uint8_t  adaptation_field_length;
  uint8_t  pointer_field;

  pid = get_pid(packet);

  /* Read the adaptation_field_control word */
  adaptation_field_control = (packet[3] >> 4) & 0x3;

  /* Go after adaptation_field_control */
  packet += 4;

  if (0 == (adaptation_field_control & 0x02))
  {
    adaptation_field_length = 0;
  }
  else
  {
    /* Read the adaptation_field_length word + 1 for its own size */
    adaptation_field_length = 1 + packet[0];
  }

  if (adaptation_field_length > (TS_PACKET_SIZE - 4))
  {
    if (! try)
    {
      print_error("%s: invalid adaptation field\n", __FUNCTION__);
    }
    return NULL;
  }

  /* Go to pointer_field */
  packet += adaptation_field_length;

  /* Read the pointer_field word  + 1 for its own size */
  pointer_field = 1 + packet[0];

  if (pointer_field > (TS_PACKET_SIZE - 4))
  {
    if (! try)
    {
      print_error("%s: invalid pointer field\n", __FUNCTION__);
    }
    return NULL;
  }

  /* Go to section */
  packet += pointer_field;

  if (dump)
  {
    print_debug("%s: dump %s section\n",
                __FUNCTION__,
                get_section_name(pid, packet[0]));
    print_debug("%s:      %02X %02X %02X %02X %02X %02X %02X %02X\n",
                __FUNCTION__,
                packet[0], packet[1], packet[2], packet[3],
                packet[4], packet[5], packet[6], packet[7]);
    print_debug("%s:      %02X %02X %02X %02X %02X %02X %02X %02X\n",
                __FUNCTION__,
                packet[8], packet[9], packet[10], packet[11],
                packet[12], packet[13], packet[14], packet[15]);
    print_debug("%s:      %02X %02X %02X %02X %02X %02X %02X %02X\n",
                __FUNCTION__,
                packet[16], packet[17], packet[18], packet[19],
                packet[20], packet[21], packet[22], packet[23]);
  }

  section_length = (((uint16_t)(packet[1] & 0x0F)) << 8) | packet[2];
  if (section_length > 1024)
  {
    if (! try)
    {
      print_error("%s: invalid section length\n", __FUNCTION__);
    }
    return NULL;
  }

  if (try)
  {
    return (uint8_t*)packet;
  }

  section_length += SECTION_HEADER_SIZE;
  section_length -= SECTION_CRC_SIZE;

  if (! check_crc32(packet, (unsigned)section_length))
  {
    print_error("%s: CRC error on TID %d (0x%02X)\n",
                __FUNCTION__, packet[0], packet[0]);
    return NULL;
  }

  return (uint8_t*)packet;
}

static bool_t is_pid_in_list(uint16_t       pid,
                             uint16_t       extended_table_id,
                             packet_info_t *list,
                             uint32_t       size)
{
  uint32_t i;

  for (i = 0; i < size; i++)
  {
    if (pid == list[i].pid)
    {
      if (extended_table_id == list[i].tid)
      {
        return TRUE;
      }
      if (0xFFFF == extended_table_id || 0xFFFF == list[i].tid)
      {
        list[i].tid = 0xFFFF;
        return TRUE;
      }
    }
  }

  return FALSE;
}

static void show_pid_list(packet_info_t *list, uint32_t size)
{
  packet_info_t temp;
  uint16_t      pid_min;
  uint32_t      nb_pids;
  uint32_t      i;
  uint32_t      j;
  uint32_t      j_min;

  if (0 == size)
  {
    print_output("%s: no PID found !\n",
                 __FUNCTION__);
    return;
  }

  nb_pids = 1;

  /*
  ** Sort the list in PID order
  */

  for (i = 0; i < size; i++)
  {
    pid_min = 0xFFFF;

    for (j = i; j < size; j++)
    {
      if (list[j].pid < pid_min)
      {
        pid_min = list[j].pid;
        j_min = j;
      }
    }

    temp = list[i];
    list[i] = list[j_min];
    list[j_min] = temp;

    if (i > 0 && list[i].pid != list[i-1].pid)
    {
      nb_pids++;
    }
  }

  /*
  ** Show the list
  */

  print_output("%s: found %lu PIDs\n",
               __FUNCTION__, nb_pids);
  for (i = 0; i < size; i++)
  {
    if (list[i].pid < 0x20)
    {
      print_output("%s:   - %d (0x%04X) - %s\n",
                   __FUNCTION__,
                   list[i].pid, list[i].pid,
                   get_section_name(list[i].pid, list[i].tid));
    }
    else if (0xFFFF == list[i].tid)
    {
      print_output("%s:   - %d (0x%04X) - PES\n",
                   __FUNCTION__,
                   list[i].pid, list[i].pid);
    }
    else
    {
      print_output("%s:   - %d (0x%04X) - PES or SECTION with table_id = %d (0x%02X)\n",
                   __FUNCTION__,
                   list[i].pid, list[i].pid,
                   list[i].tid, list[i].tid);
    }
  }
}

static uint32_t pcr1_minus_pcr2(const pcr_t *pcr1, const pcr_t *pcr2)
{
  uint32_t pcr_diff;

  if (pcr1->base_1hb == pcr2->base_1hb)
  {
    pcr_diff = pcr1->base_32lb - pcr2->base_32lb;
  }
  else
  {
    pcr_diff = pcr1->base_32lb - (0xFFFFFFFF - pcr2->base_32lb);
  }

  /*
  ** TODO: extension part of PCR
  */

  return pcr_diff;
}

static uint32_t get_bitrate(const packet_time_t *time1,
                            const packet_time_t *time2,
                            uint32_t            *duration)
{
  unsigned long long bitrate = 0;
  uint32_t           pcr_diff;

  pcr_diff = pcr1_minus_pcr2(&time1->pcr, &time2->pcr);
  if (0 != pcr_diff)
  {
    bitrate  = time1->count - time2->count + 1;
    bitrate *= 8 * TS_PACKET_SIZE;
    bitrate *= TS_PCR_BASE_CLOCK;
    bitrate /= pcr_diff;
  }

  *duration = pcr_diff / TS_PCR_BASE_CLOCK;

  return (uint32_t)bitrate;
}

static void show_bitrate(uint16_t pid, const packet_time_t *time)
{
  static uint32_t      nb_read_pcrs = 0;
  static packet_time_t first_time;
  static packet_time_t previous_time;
  uint32_t             instant_bitrate;
  uint32_t             average_bitrate;
  uint32_t             duration;

  if (0 == nb_read_pcrs)
  {
    first_time = *time;
    nb_read_pcrs = 1;
  }
  else if (0 == nb_read_pcrs++ % 40) /* this magic gives the number of pcrs between two displays */
  {
    instant_bitrate = get_bitrate(time, &previous_time, &duration);
    average_bitrate = get_bitrate(time, &first_time, &duration);

    print_output("%s: on pid %d, instant bitrate = %lubps, average bitrate = %lubps, time = %lus\n",
                 __FUNCTION__,
                 pid, instant_bitrate, average_bitrate, duration);
  }

  previous_time = *time;
}

/*
** Section level tools
*/

static uint32_t crc32_table[256];

const uint32_t const_crc32_table[256] =
{
  0x00000000, 0x04c11db7, 0x09823b6e, 0x0d4326d9, 0x130476dc, 0x17c56b6b,
  0x1a864db2, 0x1e475005, 0x2608edb8, 0x22c9f00f, 0x2f8ad6d6, 0x2b4bcb61,
  0x350c9b64, 0x31cd86d3, 0x3c8ea00a, 0x384fbdbd, 0x4c11db70, 0x48d0c6c7,
  0x4593e01e, 0x4152fda9, 0x5f15adac, 0x5bd4b01b, 0x569796c2, 0x52568b75,
  0x6a1936c8, 0x6ed82b7f, 0x639b0da6, 0x675a1011, 0x791d4014, 0x7ddc5da3,
  0x709f7b7a, 0x745e66cd, 0x9823b6e0, 0x9ce2ab57, 0x91a18d8e, 0x95609039,
  0x8b27c03c, 0x8fe6dd8b, 0x82a5fb52, 0x8664e6e5, 0xbe2b5b58, 0xbaea46ef,
  0xb7a96036, 0xb3687d81, 0xad2f2d84, 0xa9ee3033, 0xa4ad16ea, 0xa06c0b5d,
  0xd4326d90, 0xd0f37027, 0xddb056fe, 0xd9714b49, 0xc7361b4c, 0xc3f706fb,
  0xceb42022, 0xca753d95, 0xf23a8028, 0xf6fb9d9f, 0xfbb8bb46, 0xff79a6f1,
  0xe13ef6f4, 0xe5ffeb43, 0xe8bccd9a, 0xec7dd02d, 0x34867077, 0x30476dc0,
  0x3d044b19, 0x39c556ae, 0x278206ab, 0x23431b1c, 0x2e003dc5, 0x2ac12072,
  0x128e9dcf, 0x164f8078, 0x1b0ca6a1, 0x1fcdbb16, 0x018aeb13, 0x054bf6a4,
  0x0808d07d, 0x0cc9cdca, 0x7897ab07, 0x7c56b6b0, 0x71159069, 0x75d48dde,
  0x6b93dddb, 0x6f52c06c, 0x6211e6b5, 0x66d0fb02, 0x5e9f46bf, 0x5a5e5b08,
  0x571d7dd1, 0x53dc6066, 0x4d9b3063, 0x495a2dd4, 0x44190b0d, 0x40d816ba,
  0xaca5c697, 0xa864db20, 0xa527fdf9, 0xa1e6e04e, 0xbfa1b04b, 0xbb60adfc,
  0xb6238b25, 0xb2e29692, 0x8aad2b2f, 0x8e6c3698, 0x832f1041, 0x87ee0df6,
  0x99a95df3, 0x9d684044, 0x902b669d, 0x94ea7b2a, 0xe0b41de7, 0xe4750050,
  0xe9362689, 0xedf73b3e, 0xf3b06b3b, 0xf771768c, 0xfa325055, 0xfef34de2,
  0xc6bcf05f, 0xc27dede8, 0xcf3ecb31, 0xcbffd686, 0xd5b88683, 0xd1799b34,
  0xdc3abded, 0xd8fba05a, 0x690ce0ee, 0x6dcdfd59, 0x608edb80, 0x644fc637,
  0x7a089632, 0x7ec98b85, 0x738aad5c, 0x774bb0eb, 0x4f040d56, 0x4bc510e1,
  0x46863638, 0x42472b8f, 0x5c007b8a, 0x58c1663d, 0x558240e4, 0x51435d53,
  0x251d3b9e, 0x21dc2629, 0x2c9f00f0, 0x285e1d47, 0x36194d42, 0x32d850f5,
  0x3f9b762c, 0x3b5a6b9b, 0x0315d626, 0x07d4cb91, 0x0a97ed48, 0x0e56f0ff,
  0x1011a0fa, 0x14d0bd4d, 0x19939b94, 0x1d528623, 0xf12f560e, 0xf5ee4bb9,
  0xf8ad6d60, 0xfc6c70d7, 0xe22b20d2, 0xe6ea3d65, 0xeba91bbc, 0xef68060b,
  0xd727bbb6, 0xd3e6a601, 0xdea580d8, 0xda649d6f, 0xc423cd6a, 0xc0e2d0dd,
  0xcda1f604, 0xc960ebb3, 0xbd3e8d7e, 0xb9ff90c9, 0xb4bcb610, 0xb07daba7,
  0xae3afba2, 0xaafbe615, 0xa7b8c0cc, 0xa379dd7b, 0x9b3660c6, 0x9ff77d71,
  0x92b45ba8, 0x9675461f, 0x8832161a, 0x8cf30bad, 0x81b02d74, 0x857130c3,
  0x5d8a9099, 0x594b8d2e, 0x5408abf7, 0x50c9b640, 0x4e8ee645, 0x4a4ffbf2,
  0x470cdd2b, 0x43cdc09c, 0x7b827d21, 0x7f436096, 0x7200464f, 0x76c15bf8,
  0x68860bfd, 0x6c47164a, 0x61043093, 0x65c52d24, 0x119b4be9, 0x155a565e,
  0x18197087, 0x1cd86d30, 0x029f3d35, 0x065e2082, 0x0b1d065b, 0x0fdc1bec,
  0x3793a651, 0x3352bbe6, 0x3e119d3f, 0x3ad08088, 0x2497d08d, 0x2056cd3a,
  0x2d15ebe3, 0x29d4f654, 0xc5a92679, 0xc1683bce, 0xcc2b1d17, 0xc8ea00a0,
  0xd6ad50a5, 0xd26c4d12, 0xdf2f6bcb, 0xdbee767c, 0xe3a1cbc1, 0xe760d676,
  0xea23f0af, 0xeee2ed18, 0xf0a5bd1d, 0xf464a0aa, 0xf9278673, 0xfde69bc4,
  0x89b8fd09, 0x8d79e0be, 0x803ac667, 0x84fbdbd0, 0x9abc8bd5, 0x9e7d9662,
  0x933eb0bb, 0x97ffad0c, 0xafb010b1, 0xab710d06, 0xa6322bdf, 0xa2f33668,
  0xbcb4666d, 0xb8757bda, 0xb5365d03, 0xb1f740b4
};

static void init_crc(void)
{
  uint32_t i, j, k;

  for( i = 0; i < 256; i++ )
  {
    k = 0;
    for (j = (i << 24) | 0x800000; j != 0x80000000; j <<= 1)
    {
      k = (k << 1) ^ (((k ^ j) & 0x80000000) ? 0x04C11DB7 : 0);
    }

    crc32_table[i] = k;
  }

  print_debug("compare crc32 tables : %d\n",
              memcmp(crc32_table, const_crc32_table, sizeof (crc32_table)));
}

static uint32_t get_crc_v1(const uint8_t *buffer, unsigned length)
{
  uint32_t crc = 0xFFFFFFFF;
  unsigned i;

  for (i = 0; i < length; i++)
  {
    crc = (crc << 8) ^ crc32_table[((crc >> 24) ^ buffer[i]) & 0xFF];
  }
 
  return crc;
}

static uint32_t get_crc_v2(uint32_t       initial_crc,
                           const uint8_t *buffer,
                           unsigned       length)
{
  uint32_t crc = 0xFFFFFFFF;

  if (0 != initial_crc)
  {
    crc = initial_crc;
  }

  while (length--)
  {
    crc = (crc << 8) ^ const_crc32_table[((crc >> 24) ^ *buffer++) & 0xFF];
  }

  return crc;
}

static bool_t check_crc32(const uint8_t *buffer, unsigned length)
{
  uint32_t found_crc;
  uint32_t read_crc;

  //found_crc = get_crc_v1(buffer, length);
  found_crc = get_crc_v2(0, buffer, length);

  read_crc = ((uint32_t)buffer[length + 0]) << 24
           | ((uint32_t)buffer[length + 1]) << 16
           | ((uint32_t)buffer[length + 2]) <<  8
           | ((uint32_t)buffer[length + 3]) <<  0;

  print_debug("read CRC 0x%X and found 0x%X\n", read_crc, found_crc);

  return read_crc == found_crc;
}

static void update_crc32(uint8_t *buffer, unsigned length)
{
  uint32_t crc;

  crc = get_crc_v2(0, buffer, length);

  buffer[length + 0] = (uint8_t)((crc >> 24) & 0xFF);
  buffer[length + 1] = (uint8_t)((crc >> 16) & 0xFF);
  buffer[length + 2] = (uint8_t)((crc >>  8) & 0xFF);
  buffer[length + 3] = (uint8_t)((crc >>  0) & 0xFF);

  print_debug("%s: update crc32 0x%08lX\n",
              __FUNCTION__, crc);
}

static void increment_version_number(uint8_t *section)
{
  uint8_t version_number;

  /*
  ** Make sure this function works for PAT & PMT
  */

  version_number = (section[5] & 0x3E) >> 1;
  if (++version_number >= 0x1F)
  {
    version_number = 1;
  }
  section[5] &= 0xC1;
  section[5] |= version_number << 1;

  print_info("%s: update version number %d\n",
             __FUNCTION__,
             version_number);
}

static const char *get_section_name(uint16_t pid, uint8_t table_id)
{
  if (PAT_PID == pid && PAT_TID == table_id)
  {
    return "PAT";
  }

  if (CAT_PID == pid && CAT_TID == table_id)
  {
    return "CAT";
  }

  if (NIT_PID == pid && NIT_ACTUAL_TID == table_id)
  {
    return "NIT ACTUAL";
  }

  if (NIT_PID == pid && NIT_OTHER_TID == table_id)
  {
    return "NIT OTHER";
  }

  if (SDT_PID == pid && (SDT_ACTUAL_TID == table_id || SDT_OTHER_TID == table_id))
  {
    return "SDT";
  }

  if (BAT_PID == pid && BAT_TID == table_id)
  {
    return "BAT";
  }

  if (EIT_PID == pid && (EITPF_ACTUAL_TID == table_id || EITPF_OTHER_TID == table_id))
  {
    return "EITPF";
  }

  if (EIT_PID == pid
      && (EIT_SCHEDULE_ACTUAL_MIN_TID <= table_id && table_id <= EIT_SCHEDULE_ACTUAL_MAX_TID))
  {
    return "EIT SCHEDULE ACTUAL";
  }

  if (EIT_PID == pid
      && (EIT_SCHEDULE_OTHER_MIN_TID <= table_id && table_id <= EIT_SCHEDULE_OTHER_MAX_TID))
  {
    return "EIT SCHEDULE OTHER";
  }

  if (RST_PID == pid && RST_TID == table_id)
  {
    return "RST";
  }

  if (TDT_PID == pid && TDT_TID == table_id)
  {
    return "TDT";
  }

  if (TOT_PID == pid && TOT_TID == table_id)
  {
    return "TOT";
  }

  if ((NIT_PID == pid || SDT_PID == pid || EIT_PID == pid || RST_PID == pid || TDT_PID == pid)
      && ST_TID == table_id)
  {
    return "STUFFING";
  }

  if (PMT_TID == table_id)
  {
    return "PMT";
  }

  return "OTHER";
}

static const char *get_descriptor_name(uint8_t descriptor_tag)
{
  const char *name;

  switch (descriptor_tag)
  {
  case NETWORK_NAME_DESCRIPTOR_TAG:              name = "NETWORK_NAME";              break;
  case SERVICE_LIST_DESCRIPTOR_TAG:              name = "SERVICE_LIST";              break;
  case STUFFING_DESCRIPTOR_TAG:                  name = "STUFFING";                  break;
  case SATELLITE_DELIVERY_SYSTEM_DESCRIPTOR_TAG: name = "SATELLITE_DELIVERY_SYSTEM"; break;
  case CABLE_DELIVERY_SYSTEM_DESCRIPTOR_TAG:     name = "CABLE_DELIVERY_SYSTEM";     break;
  case VBI_DATA_DESCRIPTOR_TAG:                  name = "VBI_DATA";                  break;
  case VBI_TELETEXT_DESCRIPTOR_TAG:              name = "VBI_TELETEXT";              break;
  case BOUQUET_NAME_DESCRIPTOR_TAG:              name = "BOUQUET_NAME";              break;
  case SERVICE_DESCRIPTOR_TAG:                   name = "SERVICE";                   break;
  case MULTILINGUAL_SERVICE_NAME_DESCRIPTOR_TAG: name = "MULTILINGUAL_SERVICE_NAME"; break;
  default:                                       name = "UNKNOWN";                   break;
  }

  return name;
}

/*
** PAT tools
*/

#define PAT_PROGRAM_SIZE      4
#define PAT_PROGRAM_OFFSET   (SECTION_TABLE_ID_SIZE            + \
                              SECTION_SECTION_LENGTH_SIZE      + \
                              SECTION_TRANSPORT_STREAM_ID_SIZE + \
                              SECTION_VERSION_NUMBER_SIZE      + \
                              SECTION_SECTION_NUMBER_SIZE      + \
                              SECTION_LAST_SECTION_NUMBER_SIZE)
#define PAT_NOT_PAYLOAD_SIZE (SECTION_TRANSPORT_STREAM_ID_SIZE + \
                              SECTION_VERSION_NUMBER_SIZE      + \
                              SECTION_SECTION_NUMBER_SIZE      + \
                              SECTION_LAST_SECTION_NUMBER_SIZE + \
                              SECTION_CRC_SIZE)
#define PAT_MINIMUM_LENGTH   (SECTION_TRANSPORT_STREAM_ID_SIZE + \
                              SECTION_VERSION_NUMBER_SIZE      + \
                              SECTION_SECTION_NUMBER_SIZE      + \
                              SECTION_LAST_SECTION_NUMBER_SIZE + \
                              PAT_PROGRAM_SIZE                 + \
                              SECTION_CRC_SIZE)

static const uint8_t *get_pat_payload(const uint8_t *section,
                                      pat_header_t  *header)
{
  if (PAT_TID != section[0])
  {
    print_error("%s: not a PAT, TID=%d (0x%02X)\n",
                __FUNCTION__,
                section[0], section[0]);
    return NULL;
  }

  header->section_length = (((uint16_t)(section[1] & 0x0F)) << 8)
                         | section[2];
  if (header->section_length < PAT_MINIMUM_LENGTH)
  {
    print_error("%s: the PAT does not contain any program (len=%d)\n",
                __FUNCTION__,
                header->section_length);
    return NULL;
  }

  header->transport_stream_id = (((uint16_t)section[3]) << 8) | section[4];

  header->version_number = (section[5] & 0x3E) >> 1;

  header->section_number = section[6];

  header->last_section_number = section[7];

  header->number_of_programs = (header->section_length - PAT_NOT_PAYLOAD_SIZE)
                             / PAT_PROGRAM_SIZE;

  return section + PAT_PROGRAM_OFFSET;
}

static bool_t patch_pat_header(pat_header_t *header,
                               const prog_t *target_prog,
                               const prog_t *new_prog,
                               uint8_t      *section)
{
  print_info("%s: replace transport stream id %d by %d\n",
             __FUNCTION__,
             target_prog->tsid, new_prog->tsid);

  section[3] = (uint8_t)((0xFF00 & new_prog->tsid) >> 8);
  section[4] = (uint8_t)((0x00FF & new_prog->tsid) >> 0);

  header->transport_stream_id = new_prog->tsid;

  return TRUE;
}

static bool_t patch_pat_program(const pat_header_t *header,
                                const prog_t       *target_prog,
                                const prog_t       *new_prog,
                                uint8_t            *program)
{
  unsigned i;
  unsigned n;
  uint16_t program_number;

  n = header->number_of_programs;

  for (i = 0;
       i < n;
       i++, program += PAT_PROGRAM_SIZE) 
  {
    program_number = (((uint16_t)program[0]) << 8) | program[1];

    if (target_prog->number == program_number)
    {
      if (INVALID_PROGRAM_NUMBER != new_prog->number)
      {
        print_info("%s: replace program number %d by %d\n",
                   __FUNCTION__,
                   target_prog->number, new_prog->number);

        program[0] = (uint8_t)((0xFF00 & new_prog->number) >> 8);
        program[1] = (uint8_t)((0x00FF & new_prog->number) >> 0);
      }

      if (INVALID_PID != new_prog->pmt_pid)
      {
        print_info("%s: set pmt pid to %d (0x%X) to program number %d\n",
                   __FUNCTION__,
                   new_prog->pmt_pid, new_prog->pmt_pid,
                   target_prog->number);

        program[2] = (0xE0 & program[2])
                   | (uint8_t)((0x1F00 & new_prog->pmt_pid) >> 8);
        program[3] = (uint8_t)((0x00FF & new_prog->pmt_pid) >> 0);
      }

      return TRUE;
    }
  }

  return FALSE;
}

static uint16_t get_pmt_pid(const pat_header_t *header,
                            const uint8_t      *program,
                            uint16_t            target_program_number)
{
  unsigned i;
  unsigned n;
  uint16_t program_number;

  n = header->number_of_programs;

  for (i = 0;
       i < n;
       i++, program += PAT_PROGRAM_SIZE) 
  {
    program_number = (((uint16_t)program[0]) << 8) | program[1];

    if (target_program_number == program_number)
    {
      return (((uint16_t)(program[2] & 0x1F)) << 8) | program[3];
    }
  }

  print_debug("%s: program number %d (0x%X) not found\n",
              __FUNCTION__,
              target_program_number,
              target_program_number);

  return INVALID_PID;
}

static bool_t get_first_program(const pat_header_t *header,
                                const uint8_t      *program,
                                prog_t             *first_prog)
{
  unsigned i;
  unsigned n;

  n = header->number_of_programs;

  for (i = 0;
       i < n;
       i++, program += PAT_PROGRAM_SIZE) 
  {
    first_prog->number = (((uint16_t)program[0]) << 8) | program[1];
    first_prog->pmt_pid = (((uint16_t)(program[2] & 0x1F)) << 8) | program[3];

    if (0 != first_prog->number)
    {
      print_info("%s: got program number %d (0x%X) with PMT PID %d (0x%X)\n",
                  __FUNCTION__,
                  first_prog->number, first_prog->number,
                  first_prog->pmt_pid, first_prog->pmt_pid);
      return TRUE;
    }
  }

  print_info("%s: no program available\n", __FUNCTION__);

  return FALSE;
}

static void show_pat(const pat_header_t *header,
                     const uint8_t      *program)
{
  unsigned i;
  unsigned n;
  uint16_t program_number;
  uint16_t pid;

  print_output("PAT version %d of TSid %d (0x%X) has %d programs\n", 
               header->version_number,
               header->transport_stream_id,
               header->transport_stream_id,
               header->number_of_programs);

  n = header->number_of_programs;

  for (i = 0;
       i < n;
       i++, program += PAT_PROGRAM_SIZE) 
  {
    program_number = (((uint16_t)program[0]) << 8) | program[1];
    pid = (((uint16_t)(program[2] & 0x1F)) << 8) | program[3];

    if (0 == program_number)
    {
      print_output("  %d- network pid %d (0x%04X)\n",
                   i + 1,
                   pid, pid);
    }
    else
    {
      print_output("  %d- program %d (0x%04X) pid %d (0x%04X)\n",
                   i + 1,
                   program_number, program_number,
                   pid, pid);
    }
  }
}

/*
** PMT tools
*/

#define PMT_MINIMUM_ES_SIZE          5
#define PMT_PROGRAM_NUMBER_SIZE      2
#define PMT_PCR_PID_SIZE             2
#define PMT_PROGRAM_INFO_LENGTH_SIZE 2
#define PMT_INFO_OFFSET      (SECTION_TABLE_ID_SIZE            + \
                              SECTION_SECTION_LENGTH_SIZE      + \
                              PMT_PROGRAM_NUMBER_SIZE          + \
                              SECTION_VERSION_NUMBER_SIZE      + \
                              SECTION_SECTION_NUMBER_SIZE      + \
                              SECTION_LAST_SECTION_NUMBER_SIZE + \
                              PMT_PCR_PID_SIZE                 + \
                              PMT_PROGRAM_INFO_LENGTH_SIZE)
#define PMT_NOT_PAYLOAD_SIZE (SECTION_TRANSPORT_STREAM_ID_SIZE + \
                              SECTION_VERSION_NUMBER_SIZE      + \
                              SECTION_SECTION_NUMBER_SIZE      + \
                              SECTION_LAST_SECTION_NUMBER_SIZE + \
                              SECTION_CRC_SIZE)
#define PMT_MINIMUM_LENGTH   (PMT_PROGRAM_NUMBER_SIZE          + \
                              SECTION_VERSION_NUMBER_SIZE      + \
                              SECTION_SECTION_NUMBER_SIZE      + \
                              SECTION_LAST_SECTION_NUMBER_SIZE + \
                              PMT_PCR_PID_SIZE                 + \
                              PMT_PROGRAM_INFO_LENGTH_SIZE     + \
                              PMT_MINIMUM_ES_SIZE              + \
                              SECTION_CRC_SIZE)

#define OTHER_TYPE 0
#define AUDIO_TYPE 1
#define VIDEO_TYPE 2

static const char *get_stream_type_name(uint8_t stream_type)
{
  /*
  ** TO BE COMLPLETED
  */

  switch (stream_type)
  {
  case 0x01:
    return "video-11172";
  case 0x02:
    return "video-13818";
  case 0x03:
    return "audio-11172";
  case 0x04:
    return "audio-13818";
  case 0x05:
    return "private_sections";
  case 0x06:
    return "private_data";
  case 0x07:
    return "mheg";
  case 0x08:
    return "dsm-cc";
  case 0x0F:
    return "audio-ADTS";
  case 0x11:
    return "audio-LATM";
  case 0x1B:
    return "H264";
  default:
    break;
  }

  return "unknown stream_type";
}

static unsigned get_type(uint8_t stream_type)
{
  /*
  ** TO BE COMLPLETED
  */

  switch (stream_type)
  {
  case 0x01:
  case 0x02:
  case 0x1B:
    return VIDEO_TYPE;

  case 0x03:
  case 0x04:
  case 0x0F:
  case 0x11:
    return AUDIO_TYPE;

  default:
    break;
  }

  return OTHER_TYPE;
}

static const uint8_t *get_pmt_payload(const uint8_t *section,
                                      pmt_header_t  *header)
{
  const
  uint8_t  *es;
  const
  uint8_t  *crc;
  const
  uint8_t  *first_es;
  uint16_t  es_info_length;

  if (PMT_TID != section[0])
  {
    print_error("%s: not a PMT, TID=0x%02X\n",
                __FUNCTION__,
                section[0]);
    return NULL;
  }

  header->section_length = (((uint16_t)(section[1] & 0x0F)) << 8)
                         | section[2];
  if (header->section_length < PMT_MINIMUM_LENGTH)
  {
     print_error("%s: the PMT does not contain any ES (len=%d)\n",
                 __FUNCTION__,
                 header->section_length);
     return NULL;
  }

  crc = section + SECTION_HEADER_SIZE + header->section_length - 4;

  header->program_number = (((uint16_t)section[3]) << 8) | section[4];

  header->version_number = (section[5] & 0x3E) >> 1;

  header->section_number = section[6];

  header->last_section_number = section[7];

  header->pcr_pid = (((uint16_t)(section[8] & 0x1F)) << 8) | section[9];

  header->program_info_length = (((uint16_t)(section[10] & 0x0F)) << 8)
                              | section[11];

  header->number_of_es = 0;

  for (first_es = es = section + PMT_INFO_OFFSET + header->program_info_length;
       es < crc; ) 
  {
    header->number_of_es++;

    es_info_length = (((uint16_t)(es[3] & 0x0F)) << 8) | es[4];

    es += PMT_MINIMUM_ES_SIZE;
    es += es_info_length;
  }

  assert(crc == es);

  return first_es;
}

static bool_t patch_pmt_header(pmt_header_t *header,
                               const prog_t *target_prog,
                               const prog_t *new_prog,
                               uint8_t      *section)
{
  print_info("%s: replace program number %d by %d\n",
             __FUNCTION__,
             target_prog->number, new_prog->number);

  section[3] = (uint8_t)((0xFF00 & new_prog->number) >> 8);
  section[4] = (uint8_t)((0x00FF & new_prog->number) >> 0);

  header->program_number = new_prog->number;

  return TRUE;
}

static bool_t patch_pmt_es(const pmt_header_t *header,
                           const prog_t       *target_prog,
                           const prog_t       *new_prog,
                           uint8_t            *payload)
{
  uint16_t  old_pid;
  uint16_t  new_pid;
  uint16_t  es_info_length;
  uint8_t  *es;
  unsigned  i;
  bool_t    patch = FALSE;

  for (es = payload, i = 0; i < header->number_of_es; i++)
  {
    old_pid = (((uint16_t)(es[1] & 0x1F)) << 8) | es[2];
    new_pid = INVALID_PID;

    es_info_length = (((uint16_t)(es[3] & 0x0F)) << 8) | es[4];

    if (old_pid == target_prog->pcr_pid)
    {
      new_pid = new_prog->pcr_pid;
    }
    if (old_pid == target_prog->audio_pid)
    {
      new_pid = new_prog->audio_pid;
    }
    else if (old_pid == target_prog->video_pid)
    {
      new_pid = new_prog->video_pid;
    }

    if (new_pid != INVALID_PID)
    {
      es[1] = (uint8_t)((0x1F00 & new_pid) >> 8) | (0xE0 & es[1]);
      es[2] = (uint8_t)((0x00FF & new_pid) >> 0);
      patch = TRUE;
    }

    es += PMT_MINIMUM_ES_SIZE + es_info_length;
  }

  return patch;
}

static unsigned get_es_list(const pmt_header_t *header,
                            const uint8_t      *es,
                            pmt_es_t           *es_list,
                            unsigned            list_size)
{
  unsigned i;
  unsigned n;

  n = header->number_of_es;

  for (i = 0;
       i < n;
       i++, es_list++) 
  {
    assert(i < list_size);
    if (i >= list_size)
    {
      break;
    }

    es_list->stream_type = es[0];
    es_list->pid = (((uint16_t)(es[1] & 0x1F)) << 8) | es[2];
    es_list->info_length = (((uint16_t)(es[3] & 0x0F)) << 8) | es[4];
    /* memcpy(es_list->info; &es[5]; es_list->info_length); */

    es += PMT_MINIMUM_ES_SIZE;
    es += es_list->info_length;
  }

  return i;
}

static uint16_t get_first_pid(const pmt_header_t *header,
                              const uint8_t      *payload,
                              unsigned            type)
{
  pmt_es_t es_list[16];
  unsigned i;
  unsigned n;

  n = get_es_list(header, payload, es_list,
                  sizeof (es_list) / sizeof (es_list[0]));

  for (i = 0; i < n; i++)
  {
    if (get_type(es_list[i].stream_type) == type)
    {
      return es_list[i].pid;
    }
  }

  return INVALID_PID;
}

static void show_pmt(const pmt_header_t *header,
                     const uint8_t      *es)
{
  unsigned i;
  unsigned n;
  uint16_t elementary_pid;
  uint16_t info_length;
  uint8_t  stream_type;

  print_output("PMT version %d (0x%02X) for program %d (0x%02X) has %d elementary streams\n", 
               header->version_number, header->version_number,
               header->program_number, header->program_number,
               header->number_of_es);

  n = header->number_of_es;

  for (i = 0;
       i < n;
       i++) 
  {
    stream_type = es[0];
    elementary_pid = (((uint16_t)(es[1] & 0x1F)) << 8) | es[2];
    info_length = (((uint16_t)(es[3] & 0x0F)) << 8) | es[4];

    print_output("  %d- type 0x%02X (%s) pid %d (0x%02X)\n",
                 i + 1,
                 stream_type,
                 get_stream_type_name(stream_type),
                 elementary_pid, elementary_pid);

    es += PMT_MINIMUM_ES_SIZE;
    es += info_length;
  }
}

/*
** SDT tools
*/

#define SDT_MINIMUM_SIZE            (11)
#define SDT_LOOP_MINIMUM_SIZE       (5)
#define SDT_DESCRIPTOR_MINIMUM_SIZE (2)
#define SDT_MINIMUM_LENGTH          (SDT_MINIMUM_SIZE - SECTION_HEADER_SIZE)

static const char *get_service_type_description(uint8_t service_type)
{
  switch (service_type)
  {
  case 0x01: return "digital television service";
  case 0x02: return "digital radio sound service";
  case 0x03: return "Teletext service";
  case 0x04: return "NVOD reference service";
  case 0x05: return "NVOD tume-shifted service";
  case 0x06: return "mosaic service";
  case 0x0A: return "advanced codec digital radio sound service";
  case 0x0B: return "advanced codec mosaic service";
  case 0x0C: return "data broadcast service";
  case 0x0D: return "reserved for Common Interface Usage";
  case 0x0E: return "RCS Map";
  case 0x0F: return "RCS FLS";
  case 0x10: return "DVB MHP service";
  case 0x11: return "MPEG-2 HD digital television service";
  case 0x16: return "advanced codec SD digital television service";
  case 0x17: return "advanced codec SD NVOD time-shifted service";
  case 0x18: return "advanced codec SD NVOD reference service";
  case 0x19: return "advanced codec HD digital television service";
  case 0x1A: return "advanced codec HD NVOD time-shifted service";
  case 0x1B: return "advanced codec HD NVOD reference service";
  default:
    if (0x80 <= service_type && service_type <= 0xFE)
    {
      return "user defined";
    }
    break;
  }
  return "reserved for future use";
}

static const uint8_t *get_sdt_payload(const uint8_t *section,
                                      sdt_header_t  *header)
{
  uint16_t loop_size;
  uint16_t section_length;
  uint16_t descriptors_loop_length = 0;
  unsigned i;

  if (SDT_ACTUAL_TID != section[0])
  //if (SDT_ACTUAL_TID != section[0] && SDT_OTHER_TID != section[0])
  {
    print_error("%s: not a SDT, TID=%d (0x%02X)\n",
                __FUNCTION__,
                section[0], section[0]);
    return NULL;
  }

  header->section_length = (((uint16_t)(section[1] & 0x0F)) << 8)
                         | section[2];
  if (header->section_length < SDT_MINIMUM_LENGTH)
  {
    print_error("%s: the SDT is not correct (len=%d)\n",
                __FUNCTION__,
                header->section_length);
    return NULL;
  }

  if (header->section_length == SDT_MINIMUM_LENGTH)
  {
    print_error("%s: the SDT does not contain any service\n",
                __FUNCTION__);
    return NULL;
  }

  header->transport_stream_id = (((uint16_t)section[3]) << 8) | section[4];

  header->version_number = (section[5] & 0x3E) >> 1;

  header->section_number = section[6];

  header->last_section_number = section[7];

  header->original_network_id = (((uint16_t)section[8]) << 8) | section[9];

  header->number_of_services = 0;

  loop_size = header->section_length - SDT_MINIMUM_LENGTH;
  section_length = header->section_length
                 + SECTION_HEADER_SIZE
                 - SECTION_CRC_SIZE;

  for (i = SDT_MINIMUM_SIZE;
       i < section_length;
       i += SDT_LOOP_MINIMUM_SIZE + descriptors_loop_length)
  {
    if (loop_size < SDT_LOOP_MINIMUM_SIZE)
    {
      print_error("%s: SDT loop (size = %d) not complete\n",
                  __FUNCTION__,
                  loop_size);
      return NULL;
    }
    loop_size -= SDT_LOOP_MINIMUM_SIZE;

    descriptors_loop_length = (((uint16_t)(section[i + 3] & 0x0F)) << 8)
                            | section[i + 4];
    if (loop_size < descriptors_loop_length)
    {
      print_error("%s: SDT descriptors loop not complete\n",
                  __FUNCTION__);
      return NULL;
    }
    loop_size -= descriptors_loop_length;


    header->number_of_services++;
  }

  return section + SDT_MINIMUM_SIZE;
}

static void show_sdt(const sdt_header_t *header,
                     const uint8_t      *service)
{
  char        name[200];
  const char *name_separator = " by ";
  unsigned    i;
  uint16_t    service_id;
  uint16_t    descriptors_loop_length;
  uint8_t     descriptor_tag;
  uint8_t     descriptor_length;
  uint8_t     service_type;
  uint8_t     service_provider_name_length;
  uint8_t     service_name_length;

  print_output("SDT version %d of TSid %d (0x%X) and ONId %d (0x%X) has %d services\n", 
               header->version_number,
               header->transport_stream_id,
               header->transport_stream_id,
               header->original_network_id,
               header->original_network_id,
               header->number_of_services);

  for (i = 0; i < header->number_of_services; i++) 
  {
    service_id = (((uint16_t)service[0]) << 8) | service[1];

    descriptors_loop_length = (((uint16_t)(service[3] & 0x0F)) << 8)
                            | service[4];

    print_output("    - service %d (0x%X)\n",
                 service_id,
                 service_id);

    service += SDT_LOOP_MINIMUM_SIZE;

    while (descriptors_loop_length > 0)
    {
      descriptor_tag = service[0];
      descriptor_length = service[1];

      print_output("        - %s tag: %d (0x%X)\n",
                   get_descriptor_name(descriptor_tag),
                   descriptor_tag,
                   descriptor_tag);

      service += SDT_DESCRIPTOR_MINIMUM_SIZE;

      if (SERVICE_DESCRIPTOR_TAG == descriptor_tag)
      {
        service_type = service[0];
        service_provider_name_length = service[1];
        service_name_length = service[2 + service_provider_name_length];
        assert((service_provider_name_length + service_name_length + strlen(name_separator)) < sizeof (name));

        memcpy(name, &service[3 + service_provider_name_length], service_name_length);
        memcpy(name + service_name_length, name_separator, strlen(name_separator)); 
        memcpy(name + service_name_length + strlen(name_separator), &service[2], service_provider_name_length);
        name[service_name_length + strlen(name_separator) + service_provider_name_length] = 0;

        print_output("          type 0x%X: %s\n",
                     service_type,
                     get_service_type_description(service_type));
        print_output("          %s\n",
                     name);
      }
      else if (MULTILINGUAL_SERVICE_NAME_DESCRIPTOR_TAG == descriptor_tag)
      {
      }

      service += descriptor_length;

      if (descriptors_loop_length >= (SDT_DESCRIPTOR_MINIMUM_SIZE + descriptor_length))
      {
        descriptors_loop_length -= (SDT_DESCRIPTOR_MINIMUM_SIZE + descriptor_length);
      }
      else
      {
        print_error("%s: SDT loop error\n",
                    __FUNCTION__);
        descriptors_loop_length = 0;
      }
    }
  }
}

/*
** TODO: extract descriptor functions for code factorization
*/

static bool_t patch_sdt_provider_name(const sdt_header_t *header,
                                      const service_t    *new_svc,
                                      uint8_t            *section,
                                      uint16_t           *section_length)
{
  uint8_t  buffer[TS_PACKET_SIZE - 4]; /* temporary buffer where the secion is stored */
  unsigned i;
  unsigned len_diff;
  unsigned remaining_len;
  uint16_t service_id;
  uint16_t descriptors_loop_length;
  uint8_t  descriptor_tag;
  uint8_t  descriptor_length;
  uint8_t  service_provider_name_length;

  uint8_t *service = section + SDT_MINIMUM_SIZE;
  char    *sl;  /* section length pointer */
  char    *dll; /* descriptor loop length pointer */
  char    *dl;  /* descriptor length pointer */
  char    *spn; /* service provider name pointer */

  remaining_len = header->section_length
                + SECTION_HEADER_SIZE
                - SECTION_CRC_SIZE
                - SDT_MINIMUM_SIZE;

  /*
  ** TODO: improve this section_length update
  */
  sl = section + 1;

  for (i = 0; i < header->number_of_services; i++) 
  {
    service_id = (((uint16_t)service[0]) << 8) | service[1];

    descriptors_loop_length = (((uint16_t)(service[3] & 0x0F)) << 8)
                            | service[4];

    dll = &service[3];

    service += SDT_LOOP_MINIMUM_SIZE;
    remaining_len -= SDT_LOOP_MINIMUM_SIZE;

    if (service_id != new_svc->id)
    {
      service += descriptors_loop_length;
      remaining_len -= descriptors_loop_length;
      continue;
    }

    while (descriptors_loop_length > 0)
    {
      descriptor_tag = service[0];
      descriptor_length = service[1];

      dl = &service[1];

      service += SDT_DESCRIPTOR_MINIMUM_SIZE;
      remaining_len -= SDT_DESCRIPTOR_MINIMUM_SIZE;

      if (SERVICE_DESCRIPTOR_TAG == descriptor_tag)
      {
        service_provider_name_length = service[1];
        remaining_len -= 2;

        spn = &service[2];

        memcpy(buffer, spn, service_provider_name_length); /* debug */
        buffer[service_provider_name_length] = 0;
        print_output("%s: replace %s by %s\n",
                     __FUNCTION__,
                     buffer, new_svc->name);

        /*
        ** 3 copies:
        ** - save remaining part of the section into buffer
        ** - copy provider name into section
        ** - restore the remaining part of the section after provider name
        */
        remaining_len -= service_provider_name_length;
        memcpy(buffer, spn + service_provider_name_length, remaining_len);
        //dump_buffer(buffer, remaining_len, "saved section remaining");
        memcpy(spn, new_svc->name, strlen(new_svc->name));
        memcpy(spn + strlen(new_svc->name), buffer, remaining_len);
        //dump_buffer(section, TS_PACKET_SIZE - 4, "before length fields patches");

        /*
        ** Update all length fields
        */
        service[1] = strlen(new_svc->name); /* service_provider_name_length */
        if (service_provider_name_length > strlen(new_svc->name))
        {
          len_diff = service_provider_name_length - strlen(new_svc->name);
          descriptor_length -= len_diff;
          descriptors_loop_length -= len_diff;
          *section_length = header->section_length - len_diff;

          /*
          ** New provider name is shorter so need to fill with 0xFF
          */
          memset(spn + strlen(new_svc->name) + remaining_len + SECTION_CRC_SIZE,
                 0xFF,
                 len_diff);
        }
        else if (service_provider_name_length < strlen(new_svc->name))
        {
          len_diff = strlen(new_svc->name) - service_provider_name_length;
          descriptor_length += len_diff;
          descriptors_loop_length += len_diff;
          *section_length = header->section_length + len_diff;
        }
        else
        {
          len_diff = 0;
        }

        if (0 != len_diff)
        {
          dl[0]  = descriptor_length;
          dll[0] = (dll[0] & 0xF0) | ((descriptors_loop_length >> 8) & 0x0F);
          dll[1] = descriptors_loop_length & 0xFF;
          sl[0]  = (sl[0] & 0xF0) | ((*section_length >> 8) & 0x0F);
          sl[1]  = *section_length & 0xFF;
        }
        //dump_buffer(section, TS_PACKET_SIZE - 4, "after length fields patches");

        return TRUE;
      }

      service += descriptor_length;
      remaining_len -= descriptor_length;

      descriptors_loop_length -= (SDT_DESCRIPTOR_MINIMUM_SIZE + descriptor_length);
    }
  }

  return FALSE;
}

/*
** AIT tools
*/

typedef struct
{
  uint16_t patch;
  uint16_t l1;
  uint16_t l2;
  uint16_t l3;
  uint16_t l4;

} length_set_t;

static void patch_ait(uint8_t      *s1,      /* begining of the section */
                      uint8_t      *s2,      /* section position where replacement happens */
                      const char   *new_url, /* replacing string */
                      length_set_t *len)     /* set of length fields that are impacted by the patch */
{
  unsigned new_url_len;
  unsigned size;                             /* restoring size */
  unsigned index;                            /* index in the temporary buffer */
  uint16_t diff;                             /* string length difference between repllacing and replaced */
  static
  uint8_t  buffer[TS_PACKET_SIZE - 4];       /* temporary buffer where the secion is stored */

  //dump_buffer(s1, 184, "before ait patch");

  new_url_len = strlen(new_url);
  index = s2 - s1 + len->patch;              /* end of replaced string position */
  memcpy(buffer, s1, sizeof (buffer));       /* save section */
  memcpy(s2, new_url, new_url_len);          /* replace old string by new one */

  if (len->patch < new_url_len)
  {
    diff = new_url_len - len->patch;         /* new string is longer than old one */
    size = sizeof (buffer) - index - diff;   /* restoring size is what is left in section */
    len->l1 += diff;                         /* update all lengths */
    len->l2 += diff;
    len->l3 += diff;
    len->l4 += diff;
  }
  else
  {
    diff = len->patch - new_url_len;        /* old string is longer than new one */
    size = sizeof (buffer) - index;         /* restoring size is what follows in buffer */
    len->l1 -= diff;                        /* update all lengths */
    len->l2 -= diff;
    len->l3 -= diff;
    len->l4 -= diff;
    memset(s2 + new_url_len + size, 0xFF, diff);
  }
  memcpy(s2 + new_url_len, &buffer[index], size); /* restore the remaining of the section */

  len->patch = new_url_len;

  //dump_buffer(s1, 184, "after ait patch");
}

static bool_t parse_ait(uint8_t     *section,
                        const ait_t *new_ait,
                        uint16_t    *length)
{
  /*
  ** Does not manage the common decriptors loop
  */
  uint8_t  *s                                            = section;
  uint8_t  *section_length_position                      = NULL;
  uint8_t  *application_loop_length_position             = NULL;
  uint8_t  *application_descriptors_loop_length_position = NULL;
  uint8_t  *base_descriptor_length_position              = NULL;
  uint8_t  *file_descriptor_length_position              = NULL;

  uint16_t section_length;
  uint16_t application_type;
  uint16_t application_id;
  uint16_t application_loop_length;
  uint16_t application_descriptors_loop_length;
  uint16_t common_descriptors_length;
  uint16_t protocol_id;
  uint16_t application_profile;
  uint8_t  table_id;
  uint8_t  version_number;
  uint8_t  section_number;
  uint8_t  last_section_number;
  uint8_t  application_control_code;
  uint8_t  descriptor_tag;
  uint8_t  descriptor_length;
  uint8_t  transport_protocol_label;
  uint8_t  URL_base_length;
  uint8_t  URL_extension_count;
  uint8_t  URL_extension_length;
  uint8_t  application_profiles_length;
  uint8_t  service_bound_flag;
  uint8_t  visibility;
  uint8_t  application_priority;
  uint8_t  application_name_length;
  uint8_t  i;
  uint8_t  j;
  uint8_t  k;
  uint8_t  l;
  bool_t   patch = FALSE;

  table_id = s[0]; s++;
  section_length_position = s;
  section_length = (((uint16_t)(s[0] & 0x0F)) << 8) | s[1]; s += 2;
  application_type = (((uint16_t)(s[0] & 0x7F)) << 8) | s[1]; s += 2;
  version_number = (s[0] & 0x3E) >> 1; s++;
  section_number = s[0]; s++;
  last_section_number = s[0]; s++;
  common_descriptors_length = (((uint16_t)(s[0] & 0x0F)) << 8) | s[1]; s += 2;
  application_loop_length_position = s;
  application_loop_length = (((uint16_t)(s[0] & 0x0F)) << 8) | s[1]; s += 2;

  print_output("\nAIT\n");
  print_output("table_id: %d (0x%02X)\n", table_id, table_id);
  if (AIT_TID != table_id)
  {
    print_output("table_id WARNING: different from 0x%02X\n", AIT_TID);
  }
  print_output("section_length: %d (0x%02X)\n", section_length, section_length);
  print_output("application_type: %d (0x%02X)\n", application_type, application_type);
  print_output("version_number: %d (0x%02X)\n", version_number, version_number);
  print_output("section_number: %d (0x%02X)\n", section_number, section_number);
  print_output("last_section_number: %d (0x%02X)\n", last_section_number, last_section_number);
  print_output("common_descriptors_length: %d (0x%02X)\n", common_descriptors_length, common_descriptors_length);
  print_output("application_loop_length: %d (0x%02X)\n", application_loop_length, application_loop_length);

  for (i = 0; i < application_loop_length; )
  {
    print_output("  organisation_id: 0x%02X%02X%02X%02X\n",
                 s[0], s[1], s[2], s[3]); s += 4;
    application_id = ((uint16_t)s[0] << 8) | s[1]; s += 2;
    print_output("  application_id:  0x%04X %s\n",
                 application_id,
                 (0      == application_id                           ) ? "(not valid)"                :
                 (0      <  application_id && application_id < 0x4000) ? "(unsigned app)"             :
                 (0x4000 <= application_id && application_id < 0x8000) ? "(signed app)"               :
                 (0x8000 <= application_id && application_id < 0xA000) ? "(RFU)"                      :
                 (0xA000 <= application_id && application_id < 0xFFFE) ? "(signed app)"               :
                 (0xFFFE == application_id                           ) ? "(wildcard for signed apps)" :
                 (0xFFFF == application_id                           ) ? "(wildcard for all apps)"    :
                                                                         "(IMPOSSIBLE)");
    if (0 != new_ait->id) /* begining of patch */
    {
      s -= 2;
      print_output("  replace by 0x%04X\n\n", new_ait->id);
      application_id = new_ait->id;
      s[0] = (uint8_t)((0xFF00 & application_id) >> 8);
      s[1] = (uint8_t)((0x00FF & application_id) >> 0);
      patch = TRUE;
      s += 2;
    } /* end of patch */

    application_control_code = s[0]; s++;
    application_descriptors_loop_length_position = s;
    application_descriptors_loop_length = (((uint16_t)(s[0] & 0x0F)) << 8) | s[1]; s += 2;

    i += 9;

    print_output("    application_control_code: %d (0x%02X) = %s\n",
                  application_control_code, application_control_code,
                  (1 == application_control_code) ? "AUTOSTART" :
                  (2 == application_control_code) ? "PRESENT" :
                  (3 == application_control_code) ? "DESTROY" :
                  (4 == application_control_code) ? "KILL" :
                  (5 == application_control_code) ? "PREFETCH" :
                  (6 == application_control_code) ? "REMOTE" :
                  (7 == application_control_code) ? "DISABLED" :
                  (8 == application_control_code) ? "PLAYBACK_AUTOSTART" :
                  "UNKNOWN");
    print_output("    application_descriptors_loop_length: %d (0x%02X)\n",
                 application_descriptors_loop_length, application_descriptors_loop_length);

    for (j = 0; j < application_descriptors_loop_length; )
    {
      print_output("\n");

      descriptor_tag = s[0]; s++;
      descriptor_length = s[0]; s++;

      i += 2;
      j += 2;

      print_output("      descriptor_tag: %d (0x%02X) = %s\n",
                   descriptor_tag, descriptor_tag,
                   (0x00 == descriptor_tag) ? "application descriptor" :
                   (0x01 == descriptor_tag) ? "application name descriptor" :
                   (0x02 == descriptor_tag) ? "transport protocol descriptor" :
                   (0x05 == descriptor_tag) ? "external application authorization descriptor" :
                   (0x06 == descriptor_tag) ? "application recording descriptor" :
                   (0x0B == descriptor_tag) ? "application icons descriptor" :
                   (0x10 == descriptor_tag) ? "application storage descriptor" :
                   (0x14 == descriptor_tag) ? "graphics constraints descriptor" :
                   (0x15 == descriptor_tag) ? "simple application location descriptor" :
                   (0x16 == descriptor_tag) ? "application usage descriptor" :
                   (0x17 == descriptor_tag) ? "simple application boundary descriptor" :
                   (0x66 == descriptor_tag) ? "data broadcast id descriptor" :
                   (0x6F == descriptor_tag) ? "application signalling descriptor" :
                   (0x71 == descriptor_tag) ? "service identifier descriptor" :
                   "UNEXPECTED");
      print_output("      descriptor_length: %d (0x%02X)\n", descriptor_length, descriptor_length);

      if (0x00 == descriptor_tag)
      {
        application_profiles_length = s[0]; s++;

        print_output("      application_profiles_length: %d (0x%02X)\n",
                     application_profiles_length, application_profiles_length);

        for (k = 0; k < application_profiles_length / 5; k++)
        {
          application_profile = (((uint16_t)s[0]) << 8) | s[1]; s += 2;
          print_output("      profile %d: %d (0x%02X)\n", k,
                       application_profile, application_profile);
          print_output("        version: %d.%d.%d\n",
                       s[0], s[1], s[2]);
          s += 3;
        }

        service_bound_flag = s[0] >> 7;
        visibility = (s[0] & 0x60) >> 5; s++;
        application_priority = s[0]; s++;

        print_output("      service_bound_flag: %d\n",
                     service_bound_flag);
        print_output("      visibility: %d\n",
                     visibility);
        print_output("      application_priority: %d (0x%02X)\n",
                     application_priority, application_priority);

        l  = descriptor_length;
        l -= 1;
        l -= application_profiles_length;
        l -= 2;

        for (k = 0; k < l; k++)
        {
          transport_protocol_label = s[0]; s++;
          print_output("      transport_protocol_label %d: %d (0x%02X)\n", k,
                       transport_protocol_label, transport_protocol_label);
        }
      }
      else if (0x01 == descriptor_tag)
      {
        if (descriptor_length > 3)
        {
          for (k = 0; k < descriptor_length; )
          {
            print_output("      (%c%c%c) ",
                         s[0], s[1], s[2]);
            s += 3;

            application_name_length = s[0]; s++;

            for (l = 0; l < application_name_length; l++)
            {
              print_output("%c", s[l]);
            }
            print_output("\n");

            k += 4;
            k += application_name_length;
            s += application_name_length;
          }
        }
      }
      else if (0x02 == descriptor_tag)
      {
        base_descriptor_length_position = s - 1;

        protocol_id = (((uint16_t)s[0]) << 8) | s[1]; s += 2;
        transport_protocol_label = s[0]; s++;

        print_output("      protocol_id: %d (0x%02X) = %s\n",
                     protocol_id, protocol_id,
                     (1 == protocol_id) ? "Object Carousel" :
                     (3 == protocol_id) ? "HTTP" :
                     "UNEXPECTED");
        print_output("      transport_protocol_label: %d (0x%02X)\n",
                     transport_protocol_label, transport_protocol_label);

        if (3 == protocol_id)
        {
          URL_base_length = s[0]; s++;

          if (0 != URL_base_length)
          { 
            print_output("      URL base: ");
            for (k = 0; k < URL_base_length; k++)
            {
              print_output("%c", s[k]);
            }
            print_output("\n");

            if (NULL != new_ait->url.base) /* begining of patch */
            {
              length_set_t length_set;

              length_set.patch = URL_base_length;
              length_set.l1    = descriptor_length;
              length_set.l2    = application_descriptors_loop_length;
              length_set.l3    = application_loop_length;
              length_set.l4    = section_length;

              print_output("      replace by %s\n", new_ait->url.base);
              patch_ait(section, s, new_ait->url.base, &length_set);
              patch = TRUE;

              URL_base_length                     = length_set.patch;
              descriptor_length                   = length_set.l1;
              application_descriptors_loop_length = length_set.l2;
              application_loop_length             = length_set.l3;
              section_length                      = length_set.l4;

              *(s - 1) = URL_base_length;
              base_descriptor_length_position[0] = descriptor_length;
              write_12bits(application_descriptors_loop_length_position,
                           application_descriptors_loop_length);
              write_12bits(application_loop_length_position,
                           application_loop_length);
              write_12bits(section_length_position,
                           section_length);
            } /* end of patch */

            s += URL_base_length;
          }

          URL_extension_count = s[0]; s++;

          if (0 != URL_extension_count)
          {
            for (k = 0; k < URL_extension_count; k++)
            {
              print_output("      URL extension %d: ", k);
              URL_extension_length = s[0]; s++;
              for (l = 0; l < URL_extension_length; l++)
              {
                print_output("%c", s[l]);
              }
              print_output("\n");

              s += URL_extension_length;
            }
          }
        }
        else /* Object Carousel */
        {
          print_output("      ");
          for (k = 0; k < descriptor_length - 3; k++)
          {
            print_output("%c", s[k]);
          }
          print_output("\n");

          s += descriptor_length - 3;
        }
      }
      else if (0x15 == descriptor_tag)
      {
        file_descriptor_length_position = s - 1;

        print_output("      ");
        for (k = 0; k < descriptor_length; k++)
        {
          print_output("%c", s[k]);
        }
        print_output("\n");

        if (NULL != new_ait->url.file) /* begining of patch */
        {
          length_set_t length_set;

          length_set.patch = descriptor_length;
          length_set.l1    = 500; /* not used actually */
          length_set.l2    = application_descriptors_loop_length;
          length_set.l3    = application_loop_length;
          length_set.l4    = section_length;

          print_output("      replace by %s\n", new_ait->url.file);
          patch_ait(section, s, new_ait->url.file, &length_set);
          patch = TRUE;

          descriptor_length                   = length_set.patch;
          application_descriptors_loop_length = length_set.l2;
          application_loop_length             = length_set.l3;
          section_length                      = length_set.l4;

          file_descriptor_length_position[0] = descriptor_length;
          write_12bits(application_descriptors_loop_length_position,
                       application_descriptors_loop_length);
          write_12bits(application_loop_length_position,
                       application_loop_length);
          write_12bits(section_length_position,
                       section_length);
        } /* end of patch */

        s += descriptor_length;
      }
      else /* any other descriptor */
      {
        print_output("      ");
        for (k = 0; k < descriptor_length; k++)
        {
          print_output("%02X ", s[k]);
        }
        print_output("\n");
        print_output("      ");
        for (k = 0; k < descriptor_length; k++)
        {
          if (20 <= s[k] && s[k] <= 255)
          {
            print_output("%c", s[k]);
          }
          else
          {
            print_output(".");
          }
        }
        print_output("\n");

        s += descriptor_length;
      }

      i += descriptor_length;
      j += descriptor_length;
    }
  }

  *length = section_length;

  return patch;
}

/*
** HLS tools
*/

typedef enum
{
  HLS_START,
  HLS_NEXT,
  HLS_STOP,
  HLS_STREAM,

} hls_command_t;

static FILE *new_hls_chunk(char     *chunkname,
                           unsigned  suffix,
                           unsigned  index,
                           FILE     *m3u_file)
{
  FILE   *hls_file;
  size_t  nb_bytes;

  sprintf(chunkname + suffix, "_%03d.ts", index);

  hls_file = fopen(chunkname, "w");
  if (NULL == hls_file)
  {
    print_output("cannot create %s\n", chunkname);
  }

  if (NULL != m3u_file)
  {
    strcat(chunkname, "\n");
    nb_bytes = fwrite(chunkname, 1, strlen(chunkname), m3u_file);
    assert(strlen(chunkname) == nb_bytes);
  }

  return hls_file;
}

#define NO_TAG_VALUE 0xFFFFF

static void add_m3u_tag(FILE       *m3u_file,
                        const char *tag,
                        unsigned    value,
                        bool_t      comma)
{
  char   m3u_tag[50];
  size_t nb_bytes;

  if (NO_TAG_VALUE == value)
  {
    sprintf(m3u_tag, "#%s%c\n", tag, comma ? ',' : ' ');
  }
  else
  {
    sprintf(m3u_tag, "#%s:%d%c\n", tag, value, comma ? ',' : ' ');
  }

  nb_bytes = fwrite(m3u_tag, 1, strlen(m3u_tag), m3u_file);
  assert(strlen(m3u_tag) == nb_bytes);
}

static void proceed_hls(const char    *filename,
                        hls_command_t  command,
                        void          *data)
{
  static char           string[60];
  static char          *manifest;
  static FILE          *m3u_file = NULL;
  static FILE          *hls_file = NULL;
  static unsigned       suffix = 0;
  static unsigned       index = 0;
  static bool_t         previous_time_valid = 0;
  static packet_time_t  previous_time;
         packet_time_t *time;
  static uint32_t       chunk_duration = 5; /* seconds */
  uint32_t              duration;
  size_t                nb_bytes;

  if (0 == suffix)
  {
    strncpy(string,
            basename((char*)filename),
            sizeof (string) - 10); /* need room for "_%03d.ts\'0'" */
    manifest = strtok(string, "./");
    suffix = strlen(manifest);
  }

  switch (command)
  {
  case HLS_STREAM:
    nb_bytes = fwrite(data, 1, TS_PACKET_SIZE, hls_file);
    assert(TS_PACKET_SIZE == nb_bytes);
    break;

  case HLS_START:
    chunk_duration = (uint32_t)data;

    strcpy(manifest + suffix, ".m3u");
    m3u_file = fopen(manifest, "w");
    if (NULL == m3u_file)
    {
      print_output("cannot create %s\n", manifest);
    }
    else
    {
      print_output("create manifest file: %s\n", manifest);
    }
    add_m3u_tag(m3u_file, "EXTM3U", NO_TAG_VALUE, 0);
    add_m3u_tag(m3u_file, "EXT-X-STREAM-INF:PROGRAM-ID=XX,BANDWIDTH=YY", NO_TAG_VALUE, 0);
    // TODO: retrieve and write the real average bitrate value

    strcpy(manifest + suffix, "_01.m3u\n");
    nb_bytes = fwrite(manifest, 1, strlen(manifest), m3u_file);
    close_file(m3u_file);

    suffix = strlen(manifest) - 5;
    manifest[strlen(manifest) - 1] = 0;
    m3u_file = fopen(manifest, "w");
    if (NULL == m3u_file)
    {
      print_output("cannot create %s\n", manifest);
    }
    else
    {
      print_output("create manifest file: %s\n", manifest);
    }
    add_m3u_tag(m3u_file, "EXTM3U", NO_TAG_VALUE, 0);
    add_m3u_tag(m3u_file, "EXT-X-TARGETDURATION", chunk_duration, 0);
    add_m3u_tag(m3u_file, "EXT-X-MEDIA-SEQUENCE", 0, 0);

    add_m3u_tag(m3u_file, "EXTINF", chunk_duration, 1);
    hls_file = new_hls_chunk(manifest, suffix, index++, m3u_file);
    break;

  case HLS_NEXT:
    time = (packet_time_t*)data;
    if (0 == previous_time_valid)
    {
      previous_time = *time;
      previous_time_valid = 1;
      break;
    }

    get_bitrate(time, &previous_time, &duration);
    if (duration >= chunk_duration)
    {
      close_file(hls_file);

      add_m3u_tag(m3u_file, "EXTINF", chunk_duration, 1);
      hls_file = new_hls_chunk(manifest, suffix, index++, m3u_file);

      previous_time = *time;
    }
    break;

  case HLS_STOP:
    add_m3u_tag(m3u_file, "EXT-X-ENDLIST", NO_TAG_VALUE, 0);
    close_file(hls_file);
    close_file(m3u_file);
    break;

  default:
    assert(0);
    break;
  }
}

/*
** MAIN
*/

typedef enum
{
  SHOW_ALL_PID,
  SHOW_ALL_PAT,
  SHOW_ALL_PMT,
  SHOW_ALL_SDT,
  SHOW_PAT,
  SHOW_PMT,
  SHOW_SDT,
  SHOW_AIT,
  SHOW_PID,
  SHOW_BR,
  SELECT_PROGRAM_NUMBER,
  SELECT_AUDIO_PID,
  SELECT_VIDEO_PID,
  REPLACE_TRANSPORT_STREAM_ID,
  REPLACE_PROGRAM_NUMBER,
  REPLACE_PMT_PID,
  REPLACE_PCR_PID,
  REPLACE_AUDIO_PID,
  REPLACE_VIDEO_PID,
  REPLACE_PID,
  REPLACE_PROVIDER_NAME,
  REPLACE_AIT_ID,
  REPLACE_AIT_URL,
  TOGGLE_TSC,
  HLS,
  NB_COMMANDS,

} command_t;

static const char *command_string[NB_COMMANDS] =
{
  "-show-all-pid",
  "-show-all-pat",
  "-show-all-pmt",
  "-show-all-sdt",
  "-show-pat",
  "-show-pmt",
  "-show-sdt",
  "-show-ait",
  "-show-pid",
  "-show-br",
  "-pn",
  "-aud",
  "-vid",
  "-rep-tsid",
  "-rep-pn",
  "-rep-pmt",
  "-rep-pcr",
  "-rep-aud",
  "-rep-vid",
  "-rep-pid",
  "-rep-provider",
  "-rep-ait-id",
  "-rep-ait-url",
  "-toggle-tsc",
  "-hls",
};

#define IS_COMMAND(cs,c) (0 == strcmp(cs, command_string[c]))
#define IS_ACTIVE(uc,c) (0 != (uc & (COMMAND_MASK_##c)))
#define DECLARE_COMMAND_MASK(c) MASK_NAME(COMMAND_MASK_,c) = (1 << (c))
#define MASK_NAME(a,b) a##b

typedef enum
{
  DECLARE_COMMAND_MASK(SHOW_ALL_PID),
  DECLARE_COMMAND_MASK(SHOW_ALL_PAT),
  DECLARE_COMMAND_MASK(SHOW_ALL_PMT),
  DECLARE_COMMAND_MASK(SHOW_ALL_SDT),
  DECLARE_COMMAND_MASK(SHOW_PAT),
  DECLARE_COMMAND_MASK(SHOW_PMT),
  DECLARE_COMMAND_MASK(SHOW_SDT),
  DECLARE_COMMAND_MASK(SHOW_AIT),
  DECLARE_COMMAND_MASK(SHOW_PID),
  DECLARE_COMMAND_MASK(SHOW_BR),
  DECLARE_COMMAND_MASK(SELECT_PROGRAM_NUMBER),
  DECLARE_COMMAND_MASK(SELECT_AUDIO_PID),
  DECLARE_COMMAND_MASK(SELECT_VIDEO_PID),
  DECLARE_COMMAND_MASK(REPLACE_TRANSPORT_STREAM_ID),
  DECLARE_COMMAND_MASK(REPLACE_PROGRAM_NUMBER),
  DECLARE_COMMAND_MASK(REPLACE_PMT_PID),
  DECLARE_COMMAND_MASK(REPLACE_PCR_PID),
  DECLARE_COMMAND_MASK(REPLACE_AUDIO_PID),
  DECLARE_COMMAND_MASK(REPLACE_VIDEO_PID),
  DECLARE_COMMAND_MASK(REPLACE_PID),
  DECLARE_COMMAND_MASK(REPLACE_PROVIDER_NAME),
  DECLARE_COMMAND_MASK(REPLACE_AIT_ID),
  DECLARE_COMMAND_MASK(REPLACE_AIT_URL),
  DECLARE_COMMAND_MASK(TOGGLE_TSC),
  DECLARE_COMMAND_MASK(HLS),

} command_mask_t;

static bool_t goto_first_packet(FILE *file)
{
  size_t   nb_bytes;
  int      ret;
  unsigned i;
  char     c;

  for (i = 0;
       i < TS_PACKET_SIZE;
       i++)
  {
    nb_bytes = fread(&c, 1, 1, file);
    assert(1 == nb_bytes);

    if (TS_SYNCHRO_BYTE == c)
    {
      ret = fseek(file, -1, SEEK_CUR);
      assert(0 == ret);
      return TRUE;
    }
  }

  return FALSE;
}

static void parse_ts(const char      *filename,
                     FILE            *file,
                     uint32_t         command,
                     prog_t          *target_prog,
                     const prog_t    *new_prog,
                     const service_t *service,
                     const ait_t     *ait,
                     uint16_t         pid1,
                     uint16_t         pid2,
                     uint16_t         tsc_pid)
{
  int       ret;
  static
  uint8_t   packet[TS_PACKET_SIZE];
  uint8_t  *section;
  const
  uint8_t  *payload;
  uint8_t   pat_version_number = INVALID_VERSION_NUMBER;
  uint8_t   pmt_version_number = INVALID_VERSION_NUMBER;
  uint8_t   sdt_version_number = INVALID_VERSION_NUMBER;
  uint16_t  pcr_pid = INVALID_PID;
  uint16_t  pid;
  uint16_t  section_length;
  uint32_t  nb_packets;
  size_t    nb_bytes;
  bool_t    rewind_file;
  bool_t    patch_section;
  bool_t    patch_es;

  uint32_t      pid_info_list_size;
  packet_info_t pid_info_list[128];
  pat_header_t  pat_header;
  pmt_header_t  pmt_header;
  sdt_header_t  sdt_header;
  packet_time_t packet_time;
  prog_t        first_prog;

  init_prog(&first_prog);

  if (FALSE == goto_first_packet(file))
  {
    print_info("%s: not a TS file\n",
               __FUNCTION__);
    return;
  }

  nb_packets = 0;
  packet_time.count = 0;
  pid_info_list_size = 0;

  if (IS_ACTIVE(command, HLS))
  {
    proceed_hls(filename, HLS_START, (void*)5);
  }

  while (0 == feof(file))
  {
    rewind_file = FALSE;
    patch_section = FALSE;
    patch_es = FALSE;

    nb_bytes = fread(packet, 1, TS_PACKET_SIZE, file);
    if (TS_PACKET_SIZE != nb_bytes)
    {
      print_info("%s: truncated packet\n",
                 __FUNCTION__);
      break;
    }
    assert(packet[0] == TS_SYNCHRO_BYTE);

    nb_packets++;

    pid = get_pid(packet);

    if (pid != INVALID_PID)
    {
      /*
      ** TODO: just one show pid command should be enough
      */

      if (IS_ACTIVE(command, SHOW_ALL_PID))
      {
        print_output("%s: packet #%ld pid %d\n",
                     __FUNCTION__,
                     nb_packets, pid);
      }

      if (IS_ACTIVE(command, SHOW_PID))
      {
        uint16_t extended_table_id;

        section = get_section(packet, TRUE, FALSE);
        if (NULL == section)
        {
          extended_table_id = 0xFFFF;
        }
        else
        {
          extended_table_id = section[0];
        }

        if (! is_pid_in_list(pid, extended_table_id, pid_info_list, pid_info_list_size))
        {
          if (pid_info_list_size < sizeof (pid_info_list) / sizeof (pid_info_list[0]))
          {
            pid_info_list[pid_info_list_size].pid = pid;
            pid_info_list[pid_info_list_size].tid = extended_table_id;
            pid_info_list_size++;
          }
          else
          {
            print_error("%s: pid array of %d elements is too small\n",
                        __FUNCTION__,
                        sizeof (pid_info_list) / sizeof (pid_info_list[0]));
          }
        }
      }

      if (pid == pcr_pid)
      {
        packet_time.count++;
        if (read_pcr(packet, &packet_time.pcr))
        {
          if (IS_ACTIVE(command, SHOW_BR))
          {
            show_bitrate(pid, &packet_time);
          }
          if (IS_ACTIVE(command, HLS))
          {
            proceed_hls(filename, HLS_NEXT, &packet_time);
          }
        }
      }
    }

    if (pid == INVALID_PID)
    {
      print_debug("%s: packet not identified or stuffing\n",
                  __FUNCTION__);
    }
    else if (pid == PAT_PID)
    {
      section = get_section(packet, FALSE, FALSE);
      if (NULL != section)
      {
        payload = get_pat_payload(section, &pat_header);
        if (NULL != payload)
        {
          section_length = pat_header.section_length;
          target_prog->tsid = pat_header.transport_stream_id;

          if (IS_ACTIVE(command, SHOW_ALL_PAT) ||
              (IS_ACTIVE(command, SHOW_PAT) &&
              pat_header.version_number != pat_version_number))
          {
            show_pat(&pat_header, payload);
            pat_version_number = pat_header.version_number;
          }

          uint16_t temp_pid = target_prog->pmt_pid;

          if (INVALID_PROGRAM_NUMBER == target_prog->number)
          {
            get_first_program(&pat_header, payload, &first_prog);

            target_prog->number = first_prog.number;
            target_prog->pmt_pid = first_prog.pmt_pid;
          }
          else
          {
            target_prog->pmt_pid = get_pmt_pid(&pat_header,
                                               payload,
                                               target_prog->number);
          }

          if (IS_ACTIVE(command, REPLACE_TRANSPORT_STREAM_ID))
          {
            patch_section |= patch_pat_header(&pat_header,
                                              target_prog,
                                              new_prog,
                                              section);
          }

          if (INVALID_PID != target_prog->pmt_pid)
          {
            if (INVALID_PID == temp_pid)
            {
              /*
              ** Just found the PMT PID: may want to patch previous PMTs ?
              */
              rewind_file = TRUE;
            }

            print_debug("%s: got pmt pid %d\n",
                        __FUNCTION__,
                        target_prog->pmt_pid);

            if (IS_ACTIVE(command, REPLACE_PROGRAM_NUMBER) ||
                IS_ACTIVE(command, REPLACE_PMT_PID))
            {
              patch_section |= patch_pat_program(&pat_header,
                                                 target_prog,
                                                 new_prog,
                                                 (uint8_t*)payload);
            }
          }
        }
      }
    }
    else if (pid == target_prog->pmt_pid)
    {
      section = get_section(packet, FALSE, FALSE);
      if (NULL != section)
      {
        payload = get_pmt_payload(section, &pmt_header);
        if (NULL != payload)
        {
          section_length = pmt_header.section_length;

          if (IS_ACTIVE(command, SHOW_ALL_PMT) ||
              (IS_ACTIVE(command, SHOW_PMT) &&
              pmt_header.program_number == target_prog->number &&
              pmt_header.version_number != pmt_version_number))
          {
            show_pmt(&pmt_header, payload);
            pmt_version_number = pmt_header.version_number;
          }

          if (pmt_header.program_number == target_prog->number)
          {
            target_prog->pcr_pid = pmt_header.pcr_pid;
            if (INVALID_PID == pcr_pid)
            {
              pcr_pid = target_prog->pcr_pid;
            }

            if (INVALID_PID == target_prog->audio_pid)
            {
              target_prog->audio_pid = get_first_pid(&pmt_header,
                                                     payload,
                                                     AUDIO_TYPE);
            }

            if (INVALID_PID == target_prog->video_pid)
            {
              target_prog->video_pid = get_first_pid(&pmt_header,
                                                     payload,
                                                     VIDEO_TYPE);
            }

            if (IS_ACTIVE(command, REPLACE_PROGRAM_NUMBER))
            {
              patch_section |= patch_pmt_header(&pmt_header,
                                                target_prog,
                                                new_prog,
                                                (uint8_t*)section);
            }

            if (IS_ACTIVE(command, REPLACE_PCR_PID) ||
                IS_ACTIVE(command, REPLACE_AUDIO_PID) ||
                IS_ACTIVE(command, REPLACE_VIDEO_PID))
            {
              patch_section |= patch_pmt_es(&pmt_header,
                                            target_prog,
                                            new_prog,
                                            (uint8_t*)payload);
            }
          }
        }
      }

      if (IS_ACTIVE(command, REPLACE_PMT_PID))
      {
        patch_es |= set_pid(packet, new_prog->pmt_pid);
      }
    }
    else if (pid == SDT_PID)
    {
      section = get_section(packet, FALSE, FALSE);
      if (NULL != section)
      {
        payload = get_sdt_payload(section, &sdt_header);
        if (NULL != payload)
        {
          section_length = sdt_header.section_length;

          if (IS_ACTIVE(command, SHOW_ALL_SDT) ||
              (IS_ACTIVE(command, SHOW_SDT) &&
              sdt_header.version_number != sdt_version_number))
          {
            show_sdt(&sdt_header, payload);
            sdt_version_number = sdt_header.version_number;
          }

          if (IS_ACTIVE(command, REPLACE_PROVIDER_NAME))
          {
            patch_section |= patch_sdt_provider_name(&sdt_header,
                                                     service,
                                                     section,
                                                     &section_length);
          }
        }
      }
    }
    else if (pid == target_prog->pcr_pid &&
             IS_ACTIVE(command, REPLACE_PCR_PID))
    {
      patch_es |= set_pid(packet, new_prog->pcr_pid);
    }
    else if (pid == target_prog->audio_pid &&
             IS_ACTIVE(command, REPLACE_AUDIO_PID))
    {
      patch_es |= set_pid(packet, new_prog->audio_pid);
    }
    else if (pid == target_prog->video_pid &&
             IS_ACTIVE(command, REPLACE_VIDEO_PID))
    {
      patch_es |= set_pid(packet, new_prog->video_pid);
    }
    /*
    ** Hard-code AIT.
    */
    else if (pid == ait->pid &&
             (IS_ACTIVE(command, SHOW_AIT) ||
              IS_ACTIVE(command, REPLACE_AIT_ID) ||
              IS_ACTIVE(command, REPLACE_AIT_URL)))
    {
      section = get_section(packet, FALSE, FALSE);
      patch_section |= parse_ait(section, ait, &section_length);
    }
    if (INVALID_PID != pid1 && pid == pid1)
    {
      patch_es |= set_pid(packet, pid2);
    }
    if (pid == tsc_pid && IS_ACTIVE(command, TOGGLE_TSC))
    {
      patch_es |= toggle_tsc_bits(packet);
    }

    if (patch_section)
    {
      /*
      ** 1- change version
      ** 2- calculate CRC
      */

      increment_version_number(section);

      section_length += SECTION_HEADER_SIZE;
      section_length -= SECTION_CRC_SIZE;
      update_crc32(section, section_length);
    }

    if (patch_section || patch_es)
    {
      /*
      ** 3- rewind file for one packet
      ** 4- write packet into file
      */

      ret = fseek(file, -TS_PACKET_SIZE, SEEK_CUR);
      assert(0 == ret);

      nb_bytes = fwrite(packet, 1, TS_PACKET_SIZE, file);
      fflush(file);
      assert(TS_PACKET_SIZE == nb_bytes);
    }

    if (IS_ACTIVE(command, HLS))
    {
      proceed_hls(filename, HLS_STREAM, packet);
    }
  }

  if (IS_ACTIVE(command, HLS))
  {
    proceed_hls(filename, HLS_STOP, NULL);
  }

  if (IS_ACTIVE(command, SHOW_PID))
  {
    show_pid_list(pid_info_list, pid_info_list_size);
  }

  print_info("%s: end of file (%lu packets)\n",
             __FUNCTION__,
             nb_packets);
}

static void show_usage(const char *name)
{
  print_user("\n");
  print_user("usage: %s filename.ts [commands]\n", name);
  print_user("where:\n");
  print_user("  filename.ts is input transport stream file\n");
  print_user("  [commands]  is one or several out of\n");
  print_user("\n");
  print_user("    selection commands\n");  
  print_user("    -pn <pn>           select a program with program number\n");
  print_user("                       NOTE: some display and patching commands\n");
  print_user("                       require a program selection;\n");
  print_user("                       if none selected then the first program\n");
  print_user("                       found in PAT will be automatically selected\n");
  print_user("    -aud <pid>         select a audio PID\n");
  print_user("    -vid <pid>         select a video PID\n");
  print_user("\n");
  print_user("    display commands\n");  
  print_user("    -show-br           display instant and average bitrates\n");
  print_user("    -show-pat          display the PAT\n");
  print_user("    -show-pmt          display the PMT of selected program\n");
  print_user("    -show-pid          display all detected PIDs\n");
  print_user("    -show-all-pid      display all PIDs of all packets\n");
  print_user("    -show-all-pat      display all PATs (limited interest)\n");
  print_user("    -show-all-pmt      display all PMTs of selected program (limited interest)\n");
  print_user("    -show-sdt          display the SDT\n");
  print_user("    -show-ait <pid>    display the AIT identified by its PID\n");
  print_user("\n");
  print_user("    file patching commands\n");  
  print_user("    -rep-tsid <tsid>   replace transport stream id by new one\n");
  print_user("    -rep-pn <pn>       replace selected program number by new one\n");
  print_user("    -rep-pmt <pid>     replace selected program PMT PID by new one\n");
  print_user("    -rep-pcr <pid>     replace selected program PCR PID by new one\n");
  print_user("    -rep-aud <pid>     replace selected audio PID or the first audio PID\n");
  print_user("                       found in selected program PMT by new one\n");
  print_user("    -rep-vid <pid>     replace selected video PID or the first video PID\n");
  print_user("                       found in selected program PMT by new one\n");
  print_user("    -rep-pid <p1> <p2> replace first PID by second one\n");
  print_user("    -rep-provider <provider name>\n");
  print_user("\n");
  print_user("    special AIT\n");  
  print_user("    -rep-ait-id <app_id>\n");
  print_user("    -rep-ait-url <url base> <url entry file name>\n");
  print_user("    -toggle-tsc <pid> inverse the TSC bits of packets of selected PID\n");
  print_user("\n");
  print_user("    special HLS\n");  
  print_user("    -hls <duration>    split the input TS file into chunks of duration specified in seconds\n");
  print_user("                       and create corresponing m3u manifest\n");
}

static uint32_t parse_args(int argc, char **argv,
                           prog_t    *sel_prog,
                           prog_t    *rep_prog,
                           service_t *service,
                           ait_t     *rep_ait,
                           uint16_t  *pid1,
                           uint16_t  *pid2,
                           uint16_t  *tsc_pid)
{
  char     *command;
  char     *value1;
  char     *value2;
  int       i;
  uint32_t  user_command = 0;

  unsigned  chunk_duration; /* seconds */

  for (i = 0; i < argc; i++)
  {
    command = *argv++;

    /*
    ** Commands with no argument
    */

    if (IS_COMMAND(command, SHOW_ALL_PID))
    {
      user_command |= COMMAND_MASK_SHOW_ALL_PID;
    }
    else if (IS_COMMAND(command, SHOW_PAT))
    {
      user_command |= COMMAND_MASK_SHOW_PAT;
    }
    else if (IS_COMMAND(command, SHOW_ALL_PAT))
    {
      user_command |= COMMAND_MASK_SHOW_ALL_PAT;
    }
    else if (IS_COMMAND(command, SHOW_PMT))
    {
      user_command |= COMMAND_MASK_SHOW_PMT;
    }
    else if (IS_COMMAND(command, SHOW_ALL_PMT))
    {
      user_command |= COMMAND_MASK_SHOW_ALL_PMT;
    }
    else if (IS_COMMAND(command, SHOW_SDT))
    {
      user_command |= COMMAND_MASK_SHOW_SDT;
    }
    else if (IS_COMMAND(command, SHOW_ALL_SDT))
    { 
      user_command |= COMMAND_MASK_SHOW_ALL_SDT;
    }
    else if (IS_COMMAND(command, SHOW_PID))
    { 
      user_command |= COMMAND_MASK_SHOW_PID;
    }
    else if (IS_COMMAND(command, SHOW_BR))
    { 
      user_command |= COMMAND_MASK_SHOW_BR;
    }
    else
    {
      /*
      ** Commands with 1 argument
      */

      value1 = *argv++;
      i++;

      if (IS_COMMAND(command, SELECT_PROGRAM_NUMBER))
      {
        sel_prog->number = atoi(value1);
      }
      else if (IS_COMMAND(command, SELECT_AUDIO_PID))
      {
        sel_prog->audio_pid = atoi(value1);
      }
      else if (IS_COMMAND(command, SELECT_VIDEO_PID))
      {
        sel_prog->video_pid = atoi(value1);
      }
      else if (IS_COMMAND(command, REPLACE_TRANSPORT_STREAM_ID))
      {
        rep_prog->tsid = atoi(value1);
        user_command |= COMMAND_MASK_REPLACE_TRANSPORT_STREAM_ID;
      }
      else if (IS_COMMAND(command, REPLACE_PROGRAM_NUMBER))
      {
        rep_prog->number = atoi(value1);
        user_command |= COMMAND_MASK_REPLACE_PROGRAM_NUMBER;
      }
      else if (IS_COMMAND(command, REPLACE_PMT_PID))
      {
        rep_prog->pmt_pid = atoi(value1);
        user_command |= COMMAND_MASK_REPLACE_PMT_PID;
      }
      else if (IS_COMMAND(command, REPLACE_PCR_PID))
      {
        rep_prog->pcr_pid = atoi(value1);
        user_command |= COMMAND_MASK_REPLACE_PCR_PID;
      }
      else if (IS_COMMAND(command, REPLACE_AUDIO_PID))
      {
        rep_prog->audio_pid = atoi(value1);
        user_command |= COMMAND_MASK_REPLACE_AUDIO_PID;
      }
      else if (IS_COMMAND(command, REPLACE_VIDEO_PID))
      {
        rep_prog->video_pid = atoi(value1);
        user_command |= COMMAND_MASK_REPLACE_VIDEO_PID;
      }
      else if (IS_COMMAND(command, REPLACE_AIT_ID))
      {
        rep_ait->id = atoi(value1);
        user_command |= COMMAND_MASK_REPLACE_AIT_ID;
      }
      else if (IS_COMMAND(command, REPLACE_PROVIDER_NAME))
      {
        strncpy(service->name, value1, sizeof (service->name) - 1);
        user_command |= COMMAND_MASK_REPLACE_PROVIDER_NAME;
      }
      else if (IS_COMMAND(command, SHOW_AIT))
      {
        rep_ait->pid = atoi(value1);
        user_command |= COMMAND_MASK_SHOW_AIT;
      }
      else if (IS_COMMAND(command, TOGGLE_TSC))
      {
        *tsc_pid = atoi(value1);
        user_command |= COMMAND_MASK_TOGGLE_TSC;
      }
      else if (IS_COMMAND(command, HLS))
      {
        chunk_duration = atoi(value1);
        user_command |= COMMAND_MASK_HLS;
      }
      else
      {
        /*
        ** Commands with 2 arguments
        */

        value2 = *argv++;
        i++;

        if (IS_COMMAND(command, REPLACE_PID))
        {
          *pid1 = atoi(value1);
          *pid2 = atoi(value2);
          user_command |= COMMAND_MASK_REPLACE_PID;
        }
        else if (IS_COMMAND(command, REPLACE_AIT_URL))
        {
          rep_ait->url.base = value1;
          rep_ait->url.file = value2;
          user_command |= COMMAND_MASK_REPLACE_AIT_URL;
        }
        else
        {
          /* User command error */
        }
      }
    }
  }

  if (INVALID_TRANSPORT_STREAM_ID == rep_prog->tsid)
  {
    print_user("  - no transport stream id replacing\n");
  }
  else
  {
    print_user("  - transport stream id of PAT replaced by %d (0x%X)\n",
               rep_prog->tsid, rep_prog->tsid);
  }

  if (INVALID_PROGRAM_NUMBER == sel_prog->number)
  {
    print_user("  - no program selected\n");
  }
  else
  {
    print_user("  - program with program number %d (0x%X) selected\n",
               sel_prog->number, sel_prog->number);
  }

  if (INVALID_PROGRAM_NUMBER == rep_prog->number)
  {
    print_user("  - no program number replacing\n");
  }
  else if (INVALID_PROGRAM_NUMBER == sel_prog->number)
  {
    print_user("  - first program number found in PAT replaced by %d (0x%X)\n",
               rep_prog->number, rep_prog->number);
  }
  else
  {
    print_user("  - program number replaced by %d (0x%X)\n",
               rep_prog->number, rep_prog->number);
  }

  if (INVALID_PID == rep_prog->pmt_pid)
  {
    print_user("  - no PMT PID replacing\n");
  }
  else if (INVALID_PROGRAM_NUMBER == sel_prog->number)
  {
    print_user("  - PMT PID of first program replaced by %d (0x%X)\n",
               rep_prog->pmt_pid, rep_prog->pmt_pid);
  }
  else
  {
    print_user("  - PMT PID replaced by %d (0x%X)\n",
               rep_prog->pmt_pid, rep_prog->pmt_pid);
  }

  if (INVALID_PID == rep_prog->pcr_pid)
  {
    print_user("  - no PCR PID replacing\n");
  }
  else if (INVALID_PROGRAM_NUMBER == sel_prog->number)
  {
    print_user("  - PCR PID of first program replaced by %d (0x%X)\n",
               rep_prog->pcr_pid, rep_prog->pcr_pid);
  }
  else
  {
    print_user("  - PCR PID replaced by %d (0x%X)\n",
               rep_prog->pcr_pid, rep_prog->pcr_pid);
  }

  if (INVALID_PID == rep_prog->audio_pid)
  {
    print_user("  - no audio PID replacing\n");
  }
  else if (INVALID_PID != sel_prog->audio_pid)
  {
    print_user("  - audio PID %d (0x%X) replaced by %d (0x%X)\n",
               sel_prog->audio_pid, sel_prog->audio_pid,
               rep_prog->audio_pid, rep_prog->audio_pid);
  }
  else if (INVALID_PROGRAM_NUMBER == sel_prog->number)
  {
    print_user("  - first audio PID of first program replaced by %d (0x%X)\n",
               rep_prog->audio_pid, rep_prog->audio_pid);
  }
  else
  {
    print_user("  - first audio PID of selected program replaced by %d (0x%X)\n",
               rep_prog->audio_pid, rep_prog->audio_pid);
  }

  if (INVALID_PID == rep_prog->video_pid)
  {
    print_user("  - no video PID replacing\n");
  }
  else if (INVALID_PID != sel_prog->video_pid)
  {
    print_user("  - video PID %d (0x%X) replaced by %d (0x%X)\n",
               sel_prog->video_pid, sel_prog->video_pid,
               rep_prog->video_pid, rep_prog->video_pid);
  }
  else if (INVALID_PROGRAM_NUMBER == sel_prog->number)
  {
    print_user("  - first video PID of first program replaced by %d (0x%X)\n",
               rep_prog->video_pid, rep_prog->video_pid);
  }
  else
  {
    print_user("  - first video PID of selected program replaced by %d (0x%X)\n",
               rep_prog->video_pid, rep_prog->video_pid);
  }

  if (0 != strlen(service->name) && INVALID_PROGRAM_NUMBER != sel_prog->number)
  {
    print_user("  - set provider name of selected program to %s in SDT\n",
               service->name);
  }

  if (INVALID_PID != rep_ait->pid)
  {
    print_user("  - show AIT PID %d (0x%X)\n",
               rep_ait->pid, rep_ait->pid);
  }

  if (0 != (user_command & COMMAND_MASK_SHOW_PID))
  {
    print_user("  - show all detected PIDs only once\n");
  }

  if (0 != (user_command & COMMAND_MASK_REPLACE_PID))
  {
    if (*pid1 < INVALID_PID && *pid2 <= INVALID_PID)
    {
      /*
      ** Second PID accepts invalid value
      */
      print_user("  - replace PID %d (0x%X) by %d (0x%X)\n",
                 *pid1, *pid1,
                 *pid2, *pid2);
    }
    else
    {
      print_user("  - don't replace PID because of invalid value\n");
      *pid1 = INVALID_PID;
      *pid2 = INVALID_PID;
      user_command &= ~COMMAND_MASK_REPLACE_PID;
    }
  }

  if (0 != (user_command & COMMAND_MASK_REPLACE_AIT_ID))
  {
    if (0 != rep_ait->id)
    {
      print_user("  - replace AIT ID by %d\n",
                 rep_ait->id);
    }
    else
    {
      print_user("  - need to select an AIT PID to replace the ID, abort replacing command\n");
      init_ait(rep_ait);
      user_command &= ~COMMAND_MASK_REPLACE_AIT_ID;
    }
  }

  if (0 != (user_command & COMMAND_MASK_REPLACE_AIT_URL))
  {
    if (INVALID_PID != rep_ait->pid)
    {
      print_user("  - replace AIT URL by %s %s\n",
                 rep_ait->url.base, rep_ait->url.file);
    }
    else
    {
      print_user("  - need to select an AIT PID to replace the URL, abort replacing command\n");
      init_ait(rep_ait);
      user_command &= ~COMMAND_MASK_REPLACE_AIT_URL;
    }
  }

  if (INVALID_PID != *tsc_pid)
  {
    print_user("  - toggle PID %d (0x%X)\n",
               *tsc_pid, *tsc_pid);
  }

  if (0 != (user_command & COMMAND_MASK_HLS))
  {
    print_user("  - create HLS chunks of %ds\n",
               chunk_duration);
  }

  return user_command;
}

int main(int argc, char **argv)
{
  FILE     *file;
  char     *toolname;
  char     *filename;
  uint32_t  user_command;
  prog_t    selected_prog;
  prog_t    replacing_prog;
  service_t service;
  ait_t     ait;
  uint16_t  pid1;
  uint16_t  pid2;
  uint16_t  tsc_pid;

  init_prog(&selected_prog);
  init_prog(&replacing_prog);
  init_service(&service);
  init_ait(&ait);
  pid1    = INVALID_PID;
  pid2    = INVALID_PID;
  tsc_pid = INVALID_PID;

  /*
  ** TODO: process directory in addition of file
  */

//  toolname = basename(*argv++);
  toolname = *argv++;

  if (argc < 2)
  {
    print_user("wrong number of arguments\n");
    show_usage(toolname);
    return 0;
  }

  filename = *argv++;

  print_user("%s %s\n", toolname, filename);

  user_command = parse_args(argc - 2, argv,
                            &selected_prog,
                            &replacing_prog,
                            &service,
                            &ait,
                            &pid1,
                            &pid2,
                            &tsc_pid);
  service.id = selected_prog.number;

  init_crc();

  file = fopen(filename, "rb+");
  if (NULL == file)
  {
    print_user("cannot open %s\n", filename);
    return 0;
  }

  parse_ts(filename,
           file,
           user_command,
           &selected_prog,
           &replacing_prog,
           &service,
           &ait,
           pid1,
           pid2,
           tsc_pid);

  fclose(file);

  print_user("%s end\n", toolname);

  return 0;
}


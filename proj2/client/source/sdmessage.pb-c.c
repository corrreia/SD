/* Generated by the protocol buffer compiler.  DO NOT EDIT! */
/* Generated from: sdmessage.proto */

/* Do not generate deprecated warnings for self */
#ifndef PROTOBUF_C__NO_DEPRECATED
#define PROTOBUF_C__NO_DEPRECATED
#endif

#include "../include/sdmessage.pb-c.h"
void   entry__data__init
                     (Entry__Data         *message)
{
  static const Entry__Data init_value = ENTRY__DATA__INIT;
  *message = init_value;
}
void   entry__init
                     (Entry         *message)
{
  static const Entry init_value = ENTRY__INIT;
  *message = init_value;
}
size_t entry__get_packed_size
                     (const Entry *message)
{
  assert(message->base.descriptor == &entry__descriptor);
  return protobuf_c_message_get_packed_size ((const ProtobufCMessage*)(message));
}
size_t entry__pack
                     (const Entry *message,
                      uint8_t       *out)
{
  assert(message->base.descriptor == &entry__descriptor);
  return protobuf_c_message_pack ((const ProtobufCMessage*)message, out);
}
size_t entry__pack_to_buffer
                     (const Entry *message,
                      ProtobufCBuffer *buffer)
{
  assert(message->base.descriptor == &entry__descriptor);
  return protobuf_c_message_pack_to_buffer ((const ProtobufCMessage*)message, buffer);
}
Entry *
       entry__unpack
                     (ProtobufCAllocator  *allocator,
                      size_t               len,
                      const uint8_t       *data)
{
  return (Entry *)
     protobuf_c_message_unpack (&entry__descriptor,
                                allocator, len, data);
}
void   entry__free_unpacked
                     (Entry *message,
                      ProtobufCAllocator *allocator)
{
  if(!message)
    return;
  assert(message->base.descriptor == &entry__descriptor);
  protobuf_c_message_free_unpacked ((ProtobufCMessage*)message, allocator);
}
static const ProtobufCFieldDescriptor entry__data__field_descriptors[2] =
{
  {
    "datasize",
    1,
    PROTOBUF_C_LABEL_NONE,
    PROTOBUF_C_TYPE_INT32,
    0,   /* quantifier_offset */
    offsetof(Entry__Data, datasize),
    NULL,
    NULL,
    0,             /* flags */
    0,NULL,NULL    /* reserved1,reserved2, etc */
  },
  {
    "value",
    2,
    PROTOBUF_C_LABEL_NONE,
    PROTOBUF_C_TYPE_STRING,
    0,   /* quantifier_offset */
    offsetof(Entry__Data, value),
    NULL,
    &protobuf_c_empty_string,
    0,             /* flags */
    0,NULL,NULL    /* reserved1,reserved2, etc */
  },
};
static const unsigned entry__data__field_indices_by_name[] = {
  0,   /* field[0] = datasize */
  1,   /* field[1] = value */
};
static const ProtobufCIntRange entry__data__number_ranges[1 + 1] =
{
  { 1, 0 },
  { 0, 2 }
};
const ProtobufCMessageDescriptor entry__data__descriptor =
{
  PROTOBUF_C__MESSAGE_DESCRIPTOR_MAGIC,
  "Entry.Data",
  "Data",
  "Entry__Data",
  "",
  sizeof(Entry__Data),
  2,
  entry__data__field_descriptors,
  entry__data__field_indices_by_name,
  1,  entry__data__number_ranges,
  (ProtobufCMessageInit) entry__data__init,
  NULL,NULL,NULL    /* reserved[123] */
};
static const ProtobufCFieldDescriptor entry__field_descriptors[2] =
{
  {
    "key",
    1,
    PROTOBUF_C_LABEL_NONE,
    PROTOBUF_C_TYPE_STRING,
    0,   /* quantifier_offset */
    offsetof(Entry, key),
    NULL,
    &protobuf_c_empty_string,
    0,             /* flags */
    0,NULL,NULL    /* reserved1,reserved2, etc */
  },
  {
    "data",
    2,
    PROTOBUF_C_LABEL_NONE,
    PROTOBUF_C_TYPE_MESSAGE,
    0,   /* quantifier_offset */
    offsetof(Entry, data),
    &entry__data__descriptor,
    NULL,
    0,             /* flags */
    0,NULL,NULL    /* reserved1,reserved2, etc */
  },
};
static const unsigned entry__field_indices_by_name[] = {
  1,   /* field[1] = data */
  0,   /* field[0] = key */
};
static const ProtobufCIntRange entry__number_ranges[1 + 1] =
{
  { 1, 0 },
  { 0, 2 }
};
const ProtobufCMessageDescriptor entry__descriptor =
{
  PROTOBUF_C__MESSAGE_DESCRIPTOR_MAGIC,
  "Entry",
  "Entry",
  "Entry",
  "",
  sizeof(Entry),
  2,
  entry__field_descriptors,
  entry__field_indices_by_name,
  1,  entry__number_ranges,
  (ProtobufCMessageInit) entry__init,
  NULL,NULL,NULL    /* reserved[123] */
};

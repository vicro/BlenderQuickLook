//
//  data_structures.h
//  BlenderFileReader
//
//  Created by Victor Rosales on 2014/07/15.
//  Copyright (c) 2014年 darkoleptiko. All rights reserved.
//

#ifndef BlenderFileReader_data_structures_h
#define BlenderFileReader_data_structures_h

#include <stdint.h>
#include <stdbool.h>
#include <CoreFoundation/CFByteOrder.h>

static const char BLENDER_PTR32 = '_';
static const char BLENDER_PTR64 = '-';

typedef struct bql_fileHeader
{
    // File identifier (always 'BLENDER')
    char identifier[7];
    // Size of a pointer; all pointers in the file are stored in this format.
    // '_' means 4 bytes or 32 bit and '-' means 8 bytes or 64 bits.
    char pointerSize;
    // Type of byte ordering used; 'v' means little endian and 'V'
    // means big endian.
    char endianness;
    // Version of Blender the file was created in; '254' means version 2.54
    char versionNumber[3];
} bql_fileHeader;


typedef union
{
    int32_t ptr32_t;
    int64_t ptr64_t;
} bql_address;


typedef struct bql_block
{
    // File-block identifier
    char code[4];
    // Length in bytes of data after the block header
    int32_t dataSize;
    // Memory address of the sructure before writing to disk (32bit or 64bit)
    bql_address address;
    // Index of the SDNA structure
    int32_t sdnaIndex;
    // Number of structures in the block
    int32_t count;
    // Offset in file
    long offset;
} bql_block;


typedef struct bql_structure
{
    // The type of this structure in the types list
    int16_t type_idx;
    // Number of fields in this structure
    int16_t field_count;
    // The type of each field in the types list
    int16_t * field_types_idx;
    // The name of each field in the names list
    int16_t * field_names_idx;
    
} bql_structure;


typedef struct bql_file
{
    // File header
    bql_fileHeader header;
    // File endianness
    CFByteOrder endianness;
    // The size in bytes of the block header
    int blockheader_size;
    // Blocks
    int block_count;
    bql_block *blocks;
    // Names
    int name_count;
    char ** names;
    // Types
    int type_count;
    char ** types;
    int16_t * type_lengths;
    // Structures
    int structure_count;
    bql_structure * structures;
    
    // blocks start file offset
    long blocks_offset;
    
} bql_file;

//Swap byte order using CoreFoundation methods
int16_t swapBytes_i16(int16_t val, bql_file *blendfile);
int32_t swapBytes_i32(int32_t val, bql_file *blendfile);
int64_t swapBytes_i64(int64_t val, bql_file *blendfile);


bool bql_checkEndiannness(bql_file *blendfile);
bool bql_getBlockHeaderSize(bql_file *blendfile);



#endif

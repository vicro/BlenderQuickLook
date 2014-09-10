//
//  blendfile.c
//  BlenderFileReader
//
//  Created by rosales on 2014/07/16.
//  Copyright (c) 2014年 darkoleptiko. All rights reserved.
//

#include <stdio.h>
#include "blendfile.h"

//Swap int16 to system endianness
int16_t swapBytes_i16(int16_t val, bql_file *blendfile)
{
    if(blendfile->endianness == CFByteOrderLittleEndian)
        return CFSwapInt16LittleToHost(val);
    else if(blendfile->endianness == CFByteOrderBigEndian)
        return CFSwapInt16BigToHost(val);
    else return val;
}

//Swap int32 to system endianness
int32_t swapBytes_i32(int32_t val, bql_file *blendfile)
{
    if(blendfile->endianness == CFByteOrderLittleEndian)
        return CFSwapInt32LittleToHost(val);
    else if(blendfile->endianness == CFByteOrderBigEndian)
        return CFSwapInt32BigToHost(val);
    else return val;
}

//Swap int64 to system endianness
int64_t swapBytes_i64(int64_t val, bql_file *blendfile)
{
    if(blendfile->endianness == CFByteOrderLittleEndian)
        return CFSwapInt64LittleToHost(val);
    else if(blendfile->endianness == CFByteOrderBigEndian)
        return CFSwapInt64BigToHost(val);
    else
        return val;
}

bool bql_checkEndiannness(bql_file *blendfile)
{
    if(blendfile->header.endianness == 'v')
    {
        blendfile->endianness = CFByteOrderLittleEndian;
    }
    else if(blendfile->header.endianness == 'V')
    {
        blendfile->endianness = CFByteOrderBigEndian;
    }
    else
    {
        blendfile->endianness = CFByteOrderUnknown;
        printf("Unknown endianness found (%c)\n", blendfile->header.endianness);
        return false;
    }
    return true;
}


bool bql_getBlockHeaderSize(bql_file *blendfile)
{
    bql_block block;
    
    if(blendfile->header.pointerSize == BLENDER_PTR32)
    {
        blendfile->blockheader_size =
            sizeof(block.code)            + //char[4]  code
            sizeof(block.dataSize)        + //int32    data size
            sizeof(block.address.ptr32_t) + //32bitptr address
            sizeof(block.sdnaIndex)       + //int32    sdna index
            sizeof(block.count)             //int32    count
        ;
    }
    else if(blendfile->header.pointerSize == BLENDER_PTR64)
    {
        blendfile->blockheader_size =
            sizeof(block.code)            + //char[4]  code
            sizeof(block.dataSize)        + //int32    data size
            sizeof(block.address.ptr64_t) + //64bitptr address
            sizeof(block.sdnaIndex)       + //int32    sdna index
            sizeof(block.count)             //int32    count
        ;
    }
    else
    {
        printf("Unknown pointer size found (%c)", blendfile->header.pointerSize);
        return false;
    }
    return true;
}
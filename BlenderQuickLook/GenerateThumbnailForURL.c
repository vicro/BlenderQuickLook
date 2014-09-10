#include <CoreFoundation/CoreFoundation.h>
#include <CoreServices/CoreServices.h>
#include <QuickLook/QuickLook.h>

#include "blendfile.h"

OSStatus GenerateThumbnailForURL(void *thisInterface, QLThumbnailRequestRef thumbnail, CFURLRef url, CFStringRef contentTypeUTI, CFDictionaryRef options, CGSize maxSize);
void CancelThumbnailGeneration(void *thisInterface, QLThumbnailRequestRef thumbnail);

/* -----------------------------------------------------------------------------
    Generate a thumbnail for file

   This function's job is to create thumbnail for designated file as fast as possible
   ----------------------------------------------------------------------------- */

OSStatus GenerateThumbnailForURL(void *thisInterface, QLThumbnailRequestRef thumbnail, CFURLRef url, CFStringRef contentTypeUTI, CFDictionaryRef options, CGSize maxSize)
{
    //------------------
    //OPEN THE FILE
    CFStringRef posix_path = CFURLCopyFileSystemPath(url, kCFURLPOSIXPathStyle);
    const char * cstring = CFStringGetCStringPtr(posix_path, kCFStringEncodingUTF8);
    
    FILE *ptr_file;
    ptr_file = fopen(cstring, "rb");
    
    CFRelease(posix_path);
    
    if(!ptr_file)
    {
        //ERROR
        return 1;
    }
    
    //------------------
    //PARSE THE HEADER
    
    bql_file blendfile;
    size_t readcount;
    readcount = fread(&blendfile.header, sizeof(blendfile.header), 1, ptr_file);
    
    if(readcount != 1 || strncmp(blendfile.header.identifier, "BLENDER", 7) != 0)
    {
        //ERROR not a blender file
        fclose(ptr_file);
        return 1;
    }
    
    //------------------
    //PREPARE FILE ENDIANNESS AND POINTER SIZE
    bql_checkEndiannness(&blendfile);
    bql_getBlockHeaderSize(&blendfile);
    
    
    //------------------
    //READ EACH BLOCK, LOOK FOR A "TEST" BLOCK THAT HAS THE THUMBNAIL DATA
    blendfile.blocks_offset = ftell(ptr_file);
    
    long currentOffset = blendfile.blocks_offset;
    bool isEof = false;
    bql_block block;
    
    while(!isEof)
    {
        //read the block code identifier 4 chars
        fread(&block.code, sizeof(block.code), 1, ptr_file);
        //read the block data size in bytes
        fread(&block.dataSize, sizeof(block.dataSize), 1, ptr_file);
        block.dataSize = swapBytes_i32(block.dataSize, &blendfile);
        
        if(strncmp(block.code, "TEST", 4) == 0)
        {
            //Found the Thumbnail data block
            //Move to the start of the data block
            fseek(ptr_file, currentOffset + blendfile.blockheader_size, SEEK_SET);
            
            int32_t width,height;
            
            //Read the width and height of the thumbnail
            fread(&width, sizeof(width), 1, ptr_file);
            width = swapBytes_i32(width, &blendfile);
            fread(&height, sizeof(height), 1, ptr_file);
            height = swapBytes_i32(height, &blendfile);
            
            size_t datasize = width * height * 4;
            if(datasize != block.dataSize - sizeof(width)*2)
            {
                //Error: data size mismatch
                break;
            }
            //Get RGBA data
            char * data = malloc(datasize);
            fread(data, sizeof(char), datasize, ptr_file);
            
            //Blender stores the rows upside down so we flip it
            char * rgba_data = malloc(datasize);
            int rowSize = width * 4;
            for (int y = 0; y < height; y++)
            {
                //row start
                int source_row = y * rowSize;
                int target_row = (height - y - 1) * rowSize;
                memcpy(&rgba_data[target_row], &data[source_row], rowSize);
            }
            free(data);
            
            //Now convert the data to a CGImageRef
            CGColorSpaceRef cf_colorspace = CGColorSpaceCreateDeviceRGB();
            CGContextRef bitmapContext = CGBitmapContextCreate(
               rgba_data,     //data
               width,         //width
               height,        //height
               8,             //bitsPerComponent
               width * 4,     //bytesPerRow
               cf_colorspace, //colorspace
               kCGImageAlphaNoneSkipLast //bitmapInfo
               );
            CGColorSpaceRelease(cf_colorspace);
            CGImageRef cgImage = CGBitmapContextCreateImage(bitmapContext);

            //SET THE THUMBNAIL IMAGE
            QLThumbnailRequestSetImage(thumbnail, cgImage, NULL);
            
            //memory cleanup
            CGImageRelease(cgImage);
            CGContextRelease(bitmapContext);
            free(rgba_data);
            
            break;
        }
        else if(strncmp(block.code, "ENDB", 4) == 0)
        {
            //Reached end of file
            isEof = true;
        }
        else
        {
            currentOffset += blendfile.blockheader_size + block.dataSize;
            //Move to the next block
            fseek(ptr_file, currentOffset, SEEK_SET);
        }
    }
    fclose(ptr_file);
    
    return noErr;
}

void CancelThumbnailGeneration(void *thisInterface, QLThumbnailRequestRef thumbnail)
{
    // Implement only if supported
}

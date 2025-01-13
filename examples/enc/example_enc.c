/*

    -- example_enc.c -- Reference QOI encoding usage of this library

    -- version 1.0 -- revised 2025-13-01

    MIT License

    Copyright (c) 2024-2025 Aftersol

    Permission is hereby granted, free of charge, to any person obtaining a copy
    of this software and associated documentation files (the "Software"), to deal
    in the Software without restriction, including without limitation the rights
    to use, copy, modify, merge, publish, distribute, sublicense, and/or sell
    copies of the Software, and to permit persons to whom the Software is
    furnished to do so, subject to the following conditions:

    The above copyright notice and this permission notice shall be included in all
    copies or substantial portions of the Software.

    THE SOFTWARE IS PROVIDED "AS IS", WITHOUT WARRANTY OF ANY KIND, EXPRESS OR
    IMPLIED, INCLUDING BUT NOT LIMITED TO THE WARRANTIES OF MERCHANTABILITY,
    FITNESS FOR A PARTICULAR PURPOSE AND NONINFRINGEMENT. IN NO EVENT SHALL THE
    AUTHORS OR COPYRIGHT HOLDERS BE LIABLE FOR ANY CLAIM, DAMAGES OR OTHER
    LIABILITY, WHETHER IN AN ACTION OF CONTRACT, TORT OR OTHERWISE, ARISING FROM,
    OUT OF OR IN CONNECTION WITH THE SOFTWARE OR THE USE OR OTHER DEALINGS IN THE
    SOFTWARE.


*/

#include <stdio.h>
#include <stdlib.h>
#include <string.h>

#define SIMPLIFIED_QOI_IMPLEMENTATION
#include "sQOI.h"

void print_help()
{
    printf("Example usage: qoi_enc <filename> <width> <height> <channels> <colorspace> <output>\n");
    printf("Channels:\n3: No transparency\n4: Transparency\n\n");
    printf("Colorspace:\n0: sRGB with linear alpha\n1: Linear RGB\n");
}

int main(int argc, char* argv[])
{

    qoi_desc_t desc;
    qoi_enc_t enc;
    uint8_t* qoi_file, *pixel_seek, *file_buffer;
    FILE* fp;
    uint32_t width, height;
    uint8_t channels, colorspace;
    size_t file_size;
    
    if (argc < 6)
    {
        print_help();
        return -1;
    }
    else
    {

        if (strlen(argv[1]) <= 0)
        {
            print_help();
            return -1;
        }

        width = strtoul(argv[2], NULL, 0);
        height = strtoul(argv[3], NULL, 0);

        channels = strtoul(argv[4], NULL, 0);
        colorspace = strtoul(argv[5], NULL, 0);

        if (width < 0 || height < 0)
        {
            print_help();
            return -1;
        }

        if (channels < 0)
        {
            print_help();
            return -1;
        }
        
        if (channels < 3) channels = 3;

        if (channels > 4) channels = 4;

        if (colorspace < 0 || colorspace > 1)
        {
            print_help();
            return -1;
        }
    }

    printf("Opening %s\n",argv[1]);

    fp = fopen(argv[1], "rb");

    if (!fp)
    {
        print_help();
        return -1;
    }

    fseek(fp, 0, SEEK_END);
    
    file_size = ftell(fp);

    if ((size_t)width * (size_t)height * (size_t)channels < file_size)
    {
        size_t image_size = (size_t)width * (size_t)height * (size_t)channels;
        size_t size_difference = file_size - image_size;

        printf(
            "%zu bytes are required for the file, %s. Your requested dimensions and channels amount allocates %zu bytes. That is %zu %s difference\n",
            file_size,
            argv[1],
            image_size,
            size_difference,
            /* for printing plurals from of the word "byte" */
            (size_difference > 1) ? "bytes" : "byte"
            );
        print_help();

        return -1;
    }

    fseek(fp, 0, SEEK_SET);

    file_buffer = (uint8_t*)calloc(file_size + 1, sizeof(uint8_t));
    if (!file_buffer)
    {
        fclose(fp);
        return 1;
    }

    fread(file_buffer, 1, file_size, fp);
    fclose(fp);

    qoi_desc_init(&desc);
    
    qoi_set_dimensions(&desc, width, height);
    qoi_set_channels(&desc, channels);
    qoi_set_colorspace(&desc, colorspace);
    
    qoi_file = (uint8_t*)malloc(((size_t)desc.width * (size_t)desc.height * ((size_t)desc.channels + 1)) + 14 + 8 + sizeof(size_t));
    
    if (!qoi_file)
    {
        return 1;
    }

    printf("Writing %s\n",argv[6]);

    write_qoi_header(&desc, qoi_file);

    pixel_seek = file_buffer;

    qoi_enc_init(&desc, &enc, qoi_file);

    while(!qoi_enc_done(&enc))
    {
        qoi_encode_chunk(&desc, &enc, pixel_seek);

        pixel_seek += desc.channels;
    }

    fp = fopen(argv[6], "wb");
    
    if (fp)
    {
        fwrite(qoi_file, 1, enc.offset - enc.data, fp);
        fclose(fp);
    } 

    free(qoi_file);

    return 0;
}
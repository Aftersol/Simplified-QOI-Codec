/*

    -- example_enc.c -- Reference QOI encoding usage of this library

    -- version 1.1.2 -- revised 2026-04-16

    -- Changelog --
    
    - version 1.1.2 (2026-04-16)
        - Fixed underflow if thesize of the raw image is smaller
        than requested image size and number of image channels
        - Fixed edge case of plural spelling of bytes
        
    - version 1.1.1 (2026-04-13)
        - Modified program to display version info

    - version 1.1 (2025-04-07)
        - Implemented error handling in case reading RGBA file fails
        - Strictly check requested color channels and colorspace
        according to QOI specifications

    - version 1.0 (2025-01-13)

    MIT License

    Copyright (c) 2024-2026 Aftersol

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

#define QOI_ENC_BUFFER_SIZE 131072 /* Buffer size set to 128 KiB */

#include "sQOI.h"

const char version_number[] = "version 1.1.2";
const char revised_date[] = "2026-04-16";

void print_version()
{
    printf("QOI Encoder\nversion: %s -- revised %s\n", version_number, revised_date);
}

void print_help()
{
    printf("Example usage: qoi_enc <filename> <width> <height> <channels> <colorspace> <output>\n");
    printf("Channels:\n3: No transparency\n4: Transparency\n\n");
    printf("Colorspace:\n0: sRGB with linear alpha\n1: Linear RGB\n");
}

uint8_t qoi_enc_buffer[QOI_ENC_BUFFER_SIZE];

int main(int argc, char* argv[])
{

    qoi_desc_t desc;
    qoi_enc_t enc;
    uint8_t* qoi_file, *pixel_seek, *file_buffer;
    FILE* fp;
    uint32_t width, height;
    uint8_t channels, colorspace;
    size_t file_size;
    
    print_version();

    if (argc < 6)
    {
        print_help();
        return -1;
    }
    else
    {
        /* Read arguments */
        if (strlen(argv[1]) <= 0)
        {
            print_help();
            return -1;
        }

        width = strtoul(argv[2], NULL, 0);
        height = strtoul(argv[3], NULL, 0);

        /* Check if requested amount or color channels is 3 or 4 according to QOI specifications */
        if (strtoul(argv[4], NULL, 0) >= 3 && strtoul(argv[4], NULL, 0) <= 4) {
            channels = strtoul(argv[4], NULL, 0);
        }
        else
        {
            printf("Channels entered must be 3 (RGB) or 4 (RGBA)\n");
            print_help();
            return -1;
        }

        /* Check if requested colorspace is according to QOI specifications:
           0 (sRGB with linear alpha)
           1 (linear RGB)  
        */
        if (strtoul(argv[5], NULL, 0) >= 0 && strtoul(argv[5], NULL, 0) <= 1) {
            colorspace = strtoul(argv[5], NULL, 0);
        }
        else
        {
            printf("Colorspace entered must be 0 (sRGB with linear alpha) or 1 (linear RGB)\n");
            print_help();
            return -1;
        }

        if (width < 0 || height < 0)
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
        size_t size_difference = image_size - file_size;

        printf(
            "%zu %s are required for the file, %s. Your requested image dimensions and number of color channels amount allocates %zu %s. That is %zu %s difference\n",
            file_size,
            /* for printing plurals from of the word "byte" */
            (file_size > 1) ? "bytes" : "byte",
            argv[1],
            image_size,
            /* for printing plurals from of the word "byte" */
            (image_size > 1) ? "bytes" : "byte",
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

    if (fread(file_buffer, 1, file_size, fp) < file_size) 
    {
        if (ferror(fp)) 
        {
            printf("An error has occur while reading %s\n", argv[1]);
            print_help();
    
            fclose(fp);
            free(file_buffer);
    
            return 1;
        }
    }

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

    printf("Encoding %s to %s. Please wait . . .\n", argv[1], argv[6]);

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

/*

    -- example_dec.c -- Reference QOI decoding usage of this library

    -- version 1.0 -- revised 2023-27-06

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
    printf("Example usage: qoi_dec <qoi file> <raw output file>\n");
}

int main(int argc, char* argv[])
{
    /* QOI variables */
    qoi_desc_t desc;
    qoi_dec_t dec;
    qoi_pixel_t px;

    unsigned char* qoi_bytes, *bytes;
    size_t raw_image_length, seek, buffer_size;

    FILE* fp;

    if (argc < 3)
    {
        print_help();
        return -1;
    }
    else
    {
        if (strlen(argv[1]) <= 0 || strlen(argv[2]) <= 0)
        {
            print_help();
            return -1;
        }
    }

    printf("Opening %s\n", argv[1]);

    fp = fopen(argv[1], "rb");
    if (!fp)
    {
        printf("Cannot open %s\n", argv[1]);

        print_help();
        return -1;
    }

    fseek(fp, 0, SEEK_END);
    buffer_size = ftell(fp);
    fseek(fp, 0, SEEK_SET);

    printf("Reading %s\n", argv[1]);

    qoi_bytes = (unsigned char*)calloc(buffer_size + 4, 1);

    if (!qoi_bytes)
    {
        fclose(fp);
        return 3;
    }

    fread(qoi_bytes, 1, buffer_size, fp);

    fclose(fp);

    /* Set up QOI decoding process */
    qoi_desc_init(&desc);

    if (!read_qoi_header(&desc, qoi_bytes))
    {
        printf("The file you opened is not a QOIF file\n");
        print_help();
        fclose(fp);
        return 1;
    }

    printf("QOI Info\n");
    printf("Image dimensions %ux%u\n", desc.width, desc.height);
    printf("Number of channels: %u\n", desc.channels);
    printf("Colorspace: %u\n", desc.colorspace);

    raw_image_length = (size_t)desc.width * (size_t)desc.height * (size_t)desc.channels;
    seek = 0;

    if (raw_image_length == 0)
    {
        fclose(fp);
        return 2;
    }
    
    qoi_dec_init(&desc, &dec, qoi_bytes, buffer_size);

    /* Creates a blank image for the decoder to work on */
    bytes = (unsigned char*)malloc(raw_image_length * sizeof(unsigned char) + 4);
    if (!bytes)
    {
        return 3;
    }

    printf("Decoding %s into %s. Please wait . . .\n", argv[1], argv[2]);

    /*  Keep decoding the pixels until
        all pixels are done decompressing */
    while (!qoi_dec_done(&dec))
    {
        px = qoi_decode_chunk(&dec);

        /*  Do something with the pixel values below */

        bytes[seek] = px.red;
        bytes[seek + 1] = px.green;
        bytes[seek + 2] = px.blue;

        if (desc.channels > 3) bytes[seek + 3] = px.alpha;

        seek += desc.channels;

    }

    free(qoi_bytes);

    fp = fopen(argv[2], "wb");

    if (!fp)
    {
        free(bytes);
        return 4;
    }

    fwrite(bytes, 1, raw_image_length, fp);

    fclose(fp);

    free(bytes);

    return 0;
}
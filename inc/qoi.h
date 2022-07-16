/*

    ,';::::::::::::::::::::::::::::::::::::::::::::::::::::::;',
    oWMMMMMMMMMMMMMMMMMMMMMMMMMMMMMMMMMMMMMMMMMMMMMMMMMMMMMMWo 
    dMMMMMMMMMMMMMMMMMMMMMMMMMMMMMMMMMMMMMMMMMMMMMMMMMMMMMMMMd 
    dMMMMMMMMMMWX0OkkOKNMMMMMMMMMMMWX0OkO0XWMMMMMMMNXNMMMMMMMd 
    dMMMMMMMWKd:;;;,'..'ckNMMMMWKkd;.......,lONMMMK:.cKMMMMMMd 
    dMMMMMMNx;:xKNNNX0d,  ,OWMNd. .:d0KK0xc. .:0WMk. .OMMMMMMd 
    dMMMMMWx,dNMMMMMMMMNl. .kNo..ckWMMMMMMW0;  'OWk. .kMMMMMMd 
    dMMMMMN:'OMMMMMWKk0W0'  :x' lXNMMMMMMMMMO.  :Xx. .kMMMMMMd 
    dMMMMMWl.;KMMMWx. :Kx.  ;x' :0XMMMMMMMMMO.  ;Kx. .xMMMMMMd 
    dMMMMMMXxx00OOk;  .'.  .dXc .'lKWMMMMMW0;   lNd. .xMMMMMMd 
    dMMMMMMMWx'.          .oNMKc   .cdkOkd:.   :KWd  .dMMMMMMd 
    dMMMMMMMWx.          .oXMMMNxc;.        .,dXMWd. .xMMMMMMd 
    .dWMMMMMMMWKdc;,,,::'  .oNMMMMMN0dl:;;:cdONMMMMXxlxXMMMMMM
    .dWMMMMMMMMMMMWWWWMWXOkOXWMMMMMMMMMMWWMMMMMMMMMMMMMMMMMMMM 
    dWMMMMMMMMMMMMMMMMMMMMMMMMMMMMMMMMMMMMMMMMMMMMMMMMMMMMMMMM 
    .;kOOOOOOOOOOOOOOOOOOOOOOOOOOOOOOOOOOOOOOOOOOOOOOOOOOOOOOk;.
    c. ...................................................... .c

    THE QUITE OK IMAGE FORMAT
    Implementation based on the Version 1.0 created by Dominic Szablewski
    Published on 2022.01.05 – qoiformat.org - https://qoiformat.org/qoi-specification.pdf

    Reference QOI implementation - https://github.com/phoboslab/qoi

    A QOI file consists of a 14-byte header, followed by any number of
    data “chunks” and an 8-byte end marker.

    qoi_header {
        char magic[4]; // magic bytes "qoif"
        uint32_t width; // image width in pixels (BE)
        uint32_t height; // image height in pixels (BE)
        uint8_t channels; // 3 = RGB, 4 = RGBA
        uint8_t colorspace; // 0 = sRGB with linear alpha
        // 1 = all channels linear
    };

*/

/*

    Define QOI_IMPLEMENTATION in only "one" of your source code files (.c, .cpp .cxx), not your header files
    before you include this library to create this implementation

    #define QOI_IMPLEMENTATION
    #include "qoi.h"

*/

#ifndef QOI_H_IMPLEMENTATION
#define QOI_H_IMPLEMENTATION

#ifdef QOI_IMPLEMENTATION

#ifdef __cplusplus
extern "C" {
#endif

#include <stdint.h>
#include <stddef.h>
#include <stdbool.h>

/* QOI OPCODES */

#define QOI_TAG      0xC0
#define QOI_TAG_MASK 0x3F

#define QOI_OP_RGB   0xFE
#define QOI_OP_RGBA  0xFF

#define QOI_OP_INDEX 0x00
#define QOI_OP_DIFF  0x40
#define QOI_OP_LUMA  0x80
#define QOI_OP_RUN   0xC0

enum qoi_pixel_color {QOI_RED, QOI_GREEN, QOI_BLUE, QOI_ALPHA};
enum qoi_channels {QOI_WHITESPACE = 3, QOI_TRANSPARENT = 4};
enum qoi_colorspace {QOI_SRGB, QOI_LINEAR};

/* QOI end of file */
static const uint8_t QOI_PADDING[8] = {0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x01};

/* QOI descriptor as read by the header */
typedef struct
{
    uint32_t width; /* in big endian */
    uint32_t height; /* in big endian */
    uint8_t channels;
    uint8_t colorspace;
} qoi_desc_t;

/* pixel values */
typedef union
{
    struct {
        uint8_t red;
        uint8_t green;
        uint8_t blue;
        uint8_t alpha;
    };
    uint8_t channels[4];
} qoi_pixel_t;

typedef struct
{
    /*
        A running array[64] (zero-initialized) of previously seen pixel
        values is maintained by the encoder and decoder. Each pixel that is
        seen by the encoder and decoder is put into this array at the
        position formed by a hash function of the color value. 
    */
    qoi_pixel_t buffer[64];

    qoi_pixel_t prev_pixel;

    size_t pixel_offset, len;

    uint8_t* data;
    uint8_t* offset;

    uint8_t run : 8;
    uint32_t pad : 24;

} qoi_enc_t;

typedef struct
{
    /*
        A running array[64] (zero-initialized) of previously seen pixel
        values is maintained by the encoder and decoder. Each pixel that is
        seen by the encoder and decoder is put into this array at the
        position formed by a hash function of the color value. 
    */
    qoi_pixel_t buffer[64];

    size_t len;

    uint8_t* data;
    uint8_t* offset;

    uint8_t run : 8;
    uint32_t pad : 24;
} qoi_dec_t;

/* Machine specific code */

static inline void qoi_swap_bytes_32(void* byte);
static bool is_little_endian();

/* Pixel related code */

void qoi_set_pixel_rgb(qoi_pixel_t* pixel, uint8_t red, uint8_t green, uint8_t blue);
void qoi_set_pixel_rgba(qoi_pixel_t* pixel, uint8_t red, uint8_t green, uint8_t blue, uint8_t alpha);

void qoi_initalize_pixel(qoi_pixel_t* pixel);
static bool qoi_cmp_pixel(qoi_pixel_t pixel1, qoi_pixel_t pixel2, const uint8_t channels);
static inline int32_t qoi_get_index_position(qoi_pixel_t pixel);

/* QOI descriptor functions */

bool qoi_desc_init(qoi_desc_t *desc);
void qoi_set_dimensions(qoi_desc_t *desc, uint32_t width, uint32_t height);
void qoi_set_channels(qoi_desc_t* desc, uint8_t channels);
void qoi_set_colorspace(qoi_desc_t* desc, uint8_t colorspace);

bool read_qoi_header(qoi_desc_t *desc, void* data);

/* QOI decoder functions */

bool qoi_dec_init(qoi_dec_t* dec, void* data, size_t len);
bool qoi_dec_done(qoi_dec_t* dec);
qoi_pixel_t qoi_decode_chunk(qoi_dec_t* dec, qoi_pixel_t pixel);

/* Swap a 32 bit word between endianess */
static inline void qoi_swap_bytes_32(void* byte)
{
    uint32_t* byte_ptr = (uint32_t*)byte;
    #ifdef _MSC_VER
    *byte_ptr = _byteswap_ulong(*byte_ptr);

    #elif defined(__GNUC__)
    *byte_ptr = __builtin_bswap32(*byte_ptr);

    #else
    const uint32_t value = *byte;
    *byte_ptr = ((value >> 24) & 0xFF | (value << 8) & 0xFF0000 | (value >> 8) & 0xFF00 | (value >> 24) & 0xFF000000)
    #endif
}

/* https://stackoverflow.com/a/4240014 */

/* Check if machine is little endian for converting big endian values to little endian values */
static bool is_little_endian()
{
    const uint32_t val = 1;
    return (*((char*)&val) == 1);

}

/* Compares two pixels for the same color */
static bool qoi_cmp_pixel(qoi_pixel_t pixel1, qoi_pixel_t pixel2, const uint8_t channels)
{
    bool is_same_pixel = (
        pixel1.red == pixel2.red &&
        pixel1.green == pixel2.green &&
        pixel1.blue == pixel2.blue);

    if (channels > 3)
        is_same_pixel = is_same_pixel && (pixel1.alpha == pixel2.alpha);

    return is_same_pixel;
}

/* Sets the RGB pixel by a certain pixel value */
inline void qoi_set_pixel_rgb(qoi_pixel_t* pixel, uint8_t red, uint8_t green, uint8_t blue)
{
    pixel->red = red;
    pixel->green = green;
    pixel->blue = blue;
}

/* Sets the RGBA pixel by a certain pixel value including an transparency alpha value */
inline void qoi_set_pixel_rgba(qoi_pixel_t* pixel, uint8_t red, uint8_t green, uint8_t blue, uint8_t alpha)
{
    pixel->red = red;
    pixel->green = green;
    pixel->blue = blue;
    pixel->alpha = alpha;
}

/* Initalizes the pixels to the default state */
void qoi_initalize_pixel(qoi_pixel_t* pixel)
{
    if (pixel == NULL) return;
    qoi_set_pixel_rgba(pixel, 0, 0, 0, 0);
}

/* Hashing function for pixels: up to 64 possible hash values */
static inline int32_t qoi_get_index_position(qoi_pixel_t pixel)
{
    return (pixel.red * 3 + pixel.green * 5 + pixel.blue * 7 + pixel.alpha * 11) % 64;
}

/* Initalize the QOI desciptor to the default value */
bool qoi_desc_init(qoi_desc_t *desc)
{
    if (desc == NULL) return false;

    desc->width = 0;
    desc->height = 0;
    desc->channels = 0;
    desc->colorspace = 0;

    return true;
};

/* Sets the image dimensions of an image for QOI descriptor */
inline void qoi_set_dimensions(qoi_desc_t* desc, uint32_t width, uint32_t height)
{
    desc->width = width;
    desc->height = height;
}

/* Sets the amount of channels of an image for QOI descriptor */
inline void qoi_set_channels(qoi_desc_t* desc, uint8_t channels)
{
    desc->channels = channels;
}

/* Sets the colorspace of an image for QOI descriptor */
inline void qoi_set_colorspace(qoi_desc_t *desc, uint8_t colorspace)
{
    desc->colorspace = colorspace;
}

/* Writes the QOI metadata information to the file */
void write_qoi_header(qoi_desc_t *desc, void* dest)
{
    if (dest == NULL || desc == NULL) return;

    uint8_t *byte = (uint8_t*)dest;

    /* Write the magic characters to the file first */
    byte[0] = 'q';
    byte[1] = 'o';
    byte[2] = 'i';
    byte[3] = 'f';

    /* Writes all the metadata information about the image to the file */

    if (is_little_endian())
    {
        uint32_t* dimension_ptr = (uint32_t*)&byte[4];
        uint32_t width_be = desc->width;
        uint32_t height_be = desc->height;

        qoi_swap_bytes_32(&width_be);
        qoi_swap_bytes_32(&height_be);

        dimension_ptr[0] = width_be;
        dimension_ptr[1] = height_be;
    }
    else
    {
        uint32_t* dimension_ptr = (uint32_t*)&byte[4];

        dimension_ptr[0] = desc->width;
        dimension_ptr[1] = desc->height;
    }

    byte[12] = desc->channels;
    byte[13] = desc->colorspace;
}

/* Check for vaild QOIF file */
bool read_qoi_header(qoi_desc_t *desc, void* data)
{
    if (data == NULL || desc == NULL) return false;

    uint8_t *byte = (uint8_t*)data;

    /* Check magic values for vaild QOIF file */
    if (!(byte[0] == 'q' &&
        byte[1] == 'o' &&
        byte[2] == 'i' &&
        byte[3] == 'f')
    ) return false;

    /* Read the header for information on how big should the image be and how to decode the image */

    qoi_set_dimensions(desc, *((uint32_t*)&byte[4]), *((uint32_t*)&byte[8]));
    qoi_set_channels(desc, *((uint8_t*)&byte[12]));
    qoi_set_colorspace(desc, *((uint8_t*)&byte[13]));

    if (is_little_endian())
    {
        qoi_swap_bytes_32(&desc->width);
        qoi_swap_bytes_32(&desc->height);
    }

    return true;
}

/* Initalize the QOI encoder to the default state */
bool qoi_enc_init(qoi_desc_t* desc, qoi_enc_t* enc, void* data)
{
    if (enc == NULL || desc == NULL || data == NULL) return false;

    for (uint8_t element = 0; element < 64; element++)
        qoi_initalize_pixel(&enc->buffer[element]);

    enc->len = desc->width * desc->height;

    enc->pad = 0;
    enc->run = 0;
    enc->pixel_offset = 0;

    /*  
        The decoder and encoder start with 
        {r: 0, g: 0, b: 0, a: 255}
        as the previous pixel value. 
    */
    qoi_set_pixel_rgba(&enc->prev_pixel, 0, 0, 0, 255);

    enc->data = (uint8_t*)data;
    enc->offset = enc->data + 14;

    return true;
}

/* Check if the encoder has finished processing the image */
bool qoi_enc_done(qoi_enc_t* enc)
{
    return (enc->pixel_offset >= enc->len);
}

/* Initalize the decoder to the default state */
bool qoi_dec_init(qoi_dec_t* dec, void* data, size_t len)
{
    if (dec == NULL || data == NULL) return false;

    /*
        A running array[64] (zero-initialized) of previously seen pixel
        values is maintained by the encoder and decoder. 
    */
    for (uint8_t element = 0; element < 64; element++)
        qoi_initalize_pixel(&dec->buffer[element]);
    
    dec->run = 0;
    dec->pad = 0;
    
    dec->len = len;

    dec->data = (uint8_t*)data;
    dec->offset = dec->data + 14;

    return true;
}

/* Has the decoder reached the end of file? */
bool qoi_dec_done(qoi_dec_t* dec)
{
    return (dec->offset - dec->data > dec->len - 8);
}

/* 
    WARNING: In this function below, you must provide enough memory to put the encoded images 
    The safest amount of space to store encoded images is the equation below

    (image width) * (image height) * ((amount of channels in a pixel) + 1) = bytes required to store encoded image
*/

void qoi_encode_chunk(qoi_desc_t *desc, qoi_enc_t *enc, void *qoi_pixel_bytes)
{

    /* 
        Assume that the pixel byte order is the following below
        bytes[0] = red;
        bytes[1] = green;
        bytes[2] = blue;
        bytes[3] = alpha;
    */

    qoi_pixel_t cur_pixel = *((qoi_pixel_t*)qoi_pixel_bytes);

    /* Assume an RGB pixel with three channels has an alpha value that makes pixels opaque */
    if (desc->channels < 4) 
        cur_pixel.alpha = 255;

    uint8_t index_pos = qoi_get_index_position(cur_pixel);

    /* Increment run length by 1 if pixels are the same */
    if (qoi_cmp_pixel(cur_pixel, enc->prev_pixel, desc->channels))
    {
        /*  Note that the runlengths 63 and 64 (b111110 and b111111) are illegal as they are
            occupied by the QOI_OP_RGB and QOI_OP_RGBA tags. */
        if (++enc->run >= 62 || enc->pixel_offset >= enc->len)
        {
            /* The run-length is stored with a bias of -1 */
            uint8_t tag = QOI_OP_RUN | (enc->run - 1);
            enc->run = 0;
            
            enc->offset++[0] = tag;
        }
    }
    else
    {
        if (enc->run > 0)
        {
            /*  Write opcode for because there are differences in pixels
                The run-length is stored with a bias of -1*/
            uint8_t tag = QOI_OP_RUN | (enc->run - 1);
            enc->run = 0;

            enc->offset++[0] = tag;

        }
        
        /* Check if pixels exist in one of the pixel hash buffers */
        if (qoi_cmp_pixel(enc->buffer[index_pos], cur_pixel, 4))
        {
            uint8_t tag = QOI_OP_INDEX | index_pos;
            enc->offset++[0] = tag;

        }
        else
        {
            enc->buffer[index_pos] = cur_pixel;

            /* QOI doesn't have opcodes for alpha values so check alpha values between two pixels first */
            if (desc->channels > 3 && cur_pixel.alpha != enc->prev_pixel.alpha)
            {
                uint8_t tag[5] =
                {
                    QOI_OP_RGBA,
                    cur_pixel.red,
                    cur_pixel.green,
                    cur_pixel.blue,
                    cur_pixel.alpha
                };

                enc->offset[0] = tag[0];
                enc->offset[1] = tag[1];
                enc->offset[2] = tag[2];
                enc->offset[3] = tag[3];
                enc->offset[4] = tag[4];
                enc->offset += 5;
            }
            else
            {
                /* Check the difference between color values to determine opcode */
                int8_t red_dif, green_dif, blue_dif;
                int8_t dr_dg, db_dg;

                red_dif = cur_pixel.red - enc->prev_pixel.red;
                green_dif = cur_pixel.green - enc->prev_pixel.green;
                blue_dif = cur_pixel.blue - enc->prev_pixel.blue;
                
                dr_dg = red_dif - green_dif;
                db_dg = blue_dif - green_dif;

                if (
                    red_dif >= -2 && red_dif <= 1 &&
                    green_dif >= -2 && green_dif <= 1 &&
                    blue_dif >= -2 && blue_dif <= 1
                )
                {
                    uint8_t tag =
                        QOI_OP_DIFF |
                        (uint8_t)(red_dif + 2) << 4 |
                        (uint8_t)(green_dif + 2) << 2 |
                        (uint8_t)(blue_dif + 2);
                        enc->offset[0] = tag;
                        enc->offset++;
                }

                else if (
                    dr_dg >= -8 && dr_dg <= 7 &&
                    green_dif >= -32 && green_dif <= 31 &&
                    db_dg >= -8 && db_dg <= 7
                )
                {
                    uint8_t tag[2] = {
                        QOI_OP_LUMA | (uint8_t)(green_dif + 32), 
                        (uint8_t)(dr_dg + 8) << 4 | (uint8_t)(db_dg + 8)
                    };
                    enc->offset[0] = tag[0];
                    enc->offset[1] = tag[1];
                    enc->offset += 2;
                }

                /* otherwise write an RGB tag containting the RGB values of a pixel */
                else
                {
                    uint8_t tag[4] = {
                        QOI_OP_RGB,
                        cur_pixel.red,
                        cur_pixel.green,
                        cur_pixel.blue
                    };

                    enc->offset[0] = tag[0];
                    enc->offset[1] = tag[1];
                    enc->offset[2] = tag[2];
                    enc->offset[3] = tag[3];

                    enc->offset += 4;
                }
            }

        }
    }

    /* Advance the pixel offset by one and sets the previous pixel to the current pixel */
    enc->prev_pixel = cur_pixel;
    enc->pixel_offset++;

    /* Write QOI padding when finished encoding the image */
    if (qoi_enc_done(enc))
    {
        enc->offset[0] = QOI_PADDING[0];
        enc->offset[1] = QOI_PADDING[1];
        enc->offset[2] = QOI_PADDING[2];
        enc->offset[3] = QOI_PADDING[3];
        enc->offset[4] = QOI_PADDING[4];
        enc->offset[5] = QOI_PADDING[5];
        enc->offset[6] = QOI_PADDING[6];
        enc->offset[7] = QOI_PADDING[7];
        enc->offset += 8;
    }
}


/* 
    WARNING: In this function below, you must provide enough memory to put the decoded images 
    The safest amount of space to store decoded images is the equation below

    (image width) * (image height) * (amount of channels in a pixel) = bytes required to store decoded image
*/

qoi_pixel_t qoi_decode_chunk(qoi_dec_t* dec, qoi_pixel_t pixel)
{
    qoi_pixel_t px = pixel; /* modified pixel to be returned */

    if (dec->run > 0)
        dec->run--;

    else
    {
        uint8_t tag = dec->offset[0]; /* opcode for qoi decompression */

        /*  
            The 8-bit tags have precedence over the 2-bit tags. 
            A decoder must check for the presence of an 8-bit tag first. 
        */

        if (tag == QOI_OP_RGB) /* RGB pixel */
        {
            px.red = dec->offset[1];
            px.green = dec->offset[2];
            px.blue = dec->offset[3];
            dec->offset += 4;
        }
        else if (tag == QOI_OP_RGBA) /* RGBA pixel */
        {
            px.red = dec->offset[1];
            px.green = dec->offset[2];
            px.blue = dec->offset[3];
            px.alpha = dec->offset[4];
            dec->offset += 5;
        }
        else
        {
            uint8_t tag_type = (tag & QOI_TAG); /* opcode for qoi decompression */

            switch(tag_type)
            {
                case QOI_OP_INDEX:
                {
                    px = dec->buffer[tag & QOI_TAG_MASK];
                    dec->offset += 1;

                    break;
                }
                case QOI_OP_DIFF:
                {
                    uint8_t diff = tag & QOI_TAG_MASK;
                    uint8_t red_diff = ((diff >> 4) & 0x03) - 2;
                    uint8_t green_diff = ((diff >> 2) & 0x03) - 2;
                    uint8_t blue_diff = (diff & 0x03) - 2;

                    px.red += red_diff;
                    px.green += green_diff;
                    px.blue += blue_diff;
                    dec->offset += 1;

                    break;
                }
                case QOI_OP_LUMA:
                {
                    uint8_t lumaGreen = (tag & QOI_TAG_MASK) - 32;

                    px.red += lumaGreen + ((dec->offset[1] & 0xF0) >> 4) - 8;
                    px.green += lumaGreen;
                    px.blue += lumaGreen + (dec->offset[1] & 0x0F) - 8;

                    dec->offset += 2;

                    break;
                }
                case QOI_OP_RUN:
                {
                    dec->run = tag & QOI_TAG_MASK;
                    dec->offset += 1;

                    break;
                }
                default:
                {
                    dec->offset += 1; /* move on to the new packet if there is an invaild opcode */

                    break;
                }
            }           
        }

        dec->buffer[qoi_get_index_position(px)] = px;           
    }
    
    return px;
}

#ifdef __cplusplus
}
#endif

#endif /* QOI_IMPLEMENTATION */

#endif /* QOI_H_IMPLEMENTATION */
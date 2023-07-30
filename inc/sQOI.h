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

    Define SIMPLIFIED_QOI_IMPLEMENTATION in only "one" of your source code files (.c, .cpp .cxx), not your header files
    before you include this library to create this implementation

    #define SIMPLIFIED_QOI_IMPLEMENTATION
    #include "sQOI.h"

*/

#ifndef SIMPLIFIED_QOI_H_IMPLEMENTATION
#define SIMPLIFIED_QOI_H_IMPLEMENTATION

#ifdef SIMPLIFIED_QOI_IMPLEMENTATION

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

/* QOI magic number */
static const uint8_t QOI_MAGIC[4] = {'q', 'o', 'i', 'f'};

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
    uint32_t concatenated_pixel_values;
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

    qoi_pixel_t prev_pixel;

    size_t pixel_seek, img_area, qoi_len;

    uint8_t* data;
    uint8_t* offset;

    uint8_t run : 8;
    uint32_t pad : 24;
} qoi_dec_t;

/* Machine specific code */

static inline uint32_t qoi_get_be32(uint32_t value);
static inline uint32_t qoi_to_be32(uint32_t value);

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

void write_qoi_header(qoi_desc_t *desc, void* dest);
bool read_qoi_header(qoi_desc_t *desc, void* data);

/* QOI encoder functions */

bool qoi_enc_init(qoi_desc_t* desc, qoi_enc_t* enc, void* data);
bool qoi_enc_done(qoi_enc_t* enc);

void qoi_encode_chunk(qoi_desc_t *desc, qoi_enc_t *enc, void *qoi_pixel_bytes);

static inline void qoi_enc_rgb(qoi_enc_t *enc, qoi_pixel_t px);
static inline void qoi_enc_rgba(qoi_enc_t *enc, qoi_pixel_t px);

static inline void qoi_enc_index(qoi_enc_t *enc, uint8_t index_pos);
static inline void qoi_enc_diff(qoi_enc_t *enc, uint8_t red_diff, uint8_t green_diff, uint8_t blue_diff);
static inline void qoi_enc_luma(qoi_enc_t *enc, uint8_t green_diff, uint8_t dr_dg, uint8_t db_dg);
static inline void qoi_enc_run(qoi_enc_t *enc);

/* QOI decoder functions */

bool qoi_dec_init(qoi_desc_t* desc, qoi_dec_t* dec, void* data, size_t len);
bool qoi_dec_done(qoi_dec_t* dec);

qoi_pixel_t qoi_decode_chunk(qoi_dec_t* dec);

static inline void qoi_dec_rgb(qoi_dec_t* dec);
static inline void qoi_dec_rgba(qoi_dec_t* dec);

static inline void qoi_dec_index(qoi_dec_t* dec, uint8_t tag); 
static inline void qoi_dec_diff(qoi_dec_t* dec, uint8_t tag);
static inline void qoi_dec_luma(qoi_dec_t* dec, uint8_t tag);
static inline void qoi_dec_run(qoi_dec_t* dec, uint8_t tag);

/* Extract a 32-bit big endian integer regardless of endianness */
static inline uint32_t qoi_get_be32(uint32_t value)
{
    uint8_t* bytes = (uint8_t*)&value;
    uint32_t be_value = (uint32_t) (
            (bytes[0] << 24) |
            (bytes[1] << 16) |
            (bytes[2] << 8) |
            (bytes[3])
        );
    
    return be_value;
}

/* Write a 32-bit big endian integer regardless of endianness */
static inline uint32_t qoi_to_be32(uint32_t value)
{
    uint8_t bytes[4];

    bytes[0] = (value >> 24);
    bytes[1] = (value >> 16);
    bytes[2] = (value >> 8);
    bytes[3] = (value);
    
    return *((uint32_t*)bytes);
}

/* Compares two pixels for the same color */
static bool qoi_cmp_pixel(qoi_pixel_t pixel1, qoi_pixel_t pixel2, const uint8_t channels)
{
    if (channels < 4) /* RGB pixels have three channels; RGBA pixels have four channels for the alpha channel */
    {
        /* O2 optimization will mask out these alpha values using AND instruction so it compares only the colors of a pixel */
        pixel1.alpha = 0;
        pixel2.alpha = 0;
    }

    return pixel1.concatenated_pixel_values == pixel2.concatenated_pixel_values; /* compare pixels */
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
    byte[0] = QOI_MAGIC[0];
    byte[1] = QOI_MAGIC[1];
    byte[2] = QOI_MAGIC[2];
    byte[3] = QOI_MAGIC[3];

    /* Writes all the metadata information about the image to the file */

    uint32_t* dimension_ptr = (uint32_t*)&byte[4];

    /* Writes the width and height values of the image to QOI header which stores them in big endian */
    dimension_ptr[0] = qoi_to_be32(desc->width);
    dimension_ptr[1] = qoi_to_be32(desc->height);

    byte[12] = desc->channels;
    byte[13] = desc->colorspace;
}

/* Check for vaild QOIF file */
bool read_qoi_header(qoi_desc_t *desc, void* data)
{
    if (data == NULL || desc == NULL) return false;

    uint8_t *byte = (uint8_t*)data;

    /* Check magic values for vaild QOIF file */
    if (!(byte[0] == QOI_MAGIC[0] &&
        byte[1] == QOI_MAGIC[1] &&
        byte[2] == QOI_MAGIC[2] &&
        byte[3] == QOI_MAGIC[3])
    ) return false;

    /* Read the header for information on how big should the image be and how to decode the image */

    qoi_set_dimensions(desc, *((uint32_t*)&byte[4]), *((uint32_t*)&byte[8]));
    qoi_set_channels(desc, *((uint8_t*)&byte[12]));
    qoi_set_colorspace(desc, *((uint8_t*)&byte[13]));

    /* Get width and height of the image from QOI header which stores these values in big endian */
    desc->width = qoi_get_be32(desc->width);
    desc->height = qoi_get_be32(desc->height);

    return true;
}

/* Initalize the QOI encoder to the default state */
bool qoi_enc_init(qoi_desc_t* desc, qoi_enc_t* enc, void* data)
{
    if (enc == NULL || desc == NULL || data == NULL) return false;

    for (uint8_t element = 0; element < 64; element++)
        qoi_initalize_pixel(&enc->buffer[element]); /* Initalize all the pixels in the buffer to zero for each channel of each pixels */

    enc->len = (size_t)desc->width * (size_t)desc->height;

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
    return (enc->pixel_offset >= enc->len); /* Has the encoder encoded all the pixels yet? */
}

/* Initalize the decoder to the default state */
bool qoi_dec_init(qoi_desc_t* desc, qoi_dec_t* dec, void* data, size_t len)
{
    if (dec == NULL || data == NULL || len < 14) return false;

    /*
        A running array[64] (zero-initialized) of previously seen pixel
        values is maintained by the encoder and decoder. 
    */
    for (uint8_t element = 0; element < 64; element++)
        qoi_initalize_pixel(&dec->buffer[element]);

    qoi_set_pixel_rgba(&dec->prev_pixel, 0, 0, 0, 255);
    
    dec->run = 0;
    dec->pad = 0;
    
    dec->pixel_seek = 0;
    dec->img_area = (size_t)desc->width * (size_t)desc->height;
    dec->qoi_len = len;

    dec->data = (uint8_t*)data;
    dec->offset = dec->data + 14;

    return true;
}

/* Has the decoder reached the end of file? */
bool qoi_dec_done(qoi_dec_t* dec)
{
    /* Subtract eight from qoi_len because of QOI padding */
    return (dec->offset - dec->data > dec->qoi_len - 8) || (dec->pixel_seek >= dec->img_area); /* Has the decoder decoded all the pixels yet or reached the end of the file? */
}

/* Place the RGB information into the QOI file */
static inline void qoi_enc_rgb(qoi_enc_t *enc, qoi_pixel_t px)
{
    uint8_t tag[4] = {
        QOI_OP_RGB,
        px.red,
        px.green,
        px.blue
    };

    enc->offset[0] = tag[0]; /* RGB opcode */
    enc->offset[1] = tag[1]; /* Red */
    enc->offset[2] = tag[2]; /* Green */
    enc->offset[3] = tag[3]; /* Blue */

    enc->offset += 4;
}

/* Place the RGBA information into the QOI file */
static inline void qoi_enc_rgba(qoi_enc_t *enc, qoi_pixel_t px)
{
    uint8_t tag[5] =
    {
        QOI_OP_RGBA,
        px.red,
        px.green,
        px.blue,
        px.alpha
    };

    enc->offset[0] = tag[0]; /* RGBA opcode */
    enc->offset[1] = tag[1]; /* Red */
    enc->offset[2] = tag[2]; /* Green */
    enc->offset[3] = tag[3]; /* Blue */
    enc->offset[4] = tag[4]; /* Alpha */

    enc->offset += 5;
}

/* Place the index position of the buffer into the QOI file */
static inline void qoi_enc_index(qoi_enc_t *enc, uint8_t index_pos)
{
    /* The run-length is stored with a bias of -1 */
    uint8_t tag = QOI_OP_INDEX | index_pos;
    enc->offset++[0] = tag;
}

/* Place the differences between color values into the QOI file */
static inline void qoi_enc_diff(qoi_enc_t *enc, uint8_t red_diff, uint8_t green_diff, uint8_t blue_diff)
{
    uint8_t tag =
        QOI_OP_DIFF |
        (uint8_t)(red_diff + 2) << 4 |
        (uint8_t)(green_diff + 2) << 2 |
        (uint8_t)(blue_diff + 2);

        enc->offset[0] = tag;
        
        enc->offset++;
}

/* Place the luma values into the QOI file */
static inline void qoi_enc_luma(qoi_enc_t *enc, uint8_t green_diff, uint8_t dr_dg, uint8_t db_dg)
{
    uint8_t tag[2] = {
        QOI_OP_LUMA | (uint8_t)(green_diff + 32), 
        (uint8_t)(dr_dg + 8) << 4 | (uint8_t)(db_dg + 8)
    };

    enc->offset[0] = tag[0];
    enc->offset[1] = tag[1];

    enc->offset += 2;
}

/* Place the run length of a pixel color information into the QOI file */
static inline void qoi_enc_run(qoi_enc_t *enc)
{
    /* The run-length is stored with a bias of -1 */
    uint8_t tag = QOI_OP_RUN | (enc->run - 1);
    enc->run = 0;
    
    enc->offset++[0] = tag;
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
            qoi_enc_run(enc);
        }
    }
    else
    {
        if (enc->run > 0)
        {
            /*  Write opcode for because there are differences in pixels
                The run-length is stored with a bias of -1 */
            qoi_enc_run(enc);
        }
        
        /* Check if pixels exist in one of the pixel hash buffers */
        if (qoi_cmp_pixel(enc->buffer[index_pos], cur_pixel, 4))
        {
            qoi_enc_index(enc, index_pos);
        }
        else
        {
            enc->buffer[index_pos] = cur_pixel;

            /* QOI doesn't have opcodes for alpha values so check alpha values between two pixels first */
            if (desc->channels > 3 && cur_pixel.alpha != enc->prev_pixel.alpha)
            {
                qoi_enc_rgba(enc, cur_pixel);
            }
            else
            {
                /* Check the difference between color values to determine opcode */
                int8_t red_diff, green_diff, blue_diff;
                int8_t dr_dg, db_dg;

                red_diff = cur_pixel.red - enc->prev_pixel.red;
                green_diff = cur_pixel.green - enc->prev_pixel.green;
                blue_diff = cur_pixel.blue - enc->prev_pixel.blue;
                
                dr_dg = red_diff - green_diff;
                db_dg = blue_diff - green_diff;

                if (
                    red_diff >= -2 && red_diff <= 1 &&
                    green_diff >= -2 && green_diff <= 1 &&
                    blue_diff >= -2 && blue_diff <= 1
                )
                {
                    qoi_enc_diff(enc, red_diff, green_diff, blue_diff);
                }

                else if (
                    dr_dg >= -8 && dr_dg <= 7 &&
                    green_diff >= -32 && green_diff <= 31 &&
                    db_dg >= -8 && db_dg <= 7
                )
                {
                    qoi_enc_luma(enc, green_diff, dr_dg, db_dg);
                }

                /* otherwise write an RGB tag containting the RGB values of a pixel */
                else
                {
                    qoi_enc_rgb(enc, cur_pixel);
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

/* Get and set the RGB values from the QOI file */
static inline void qoi_dec_rgb(qoi_dec_t* dec)
{
    dec->prev_pixel.red = dec->offset[1];
    dec->prev_pixel.green = dec->offset[2];
    dec->prev_pixel.blue = dec->offset[3];

    dec->offset += 4;
}

/* Get and set the RGB values from the QOI file */
static inline void qoi_dec_rgba(qoi_dec_t* dec)
{
    dec->prev_pixel.red = dec->offset[1];
    dec->prev_pixel.green = dec->offset[2];
    dec->prev_pixel.blue = dec->offset[3];
    dec->prev_pixel.alpha = dec->offset[4];

    dec->offset += 5;
}

/* Get and set the seek position of the buffer from the QOI file */
static inline void qoi_dec_index(qoi_dec_t* dec, uint8_t tag)
{
    dec->prev_pixel = dec->buffer[tag & QOI_TAG_MASK];

    dec->offset += 1;
}

/* Get the differences between values of the color channels from the QOI file */
static inline void qoi_dec_diff(qoi_dec_t* dec, uint8_t tag)
{
    uint8_t diff = tag & QOI_TAG_MASK;

    /* Do some wizardary to get the differences between three color channel  values */

    uint8_t red_diff = ((diff >> 4) & 0x03) - 2;
    uint8_t green_diff = ((diff >> 2) & 0x03) - 2;
    uint8_t blue_diff = (diff & 0x03) - 2;

    /* Add up the differences between three color channel values individually */

    dec->prev_pixel.red += red_diff;
    dec->prev_pixel.green += green_diff;
    dec->prev_pixel.blue += blue_diff;

    dec->offset += 1;
}

/* Gets the luma values from the QOI file */
static inline void qoi_dec_luma(qoi_dec_t* dec, uint8_t tag)
{
    uint8_t lumaGreen = (tag & QOI_TAG_MASK) - 32;

    /* Do some lumaGreen wizardary to get and add the differences between three color channel values */

    dec->prev_pixel.red += lumaGreen + ((dec->offset[1] & 0xF0) >> 4) - 8;
    dec->prev_pixel.green += lumaGreen;
    dec->prev_pixel.blue += lumaGreen + (dec->offset[1] & 0x0F) - 8;

    dec->offset += 2;
}

/* Gets the run length of the pixel from the QOI file */
static inline void qoi_dec_run(qoi_dec_t* dec, uint8_t tag){
    dec->run = tag & QOI_TAG_MASK;

    dec->offset += 1;
}

/* 
    WARNING: In this function below, you must provide enough memory to put the decoded images 
    The safest amount of space to store decoded images is the equation below

    (image width) * (image height) * (amount of channels in a pixel) = bytes required to store decoded image
*/

qoi_pixel_t qoi_decode_chunk(qoi_dec_t* dec)
{

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
            qoi_dec_rgb(dec);
        }
        else if (tag == QOI_OP_RGBA) /* RGBA pixel */
        {
            qoi_dec_rgba(dec);
        }
        else
        {
            uint8_t tag_type = (tag & QOI_TAG); /* opcode for qoi decompression */

            switch(tag_type)
            {
                case QOI_OP_INDEX:
                {
                    qoi_dec_index(dec, tag);

                    break;
                }
                case QOI_OP_DIFF:
                {
                    qoi_dec_diff(dec, tag);

                    break;
                }
                case QOI_OP_LUMA:
                {
                    qoi_dec_luma(dec, tag);

                    break;
                }
                case QOI_OP_RUN:
                {
                    qoi_dec_run(dec, tag);

                    break;
                }
                default:
                {
                    dec->offset += 1; /* move on to the new packet if there is an invaild opcode */

                    break;
                }
            }           
        }

        dec->buffer[qoi_get_index_position(dec->prev_pixel)] = dec->prev_pixel;           
    }
    
    dec->pixel_seek++;
    return dec->prev_pixel;
}

#ifdef __cplusplus
}
#endif

#endif /* SIMPLIFIED_QOI_IMPLEMENTATION */

#endif /* SIMPLIFIED_QOI_H_IMPLEMENTATION */
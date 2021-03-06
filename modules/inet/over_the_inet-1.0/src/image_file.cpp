#include "image_file.h"


void
read_png_file (FILE *fp, struct Timage &image)
{
  png_structp png =
    png_create_read_struct (PNG_LIBPNG_VER_STRING, NULL, NULL, NULL);
  if (!png)
    abort ();

  png_infop info = png_create_info_struct (png);
  if (!info)
    abort ();

  if (setjmp (png_jmpbuf (png)))
    abort ();

  png_init_io (png, fp);

  png_set_user_limits (png, 0x7fffffffL, 0x7fffffffL);

  png_read_info (png, info);

  image.width = png_get_image_width (png, info);
  image.height = png_get_image_height (png, info);
  image.color_type = png_get_color_type (png, info);
  image.bit_depth = png_get_bit_depth (png, info);

#ifndef NDEBUG
  fprintf(stderr, "width: %i\n"
                  "height: %i\n"
                  "depth: %i\n", image.width, image.height, image.bit_depth);
#endif
  // Read any color_type into 8bit depth, RGBA format.

  if (image.bit_depth == 16)
    png_set_strip_16 (png);

  if (image.color_type == PNG_COLOR_TYPE_PALETTE)
    png_set_palette_to_rgb (png);

  // PNG_COLOR_TYPE_GRAY_ALPHA is always 8 or 16bit depth.
  if (image.color_type == PNG_COLOR_TYPE_GRAY && image.bit_depth < 8)
    png_set_expand_gray_1_2_4_to_8 (png);

  if (png_get_valid (png, info, PNG_INFO_tRNS))
    png_set_tRNS_to_alpha (png);

  // These color_type don't have an alpha channel then fill it with 0xff.
  if (image.color_type == PNG_COLOR_TYPE_RGB ||
      image.color_type == PNG_COLOR_TYPE_GRAY ||
      image.color_type == PNG_COLOR_TYPE_PALETTE)
    png_set_filler (png, 0xFF, PNG_FILLER_AFTER);

  if (image.color_type == PNG_COLOR_TYPE_GRAY ||
      image.color_type == PNG_COLOR_TYPE_GRAY_ALPHA)
    png_set_gray_to_rgb (png);

  png_read_update_info (png, info);

  image.row_pointers =
    (png_bytep *) malloc (sizeof (png_bytep) * image.height);
  for (int y = 0; y < image.height; y++)
    {
      image.row_pointers[y] =
	(png_byte *) malloc (png_get_rowbytes (png, info));
    }

  png_read_image (png, image.row_pointers);

  fclose (fp);

  png_destroy_read_struct (&png, &info, NULL);
  png = NULL;
  info = NULL;
}

void
write_png_file (char *filename, struct Timage image)
{

  FILE *fp = fopen (filename, "wb");
  if (!fp)
    abort ();

  png_structp png =
    png_create_write_struct (PNG_LIBPNG_VER_STRING, NULL, NULL, NULL);
  if (!png)
    abort ();

  png_infop info = png_create_info_struct (png);
  if (!info)
    abort ();
  if (setjmp (png_jmpbuf (png)))
    abort ();

  png_init_io (png, fp);

  png_set_user_limits (png, 0x7fffffffL, 0x7fffffffL);

  // Output is 8bit depth, RGBA format.
  png_set_IHDR (png,
		info,
		image.width, image.height,
		8,
		PNG_COLOR_TYPE_RGBA,
		PNG_INTERLACE_NONE,
		PNG_COMPRESSION_TYPE_DEFAULT, PNG_FILTER_TYPE_DEFAULT);
  png_write_info (png, info);

  png_write_image (png, image.row_pointers);
  png_write_end (png, NULL);

  for (int y = 0; y < image.height; y++)
    {
      free (image.row_pointers[y]);
    }
  free (image.row_pointers);

  fclose (fp);

  if (png && info)
    png_destroy_write_struct (&png, &info);
}

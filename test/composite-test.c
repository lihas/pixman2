#include <gtk/gtk.h>
#include <stdlib.h>
#include <stdio.h>
#include "pixman.h"
#include "utils.h"

#define WIDTH	100
#define HEIGHT	100

typedef struct {
    const char *name;
    pixman_op_t op;
} Operator;

static const Operator operators[] = {
    { "PIXMAN_OP_CLEAR", PIXMAN_OP_CLEAR },
    { "PIXMAN_OP_SRC", PIXMAN_OP_SRC },
    { "PIXMAN_OP_DST", PIXMAN_OP_DST },
    { "PIXMAN_OP_OVER", PIXMAN_OP_OVER },
    { "PIXMAN_OP_OVER_REVERSE", PIXMAN_OP_OVER_REVERSE },
    { "PIXMAN_OP_IN", PIXMAN_OP_IN },
    { "PIXMAN_OP_IN_REVERSE", PIXMAN_OP_IN_REVERSE },
    { "PIXMAN_OP_OUT", PIXMAN_OP_OUT },
    { "PIXMAN_OP_OUT_REVERSE", PIXMAN_OP_OUT_REVERSE },
    { "PIXMAN_OP_ATOP", PIXMAN_OP_ATOP },
    { "PIXMAN_OP_ATOP_REVERSE", PIXMAN_OP_ATOP_REVERSE },
    { "PIXMAN_OP_XOR", PIXMAN_OP_XOR },
    { "PIXMAN_OP_ADD", PIXMAN_OP_ADD },
    { "PIXMAN_OP_SATURATE", PIXMAN_OP_SATURATE },

    { "PIXMAN_OP_MULTIPLY", PIXMAN_OP_MULTIPLY },
    { "PIXMAN_OP_SCREEN", PIXMAN_OP_SCREEN },
    { "PIXMAN_OP_OVERLAY", PIXMAN_OP_OVERLAY },
    { "PIXMAN_OP_DARKEN", PIXMAN_OP_DARKEN },
    { "PIXMAN_OP_LIGHTEN", PIXMAN_OP_LIGHTEN },
    { "PIXMAN_OP_COLOR_DODGE", PIXMAN_OP_COLOR_DODGE },
    { "PIXMAN_OP_COLOR_BURN", PIXMAN_OP_COLOR_BURN },
    { "PIXMAN_OP_HARD_LIGHT", PIXMAN_OP_HARD_LIGHT },
    { "PIXMAN_OP_SOFT_LIGHT", PIXMAN_OP_SOFT_LIGHT },
    { "PIXMAN_OP_DIFFERENCE", PIXMAN_OP_DIFFERENCE },
    { "PIXMAN_OP_EXCLUSION", PIXMAN_OP_EXCLUSION },
    { "PIXMAN_OP_HSL_HUE", PIXMAN_OP_HSL_HUE },
    { "PIXMAN_OP_HSL_SATURATION", PIXMAN_OP_HSL_SATURATION },
    { "PIXMAN_OP_HSL_COLOR", PIXMAN_OP_HSL_COLOR },
    { "PIXMAN_OP_HSL_LUMINOSITY", PIXMAN_OP_HSL_LUMINOSITY },
    { "PIXMAN_OP_FLASH_SUBTRACT", PIXMAN_OP_FLASH_SUBTRACT }
};

static uint32_t
reader (const void *src, int size)
{
    switch (size)
    {
    case 1:
	return *(uint8_t *)src;
    case 2:
	return *(uint16_t *)src;
    case 4:
	return *(uint32_t *)src;
    default:
	g_assert_not_reached();
    }
}

static void
writer (void *src, uint32_t value, int size)
{
    switch (size)
    {
    case 1:
	*(uint8_t *)src = value;
	break;

    case 2:
	*(uint16_t *)src = value;
	break;

    case 4:
	*(uint32_t *)src = value;
	break;
    }
}

int
main (int argc, char **argv)
{
    GtkWidget *window, *swindow;
    GtkWidget *table;
    uint32_t *dest = malloc (WIDTH * HEIGHT * 4);
    uint32_t *src = malloc (WIDTH * HEIGHT * 4);
    pixman_image_t *src_img;
    pixman_image_t *dest_img;
    int i;

    gtk_init (&argc, &argv);

    window = gtk_window_new (GTK_WINDOW_TOPLEVEL);
    g_signal_connect (window, "delete-event",
		      G_CALLBACK (gtk_main_quit),
		      NULL);
    table = gtk_table_new (G_N_ELEMENTS (operators) / 4, 4, TRUE);

    for (i = 0; i < WIDTH * HEIGHT; ++i)
        src[i] = 0x7f7f0000; /* red */

    src_img = pixman_image_create_bits (PIXMAN_a8r8g8b8,
					WIDTH, HEIGHT,
					src,
					WIDTH * 4);
    pixman_image_set_accessors (src_img, reader, writer);

    dest_img = pixman_image_create_bits (PIXMAN_a8r8g8b8,
					 WIDTH, HEIGHT,
					 dest,
					 WIDTH * 4);
    pixman_image_set_accessors (dest_img, reader, writer);

    for (i = 0; i < G_N_ELEMENTS (operators); ++i) {
        int j;
	GtkWidget *image;
	GdkPixbuf *pixbuf;
	GtkWidget *vbox;
	GtkWidget *label;

	vbox = gtk_vbox_new (FALSE, 0);

	label = gtk_label_new (operators[i].name);
	gtk_box_pack_start (GTK_BOX (vbox), label, FALSE, FALSE, 6);
	gtk_widget_show (label);

	for (j = 0; j < WIDTH * HEIGHT; ++j)
	    dest[j] = 0x7f00007f; /* blue */

	pixman_image_composite (operators[i].op, src_img, NULL, dest_img,
				0, 0, 0, 0, 0, 0, WIDTH, HEIGHT);
	pixbuf = pixbuf_from_argb32 (pixman_image_get_data (dest_img), TRUE,
				     WIDTH, HEIGHT, WIDTH * 4);
	image = gtk_image_new_from_pixbuf (pixbuf);
	gtk_box_pack_start (GTK_BOX (vbox), image, FALSE, FALSE, 0);
	gtk_widget_show (image);

	gtk_table_attach_defaults (GTK_TABLE (table), vbox,
				   i % 4, (i % 4) + 1, i / 4, (i / 4) + 1);
	gtk_widget_show (vbox);

	g_object_unref (pixbuf);
    }

    pixman_image_unref (src_img);
    free (src);
    pixman_image_unref (dest_img);
    free (dest);

    swindow = gtk_scrolled_window_new (NULL, NULL);
    gtk_scrolled_window_set_policy (GTK_SCROLLED_WINDOW (swindow),
				    GTK_POLICY_AUTOMATIC, GTK_POLICY_AUTOMATIC);
    gtk_scrolled_window_add_with_viewport (GTK_SCROLLED_WINDOW (swindow), table);
    gtk_widget_show (table);

    gtk_container_add (GTK_CONTAINER (window), swindow);
    gtk_widget_show (swindow);

    gtk_widget_show (window);

    gtk_main ();

    return 0;
}

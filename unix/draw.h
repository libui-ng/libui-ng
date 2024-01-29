// 5 may 2016

// draw.c
struct uiDrawContext {
	cairo_t *cr;
	GtkStyleContext *style;
};

struct uiImageBuffer {
	int Width;
	int Height;
	int Stride;

	cairo_surface_t *buf;
};

// drawpath.c
extern void uiprivRunPath(uiDrawPath *p, cairo_t *cr);
extern uiDrawFillMode uiprivPathFillMode(uiDrawPath *path);

// drawmatrix.c
extern void uiprivM2C(uiDrawMatrix *m, cairo_matrix_t *c);

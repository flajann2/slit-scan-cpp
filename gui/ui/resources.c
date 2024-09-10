#include <gio/gio.h>

#if defined (__ELF__) && ( __GNUC__ > 2 || (__GNUC__ == 2 && __GNUC_MINOR__ >= 6))
# define SECTION __attribute__ ((section (".gresource.gresource"), aligned (sizeof(void *) > 8 ? sizeof(void *) : 8)))
#else
# define SECTION
#endif

static const SECTION union { const guint8 data[3583]; const double alignment; void * const ptr;}  gresource_resource_data = {
  "\107\126\141\162\151\141\156\164\000\000\000\000\000\000\000\000"
  "\030\000\000\000\310\000\000\000\000\000\000\050\006\000\000\000"
  "\000\000\000\000\001\000\000\000\001\000\000\000\004\000\000\000"
  "\004\000\000\000\005\000\000\000\042\314\324\105\004\000\000\000"
  "\310\000\000\000\012\000\114\000\324\000\000\000\330\000\000\000"
  "\226\207\374\175\000\000\000\000\330\000\000\000\004\000\114\000"
  "\334\000\000\000\344\000\000\000\170\166\022\320\001\000\000\000"
  "\344\000\000\000\022\000\166\000\370\000\000\000\274\002\000\000"
  "\324\265\002\000\377\377\377\377\274\002\000\000\001\000\114\000"
  "\300\002\000\000\304\002\000\000\354\213\170\174\003\000\000\000"
  "\304\002\000\000\003\000\114\000\310\002\000\000\314\002\000\000"
  "\247\101\315\014\001\000\000\000\314\002\000\000\020\000\166\000"
  "\340\002\000\000\376\015\000\000\141\164\157\155\154\157\147\151"
  "\153\057\000\000\001\000\000\000\147\165\151\057\002\000\000\000"
  "\005\000\000\000\146\151\154\145\137\147\145\154\137\163\145\154"
  "\145\143\164\056\165\151\000\000\264\001\000\000\000\000\000\000"
  "\074\077\170\155\154\040\166\145\162\163\151\157\156\075\042\061"
  "\056\060\042\040\145\156\143\157\144\151\156\147\075\042\125\124"
  "\106\055\070\042\077\076\012\074\041\055\055\040\103\162\145\141"
  "\164\145\144\040\167\151\164\150\040\103\141\155\142\141\154\141"
  "\143\150\145\040\060\056\071\060\056\064\040\055\055\076\012\074"
  "\151\156\164\145\162\146\141\143\145\076\074\041\055\055\040\151"
  "\156\164\145\162\146\141\143\145\055\144\145\163\143\162\151\160"
  "\164\151\157\156\040\106\151\154\145\040\163\145\154\145\143\164"
  "\157\162\040\164\157\040\154\157\141\144\040\147\145\154\163\012"
  "\040\055\055\076\074\162\145\161\165\151\162\145\163\040\154\151"
  "\142\075\042\147\164\153\042\040\166\145\162\163\151\157\156\075"
  "\042\064\056\061\064\042\057\076\074\157\142\152\145\143\164\040"
  "\143\154\141\163\163\075\042\107\164\153\106\151\154\145\103\150"
  "\157\157\163\145\162\116\141\164\151\166\145\042\040\151\144\075"
  "\042\146\151\154\145\137\147\145\154\137\143\150\157\157\163\145"
  "\162\042\076\074\160\162\157\160\145\162\164\171\040\156\141\155"
  "\145\075\042\155\157\144\141\154\042\076\124\162\165\145\074\057"
  "\160\162\157\160\145\162\164\171\076\074\160\162\157\160\145\162"
  "\164\171\040\156\141\155\145\075\042\163\145\154\145\143\164\055"
  "\155\165\154\164\151\160\154\145\042\076\124\162\165\145\074\057"
  "\160\162\157\160\145\162\164\171\076\074\160\162\157\160\145\162"
  "\164\171\040\156\141\155\145\075\042\164\151\164\154\145\042\076"
  "\107\145\154\040\103\150\157\157\163\145\162\074\057\160\162\157"
  "\160\145\162\164\171\076\074\160\162\157\160\145\162\164\171\040"
  "\156\141\155\145\075\042\166\151\163\151\142\154\145\042\076\124"
  "\162\165\145\074\057\160\162\157\160\145\162\164\171\076\074\057"
  "\157\142\152\145\143\164\076\074\057\151\156\164\145\162\146\141"
  "\143\145\076\012\000\000\050\165\165\141\171\051\057\000\000\000"
  "\004\000\000\000\144\145\057\000\000\000\000\000\163\154\151\164"
  "\163\143\141\156\137\155\141\151\156\056\165\151\000\000\000\000"
  "\016\013\000\000\000\000\000\000\074\077\170\155\154\040\166\145"
  "\162\163\151\157\156\075\042\061\056\060\042\040\145\156\143\157"
  "\144\151\156\147\075\042\125\124\106\055\070\042\077\076\012\074"
  "\041\055\055\040\103\162\145\141\164\145\144\040\167\151\164\150"
  "\040\103\141\155\142\141\154\141\143\150\145\040\060\056\071\060"
  "\056\064\040\055\055\076\012\074\151\156\164\145\162\146\141\143"
  "\145\076\074\041\055\055\040\151\156\164\145\162\146\141\143\145"
  "\055\156\141\155\145\040\163\154\151\164\163\143\141\156\137\155"
  "\141\151\156\056\165\151\040\055\055\076\074\162\145\161\165\151"
  "\162\145\163\040\154\151\142\075\042\147\164\153\042\040\166\145"
  "\162\163\151\157\156\075\042\064\056\061\064\042\057\076\074\157"
  "\142\152\145\143\164\040\143\154\141\163\163\075\042\107\164\153"
  "\127\151\156\144\157\167\042\040\151\144\075\042\163\163\137\155"
  "\141\151\156\137\167\151\156\144\157\167\042\076\074\160\162\157"
  "\160\145\162\164\171\040\156\141\155\145\075\042\144\145\146\141"
  "\165\154\164\055\150\145\151\147\150\164\042\076\065\060\060\074"
  "\057\160\162\157\160\145\162\164\171\076\074\160\162\157\160\145"
  "\162\164\171\040\156\141\155\145\075\042\144\145\146\141\165\154"
  "\164\055\167\151\144\164\150\042\076\070\060\060\074\057\160\162"
  "\157\160\145\162\164\171\076\074\160\162\157\160\145\162\164\171"
  "\040\156\141\155\145\075\042\150\145\170\160\141\156\144\042\076"
  "\124\162\165\145\074\057\160\162\157\160\145\162\164\171\076\074"
  "\160\162\157\160\145\162\164\171\040\156\141\155\145\075\042\164"
  "\151\164\154\145\042\076\123\154\151\164\040\123\143\141\156\074"
  "\057\160\162\157\160\145\162\164\171\076\074\160\162\157\160\145"
  "\162\164\171\040\156\141\155\145\075\042\166\145\170\160\141\156"
  "\144\042\076\124\162\165\145\074\057\160\162\157\160\145\162\164"
  "\171\076\074\143\150\151\154\144\076\074\157\142\152\145\143\164"
  "\040\143\154\141\163\163\075\042\107\164\153\107\162\151\144\042"
  "\040\151\144\075\042\163\163\137\155\141\151\156\137\147\162\151"
  "\144\042\076\074\160\162\157\160\145\162\164\171\040\156\141\155"
  "\145\075\042\143\157\154\165\155\156\055\150\157\155\157\147\145"
  "\156\145\157\165\163\042\076\124\162\165\145\074\057\160\162\157"
  "\160\145\162\164\171\076\074\160\162\157\160\145\162\164\171\040"
  "\156\141\155\145\075\042\143\157\154\165\155\156\055\163\160\141"
  "\143\151\156\147\042\076\061\074\057\160\162\157\160\145\162\164"
  "\171\076\074\160\162\157\160\145\162\164\171\040\156\141\155\145"
  "\075\042\150\141\154\151\147\156\042\076\142\141\163\145\154\151"
  "\156\145\074\057\160\162\157\160\145\162\164\171\076\074\160\162"
  "\157\160\145\162\164\171\040\156\141\155\145\075\042\150\145\170"
  "\160\141\156\144\042\076\124\162\165\145\074\057\160\162\157\160"
  "\145\162\164\171\076\074\160\162\157\160\145\162\164\171\040\156"
  "\141\155\145\075\042\162\157\167\055\163\160\141\143\151\156\147"
  "\042\076\061\074\057\160\162\157\160\145\162\164\171\076\074\160"
  "\162\157\160\145\162\164\171\040\156\141\155\145\075\042\166\141"
  "\154\151\147\156\042\076\142\141\163\145\154\151\156\145\055\146"
  "\151\154\154\074\057\160\162\157\160\145\162\164\171\076\074\160"
  "\162\157\160\145\162\164\171\040\156\141\155\145\075\042\166\145"
  "\170\160\141\156\144\042\076\124\162\165\145\074\057\160\162\157"
  "\160\145\162\164\171\076\074\143\150\151\154\144\076\074\157\142"
  "\152\145\143\164\040\143\154\141\163\163\075\042\107\164\153\102"
  "\157\170\042\040\151\144\075\042\163\163\137\155\141\151\156\137"
  "\160\162\157\152\137\164\157\157\154\163\042\076\074\160\162\157"
  "\160\145\162\164\171\040\156\141\155\145\075\042\163\160\141\143"
  "\151\156\147\042\076\061\074\057\160\162\157\160\145\162\164\171"
  "\076\074\143\150\151\154\144\076\074\157\142\152\145\143\164\040"
  "\143\154\141\163\163\075\042\107\164\153\102\165\164\164\157\156"
  "\042\040\151\144\075\042\163\163\137\155\141\151\156\137\154\157"
  "\141\144\042\076\074\160\162\157\160\145\162\164\171\040\156\141"
  "\155\145\075\042\154\141\142\145\154\042\076\114\157\141\144\074"
  "\057\160\162\157\160\145\162\164\171\076\074\057\157\142\152\145"
  "\143\164\076\074\057\143\150\151\154\144\076\074\143\150\151\154"
  "\144\076\074\157\142\152\145\143\164\040\143\154\141\163\163\075"
  "\042\107\164\153\102\165\164\164\157\156\042\040\151\144\075\042"
  "\163\163\137\155\141\151\156\137\163\141\166\145\042\076\074\160"
  "\162\157\160\145\162\164\171\040\156\141\155\145\075\042\154\141"
  "\142\145\154\042\076\123\141\166\145\074\057\160\162\157\160\145"
  "\162\164\171\076\074\057\157\142\152\145\143\164\076\074\057\143"
  "\150\151\154\144\076\074\143\150\151\154\144\076\074\157\142\152"
  "\145\143\164\040\143\154\141\163\163\075\042\107\164\153\102\165"
  "\164\164\157\156\042\040\151\144\075\042\163\163\137\155\141\151"
  "\156\137\163\141\166\145\137\141\163\042\076\074\160\162\157\160"
  "\145\162\164\171\040\156\141\155\145\075\042\154\141\142\145\154"
  "\042\076\123\141\166\145\040\141\163\074\057\160\162\157\160\145"
  "\162\164\171\076\074\057\157\142\152\145\143\164\076\074\057\143"
  "\150\151\154\144\076\074\143\150\151\154\144\076\074\157\142\152"
  "\145\143\164\040\143\154\141\163\163\075\042\107\164\153\102\165"
  "\164\164\157\156\042\040\151\144\075\042\163\163\137\155\141\151"
  "\156\137\156\145\167\042\076\074\160\162\157\160\145\162\164\171"
  "\040\156\141\155\145\075\042\154\141\142\145\154\042\076\116\145"
  "\167\074\057\160\162\157\160\145\162\164\171\076\074\057\157\142"
  "\152\145\143\164\076\074\057\143\150\151\154\144\076\074\143\150"
  "\151\154\144\076\074\157\142\152\145\143\164\040\143\154\141\163"
  "\163\075\042\107\164\153\102\165\164\164\157\156\042\040\151\144"
  "\075\042\163\163\137\155\141\151\156\137\163\145\164\164\151\156"
  "\147\163\042\076\074\160\162\157\160\145\162\164\171\040\156\141"
  "\155\145\075\042\154\141\142\145\154\042\076\123\145\164\164\151"
  "\156\147\163\074\057\160\162\157\160\145\162\164\171\076\074\057"
  "\157\142\152\145\143\164\076\074\057\143\150\151\154\144\076\074"
  "\154\141\171\157\165\164\076\074\160\162\157\160\145\162\164\171"
  "\040\156\141\155\145\075\042\143\157\154\165\155\156\042\076\060"
  "\074\057\160\162\157\160\145\162\164\171\076\074\160\162\157\160"
  "\145\162\164\171\040\156\141\155\145\075\042\143\157\154\165\155"
  "\156\055\163\160\141\156\042\076\067\074\057\160\162\157\160\145"
  "\162\164\171\076\074\160\162\157\160\145\162\164\171\040\156\141"
  "\155\145\075\042\162\157\167\042\076\060\074\057\160\162\157\160"
  "\145\162\164\171\076\074\160\162\157\160\145\162\164\171\040\156"
  "\141\155\145\075\042\162\157\167\055\163\160\141\156\042\076\061"
  "\074\057\160\162\157\160\145\162\164\171\076\074\057\154\141\171"
  "\157\165\164\076\074\057\157\142\152\145\143\164\076\074\057\143"
  "\150\151\154\144\076\074\143\150\151\154\144\076\074\157\142\152"
  "\145\143\164\040\143\154\141\163\163\075\042\107\164\153\102\157"
  "\170\042\040\151\144\075\042\163\163\137\155\141\151\156\137\147"
  "\145\154\137\154\145\146\164\042\076\074\160\162\157\160\145\162"
  "\164\171\040\156\141\155\145\075\042\157\162\151\145\156\164\141"
  "\164\151\157\156\042\076\166\145\162\164\151\143\141\154\074\057"
  "\160\162\157\160\145\162\164\171\076\074\143\150\151\154\144\076"
  "\074\157\142\152\145\143\164\040\143\154\141\163\163\075\042\107"
  "\164\153\102\165\164\164\157\156\042\040\151\144\075\042\163\163"
  "\137\155\141\151\156\137\147\145\154\137\147\145\164\137\154\145"
  "\146\164\042\076\074\160\162\157\160\145\162\164\171\040\156\141"
  "\155\145\075\042\154\141\142\145\154\042\076\114\145\146\164\040"
  "\107\145\154\040\114\157\141\144\074\057\160\162\157\160\145\162"
  "\164\171\076\074\057\157\142\152\145\143\164\076\074\057\143\150"
  "\151\154\144\076\074\143\150\151\154\144\076\074\157\142\152\145"
  "\143\164\040\143\154\141\163\163\075\042\107\164\153\114\151\163"
  "\164\102\157\170\042\040\151\144\075\042\163\163\137\155\141\151"
  "\156\137\147\145\154\137\154\151\163\164\137\154\145\146\164\042"
  "\057\076\074\057\143\150\151\154\144\076\074\154\141\171\157\165"
  "\164\076\074\160\162\157\160\145\162\164\171\040\156\141\155\145"
  "\075\042\143\157\154\165\155\156\042\076\060\074\057\160\162\157"
  "\160\145\162\164\171\076\074\160\162\157\160\145\162\164\171\040"
  "\156\141\155\145\075\042\143\157\154\165\155\156\055\163\160\141"
  "\156\042\076\062\074\057\160\162\157\160\145\162\164\171\076\074"
  "\160\162\157\160\145\162\164\171\040\156\141\155\145\075\042\162"
  "\157\167\042\076\061\074\057\160\162\157\160\145\162\164\171\076"
  "\074\160\162\157\160\145\162\164\171\040\156\141\155\145\075\042"
  "\162\157\167\055\163\160\141\156\042\076\065\074\057\160\162\157"
  "\160\145\162\164\171\076\074\057\154\141\171\157\165\164\076\074"
  "\057\157\142\152\145\143\164\076\074\057\143\150\151\154\144\076"
  "\074\143\150\151\154\144\076\074\157\142\152\145\143\164\040\143"
  "\154\141\163\163\075\042\107\164\153\104\162\141\167\151\156\147"
  "\101\162\145\141\042\040\151\144\075\042\163\163\137\155\141\151"
  "\156\137\143\141\156\166\141\163\042\076\074\154\141\171\157\165"
  "\164\076\074\160\162\157\160\145\162\164\171\040\156\141\155\145"
  "\075\042\143\157\154\165\155\156\042\076\062\074\057\160\162\157"
  "\160\145\162\164\171\076\074\160\162\157\160\145\162\164\171\040"
  "\156\141\155\145\075\042\143\157\154\165\155\156\055\163\160\141"
  "\156\042\076\063\074\057\160\162\157\160\145\162\164\171\076\074"
  "\160\162\157\160\145\162\164\171\040\156\141\155\145\075\042\162"
  "\157\167\042\076\061\074\057\160\162\157\160\145\162\164\171\076"
  "\074\160\162\157\160\145\162\164\171\040\156\141\155\145\075\042"
  "\162\157\167\055\163\160\141\156\042\076\065\074\057\160\162\157"
  "\160\145\162\164\171\076\074\057\154\141\171\157\165\164\076\074"
  "\057\157\142\152\145\143\164\076\074\057\143\150\151\154\144\076"
  "\074\143\150\151\154\144\076\074\157\142\152\145\143\164\040\143"
  "\154\141\163\163\075\042\107\164\153\102\157\170\042\040\151\144"
  "\075\042\163\163\137\155\141\151\156\137\147\145\154\137\162\151"
  "\147\150\164\042\076\074\160\162\157\160\145\162\164\171\040\156"
  "\141\155\145\075\042\157\162\151\145\156\164\141\164\151\157\156"
  "\042\076\166\145\162\164\151\143\141\154\074\057\160\162\157\160"
  "\145\162\164\171\076\074\143\150\151\154\144\076\074\157\142\152"
  "\145\143\164\040\143\154\141\163\163\075\042\107\164\153\102\165"
  "\164\164\157\156\042\040\151\144\075\042\163\163\137\155\141\151"
  "\156\137\147\145\154\137\147\145\164\137\162\151\147\150\164\042"
  "\076\074\160\162\157\160\145\162\164\171\040\156\141\155\145\075"
  "\042\154\141\142\145\154\042\076\122\151\147\150\164\040\107\145"
  "\154\040\114\157\141\144\074\057\160\162\157\160\145\162\164\171"
  "\076\074\057\157\142\152\145\143\164\076\074\057\143\150\151\154"
  "\144\076\074\143\150\151\154\144\076\074\157\142\152\145\143\164"
  "\040\143\154\141\163\163\075\042\107\164\153\114\151\163\164\102"
  "\157\170\042\040\151\144\075\042\163\163\137\155\141\151\156\137"
  "\147\145\154\137\154\151\163\164\137\162\151\147\150\164\042\057"
  "\076\074\057\143\150\151\154\144\076\074\154\141\171\157\165\164"
  "\076\074\160\162\157\160\145\162\164\171\040\156\141\155\145\075"
  "\042\143\157\154\165\155\156\042\076\065\074\057\160\162\157\160"
  "\145\162\164\171\076\074\160\162\157\160\145\162\164\171\040\156"
  "\141\155\145\075\042\143\157\154\165\155\156\055\163\160\141\156"
  "\042\076\062\074\057\160\162\157\160\145\162\164\171\076\074\160"
  "\162\157\160\145\162\164\171\040\156\141\155\145\075\042\162\157"
  "\167\042\076\061\074\057\160\162\157\160\145\162\164\171\076\074"
  "\160\162\157\160\145\162\164\171\040\156\141\155\145\075\042\162"
  "\157\167\055\163\160\141\156\042\076\065\074\057\160\162\157\160"
  "\145\162\164\171\076\074\057\154\141\171\157\165\164\076\074\057"
  "\157\142\152\145\143\164\076\074\057\143\150\151\154\144\076\074"
  "\057\157\142\152\145\143\164\076\074\057\143\150\151\154\144\076"
  "\074\057\157\142\152\145\143\164\076\074\057\151\156\164\145\162"
  "\146\141\143\145\076\012\000\000\050\165\165\141\171\051" };

static GStaticResource static_resource = { gresource_resource_data.data, sizeof (gresource_resource_data.data) - 1 /* nul terminator */, NULL, NULL, NULL };

G_MODULE_EXPORT
GResource *gresource_get_resource (void);
GResource *gresource_get_resource (void)
{
  return g_static_resource_get_resource (&static_resource);
}
/* GLIB - Library of useful routines for C programming
 * Copyright (C) 1995-1997  Peter Mattis, Spencer Kimball and Josh MacDonald
 *
 * SPDX-License-Identifier: LGPL-2.1-or-later
 *
 * This library is free software; you can redistribute it and/or
 * modify it under the terms of the GNU Lesser General Public
 * License as published by the Free Software Foundation; either
 * version 2.1 of the License, or (at your option) any later version.
 *
 * This library is distributed in the hope that it will be useful,
 * but WITHOUT ANY WARRANTY; without even the implied warranty of
 * MERCHANTABILITY or FITNESS FOR A PARTICULAR PURPOSE.	 See the GNU
 * Lesser General Public License for more details.
 *
 * You should have received a copy of the GNU Lesser General Public
 * License along with this library; if not, see <http://www.gnu.org/licenses/>.
 */

/*
 * Modified by the GLib Team and others 1997-2000.  See the AUTHORS
 * file for a list of people on the GLib Team.  See the ChangeLog
 * files for a list of changes.  These files are distributed with
 * GLib at ftp://ftp.gtk.org/pub/gtk/.
 */

#ifndef __G_CONSTRUCTOR_H__
#define __G_CONSTRUCTOR_H__

/*
  If G_HAS_CONSTRUCTORS is true then the compiler support *both* constructors and
  destructors, in a usable way, including e.g. on library unload. If not you're on
  your own.

  Some compilers need #pragma to handle this, which does not work with macros,
  so the way you need to use this is (for constructors):

  #ifdef G_DEFINE_CONSTRUCTOR_NEEDS_PRAGMA
  #pragma G_DEFINE_CONSTRUCTOR_PRAGMA_ARGS(my_constructor)
  #endif
  G_DEFINE_CONSTRUCTOR(my_constructor)
  static void my_constructor(void) {
   ...
  }

*/

#ifndef __GTK_DOC_IGNORE__

#if  __GNUC__ > 2 || (__GNUC__ == 2 && __GNUC_MINOR__ >= 7)

#define G_HAS_CONSTRUCTORS 1

#define G_DEFINE_CONSTRUCTOR(_func) static void __attribute__((constructor)) _func (void);
#define G_DEFINE_DESTRUCTOR(_func) static void __attribute__((destructor)) _func (void);

#elif defined (_MSC_VER)

/*
 * Only try to include gslist.h if not already included via glib.h,
 * so that items using gconstructor.h outside of GLib (such as
 * GResources) continue to build properly.
 */
#ifndef __G_LIB_H__
#include "gslist.h"
#endif

#include <stdlib.h>

#define G_HAS_CONSTRUCTORS 1

/* We do some weird things to avoid the constructors being optimized
 * away on VS2015 if WholeProgramOptimization is enabled. First we
 * make a reference to the array from the wrapper to make sure its
 * references. Then we use a pragma to make sure the wrapper function
 * symbol is always included at the link stage. Also, the symbols
 * need to be extern (but not dllexport), even though they are not
 * really used from another object file.
 */

/* We need to account for differences between the mangling of symbols
 * for x86 and x64/ARM/ARM64 programs, as symbols on x86 are prefixed
 * with an underscore but symbols on x64/ARM/ARM64 are not.
 */
#ifdef _M_IX86
#define G_MSVC_SYMBOL_PREFIX "_"
#else
#define G_MSVC_SYMBOL_PREFIX ""
#endif

#define G_DEFINE_CONSTRUCTOR(_func) G_MSVC_CTOR (_func, G_MSVC_SYMBOL_PREFIX)
#define G_DEFINE_DESTRUCTOR(_func) G_MSVC_DTOR (_func, G_MSVC_SYMBOL_PREFIX)

#define G_MSVC_CTOR(_func,_sym_prefix) \
  static void _func(void); \
  extern int (* _array ## _func)(void);              \
  int _func ## _wrapper(void);              \
  int _func ## _wrapper(void) { _func(); g_slist_find (NULL,  _array ## _func); return 0; } \
  __pragma(comment(linker,"/include:" _sym_prefix # _func "_wrapper")) \
  __pragma(section(".CRT$XCU",read)) \
  __declspec(allocate(".CRT$XCU")) int (* _array ## _func)(void) = _func ## _wrapper;

#define G_MSVC_DTOR(_func,_sym_prefix) \
  static void _func(void); \
  extern int (* _array ## _func)(void);              \
  int _func ## _constructor(void);              \
  int _func ## _constructor(void) { atexit (_func); g_slist_find (NULL,  _array ## _func); return 0; } \
   __pragma(comment(linker,"/include:" _sym_prefix # _func "_constructor")) \
  __pragma(section(".CRT$XCU",read)) \
  __declspec(allocate(".CRT$XCU")) int (* _array ## _func)(void) = _func ## _constructor;

#elif defined(__SUNPRO_C)

/* This is not tested, but i believe it should work, based on:
 * http://opensource.apple.com/source/OpenSSL098/OpenSSL098-35/src/fips/fips_premain.c
 */

#define G_HAS_CONSTRUCTORS 1

#define G_DEFINE_CONSTRUCTOR_NEEDS_PRAGMA 1
#define G_DEFINE_DESTRUCTOR_NEEDS_PRAGMA 1

#define G_DEFINE_CONSTRUCTOR_PRAGMA_ARGS(_func) \
  init(_func)
#define G_DEFINE_CONSTRUCTOR(_func) \
  static void _func(void);

#define G_DEFINE_DESTRUCTOR_PRAGMA_ARGS(_func) \
  fini(_func)
#define G_DEFINE_DESTRUCTOR(_func) \
  static void _func(void);

#else

/* constructors not supported for this compiler */

#endif

#endif /* __GTK_DOC_IGNORE__ */
#endif /* __G_CONSTRUCTOR_H__ */

#ifdef G_HAS_CONSTRUCTORS

#ifdef G_DEFINE_CONSTRUCTOR_NEEDS_PRAGMA
#pragma G_DEFINE_CONSTRUCTOR_PRAGMA_ARGS(gresourceresource_constructor)
#endif
G_DEFINE_CONSTRUCTOR(gresourceresource_constructor)
#ifdef G_DEFINE_DESTRUCTOR_NEEDS_PRAGMA
#pragma G_DEFINE_DESTRUCTOR_PRAGMA_ARGS(gresourceresource_destructor)
#endif
G_DEFINE_DESTRUCTOR(gresourceresource_destructor)

#else
#warning "Constructor not supported on this compiler, linking in resources will not work"
#endif

static void gresourceresource_constructor (void)
{
  g_static_resource_init (&static_resource);
}

static void gresourceresource_destructor (void)
{
  g_static_resource_fini (&static_resource);
}
